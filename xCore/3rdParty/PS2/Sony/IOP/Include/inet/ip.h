/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.05
 *                           Shift-JIS
 *
 *      Copyright (C) 1995-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - ip.h
 *                      Internet Protocol
 *
 * $Id: ip.h,v 1.6 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_IP_H)
#define _IP_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
#if 0
#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
	u_char ver	: 4;	/* version */
	u_char hl	: 4;	/* header length */
#else	/* BIG_ENDIAN */
	u_char hl	: 4;	/* header length */
	u_char ver	: 4;	/* version */
#endif	/* BIG_ENDIAN */
#else
	u_char ver_hl;		/* version & header length */
#endif
	u_char tos;		/* type of service */
	u_short tl;		/* total length */
	u_short id;		/* identification */
	u_short off;		/* fragment flags & offset */
	u_char ttl;		/* time to live */
	u_char proto;		/* protocol */
	u_short chksum;		/* header checksum */
	u_long src;		/* source IP address */
	u_long dst;		/* destination IP address */
	u_char opts[0];		/* options (if any) */
} IP_HDR;

#define ip_get_ver(p)		(((p)->ver_hl >> 4) & 0xf)
#define ip_get_hl(p)		(((p)->ver_hl >> 0) & 0xf)
#define ip_set_ver_hl(p, ver, hl)	\
	((p)->ver_hl = ((ver) << 4) | (hl))

#define IP_VER			4

#define IP_TOS_DEFAULT		0

#define IP_OFF_MASK		0x1fff
#define IP_OFF_MORE		0x2000
#define IP_OFF_DONT		0x4000

#define IP_TTL_MAX		255
#define IP_TTL_DEFAULT		64

#define IP_PROTO_ICMP		1
#define IP_PROTO_IGMP		2
#define IP_PROTO_TCP		6
#define IP_PROTO_UDP		17

#define IP_OPT_COPIED		0x80
#define IP_OPT_CLASS		0x60
#define IP_OPT_NUMBER		0x1f

#define IP_OPT_EOL		0x00	/* End of option list */
#define IP_OPT_NOP		0x01	/* No operation */
#define IP_OPT_SECURITY		0x82	/* Security */
#define IP_OPT_LSRR		0x83	/* Loose source routing */
#define IP_OPT_SSRR		0x89	/* Strict source routing */
#define IP_OPT_RR		0x07	/* Record route */
#define IP_OPT_SATID		0x88	/* Stream ID */
#define IP_OPT_TS		0x44	/* Internet timestamp */
#define IP_OPT_RA		0x94	/* Router alert */

#define IP_MAX_WINDOW		0xffff

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define ip_ntoh(p)
#define ip_hton(p)
#else

#if defined(__mips__)
#define ip_ntoh(p)	( \
	(p)->tl = ntohs((p)->tl), \
	(p)->id = ntohs((p)->id), \
	(p)->off = ntohs((p)->off), \
	put4u(&(p)->src, ntohl(get4u(&(p)->src))), \
	put4u(&(p)->dst, ntohl(get4u(&(p)->dst))))

#define ip_hton(p)	( \
	(p)->tl = htons((p)->tl), \
	(p)->id = htons((p)->id), \
	(p)->off = htons((p)->off), \
	put4u(&(p)->src, htonl(get4u(&(p)->src))), \
	put4u(&(p)->dst, htonl(get4u(&(p)->dst))))
#else
#define ip_ntoh(p)	((p)->tl = ntohs((p)->tl), \
			 (p)->id = ntohs((p)->id), \
			 (p)->off = ntohs((p)->off), \
			 (p)->src = ntohl((p)->src), \
			 (p)->dst = ntohl((p)->dst))

#define ip_hton(p)	((p)->tl = htons((p)->tl), \
			 (p)->id = htons((p)->id), \
			 (p)->off = htons((p)->off), \
			 (p)->src = htonl((p)->src), \
			 (p)->dst = htonl((p)->dst))
#endif

#endif

#if defined(__cplusplus)
}
#endif

#endif	/* _IP_H */
