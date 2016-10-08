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


#ifndef _NETIPV6_IF_H_
#define _NETIPV6_IF_H_

#include <netipv6/ipv6.h>
#include <netstd/time.h>

#define NETIPV6_IF_ADDR_MAX 8



/**************************************************************************/ /*!
 * @brief Possible IPv6 address states.
 * @see fnet_netif_get_ip6_addr(), fnet_netif_ip6_addr_info
 ******************************************************************************/
typedef enum
{
    FNET_NETIF_IP6_ADDR_STATE_NOT_USED = 0,     /**< @brief Not used.*/
    FNET_NETIF_IP6_ADDR_STATE_TENTATIVE = 1,    /**< @brief Tentative address - (RFC4862) an address whose uniqueness on a link is being
                                                 * verified, prior to its assignment to an interface. A tentative
                                                 * address is not considered assigned to an interface in the usual
                                                 * sense. An interface discards received packets addressed to a
                                                 * tentative address, but accepts Neighbor Discovery packets related
                                                 * to Duplicate Address Detection for the tentative address.
                                                 */
    FNET_NETIF_IP6_ADDR_STATE_PREFERRED = 2 	/**< @brief Preferred address - (RFC4862) an address assigned to an interface whose use by
                                                 * upper-layer protocols is unrestricted. Preferred addresses may be
                                                 * used as the source (or destination) address of packets sent from
                                                 * (or to) the interface.
                                                 */
} fnet_netif_ip6_addr_state_t;

typedef struct netipv6_if_addr
{
	ipv6_addr_t   address;                   /* IPv6 address.*/
	ipv6_addr_t   solicited_multicast_addr;  /* Solicited-node multicast */

	net_time_t    creation_time;             /* Time of entry creation (in seconds).*/
	net_time_t    lifetime;                  /* Address lifetime (in seconds). 0xFFFFFFFF = Infinite Lifetime
	                                          * RFC4862. A link-local address has an infinite preferred and valid lifetime; it
	                                          * is never timed out.*/
	size_t        prefix_length;             /* Prefix length (in bits). The number of leading bits
	                                          * in the Prefix that are valid. */
	uint32_t      dad_transmit_counter;      /* Counter used by DAD. Equals to the number
	                                          * of NS transmits till DAD is finished.*/
	net_time_t    state_time;                /* Time of last state event.*/
	unsigned      state : 2;                 /* Address current state. (fnet_netif_ip6_addr_state_t)*/
	unsigned      used : 1;                  /* Is the entry in use? */

} netipv6_if_addr_t;

typedef struct netipv6_if {
	netipv6_if_addr_t addrs[NETIPV6_IF_ADDR_MAX];
	uint8_t           hop_limit;
	size_t            pmtu;
	unsigned          disabled : 1; /* < IPv6 is Disabled*/
	unsigned          pmtu_on : 1; /* < IPv6/ICMPv6 PMTU Enabled*/
} netipv6_if_t;

#endif

