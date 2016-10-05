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
#include <netipv4/output.h>
//#include <netipv4/defs.h>
#include <netipv4/ipv4_header.h>
#include <netstd/endianness.h>
#include <netif/ifapi.h>

void netipv4_output(
	netif_t *nif, netpkt_t *pkt, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr,
	uint8_t protocol,uint8_t tos, uint8_t ttl, char DF, char do_not_route, uint16_t *checksum
){
	fnet_ip_header_t   *ipheader;
	uint16_t           fragment;
	uint32_t           total_length;
	
	if(nif == 0) goto DROP;
	
	/* If source address not specified, use address of outgoing interface */
	if( IP4ADDR_EQ(src_addr->ip.v4,0) ) src_addr->ip.v4 = nif->ipv4.address;
	
	total_length = NETPKT_LENGTH(pkt);
	/* if((nb->total_length + sizeof(fnet_ip_header_t)) > FNET_IP_MAX_PACKET) */
	
	fragment = 0;
	if( DF ) fragment |= FNET_IP_DF;
	
	netpkt_leveldown(pkt);
	
	/* Construct IP header */
	if( netpkt_pushfront( pkt, sizeof(fnet_ip_header_t) ) ) goto DROP;
	
	/* Pseudo checksum. */
	if(checksum)
	{
		// *checksum = fnet_checksum_pseudo_end( *checksum, (fnet_uint8_t *)&src_ip, (fnet_uint8_t *)&dest_ip, sizeof(fnet_ip4_addr_t) );
	}
	
	ipheader = netpkt_data(pkt);
	
	ipheader->flags_fragment_offset = hton16(fragment);
	
	ipheader->tos = tos;                         /* Type of service */
	ipheader->ttl = ttl;                         /* time to live */
	ipheader->protocol = protocol;               /* protocol */
	ipheader->source_addr = src_addr->ip.v4;     /* source address */
	ipheader->desination_addr = src_addr->ip.v4; /* destination address */
	
	ipheader->total_length = hton16((uint16_t)total_length);
	//ipheader->id = fnet_htons(ip_id++);              /* Id */
	
	if(total_length > nif->netif_mtu) /* IP Fragmentation. */
	{
		// TODO: IP fragmentation
		goto DROP;
	}else{
		nif->netif_class->ifapi_send_l3_ipv4(nif,pkt,&(dst_addr->ip.v4));
	}
	
DROP:
	netpkt_free(pkt);
}

