/*
 *   Copyright 2016 Simon Schmidt
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

#include <netprot/input.h>
#include <netprot/defaults.h>

#include <neticmp/input.h>
#include <neticmp/output.h>
#include <neticmp6/input.h>
#include <neticmp6/output.h>
#include <neticmp6/icmp6_header.h>
#include <netudp/input.h>
#include <nettcp/input.h>
#include <netgre/input.h>
#include <nettcp/response.h>

#include <netstd/stdint.h>
#include <netstd/packing.h>

#include <netsock/hashtab.h>

/*
 * Both TCP and UDP/UDPLite begin with a source and a destination port.
 */

typedef struct NETSTD_PACKED
{
    uint16_t src_port ; /* Source port number.*/
    uint16_t dst_port ; /* Destination port number.*/
} gen_tcp_udp_header_t;

void netprot_input(netif_t *nif, netpkt_t *pkt, uint8_t protocol, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr){
	netsock_flow_t* flow;
	gen_tcp_udp_header_t *porthdr;
	
	/* Extract port numbers from UDP and TCP headers. */
	switch(protocol){
	case IP_PROTOCOL_UDP:
	case IP_PROTOCOL_TCP:
		if( netpkt_pullup( pkt, sizeof(gen_tcp_udp_header_t) ) ) goto DROP;
		porthdr = netpkt_data(pkt);
		src_addr->port = porthdr->src_port;
		dst_addr->port = porthdr->dst_port;
		break;
	default:
		src_addr->port = 0;
		dst_addr->port = 0;
	}

	switch(protocol){
	case IP_PROTOCOL_ICMP:
		if(src_addr->type != NET_SKA_IN) goto NO_PROTO;
		neticmp_input(nif,pkt,src_addr,dst_addr);
		return;
	case IP_PROTOCOL_ICMP6:
		if(src_addr->type != NET_SKA_IN6) goto NO_PROTO;
		neticmp6_input(nif,pkt,src_addr,dst_addr);
		return;
	case IP_PROTOCOL_UDP:
		if(! nif->sockets ) goto NO_PROTO;
		flow = netsock_lookup_flow(nif->sockets, protocol, src_addr, dst_addr);
		if(! flow ) flow = netsock_lookup_flow_port(nif->sockets, protocol, dst_addr);
		
		if(! flow ) goto NO_PROTO; /* No socket has been found. */
		
		netudp_input(nif, pkt, flow, src_addr, dst_addr);
		return;
	case IP_PROTOCOL_TCP:
		/*
		 * When no socket hashtable is present, we treat TCP as an unreachable protocol.
		 */
		if(! nif->sockets ) goto NO_PROTO;
		flow = netsock_lookup_flow(nif->sockets, protocol, src_addr, dst_addr);
		if(! flow ) flow = netsock_lookup_flow_port(nif->sockets, protocol, dst_addr);
		
		/*
		 * When eighter a listening socket or a connected socket has
		 * been found, submit the packet to it.
		 */
		if( flow ) nettcp_input(nif, pkt, flow, src_addr, dst_addr);
		/*
		 * When port or socket not reachable, send TCP-RST.
		 */
		else nettcp_sendrst(nif, src_addr, dst_addr, pkt);
		return;
	case IP_PROTOCOL_GRE:
		/*
		 * GRE uses the socket hashtable.
		 */
		if(! nif->sockets ) goto NO_PROTO;
		
		flow = netsock_lookup_flow(nif->sockets, protocol, src_addr, dst_addr);
		if(! flow ) flow = netsock_lookup_flow_port(nif->sockets, protocol, dst_addr);
		if(! flow ) goto NO_PROTO;
		
		netgre_input(nif, pkt, flow, src_addr, dst_addr);
		return;
	}
	
NO_PROTO:
	switch(src_addr->type){
	//case NET_SKA_IN:
	/* TODO:
	 *
	 * fnet_netbuf_free_chain(nb);
	 * fnet_icmp_error(netif, FNET_ICMP_UNREACHABLE, FNET_ICMP_UNREACHABLE_PROTOCOL, ip4_nb);
	 */
	case NET_SKA_IN6:
		/* RFC 2460 4:If, as a result of processing a header, a node is required to proceed
		 * to the next header but the Next Header value in the current header is
		 * unrecognized by the node, it should discard the packet and send an
		 * ICMP Parameter Problem message to the source of the packet, with an
		 * ICMP Code value of 1 ("unrecognized Next Header type encountered")
		 * and the ICMP Pointer field containing the offset of the unrecognized
		 * value within the original packet. The same action should be taken if
		 * a node encounters a Next Header value of zero in any header other
		 * than an IPv6 header.*/

		/*
		 * When coming from Layer 2, an IPv6 packet carries a pointer to the Last next_header
		 * inside its netpkt_t structure.
		 */
		neticmp6_error(nif,pkt,protocol,src_addr,dst_addr,FNET_ICMP6_TYPE_PARAM_PROB,FNET_ICMP6_CODE_PP_NEXT_HEADER);
		return;
	}
DROP:
	netpkt_free(pkt);
}

