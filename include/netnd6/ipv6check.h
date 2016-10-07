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


#ifndef _NETND6_IP6CHECK_H_
#define _NETND6_IP6CHECK_H_

#include <netpkt/pkt.h>

/*
 * Returns 0 if IPV6_HOP_LIMIT(pkt) == hoplimit; non-0 otherwise.
 */
int netnd6_check_hop_limit(netpkt_t *pkt,uint8_t hoplimit);

#endif

