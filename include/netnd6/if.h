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


#ifndef _NETND6_IF_H_
#define _NETND6_IF_H_

#include <netpkt/pkt.h>
#include <netstd/stdint.h>
#include <netipv6/ipv6.h>
#include <netif/mac.h>
#include <netstd/time.h>
#include <netstd/mutex.h>

#define FNET_ND6_NEIGHBOR_CACHE_SIZE         20
#define FNET_ND6_PREFIX_LIST_SIZE            8
#define FNET_ND6_REDIRECT_TABLE_SIZE         8
#define FNET_ND6_RDNSS_LIST_SIZE             8

/**************************************************************
* RFC4861. 10. Protocol Constants.
***************************************************************
*    Host constants:
*        MAX_RTR_SOLICITATION_DELAY 1 second
*        RTR_SOLICITATION_INTERVAL 4 seconds
*        MAX_RTR_SOLICITATIONS 3 transmissions
*    Node constants:
*        MAX_MULTICAST_SOLICIT 3 transmissions
*        MAX_UNICAST_SOLICIT 3 transmissions
*        MAX_ANYCAST_DELAY_TIME 1 second
*        MAX_NEIGHBOR_ADVERTISEMENT 3 transmissions
*        REACHABLE_TIME 30,000 milliseconds
*        RETRANS_TIMER 1,000 milliseconds
*        DELAY_FIRST_PROBE_TIME 5 seconds
*        MIN_RANDOM_FACTOR .5
*        MAX_RANDOM_FACTOR 1.5
***************************************************************/

/* If a host sends MAX_RTR_SOLICITATIONS solicitations, and receives no
 * Router Advertisements after having waited MAX_RTR_SOLICITATION_DELAY
 * seconds after sending the last solicitation, the host concludes that
 * there are no routers on the link for the purpose of [ADDRCONF].
 * However, the host continues to receive and process Router
 * Advertisements messages in the event that routers appear on the link.
 *
 * A host SHOULD transmit up to MAX_RTR_SOLICITATIONS Router
 * Solicitation messages, each separated by at least
 * RTR_SOLICITATION_INTERVAL seconds.
 */
#define FNET_ND6_MAX_RTR_SOLICITATIONS       (3U)        /* transmissions */
#define FNET_ND6_MAX_RTR_SOLICITATION_DELAY  (1000U)     /* ms */
#define FNET_ND6_RTR_SOLICITATION_INTERVAL   (4000U)     /* ms */

/* If no Neighbor Advertisement is received after MAX_MULTICAST_SOLICIT
* solicitations, address resolution has failed. The sender MUST return
* ICMP destination unreachable indications with code 3 (Address
* Unreachable) for each packet queued awaiting address resolution.
*/
#define FNET_ND6_MAX_MULTICAST_SOLICIT       (3U)        /* transmissions */

/*
 * Default value of the time between retransmissions of Neighbor
 * Solicitation messages to a neighbor when
 * resolving the address or when probing the
 * reachability of a neighbor. Also used during Duplicate
 * Address Detection (RFC4862).
 */
#define FNET_ND6_RETRANS_TIMER               (1000U)     /* ms */

/*
 * Default value of the time a neighbor is considered reachable after
 * receiving a reachability confirmation.
 *
 * This value should be a uniformly distributed
 * random value between MIN_RANDOM_FACTOR and
 * MAX_RANDOM_FACTOR times BaseReachableTime
 * milliseconds. A new random value should be
 * calculated when BaseReachableTime changes (due to
 * Router Advertisements) or at least every few
 * hours even if
 */
#define FNET_ND6_REACHABLE_TIME              (30000U)    /* ms */

/*
 * If no reachability confirmation is received
 * within DELAY_FIRST_PROBE_TIME seconds of entering the
 * DELAY state, send a Neighbor Solicitation and change
 * the state to PROBE.
 */
#define FNET_ND6_DELAY_FIRST_PROBE_TIME      (5000U)     /*ms*/

/*
 * If no response is
 * received after waiting RetransTimer milliseconds after sending the
 * MAX_UNICAST_SOLICIT solicitations, retransmissions cease and the
 * entry SHOULD be deleted.
 */
#define FNET_ND6_MAX_UNICAST_SOLICIT         (3U)        /*times*/

/*
 * ND6 general timer resolution.
 */
#define FNET_ND6_TIMER_PERIOD                (100U)      /* ms */

#define FNET_ND6_PREFIX_LENGTH_DEFAULT       (64U)            /* Default prefix length, in bits.*/
#define FNET_ND6_PREFIX_LIFETIME_INFINITE    (0xFFFFFFFFU)    /* A lifetime value of all one bits (0xffffffff) represents infinity. */
#define FNET_ND6_RDNSS_LIFETIME_INFINITE     (0xFFFFFFFFU)    /* A lifetime value of all one bits (0xffffffff) represents infinity. */


/***********************************************************************
* Prefix state.
***********************************************************************/
typedef enum fnet_nd6_prefix_state
{
    FNET_ND6_PREFIX_STATE_NOTUSED = 0,      /* The entry is not used - free.*/
    FNET_ND6_PREFIX_STATE_USED = 1          /* The entry is used.*/
} fnet_nd6_prefix_state_t;


/***********************************************************************
* Prefix List entry, based on RFC4861.
* Prefix List entries are created from information received in Router
* Advertisements.
***********************************************************************/
typedef struct
{
	ipv6_addr_t        prefix;         /* Prefix of an IPv6 address. */
	size_t             prefix_length;  /* Prefix length (in bits). The number of leading bits
	                                    * in the Prefix that are valid. */          
	net_time_t         lifetime;       /* Valid Lifetime
	                                    * 32-bit unsigned integer. The length of time in
                                            * seconds (relative to the time the packet is sent)
                                            * that the prefix is valid for the purpose of on-link
                                            * determination. A value of all one bits
                                            * (0xffffffff) represents infinity. The Valid
                                            * Lifetime is also used by [ADDRCONF].*/
	net_time_t         creation_time;  /* Time of entry creation, in seconds.*/
	unsigned           used  : 1;      /* Prefix state.*/
} fnet_nd6_prefix_entry_t;

/**************************************************************
* Neighbor's reachability states, based on RFC4861.
**************************************************************/
typedef enum fnet_nd6_neighbor_state
{
    FNET_ND6_NEIGHBOR_STATE_NOTUSED = 0,    /* The entry is not used - free.*/
    FNET_ND6_NEIGHBOR_STATE_INCOMPLETE = 1, /* Address resolution is in progress and the link-layer
                                             * address of the neighbor has not yet been determined.*/
    FNET_ND6_NEIGHBOR_STATE_REACHABLE = 2,  /* Roughly speaking, the neighbor is known to have been
                                             * reachable recently (within tens of seconds ago).*/
    FNET_ND6_NEIGHBOR_STATE_STALE = 3,      /* The neighbor is no longer known to be reachable but
                                             * until traffic is sent to the neighbor, no attempt
                                             * should be made to verify its reachability.*/
    FNET_ND6_NEIGHBOR_STATE_DELAY = 4,      /* The neighbor is no longer known to be reachable, and
                                             * traffic has recently been sent to the neighbor.
                                             * Rather than probe the neighbor immediately, however,
                                             * delay sending probes for a short while in order to
                                             * give upper-layer protocols a chance to provide
                                             * reachability confirmation.*/
    FNET_ND6_NEIGHBOR_STATE_PROBE = 5       /* The neighbor is no longer known to be reachable, and
                                             * unicast Neighbor Solicitation probes are being sent to
                                             * verify reachability.*/
} fnet_nd6_neighbor_state_t;


/***********************************************************************
* Neighbor Cache entry, based on RFC4861.
***********************************************************************/
typedef struct fnet_nd6_neighbor_entry
{
	ipv6_addr_t                 ip_addr;        /* Neighbor's on-link unicast IP address. */
	mac_addr_t                  ll_addr;        /* Its link-layer address. Actual size is defiined by fnet_netif_api_t->netif_hw_addr_size. */
	net_time_t                  state_time;     /* Time of last state event.*/
	netpkt_t                    *waiting_pkts;  /* Pointer to any queued packetwaiting for address resolution to complete.*/
	/* RFC 4861 7.2.2: While waiting for address resolution to complete, the sender MUST,
	 * for each neighbor, retain a small queue of packets waiting for
	 * address resolution to complete. The queue MUST hold at least one
	 * packet, and MAY contain more.
	 * When a queue  overflows, the new arrival SHOULD replace the oldest entry.*/
	uint32_t                    solicitation_send_counter;  /* Counter - how many soicitations where sent.*/
	ipv6_addr_t                 solicitation_src_ip_addr;   /* IP address used during AR solicitation messages. */
	net_time_t                  creation_time;              /* Time of entry creation, in seconds.*/
	
	/* Default Router list entry info.*/
	net_time_t                  router_lifetime;    /* The lifetime associated
	                                                 * with the default router in units of seconds. The
	                                                 * field can contain values up to 65535 and receivers
	                                                 * should handle any value, while the sending rules in
	                                                 * Section 6 limit the lifetime to 9000 seconds. A
	                                                 * Lifetime of 0 indicates that the router is not a
	                                                 * default router and SHOULD NOT appear on the default router list.
	                                                 * It is used only if "is_router" is 1.*/
	unsigned                    is_router  : 1;     /* A flag indicating whether the neighbor is a router or a host.*/
	
	fnet_nd6_neighbor_state_t   state : 3;          /* Neighbor's reachability state.*/
} fnet_nd6_neighbor_entry_t;

/***********************************************************************
* Redirect Table entry.
***********************************************************************/
typedef struct
{
    ipv6_addr_t         destination_addr;   /* Destination Address. The IP address of the destination that is
                                             * redirected to the target. */
    ipv6_addr_t         target_addr;        /* Target Address. An IP address that is a better first hop to use for
                                             * the ICMP Destination Address. When the target is
                                             * the actual endpoint of communication, i.e., the
                                             * destination is a neighbor, the Target Address field
                                             * MUST contain the same value as the ICMP Destination
                                             * Address field. Otherwise, the target is a better
                                             * first-hop router and the Target Address MUST be the
                                             * router's link-local address so that hosts can
                                             * uniquely identify routers. */
    net_time_t          creation_time;      /* Time of entry creation.*/
} fnet_nd6_redirect_entry_t;

/***********************************************************************
* Recursive DNS Server List entry, based on RFC6106.
***********************************************************************/
typedef struct
{
	ipv6_addr_t               rdnss_addr;         /* IPv6 address of the Recursive
	                                               * DNS Server, which is available for recursive DNS resolution
	                                               * service in the network advertising the RDNSS option. */
	net_time_t                creation_time;      /* Time of entry creation, in seconds.*/
	net_time_t                lifetime;           /* The maximum time, in
	                                               * seconds (relative to the time the packet is sent),
	                                               * over which this DNSSL domain name MAY be used for
	                                               * name resolution.
	                                               * A value of all one bits (0xffffffff) represents
	                                               * infinity.  A value of zero means that the DNSSL
	                                               * domain name MUST no longer be used.*/
} fnet_nd6_rdnss_entry_t;




typedef struct netnd6_if {
	net_mutex_t                nd6_lock;
	/*************************************************************
	* Neighbor Cache.
	* RFC4861 5.1: A set of entries about individual neighbors to
	* which traffic has been sent recently.
	**************************************************************/
	/*************************************************************
	* Combined with Default Router List.
	* RFC4861 5.1: A list of routers to which packets may be sent..
	**************************************************************/
	fnet_nd6_neighbor_entry_t  neighbor_cache[FNET_ND6_NEIGHBOR_CACHE_SIZE];
	
	/*************************************************************
	* Prefix List.
	* RFC4861 5.1: A list of the prefixes that define a set of
	* addresses that are on-link.
	**************************************************************/
	fnet_nd6_prefix_entry_t     prefix_list[FNET_ND6_PREFIX_LIST_SIZE];

	/* Redirect Table. Used only when target address != destination address. */
	fnet_nd6_redirect_entry_t   redirect_table[FNET_ND6_REDIRECT_TABLE_SIZE];

	fnet_nd6_rdnss_entry_t      rdnss_list[FNET_ND6_RDNSS_LIST_SIZE];
	
} netnd6_if_t;

#endif

