/*
 *   Copyright 2016 Simon Schmidt
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
#include <netipv4/check.h>

#include <netipv4/defs.h>

int netipv4_addr_is_broadcast(netif_t *nif,ipv4_addr_t addr){
	
	if(
		IP4ADDR_EQ(addr,IP4_ADDR_BROADCAST)|| /* Limited broadcast */
		IP4ADDR_EQ(addr,0)||
		IP4ADDR_EQ(addr,IP4_ADDR_LINK_LOCAL_BROADCAST) /* Link-local broadcast (RFC3927)*/
	) return 1;
	if( !nif ) return 0; /* Null-Pointer check */
	if(
		IP4ADDR_EQ(addr,nif->ipv4.netbroadcast)||
		IP4ADDR_EQ(addr,nif->ipv4.subnetbroadcast)||
		IP4ADDR_EQ(addr,nif->ipv4.subnet)
	) return 1;
	return 0;
}


