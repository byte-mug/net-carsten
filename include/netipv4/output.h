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


#ifndef _NETIPV4_INPUT_H_
#define _NETIPV4_INPUT_H_

#include <netif/if.h>
#include <netpkt/pkt.h>
#include <netsock/addr.h>

void netipv4_output(
	netif_t *nif,
	netpkt_t *pkt,
	net_sockaddr_t *src_addr,
	net_sockaddr_t *dst_addr,
	uint8_t protocol,
	uint8_t tos,
	uint8_t ttl,
	char DF,
	char do_not_route,
	uint16_t *checksum
);

#endif

