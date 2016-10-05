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
#pragma once


#ifdef __HOST_ENDIAN_BIG
/* Host byte order === Network byte order */

#define __HAS_HTON__

#define hton16(i) (i)
#define ntoh16(i) (i)
#define hton32(i) (i)
#define ntoh32(i) (i)

#endif /* HOST_ENDIAN_BIG */




#if defined(__HAS_BSWAP__) && !defined(__HAS_HTON__)
/*
 *     Host byte order =!= Network byte order
 * AND we have fast Byteswap primitives (GCC).
 */

#include <byteswap.h>

#define __HAS_HTON__

#define hton16(i) __bswap_16(i)
#define ntoh16(i) __bswap_16(i)
#define hton32(i) __bswap_32(i)
#define ntoh32(i) __bswap_32(i)

#endif /* HAS_BSWAP */


#ifndef __HAS_HTON__

/* Host byte order =!= Network byte order */

inline uint16_t hton16(uint16_t t){
	return (t>>8) | (t<<8);
}

inline uint16_t ntoh16(uint16_t t){
	return (t>>8) | (t<<8);
}

inline uint32_t hton32(uint32_t t){
	return ((t>>24)&0xff) | ((t<<8)&0xff0000) | ((t>>8)&0xff00) | ((t<<24)&0xff000000);
}

inline uint32_t ntoh32(uint32_t t){
	return ((t>>24)&0xff) | ((t<<8)&0xff0000) | ((t>>8)&0xff00) | ((t<<24)&0xff000000);
}

#endif /* __HAS_HTON__ */

