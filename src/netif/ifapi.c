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

#include <netif/ifapi.h>
#include <netif/hwaddr.h>
#include <netif/l2defs.h>

#include <netipv6/ipv6.h>
#include <netipv6/check.h>
#include <netipv6/defs.h>

#include <netnd6/table.h>
#include <netnd6/send.h>


/************************************************************************
 * Ethernet Multicast Address
 ***********************************************************************/
/* RFC1112 6.4: An IP host group address is mapped to an Ethernet multicast address
 * by placing the low-order 23-bits of the IP address into the low-order
 * 23 bits of the Ethernet multicast address 01-00-5E-00-00-00 (hex).
 */
#if 0
#define FNET_ETH_MULTICAST_IP4_TO_MAC(ip4_addr, mac_addr)  \
    do{   \
        (mac_addr)[0] = 0x01U; \
        (mac_addr)[1] = 0x00U; \
        (mac_addr)[2] = 0x5EU; \
        (mac_addr)[3] = (fnet_uint8_t)(((fnet_uint8_t *)(&(ip4_addr)))[1] & 0x7FU); \
        (mac_addr)[4] = ((fnet_uint8_t *)(&(ip4_addr)))[2];  \
        (mac_addr)[5] = ((fnet_uint8_t *)(&(ip4_addr)))[3];  \
    }while(0)
#endif

/* For IPv6 */
#define FNET_ETH_MULTICAST_IP6_TO_MAC(ip6_addr, mac_addr)        \
    do{   \
        (mac_addr)[0] = 0x33U;               \
        (mac_addr)[1] = 0x33U;               \
        (mac_addr)[2] = (ip6_addr).addr[12]; \
        (mac_addr)[3] = (ip6_addr).addr[13]; \
        (mac_addr)[4] = (ip6_addr).addr[14]; \
        (mac_addr)[5] = (ip6_addr).addr[15];  \
    }while(0)




/**
 * @brief Default implementation of netif_api->ifapi_send_l2.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param addr  destination mac-address
 */
void netif_api_send_l2(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol){
	netpkt_free(pkt);
}

/**
 * @brief Default implementation of netif_api->ifapi_send_l2_all.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param addr  destination mac-address
 *
 * This function sends an entire chain of packets at once.
 */
void netif_send_l2_all(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol){
	netpkt_free_all(pkt);
}

/**
 * @brief Default implementation of netif_api->ifapi_send_l3_ipv4.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param pkt   destination IPv4-address (Pointer)
 */
void netif_api_send_l3_ipv4(netif_t* nif,netpkt_t* pkt,void* addr){
	netpkt_free(pkt);
}

static inline void netif_enqueue_chain(netpkt_t* pkt, netpkt_t* ll){
	while(pkt->next_chain) pkt = pkt->next_chain;
	pkt->next_chain = ll;
}

typedef void (*send2_t)(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol);

static void netif_api_send_l3_ipv6_gen(netif_t* nif,netpkt_t* pkt, void* srcaddr,void* addr, send2_t send2){
	hwaddr_t      hwaddr;
	ipv6_addr_t   ipaddr;
	ipv6_addr_t   ipsrc;
	
	ipaddr = *((ipv6_addr_t*)addr);
	
	/********************************************
	 * Multicast.
	 ********************************************/
	if (IP6_ADDR_IS_MULTICAST(ipaddr))
	{
		hwaddr.length = nif->device_addr.length;
		FNET_ETH_MULTICAST_IP6_TO_MAC(ipaddr, hwaddr.buffer);
		hwaddr.buffer[6] = 0xFF;
		hwaddr.buffer[7] = 0xFE;
	}
	/********************************************
	 * Unicast.
	 ********************************************/
	else{
		fnet_nd6_neighbor_entry_t *neighbor;
		char send_solicitation = 0;
		
		if(srcaddr)
			ipsrc  = *((ipv6_addr_t*)srcaddr);
		else
			/*
			 * We don't call netipv6_select_src_addr_nsol() here, because
			 * netnd6_neighbor_solicitation_send() is going to call it
			 * anyways!
			 *
			 * Instead, we call netipv6_select_src_addr_rsol() to quickly
			 * grab one of our IPv6 addresses, which will cause
			 * netnd6_neighbor_solicitation_send() to call
			 * netipv6_select_src_addr_nsol() for the source adddress!
			 */
			netipv6_select_src_addr_rsol(nif,&ipsrc);
		
		net_mutex_lock(nif->nd6->nd6_lock);
		/* Possible redirection.*/
		netnd6_redirect_table_get(nif, &ipaddr);
		
		/* Check Neigbor cache.*/
		neighbor = netnd6_neighbor_cache_get(nif, &ipaddr);
		
		/*
		 * RFC4861 7.2.2: When a node has a unicast packet to send to a neighbor, but does not
		 * know the neighbor's link-layer address, it performs address resolution.
		 * For multicast-capable interfaces, this entails creating a
		 * Neighbor Cache entry in the INCOMPLETE state and transmitting a
		 * Neighbor Solicitation message targeted at the neighbor. The
		 * solicitation is sent to the solicited-node multicast address
		 * corresponding to the target address.
		 */
		if(! neighbor ){
			/*
			 * RFC4861 7.2.Address resolution is performed only on addresses that are determined to be
			 * on-link and for which the sender does not know the corresponding link-layer address.
			 * Address resolution is never performed on multicast addresses.
			 */
			if( IP6_ADDR_IS_LINKLOCAL(ipaddr)
				|| netnd6_prefix_list_lookup(nif,&ipaddr) )
			/* Destimnation is ON local-link.*/
			{
				neighbor = netnd6_neighbor_cache_add2(nif, &ipaddr, 0, FNET_ND6_NEIGHBOR_STATE_INCOMPLETE);
				
				neighbor->state_time = net_timer_ms();
				neighbor->solicitation_send_counter = 0u;
				neighbor->solicitation_src_ip_addr = ipsrc;
				
				/* AR: Transmitting a Neighbor Solicitation message targeted at the neighbor.*/
				send_solicitation = 1;
			}
			else
			{
				/* Try to use the router, if exists.*/
				neighbor = netnd6_get_router(nif);
				
				if(! neighbor ) {
					net_mutex_unlock(nif->nd6->nd6_lock);
					netpkt_free_all(pkt);
					return;
				}
				ipaddr = neighbor->ip_addr;
			}
		}
		
		/* Link -layer address is not initialized. */
		if( (neighbor->state != FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
			&& (neighbor->ll_addr2.length == 0) )
		{
			neighbor->state = FNET_ND6_NEIGHBOR_STATE_INCOMPLETE;
			neighbor->state_time = net_timer_ms();
			neighbor->solicitation_send_counter = 0u;
			ipsrc = neighbor->solicitation_src_ip_addr;
			/* AR: Transmitting a Neighbor Solicitation message targeted at the neighbor.*/
			send_solicitation = 1;
		}
			
		if(neighbor->state == FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
		{
			netif_enqueue_chain(pkt,neighbor->waiting_pkts);
			neighbor->waiting_pkts = pkt;
			net_mutex_unlock(nif->nd6->nd6_lock);
			if( send_solicitation ) 
				netnd6_neighbor_solicitation_send(nif, &ipsrc, 0 /* NULL for AR */, &ipaddr);
			return;
		}
		
		if(neighbor->state == FNET_ND6_NEIGHBOR_STATE_STALE)
		/*
		 * RFC4861 7.3.3: The first time a node sends a packet to a neighbor whose entry is
		 * STALE, the sender changes the state to DELAY and sets a timer to
		 * expire in DELAY_FIRST_PROBE_TIME seconds.
		 */
		{
			neighbor->state = FNET_ND6_NEIGHBOR_STATE_DELAY;
			neighbor->state_time = net_timer_ms();
		}
		/* Get destination MAC/HW address.*/
		hwaddr = neighbor->ll_addr2;
		
		net_mutex_unlock(nif->nd6->nd6_lock);
		
		if( send_solicitation ) 
			netnd6_neighbor_solicitation_send(nif, &ipsrc, 0 /* NULL for AR */, &ipaddr);
	}
	
	send2(nif,pkt,(mac_addr_t*)(hwaddr.buffer),NETPROT_L3_IPV6);
}

/**
 * @brief Default implementation of netif_api->ifapi_send_l3_ipv6.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param pkt   destination IPv6-address (Pointer)
 */
void netif_api_send_l3_ipv6(netif_t* nif,netpkt_t* pkt,void* srcaddr,void* addr){
	pkt->next_chain = 0;
	netif_api_send_l3_ipv6_gen( nif, pkt, srcaddr, addr, nif->netif_class->ifapi_send_l2 );
}

/**
 * @brief Default implementation of netif_api->ifapi_send_l3_ipv6_all.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param pkt   destination IPv6-address (Pointer)
 *
 * This function sends an entire chain of packets at once.
 */
void netif_api_send_l3_ipv6_all(netif_t* nif,netpkt_t* pkt,void* srcaddr,void* addr){
	netif_api_send_l3_ipv6_gen( nif, pkt, srcaddr, addr, nif->netif_class->ifapi_send_l2_all );
}

