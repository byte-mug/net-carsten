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


#ifndef _NETARP_IF_H_
#define _NETARP_IF_H_

#include <netif/mac.h>
#include <netipv4/ipv4.h>

#define NETARP_TABLE_SIZE 16

typedef struct
{
	mac_addr_t  hard_addr;      /**< Hardware address.*/
	ipv4_addr_t prot_addr;      /**< Protocol address.*/
#if 0
	fnet_time_t     cr_time;        /**< Time of entry creation.*/
	fnet_netbuf_t   *hold;          /**< Last packet until resolved/timeout.*/
	fnet_time_t     hold_time;      /**< Time of the last request.*/
#endif
	unsigned    resolved : 1;
} fnet_arp_entry_t;

typedef struct netarp_if{
	fnet_arp_entry_t    arp_table[NETARP_TABLE_SIZE];   /* ARP cache table.*/
	ipv4_addr_t         arp_probe_ipaddr;               /* ARP probe address.*/
} netarp_if_t;

#endif

