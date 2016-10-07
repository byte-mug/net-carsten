/*
 *   Copyright 2016 Simon Schmidt
 *   Copyright 2011-2016 by Andrey Butok. FNET Community.
 *   Copyright 2008-2010 by Andrey Butok. Freescale Semiconductor, Inc.
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


#ifndef _NETPROT_CHECKSUM_H_
#define _NETPROT_CHECKSUM_H_

#include <netstd/stdint.h>
#include <netpkt/pkt.h>

uint16_t netprot_checksum_buf(void* ptr, size_t len);
uint16_t netprot_checksum(netpkt_t *pkt, size_t len);
uint16_t netprot_checksum_pseudo_start( netpkt_t *pkt, uint8_t protocol, uint16_t protocol_len );
uint16_t netprot_checksum_pseudo_end( uint16_t sum_s, const uint8_t *ip_src, uint8_t *ip_dest, size_t addr_size );

#endif

