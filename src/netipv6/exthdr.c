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
#include <netipv6/exthdr.h>

#include <netipv6/ipv6_header.h>

void netipv6_ext_header_process(netif_t *netif, uint8_t *pnext_header, ipv6_addr_t *src, ipv6_addr_t *dst, netpkt_t **ppkt){
	netpkt_t*                  pkt;
	uint32_t                   hcount;
	uint8_t                    next_header;
	netipv6_ext_generic_t      *opt_hdr;
	
	/* RFC 2460 4:
	 * Therefore, extension headers must
	 * be processed strictly in the order they appear in the packet; a
	 * receiver must not, for example, scan through a packet looking for a
	 * particular kind of extension header and process that header prior to
	 * processing all preceding ones.
	 */
	
	pkt = *ppkt;
	next_header = *pnext_header;
	hcount = 0;
	
	/* Process headers.*/
	for(;;)
	{
		/* IPv6 nodes must accept and attempt to process extension headers in
		 * any order and occurring any number of times in the same packet,
		 * except for the Hop-by-Hop Options header which is restricted to
		 * appear immediately after an IPv6 header only.*/
		if( hcount && ( next_header==FNET_IP6_TYPE_HOP_BY_HOP_OPTIONS ) ) goto DROP;
		
		hcount++;
		
		switch(next_header){
		case FNET_IP6_TYPE_HOP_BY_HOP_OPTIONS:
		case FNET_IP6_TYPE_ROUTING_HEADER:
		case FNET_IP6_TYPE_DESTINATION_OPTIONS:
			if( netpkt_pullup(pkt,sizeof(netipv6_ext_generic_t)) ) goto DROP;
			opt_hdr = netpkt_data(pkt);
			next_header = opt_hdr->next_header;
			opt_hdr->hdr_ext_length;
			if( netpkt_pullfront(pkt,8 + opt_hdr->hdr_ext_length ) ) goto DROP;
			break;
		case FNET_IP6_TYPE_NO_NEXT_HEADER: goto DROP;
		case FNET_IP6_TYPE_FRAGMENT_HEADER:
			// TODO: IPv6 reassembly
			goto DROP;
		default: goto DONE;
		}
	}
DONE:
	*pnext_header = next_header;
	return;
DROP:
	netpkt_free(pkt);
	*ppkt = 0;
	return;
}

