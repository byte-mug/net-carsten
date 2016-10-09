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

#include <neticmp/output.h>
#include <neticmp/icmp_header.h>
#include <netipv4/output.h>
#include <netipv4/hldefs.h>
#include <netprot/defaults.h>
#include <netprot/checksum.h>

void neticmp_output(netif_t *nif,netpkt_t *pkt, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr){
	fnet_icmp_header_t *hdr;
	
	hdr           = netpkt_data(pkt);
	/* Checksum calculation.*/
	hdr->checksum = 0u;
	hdr->checksum = netprot_checksum_buf((void*)hdr,NETPKT_LENGTH(pkt));
	
	netipv4_output(nif,pkt,src_addr,dst_addr,IP_PROTOCOL_ICMP, IP_TOS_NORMAL, IP_TTL_DEFAULT, /*DF=*/0, /*dont_route=*/0 );
}
