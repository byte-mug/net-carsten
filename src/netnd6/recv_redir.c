/*
 *   Copyright 2016 Simon Schmidt
 *   Copyright 2011-2016 by Andrey Butok. FNET Community.
 *   Copyright 2008-2010 by Andrey Butok. Freescale Semiconductor, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <netnd6/ipv6check.h>
#include <netnd6/receive.h>
#include <netnd6/send.h>
#include <netnd6/nd6_header.h>
#include <netnd6/if.h>
#include <netnd6/table.h>
#include <netnd6/pktcheck.h>

#include <netipv6/defs.h>
#include <netipv6/ctrl.h>
#include <netipv6/check.h>
#include <netipv6/if.h>

#include <netif/ifapi.h>

#include <netstd/endianness.h>

void netnd6_redirect_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
	fnet_nd6_rd_header_t           *hdr;
	ipv6_addr_t                    target_addr;
	fnet_nd6_option_header_t       *nd_option;
	uint32_t                       size;
	char                           nd_option_tlla;
	fnet_nd6_option_lla_header_t   *hdr_tlla;
	fnet_nd6_neighbor_entry_t      *neighbor_cache_entry;
	ipv6_addr_t                    queue_addr;
	netpkt_t                       *pkts = 0;  /* Send-Chain. */
	
	hwaddr_t                       tlla_addr2 = nif->device_addr;
	
	/*
	 * Validation RFC4861 (8.1.1).
	 * The IP Hop Limit field has a value of 255, i.e., the packet
	 * could not possibly have been forwarded by a router.
	 */
	if( netnd6_check_hop_limit(pkt,255) ) goto DROP;
	
	/*
	 * The header must reside in contiguous area of memory.
	 *
	 * Validation RFC4861 (8.1.1). ICMP length (derived from the IP length) is 40 or more octets.
	 */
	if( netpkt_pullup(pkt,sizeof(fnet_nd6_rd_header_t)) ) goto DROP;
	
	hdr                 =  netpkt_data(pkt);
	target_addr         =  hdr->target_addr;
	
	/* Validation RFC4861 (8.1.1). */
	if(
		/* ICMP Code is 0.*/
		(hdr->icmp6_header.code != 0u) ||
		
		/* MUST be the link-local address.*/
		(!IP6_ADDR_IS_LINKLOCAL(*src_ip))||
		
		/* The ICMP Destination Address field in the redirect message does
		 * not contain a multicast address.*/
		(!IP6_ADDR_IS_MULTICAST(*dst_ip))||
		
		/* The ICMP Target Address is either a link-local address (when
		 * redirected to a router) or the same as the ICMP Destination
		 * Address (when redirected to the on-link destination). */
		((!IP6_ADDR_IS_LINKLOCAL(target_addr)) &&
			(!IP6ADDR_EQ(*dst_ip, target_addr)))
		
		/* TODO: The IP source address of the Redirect is the same as the current
		 * first-hop router for the specified ICMP Destination Address.*/
	) goto DROP;
	
	if( netpkt_pullfront(pkt,sizeof(fnet_nd6_na_header_t)) ) goto DROP;
	
	/************************************************************
	 * Handle posible options.
	 ************************************************************
	 * The contents of any defined options that are not specified
	 * to be used with Redirect messages MUST be
	 * ignored and the packet processed as normal. The only defined
	 * options that may appear are the Target Link-Layer Address,
	 * Prefix Information and MTU options.
	 ************************************************************/
	
	nd_option_tlla = 0;
	while(NETPKT_LENGTH(pkt)){
		if( netpkt_pullup(pkt,sizeof(fnet_nd6_option_header_t)) ) goto DROP;
		
		nd_option = netpkt_data(pkt);
		
		/*
		 * Validation RFC4861 (7.1.2). All included options have a length that
		 * is greater than zero.
		 */
		if(nd_option->length == 0u) goto DROP;
		
		size = nd_option->length<<3;
		
		/*
		 * Handle Source link-layer address option only.
		 */
		if( (nd_option->type == FNET_ND6_OPTION_SOURCE_LLA) && (size > sizeof(fnet_nd6_option_lla_header_t)) ) {
			if( netpkt_pullup(pkt,size) ) goto DROP;
			nd_option_tlla = 1;
			hdr_tlla = netpkt_data(pkt);
			netif_hwaddr_load(&tlla_addr2,hdr_tlla->addr);
		}
		/* else, silently ignore any options they do not recognize
		 * and continue processing the message.
		 */
		if( netpkt_pullfront(pkt,size) ) goto DROP;
	}
	
	net_mutex_lock(nif->nd6->nd6_lock);
	/*
	 * RFC4861: If the redirect contains a Target Link-Layer Address option, the host
	 * either creates or updates the Neighbor Cache entry for the target.
	 */
	if(nd_option_tlla){
		neighbor_cache_entry = netnd6_neighbor_cache_get(nif,&target_addr);
		if(! neighbor_cache_entry )
		{
			/*  If a Neighbor Cache entry is
			 * created for the target, its reachability state MUST be set to STALE
			 * as specified in Section 7.3.3. */
			neighbor_cache_entry = netnd6_neighbor_cache_add2(nif, &target_addr, &tlla_addr2, FNET_ND6_NEIGHBOR_STATE_STALE);
		}
		else
		{
			/* If a cache entry already existed and
			 * it is updated with a different link-layer address, its reachability
			 * state MUST also be set to STALE. */
			if( !netif_hwaddr_eq(&tlla_addr2, &(neighbor_cache_entry->ll_addr2)) )
			{
				neighbor_cache_entry->ll_addr2 = tlla_addr2;
				neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
			}
		}
		
		/* Sends any packets queued for the neighbor awaiting address resolution.
		 */
		pkts = neighbor_cache_entry->waiting_pkts;
		neighbor_cache_entry->waiting_pkts = 0;
		queue_addr = neighbor_cache_entry->ip_addr;
	}
	else
	{
		if(! neighbor_cache_entry )
		{
			/*  If a Neighbor Cache entry is
			 * created for the target, its reachability state MUST be set to STALE
			 * as specified in Section 7.3.3. */
			neighbor_cache_entry = netnd6_neighbor_cache_add2(nif, &target_addr, /*NULLPTR*/0, FNET_ND6_NEIGHBOR_STATE_STALE);
		}
	}
	
	/* If the Target Address is not the same
	 * as the Destination Address, the host MUST set IsRouter to TRUE for
	 * the target.*/
	if( !IP6ADDR_EQ(*dst_ip, target_addr) )
	{
		neighbor_cache_entry->is_router = 1;
		
		/* Add to redirect table.*/
		netnd6_redirect_table_add(nif,dst_ip,&target_addr);
	}
	net_mutex_unlock(nif->nd6->nd6_lock);
	
DROP:
	if(pkts){
		nif->netif_class->ifapi_send_l3_ipv6_all(nif,pkts,0/* Not needed here */,(void*)&queue_addr);
	}
	netpkt_free(pkt);
}

