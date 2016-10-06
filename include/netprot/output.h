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


#ifndef _NETPROT_OUTPUT_H_
#define _NETPROT_OUTPUT_H_

#include <netif/if.h>
#include <netpkt/pkt.h>
#include <netsock/addr.h>

struct netprot_opts;

/**
 * @brief Submits an output packet to the IP stack.
 * @param nif       network interface
 * @param pkt       the packet (layer 4)
 * @param protocol  The layer 4 protocol (eg. TCP, UDP, UDP-lite, SCTP, etc...)
 * @param src_addr  The source address (local address)
 * @param dst_addr  The destination address (remote address)
 * @param checksum  A pointer to the Header Checksum
 */
void netprot_ip_output(
	netif_t *nif,
	netpkt_t *pkt,
	uint8_t protocol,
	net_sockaddr_t *src_addr,
	net_sockaddr_t *dst_addr,
	uint16_t *checksum,
	const struct netprot_opts *opts
);

#endif

