/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.04
 *                           Shift-JIS
 *
 *      Copyright (C) 1995-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - tcp.h
 *                  Transmission Control Protocol
 *
 * $Id: tcp.h,v 1.7 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_TCP_H)
#define _TCP_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	u_short sport;		/* source port */
	u_short dport;		/* destination port */
	u_long seq;		/* sequence number */
	u_long ack;		/* acknowledgement number */
#if 0
#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
	u_char off	: 4;	/* data offset */
	u_char reserved	: 4;	/* reserved */
#else	/* BIG_ENDIAN */
	u_char reserved	: 4;	/* reserved */
	u_char off	: 4;	/* data offset */
#endif
#else
	u_char off_reserved;	/* bit7-4:data offset bit3-0:reserved */
#endif
	u_char flags;		/* control bits */
	u_short win;		/* window */
	u_short chksum;		/* checksum */
	u_short urp;		/* urgent pointer */
	u_char opts[0];		/* options */
} TCP_HDR;

#define tcp_get_off(p)		((p)->off_reserved >> 4)
#define tcp_set_off(p, off)	((p)->off_reserved = (off) << 4)

#define TH_FIN		0x01
#define TH_SYN		0x02
#define TH_RST		0x04
#define TH_PSH		0x08
#define TH_ACK		0x10
#define TH_URG		0x20

#define THO_END		0x00
#define THO_NOP		0x01
#define THO_MSS		0x02

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define tcp_ntoh(p)
#define tcp_hton(p)
#else

#if defined(__mips__)
#define tcp_ntoh(p)	( \
	(p)->sport = ntohs((p)->sport), \
	(p)->dport = ntohs((p)->dport), \
	put4u(&(p)->seq, ntohl(get4u(&(p)->seq))), \
	put4u(&(p)->ack, ntohl(get4u(&(p)->ack))), \
	(p)->win = ntohs((p)->win), \
	(p)->urp = ntohs((p)->urp))

#define tcp_hton(p)	( \
	(p)->sport = htons((p)->sport), \
	(p)->dport = htons((p)->dport), \
	put4u(&(p)->seq, htonl(get4u(&(p)->seq))), \
	put4u(&(p)->ack, htonl(get4u(&(p)->ack))), \
	(p)->win = htons((p)->win), \
	(p)->urp = htons((p)->urp))
#else
#define tcp_ntoh(p)	((p)->sport = ntohs((p)->sport), \
			 (p)->dport = ntohs((p)->dport), \
			 (p)->seq = ntohl((p)->seq), \
			 (p)->ack = ntohl((p)->ack), \
			 (p)->win = ntohs((p)->win), \
			 (p)->urp = ntohs((p)->urp))

#define tcp_hton(p)	((p)->sport = htons((p)->sport), \
			 (p)->dport = htons((p)->dport), \
			 (p)->seq = htonl((p)->seq), \
			 (p)->ack = htonl((p)->ack), \
			 (p)->win = htons((p)->win), \
			 (p)->urp = htons((p)->urp))
#endif

#endif

#if defined(__cplusplus)
}
#endif

#endif	/* _TCP_H */
