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

#include <netipv6/input.h>
#include <netipv6/if.h>
#include <netipv6/ipv6_header.h>
#include <netipv6/check.h>
#include <netprot/input.h>
#include <netstd/endianness.h>

void netipv6_input( netif_t *netif, netpkt_t *pkt ){
	fnet_ip6_header_t   *hdr;
	size_t              pkt_length;
	size_t              total_length;
	size_t              header_length;
	net_sockaddr_t      src_addr;
	net_sockaddr_t      dst_addr;
	uint8_t             next_header; /* aka 'protocol' */

	/* RFC 4862: By disabling IP operation,
         * silently drop any IP packets received on the interface.*/
	if(!( netif->ipv6 )) goto DROP;
	if( netif->ipv6->disabled ) goto DROP;
	
	/* The header must reside in contiguous area of memory. */
	if( netpkt_pullup(pkt,sizeof(fnet_ip6_header_t)) ) goto DROP;
	
	hdr            = netpkt_data(pkt);
	pkt_length     = NETPKT_LENGTH(pkt);
	total_length = hton16(hdr->length)+sizeof(fnet_ip6_header_t);
	
	/*
	 * Ensuring correct packet bounds, packet sanity.
	 * 
	 * total_length <= pkt_length
	 */
	if(
		(pkt_length  < total_length)||
		(FNET_IP6_HEADER_GET_VERSION(hdr) != 6u)
	)goto DROP;
	
	if(pkt_length > total_length){
		/* Logical size and the physical size of the packet should be the same.*/
		netpkt_setlength(pkt,(uint32_t)total_length);
	}
	
	src_addr.type  = NET_SKA_IN6;
	src_addr.ip.v6 = hdr->source_addr;
	dst_addr.type  = NET_SKA_IN6;
	dst_addr.ip.v6 = hdr->destination_addr;
	next_header    = hdr->next_header;
	
	if(
		netipv6_addr_is_multicast(netif,&(src_addr.ip.v6))||
		(!netipv6_addr_is_self(netif,&(dst_addr.ip.v6)))
	) goto DROP;
	
	/*
	 * Remember the current offset in the packet.
	 */
	netpkt_levelup(pkt);
	
	if( netpkt_pullfront(pkt, sizeof(fnet_ip6_header_t) ) ) goto DROP;
	
	/********************************************
	 * Extension headers processing.
	 *********************************************/
	/* */
	/* if(fnet_ip6_ext_header_process(netif, &next_header, &(src_addr.ip.v6), &(dst_addr.ip.v6), &pkt, ip6_nb) == FNET_ERR) return; */
	
	/* Note: (http://www.cisco.com/web/about/ac123/ac147/archived_issues/ipj_9-3/ipv6_internals.html)
	 * Note that there is no standard extension header format, meaning that when a host
	 * encounters a header that it does not recognize in the protocol chain, the only thing
	 * it can do is discard the packet. Worse, firewalls and routers configured to filter IPv6
	 * have the same problem: as soon as they encounter an unknown extension header,
	 * they must decide to allow or disallow the packet, even though another header deeper
	 * inside the packet would possibly trigger the opposite behavior. In other words, an IPv6
	 * packet with a TCP payload that would normally be allowed through could be blocked if
	 * there is an unknown extension header between the IPv6 and TCP headers.
	 */
	
	netprot_input(netif,pkt,next_header,&src_addr,&dst_addr);
	
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
	 * fnet_netbuf_free_chain(nb);
	 * fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_NEXT_HEADER,
	 *                (fnet_uint32_t)(next_header) - (fnet_uint32_t)(ip6_nb->data_ptr), ip6_nb );
	 *
	 * TBD not tested.
	 */
	
	return;
DROP:
	netpkt_free(pkt);
}

