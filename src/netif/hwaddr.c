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

#include <netif/hwaddr.h>
#include <netstd/mem.h>

int netif_hwaddr_eq(const hwaddr_t* a,const hwaddr_t* b){
	if(a->length>8) return 0;
	if(a->length!=b->length) return 0;
	return !memcmp(a->buffer,b->buffer,a->length);
}

int netif_hwaddr_load(hwaddr_t* addr,const void* data){
	if(addr->length>8) return 0;
	memcpy(addr->buffer,data,addr->length);
	return 1;
}

int netif_hwaddr_store(const hwaddr_t* addr,void* data){
	if(addr->length>8) return 0;
	memcpy(data,addr->buffer,addr->length);
	return 1;
}