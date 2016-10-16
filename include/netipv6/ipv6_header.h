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


#ifndef _NETIPV6_HEADER_H_
#define _NETIPV6_HEADER_H_


#include <netstd/stdint.h>
#include <netstd/packing.h>
#include <netipv6/ipv6.h>

/*********************************************************************
* IP packet header
**********************************************************************
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |Version| Traffic Class |           Flow Label                  |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |         Payload Length        |  Next Header  |   Hop Limit   |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                         Source Address                        +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                      Destination Address                      +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
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
    ipv6_addr_t source_addr        ;   /* 128-bit address of the originator of the packet. */
    ipv6_addr_t destination_addr   ;   /* 128-bit address of the intended recipient of the
                                        * packet (possibly not the ultimate recipient, if
                                        * a Routing header is present). */
} fnet_ip6_header_t;

/*
 * Offset of the next_header field inside the IPv6 header.
 */
#define NETIPV6_IP6HDR_OFFSETOF_NEXT_HEADER  6

#define FNET_IP6_HEADER_GET_VERSION(x)                   (((x)->version__tclass & 0xF0u)>>4)

/******************************************************************
* Extension header types
*******************************************************************/
#define FNET_IP6_TYPE_HOP_BY_HOP_OPTIONS                     (0U)
#define FNET_IP6_TYPE_DESTINATION_OPTIONS                    (60U)
#define FNET_IP6_TYPE_ROUTING_HEADER                         (43U)
#define FNET_IP6_TYPE_FRAGMENT_HEADER                        (44U)
#define FNET_IP6_TYPE_AUTHENTICATION_HEADER                  (51U)
#define FNET_IP6_TYPE_ENCAPSULATION_SECURITY_PAYLOAD_HEADER  (50U)
#define FNET_IP6_TYPE_MOBILITY_HEADER                        (135U)
#define FNET_IP6_TYPE_NO_NEXT_HEADER                         (59U) /* RFC 2460: The value 59 in the Next Header field of an IPv6 header or any
                                                                    * extension header indicates that there is nothing following that
                                                                    * header. If the Payload Length field of the IPv6 header indicates the
                                                                    * presence of octets past the end of a header whose Next Header field
                                                                    * contains 59, those octets must be ignored.*/


/***********************************************************************
 * This header format is used by the following extension headers:
 * - Hop-by-Hop Options Header     (next_header=0)
 * - Routing Header                (next_header=43)
 * - Destination Options Header    (next_header=60)
 *
 * The Hop-by-Hop Options header is used to carry optional information
 * that must be examined by every node along a packet's delivery path.
 ***********************************************************************
 * RFC 2460 4.3, 4.4 and 4.6:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Next Header   | Hdr Ext Len   |                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
 *  |                                                               |
 *  .                                                               .
 *  .                      . . . . . . . .                          .
 *  .                                                               .
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    uint8_t   next_header     ;   /* 8-bit selector. Identifies the type of header
                                   * immediately following the Options
                                   * header. */
    uint8_t   hdr_ext_length  ;   /* 8-bit unsigned integer. Length of the Hop-by-
                                   * Hop Options header in 8-octet units, not
                                   * including the first 8 octets. */
} netipv6_ext_generic_t;

/***********************************************************************
 * Options (used in op-by-Hop Options Header & Destination Options Header)
 ***********************************************************************
 * RFC 2460 4.2:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- - - - - - - - -
 * | Option Type   | Opt Data Len  | Option Data
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- - - - - - - - -
 ***********************************************************************/
typedef struct NETSTD_PACKED
{
    uint8_t    type           ;   /* 8-bit identifier of the type of option. */
    uint8_t    data_length    ;   /* 8-bit unsigned integer. Length of the Option
                                                         * Data field of this option, in octets. */
} fnet_ip6_option_header_t;


#define FNET_IP6_OPTION_TYPE_PAD1   (0x00u)  /* The Pad1 option is used to insert 
                                             * one octet of padding into the Options area of a header.*/
#define FNET_IP6_OPTION_TYPE_PADN   (0x01u)  /* The PadN option is used to insert two or more octets of padding
                                             * into the Options area of a header. For N octets of padding, the
                                             * Opt Data Len field contains the value N-2, and the Option Data
                                             * consists of N-2 zero-valued octets. */

/* RFC 2460: The Option Type identifiers are internally encoded such that their
 * highest-order two bits specify the action that must be taken if the
 * processing IPv6 node does not recognize the Option Type:*/
#define FNET_IP6_OPTION_TYPE_UNRECOGNIZED_MASK          (0xC0u)

#define FNET_IP6_OPTION_TYPE_UNRECOGNIZED_SKIP          (0x00u)  /* 00 - skip over this option and continue processing the header.*/
#define FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD       (0x40u)  /* 01 - discard the packet. */
#define FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD_ICMP  (0x80u)  /* 10 - discard the packet and, regardless of whether or not the
                                                                 * packets Destination Address was a multicast address, send an
                                                                 * ICMP Parameter Problem, Code 2, message to the packets
                                                                 * Source Address, pointing to the unrecognized Option Type.*/
#define FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD_UICMP (0xC0u)  /* 11 - discard the packet and, only if the packets Destination
                                                                 * Address was not a multicast address, send an ICMP Parameter
                                                                 * Problem, Code 2, message to the packets Source Address,
                                                                 * pointing to the unrecognized Option Type.*/


#endif

