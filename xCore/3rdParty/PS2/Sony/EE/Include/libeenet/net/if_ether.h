/*	$NetBSD: if_ether.h,v 1.17.2.2 2001/06/07 17:00:01 he Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if_ether.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _NET_IF_ETHER_H_
#define _NET_IF_ETHER_H_

/*
 * Some basic Ethernet constants.
 */
#define	ETHER_ADDR_LEN	6	/* length of an Ethernet address */
#define	ETHER_TYPE_LEN	2	/* length of the Ethernet type field */
#define	ETHER_CRC_LEN	4	/* length of the Ethernet CRC */
#define	ETHER_HDR_LEN	((ETHER_ADDR_LEN * 2) + ETHER_TYPE_LEN)
#define	ETHER_MIN_LEN	64	/* minimum frame length, including CRC */
#define	ETHER_MAX_LEN	1518	/* maximum frame length, including CRC */

/*
 * Some Ethernet extensions.
 */
#define	ETHER_VLAN_ENCAP_LEN 4	/* length of 802.1Q VLAN encapsulation */

/*
 * Ethernet address - 6 octets
 * this is only used by the ethers(3) functions.
 */

#if defined(__GNUC__)
#define SCE_EENET_GCC_PACKED  __attribute__((packed))
#endif  /* defined(__GNUC__) */
#if defined(__MWERKS__)
#define SCE_EENET_GCC_PACKED
#pragma options align=packed
#endif  /* defined(__MWERKS__) */

struct sceEENetEtherAddr {
	u_int8_t ether_addr_octet[ETHER_ADDR_LEN];
} SCE_EENET_GCC_PACKED;

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct	sceEENetEtherHeader {
	u_int8_t  ether_dhost[ETHER_ADDR_LEN];
	u_int8_t  ether_shost[ETHER_ADDR_LEN];
	u_int16_t ether_type;
} SCE_EENET_GCC_PACKED;

#if defined(__MWERKS__)
#pragma options align=reset
#endif  /* defined(__MWERKS__) */
#undef SCE_EENET_GCC_PACKED

#include <net/ethertypes.h>

#define	ETHER_IS_MULTICAST(addr) (*(addr) & 0x01) /* is address mcast/bcast? */

#define	ETHERMTU	(ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define	ETHERMIN	(ETHER_MIN_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

/*
 * Prototype ethers(3) functions.
 */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

char *	sceEENetEtherNtoa (struct sceEENetEtherAddr *);
struct sceEENetEtherAddr *
	sceEENetEtherAton (const char *);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _NET_IF_ETHER_H_ */
