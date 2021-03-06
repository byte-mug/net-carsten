/*
 *   Copyright 2016 Simon Schmidt
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


#ifndef _NETND6_TABLE_H_
#define _NETND6_TABLE_H_

#include <netif/if.h>
#include <netnd6/if.h>
#include <netipv6/ipv6.h>
#include <netipv6/if.h>
#include <netif/hwaddr.h>

fnet_nd6_neighbor_entry_t* netnd6_neighbor_cache_get(netif_t *nif, ipv6_addr_t *src_ip);

fnet_nd6_neighbor_entry_t* netnd6_neighbor_cache_add2(netif_t *nif, ipv6_addr_t *src_ip, hwaddr_t *ll_addr, fnet_nd6_neighbor_state_t state);

fnet_nd6_neighbor_entry_t* netnd6_get_router(netif_t *nif);

fnet_nd6_prefix_entry_t*   netnd6_prefix_list_get(netif_t *nif, const ipv6_addr_t *prefix);

/*
 * Returns the prefix-entry, that the given 'addr' parameter matches.
 */
fnet_nd6_prefix_entry_t*   netnd6_prefix_list_lookup(netif_t *nif, const ipv6_addr_t *addr);

fnet_nd6_prefix_entry_t*   netnd6_prefix_list_add(netif_t *nif, const ipv6_addr_t *prefix, uint32_t prefix_length, net_time_t lifetime);

fnet_nd6_redirect_entry_t* netnd6_redirect_table_add(netif_t *nif, const ipv6_addr_t *destination_addr, const ipv6_addr_t *target_addr);

void netnd6_redirect_table_get(netif_t *nif, ipv6_addr_t *destination_addr);

void netnd6_dad_failed(
	netif_t *nif,
	netipv6_if_addr_t *addr_info
);

void netnd6_dad_start(
	netif_t *nif,
	netipv6_if_addr_t *addr_info
);

#endif

