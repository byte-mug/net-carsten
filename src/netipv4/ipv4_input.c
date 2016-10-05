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
#include <netipv4/ipv4_input.h>
#include <netipv4/defs.h>
#include <netipv4/check.h>
#include <netipv4/ipv4_header.h>

#include <netsock/addr.h>
#include <netstd/endianness.h>


void netipv4_input( netif_t *netif, netpkt_t *pkt ){
	fnet_ip_header_t    *hdr;
	ipv4_addr_t         destination_addr;
	size_t              pkt_length;
	size_t              total_length;
	size_t              header_length;
	uint16_t            fragment;
	net_sockaddr_t      src_addr;
	net_sockaddr_t      dst_addr;
	
	/* The header must reside in contiguous area of memory. */
	if( netpkt_pullup(pkt,sizeof(fnet_ip_header_t)) ) goto DROP;
	
	hdr = netpkt_data(pkt);
	destination_addr = hdr->desination_addr;
	total_length = ntoh16(hdr->total_length);
	header_length = (size_t)FNET_IP_HEADER_GET_HEADER_LENGTH(hdr) << 2;
	pkt_length = NETPKT_LENGTH(pkt);
	
	/*
	 * Ensuring correct packet bounds, packet sanity.
	 * 
	 * sizeof(fnet_ip_header_t) <= header_length <= total_length <= pkt_length
	 */
	if(
		(header_length < sizeof(fnet_ip_header_t) )||
		(total_length < header_length)||
		(pkt_length  < total_length)||
		(FNET_IP_HEADER_GET_VERSION(hdr) != 4u)
		/* (fnet_checksum(nb, header_length) != 0u) */
	)goto DROP;
	
	/* Loopback packets skip the address validation. */
	if( netif->flags & NETIF_IS_LOOPBACK ) goto CHECK_DONE;
	
	if(
		netipv4_addr_is_broadcast(netif,destination_addr)||
		/* ((destination_addr==0u) && pkt->flags & NETPKT_FLAG_BROAD_L2 )|| */
		(IP4_ADDR_IS_MULTICAST(destination_addr))
	) pkt->flags |= NETPKT_FLAG_BROAD_L3;
	
	/* If not for me, drop! */
	if(!(
		(pkt->flags & NETPKT_FLAG_BROAD_L3)||
		IP4ADDR_EQ(netif->ipv4.address,destination_addr)
	))goto DROP;
	
CHECK_DONE:
	fragment = ntoh16(hdr->flags_fragment_offset);
	
	/* TODO: if(pkt_length > total_length) */
	
	if( fragment & ~FNET_IP_DF ){
		/* TODO: fragmentation */
		goto DROP;
	}
	
	src_addr.type  = NET_SKA_IN;
	src_addr.ip.v4 = hdr->source_addr;
	dst_addr.type  = NET_SKA_IN;
	dst_addr.ip.v4 = destination_addr;
	
	/*
	 * Remember the current offset in the packet.
	 */
	netpkt_levelup(pkt);
	
	if( netpkt_pullfront(pkt,(uint32_t)header_length) ) goto DROP;
	
	
	
	netpkt_free(pkt);
	/*
	 * fnet_netbuf_free_chain(nb);
	 * fnet_icmp_error(netif, FNET_ICMP_UNREACHABLE, FNET_ICMP_UNREACHABLE_PROTOCOL, ip4_nb);
	 */
	
	return;
DROP:
	netpkt_free(pkt);
}

