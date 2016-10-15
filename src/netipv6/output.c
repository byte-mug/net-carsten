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

#include <netipv6/output.h>
#include <netipv6/ipv6.h>
#include <netipv6/defs.h>
#include <netipv6/ipv6_header.h>
#include <netipv6/check.h>
#include <netipv6/if.h>

#include <netif/ifapi.h>

#include <netstd/endianness.h>

void netipv6_output(
	netif_t *nif,
	netpkt_t *pkt,
	net_sockaddr_t *src_addr,
	net_sockaddr_t *dst_addr,
	uint8_t protocol,
	uint8_t hop_limit,
	uint8_t tclass
) {
	fnet_ip6_header_t   *ip6_header;
	uint32_t            total_length;
	uint32_t            payload_length;
	ipv6_addr_t         dst_ip;
	
	if(nif == 0) goto DROP;
	
	/* RFC 4862: By disabling IP operation, the node will then not
	 * send any IP packets from the interface.*/
	if( netipv6_deactivated(nif) ) goto DROP;
	
	dst_ip = dst_addr->ip.v6;
	
	/* Validate destination address. */
	/* RFC3513: The unspecified address must not be used as the destination address
	 * of IPv6 packets or in IPv6 Routing Headers.*/
	if( IP6_ADDR_IS_UNSPECIFIED(dst_ip) ) goto DROP;
	
	if( hop_limit == 0 ) hop_limit = nif->ipv6->hop_limit;
	
	/*
	 * Get length before actual IP header creation.
	 */
	payload_length = NETPKT_LENGTH(pkt);
	
	/****** Construct IP header. ******/
	if( netpkt_leveldown(pkt) ) goto DROP;
	
	if( netpkt_pushfront( pkt, sizeof(fnet_ip6_header_t) ) ) goto DROP;
	
	if( netpkt_pullup_lite( pkt, sizeof(fnet_ip6_header_t) ) ) goto DROP;
	
	ip6_header                    =  netpkt_data(pkt);
	ip6_header->version__tclass   =  (6 << 4) | (tclass>>4);
	ip6_header->tclass__flowl     =  tclass << 4;
	ip6_header->flowl             =  0u;
	ip6_header->length            =  hton16((uint16_t)payload_length); /* XXX: jumbo frames? */
	ip6_header->next_header       =  protocol;
	ip6_header->hop_limit         =  hop_limit;
	ip6_header->source_addr       =  src_addr->ip.v6;
	ip6_header->destination_addr  =  dst_ip;
	
	total_length = NETPKT_LENGTH(pkt);
	
	if(total_length > nif->netif_mtu) /* IP Fragmentation. */
	{
		// TODO: IP fragmentation
		goto DROP;
	}else{
		nif->netif_class->ifapi_send_l3_ipv6(nif,pkt,&dst_ip);
	}

DROP:
	netpkt_free(pkt);
}

