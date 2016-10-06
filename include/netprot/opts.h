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


#ifndef _NETPROT_OPTS_H_
#define _NETPROT_OPTS_H_

#include <netstd/stdint.h>

typedef struct netprot_opts {
	uint8_t  tos; /* IPv4 TOS */
	uint8_t  ttl; /* IPv4 TTL */
	
	uint8_t  traf_cls;  /* IPv6 traffic class */
	uint8_t  hop_limit; /* IPv6 hop limit (TTL) */
	
	unsigned dont_fragment : 1;
	unsigned dont_route : 1;
} netprot_opts_t;



#endif

