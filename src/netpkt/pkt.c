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


#include <netstd/mem.h>
#include <netpkt/pkt.h>

int netpkt_pullup(netpkt_t *pkt,size_t len){
	netpkt_seg_t *seg;
	netpkt_seg_t *seg2;
	uint32_t      offset,dend,P,T,L;
	
	offset = NETPKT_OFFSET(pkt);
	
	/*
	 * If ( len > NETPKT_LENGTH(pkt) ) then fail fast.
	 */
	if( ( offset + len ) > pkt->offset_length ) return -1;
	
	seg = pkt->segs;
	while( seg ){
		P = NETPKT_SEG_LENGTH(seg);

		/*
		 * When offset is in [0,P) then stop.
		 */
		if( P > offset ) break;

		offset -= P;
		seg = seg->next;
	}
	
	if( !seg ) return -1;
	
	/*
	 * 'dend' : data end (offset).
	 */
	dend = offset + len;
	
	if( dend <= P ) return 0; /* It is already continous, don't pull! */
	
	seg2 = seg->next;
	
	if( !seg2 ) return -1;
	
	T = NETPKT_SEG_TAILROOM(seg);
	
	/*
	 * Variable rededication: 'dend' is now 'How much bytes to copy'
	 */
	dend -= P;
	
	if( dend <= T ){
		L = NETPKT_SEG_LENGTH(seg2);
		
		if( dend > L ) return -1; /* Next segment too short. */
		
		memcpy(seg->data_end,seg2->data_ptr,dend);
		seg->data_end  += dend;
		seg2->data_ptr += dend;
	}else{
		/*
		 * Fallback:
		 *
		 * We are unable to pull-up the data, so we push it down to
		 * the next segment.
		 */
		dend = P - offset;
		
		L = NETPKT_SEG_HEADROOM(seg2);
		
		if( dend > L ) return -1; /* Not enough Headroom. */
		
		seg->data_end  -= dend;
		seg2->data_ptr -= dend;
		memcpy(seg2->data_ptr,seg->data_end,dend);
	}
	return 0;
}

int netpkt_pullfront(netpkt_t *pkt,uint32_t len){
	NETPKT_OFFSET(pkt) += len;
	return 0;
}

int netpkt_pushfront(netpkt_t *pkt,uint32_t len){
	if( NETPKT_OFFSET(pkt) < len ) return -1;
	NETPKT_OFFSET(pkt) -= len;
	return 0;
}

void netpkt_free_all(netpkt_t *pkt){
	netpkt_t *next;
	
	while( pkt ){
		next = pkt->next_chain;
		netpkt_free(pkt);
		pkt = next;
	}
}

void netpkt_free(netpkt_t *pkt){
	// TODO: free.
}

