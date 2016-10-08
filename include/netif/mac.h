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


#ifndef _NETIF_MAC_H_
#define _NETIF_MAC_H_

#include <netstd/stdint.h>

typedef struct mac_addr{
	uint8_t mac[6];
} mac_addr_t;

#define NETIF_MACADDR_EQ(a,b) (\
	((a).mac[0] == (b).mac[0])&&\
	((a).mac[1] == (b).mac[1])&&\
	((a).mac[2] == (b).mac[2])&&\
	((a).mac[3] == (b).mac[3])&&\
	((a).mac[4] == (b).mac[4])&&\
	((a).mac[5] == (b).mac[5])   )

#endif

