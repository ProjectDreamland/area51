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
 *                         inet - icmp.h
 *                 Internet Control Message Protoctol
 *
 * $Id: icmp.h,v 1.6 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_ICMP_H)
#define _ICMP_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	u_char type;
	u_char code;
	u_short chksum;
	u_short id;
	u_short seq;
} ICMP_HDR;

#define ICMP_TYPE_ECHO_REPLY		0

#define ICMP_TYPE_DEST_UNREACH		3
#define		ICMP_CODE_NET_UNREACH		0
#define		ICMP_CODE_HOST_UNREACH		1
#define		ICMP_CODE_PROTO_UNREACH		2
#define		ICMP_CODE_PORT_UNREACH		3
#define		ICMP_CODE_FRAG_AND_DF		4
#define		ICMP_CODE_SRC_HOST_FAILED	5
#define		ICMP_CODE_DST_NET_UNKNOWN	6
#define		ICMP_CODE_DST_HOST_UNKNOWN	7
#define		ICMP_CODE_SRC_HOST_ISOLATED	8
#define		ICMP_CODE_NET_ADMIN_PROHIBITED	9
#define		ICMP_CODE_NET_HOST_PROHIBITED	10
#define		ICMP_CODE_NET_TOS		11
#define		ICMP_CODE_HOST_TOS		12

#define	ICMP_TYPE_SOURCE_QUENCH		4

#define	ICMP_TYPE_REDIRECT		5

#define ICMP_TYPE_ECHO_REQUEST		8

#define ICMP_TYPE_TIME_EXCEEDED		11
#define		ICMP_CODE_TTL_EXCEEDED		0
#define		ICMP_CODE_FRT_EXCEEDED		1

#define	ICMP_TYPE_PARAMETER_PROBLEM	12

#define	ICMP_TYPE_TIMESTAMP_REQUEST	13

#define	ICMP_TYPE_TIMESTAMP_REPLY	14

#define	ICMP_TYPE_INFORMATION_REQUEST	15	/* (obsolete) */

#define	ICMP_TYPE_INFORMATION_REPLY	16	/* (obsolete) */

#define	ICMP_TYPE_ADDRESS_MASK_REQUEST	17

#define	ICMP_TYPE_ADDRESS_MASK_REPLY	18

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define icmp_ntoh(p)
#define icmp_hton(p)
#else

#define icmp_ntoh(p)	((p)->id = ntohs((p)->id), \
			 (p)->seq = ntohs((p)->seq))

#define icmp_hton(p)	((p)->id = htons((p)->id), \
			 (p)->seq = htons((p)->seq))
#endif

#if defined(__cplusplus)
}
#endif

#endif	/* _ICMP_H */
