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
#include <netipv6/ctrl.h>
#include <netipv6/check.h>
#include <netipv6/defs.h>
#include <netstd/mem.h>
#include <netif/mac.h>

#define IPV6_PREFIX_LENGTH_DEFAULT       (64U)            /* Default prefix length, in bits.*/

int netipv6_set_ip6_addr_autoconf(netif_t *netif, ipv6_addr_t *ip_addr)
{
	int result = 0;
	uint8_t *hw_addr;
	
	hw_addr = netif->device_addr.buffer;
	
	{
		/* Build Interface identifier.*/
		/* Set the 8 last bytes of the IP address based on the Layer 2 identifier.*/
		switch(netif->device_addr.length)
		{
		case 6: /* IEEE 48-bit MAC addresses. */
			memcpy(&(ip_addr->addr[8]), hw_addr,  3u);
			ip_addr->addr[11] = 0xffu;
			ip_addr->addr[12] = 0xfeu;
			memcpy(&(ip_addr->addr[13]), hw_addr+3,  3u);
			ip_addr->addr[8] ^= 0x02u;

			break;
		case 8: /* IEEE EUI-64 identifier.*/
			memcpy(&(ip_addr->addr[8]), hw_addr,  8u);
			ip_addr->addr[8] ^= 0x02u;
			break;
		/* TBD for others.*/
		default:
			result = -1;
			break;
		}
	}
	return result;
}

/************************************************************************
* DESCRIPTION: This function binds the IPv6 address to a hardware interface.
*************************************************************************/

int netipv6_bind_addr_prv(
	netif_t *nif,
	const ipv6_addr_t *addr,
	fnet_netif_ip_addr_type_t addr_type,
	net_time_t lifetime /*in seconds*/,
	size_t prefix_length /* bits */
){
	int               result = -1;
	int               i;
	netipv6_if_addr_t *if_addr_ptr = 0;
	netipv6_if_t      *ipv6 = nif->ipv6;
	
	if( IP6_ADDR_IS_MULTICAST(*addr) ) return -1;
	
	/* Find free address entry. */
	for(i = 0u; i < NETIPV6_IF_ADDR_MAX; i++)
	{
		if(! ipv6->addrs[i].used )
		{
			if_addr_ptr = &ipv6->addrs[i];
			break; /* Found free entry.*/
		}
	}
	
	if(if_addr_ptr){
		/* Copying address. */
		if_addr_ptr->address = *addr;
		
		/* If the address is zero => make it link-local.*/
		if(IP6_ADDR_IS_UNSPECIFIED(if_addr_ptr->address))
		{
			/* Set link-local address. */
			if_addr_ptr->address.addr[0] = 0xFEu;
			if_addr_ptr->address.addr[1] = 0x80u;
		}
		
		if_addr_ptr->type = addr_type; /* Set type.*/
		
		/* If we are doing Autoconfiguration, the ip_addr points to prefix.*/
		if(addr_type == FNET_NETIF_IP_ADDR_TYPE_AUTOCONFIGURABLE)
		{
			/* Construct address from prefix and interface id. */
			if((prefix_length != IPV6_PREFIX_LENGTH_DEFAULT)||
			netipv6_set_ip6_addr_autoconf(nif,&if_addr_ptr->address)
			)
			{
			goto COMPLETE;
			}
		}
		
		/* Check if addr already exists. Do it here to cover Autoconfiguration case.*/
		if(netipv6_get_address_info(nif, &if_addr_ptr->address))
		{
			/* The address is already bound.*/
			result = 0;
			goto COMPLETE;
		}
		
		/* Save creation time, in seconds.*/
		if_addr_ptr->creation_time = net_timer_seconds();
		
		/* Set lifetime, in seconds.*/
		if_addr_ptr->lifetime = lifetime;
		
		/* If supports ND6. */
		if(nif->nd6){
			/*
			 * An address on which the Duplicate Address Detection procedure is
			 * applied is said to be tentative until the procedure has completed
			 * successfully.
			 */
			if_addr_ptr->state = FNET_NETIF_IP6_ADDR_STATE_TENTATIVE;
			
			/* Get&Set the solicited-node multicast group-address for assigned ip_addr. */
			//fnet_ip6_get_solicited_multicast_addr(&if_addr_ptr->address, &if_addr_ptr->solicited_multicast_addr);
			
			/*************************************************************************
			 * Join Multicast ADDRESSES.
			 * When a multicast-capable interface becomes enabled, the node MUST
			 * join the all-nodes multicast address on that interface, as well as
			 * the solicited-node multicast address corresponding to each of the IP
			 * addresses assigned to the interface.
			 **************************************************************************/
			/* Join solicited multicast address group.*/
			//fnet_ip6_multicast_join(netif, &if_addr_ptr->solicited_multicast_addr);
			
			/* Start Duplicate Address Detection (DAD).
			 * RFC4862:  The Duplicate Address Detection algorithm is performed on all addresses,
			 * independently of whether they are obtained via stateless
			 * autoconfiguration or DHCPv6.
			 */
			//fnet_nd6_dad_start(netif , if_addr_ptr);
			// TODO: implement!
		}
		else
		{
			if_addr_ptr->state = FNET_NETIF_IP6_ADDR_STATE_PREFERRED;
		}
		result = 0;
	}
COMPLETE:
	return result;
}

int netipv6_unbind_addr_prv ( netif_t *nif, netipv6_if_addr_t *if_addr) {
	if(! (nif && if_addr && (if_addr->used) ) ) return -1;
	
	/* Leave Multicast group.*/
	//fnet_ip6_multicast_leave(netif, &if_addr->solicited_multicast_addr);
	
	/* Mark as Not Used.*/
	if_addr->used = 0;
	return 0;
}

