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


#ifndef _NETND6_PKTCHECK_H_
#define _NETND6_PKTCHECK_H_

#include <netpkt/pkt.h>

/*
 * RFC4861 - Validation: All included options have a length
 * that is greater than zero.
 *
 * Returns 0 if Validation passed, non-0 if not.
 */
int netnd6_check_options(netpkt_t *pkt);

#endif

