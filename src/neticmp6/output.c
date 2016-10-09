/*
 *   Copyright 2016 Simon Schmidt
 * Copyright 2011-2016 by Andrey Butok. FNET Community.
 * Copyright 2008-2010 by Andrey Butok. Freescale Semiconductor, Inc.
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

#include <neticmp6/output.h>
#include <neticmp6/icmp6_header.h>
#include <netipv6/output.h>
#include <netipv6/ipv6.h>
#include <netprot/checksum.h>
#include <netprot/defaults.h>

void neticmp6_output(netif_t *nif,netpkt_t *pkt, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr,uint8_t hop_limit){
	fnet_icmp6_header_t                     *hdr;
	uint16_t                                checksum;
	
	hdr           = netpkt_data(pkt);
	/* Checksum calculation.*/
	hdr->checksum = 0u;
	checksum = netprot_checksum_pseudo_start(pkt,IP_PROTOCOL_ICMP6,(uint16_t)NETPKT_LENGTH(pkt));
	hdr->checksum = netprot_checksum_pseudo_end(
			checksum,
			(uint8_t*)&(src_addr->ip.v6),
			(uint8_t*)&(dst_addr->ip.v6),
			sizeof(ipv6_addr_t)
	);
	
	netipv6_output(nif,pkt,src_addr,dst_addr,IP_PROTOCOL_ICMP6,hop_limit,0);
}

