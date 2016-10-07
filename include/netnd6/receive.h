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


#ifndef _NETND6_RECEIVE_H_
#define _NETND6_RECEIVE_H_

#include <netif/if.h>
#include <netpkt/pkt.h>
#include <netipv6/ipv6.h>

void netnd6_neighbor_solicitation_reveive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip);
void netnd6_neighbor_advertisement_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip);
void netnd6_router_advertisement_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip);
void netnd6_redirect_receive(netif_t *nif,netpkt_t *pkt, ipv6_addr_t *src_ip, ipv6_addr_t *dst_ip);

#endif

