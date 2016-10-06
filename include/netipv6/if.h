/*
 *   Copyright 2016 Simon Schmidt
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


#ifndef _NETIPV6_IF_H_
#define _NETIPV6_IF_H_

#include <netipv6/ipv6.h>

#define NETIPV6_IF_ADDR_MAX 8

typedef struct netipv6_if_addr
{
	ipv6_addr_t   address;                   /* IPv6 address.*/
	ipv6_addr_t   solicited_multicast_addr;  /* Solicited-node multicast */
	unsigned      used : 1;                  /* Is the entry in use? */

#if 0
    fnet_time_t                 creation_time;          /* Time of entry creation (in seconds).*/
    fnet_time_t                 lifetime;               /* Address lifetime (in seconds). 0xFFFFFFFF = Infinite Lifetime
                                                         * RFC4862. A link-local address has an infinite preferred and valid lifetime; it
                                                         * is never timed out.*/
    fnet_size_t                 prefix_length;          /* Prefix length (in bits). The number of leading bits
                                                         * in the Prefix that are valid. */
    fnet_index_t                dad_transmit_counter;   /* Counter used by DAD. Equals to the number
                                                         * of NS transmits till DAD is finished.*/
    fnet_time_t                 state_time;             /* Time of last state event.*/
#endif

} netipv6_if_addr_t;

typedef struct netipv6_if {
	netipv6_if_addr_t addrs[NETIPV6_IF_ADDR_MAX];
	uint8_t hop_limit;
	unsigned disabled : 1; /* < IPv6 is Disabled*/
} netipv6_if_t;

#endif

