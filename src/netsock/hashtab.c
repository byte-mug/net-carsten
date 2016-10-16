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
#include <netsock/hashtab.h>

typedef const uint8_t* byteptr;

/*
 * FNV1a Hash function 32 bit - Constants
 */

static const uint32_t FNV_prime = 16777619U;
static const uint32_t FNV_basis = 2166136261U;

/*
 * FNV1a Hash function 32 bit - code
 */
static inline uint32_t fnv1a(uint32_t hash,const uint8_t* data,unsigned len){
	for(;len;len--,data++){
		hash ^= *data;
		hash *= FNV_prime;
	}
	return hash;
}
static inline uint32_t fnv1a_short(uint32_t hash,uint16_t data){
	hash ^= (data&0xff);
	hash *= FNV_prime;
	hash ^= (data >> 8);
	hash *= FNV_prime;
	return hash;
}

/*
 * Perform a hash on an address tuple.
 */
static uint32_t netsock_hash_tuple(uint8_t protocol, const net_sockaddr_t *remote_a, const net_sockaddr_t *local_a){
	uint32_t hash = FNV_basis;
	
	/* Hash the protocol byte. */
	hash ^= protocol;
	hash *= FNV_prime;
	
	switch(remote_a->type){
	case NET_SKA_IN:
		hash = fnv1a(hash,(byteptr)&(remote_a->ip.v4),sizeof(ipv4_addr_t));
		hash = fnv1a(hash,(byteptr)&(local_a->ip.v4),sizeof(ipv4_addr_t));
		break;
	case NET_SKA_IN6:
		hash = fnv1a(hash,(byteptr)&(remote_a->ip.v6),sizeof(ipv6_addr_t));
		hash = fnv1a(hash,(byteptr)&(local_a->ip.v6),sizeof(ipv6_addr_t));
		break;
	}
	hash = fnv1a_short(hash,remote_a->port);
	hash = fnv1a_short(hash,local_a->port);
	return hash;
}

/*
 * Compare two addresses.
 */
static int netsock_eq(const net_sockaddr_t *addr1,const net_sockaddr_t *addr2){
	if( (addr1->port) != (addr2->port) ) return 0;
	if( (addr1->type) != (addr2->type) ) return 0;
	switch(addr1->type){
	case NET_SKA_IN:
		return IP4ADDR_EQ(addr1->ip.v4,addr2->ip.v4);
	case NET_SKA_IN6:
		return IP6ADDR_EQ(addr1->ip.v6,addr2->ip.v6);
	}
	/*
	 * In the case of an unknown IP type, wo didn't hash, so, don't compare.
	 */
	return 1;
}

/*
 * Linked-list insertion.
 */
static void netsock_insert_at(netsock_flow_t** pos, netsock_flow_t* flow){
	/* Extract previous pointer. */
	netsock_flow_t* other = *pos;
	/* Perform insertion on Structure level. */
	flow->prev = pos;
	flow->tail = other;
	
	/* Set the other object's prevous reference pointer to our tail-field. */
	if(other) other->prev = &(flow->tail);
	
	/* Update the current position. */
	*pos = flow;
}

/*
 * Linked-list removal.
 */
static void netsock_remove_at(netsock_flow_t* flow){
	netsock_flow_t* next;
	
	/* Extract next element. */
	next = flow->tail;
	
	/* If non-null, set the next's prev field to ours.*/
	if(next) next->prev = flow->prev;
	
	/* Let the variable, pointed to by the prev-field to the next element. */
	*(flow->prev) = next;
}

/*
 * Looks up a Flow and increments it's reference count.
 */
netsock_flow_t* netsock_lookup_flow(netsock_ht_t* table, uint8_t protocol, const net_sockaddr_t *remote_a, const net_sockaddr_t *local_a){
	const uint32_t hash = netsock_hash_tuple(protocol,remote_a,local_a);
	const uint32_t idx = NETSOCK_HT_HASHMOD(hash);
	
	net_mutex_lock(table->bucket_locks[hash%NETSOCK_HT_PORTS]);
	netsock_flow_t* cur = table->ht_connections[idx];
	for(;cur;cur = cur->tail){
		if(cur->hash_a != hash) continue;
		if(cur->protocol != protocol) continue;
		if(!netsock_eq(&(cur->remote_a),remote_a)) continue;
		if(!netsock_eq(&(cur->local_a),local_a)) continue;
		cur->refc++;
		
		net_mutex_unlock(table->bucket_locks[hash%NETSOCK_HT_PORTS]);
		return cur;
	}
	net_mutex_unlock(table->bucket_locks[hash%NETSOCK_HT_PORTS]);
	return 0;
}
netsock_flow_t* netsock_lookup_flow_port(netsock_ht_t* table, uint8_t protocol, const net_sockaddr_t *local_a){
	const uint16_t idx = local_a->port;
	
	net_mutex_lock(table->bucket_locks[idx]);
	netsock_flow_t* cur = table->ht_ports[idx];
	for(;cur;cur = cur->tail){
		if(cur->hash_a != idx) continue;
		if(cur->protocol != protocol) continue;
		if(cur->local_a.type && !netsock_eq(&(cur->local_a),local_a)) continue;
		cur->refc++;
		
		net_mutex_unlock(table->bucket_locks[idx]);
		return cur;
	}
	net_mutex_unlock(table->bucket_locks[idx]);
	return 0;
}

void netsock_add_flow(netsock_ht_t* table, netsock_flow_t* flow){
	flow->hash_a = netsock_hash_tuple(flow->protocol,&(flow->remote_a),&(flow->local_a));
	flow->refc = 1;
	const uint32_t idx = NETSOCK_HT_HASHMOD(flow->hash_a);
	
	net_mutex_lock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
	netsock_insert_at(&(table->ht_connections[idx]),flow);
	net_mutex_unlock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
}
void netsock_add_flow_port(netsock_ht_t* table, netsock_flow_t* flow){
	flow->hash_a = (uint32_t)(flow->local_a.port);
	flow->refc = 1;
	const uint32_t idx = flow->hash_a;
	
	net_mutex_lock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
	netsock_insert_at(&(table->ht_ports[idx]),flow);
	net_mutex_unlock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
}

void netsock_remove_flow(netsock_ht_t* table, netsock_flow_t* flow){
	net_mutex_lock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
	netsock_remove_at(flow);
	flow->refc--;
	net_mutex_unlock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
	if(flow->refc==0) flow->freeflow(flow);
}

/*
 * Decrements the reference count of a Flow.
 */
void netsock_decr_flow(netsock_ht_t* table, netsock_flow_t* flow){
	net_mutex_lock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
	flow->refc--;
	net_mutex_unlock(table->bucket_locks[flow->hash_a%NETSOCK_HT_PORTS]);
	if(flow->refc==0) flow->freeflow(flow);
}
