/*
 *   Copyright 2016 Simon Schmidt
 *   Copyright 2011-2016 by Andrey Butok. FNET Community.
 *   Copyright 2008-2010 by Andrey Butok. Freescale Semiconductor, Inc.
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

#include <netprot/checksum.h>
#include <netstd/endianness.h>

static uint32_t fnet_checksum_low(uint32_t sum, size_t length, const uint16_t *d_ptr)
{
    uint16_t   p_byte1;
    int32_t   current_length = (int32_t)length;

    while((current_length -= 32) >= 0)
    {
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
    }
    current_length += 32;

    while((current_length -= 8) >= 0)
    {
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
        sum += *d_ptr++;
    }
    current_length += 8;

    while((current_length -= 2) >= 0)
    {
        sum += *d_ptr++;
    }

    current_length += 2;
    if(current_length)
    {
        p_byte1 = ( ((const uint8_t*)d_ptr)[0] << 8 );
        sum += hton16(p_byte1);
    }
    return sum;
}

static uint32_t fnet_checksum_pkt(netpkt_t *pkt, size_t length){
	netpkt_seg_t *seg;
	size_t       sublen;
	uint32_t     sum;
	uint16_t     oddbuf;
	uint8_t      oddptr;
	
	sum = 0;
	
	seg = pkt->segs;
	
	while( seg && length ){
		sublen = NETPKT_SEG_LENGTH(seg);
		if( sublen > length ) sublen = length;
		if(oddptr){
			oddbuf |= ((const uint8_t*)seg->data_ptr)[0];
			sum += (uint32_t)hton16(oddbuf);
			sum = fnet_checksum_low(sum, (sublen-1)&~1 ,seg->data_ptr+1);
			oddptr = !(sublen & 1);
		}else{
			sum = fnet_checksum_low(sum, sublen&~1 ,seg->data_ptr);
			oddptr = sublen & 1;
		}
		oddbuf = ((const uint8_t*)(seg->data_end-1))[0] << 8;
		length -= sublen;
		/* Add in one accumulated carry (prevent integer overflow) */
		sum = (sum & 0xffffu) + (sum >> 16);
		seg = seg->next;
	}
	if(oddptr) sum += (uint32_t)hton16(oddbuf&0xff00u);
		
	return sum;
}

uint16_t netprot_checksum_buf(void* ptr, size_t len)
{
    uint32_t sum = fnet_checksum_low(0, len, ptr);

    /* Add potential carries - no branches. */

    sum += 0xffffU; /* + 0xffff acording to RFC1624*/

    /* Add accumulated carries */
    while ((sum >> 16) != 0u)
    {
        sum = (sum & 0xffffu) + (sum >> 16);
    }

    return (uint16_t)(0xffffu & ~sum);
}

uint16_t netprot_checksum(netpkt_t *pkt, size_t len)
{
    uint32_t sum = fnet_checksum_pkt(pkt, len);

    /* Add potential carries - no branches. */

    sum += 0xffffU; /* + 0xffff acording to RFC1624*/

    /* Add accumulated carries */
    while ((sum >> 16) != 0u)
    {
        sum = (sum & 0xffffu) + (sum >> 16);
    }

    return (uint16_t)(0xffffu & ~sum);
}

uint16_t netprot_checksum_pseudo_start( netpkt_t *pkt, uint8_t protocol, uint16_t protocol_len ){
	netpkt_seg_t *seg;
	const uint16_t *begin,*end;
	uint32_t sum;
	
	
	sum = fnet_checksum_pkt(pkt, (size_t)protocol_len);
	sum += (uint32_t)hton16((uint16_t)protocol);
	sum += (uint32_t)hton16(protocol_len);
	
	 sum += 0xffffu; /*  + 0xffff acording to RFC1624*/

	/* Add in accumulated carries */
	while ( (sum >> 16) != 0u) sum = (sum & 0xffffu) + (sum >> 16);
	return (uint16_t)(sum);
}

uint16_t netprot_checksum_pseudo_end( uint16_t sum_s, const uint8_t *ip_src, uint8_t *ip_dest, size_t addr_size )
{
    uint32_t sum = 0U;

    sum = sum_s;

    sum = fnet_checksum_low(sum, addr_size, (const uint16_t *)ip_src);
    sum = fnet_checksum_low(sum, addr_size, (const uint16_t *)ip_dest);

    sum += 0xffffU; /* Add in accumulated carries + 0xffff acording to RFC1624*/

    /* Add accumulated carries */
    while ( (sum >> 16) != 0u)
    {
        sum = (sum & 0xffffu) + (sum >> 16);
    }

    return (uint16_t)(0xffffu & ~sum);
}



