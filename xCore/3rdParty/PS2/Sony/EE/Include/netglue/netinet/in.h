/* SCE CONFIDENTIAL
"PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                 <netglue - netglue/netinet/in.h>
 *                          <header for IP>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Sep,21,2001     komaki      first version
 *       1.10           Apr,24,2003     ksh         prefix
 */

#ifndef __NETGLUE_NETINET_IN_H__
#define __NETGLUE_NETINET_IN_H__

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#define	SCE_NETGLUE_IPPROTO_IP	0
#define	SCE_NETGLUE_IPPROTO_ICMP	1
#define	SCE_NETGLUE_IPPROTO_TCP	6
#define	SCE_NETGLUE_IPPROTO_UDP	17

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define	IPPROTO_IP	SCE_NETGLUE_IPPROTO_IP
#define	IPPROTO_ICMP	SCE_NETGLUE_IPPROTO_ICMP
#define	IPPROTO_TCP	SCE_NETGLUE_IPPROTO_TCP
#define	IPPROTO_UDP	SCE_NETGLUE_IPPROTO_UDP
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

typedef struct sceNetGlueInAddr {
	u_int s_addr;
} sceNetGlueInAddr_t;

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define in_addr		sceNetGlueInAddr
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#define	SCE_NETGLUE_IN_CLASSA(i)		(((u_int)(i) & 0x80000000) == 0)
#define	SCE_NETGLUE_IN_CLASSA_NET	0xff000000
#define	SCE_NETGLUE_IN_CLASSA_NSHIFT	24
#define	SCE_NETGLUE_IN_CLASSA_HOST	0x00ffffff
#define	SCE_NETGLUE_IN_CLASSA_MAX	128

#define	SCE_NETGLUE_IN_CLASSB(i)		(((u_int)(i) & 0xc0000000) == 0x80000000)
#define	SCE_NETGLUE_IN_CLASSB_NET	0xffff0000
#define	SCE_NETGLUE_IN_CLASSB_NSHIFT	16
#define	SCE_NETGLUE_IN_CLASSB_HOST	0x0000ffff
#define	SCE_NETGLUE_IN_CLASSB_MAX	65536

#define	SCE_NETGLUE_IN_CLASSC(i)		(((u_int)(i) & 0xe0000000) == 0xc0000000)
#define	SCE_NETGLUE_IN_CLASSC_NET	0xffffff00
#define	SCE_NETGLUE_IN_CLASSC_NSHIFT	8
#define	SCE_NETGLUE_IN_CLASSC_HOST	0x000000ff

#define	SCE_NETGLUE_IN_CLASSD(i)		(((u_int)(i) & 0xf0000000) == 0xe0000000)
#define	SCE_NETGLUE_IN_MULTICAST(i)	IN_CLASSD(i)

#define	SCE_NETGLUE_INADDR_ANY		(u_int)0x00000000
#define	SCE_NETGLUE_INADDR_LOOPBACK	(u_int)0x7f000001
#define	SCE_NETGLUE_INADDR_BROADCAST	(u_int)0xffffffff
#define	SCE_NETGLUE_INADDR_NONE		0xffffffff

#define	SCE_NETGLUE_IN_LOOPBACKNET		127

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define IN_CLASSA(i)		SCE_NETGLUE_IN_CLASSA(i)
#define IN_CLASSA_NET		SCE_NETGLUE_IN_CLASSA_NET
#define IN_CLASSA_NSHIFT	SCE_NETGLUE_IN_CLASSA_NSHIFT
#define IN_CLASSA_HOST		SCE_NETGLUE_IN_CLASSA_HOST
#define IN_CLASSA_MAX		SCE_NETGLUE_IN_CLASSA_MAX

#define IN_CLASSB(i)		SCE_NETGLUE_IN_CLASSB(i)
#define IN_CLASSB_NET		SCE_NETGLUE_IN_CLASSB_NET
#define IN_CLASSB_NSHIFT	SCE_NETGLUE_IN_CLASSB_NSHIFT
#define IN_CLASSB_HOST		SCE_NETGLUE_IN_CLASSB_HOST
#define IN_CLASSB_MAX		SCE_NETGLUE_IN_CLASSB_MAX

#define IN_CLASSC(i)		SCE_NETGLUE_IN_CLASSC(i)
#define IN_CLASSC_NET		SCE_NETGLUE_IN_CLASSC_NET
#define IN_CLASSC_NSHIFT	SCE_NETGLUE_IN_CLASSC_NSHIFT
#define IN_CLASSC_HOST		SCE_NETGLUE_IN_CLASSC_HOST

#define IN_CLASSD(i)		SCE_NETGLUE_IN_CLASSD(i)	
#define IN_MULTICAST(i)		SCE_NETGLUE_IN_MULTICAST(i)

#define INADDR_ANY		SCE_NETGLUE_INADDR_ANY
#define INADDR_LOOPBACK		SCE_NETGLUE_INADDR_LOOPBACK
#define INADDR_BROADCAST	SCE_NETGLUE_INADDR_BROADCAST
#define INADDR_NONE		SCE_NETGLUE_INADDR_NONE	

#define IN_LOOPBACKNET		SCE_NETGLUE_IN_LOOPBACKNET
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

typedef struct sceNetGlueSockaddrIn {
	u_char			sin_len;
	u_char			sin_family;
	u_short			sin_port;
	sceNetGlueInAddr_t	sin_addr;
	char			sin_zero[8];
} sceNetGlueSockaddrIn_t;

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define sockaddr_in	sceNetGlueSockaddrIn
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#define SCE_NETGLUE_IP_MULTICAST_IF		9
#define SCE_NETGLUE_IP_MULTICAST_TTL	10
#define SCE_NETGLUE_IP_MULTICAST_LOOP	11
#define SCE_NETGLUE_IP_ADD_MEMBERSHIP	12
#define SCE_NETGLUE_IP_DROP_MEMBERSHIP	13

#define SCE_NETGLUE_IP_DEFAULT_MULTICAST_TTL	1
#define SCE_NETGLUE_IP_DEFAULT_MULTICAST_LOOP	1

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define IP_MULTICAST_IF		SCE_NETGLUE_IP_MULTICAST_IF
#define IP_MULTICAST_TTL	SCE_NETGLUE_IP_MULTICAST_TTL
#define IP_MULTICAST_LOOP	SCE_NETGLUE_IP_MULTICAST_LOOP
#define IP_ADD_MEMBERSHIP	SCE_NETGLUE_IP_ADD_MEMBERSHIP
#define IP_DROP_MEMBERSHIP	SCE_NETGLUE_IP_DROP_MEMBERSHIP

#define IP_DEFAULT_MULTICAST_TTL	SCE_NETGLUE_IP_DEFAULT_MULTICAST_TTL
#define IP_DEFAULT_MULTICAST_LOOP	SCE_NETGLUE_IP_DEFAULT_MULTICAST_LOOP
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

u_int	sceNetGlueHtonl(u_int);
u_short	sceNetGlueHtons(u_short);
u_int	sceNetGlueNtohl(u_int);
u_short	sceNetGlueNtohs(u_short);

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#undef	htonl
#undef	htons
#undef	ntohl
#undef	ntohs
#define htonl	sceNetGlueHtonl
#define htons	sceNetGlueHtons
#define ntohl	sceNetGlueNtohl
#define ntohs	sceNetGlueNtohs
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __NETGLUE_NETINET_IN_H__ */
