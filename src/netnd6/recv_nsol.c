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


#include <netnd6/receive.h>
#include <netnd6/send.h>
#include <netnd6/nd6_header.h>
#include <netnd6/if.h>
#include <netnd6/table.h>
#include <netnd6/ipv6check.h>
#include <netnd6/pktcheck.h>


#include <netipv6/defs.h>
#include <netipv6/ctrl.h>
#include <netipv6/check.h>
#include <netipv6/if.h>

#include <netif/ifapi.h>

#include <netstd/endianness.h>

#define AS_MAC(a) *((mac_addr_t*)(a))

static void netnd6_nsol_handle_lla(
	netif_t *nif,
	uint32_t size,
	fnet_nd6_option_lla_header_t  *nd_option_slla,
	ipv6_addr_t *src_ip
);

void netnd6_neighbor_solicitation_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
	fnet_nd6_ns_header_t       *hdr;
	partial_ipv6_header_t      *iphdr;
	uint32_t                   size;
	ipv6_addr_t                target_addr;
	netipv6_if_addr_t          *target_nifaddr;
	fnet_nd6_option_header_t   *nd_option;
	
	ipv6_addr_t                ip_allnodes_addr = IP6_ADDR_LINKLOCAL_ALLNODES_INIT;
	
	
	/*
	 * Validation RFC4861 (7.1.1).
	 * The IP Hop Limit field has a value of 255, i.e., the packet
	 * could not possibly have been forwarded by a router.
	 */
	if( netnd6_check_hop_limit(pkt,255) ) goto DROP;
	
	/*
	 * The header must reside in contiguous area of memory.
	 *
	 * Validation RFC4861 (7.1.1). ICMP length (derived from the IP length) is 24 or more octets.
	 */
	if( netpkt_pullup(pkt,sizeof(fnet_nd6_ns_header_t)) ) goto DROP;
	
	hdr = netpkt_data(pkt);
	
	/* Validation RFC4861 (7.1.1). */
	if(
		(hdr->icmp6_header.code != 0u) ||  /* ICMP Code is 0.*/
		IP6_ADDR_IS_MULTICAST(hdr->target_addr)  /* Target Address is not a multicast address. */
	) goto DROP;
	
	target_addr = hdr->target_addr;
	
	if( netpkt_pullfront(pkt,sizeof(fnet_nd6_ns_header_t)) ) goto DROP;
	
	/*
	 * Validation RFC4861 (7.1.1). All included options have a length that
	 * is greater than zero.
	 */
	if( netnd6_check_options(pkt) ) goto DROP;
	
	
	while(NETPKT_LENGTH(pkt)){
		if( netpkt_pullup(pkt,sizeof(fnet_nd6_option_header_t)) ) goto DROP;
		
		nd_option = netpkt_data(pkt);
		
		size = nd_option->length<<3;
		
		if( (nd_option->type == FNET_ND6_OPTION_SOURCE_LLA) ) {
			if(size >= (sizeof(fnet_nd6_option_header_t) + NETIF_HWADDR_SIZE(nif) ) ){
				if( netpkt_pullup(pkt,size) ) goto DROP;
				netnd6_nsol_handle_lla(nif,size,netpkt_data(pkt),src_ip);
			}
		}
		/* else, silently ignore any options they do not recognize
		 * and continue processing the message.
		 */
		if( netpkt_pullfront(pkt,size) ) goto DROP;
	}
	
	/*
	 * Get Target Address Info, according tp Target Address of NS message.
         */
	target_nifaddr = netipv6_get_address_info(nif,&target_addr);
	
        /*
	 * Check if we are the target.
         */
	if( target_nifaddr ){
		/* Duplicate Address Detection (DAD) sends Neighbor Solicitation
		 * messages with an unspecified source address targeting its own
		 * "tentative" address.
		 */
		if(IP6_ADDR_IS_UNSPECIFIED(*src_ip)){
			/* === Duplicate Address Detection ===*/
			
			/*
			 * IP Destination address must be our solitation address.
			 */
			if( netipv6_addr_is_own_ip6_solicited_multicast(nif,dst_ip) )
			{
				if(target_nifaddr->state != FNET_NETIF_IP6_ADDR_STATE_TENTATIVE)
				{
					/* If the source of the solicitation is the unspecified address, the
					 * node MUST set the Solicited flag to zero and multicast the
					 * advertisement to the all-nodes address.
					 */
					
					netnd6_neighbor_advertisement_send(nif, &(target_nifaddr->address), &ip_allnodes_addr, FNET_ND6_NA_FLAG_OVERRIDE);
				}
				else
				{
					//fnet_nd6_dad_failed(netif , target_if_addr_info); /* => DAD is failed. */
					/*
					 * MUST be silently discarded, if the Target Address is a "tentative"
					 * address on which Duplicate Address Detection is being performed [ADDRCONF].
					 */
				}
			}
			/*
			 * else DROP IT.
			 */
		}
		else if(
			/* === Address Resolution === */
			(netipv6_addr_is_own_ip6_solicited_multicast(nif,dst_ip))||
			/* === Neighbor Unreachability Detection === */
			((IP6ADDR_EQ(target_addr, *dst_ip)))
		) {
			/*
			 * Sends a Neighbor Advertisement response.
			 */
			netnd6_neighbor_advertisement_send(nif, &(target_nifaddr->address), src_ip, FNET_ND6_NA_FLAG_SOLICITED | FNET_ND6_NA_FLAG_OVERRIDE);
		}
		else
		{} /* else: Bad packet.*/
	}
	
DROP:
	netpkt_free(pkt);
}

static void netnd6_nsol_handle_lla(
	netif_t *nif,
	uint32_t size,
	fnet_nd6_option_lla_header_t  *nd_option_slla,
	ipv6_addr_t *src_ip
){
	fnet_nd6_neighbor_entry_t  *neighbor_cache_entry;
	netpkt_t                   *pkts    = 0; /* Send-Chain. */
	ipv6_addr_t                queue_addr;
	hwaddr_t                   slla_addr = nif->device_addr;
	
	netif_hwaddr_load(&slla_addr,nd_option_slla->addr);
	
	/*
	 * Validation RFC4861 (7.1.1): If the IP source address is the
	 * unspecified address, there is no source link-layer address option in the message.
	 * NOTE: RFC4861 (7):Duplicate Address Detection sends Neighbor Solicitation
	 * messages with an unspecified source address targeting its own
	 * "tentative" address.
	 */
	if( IP6_ADDR_IS_UNSPECIFIED(*src_ip) ){
		/*
		 * If the Source Address is the unspecified address, the node MUST NOT
                 * create or update the Neighbor Cache entry.
                 */
		return;
	}
	
	/*
	 * RFC 48617.2.3: the recipient
	 * SHOULD create or update the Neighbor Cache entry for the IP Source
	 * Address of the solicitation.
	 */
	
	net_mutex_lock(nif->nd6->nd6_lock);
	
	neighbor_cache_entry = netnd6_neighbor_cache_get(nif, src_ip);
	
	if(! neighbor_cache_entry )
	{
		/*
		 * If an entry does not already exist, the
		 * node SHOULD create a new one and set its reachability state to STALE
		 * as specified in Section 7.3.3.
		 * If a Neighbor Cache entry is created, the IsRouter flag SHOULD be set
		 * to FALSE. This will be the case even if the Neighbor Solicitation is
		 * sent by a router since the Neighbor Solicitation messages do not
		 * contain an indication of whether or not the sender is a router. In
		 * the event that the sender is a router, subsequent Neighbor
		 * Advertisement or Router Advertisement messages will set the correct
		 * IsRouter value.
		 */
		netnd6_neighbor_cache_add2(nif,src_ip, &slla_addr, FNET_ND6_NEIGHBOR_STATE_STALE);
	}
	else
	{
		/*
		 * If an entry already exists, and the
		 * cached link-layer address differs from the one in the received Source
		 * Link-Layer option, the cached address should be replaced by the
		 * received address, and the entry's reachability state MUST be set to
		 * STALE.
		 * If a Neighbor Cache entry already exists, its
		 * IsRouter flag MUST NOT be modified.
		 */
		if(  !netif_hwaddr_eq( &(slla_addr), &(neighbor_cache_entry->ll_addr2) )  )
		{
			neighbor_cache_entry->ll_addr2 = slla_addr;
			neighbor_cache_entry->state    = FNET_ND6_NEIGHBOR_STATE_STALE;
		}
		else{
			/* RFC4861: Appendix C
			 */ /*TBD ??*/
			if(neighbor_cache_entry->state == FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
			{
				neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
			}
		}
		
		/*
		 * - It sends any packets queued for the neighbor awaiting address
		 *   resolution.
		 */
		pkts = neighbor_cache_entry->waiting_pkts;
		neighbor_cache_entry->waiting_pkts = 0;
		queue_addr = neighbor_cache_entry->ip_addr;
	}
	net_mutex_unlock(nif->nd6->nd6_lock);
	
	if(pkts) {
		nif->netif_class->ifapi_send_l3_ipv6_all(nif,pkts,(void*)&queue_addr);
	}
}

