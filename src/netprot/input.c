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
#include <neticmp6/input.h>
#include <netudp/input.h>

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
		if(! nif->udp ) break;
		flow = netsock_lookup_flow(nif->udp, protocol, src_addr, dst_addr);
		if(! flow ) break;
		netudp_input(nif, pkt, flow, src_addr, dst_addr);
		return;
	//case IP_PROTOCOL_TCP:
		
	}
	
NO_PROTO:
	netpkt_free(pkt);
}

