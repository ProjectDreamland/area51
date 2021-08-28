/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      I/O Processor Library
 *                          Version 1.01
 *                           Shift-JIS
 *
 *      Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         inet - igmp.h
 *                 Internet Group Mangement Protoctol
 *
 * $Id: igmp.h,v 1.2 2002/03/27 05:11:13 xokano Exp $
 */

#if !defined(_IGMP_H)
#define _IGMP_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	u_char type;
	u_char code;
	u_short chksum;
	u_long group;
} IGMP_HDR;

#define IGMP_TYPE_MEMBERSHIP_QUERY	0x11	/* Membership Query */
#define IGMP_TYPE_V1_MEMBERSHIP_REPORT	0x12	/* V1 Membership Report */
#define IGMP_TYPE_V2_MEMBERSHIP_REPORT	0x16	/* V2 Membership Report */
#define IGMP_TYPE_V2_LEAVE_GROUP	0x17	/* V2 Leave Group */

#define IGMP_MAX_HOST_REPORT_DELAY	100	/* 10 [sec] */
#define IGMP_TIMER_SCALE		10	/* 1/10 [sec] */

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#define igmp_ntoh(p)
#define igmp_hton(p)
#else

#define igmp_ntoh(p)	put4u(&(p)->group, ntohl(get4u(&(p)->group)))
#define igmp_hton(p)	put4u(&(p)->group, htonl(get4u(&(p)->group)))
#endif

#if defined(__cplusplus)
}
#endif

#endif	/* _IGMP_H */
