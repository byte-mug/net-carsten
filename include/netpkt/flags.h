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


#ifndef _NETPKT_FLAGS_H_
#define _NETPKT_FLAGS_H_

#include <netpkt/seg.h>

#define NETPKT_MAX_LEVELS 8

#define NETPKT_FLAG_BROAD_L2      0x0001
#define NETPKT_FLAG_BROAD_L3      0x0002

/*
 * To protect against "hole-196", some wireless network interfaces may ban
 * Unicast IP packets encapsulated in a multicast Layer 2 frame.
 */
#define NETPKT_FLAG_NO_UNICAST_L3 0x0004

#endif

