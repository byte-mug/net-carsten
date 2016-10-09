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

void netnd6_neighbor_advertisement_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
	uint32_t                   size;
	fnet_nd6_option_header_t   *nd_option;
	fnet_nd6_na_header_t       *hdr;
	ipv6_addr_t                target_addr;
	netipv6_if_addr_t          *target_nifaddr;
	fnet_nd6_neighbor_entry_t  *neighbor_cache_entry;
	char                       is_solicited;
	char                       is_router;
	char                       is_override;
	char                       is_ll_addr_changed;
	char                       nd_option_tlla;
	fnet_nd6_option_lla_header_t *hdr_tlla;
	netpkt_t                   *pkts    = 0; /* Send-Chain. */
	ipv6_addr_t                queue_addr;
	
	hwaddr_t                   tlla_addr2 = nif->device_addr;
	
	/*
	 * Validation RFC4861 (7.1.2).
	 * The IP Hop Limit field has a value of 255, i.e., the packet
         * could not possibly have been forwarded by a router.
	 */
	if( netnd6_check_hop_limit(pkt,255) ) goto DROP;
	
	/*
	 * The header must reside in contiguous area of memory.
	 *
	 * Validation RFC4861 (7.1.2). ICMP length (derived from the IP length) is 24 or more octets.
	 */
	if( netpkt_pullup(pkt,sizeof(fnet_nd6_na_header_t)) ) goto DROP;
	
	hdr = netpkt_data(pkt);
	
	is_solicited = (hdr->flag & FNET_ND6_NA_FLAG_SOLICITED)? 1:0;
	
	
	/* Validation RFC4861 (7.1.2). */
	if(
		(hdr->icmp6_header.code != 0u) ||                 /* ICMP Code is 0.*/
		IP6_ADDR_IS_MULTICAST(hdr->target_addr) ||        /* Target Address is not a multicast address. */
		(is_solicited && IP6_ADDR_IS_MULTICAST(*dst_ip) ) /* If the IP Destination Address is a multicast
		                                                   * address the Solicited flag is zero.*/
	) goto DROP;
	
	target_addr = hdr->target_addr;
	
	/* Extract flag values.*/
        is_router = (hdr->flag & FNET_ND6_NA_FLAG_ROUTER) ? 1:0;
        is_override = (hdr->flag & FNET_ND6_NA_FLAG_OVERRIDE) ? 1:0;
	
	if( netpkt_pullfront(pkt,sizeof(fnet_nd6_na_header_t)) ) goto DROP;
	
	/*
	 * Validation RFC4861 (7.1.2). All included options have a length that
	 * is greater than zero.
	 */
	if( netnd6_check_options(pkt) ) goto DROP;
	/*
	 * A Neighbor Advertisements that passes the validity checks is called a
         * "valid advertisement".
         */
	
	nd_option_tlla = 0;
	while(NETPKT_LENGTH(pkt)){
		if( netpkt_pullup(pkt,sizeof(fnet_nd6_option_header_t)) ) goto DROP;
		
		nd_option = netpkt_data(pkt);
		
		size = nd_option->length<<3;
		
		/*
		 * Handle Target link-layer address option only.
		 */
		if( (nd_option->type == FNET_ND6_OPTION_TARGET_LLA) ) {
			if(size >= (sizeof(fnet_nd6_option_header_t) + NETIF_HWADDR_SIZE(nif) ) ){
				if( netpkt_pullup(pkt,size) ) goto DROP;
				nd_option_tlla = 1;
				hdr_tlla = netpkt_data(pkt);
				netif_hwaddr_load(&tlla_addr2,hdr_tlla->addr);
			}
		}
		/* else, silently ignore any options they do not recognize
		 * and continue processing the message.
		 */
		if( netpkt_pullfront(pkt,size) ) goto DROP;
	}
	
	/*
	 * Get Target Address Info, according tp Target Address of NA message.
         */
	target_nifaddr = netipv6_get_address_info(nif,&target_addr);
	
	if( target_nifaddr )
	{
		/* Duplicated address!!!!! */
		if( target_nifaddr->state == FNET_NETIF_IP6_ADDR_STATE_TENTATIVE )
		{
			netnd6_dad_failed(nif,target_nifaddr); /* => DAD is failed. */
		}
		goto DROP;
	}
	
	/************************************************************
	 * Handle NA message.
	 ************************************************************/
	{
		net_mutex_lock(nif->nd6->nd6_lock);
		/*
		 * RFC4861 7.2.5: Neighbor Cache is searched for the target's entry.
		 */
		neighbor_cache_entry = netnd6_neighbor_cache_get(nif,&(hdr->target_addr));
		if(! neighbor_cache_entry )
		{
			/*
			 * If no entry exists, the advertisement SHOULD be silently discarded.
			 */
			goto DROP_L;
		}
		
		/* If the target's Neighbor Cache entry is in the INCOMPLETE state.*/
		if(neighbor_cache_entry->state == FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
		{
			/* If the link layer has addresses and no Target Link-Layer Address option is
			 * included, the receiving node SHOULD silently discard the received
			 * advertisement.*/
			if(!nd_option_tlla) goto DROP_L;
			
			/*
			 * Otherwise, the receiving node performs the following
			 * steps:
			 * - It records the link-layer address in the Neighbor Cache entry.
			 */
			neighbor_cache_entry->ll_addr2 = tlla_addr2;
			/*
			 * - If the advertisement's Solicited flag is set, the state of the
			 *   entry is set to REACHABLE; otherwise, it is set to STALE.
			 */
			if (is_solicited)
			{
				neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_REACHABLE;
				/* Reset Reachable Timer. */
				neighbor_cache_entry->state_time = net_timer_ms();
			}
			else
			{
				neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
			}
			
			/*
			 * - It sets the IsRouter flag in the cache entry based on the Router
			 *   flag in the received advertisement.
			 */
			neighbor_cache_entry->is_router = is_router ? 1:0;
			/* - It sends any packets queued for the neighbor awaiting address
			 *   resolution.
			 */
			pkts = neighbor_cache_entry->waiting_pkts;
			neighbor_cache_entry->waiting_pkts = 0;
			queue_addr = neighbor_cache_entry->ip_addr;
		}
		else
		{
			/* If the target's Neighbor Cache entry is in any state other than
			 * INCOMPLETE.
			 */
			if( nd_option_tlla )
			{
				/* If supplied link-layer address differs. */
				is_ll_addr_changed = netif_hwaddr_eq(&(tlla_addr2),&(neighbor_cache_entry->ll_addr2)) ? 0:1;
			}
			else
			{
				is_ll_addr_changed = 0;
			}
			
			/* I. If the Override flag is clear and the supplied link-layer address
			 *     differs from that in the cache.
			 */
			if( (!is_override) && is_ll_addr_changed)
			{
				/* a. If the state of the entry is REACHABLE, set it to STALE, but
				 *    do not update the entry in any other way.
				 */
				if(neighbor_cache_entry->state == FNET_ND6_NEIGHBOR_STATE_REACHABLE)
				{
					neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
				}
				/* b. Otherwise, the received advertisement should be ignored and
				 *    MUST NOT update the cache.
				 */
				goto DROP_L;
			}
			/* II. If the Override flag is set, or the supplied link-layer address
			 *     is the same as that in the cache, or no Target Link-Layer Address
			 *     option was supplied, the received advertisement MUST update the
			 *     Neighbor Cache entry as follows:
			 */
			else
			{
				/* - The link-layer address in the Target Link-Layer Address option
				 *   MUST be inserted in the cache (if one is supplied and differs
				 *   from the already recorded address).
				 */
				if(nd_option_tlla)
				{
					neighbor_cache_entry->ll_addr2 = tlla_addr2;
				}
				
				/* - If the Solicited flag is set, the state of the entry MUST be
				 *   set to REACHABLE.
				 */
				if(is_solicited)
				{
					neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_REACHABLE;
					/* Reset Reachable Timer.*/
					neighbor_cache_entry->state_time = net_timer_ms();
				}
				
				/* If the
				 *   address was updated with a different address, the state
				 *   MUST be set to STALE. Otherwise, the entry's state remains
				 *   unchanged.
				 */
				else if(is_ll_addr_changed)
				{
					neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
				}
				else
				{}
				/* - The IsRouter flag in the cache entry MUST be set based on the
				 *   Router flag in the received advertisement. In those cases
				 *   where the IsRouter flag changes from TRUE to FALSE as a result
				 *   of this update, the node MUST remove that router from the
				 *   Default Router List and update the Destination Cache entries
				 *   for all destinations using that neighbor as a router as
				 *   specified in Section 7.3.3.
				 */
				if((neighbor_cache_entry->is_router) && (!is_router))
				{
					/* Delete Cache entry. */
					neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_NOTUSED;
					neighbor_cache_entry->router_lifetime = 0u;
				}
			}
		}
DROP_L:
		net_mutex_unlock(nif->nd6->nd6_lock);
	}
	
DROP:
	if(pkts) {
		nif->netif_class->ifapi_send_l3_ipv6_all(nif,pkts,(void*)&queue_addr);
	}
	netpkt_free(pkt);
}


