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


#ifndef _NETIPV4_HEADER_H_
#define _NETIPV4_HEADER_H_


#include <netstd/stdint.h>
#include <netstd/packing.h>
#include <netipv4/ipv4.h>

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of IP header.
 ******************************************************************************/

typedef struct NETSTD_PACKED
{
    uint8_t version__header_length    ;   /**< version =4 & header length (x4) (>=5)*/
    uint8_t tos                       ;   /**< type of service */
    uint16_t  total_length            ;   /**< total length */
    uint16_t  id                      ;   /**< identification */
    uint16_t  flags_fragment_offset   ;   /**< flags & fragment offset field (measured in 8-byte order).*/
    uint8_t  ttl                      ;   /**< time to live */
    uint8_t  protocol                 ;   /**< protocol */
    uint16_t  checksum                ;   /**< checksum */
    ipv4_addr_t source_addr           ;   /**< source address */
    ipv4_addr_t desination_addr       ;   /**< destination address */
} fnet_ip_header_t;

#define FNET_IP_HEADER_GET_VERSION(x)                   (((x)->version__header_length & 0xF0u)>>4)
#define FNET_IP_HEADER_SET_VERSION(x, version)          ((x)->version__header_length = (fnet_uint8_t)(((x)->version__header_length & 0x0Fu)|(((version)&0x0Fu)<<4)))
#define FNET_IP_HEADER_GET_HEADER_LENGTH(x)             ((x)->version__header_length & 0x0Fu)
#define FNET_IP_HEADER_SET_HEADER_LENGTH(x, length)     ((x)->version__header_length = (fnet_uint8_t)(((x)->version__header_length & 0xF0u)|((length)&0x0Fu)))

#define FNET_IP_DF              (0x4000U)    /* dont fragment flag */
#define FNET_IP_MF              (0x2000U)    /* more fragments flag */
#define FNET_IP_FLAG_MASK       (0xE000U)    /* mask for fragmenting bits */
#define FNET_IP_OFFSET_MASK     (0x1fffU)    /* mask for fragmenting bits */

#endif

