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
#include <netprot/output.h>
#include <netipv4/output.h>
#include <netipv6/output.h>
#include <netipv4/ipv4.h>
#include <netipv6/ipv6.h>
#include <netprot/checksum.h>
#include <netprot/opts.h>

static const struct netprot_opts np_defaults = {
	.tos = 0,
	.ttl = 255,
	.dont_fragment = 0,
	.dont_route = 0,
};

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
){
	/* ASSERT(src_addr->type == dst_addr->type); */
	
	if( !opts ) opts = & np_defaults;
	
	if( src_addr->type == 4 ){
		/* Pseudo checksum. */
		if( checksum ) *checksum = netprot_checksum_pseudo_end(
			*checksum,
			(uint8_t*)&(src_addr->ip.v4),
			(uint8_t*)&(dst_addr->ip.v4),
			sizeof(ipv4_addr_t)
			);
		netipv4_output(nif,pkt,src_addr,dst_addr,protocol,
			opts->tos,           // TOS
			opts->ttl,           // TTL
			opts->dont_fragment, // don't fragment
			opts->dont_route     // don't route
		);
	} else {
		/* Pseudo checksum. */
		if( checksum ) *checksum = netprot_checksum_pseudo_end(
			*checksum,
			(uint8_t*)&(src_addr->ip.v6),
			(uint8_t*)&(dst_addr->ip.v6),
			sizeof(ipv6_addr_t)
			);
		/* XXX Add IPv6 specific netprot_opts fields. */
		netipv6_output(nif,pkt,src_addr,dst_addr,protocol,
			opts->ttl,  // TTL
			0           // Traffic class
		);
	}
}

