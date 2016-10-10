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


#ifndef _NETIF_IF_H_
#define _NETIF_IF_H_

#include <netstd/stdint.h>
#include <netipv4/ipv4.h>
#include <netif/mac.h>
#include <netif/hwaddr.h>

#define NETIF_IS_LOOPBACK 0x01

struct netif_api;
struct netipv6_if;
struct netarp_if;
struct netnd6_if;
struct netsock_ht;

typedef const struct netif_api* netif_api_v;

typedef struct netif{
	void         *netif_inst;
	netif_api_v  netif_class;
	size_t       netif_mtu;
	
	struct netif *next;
	
	/* IPv4 specific. */
	struct{
		ipv4_addr_t address;
		ipv4_addr_t netbroadcast;
		ipv4_addr_t subnetbroadcast;
		ipv4_addr_t subnet;
		ipv4_addr_t subnetmask;
		ipv4_addr_t gateway;
	}ipv4;
	volatile uint32_t ipv4_id;
	
	struct netipv6_if *ipv6;
	
	struct netarp_if  *arp;
	
	struct netnd6_if  *nd6;
	
	struct netsock_ht *udp;
	
	struct netsock_ht *tcp;
	
	/* Device specific. */
	mac_addr_t device_mac;
	hwaddr_t   device_addr;
	uint8_t flags;
} netif_t;

#define NETIF_HWADDR_SIZE(nif) ((nif)->device_addr.length)

#endif

