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
#include <netgre/output.h>
#include <netgre/gre_header.h>
#include <netgre/instance.h>
#include <netif/if.h>
#include <netprot/defaults.h>

#include <netstd/mem.h>
#include <netstd/endianness.h>

/*
 * (netvnic_t*)->vnic_output
 */
void netgre_output(netvnic_t* vnic,netpkt_t* pkt,uint16_t protocol,hwaddr_t* dst){
	uint16_t         hdrlen = sizeof(netgre_header_t);
	netgre_header_t  *hdr;
	netgre_inst_t    *inst;
	
	inst = (netgre_inst_t*)(vnic->vnic_out_inst);
	
	/* Increase the pkt->level up to 3, if below. */
	while(pkt->level<3) netpkt_levelup(pkt);
	
	/* Construct GRE header.*/
	if( netpkt_leveldown(pkt) ) goto DROP;
	
	if( netpkt_pushfront( pkt, sizeof(netgre_header_t) ) ) goto DROP;
	
	if( netpkt_pullup_lite( pkt, sizeof(netgre_header_t) ) ) goto DROP;
	
	hdr = netpkt_data( pkt );
	
	/*
	 * We fill the entire header with null-bytes, except the protocol field.
	 */
	net_bzero( hdr ,sizeof(netgre_header_t) );
	
	hdr->protocol_type = hton16(protocol);
	
	netprot_ip_output( inst->nif, pkt, IP_PROTOCOL_TCP, &(inst->local_addr), &(inst->remote_addr), 0, 0 );
	
	return;
DROP:
	netpkt_free( pkt );
}
