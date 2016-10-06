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


#ifndef _NETARP_TABLE_H_
#define _NETARP_TABLE_H_

#include <netif/if.h>
#include <netpkt/pkt.h>
#include <netif/mac.h>
#include <netipv4/ipv4.h>

/*
 * Updates an ARP table entry. If 'create' is non-0 (TRUE), the entry is created if not yet existing.
 * 
 * If the ARP entry had one or more associated, unsent packets, it will return them as chain.
 */
netpkt_t *netarp_tab_update( netif_t *netif, ipv4_addr_t prot_addr, mac_addr_t hard_addr, char create);

#endif

