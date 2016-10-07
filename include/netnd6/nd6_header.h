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


#ifndef _NETND6_HEADERS_H_
#define _NETND6_HEADERS_H_

#include <neticmp6/icmp6_header.h>
#include <netipv6/ipv6.h>

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

/*********************************************************************
* Partial IP packet header for Argument validation.
**********************************************************************
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |Version| Traffic Class |           Flow Label                  |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |         Payload Length        |  Next Header  |   Hop Limit   |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   .                                                               .
*   .                                                               .
**********************************************************************/
typedef struct NETSTD_PACKED
{
    uint8_t   version__tclass      ;   /* 4-bit Internet Protocol version number = 6, 8-bit traffic class field. */
    uint8_t   tclass__flowl        ;   /* 20-bit flow label. */
    uint16_t   flowl               ;
    uint16_t   length              ;   /* Length of the IPv6
                                        * payload, i.e., the rest of the packet following
                                        * this IPv6 header, in octets. */
    uint8_t   next_header          ;   /* Identifies the type of header
                                        * immediately following the IPv6 header.  Uses the
                                        * same values as the IPv4 Protocol field [RFC-1700
                                        * et seq.].*/
    uint8_t   hop_limit            ;   /* Decremented by 1 by
                                        * each node that forwards the packet. The packet
                                        * is discarded if Hop Limit is decremented to
                                        * zero. */
} partial_ipv6_header_t;


/**********************************************************************
* Neighbor Solicitation Message Format (RFC 4861)
***********************************************************************
* Nodes send Neighbor Solicitations to request the link-layer address
* of a target node while also providing their own link-layer address to
* the target.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_icmp6_header_t icmp6_header    ;
    uint8_t             _reserved[4]    ;
    ipv6_addr_t         target_addr     ;
} fnet_nd6_ns_header_t;

/**********************************************************************
* Neighbor Advertisement Message Format (RFC 4861)
***********************************************************************
* A node sends Neighbor Advertisements in response to Neighbor
* Solicitations and sends unsolicited Neighbor Advertisements in order
* to (unreliably) propagate new information quickly.
*
*	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |R|S|O|                     Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_icmp6_header_t icmp6_header    ;
    uint8_t             flag            ;
    uint8_t             _reserved[3]    ;
    ipv6_addr_t         target_addr     ;
} fnet_nd6_na_header_t;
/* NA flags.*/
#define FNET_ND6_NA_FLAG_ROUTER      (0x80U) /* Router flag. When set, the R-bit indicates that
                                             * the sender is a router. The R-bit is used by
                                             * Neighbor Unreachability Detection to detect a
                                             * router that changes to a host.*/
#define FNET_ND6_NA_FLAG_SOLICITED   (0x40U) /* Solicited flag. When set, the S-bit indicates that
                                             * the advertisement was sent in response to a
                                             * Neighbor Solicitation from the Destination address.
                                             * The S-bit is used as a reachability confirmation
                                             * for Neighbor Unreachability Detection. It MUST NOT
                                             * be set in multicast advertisements or in
                                             * unsolicited unicast advertisements.*/
#define FNET_ND6_NA_FLAG_OVERRIDE    (0x20U) /* Override flag. When set, the O-bit indicates that
                                             * the advertisement should override an existing cache
                                             * entry and update the cached link-layer address.
                                             * When it is not set the advertisement will not
                                             * update a cached link-layer address though it will
                                             * update an existing Neighbor Cache entry for which
                                             * no link-layer address is known. It SHOULD NOT be
                                             * set in solicited advertisements for anycast
                                             * addresses and in solicited proxy advertisements.
                                             * It SHOULD be set in other solicited advertisements
                                             * and in unsolicited advertisements.*/

/**********************************************************************
* Redirect Message Format (RFC 4861)
***********************************************************************
* Routers send Redirect packets to inform a host of a better first-hop
* node on the path to a destination.
*
*	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                    Destination Address                        +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/

typedef struct NETSTD_PACKED
{
    fnet_icmp6_header_t icmp6_header        ;
    uint8_t             _reserved[4]        ;
    ipv6_addr_t         target_addr         ;
    ipv6_addr_t         destination_addr    ;
} fnet_nd6_rd_header_t;

/**********************************************************************
* Router Solicitation Message Format
***********************************************************************
* Hosts send Router Solicitations in order to prompt routers to
* generate Router Advertisements quickly.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_icmp6_header_t icmp6_header    ;
    uint8_t             _reserved[4]    ;
} fnet_nd6_rs_header_t;


/**********************************************************************
* Router Advertisement Message Format
***********************************************************************
* Routers send out Router Advertisement messages periodically, or in
* response to Router Solicitations.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                      Reachable Time                           |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                      Retrans Timer                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/

typedef struct NETSTD_PACKED
{
    fnet_icmp6_header_t icmp6_header    ;   /* ICMPv6 header.*/
    uint8_t             cur_hop_limit   ;   /* 8-bit unsigned integer. The default value that
                                             * should be placed in the Hop Count field of the IP
                                             * header for outgoing IP packets. A value of zero
                                             * means unspecified (by this router). */
    uint8_t             flag            ;   /* ND6_RS_FLAG_M and/or ND6_RS_FLAG_O flags.*/
    uint16_t            router_lifetime ;   /* 16-bit unsigned integer. The lifetime associated
                                             * with the default router in units of seconds. The
                                             * field can contain values up to 65535 and receivers
                                             * should handle any value, while the sending rules in
                                             * Section 6 limit the lifetime to 9000 seconds. A
                                             * Lifetime of 0 indicates that the router is not a
                                             * default router and SHOULD NOT appear on the default
                                             * router list. The Router Lifetime applies only to
                                             * the router's usefulness as a default router; it
                                             * does not apply to information contained in other
                                             * message fields or options. Options that need time
                                             * limits for their information include their own
                                             * lifetime fields.*/
    uint32_t            reachable_time  ;   /* 32-bit unsigned integer. The time, in
                                             * milliseconds, that a node assumes a neighbor is
                                             * reachable after having received a reachability
                                             * confirmation. Used by the Neighbor Unreachability
                                             * Detection algorithm (see Section 7.3). A value of
                                             * zero means unspecified (by this router). */
    uint32_t            retrans_timer   ;   /* 32-bit unsigned integer. The time, in
                                             * milliseconds, between retransmitted Neighbor
                                             * Solicitation messages. Used by address resolution
                                             * and the Neighbor Unreachability Detection algorithm
                                             * (see Sections 7.2 and 7.3). A value of zero means
                                             * unspecified (by this router).*/
} fnet_nd6_ra_header_t;

/* RA flags */
#define FNET_ND6_RA_FLAG_M   (0x80U) /* 1-bit "Managed address configuration" flag. When
                                     * set, it indicates that addresses are available via
                                     * Dynamic Host Configuration Protocol [DHCPv6].
                                     * If the M flag is set, the O flag is redundant and
                                     * can be ignored because DHCPv6 will return all
                                     * available configuration information.*/
#define FNET_ND6_RA_FLAG_O   (0x40U) /* 1-bit "Other configuration" flag. When set, it
                                     * indicates that other configuration information is
                                     * available via DHCPv6. Examples of such information
                                     * are DNS-related information or information on other
                                     * servers within the network.*/
/* Note: If neither M nor O flags are set, this indicates that no
 * information is available via DHCPv6.*/


/* ND option types (RFC4861). */
#define FNET_ND6_OPTION_SOURCE_LLA          (1U)     /* Source Link-layer Address.*/
#define FNET_ND6_OPTION_TARGET_LLA          (2U)     /* Target Link-layer Address.*/
#define FNET_ND6_OPTION_PREFIX              (3U)     /* Prefix Information.*/
#define FNET_ND6_OPTION_REDIRECTED_HEADER   (4U)     /* Redirected Header.*/
#define FNET_ND6_OPTION_MTU                 (5U)     /* MTU. */
#define FNET_ND6_OPTION_RDNSS               (25U)    /* RDNSS RFC6106. */

/***********************************************************************
 * ND option header
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    uint8_t type      ;   /* Identifier of the type of option.*/
    uint8_t length    ;   /* The length of the option
                           * (including the type and length fields) in units of
                           * 8 octets.  The value 0 is invalid.  Nodes MUST
                           * silently discard an ND packet that contains an
                           * option with length zero.*/
} fnet_nd6_option_header_t;


/***********************************************************************
 * Source/Target Link-layer Address option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    |       Link-Layer Address ...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_nd6_option_header_t    option_header   ;   /* Option general header.*/
    uint8_t                     addr[6]         ;   /* The length of the option. Can be more or less than 6.*/
} fnet_nd6_option_lla_header_t;


/***********************************************************************
 * MTU option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    |           Reserved            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          MTU                                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_nd6_option_header_t    option_header   ;   /* Option general header.*/
    uint8_t                     _reserved[2]    ;
    uint32_t                    mtu             ;   /* The recommended MTU for the link.*/
} fnet_nd6_option_mtu_header_t;

/***********************************************************************
 * Prefix Information option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    | Prefix Length |L|A| Reserved1 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Valid Lifetime                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Preferred Lifetime                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Reserved2                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  +                                                               +
 *  |                                                               |
 *  +                             Prefix                            +
 *  |                                                               |
 *  +                                                               +
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_nd6_option_header_t    option_header   ;   /* Option general header.*/
    uint8_t                     prefix_length   ;   /* The number of leading bits
                                                     * in the Prefix that are valid. The value ranges
                                                     * from 0 to 128. The prefix length field provides
                                                     * necessary information for on-link determination
                                                     * (when combined with the L flag in the prefix
                                                     * information option). It also assists with address
                                                     * autoconfiguration as specified in [ADDRCONF], for
                                                     * which there may be more restrictions on the prefix
                                                     * length.*/
    uint8_t                     flag            ;   /* ND6_OPTION_FLAG_L and/or ND6_OPTION_FLAG_O flags.*/
    uint32_t                    valid_lifetime  ;   /* The length of time in
                                                     * seconds (relative to the time the packet is sent)
                                                     * that the prefix is valid for the purpose of on-link
                                                     * determination. A value of all one bits
                                                     * (0xffffffff) represents infinity. The Valid
                                                     * Lifetime is also used by [ADDRCONF].*/
    uint32_t                    prefered_lifetime ; /* The length of time in
                                                     * seconds (relative to the time the packet is sent)
                                                     * that addresses generated from the prefix via
                                                     * stateless address autoconfiguration remain
                                                     * preferred [ADDRCONF]. A value of all one bits
                                                     * (0xffffffff) represents infinity. See [ADDRCONF].
                                                     * Note that the value of this field MUST NOT exceed
                                                     * the Valid Lifetime field to avoid preferring
                                                     * addresses that are no longer valid.*/
    uint32_t                    _reserved       ;
    ipv6_addr_t                 prefix          ;   /* An IP address or a prefix of an IP address. The
                                                     * Prefix Length field contains the number of valid
                                                     * leading bits in the prefix. The bits in the prefix
                                                     * after the prefix length are reserved and MUST be
                                                     * initialized to zero by the sender and ignored by
                                                     * the receiver. A router SHOULD NOT send a prefix
                                                     * option for the link-local prefix and a host SHOULD
                                                     * ignore such a prefix option.*/

} fnet_nd6_option_prefix_header_t;


#define FNET_ND6_OPTION_FLAG_L  (0x80U)  /* 1-bit on-link flag. When set, indicates that this
                                         * prefix can be used for on-link determination. When
                                         * not set the advertisement makes no statement about
                                         * on-link or off-link properties of the prefix. In
                                         * other words, if the L flag is not set a host MUST
                                         * NOT conclude that an address derived from the
                                         * prefix is off-link. That is, it MUST NOT update a
                                         * previous indication that the address is on-link.*/
#define FNET_ND6_OPTION_FLAG_A  (0x40U)  /* 1-bit autonomous address-configuration flag. When
                                         * set indicates that this prefix can be used for
                                         * stateless address configuration as specified in
                                         * [ADDRCONF].*/

/***********************************************************************
 * Recursive DNS Server header (RFC 6106):
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |     Type      |     Length    |           Reserved            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                           Lifetime                            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   :            Addresses of IPv6 Recursive DNS Servers            :
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    fnet_nd6_option_header_t    option_header   ;   /* Option general header.*/
    uint16_t                    _reserved       ;
    uint32_t                    lifetime        ;   /* The maximum time, in
                                                     * seconds (relative to the time the packet is sent),
                                                     * over which this RDNSS address MAY be used for name
                                                     * resolution.*/
    ipv6_addr_t                 address[1]      ;   /* One or more 128-bit IPv6 addresses of the recursive
                                                     * DNS servers.  The number of addresses is determined
                                                     * by the Length field.  That is, the number of
                                                     * addresses is equal to (Length - 1) / 2.*/
} fnet_nd6_option_rdnss_header_t;


#endif

