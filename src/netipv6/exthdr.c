/*
 *   Copyright 2016 Simon Schmidt
 *   Copyright 2011-2016 by Andrey Butok. FNET Community.
 *   Copyright 2008-2010 by Andrey Butok. Freescale Semiconductor, Inc.
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

#include <netipv6/defs.h>

enum{
	I6OPT_KEEP,
	I6OPT_DISCARD,
	I6OPT_DISCARD_ICMP,
	I6OPT_DISCARD_UICMP,
};

static int netipv6_ext_options(void *data,size_t size){
	fnet_ip6_option_header_t *option;

	while(size){
		option=data;
		switch(option->type){
		/* The RFC2460 supports only PAD0 and PADN options.*/
		case FNET_IP6_OPTION_TYPE_PAD1:
			data++;
			size--;
			break;
		case FNET_IP6_OPTION_TYPE_PADN:
			data += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			size += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			break;
		
		/*
		 * RFC 6275  6.3.  Home Address Option:
		 * 
		 *  The Home Address option is carried by the Destination Option
		 *  extension header (Next Header value = 60).  It is used in a packet
		 *  sent by a mobile node while away from home, to inform the recipient
		 *  of the mobile node's home address.
		 * 
		 *  The Home Address option is encoded in type-length-value (TLV) format
		 *  as follows:
		 *  
		 *  Option Type
		 *    201 = 0xC9
		 *
		 *  Option Length:
		 *    8-bit unsigned integer.  Length of the option, in octets,
		 *    excluding the Option Type and Option Length fields.  This field
		 *    MUST be set to 16.
		 */
		case 0xC9:
			/*
			 * We recognize this option, but we cannot consume the Home Address (yet).
			 *
			 * The option length is required to be 16, however, when the peer violates the
			 * protocol, we are going to silently ignore it.
			 */
			data += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			size += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			break;
		
		/*
		 * RFC 6788   7.  Line-Identification Option (LIO)
		 *
		 *   The Line-Identification Option (LIO) is a destination option that can
		 *   be included in IPv6 datagrams that tunnel Router Solicitation and
		 *   Router Advertisement messages.  The use of the Line-ID option in any
		 *   other IPv6 datagrams is not defined by this document.  Multiple Line-
		 *   ID destination options MUST NOT be present in the same IPv6 datagram.
		 *   The LIO has no alignment requirement.
		 *
		 *    0                   1                   2                   3
		 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *                                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *                                   |  Option Type  | Option Length |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   | LineIDLen     |     Line ID...
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *
		 *                Figure 4: Line-Identification Option Layout
		 *
		 */
		case 0x8C:
			/*
			 * We recognize this option, but we cannot consume it (yet).
			 *
			 */
			data += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			size += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			break;
		
		/* Endpoint Identification (DEPRECATED) [[CHARLES LYNN]] */
		case 0x8A:
		/* Deprecated [RFC7731] */
		case 0x4D:
			/*
			 * We recognize this option. Skip it!
			 */
			data += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			size += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			break;
		
		/* RFC 7731 6.1. MPL Option */
		/*
		 * RFC 7731 6.1. MPL Option:
		 *
		 *    The MPL Option is carried in MPL Data Messages in an IPv6 Hop-by-Hop
		 *    Options header, immediately following the IPv6 header.  The MPL
		 *    Option has the following format:
		 * 
		 *       0                   1                   2                   3
		 *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *                                      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *                                      |  Option Type  |  Opt Data Len |
		 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *      | S |M|V|  rsv  |   sequence    |      seed-id (optional)       |
		 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */
		case 0x6D:
			/*
			 * We fundamentally understand it, but it has no effect on us.
			 */
			data += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			size += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			break;
		/*
		 * RFC 6553   3.  Format of the RPL Option
		 *
		 *      0                   1                   2                   3
		 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *                                     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *                                     |  Option Type  |  Opt Data Len |
		 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *     |O|R|F|0|0|0|0|0| RPLInstanceID |          SenderRank           |
		 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *     |                         (sub-TLVs)                            |
		 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *
		 *                           Figure 1: RPL Option
		 */
		case 0x63:
			/*
			 * We fundamentally understand it, but it has no effect on us.
			 */
			data += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			size += sizeof(fnet_ip6_option_header_t) + (size_t)option->data_length;
			break;
		/*
		 * XXX Jumbo Payload [RFC2675]
		 * case 0xC2:
		 */
		/* Unrecognized Options.*/
		default:
			/* The Option Type identifiers are internally encoded such that their
			 * highest-order two bits specify the action that must be taken if the
			 * processing IPv6 node does not recognize the Option Type.*/
			switch(option->type & FNET_IP6_OPTION_TYPE_UNRECOGNIZED_MASK){
			/* 00 - skip over this option and continue processing the header.*/
			case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_SKIP:
				break;
			/* 01 - discard the packet. */
			case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD:
				return I6OPT_DISCARD;
			/* 10 - discard the packet and, regardless of whether or not the
			 *      packet's Destination Address was a multicast address, send an
			 *      ICMP Parameter Problem, Code 2, message to the packet's
			 *      Source Address, pointing to the unrecognized Option Type.*/
			case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD_ICMP:
				return I6OPT_DISCARD_ICMP;
			/* 11 - discard the packet and, only if the packet's Destination
			 *      Address was not a multicast address, send an ICMP Parameter
			 *      Problem, Code 2, message to the packet's Source Address,
			 *      pointing to the unrecognized Option Type.*/
			case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD_UICMP:
				return I6OPT_DISCARD_UICMP;
			}
		}
	}
	return I6OPT_KEEP;
}

void netipv6_ext_header_process(netif_t *netif, uint8_t *pnext_header, ipv6_addr_t *src, ipv6_addr_t *dst, netpkt_t **ppkt){
	netpkt_t*                  pkt;
	uint32_t                   hcount;
	uint8_t                    next_header;
	netipv6_ext_generic_t      *opt_hdr;
	size_t                     size;
	
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
		case FNET_IP6_TYPE_DESTINATION_OPTIONS:
			if( netpkt_pullup(pkt,sizeof(netipv6_ext_generic_t)) ) goto DROP;
			opt_hdr = netpkt_data(pkt);
			next_header = opt_hdr->next_header;
			pkt->ipv6.error_pointer = NETPKT_OFFSET(pkt);
			size = (opt_hdr->hdr_ext_length * 8) + 8;
			if( netpkt_pullup(pkt,size) ) goto DROP;
			
			switch( netipv6_ext_options(netpkt_data(pkt)+2,size-2) ){
			case I6OPT_DISCARD_UICMP:
				if( IP6_ADDR_IS_MULTICAST(*dst) ) goto DROP;
			case I6OPT_DISCARD_ICMP:
				goto ICMP_ERROR;
			case I6OPT_DISCARD:
				goto DROP;
			}
			
			if( netpkt_pullfront(pkt,size ) ) goto DROP;
			break;
		case FNET_IP6_TYPE_ROUTING_HEADER:
			if( netpkt_pullup(pkt,sizeof(netipv6_ext_generic_t)) ) goto DROP;
			opt_hdr = netpkt_data(pkt);
			next_header = opt_hdr->next_header;
			pkt->ipv6.error_pointer = NETPKT_OFFSET(pkt);
			size = (opt_hdr->hdr_ext_length * 8) + 8;
			/* TODO: handle routing header. */
			if( netpkt_pullfront(pkt,size ) ) goto DROP;
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
ICMP_ERROR:
	/* TODO: ICMP error message. */
	netpkt_free(pkt);
	*ppkt = 0;
	return;
}

