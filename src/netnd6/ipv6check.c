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

#include <netnd6/ipv6check.h>
#include <netnd6/nd6_header.h>
#include <netipv6/defs.h>

int netnd6_check_hop_limit(netpkt_t *pkt,uint8_t hoplimit){
	partial_ipv6_header_t* iphdr;
	uint32_t               icmp_offset;
	int                    bad = -1;
	
	icmp_offset = NETPKT_OFFSET(pkt); /* Remember the Offset of the ICMP header. */
	
	netpkt_switchlevel(pkt,-1);
	
	/*
	 * The IPv6 header was dropped (eg. IP datagram fragmentation). So we cannot verify
	 * the Hop-Limit.
	 */
	if( icmp_offset == NETPKT_OFFSET(pkt) ) goto BAD;
	
	/* The header must reside in contiguous area of memory. */
	if( netpkt_pullup(pkt,sizeof(partial_ipv6_header_t)) ) goto BAD;
	
	iphdr = netpkt_data(pkt);
	if( iphdr->hop_limit != hoplimit ) goto BAD; /* The IP Hop Limit field has a value of hoplimit.*/
	
	bad = 0;
BAD:
	/*
	 * Switch back from the IP header level up to the ICMP level.
	 */
	netpkt_switchlevel(pkt,1);
	
	return bad;
}

