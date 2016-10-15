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

/*
 * Pulls up 'len' bytes to the current offset.
 *
 * On success it returns 0, non-0 otherwise.
 */
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

/*
 * Pulls up 'len' bytes to the current offset, without copying data, so the
 * data between the current offset and (offset+'len') may be lost.
 *
 * This is usefull when a packet header is being prepended and and the
 * uninitialized data will be overwritten anyways.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pullup_lite(netpkt_t *pkt,size_t len){
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
	}
	return 0;
}

/*
 * Gets the Data pointer to the current offset.
 */
void *netpkt_data(netpkt_t *pkt){
	netpkt_seg_t *seg;
	uint32_t      offset,P;
	
	offset = NETPKT_OFFSET(pkt);
	
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
	
	if( seg ) return seg->data_ptr+offset;
	return (void*)0;
}

/*
 * Pull in packet head. Decrease packet data length by removing data from the
 * head of the packet.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pullfront(netpkt_t *pkt,uint32_t len){
	NETPKT_OFFSET(pkt) += len;
	return 0;
}

/*
 * Push out packet head. Increase packet data length by adding uninitialized
 * data to the head of the packet.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pushfront(netpkt_t *pkt,uint32_t len){
	if( NETPKT_OFFSET(pkt) < len ) return -1;
	NETPKT_OFFSET(pkt) -= len;
	return 0;
}

/*
 * Sets the packet's length (relative to the offset).
 *
 * On success it returns 0, non-0 otherwise.
 */
void netpkt_setlength(netpkt_t *pkt,uint32_t len){
	pkt->offset_length = len + NETPKT_OFFSET(pkt);
}

/*
 * Raises the pkt->level variable by 1 and copies the offset from the old to
 * the new level.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_levelup(netpkt_t *pkt){
	uint32_t offset;

	if(pkt->level >= NETPKT_MAX_LEVELS)return -1;
	offset = NETPKT_OFFSET(pkt);
	pkt->level++;
	NETPKT_OFFSET(pkt) = offset;

	return 0;
}

/*
 * Lowers the pkt->level variable by 1 and copies the offset from the old to
 * the new level.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_leveldown(netpkt_t *pkt){
	uint32_t offset;

	if( pkt->level == 0)return -1;
	offset = NETPKT_OFFSET(pkt);
	pkt->level--;
	NETPKT_OFFSET(pkt) = offset;

	return 0;
}

/*
 * Depending on 'direction' this function performs the following task:
 *
 * 'direction < 0': It lowers the pkt->level variable by 1.
 *
 * Otherwise it raises the pkt->level variable by 1.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_switchlevel(netpkt_t *pkt,int direction){
	if(direction<0){
		if( pkt->level == 0)return -1;
		pkt->level--;
	}else{
		if(pkt->level >= NETPKT_MAX_LEVELS)return -1;
		pkt->level++;
	}
	return 0;
}

/*
 * Frees an entire chain of network packets.
 */
void netpkt_free_all(netpkt_t *pkt){
	netpkt_t *next;
	
	while( pkt ){
		next = pkt->next_chain;
		netpkt_free(pkt);
		pkt = next;
	}
}

/*
 * Frees a single network packet.
 */
void netpkt_free(netpkt_t *pkt){
	// TODO: free.
}

