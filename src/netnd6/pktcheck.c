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

#include <netnd6/pktcheck.h>
#include <netnd6/nd6_header.h>

int netnd6_check_options(netpkt_t *pkt){
	int                       result = 0;
	fnet_nd6_option_header_t* nd_option;
	
	/*
	 * Levelup: Remember the current position, so we can rewind it.
	 */
	netpkt_levelup(pkt);
	
	while(NETPKT_LENGTH(pkt)){
		if( netpkt_pullup(pkt,sizeof(fnet_nd6_option_header_t)) ) goto DROP;
		
		nd_option = netpkt_data(pkt);
		
		/* Drop packet, if option invalid. */
		if(nd_option->length == 0u) goto DROP;
		
		if( netpkt_pullfront(pkt,((uint32_t)nd_option->length << 3) ) ) goto DROP;
	}
	

	/*
	 * Rewind the position.
	 */
	netpkt_switchlevel(pkt,-1);
	
	return 0;
	
DROP:
	/*
	 * Rewind the position.
	 */
	netpkt_switchlevel(pkt,-1);
	return -1;
}


