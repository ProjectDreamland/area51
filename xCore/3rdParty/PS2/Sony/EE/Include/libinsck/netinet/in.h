/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                 <libinsck - libinsck/netinet/in.h>
 *                          <header for IP>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           May,17,2001     komaki      first version
 *       1.10           Apr,24,2003     mka, ksh    prefix
 */

#ifndef __INSOCK_NETINET_IN_H__
#define __INSOCK_NETINET_IN_H__

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#define SCE_INSOCK_IPPROTO_IP	0
#define	SCE_INSOCK_IPPROTO_ICMP	1
#define SCE_INSOCK_IPPROTO_IGMP	2
#define	SCE_INSOCK_IPPROTO_TCP	6
#define	SCE_INSOCK_IPPROTO_UDP	17

#if !defined(sceInsockDisableSocketSymbolAliases)
#define IPPROTO_IP	SCE_INSOCK_IPPROTO_IP
#define	IPPROTO_ICMP	SCE_INSOCK_IPPROTO_ICMP
#define IPPROTO_IGMP	SCE_INSOCK_IPPROTO_IGMP
#define	IPPROTO_TCP	SCE_INSOCK_IPPROTO_TCP
#define	IPPROTO_UDP	SCE_INSOCK_IPPROTO_UDP
#endif	/* !sceInsockDisableSocketSymbolAliases */

typedef struct sceInsockInAddr {
	u_int s_addr;
} sceInsockInAddr_t;

#if !defined(sceInsockDisableSocketSymbolAliases)
#define in_addr		sceInsockInAddr
#endif	/* !sceInsockDisableSocketSymbolAliases */

#define	SCE_INSOCK_IN_CLASSA(i)		(((u_int)(i) & 0x80000000) == 0)
#define	SCE_INSOCK_IN_CLASSA_NET	0xff000000
#define	SCE_INSOCK_IN_CLASSA_NSHIFT	24
#define	SCE_INSOCK_IN_CLASSA_HOST	0x00ffffff
#define	SCE_INSOCK_IN_CLASSA_MAX	128

#define	SCE_INSOCK_IN_CLASSB(i)		(((u_int)(i) & 0xc0000000) == 0x80000000)
#define	SCE_INSOCK_IN_CLASSB_NET	0xffff0000
#define	SCE_INSOCK_IN_CLASSB_NSHIFT	16
#define	SCE_INSOCK_IN_CLASSB_HOST	0x0000ffff
#define	SCE_INSOCK_IN_CLASSB_MAX	65536

#define	SCE_INSOCK_IN_CLASSC(i)		(((u_int)(i) & 0xe0000000) == 0xc0000000)
#define	SCE_INSOCK_IN_CLASSC_NET	0xffffff00
#define	SCE_INSOCK_IN_CLASSC_NSHIFT	8
#define	SCE_INSOCK_IN_CLASSC_HOST	0x000000ff

#define	SCE_INSOCK_IN_CLASSD(i)		(((u_int)(i) & 0xf0000000) == 0xe0000000)
#define	SCE_INSOCK_IN_MULTICAST(i)	IN_CLASSD(i)

#define	SCE_INSOCK_INADDR_ANY		(u_int)0x00000000
#define	SCE_INSOCK_INADDR_LOOPBACK	(u_int)0x7f000001
#define	SCE_INSOCK_INADDR_BROADCAST	(u_int)0xffffffff
#define	SCE_INSOCK_INADDR_NONE		0xffffffff

#define	SCE_INSOCK_IN_LOOPBACKNET		127

#if !defined(sceInsockDisableSocketSymbolAliases)
#define IN_CLASSA(i)		SCE_INSOCK_IN_CLASSA(i)
#define IN_CLASSA_NET		SCE_INSOCK_IN_CLASSA_NET
#define IN_CLASSA_NSHIFT	SCE_INSOCK_IN_CLASSA_NSHIFT
#define IN_CLASSA_HOST		SCE_INSOCK_IN_CLASSA_HOST
#define IN_CLASSA_MAX		SCE_INSOCK_IN_CLASSA_MAX

#define IN_CLASSB(i)		SCE_INSOCK_IN_CLASSB(i)
#define IN_CLASSB_NET		SCE_INSOCK_IN_CLASSB_NET
#define IN_CLASSB_NSHIFT	SCE_INSOCK_IN_CLASSB_NSHIFT
#define IN_CLASSB_HOST		SCE_INSOCK_IN_CLASSB_HOST
#define IN_CLASSB_MAX		SCE_INSOCK_IN_CLASSB_MAX

#define IN_CLASSC(i)		SCE_INSOCK_IN_CLASSC(i)
#define IN_CLASSC_NET		SCE_INSOCK_IN_CLASSC_NET
#define IN_CLASSC_NSHIFT	SCE_INSOCK_IN_CLASSC_NSHIFT
#define IN_CLASSC_HOST		SCE_INSOCK_IN_CLASSC_HOST

#define IN_CLASSD(i)		SCE_INSOCK_IN_CLASSD(i)	
#define IN_MULTICAST(i)		SCE_INSOCK_IN_MULTICAST(i)

#define INADDR_ANY		SCE_INSOCK_INADDR_ANY
#define INADDR_LOOPBACK		SCE_INSOCK_INADDR_LOOPBACK
#define INADDR_BROADCAST	SCE_INSOCK_INADDR_BROADCAST
#define INADDR_NONE		SCE_INSOCK_INADDR_NONE	

#define IN_LOOPBACKNET		SCE_INSOCK_IN_LOOPBACKNET
#endif	/* !sceInsockDisableSocketSymbolAliases */

typedef struct sceInsockSockaddrIn {
	u_char			sin_len;
	u_char			sin_family;
	u_short			sin_port;
	sceInsockInAddr_t	sin_addr;
	char			sin_zero[8];
} sceInsockSockaddrIn_t;

#if !defined(sceInsockDisableSocketSymbolAliases)
#define sockaddr_in	sceInsockSockaddrIn
#endif	/* !sceInsockDisableSocketSymbolAliases */

#define SCE_INSOCK_IP_MULTICAST_IF		9
#define SCE_INSOCK_IP_MULTICAST_TTL		10
#define SCE_INSOCK_IP_MULTICAST_LOOP		11
#define SCE_INSOCK_IP_ADD_MEMBERSHIP		12
#define SCE_INSOCK_IP_DROP_MEMBERSHIP		13

#define SCE_INSOCK_IP_DEFAULT_MULTICAST_TTL	1
#define SCE_INSOCK_IP_DEFAULT_MULTICAST_LOOP	1

#if !defined(sceInsockDisableSocketSymbolAliases)
#define IP_MULTICAST_IF			SCE_INSOCK_IP_MULTICAST_IF
#define IP_MULTICAST_TTL		SCE_INSOCK_IP_MULTICAST_TTL
#define IP_MULTICAST_LOOP		SCE_INSOCK_IP_MULTICAST_LOOP
#define IP_ADD_MEMBERSHIP		SCE_INSOCK_IP_ADD_MEMBERSHIP
#define IP_DROP_MEMBERSHIP		SCE_INSOCK_IP_DROP_MEMBERSHIP

#define IP_DEFAULT_MULTICAST_TTL	SCE_INSOCK_IP_DEFAULT_MULTICAST_TTL
#define IP_DEFAULT_MULTICAST_LOOP	SCE_INSOCK_IP_DEFAULT_MULTICAST_LOOP
#endif	/* !sceInsockDisableSocketSymbolAliases */

typedef struct sceInsockIpMreq {
	sceInsockInAddr_t imr_multiaddr;
	sceInsockInAddr_t imr_interface;
} sceInsockIpMreq_t;

#if !defined(sceInsockDisableSocketSymbolAliases)
#define ip_mreq	sceInsockIpMreq
#endif	/* !sceInsockDisableSocketSymbolAliases */

u_int	sceInsockHtonl(u_int);
u_short	sceInsockHtons(u_short);
u_int	sceInsockNtohl(u_int);
u_short	sceInsockNtohs(u_short);

#if !defined(sceInsockDisableSocketSymbolAliases)
#undef	htonl
#undef	htons
#undef	ntohl
#undef	ntohs
#define htonl	sceInsockHtonl
#define htons	sceInsockHtons
#define ntohl	sceInsockNtohl
#define ntohs	sceInsockNtohs
#endif	/* !sceInsockDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __INSOCK_NETINET_IN_H__ */
