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


#ifndef _NETSOCK_ADDR_H_
#define _NETSOCK_ADDR_H_

#include <netstd/stdint.h>
#include <netipv4/ipv4.h>
#include <netipv6/ipv6.h>

#define NET_SKA_IN  4
#define NET_SKA_IN6 6

typedef struct {
	union{
		ipv4_addr_t v4;
		ipv6_addr_t v6;
	} ip;
	uint16_t port;
	uint8_t type;
} net_sockaddr_t;


#endif

