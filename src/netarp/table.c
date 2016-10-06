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
#include <netarp/table.h>
#include <netarp/if.h>

static int netarp_tab_find(netarp_if_t *arpif, ipv4_addr_t prot_addr, char create){
	int           i;
	
	/* Find an entry to update. */
	for (i = 0; i < NETARP_TABLE_SIZE; ++i){
		if(!( arpif->arp_table[i].used ))continue;
		/*
		 * Check if the source IP address of the incoming packet matches
		 * the IP address in this ARP table entry.
		 */
		if(! IP4ADDR_EQ(prot_addr,arpif->arp_table[i].prot_addr) )continue;
		
		return i;
	}
	
	if( !create ) return NETARP_TABLE_SIZE;
	
	/* If we get here, no existing ARP table entry was found. */
	
	/* Find an unused entry in the ARP table. */
	for (i = 0; i < NETARP_TABLE_SIZE; ++i){
		if(!( arpif->arp_table[i].used ))break;
	}
	
	/*
	 * TODO:
	 * If no unused entry is found, we try to find the oldest entry and throw it
	 * away.
	 */
	
	return i;
}

netpkt_t *netarp_tab_update( netif_t *netif, ipv4_addr_t prot_addr, mac_addr_t hard_addr, char create){
	netarp_if_t   *arpif;
	int           i;
	netpkt_t      *chain;
	
	arpif = netif->arp;
	
	i = netarp_tab_find(arpif,prot_addr,create);
	
	if(i==NETARP_TABLE_SIZE) return 0;
	
	chain = arpif->arp_table[i].hold;
	
	arpif->arp_table[i].hold = 0;
	
	return chain;
}


