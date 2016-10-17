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


#ifndef _NETGRE_HEADER_H_
#define _NETGRE_HEADER_H_

#include <netstd/stdint.h>
#include <netstd/packing.h>

/*
 * <----------0-15--------------><-----------16-31---------->                             
 * +-+-+-+-+------------+-------+---------------------------+
 * |C| |K|S|  Reserved  |Version|      Protocol Type        |
 * +-+-+-+-+------------+-------+---------------------------+
 * |        Checksum (opt)      |      Reserved 1 (opt)     |
 * +----------------------------+---------------------------+
 * |                      Key (optional)                    |
 * +--------------------------------------------------------+
 * |              Sequence Number (optional)                |
 * +--------------------------------------------------------+
 *
 */
typedef struct NETSTD_PACKED {
	uint8_t flags;
	uint8_t version;
	uint16_t protocol_type;
	uint16_t checksum;
	uint16_t reserved1;
} netgre_header_t;

/* RFC2784 */
#define NETGRE_FLAGS_CHECKSUM 0x80

/* Other flags */
#define NETGRE_FLAGS_ROUTING  0x40
#define NETGRE_FLAGS_KEY      0x20
#define NETGRE_FLAGS_SEQUENCE 0x10

#define NETGRE_VERSION_MASK   0x07

#endif

