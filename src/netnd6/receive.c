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

#include <netipv6/defs.h>
#include <netipv6/check.h>
#include <netipv6/if.h>

#include <netif/mac.h>
#include <netif/ifapi.h>

#define AS_MAC(a) *((mac_addr_t*)(a))

static void netnd6_nsol_handle_lla(
	netif_t *nif,
	uint32_t size,
	fnet_nd6_option_lla_header_t  *nd_option_slla,
	ipv6_addr_t *src_ip
);

void netnd6_neighbor_solicitation_reveive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
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
	
	while(NETPKT_LENGTH(pkt)){
		if( netpkt_pullup(pkt,sizeof(fnet_nd6_option_header_t)) ) goto DROP;
		
		nd_option = netpkt_data(pkt);
		
		/*
		 * Validation RFC4861 (7.1.1). All included options have a length that
		 * is greater than zero.
		 */
		if(nd_option->length == 0u) goto DROP;
		
		size = nd_option->length<<3;
		
		/*
		 * Handle Source link-layer address option only.
		 */
		if( (nd_option->type == FNET_ND6_OPTION_SOURCE_LLA) && (size > sizeof(fnet_nd6_option_lla_header_t)) ) {
			if( netpkt_pullup(pkt,size) ) goto DROP;
			netnd6_nsol_handle_lla(nif,size,netpkt_data(pkt),src_ip);
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
	mac_addr_t                 mac_addr = AS_MAC(nd_option_slla->addr);
	
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
		netnd6_neighbor_cache_add(nif,src_ip, (mac_addr_t*)nd_option_slla->addr, FNET_ND6_NEIGHBOR_STATE_STALE);
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
		if(  !NETIF_MACADDR_EQ( mac_addr, neighbor_cache_entry->ll_addr)  )
		{
			neighbor_cache_entry->ll_addr = mac_addr;
			neighbor_cache_entry->state   = FNET_ND6_NEIGHBOR_STATE_STALE;
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
	}
	net_mutex_unlock(nif->nd6->nd6_lock);
	
	if(pkts) {
		nif->netif_class->ifapi_send_l2_all(nif,pkts,&mac_addr);
	}
}

void netnd6_neighbor_advertisement_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
	uint32_t                   size;
	fnet_nd6_option_header_t   *nd_option;
	fnet_nd6_na_header_t       *hdr;
	ipv6_addr_t                target_addr;
	mac_addr_t                 tlla_addr;
	netipv6_if_addr_t          *target_nifaddr;
	fnet_nd6_neighbor_entry_t  *neighbor_cache_entry;
	char                       is_solicited;
	char                       is_router;
	char                       is_override;
	char                       is_ll_addr_changed;
	char                       nd_option_tlla;
	fnet_nd6_option_lla_header_t *hdr_tlla;
	netpkt_t                   *pkts    = 0; /* Send-Chain. */
	
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
	
	if( netpkt_pullfront(pkt,sizeof(fnet_nd6_na_header_t)) ) goto DROP;
	
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
			tlla_addr = AS_MAC(hdr_tlla->addr);
			//netnd6_nsol_handle_lla(nif,size,netpkt_data(pkt),src_ip);
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
			//fnet_nd6_dad_failed(netif, target_if_addr_info); /* => DAD is failed. */
		}
		goto DROP;
	}
	
	/*
	 * A Neighbor Advertisements that passes the validity checks is called a
         * "valid advertisement".
         */

        /* Extract flag values.*/
        is_router = (hdr->flag & FNET_ND6_NA_FLAG_ROUTER) ? 1:0;
        is_override = (hdr->flag & FNET_ND6_NA_FLAG_OVERRIDE) ? 1:0;
	
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
			neighbor_cache_entry->ll_addr = tlla_addr;
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
		}
		else
		{
			/* If the target's Neighbor Cache entry is in any state other than
			 * INCOMPLETE.
			 */
			if( nd_option_tlla )
			{
				/* If supplied link-layer address differs. */
				is_ll_addr_changed = NETIF_MACADDR_EQ(tlla_addr,neighbor_cache_entry->ll_addr) ? 0:1;
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
				goto DROP;
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
					neighbor_cache_entry->ll_addr = tlla_addr;
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
		nif->netif_class->ifapi_send_l2_all(nif,pkts,&tlla_addr);
	}
	netpkt_free(pkt);
}

void netnd6_router_advertisement_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
	netpkt_free(pkt);
}

void netnd6_redirect_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip){
	netpkt_free(pkt);
}

