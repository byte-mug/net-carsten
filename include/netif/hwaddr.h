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


#ifndef _NETIF_HWADDR_H_
#define _NETIF_HWADDR_H_

#include <netstd/stdint.h>

typedef struct hwaddr_s{
	uint8_t length;
	uint8_t buffer[8];
} hwaddr_t;

int netif_hwaddr_eq(const hwaddr_t* a,const hwaddr_t* b);

int netif_hwaddr_load(hwaddr_t* addr,const void* data);

int netif_hwaddr_store(const hwaddr_t* addr,void* data);

#endif

