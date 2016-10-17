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


#ifndef _NETVNIC_VNIC_H_
#define _NETVNIC_VNIC_H_

#include <netstd/stdint.h>
#include <netpkt/pkt.h>
#include <netif/hwaddr.h>

#define NETVNIC_FLAGS_HAS_HWADDR  0x01

/* Note: Yet experimental API. */

typedef struct netvnic{
	void           *vnic_in_inst;
	void           *vnic_out_inst;
	
	void (*vnic_input) (struct netvnic* vnic,netpkt_t* pkt,uint16_t encap_proto);
	void (*vnic_output) (struct netvnic* vnic,netpkt_t* pkt,hwaddr_t* dst);
	
	hwaddr_t       vnic_hwaddr;
	uint8_t        flags;
} netvnic_t;

#endif

