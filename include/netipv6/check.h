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


#ifndef _NETIPV6_CHECK_H_
#define _NETIPV6_CHECK_H_

#include <netif/if.h>
#include <netipv6/ipv6.h>

struct netipv6_if_addr;

int netipv6_deactivated(netif_t *nif);

/*
 * Returns non-0 if the address is directed at ourself.
 */
int netipv6_addr_is_self(netif_t *nif, ipv6_addr_t *addr, uint16_t pkt_flags);

/*
 * Returns non-0 if the address is our IPv6 Solicited Multicast.
 */
int netipv6_addr_is_own_ip6_solicited_multicast(netif_t *nif, ipv6_addr_t *addr);

/*
 * Selects the best source address to use with a destination address.
 * Just enough to implement Neighbor Solicitation Message.
 */
int netipv6_select_src_addr_nsol(netif_t *nif, ipv6_addr_t *src, const ipv6_addr_t *dest);

/*
 * Returns an IPv6-address entry from the netif_t object.
 */
struct netipv6_if_addr* netipv6_get_address_info(netif_t *nif, ipv6_addr_t *addr);

int netipv6_addr_pefix_cmp(const ipv6_addr_t *addr_1, const ipv6_addr_t *addr_2, size_t prefix_length);

#endif

