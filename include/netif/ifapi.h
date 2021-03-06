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


#ifndef _NETIF_IFAPI_H_
#define _NETIF_IFAPI_H_

#include <netpkt/pkt.h>
#include <netif/if.h>
#include <netif/mac.h>

struct netif_api{
	void (*ifapi_send_l2)(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol);
	void (*ifapi_send_l2_all)(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol);
	void (*ifapi_send_l3_ipv4)(netif_t* nif,netpkt_t* pkt,void* addr);
	void (*ifapi_send_l3_ipv6)(netif_t* nif,netpkt_t* pkt,void* srcaddr,void* addr);
	void (*ifapi_send_l3_ipv6_all)(netif_t* nif,netpkt_t* pkt,void* srcaddr,void* addr);
};

/**
 * @brief Default implementation of netif_api->ifapi_send_l2.
 * @param nif       netif-instance
 * @param pkt       network packet
 * @param addr      destination mac-address
 * @param protocol  the 'EtherType' of the L3 protocol
 */
void netif_api_send_l2(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol);

/**
 * @brief Default implementation of netif_api->ifapi_send_l2_all.
 * @param nif       netif-instance
 * @param pkt       network packet
 * @param addr      destination mac-address
 * @param protocol  the 'EtherType' of the L3 protocol
 *
 * This function sends an entire chain of packets at once.
 */
void netif_send_l2_all(netif_t* nif,netpkt_t* pkt,mac_addr_t* addr,uint16_t protocol);

/**
 * @brief Default implementation of netif_api->ifapi_send_l3_ipv4.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param pkt   destination IPv4-address (Pointer)
 */
void netif_api_send_l3_ipv4(netif_t* nif,netpkt_t* pkt,void* addr);

/**
 * @brief Default implementation of netif_api->ifapi_send_l3_ipv6.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param pkt   destination IPv6-address (Pointer)
 */
void netif_api_send_l3_ipv6(netif_t* nif,netpkt_t* pkt,void* srcaddr,void* addr);

/**
 * @brief Default implementation of netif_api->ifapi_send_l3_ipv6_all.
 * @param nif   netif-instance
 * @param pkt   network packet
 * @param pkt   destination IPv6-address (Pointer)
 *
 * This function sends an entire chain of packets at once.
 */
void netif_api_send_l3_ipv6_all(netif_t* nif,netpkt_t* pkt,void* srcaddr,void* addr);

#endif



