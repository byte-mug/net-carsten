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

#include <neticmp6/input.h>
#include <neticmp6/output.h>
#include <neticmp6/icmp6_header.h>
#include <netipv6/ipv6.h>
#include <netipv6/defs.h>
#include <netipv6/if.h>
#include <netnd6/receive.h>
#include <netprot/checksum.h>
#include <netprot/notify.h>
#include <netstd/endianness.h>

void neticmp6_input(netif_t *nif,netpkt_t *pkt, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr){
	fnet_icmp6_header_t      *hdr;
	fnet_icmp6_err_header_t  *icmp6_err;
	fnet_prot_notify_t       prot_cmd;
	ipv6_addr_t              src_ip;
	ipv6_addr_t              dest_ip;
	uint16_t                 sum;
	uint32_t                 pkt_length;
	uint32_t                 pmtu;
	
	src_ip     = src_addr->ip.v6;
	dest_ip    = dst_addr->ip.v6;
	
	pkt_length = NETPKT_LENGTH(pkt);
	
	/* The header must reside in contiguous area of memory. */
	if( netpkt_pullup(pkt,sizeof(fnet_icmp6_header_t)) ) goto DROP;
	
	hdr = netpkt_data(pkt);
	
	sum = netprot_checksum_pseudo_start(pkt,NETICMP6_PROTOCOL_NUM,pkt_length);
	sum = netprot_checksum_pseudo_end( sum, (uint8_t*)&src_ip, (uint8_t*)&dest_ip, sizeof(ipv6_addr_t));
	
	if(sum) goto DROP;
	
	switch (hdr->type){
	/**************************
	 * Neighbor Solicitation.
	 **************************/
	case FNET_ICMP6_TYPE_NEIGHBOR_SOLICITATION:
		netnd6_neighbor_solicitation_reveive(nif,pkt,&src_ip,&dest_ip);
		break;
	/**************************
	 * Neighbor Advertisemnt.
	 **************************/
	case FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT:
		netnd6_neighbor_advertisement_receive(nif,pkt,&src_ip,&dest_ip);
		break;
	/**************************
	 * Router Advertisemnt.
	 **************************/
	case FNET_ICMP6_TYPE_ROUTER_ADVERTISEMENT:
		netnd6_router_advertisement_receive(nif,pkt,&src_ip,&dest_ip);
		break;
	/**************************
	 * Router Advertisemnt.
	 **************************/
	case FNET_ICMP6_TYPE_REDIRECT:
		netnd6_redirect_receive(nif,pkt,&src_ip,&dest_ip);
		break;
	/**************************
	 * Multicast Listener Query.
	 **************************/
	case FNET_ICMP6_TYPE_MULTICAST_LISTENER_QUERY:
		//fnet_mld_query_receive(netif, src_ip, dest_ip, nb, ip6_nb);
		netpkt_free(pkt);
		break;
	/**************************
	 * Echo Request.
	 * RFC4443 4.1: Every node MUST implement an ICMPv6 Echo responder function that
	 * receives Echo Requests and originates corresponding Echo Replies.
	 **************************/
	case FNET_ICMP6_TYPE_ECHO_REQ:
		hdr->type = FNET_ICMP6_TYPE_ECHO_REPLY;
		
		/* RFC4443: the source address of the reply MUST be a unicast
		 * address belonging to the interface on which
		 * the Echo Request message was received.*/
		if(IP6_ADDR_IS_MULTICAST(dest_ip)) goto DROP; /* TODO: find corresponding dest_ip to src_ip */
		
		neticmp6_output(nif,pkt,dst_addr,src_addr);
                break;
	/**************************
	 * Packet Too Big Message.
	 **************************/
	case FNET_ICMP6_TYPE_PACKET_TOOBIG:
		if(nif->ipv6->pmtu_on) /* If PMTU is enabled for the interface.*/
		{
			
			
			/* The header must reside in contiguous area of memory. */
			if( netpkt_pullup(pkt,sizeof(fnet_icmp6_err_header_t)) ) goto DROP;
			
			icmp6_err = netpkt_data(pkt);
			
			/* RFC 1981.Upon receipt of such a
			 * message, the source node reduces its assumed PMTU for the path based
			 * on the MTU of the constricting hop as reported in the Packet Too Big
			 * message.*/
			pmtu = ntoh32(icmp6_err->data);
			
			/* A node MUST NOT increase its estimate of the Path MTU in response to
			 * the contents of a Packet Too Big message. */
			if(nif->ipv6->pmtu > pmtu) nif->ipv6->pmtu = pmtu;
		}
		goto DROP;
                break;
	/**************************
	 * Destination Unreachable.
	 **************************/
	case FNET_ICMP6_TYPE_DEST_UNREACH:
		switch(hdr->code)
		{
		case FNET_ICMP6_CODE_DU_NO_ROUTE:           /* No route to destination. */
		case FNET_ICMP6_CODE_DU_BEYOND_SCOPE:       /* Beyond scope of source address.*/
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_NET;
			break;
		case FNET_ICMP6_CODE_DU_ADMIN_PROHIBITED:   /* Communication with destination administratively prohibited. */
		case FNET_ICMP6_CODE_DU_ADDR_UNREACH:       /* Address unreachable.*/
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_HOST;
			break;
		case FNET_ICMP6_CODE_DU_PORT_UNREACH:       /* Port unreachable.*/
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_PORT;
			break;
		default: goto DROP;
                }
		netprot_notify(nif,pkt,prot_cmd,src_addr,dst_addr); /* Protocol notification.*/
		break;
	/*
	 * Parameter Problems.
	 */
	case FNET_ICMP6_TYPE_PARAM_PROB:
		switch(hdr->code){
		
		/* Protocol unreachable */
		case FNET_ICMP6_CODE_PP_NEXT_HEADER:
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_PROTOCOL;
			
			/* Protocol notification.*/
			netprot_notify(nif,pkt,prot_cmd,src_addr,dst_addr);
			break;
		default: goto DROP;
		}
		break;
	default: goto DROP;
	}
	return;
DROP:
	netpkt_free(pkt);
}

