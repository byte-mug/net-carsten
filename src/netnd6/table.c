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

fnet_nd6_neighbor_entry_t* netnd6_neighbor_cache_add(netif_t *nif, ipv6_addr_t *src_ip, mac_addr_t *ll_addr, fnet_nd6_neighbor_state_t state){
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
	if( ll_addr )
		entry->ll_addr = *ll_addr;
	
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

fnet_nd6_prefix_entry_t*   netnd6_prefix_list_add(netif_t *nif, const ipv6_addr_t *prefix, uint32_t prefix_length, net_time_t lifetime){
	netnd6_if_t                 *nd6_if;
	int                         i;
	fnet_nd6_prefix_entry_t   *entry = 0;

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

