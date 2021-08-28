/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.02
 *                           Shift-JIS
 *
 *      Copyright (C) 1995-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - arp.h
 *                 Address Resolution Protocol
 *
 * $Id: arp.h,v 1.6 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_ARP_H)
#define _ARP_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	u_short hard;
	u_short proto;
	u_char elen;
	u_char ilen;
	u_short op;
	ETHER_ADR seadr;
	u_char __siadr[4];
	ETHER_ADR teadr;
	u_char __tiadr[4];
} ARP_HDR;

#define ARP_SIADR(p)	(*(u_long *)(p)->__siadr)
#define ARP_TIADR(p)	(*(u_long *)(p)->__tiadr)

#define ARP_HARD_ETHER		0x0001

#define ARP_OP_REQUEST		0x0001
#define ARP_OP_REPLY		0x0002
#define ARP_OP_REQUEST_REV	0x0003
#define ARP_OP_REPLY_REV	0x0004

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define arp_ntoh(p)
#define arp_hton(p)
#else	/* BIG_ENDIAN */

#if defined(__mips__)
#define arp_ntoh(p)		( \
	(p)->hard = ntohs((p)->hard), \
	(p)->proto = ntohs((p)->proto), \
	(p)->op = ntohs((p)->op), \
	put4u(&ARP_SIADR(p), ntohl(get4u(&ARP_SIADR(p)))), \
	put4u(&ARP_TIADR(p), ntohl(get4u(&ARP_TIADR(p)))))

#define arp_hton(p)		( \
	(p)->hard = htons((p)->hard), \
	(p)->proto = htons((p)->proto), \
	(p)->op = htons((p)->op), \
	put4u(&ARP_SIADR(p), htonl(get4u(&ARP_SIADR(p)))), \
	put4u(&ARP_TIADR(p), htonl(get4u(&ARP_TIADR(p)))))

#else	/* __mips */

#define arp_ntoh(p)		((p)->hard = ntohs((p)->hard), \
				 (p)->proto = ntohs((p)->proto), \
				 (p)->op = ntohs((p)->op), \
				 ARP_SIADR(p) = ntohl(ARP_SIADR(p)), \
				 ARP_TIADR(p) = ntohl(ARP_TIADR(p)))

#define arp_hton(p)		((p)->hard = htons((p)->hard), \
				 (p)->proto = htons((p)->proto), \
				 (p)->op = htons((p)->op), \
				 ARP_SIADR(p) = htonl(ARP_SIADR(p)), \
				 ARP_TIADR(p) = htonl(ARP_TIADR(p)))

#endif	/* __mips */
#endif	/* BIG_ENDIAN */

#if defined(__cplusplus)
}
#endif

#endif	/* _ARP_H */
