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


#ifndef _NETPKT_PKT_H_
#define _NETPKT_PKT_H_

#include <netpkt/seg.h>
#include <netpkt/flags.h>

#define NETPKT_MAX_LEVELS 8

typedef struct netpkt{
	struct netpkt* next_chain;
	netpkt_seg_t*  segs;
	uint32_t       offset_length;
	uint32_t       offsets[NETPKT_MAX_LEVELS];
	uint16_t       flags;
	uint8_t        level;
} netpkt_t;

#define NETPKT_OFFSET(pkt) ( (pkt)->offsets[((pkt)->level)] )
#define NETPKT_LENGTH(pkt) ( (pkt)->offset_length - NETPKT_OFFSET(pkt) )

/*
 * Pulls up 'len' bytes to the current offset.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pullup(netpkt_t *pkt,size_t len);

/*
 * Pulls up 'len' bytes to the current offset, without copying data, so the
 * data between the current offset and (offset+'len') may be lost.
 *
 * This is usefull when a packet header is being prepended and and the
 * uninitialized data will be overwritten anyways.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pullup_lite(netpkt_t *pkt,size_t len);

/*
 * Pull in packet head. Decrease packet data length by removing data from the
 * head of the packet.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pullfront(netpkt_t *pkt,uint32_t len);

/*
 * Push out packet head. Increase packet data length by adding uninitialized
 * data to the head of the packet.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_pushfront(netpkt_t *pkt,uint32_t len);

/*
 * Sets the packet's length (relative to the offset).
 */
void netpkt_setlength(netpkt_t *pkt,uint32_t len);

/*
 * Raises the pkt->level variable by 1 and copies the offset from the old to
 * the new level.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_levelup(netpkt_t *pkt);

/*
 * Lowers the pkt->level variable by 1 and copies the offset from the old to
 * the new level.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_leveldown(netpkt_t *pkt);

/*
 * Depending on 'direction' this function performs the following task:
 *
 * 'direction < 0': It lowers the pkt->level variable by 1.
 *
 * Otherwise it raises the pkt->level variable by 1.
 *
 * On success it returns 0, non-0 otherwise.
 */
int netpkt_switchlevel(netpkt_t *pkt,int direction);

/*
 * Gets the Data pointer to the current offset.
 */
void *netpkt_data(netpkt_t *pkt);

/*
 * Frees an entire chain of network packets.
 */
void netpkt_free_all(netpkt_t *pkt);

/*
 * Frees a single network packet.
 */
void netpkt_free(netpkt_t *pkt);

#endif

