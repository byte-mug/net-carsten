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



