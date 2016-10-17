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
#include <netipv6/ipv6_header.h>
#include <netipv6/defs.h>
#include <netprot/checksum.h>
#include <netprot/defaults.h>

#include <netstd/endianness.h>

/* TODO: move this elsewhere. */
#define FNET_IP6_DEFAULT_MTU     1280u   /* Minimum IPv6 datagram size which    
                                          * must be supported by all IPv6 hosts */

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

void neticmp6_error(netif_t *nif,netpkt_t *pkt,uint32_t protocol, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr,uint8_t type,uint8_t code){
	uint32_t                param, size;
	fnet_icmp6_err_header_t *icmp6_err_header;
	
	/*******************************************************************
	 * RFC 4443:
	 * (e) An ICMPv6 error message MUST NOT be originated as a result of
	 * receiving the following:
	 *******************************************************************/
	/* (e.1) An ICMPv6 error message. */
	/* (e.2) An ICMPv6 REDIRECT message [IPv6-DISC].*/
	if(protocol==IP_PROTOCOL_ICMP6){
		goto DROP;
	}
	
	/*
	 * (e.3) A packet destined to an IPv6 multicast address. (There are
	 * two exceptions to this rule: (1) the Packet Too Big Message
	 * (Section 3.2) to allow Path MTU discovery to work for IPv6
	 * multicast, and (2) the Parameter Problem Message, Code 2
	 * (Section 3.4) reporting an unrecognized IPv6 option (see
	 * Section 4.2 of [IPv6]) that has the Option Type highestorder
	 * two bits set to 10).
	 * (e.4) A packet sent as a link-layer multicast (the exceptions
	 * from e.3 apply to this case, too).
	 * (e.5) A packet sent as a link-layer broadcast (the exceptions
	 * from e.3 apply to this case, too).
	 */
	if( pkt->flags & (NETPKT_FLAG_BROAD_L3|NETPKT_FLAG_BROAD_L2) ){
		if(!(
			(type == FNET_ICMP6_TYPE_PACKET_TOOBIG) ||
			((type == FNET_ICMP6_TYPE_PARAM_PROB) && (code == FNET_ICMP6_CODE_PP_OPTION))
		))  goto DROP;
	}
	
	/*
	 * (e.6) A packet whose source address does not uniquely identify a
	 * single node -- e.g., the IPv6 Unspecified Address, an IPv6
	 * multicast address, or an address known by the ICMP message
	 * originator to be an IPv6 anycast address.
	 */
	if( IP6_ADDR_MULTICAST_SCOPE(src_addr->ip.v6) || IP6_ADDR_IS_UNSPECIFIED(src_addr->ip.v6) )
		goto DROP;
	
	param = NETPKT_OFFSET(pkt);
	
	/* Select Layer 2 header. (We're in layer 3) */
	netpkt_switchlevel(pkt,-1);
	
	/* Do we have no Layer 2 header? */
	size = NETPKT_OFFSET(pkt);
	if( param == size ){
		goto DROP;
	}
	
	/* Initialize the param*/
	param = pkt->ipv6.error_pointer;
	if( pkt->ipv6.param_is_pointer )
		param -= size;
	
	/* Increase the pkt->level up to 3, if below. */
	while(pkt->level<3) netpkt_levelup(pkt);
	
	/* Limit to FNET_IP6_DEFAULT_MTU. */
	size = FNET_IP6_DEFAULT_MTU - sizeof(fnet_icmp6_err_header_t) + sizeof(fnet_ip6_header_t);
        if( NETPKT_LENGTH(pkt) > size)
		netpkt_setlength(pkt,size);
	
	/* Construct ICMPv6 error header.*/
	if( netpkt_leveldown(pkt) ) goto DROP;
	
	if( netpkt_pushfront( pkt, sizeof(fnet_icmp6_err_header_t) ) ) goto DROP;
	
	if( netpkt_pullup_lite( pkt, sizeof(fnet_icmp6_err_header_t) ) ) goto DROP;
	
	icmp6_err_header = netpkt_data(pkt);
	icmp6_err_header->icmp6_header.type = type;
	icmp6_err_header->icmp6_header.code = code;
	icmp6_err_header->data = hton32(param);
	
	neticmp6_output(nif,pkt,dst_addr,src_addr,0);
	return;
DROP:
	netpkt_free(pkt);
}

