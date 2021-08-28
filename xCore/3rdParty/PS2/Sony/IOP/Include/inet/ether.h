/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.03
 *                           Shift-JIS
 *
 *      Copyright (C) 1995-2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                        inet - ether.h
 *                        Ethernet Header
 *
 * $Id: ether.h,v 1.6 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_ETHER_H)
#define _ETHER_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef u_char ETHER_ADR[6];

typedef struct {
	ETHER_ADR dst, src;
	u_short type;
} ETHER_HDR;

#define ETHER_TYPE_IP		0x0800
#define ETHER_TYPE_ARP		0x0806
#define ETHER_TYPE_RARP		0x8035
#define ETHER_TYPE_PPPOE_DS	0x8863
#define ETHER_TYPE_PPPOE_SS	0x8864

#define ETHER_MIN	46
#define ETHER_MTU	1500

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define ether_ntoh(p)
#define ether_hton(p)
#else

#define ether_ntoh(p)		((p)->type = ntohs((p)->type))
#define ether_hton(p)		((p)->type = htons((p)->type))

#endif

#define eadr_cmp(a, b)	(((u_short *)a)[0] != ((u_short *)b)[0] \
			|| ((u_short *)a)[1] != ((u_short *)b)[1] \
			|| ((u_short *)a)[2] != ((u_short *)b)[2])

#define eadr_copy(a, b) (((u_short *)b)[0] = ((u_short *)a)[0], \
			 ((u_short *)b)[1] = ((u_short *)a)[1], \
			 ((u_short *)b)[2] = ((u_short *)a)[2])

#if defined(__cplusplus)
}
#endif

#endif	/* _ETHER_H */
