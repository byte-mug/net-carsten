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

#include <netarp/input.h>
#include <netarp/table.h>
#include <netarp/arp_header.h>
#include <netif/ifapi.h>

#include <netstd/endianness.h>

void netarp_input( netif_t *netif, netpkt_t *pkt ){
	netpkt_t            *chain;
	netpkt_t            *tail;
	fnet_arp_header_t   *arp_hdr;
	ipv4_addr_t         sender_prot_addr;
	ipv4_addr_t         target_prot_addr;
	mac_addr_t          sender_hard_addr;
	char                create;
		
	/* The header must reside in contiguous area of memory. */
	if( netpkt_pullup(pkt,sizeof(fnet_arp_header_t)) ) goto DROP;
	
	arp_hdr = netpkt_data(pkt);
	
	sender_prot_addr = arp_hdr->sender_prot_addr;
	target_prot_addr = arp_hdr->target_prot_addr;
	sender_hard_addr = arp_hdr->sender_hard_addr;
	
	/* Check Duplicate IP address.*/
	if (!IP4ADDR_EQ(sender_prot_addr,netif->ipv4.address)){
		/*
		 * If the target protocol address is ours, we're going to create a new ARP
		 * cache entry. Otherwise we update it, if it exists.
		 */
		create = IP4ADDR_EQ(target_prot_addr,netif->ipv4.address) ? 1 : 0; /* It's for me. */
		
		/*
		 * Create or update ARP entry.
		 */
		chain = netarp_tab_update(netif,sender_prot_addr,sender_hard_addr,create);
		
		/*
		 * Send all network packets out to the 'sender_hard_addr'.
		 */
		netif->netif_class->ifapi_send_l2_all(netif,pkt,&sender_hard_addr);
	}else{
		// TODO: duplicate address detection.
	}
	
	/* ARP request. If it asked for our address, we send out a reply.*/
	if( (ntoh16(arp_hdr->op) == FNET_ARP_OP_REQUEST) && (IP4ADDR_EQ(target_prot_addr,netif->ipv4.address)) )
	{
		arp_hdr->op = hton16(FNET_ARP_OP_REPLY); /* Opcode */
		
		arp_hdr->target_hard_addr = sender_hard_addr;
		arp_hdr->sender_hard_addr = netif->device_mac;
		
		arp_hdr->target_prot_addr = arp_hdr->sender_prot_addr;
		arp_hdr->sender_prot_addr = netif->ipv4.address;
		
		netif->netif_class->ifapi_send_l2(netif,pkt,&sender_hard_addr);
		return;
	}
	
DROP:
	netpkt_free(pkt);
}

