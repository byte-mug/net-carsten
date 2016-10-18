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
	int           i,j;
	net_time_t    cur_time, cur_diff, max_diff;
	
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
	 * If no unused entry is found, we try to find the oldest entry and throw it
	 * away.
	 */
	if(i == NETARP_TABLE_SIZE){
		cur_time = net_timer_ms();
		max_diff = 0;
		j = NETARP_TABLE_SIZE;
		for (i = 0; i < NETARP_TABLE_SIZE; ++i){
			cur_diff = cur_time - arpif->arp_table[i].cr_time;
			if(cur_diff > max_diff){
				max_diff = cur_diff;
				j = i;
			}
		}
		i = j;
	}
	
	return i;
}

netpkt_t *netarp_tab_update( netif_t *netif, ipv4_addr_t prot_addr, mac_addr_t hard_addr, char create){
	netarp_if_t   *arpif;
	int           i;
	netpkt_t      *chain;
	
	arpif = netif->arp;
	chain = 0;
	
	net_mutex_lock(arpif->arp_lock);
	
	i = netarp_tab_find(arpif,prot_addr,create);
	
	/*
	 * If there is no such entry, quit the function.
	 */
	if(i==NETARP_TABLE_SIZE) goto ENDFUNC;
	
	/*
	 * Update ARP entry.
	 */
	arpif->arp_table[i].hard_addr = hard_addr;
	arpif->arp_table[i].resolved = 1;
	
	/*
	 * Pull the ARP entry's send queue.
	 */
	chain = arpif->arp_table[i].hold;
	arpif->arp_table[i].hold = 0;
	arpif->arp_table[i].hold_time = 0;
	arpif->arp_table[i].cr_time = net_timer_ms();
	
ENDFUNC:
	net_mutex_unlock(arpif->arp_lock);
	
	return chain;
}

static int netarp_tab_create(netarp_if_t *arpif){
	int           i,j;
	net_time_t    cur_time, cur_diff, max_diff;
	
	/* Find an unused entry in the ARP table. */
	for (i = 0; i < NETARP_TABLE_SIZE; ++i){
		if(!( arpif->arp_table[i].used ))break;
	}
	
	/*
	 * If no unused entry is found, we try to find the oldest entry and throw it
	 * away.
	 */
	if(i == NETARP_TABLE_SIZE){
		cur_time = net_timer_ms();
		max_diff = 0;
		j = NETARP_TABLE_SIZE;
		for (i = 0; i < NETARP_TABLE_SIZE; ++i){
			cur_diff = cur_time - arpif->arp_table[i].cr_time;
			if(cur_diff > max_diff){
				max_diff = cur_diff;
				j = i;
			}
		}
		i = j;
	}
	
	return i;
}


int netarp_tab_lookup( netif_t *netif, ipv4_addr_t prot_addr, mac_addr_t *hard_addr, netpkt_t *pkt){
	int           i,ret;
	netarp_if_t   *arpif;
	netpkt_t      *chain;
	
	arpif = netif->arp;
	
	ret = 0;
	chain = 0;
	net_mutex_lock(arpif->arp_lock);
	
	/* Find an entry to update. */
	for (i = 0; i < NETARP_TABLE_SIZE; ++i){
		if(!( arpif->arp_table[i].used )) continue;
		if(!( arpif->arp_table[i].resolved )) continue;
		/*
		 * Check if the source IP address of the incoming packet matches
		 * the IP address in this ARP table entry.
		 */
		if(! IP4ADDR_EQ(prot_addr,arpif->arp_table[i].prot_addr) )continue;
		
		/*
		 * If the ARP entry was resolved, set RETURN=non-0!
		 */
		if( arpif->arp_table[i].resolved ) ret = -1;
		goto ENDFUNC;
	}
	
	i = netarp_tab_create(arpif);
	
	chain = arpif->arp_table[i].hold;
	arpif->arp_table[i].hold = 0;
	arpif->arp_table[i].prot_addr = prot_addr;
	arpif->arp_table[i].used      = 1;
	arpif->arp_table[i].resolved  = 0;
	arpif->arp_table[i].cr_time   = net_timer_ms();
	arpif->arp_table[i].hold_time = net_timer_ms();
	
ENDFUNC:
	if(ret) {
		/* We found an resolved ARP entry. */
		*hard_addr = arpif->arp_table[i].hard_addr;
	}else{
		/* An unresolved ARP entry was found or created. */
		pkt->next_chain = arpif->arp_table[i].hold;
		arpif->arp_table[i].hold = pkt;
	}
	
	net_mutex_unlock(arpif->arp_lock);
	
	/*
	 * Free the chain of the preempted ARP entry, if any.
	 */
	if(chain)
		netpkt_free_all(chain);
	
	return ret;
}

