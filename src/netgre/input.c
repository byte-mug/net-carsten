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
#include <netgre/input.h>
#include <netsock/hashtab.h>
#include <netgre/gre_header.h>
#include <netgre/instance.h>
#include <netvnic/vnic.h>

#include <netstd/endianness.h>

void netgre_input(netif_t *nif,netpkt_t *pkt, netsock_flow_t *flow, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr){
	uint16_t         hdrlen = sizeof(netgre_header_t);
	uint16_t         protocol_type;
	netgre_header_t  *hdr;
	netgre_inst_t    *inst;
	netvnic_t        *vnic;
	
	inst = (netgre_inst_t*)(flow->instance);
	vnic = inst->vnic;
	
	if( netpkt_pullup( pkt, hdrlen ) ) goto DROP;
	
	hdr = netpkt_data( pkt );
	if( (hdr->flags & NETGRE_FLAGS_CHECKSUM) &&
		(netprot_checksum( pkt, NETPKT_LENGTH(pkt)) != 0) )
		goto DROP;
	
	if( (hdr->version & NETGRE_VERSION_MASK) != 0)
		goto DROP;
	
	protocol_type = ntoh16(hdr->protocol_type);
	
	if(hdr->flags & NETGRE_FLAGS_KEY) hdrlen += 4;
	if(hdr->flags & NETGRE_FLAGS_SEQUENCE) hdrlen += 4;
	
	if( netpkt_pullfront( pkt, hdrlen ) ) goto DROP;
	
	vnic->vnic_input( vnic, pkt, protocol_type );
	
	return;
DROP:
	netsock_decr_flow(nif->sockets,flow);
	netpkt_free(pkt);
}


