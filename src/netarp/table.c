/*
 *   Copyright 2016 Simon Schmidt
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

netpkt_t *netarp_tab_update( netif_t *netif, ipv4_addr_t prot_addr, mac_addr_t hard_addr, char create){
	netarp_if_t   *arpif;
	
	arpif = netif->arp;
	
	
	return 0;
}
