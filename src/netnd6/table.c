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

#include <netnd6/table.h>
#include <netstd/mem.h>
#include <netipv6/defs.h>
#include <netipv6/ctrl.h>
#include <netipv6/check.h>
#include <netnd6/send.h>

fnet_nd6_neighbor_entry_t* netnd6_neighbor_cache_get(netif_t *nif, ipv6_addr_t *src_ip){
	netnd6_if_t                 *nd6_if;
	int                         i;
	
	nd6_if = nif->nd6;
	
	if (! nd6_if) return 0;
	
	/* Find the entry in the cache. */
	for(i = 0u; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
	{
		if( (nd6_if->neighbor_cache[i].state != FNET_ND6_NEIGHBOR_STATE_NOTUSED) &&
			IP6ADDR_EQ(nd6_if->neighbor_cache[i].ip_addr, *src_ip))
		{
			return &(nd6_if->neighbor_cache[i]);
			break;
		}
	}
	return 0;
}

fnet_nd6_neighbor_entry_t* netnd6_neighbor_cache_add2(netif_t *nif, ipv6_addr_t *src_ip, hwaddr_t *ll_addr2, fnet_nd6_neighbor_state_t state){
	netnd6_if_t                 *nd6_if;
	int                         i;
	fnet_nd6_neighbor_entry_t   *entry = 0;
	
	nd6_if = nif->nd6;
	
	if (! nd6_if) return 0;

	/* Find not used entry.*/
	for(i = 0u; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
	{
		if( (nd6_if->neighbor_cache[i].state == FNET_ND6_NEIGHBOR_STATE_NOTUSED))
		{
			entry = &nd6_if->neighbor_cache[i];
			break;
		}
	}
	
	/* If no free entry is found.*/
	if(! entry )
	{
		entry = &nd6_if->neighbor_cache[0];
		/* Try to find the oldest entry.*/
		for(i = 0u; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
		{
			if(nd6_if->neighbor_cache[i].creation_time < entry->creation_time)
			{
				entry = &nd6_if->neighbor_cache[i];
			}
		}
	}
	
	/* Fill the informationn.*/
	
	/* Clear entry structure.*/
	net_bzero(entry,sizeof(fnet_nd6_neighbor_entry_t) );
	entry->ip_addr = *src_ip;
	if( ll_addr2 )
		entry->ll_addr2 = *ll_addr2;
	
	entry->creation_time = net_timer_seconds();
	entry->is_router = 0;
	entry->router_lifetime = 0u;
	entry->state = state;

        /* TBD Init timers reachable; last send*/
	return entry;
}

fnet_nd6_prefix_entry_t*   netnd6_prefix_list_get(netif_t *nif, const ipv6_addr_t *prefix){
	netnd6_if_t                 *nd6_if;
	int                         i;

	nd6_if = nif->nd6;
	
	if (! nd6_if) return 0;
	
	/* Find the entry in the list. */
	for(i = 0u; i < FNET_ND6_PREFIX_LIST_SIZE; i++)
	{
		if( (nd6_if->prefix_list[i].used) && IP6ADDR_EQ(nd6_if->prefix_list[i].prefix, *prefix))
		{
			return &nd6_if->prefix_list[i];
		}
	}
	return 0;
}

fnet_nd6_prefix_entry_t*   netnd6_prefix_list_lookup(netif_t *nif, const ipv6_addr_t *addr){
	netnd6_if_t                 *nd6_if;
	int                         i;

	nd6_if = nif->nd6;
	
	if (! nd6_if) return 0;
	
	/* Find the entry in the list. */
	for(i = 0u; i < FNET_ND6_PREFIX_LIST_SIZE; i++)
	{
		if(! (nd6_if->prefix_list[i].used) ) continue;
		
		if(netipv6_addr_pefix_cmp(
				&(nd6_if->prefix_list[i].prefix),addr,
				nd6_if->prefix_list[i].prefix_length
			))
		{
			return &nd6_if->prefix_list[i];
		}
	}
	return 0;
}

fnet_nd6_prefix_entry_t*   netnd6_prefix_list_add(netif_t *nif, const ipv6_addr_t *prefix, uint32_t prefix_length, net_time_t lifetime){
	netnd6_if_t                 *nd6_if;
	int                         i;
	fnet_nd6_prefix_entry_t     *entry = 0;

	nd6_if = nif->nd6;
	
	if (! nd6_if) return 0;
	
	/* Find an unused entry in the cache. Skip 1st Link_locak prefix. */
	for(i = 1u; i < FNET_ND6_PREFIX_LIST_SIZE; i++)
	{
		if(!nd6_if->prefix_list[i].used)
		{
			entry = &nd6_if->prefix_list[i];
			break;
		}
	}
	
	/* If no free entry is found. */
	if(! entry )
	{
		entry = &nd6_if->prefix_list[0];
		/* Try to find the oldest entry. */
		for(i = 1u; i < FNET_ND6_PREFIX_LIST_SIZE; i++)
		{
			if(nd6_if->prefix_list[i].creation_time < entry->creation_time)
			{
				entry = &nd6_if->prefix_list[i];
			}
		}
	}
	
	/* Fill the informationn. */
	entry->prefix = *prefix;
	entry->prefix_length = prefix_length;
	entry->lifetime = lifetime;
	entry->creation_time = net_timer_seconds();
	entry->used = 1;
	
	return entry;
}

fnet_nd6_redirect_entry_t* netnd6_redirect_table_add(netif_t *nif, const ipv6_addr_t *destination_addr, const ipv6_addr_t *target_addr){
	netnd6_if_t                 *nd6_if;
	int                         i;
	fnet_nd6_redirect_entry_t   *entry = 0;
	
	nd6_if = nif->nd6;
	
	if (! nd6_if) return 0;
	
	/* Check if the destination address exists.*/
	for(i = 0u; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
	{
		if(IP6ADDR_EQ(nd6_if->redirect_table[i].destination_addr, *destination_addr))
		{
			/* Found existing destination address.*/
			entry = &nd6_if->redirect_table[i];
			break;
		}
	}
	
	if(! entry ){
		/* Find an unused entry in the table. */
		for(i = 0u; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++){
			if(IP6_ADDR_IS_UNSPECIFIED(nd6_if->redirect_table[i].destination_addr))
			{
				entry = &nd6_if->redirect_table[i];
				break;
			}
		}
	}
	
	if(! entry ){
		entry = &nd6_if->redirect_table[0];
		/* Try to find the oldest entry. */
		for(i = 1u; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++){
			if(nd6_if->redirect_table[i].creation_time < entry->creation_time)
				entry = &nd6_if->redirect_table[i];
		}
	}
	
	/* Fill the informationn. */
	entry->destination_addr = *destination_addr;
	entry->target_addr      = *target_addr;
        entry->creation_time    = net_timer_seconds();
	
	return entry;
}

void netnd6_redirect_table_get(netif_t *nif, ipv6_addr_t *destination_addr){
	netnd6_if_t                 *nd6_if;
	int                         i;
	fnet_nd6_redirect_entry_t   *entry = 0;
	
	nd6_if = nif->nd6;
	
	if (! nd6_if) return;
	
	/* Check if the destination address exists.*/
	for(i = 0u; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
	{
		if(IP6ADDR_EQ(nd6_if->redirect_table[i].destination_addr, *destination_addr))
		{
			/* Found existing destination address.*/
			entry = &nd6_if->redirect_table[i];
			*destination_addr = nd6_if->redirect_table[i].target_addr;
			return;
		}
	}
}

void netnd6_dad_failed(
	netif_t *nif,
	netipv6_if_addr_t *addr_info
){
	ipv6_addr_t             if_ip6_address;
	/* RFC 4862: */
	/* 5.4.5. When Duplicate Address Detection Fails */
	/* Just remove address, or TBD mark it as dupicate.*/
	//fnet_netif_unbind_ip6_addr_prv ( nif, addr_info);
	netipv6_unbind_addr_prv( nif, addr_info);
	
	/* If the address is a link-local address formed from an interface
	 * identifier based on the hardware address, which is supposed to be
	 * uniquely assigned (e.g., EUI-64 for an Ethernet interface), IP
	 * operation on the interface SHOULD be disabled.
	 * In this case, the IP address duplication probably means duplicate
	 * hardware addresses are in use, and trying to recover from it by
	 * configuring another IP address will not result in a usable network.*/
	net_bzero(&if_ip6_address, sizeof(ipv6_addr_t));
	if_ip6_address.addr[0] = 0xFEu;
	if_ip6_address.addr[1] = 0x80u;
	netipv6_set_ip6_addr_autoconf(nif,&if_ip6_address);
	
	if(IP6ADDR_EQ(if_ip6_address, addr_info->address))
	{
		nif->ipv6->disabled = 1;
	}
}

void netnd6_dad_start(
	netif_t *nif,
	netipv6_if_addr_t *addr_info
){
	if(!addr_info) return;
	if(addr_info->state != FNET_NETIF_IP6_ADDR_STATE_TENTATIVE) return;
	/*
	 * To check an address, a node sends DAD Neighbor
	 * Solicitations, each separated by 1 second(TBD)..
	 */
	addr_info->dad_transmit_counter = 1;
	addr_info->state_time = net_timer_ms();  /* Save state time.*/
	netnd6_neighbor_solicitation_send(nif, 0 /* NULL for, DAD */, 0 /*set for NUD,  NULL for DAD & AR */, &(addr_info->address));
}


