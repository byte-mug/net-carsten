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

#include <netipv6/check.h>
#include <netipv6/defs.h>
#include <netipv6/if.h>
#include <netpkt/flags.h>
#include <netstd/mem.h>

static const ipv6_addr_t   ip6_addr_any                      = IP6_ADDR_ANY_INIT;
static const ipv6_addr_t   ip6_addr_loopback                 = IP6_ADDR_LOOPBACK_INIT;
static const ipv6_addr_t   ip6_addr_nodelocal_allnodes       = IP6_ADDR_NODELOCAL_ALLNODES_INIT;
static const ipv6_addr_t   ip6_addr_linklocal_allnodes       = IP6_ADDR_LINKLOCAL_ALLNODES_INIT;
static const ipv6_addr_t   ip6_addr_linklocal_allrouters     = IP6_ADDR_LINKLOCAL_ALLROUTERS_INIT;
static const ipv6_addr_t   ip6_addr_linklocal_allv2routers   = IP6_ADDR_LINKLOCAL_ALLV2ROUTERS_INIT;
static const ipv6_addr_t   ip6_addr_linklocal_prefix         = IP6_ADDR_LINKLOCAL_PREFIX_INIT;

int netipv6_deactivated(netif_t *nif){
	if(!( nif->ipv6 )) return 1;
	return nif->ipv6->disabled;
}

/*
 * Returns non-0 if the address is directed at ourself.
 */
int netipv6_addr_is_self(netif_t *nif, ipv6_addr_t *addr, uint16_t pkt_flags){
	int i;
	netipv6_if_t* nif6;
	
	if( (nif->flags) & NETIF_IS_LOOPBACK ) return -1;
	
	if( IP6_ADDR_IS_MULTICAST(*addr) ){
		switch( IP6_ADDR_MULTICAST_SCOPE(*addr) ) {
		/*
		 * RFC 4291 2.7
		 * 
		 * Nodes must not originate a packet to a multicast address whose scop
		 * field contains the reserved value 0; if such a packet is received, it
		 * must be silently dropped.
		 */
		case 0:
		/*
		 * RFC 4291 - Errata ID: 3480
		 *
		 * Section 2.7 says: 
		 *  Interface-Local scope spans only a single interface on a node
		 *  and is useful only for loopback transmission of multicast.
		 * 
		 * It should say:
		 *  Interface-Local scope spans only a single interface on a node 
		 *  and is useful only for loopback transmission of multicast.
		 *  Packets with interface-local scope received from another node 
		 *  must be discarded.
		 *
		 * It should be explicitly stated that interface-local scoped multicast packets
		 * received from the link must be discarded.
		 * The BSD implementation currently does this, but not Linux.
		 * http://www.ietf.org/mail-archive/web/ipv6/current/msg17154.html 
		 */
		case 1:
			return 0;
		}
		return -1;
	}
	
	/*
	 * To protect from "hole-196" attacks, this packet may not be an L3 unicast, so drop.
	 */
	if( pkt_flags & NETPKT_FLAG_NO_UNICAST_L3 ) return 0;
	
	nif6 = nif->ipv6;
	
	for(i=0;i<NETIPV6_IF_ADDR_MAX;++i){
		/* Skip NOT_USED addresses. */
		if( !nif6->addrs[i].used ) continue;
		
		/* Match the current Network address. */
		if( IP6ADDR_EQ(*addr,nif6->addrs[i].address) ) return -1;
	}
	return 0;
}

int netipv6_addr_is_own_ip6_solicited_multicast(netif_t *nif, ipv6_addr_t *addr){
	// TODO: implement
	return 0;
}

struct netipv6_if_addr* netipv6_get_address_info(netif_t *nif, ipv6_addr_t *addr){
	int i;
	netipv6_if_t* nif6;
	
	nif6 = nif->ipv6;
	
	for(i=0;i<NETIPV6_IF_ADDR_MAX;++i){
		/* Skip NOT_USED addresses. */
		if( !nif6->addrs[i].used ) continue;
		
		/* Match the current Network address. */
		if( IP6ADDR_EQ(*addr,nif6->addrs[i].address) ) return &nif6->addrs[i];
	}
	return 0;
}

int netipv6_addr_pefix_cmp(const ipv6_addr_t *addr_1, const ipv6_addr_t *addr_2, size_t prefix_length)
{
	int     result;
	size_t  prefix_length_bytes = prefix_length >> 3;
	uint8_t prefix_bitmask = ( 0xff<<(prefix_length&7) )&0xff;
	
	return
		(prefix_length <= 128u) &&
		(memcmp(addr_1, addr_2, prefix_length_bytes) == 0) &&
		((prefix_length_bytes<16)?(
			(addr_1->addr[prefix_length_bytes] & prefix_bitmask) ==
			(addr_2->addr[prefix_length_bytes] & prefix_bitmask) ):1 );
}

