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
#include <netarp/output.h>
#include <netarp/arp_header.h>
#include <netif/mac.h>
#include <netif/ifapi.h>
#include <netpkt/pkt.h>
#include <netmem/allocpkt.h>

#include <netstd/endianness.h>

static const mac_addr_t mac_broadcast = {.mac={0xff,0xff,0xff,0xff,0xff,0xff}};
static const mac_addr_t mac_none = {.mac={0,0,0,0,0,0}};

void netarp_request( netif_t *netif, ipv4_addr_t ipaddr ){
	fnet_arp_header_t   *arp_hdr;
	mac_addr_t          sender_addr = mac_broadcast;
	netpkt_t            *pkt;
	
	if(! (pkt = netmem_alloc_pkt(sizeof(fnet_arp_header_t))) ) return;
	
	arp_hdr = netpkt_data(pkt);
	
	arp_hdr->hard_type = hton16(1);      /* The type of hardware address (=1 for Ethernet).*/
	arp_hdr->prot_type = hton16(0x0800); /* The type of protocol address (=0x0800 for IP). */
	arp_hdr->hard_size = 6;              /* The size in bytes of the hardware address (=6). */
	arp_hdr->prot_size = 4;              /* The size in bytes of the protocol address (=4). */
	
	arp_hdr->op = hton16(FNET_ARP_OP_REQUEST); /* Opcode. */
	
	arp_hdr->target_hard_addr = mac_none;
	arp_hdr->sender_hard_addr = netif->device_mac;
	
	arp_hdr->target_prot_addr = ipaddr;             /* Protocol address of target of this packet.*/
	arp_hdr->sender_prot_addr = netif->ipv4.address; /* Protocol address of sender of this packet.*/
	
	netif->netif_class->ifapi_send_l2(netif,pkt,&sender_addr);
}


