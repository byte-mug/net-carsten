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


#ifndef _NETSOCK_FLOW_H_
#define _NETSOCK_FLOW_H_

#include <netstd/stdint.h>
#include <netsock/addr.h>

typedef struct netsock_flow {
	struct netsock_flow     *tail;    /* Next element in Linked list */
	struct netsock_flow     **prev;   /* A pointer to a reference to this object. */
	uint32_t                refc;     /* Reference count. */
	uint32_t                hash_a;   /* Precomputated address-tuple-hash for fast comparison. */
	net_sockaddr_t          remote_a; /* Remote address of this socket. */
	net_sockaddr_t          local_a;  /* Local address of this socket. */
	uint8_t                 protocol; /* Protocol ID. */
	void  (*freeflow)(struct netsock_flow* flow); /* Destructor. */
	void*                   instance; /* Protocol specific data. */
} netsock_flow_t;

#endif

