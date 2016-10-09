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
#include <netsock/addr.h>
#include <neticmp6/output.h>
#include <netipv6/ctrl.h>
#include <netipv6/check.h>
#include <netipv6/defs.h>

#include <netmem/allocpkt.h>
#include <netstd/mem.h>


void netnd6_neighbor_advertisement_send(netif_t *nif, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip, uint8_t na_flags){
	size_t                          na_packet_size;
	size_t                          option_size;
	netpkt_t                        *pkt;
	fnet_nd6_na_header_t            *na_packet;
	fnet_nd6_option_lla_header_t    *nd_option_tlla;
	net_sockaddr_t                  src_addr;
	net_sockaddr_t                  dst_addr;

	/* The ND6_Option_Target_LLA size */
	option_size = sizeof(fnet_nd6_option_header_t) + NETIF_HWADDR_SIZE(nif);
	
	/* Pad the option size to be 8-aligned. */
	option_size +=7;
	option_size -= (option_size%8);
	
	/*
	 * Compute the entire packet size.
	 */
	na_packet_size = sizeof(fnet_nd6_na_header_t) + option_size;
	
	/*
	 * Allocate the new packet.
	 */
	if(! (pkt = netmem_alloc_pkt(na_packet_size)) ) return;

	na_packet = netpkt_data(pkt);
	na_packet->icmp6_header.type = FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT;
	na_packet->icmp6_header.code = 0u;
	
	/* NA header.*/
	na_packet->flag = na_flags;    /* Flag parameter.*/
	net_bzero( na_packet->_reserved, sizeof(na_packet->_reserved));     /* Set to zeros the reserved field.*/
	na_packet->target_addr = *src_ip;
	
	nd_option_tlla = (fnet_nd6_option_lla_header_t*)(&(na_packet[1]));
	nd_option_tlla->option_header.type = FNET_ND6_OPTION_TARGET_LLA;    /* Type. */
	nd_option_tlla->option_header.length = (uint8_t)(option_size >> 3); /* Option size devided by 8, rounded up.*/
	
	netif_hwaddr_store(&(nif->device_addr),nd_option_tlla->addr);       /* Store MAC address. */
	
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

void netnd6_neighbor_solicitation_send(netif_t *nif, ipv6_addr_t *src_ip /* NULL for, DAD */, ipv6_addr_t *dst_ip /*set for NUD,  NULL for DAD & AR */, ipv6_addr_t *targ_ip){
	size_t                          na_packet_size;
	size_t                          option_size;
	netpkt_t                        *pkt;
	fnet_nd6_ns_header_t            *ns_packet;
	fnet_nd6_option_lla_header_t    *nd_option_slla;
	net_sockaddr_t                  src_addr;
	net_sockaddr_t                  dst_addr;

	src_addr.type  = NET_SKA_IN6;
	//src_addr.ip.v6 = *src_ip;
	dst_addr.type  = NET_SKA_IN6;
	//dst_addr.ip.v6 = *dst_ip;

	/* The ND6_Option_Target_LLA size */
	option_size = sizeof(fnet_nd6_option_header_t) + NETIF_HWADDR_SIZE(nif);
	
	/* Pad the option size to be 8-aligned. */
	option_size +=7;
	option_size -= (option_size%8);
	
	/*
	 * Compute the entire packet size.
	 */
	na_packet_size = sizeof(fnet_nd6_ns_header_t) + option_size;
	
	/*
	 * Allocate the new packet.
	 */
	if(! (pkt = netmem_alloc_pkt(na_packet_size)) ) return;
	
	/*
         * Neighbor Solicitations are multicast when the node needs
         * to resolve an address and unicast when the node seeks to verify the
         * reachability of a neighbor.
         */
	if(! dst_ip ){
		/* AR, DAD */
		/* Generate Solicited Multicast destination address from the target address.*/
		netipv6_get_solicited_multicast_addr(targ_ip, &dst_addr.ip.v6);
	}else{
		/* NUD */
		dst_addr.ip.v6 = *dst_ip;
	}
	
	/*
	 * RFC4861: The link-layer address for the sender. MUST NOT be
	 * included when the source IP address is the
	 * unspecified address (DAD). Otherwise, on link layers
	 * that have addresses this option MUST be included in
	 * multicast solicitations and SHOULD be included in
	 * unicast solicitations.
	 */
	if( src_ip ){ /* AR or NUD */
		/*
		 * RFC4861 7.2.2: If the source address of the packet prompting the solicitation is the
		 * same as one of the addresses assigned to the outgoing interface, that
		 * address SHOULD be placed in the IP Source Address of the outgoing
		 * solicitation. Otherwise, any one of the addresses assigned to the
		 * interface should be used.
		 */
		if(!netipv6_addr_is_self(nif, src_ip, 0)){
			if(! netipv6_select_src_addr_nsol(nif,&src_addr.ip.v6,&dst_addr.ip.v6) )
				goto DROP; /* Just in case. Should never happen.*/
		}else{
			src_addr.ip.v6 = *src_ip;
		}
		/* Fill Source link-layer address option.*/
		nd_option_slla = (fnet_nd6_option_lla_header_t *)(&(ns_packet[1]));
		nd_option_slla->option_header.type = FNET_ND6_OPTION_SOURCE_LLA; /* Type. */
		nd_option_slla->option_header.length = (uint8_t)(option_size >> 3); /* Option size devided by 8,*/
		
		netif_hwaddr_store(&(nif->device_addr),nd_option_slla->addr);       /* Store MAC address. */
	}else{
		/* Source IP address is the
		 * unspecified address for DAD.
		 */
		src_addr.ip.v6 = (ipv6_addr_t)IP6_ADDR_ANY_INIT;
	}
	
	/* Fill ICMP Header */
	ns_packet                    = netpkt_data(pkt);
	ns_packet->icmp6_header.type = FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT;
	ns_packet->icmp6_header.code = 0u;
	
	/* Fill NS Header.*/
	net_bzero( ns_packet->_reserved, sizeof(ns_packet->_reserved));     /* Set to zeros the reserved field.*/
	ns_packet->target_addr       = *targ_ip;
	
	/* Send ICMPv6 message.*/
	neticmp6_output(nif,pkt, &src_addr, &dst_addr,255);
	return;
DROP:
	netpkt_free(pkt);
}

