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


#ifndef _NETSOCK_HASHTAB_H_
#define _NETSOCK_HASHTAB_H_

#include <netstd/stdint.h>
#include <netsock/flow.h>
#include <netstd/mutex.h>

/* 1 million entries! */
#define NETSOCK_HT_ENTRIES 0x100000

#define NETSOCK_HT_HASHMOD(x)  ((x)&(NETSOCK_HT_ENTRIES-1))

/* One slot for every port. */
#define NETSOCK_HT_PORTS 0x10000

typedef struct netsock_ht {
	netsock_flow_t*         ht_connections[NETSOCK_HT_ENTRIES];  /* TCP or UDP connections. Addressed by an address tuple. */
	netsock_flow_t*         ht_ports[NETSOCK_HT_PORTS];          /* Listening TCP or UDP ports. */
	net_mutex_t             bucket_locks[NETSOCK_HT_PORTS];      /* Bucket locks */
} netsock_ht_t;

/*
 * Looks up a Flow and increments it's reference count.
 */
netsock_flow_t* netsock_lookup_flow(netsock_ht_t* table, uint8_t protocol, const net_sockaddr_t *remote_a, const net_sockaddr_t *local_a);
netsock_flow_t* netsock_lookup_flow_port(netsock_ht_t* table, uint8_t protocol, const net_sockaddr_t *local_a);

void netsock_add_flow(netsock_ht_t* table, netsock_flow_t* flow);
void netsock_add_flow_port(netsock_ht_t* table, netsock_flow_t* flow);

void netsock_remove_flow(netsock_ht_t* table, netsock_flow_t* flow);

/*
 * Decrements the reference count of a Flow.
 */
void netsock_decr_flow(netsock_ht_t* table, netsock_flow_t* flow);

#endif

