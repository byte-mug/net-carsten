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


#ifndef _NETIPV6_CTRL_H_
#define _NETIPV6_CTRL_H_

#include <netif/if.h>
#include <netpkt/pkt.h>
#include <netipv6/ipv6.h>

/*
 * Join a IPv6 multicast group.
 * Return 0 on success, non-0 on error.
 */
int netipv6_multicast_join_prv(netif_t *netif, const ipv6_addr_t *ip_addr);

/*
 * Leave a IPv6 multicast group.
 * Return 0 on success, non-0 on error.
 */
int netipv6_multicast_leave_prv(netif_t *netif, const ipv6_addr_t *ip_addr);

#endif

