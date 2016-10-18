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
#include <netif/driveri.h>

#include <netipv4/input.h>
#include <netipv6/input.h>
#include <netarp/input.h>

#include <netif/l2defs.h>

void netif_input_layer3 (netif_t* nif,netpkt_t* pkt,uint16_t protocol){
	switch(protocol){
	case NETPROT_L3_IPV4:
		netipv4_input( nif, pkt );
		return;
	case NETPROT_L3_ARP:
		netarp_input( nif, pkt );
		return;
	case NETPROT_L3_IPV6:
		netipv6_input( nif, pkt );
		return;
	}
	netpkt_free(pkt);
}

