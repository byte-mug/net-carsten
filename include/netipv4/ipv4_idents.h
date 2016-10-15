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


#ifndef _NETIPV4_IDENT_H_
#define _NETIPV4_IDENT_H_

#include <netipv4/ipv4.h>
#include <netif/if.h>

/*
 * Returns the next value for the ID field.
 */
uint32_t netipv4_next_id(netif_t *nif,ipv4_addr_t src,ipv4_addr_t dest);

#endif

