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


#ifndef _NETPROT_DEFAULTS_H_
#define _NETPROT_DEFAULTS_H_

/************************************************************************
*    Definitions.
*************************************************************************/
#define IP_MAX_PACKET      (FNET_CFG_IP_MAX_PACKET)

#define IP_MAX_OPTIONS     (40) /* Maximum option field length */

/************************************************************************
*    IP implementation parameters.
*************************************************************************/
#define IP_VERSION         (4)   /* IP version */
#define IP_TTL_MAX         (255) /* maximum time to live */
#define IP_TTL_DEFAULT     (64)  /* default ttl, from RFC 1340 */

/************************************************************************
*    Supported protocols.
*************************************************************************/
#define IP_PROTOCOL_ICMP   (1)
#define IP_PROTOCOL_IGMP   (2)
#define IP_PROTOCOL_UDP    (17)
#define IP_PROTOCOL_TCP    (6)
#define IP_PROTOCOL_ICMP6  (58)

#endif
