/*
 *   Copyright 2016 Simon Schmidt
 *   Copyright 2011-2016 by Andrey Butok. FNET Community.
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
#include <neticmp/input.h>
#include <neticmp/output.h>
#include <neticmp/icmp_header.h>
#include <netipv4/ipv4.h>
#include <netipv4/defs.h>
#include <netipv4/check.h>
#include <netprot/checksum.h>
#include <netprot/notify.h>

void neticmp_input(netif_t *nif,netpkt_t *pkt, net_sockaddr_t *src_addr, net_sockaddr_t *dst_addr){
	fnet_icmp_header_t      *hdr;
	fnet_prot_notify_t      prot_cmd;
	ipv4_addr_t             src_ip;
	ipv4_addr_t             dest_ip;
	uint32_t                pkt_length;
	
	
	src_ip     = src_addr->ip.v4;
	dest_ip    = dst_addr->ip.v4;
	
	/*
	 * Source address must not be any multicast/broadcast address.
	 */
	if( IP4_ADDR_IS_MULTICAST(src_ip) || netipv4_addr_is_broadcast(nif,src_ip) ) goto DROP;
	
	pkt_length = NETPKT_LENGTH(pkt);
	
	/*
	 * Checksum test.
	 */
	if(netprot_checksum(pkt, pkt_length) != 0u) goto DROP;
	
	if( netpkt_pullup(pkt,sizeof(fnet_icmp_header_t)) ) goto DROP;
	
	hdr        = netpkt_data(pkt);
	
	switch(hdr->type){
	/**************************
	 * ICMP Request Processing
	 **************************/
	case FNET_ICMP_ECHO:
		/* An ICMP Echo Request destined to an IP broadcast or IP
		 * multicast address MAY be silently discarded.(RFC1122)*/
		if(pkt->flags & NETPKT_FLAG_BROAD_L3) goto DROP;
		hdr->type = FNET_ICMP_ECHOREPLY;
		neticmp_output(nif,pkt,src_addr,dst_addr);
		break;
		
	/**************************
	 * ICMP Error Processing
	 **************************/
	case FNET_ICMP_UNREACHABLE:
		switch(hdr->code){
		case FNET_ICMP_UNREACHABLE_NET:           /* net unreachable */
		case FNET_ICMP_UNREACHABLE_NET_UNKNOWN:   /* unknown net */
		case FNET_ICMP_UNREACHABLE_NET_PROHIB:    /* prohibited access */
		case FNET_ICMP_UNREACHABLE_TOSNET:        /* bad tos for net */
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_NET;
			break;
		case FNET_ICMP_UNREACHABLE_HOST:          /* host unreachable */
		case FNET_ICMP_UNREACHABLE_HOST_UNKNOWN:  /* unknown host */
		case FNET_ICMP_UNREACHABLE_ISOLATED:      /* src host isolated */
		case FNET_ICMP_UNREACHABLE_HOST_PROHIB:   /* ditto */
		case FNET_ICMP_UNREACHABLE_TOSHOST:       /* bad tos for host */
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_HOST;
			break;
		
		case FNET_ICMP_UNREACHABLE_PROTOCOL:      /* protocol unreachable */
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_PROTOCOL;
			break;
		
		case FNET_ICMP_UNREACHABLE_PORT:          /* port unreachable */
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_PORT;
			break;
		
		case FNET_ICMP_UNREACHABLE_SRCFAIL:       /* source route failed */
			prot_cmd = FNET_PROT_NOTIFY_UNREACH_SRCFAIL;
			break;
		case FNET_ICMP_UNREACHABLE_NEEDFRAG:      /* fragmentation needed and DF set*/
			prot_cmd = FNET_PROT_NOTIFY_MSGSIZE;
			break;
		
		default: goto DROP;
                }
		netprot_notify(nif,pkt,prot_cmd,src_addr,dst_addr); /* Protocol notification.*/
		break;
	
	case FNET_ICMP_TIMXCEED:
		switch(hdr->code) {
		case FNET_ICMP_TIMXCEED_INTRANS:          /* time to live exceeded in transit (ttl==0)*/
			prot_cmd = FNET_PROT_NOTIFY_TIMXCEED_INTRANS;
			break;
		
		case FNET_ICMP_TIMXCEED_REASS:            /* fragment reassembly time exceeded (ttl==0)*/
			prot_cmd = FNET_PROT_NOTIFY_TIMXCEED_REASS;
			break;
		
		default: goto DROP;
                }

                netprot_notify(nif,pkt,prot_cmd,src_addr,dst_addr); /* Protocol notification.*/
                break;
	
	case FNET_ICMP_PARAMPROB:                       /* Parameter Problem Message.*/
		if(hdr->code > 1u) goto DROP;
		
		prot_cmd = FNET_PROT_NOTIFY_PARAMPROB;
		netprot_notify(nif,pkt,prot_cmd,src_addr,dst_addr); /* Protocol notification.*/
		break;
	
	case FNET_ICMP_SOURCEQUENCH:                    /* Source Quench Message; packet lost, slow down.*/
		if(hdr->code) goto DROP;
		
		prot_cmd = FNET_PROT_NOTIFY_QUENCH;
		netprot_notify(nif,pkt,prot_cmd,src_addr,dst_addr); /* Protocol notification.*/
		break;
	/************************
	 * Ignore others
	 ************************/
	/*
	case FNET_ICMP_REDIRECT:
	case FNET_ICMP_ECHOREPLY:
	case FNET_ICMP_ROUTERADVERT:
	case FNET_ICMP_ROUTERSOLICIT:
	case FNET_ICMP_TSTAMPREPLY:
	case FNET_ICMP_IREQREPLY:
	case FNET_ICMP_MASKREPLY:*/
	default: goto DROP;
	}
	return;
DROP:
	netpkt_free(pkt);
}

