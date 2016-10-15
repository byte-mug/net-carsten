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

#include <netipv4/ipv4_idents.h>

static const uint32_t FNV_prime = 16777619U;
static const uint32_t FNV_basis = 2166136261U;

static inline uint32_t fnv1a_int(uint32_t hash,uint32_t data){
	hash ^= (data&0xff);
	hash *= FNV_prime;
	hash ^= (data >> 8);
	hash *= FNV_prime;
	hash ^= (data >> 16);
	hash *= FNV_prime;
	hash ^= (data >> 24);
	hash *= FNV_prime;
	return hash;
}

/*
 * Returns the next value for the ID field.
 */
uint32_t netipv4_next_id(netif_t *nif,ipv4_addr_t src,ipv4_addr_t dest){
	uint32_t hash = FNV_basis;
	hash = fnv1a_int(hash,(uint32_t)src);
	hash = fnv1a_int(hash,(uint32_t)dest);
	/* TODO atomic operation or locks. */
	uint32_t nextid = (nif->ipv4_id->table[hash&NETIPV4_ID_TAB_MASK]++);
	return (uint16_t)nextid;
}

