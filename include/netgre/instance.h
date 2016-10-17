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


#ifndef _NETGRE_INSTANCE_H_
#define _NETGRE_INSTANCE_H_

#include <netif/if.h>
#include <netvnic/vnic.h>
#include <netsock/addr.h>

typedef struct netgre_inst{
	netvnic_t *vnic;
	netif_t   *nif;
	
	/*
	 * Address tuple for outbound/egress packets.
	 */
	net_sockaddr_t local_addr;
	net_sockaddr_t remote_addr;
} netgre_inst_t;

#endif

