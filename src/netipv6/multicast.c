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

#include <netipv6/multicast.h>
#include <netipv6/if.h>

/*
 * Join a IPv6 multicast group.
 * Return 0 on success, non-0 on error.
 */
int netipv6_multicast_join_prv(netif_t *netif, const ipv6_addr_t *ip_addr){
	netipv6_if_t *nif6;
	int i,freei;

	nif6 = netif->ipv6;

	freei = NETIPV6_IF_MULTCAST_MAX;
	for( i = 0 ; i < NETIPV6_IF_MULTCAST_MAX ; ++i ){
		if(! nif6->multicasts[i].used ){
			if(freei == NETIPV6_IF_MULTCAST_MAX) freei = i;
			continue;
		}
		if(! IP6ADDR_EQ(*ip_addr,nif6->multicasts[i].multicast) ) continue;
		
		/* Increment usage counter. */
		nif6->multicasts[i].refc++;
		
		/* MLDone already sent? */
		if( nif6->multicasts[i].mlddone ){
			nif6->multicasts[i].reported = 0;
			nif6->multicasts[i].mlddone = 0;
		}
		return 0;
	}
	
	/*
	 * No free entry?
	 */
	if( freei == NETIPV6_IF_MULTCAST_MAX) return -1;
	
	nif6->multicasts[freei].multicast = *ip_addr;
	nif6->multicasts[freei].refc = 1; /* Just created, so Reference counter will be 1. */
	nif6->multicasts[freei].used = 1;
	nif6->multicasts[freei].reported = 0;
	nif6->multicasts[freei].mlddone = 0;
	
	return 0;
}

/*
 * Leave a IPv6 multicast group.
 * Return 0 on success, non-0 on error.
 */
int netipv6_multicast_leave_prv(netif_t *netif, const ipv6_addr_t *ip_addr){
	netipv6_if_t *nif6;
	int i;

	nif6 = netif->ipv6;

	for( i = 0 ; i < NETIPV6_IF_MULTCAST_MAX ; ++i ){
		if( nif6->multicasts[i].used ) continue;
		if( IP6ADDR_EQ(*ip_addr,nif6->multicasts[i].multicast) ) break;
	}
	
	/* Not found? */
	if( i == NETIPV6_IF_MULTCAST_MAX) return -1;
	
	if(nif6->multicasts[i].refc)
		nif6->multicasts[i].refc--;
	return 0;
}


