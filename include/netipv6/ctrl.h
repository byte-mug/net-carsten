/*
 *   Copyright 2016 Simon Schmidt
 *   Copyright 2011-2016 by Andrey Butok. FNET Community.
 *   Copyright 2008-2010 by Andrey Butok. Freescale Semiconductor, Inc.
 *   Copyright 2003 by Andrey Butok. Motorola SPS.
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
#include <netstd/time.h>
#include <netipv6/if.h>

/************************************************************************
* DESCRIPTION: This function binds the IPv6 address to a hardware interface.
*************************************************************************/

int netipv6_set_ip6_addr_autoconf(netif_t *netif, ipv6_addr_t *ip_addr);

void netipv6_get_solicited_multicast_addr(const ipv6_addr_t *ip_addr, ipv6_addr_t *solicited_multicast_addr);

int netipv6_bind_addr_prv(
	netif_t *nif,
	const ipv6_addr_t *addr,
	fnet_netif_ip_addr_type_t addr_type,
	net_time_t lifetime /*in seconds*/,
	size_t prefix_length /* bits */
);
int netipv6_unbind_addr_prv (netif_t *nif, netipv6_if_addr_t *if_addr);

#endif

