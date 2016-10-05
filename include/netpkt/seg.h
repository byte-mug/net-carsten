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


#ifndef _NETPKT_SEG_H_
#define _NETPKT_SEG_H_

#include <netstd/stdint.h>

/*
 * <--------------- Segment Buffer --------------->
 *
 * +----------------------------------------------+
 * | Headroom...  | Segment Data... | Tailroom... |
 * +----------------------------------------------+
 * ^ data         ^ data_ptr        ^ data_end    ^ datalimit
 */

typedef struct netpkt_seg{
	struct netpkt_seg* next;
	
	void*  data;       /* < Pointer to the beginning of the Buffer. */
	void*  data_ptr;   /* < Pointer to the beginning of the data. */
	void*  data_end;   /* < Pointer to the end of the data. */
	void*  datalimit;  /* < Pointer to the end of the Buffer. */
} netpkt_seg_t;

#define NETPKT_SEG_HEADROOM(seg) ((size_t)( (seg)->data_ptr - (seg)->data ))

#define NETPKT_SEG_LENGTH(seg) ((size_t)( (seg)->data_end - (seg)->data_ptr ))

#define NETPKT_SEG_TAILROOM(seg) ((size_t)( (seg)->datalimit - (seg)->data_end ))

#endif

