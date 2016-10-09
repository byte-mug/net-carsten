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


#include <netnd6/send.h>
#include <netnd6/nd6_header.h>
#include <netpkt/pkt.h>
#include <netif/mac.h>
#include <netsock/addr.h>
#include <neticmp6/output.h>

#include <netmem/allocpkt.h>
#include <netstd/mem.h>


void netnd6_neighbor_advertisement_send(netif_t *nif, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip, uint8_t na_flags){
	size_t                          na_packet_size;
	netpkt_t                        *pkt;
	fnet_nd6_na_header_t            *na_packet;
	fnet_nd6_option_lla_header_t    *nd_option_tlla;
	net_sockaddr_t                  src_addr;
	net_sockaddr_t                  dst_addr;

	na_packet_size = sizeof(fnet_nd6_na_header_t) + sizeof(fnet_nd6_option_header_t) + sizeof(mac_addr_t);
    
	if(! (pkt = netmem_alloc_pkt(na_packet_size)) ) return;

	na_packet = netpkt_data(pkt);
	na_packet->icmp6_header.type = FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT;
	na_packet->icmp6_header.code = 0u;
	
	/* NA header.*/
	na_packet->flag = na_flags;    /* Flag parameter.*/
	net_bzero( na_packet->_reserved, sizeof(na_packet->_reserved));  /* Set to zeros the reserved field.*/
	na_packet->target_addr = *src_ip;
	
	nd_option_tlla = (fnet_nd6_option_lla_header_t*)(&(na_packet[1]));
	nd_option_tlla->option_header.type = FNET_ND6_OPTION_TARGET_LLA; /* Type. */
	nd_option_tlla->option_header.length = (uint8_t)((sizeof(mac_addr_t) + sizeof(fnet_nd6_option_header_t)) >> 3); /* Option size devided by 8,*/
	
	/* Send ICMPv6 message.*/
        
	src_addr.type  = NET_SKA_IN6;
	src_addr.ip.v6 = *src_ip;
	dst_addr.type  = NET_SKA_IN6;
	dst_addr.ip.v6 = *dst_ip;
	
	neticmp6_output(nif,pkt, &src_addr, &dst_addr,255);
	return;
DROP:
	netpkt_free(pkt);
}

