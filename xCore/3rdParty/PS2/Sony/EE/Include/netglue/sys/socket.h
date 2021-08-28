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
 *                 <netglue - netglue/sys/socket.h>
 *                    <header for network socket>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Sep,21,2001     komaki      first version
 *       1.10           Apr,24,2003     ksh         prefix
 */

#ifndef __NETGLUE_SYS_SOCKET_H__
#define	__NETGLUE_SYS_SOCKET_H__

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef u_char		sceNetGlueSaFamily_t;
typedef u_int		sceNetGlueSocklen_t;

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define sa_family_t	sceNetGlueSaFamily_t
#define socklen_t	sceNetGlueSocklen_t
#endif	/* !sceNetGlueDisableSocketSymbolAliases */
 
#define	SCE_NETGLUE_SOCK_STREAM	1	/* stream socket */
#define	SCE_NETGLUE_SOCK_DGRAM	2	/* datagram socket */
#define	SCE_NETGLUE_SOCK_RAW	3	/* raw-protocol interface */

#define	SCE_NETGLUE_AF_INET	2	/* internetwork: UDP, TCP, etc. */

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define	SOCK_STREAM	SCE_NETGLUE_SOCK_STREAM	
#define	SOCK_DGRAM	SCE_NETGLUE_SOCK_DGRAM	
#define	SOCK_RAW	SCE_NETGLUE_SOCK_RAW	
#define	AF_INET		SCE_NETGLUE_AF_INET	
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

typedef struct sceNetGlueSockaddr {
	u_char		sa_len;		/* total length */
	sceNetGlueSaFamily_t	sa_family;	/* address family */
	char		sa_data[14];	/* actually longer; address value */
} sceNetGlueSockaddr_t;

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define sockaddr	sceNetGlueSockaddr
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#define SCE_NETGLUE_SHUT_RD	0		/* shutdown the reading side */
#define SCE_NETGLUE_SHUT_WR	1		/* shutdown the writing side */
#define SCE_NETGLUE_SHUT_RDWR	2		/* shutdown both sides */

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define SHUT_RD		SCE_NETGLUE_SHUT_RD
#define SHUT_WR		SCE_NETGLUE_SHUT_WR
#define SHUT_RDWR	SCE_NETGLUE_SHUT_RDWR
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

int	sceNetGlueAccept(int, sceNetGlueSockaddr_t *, sceNetGlueSocklen_t *);
int	sceNetGlueBind(int, const sceNetGlueSockaddr_t *, sceNetGlueSocklen_t);
int	sceNetGlueConnect(int, const sceNetGlueSockaddr_t *, sceNetGlueSocklen_t);
int	sceNetGlueGetpeername(int, sceNetGlueSockaddr_t *, sceNetGlueSocklen_t *);
int	sceNetGlueGetsockname(int, sceNetGlueSockaddr_t *, sceNetGlueSocklen_t *);
int	sceNetGlueGetsockopt(int, int, int, void *, sceNetGlueSocklen_t *);
int	sceNetGlueListen(int, int);
ssize_t	sceNetGlueRecv(int, void *, size_t, int);
ssize_t	sceNetGlueRecvfrom(int, void *, size_t, int, sceNetGlueSockaddr_t *,
		sceNetGlueSocklen_t *);
ssize_t	sceNetGlueSend(int, const void *, size_t, int);
ssize_t	sceNetGlueSendto(int, const void *,
		size_t, int, const sceNetGlueSockaddr_t *, sceNetGlueSocklen_t);
int	sceNetGlueSetsockopt(int, int, int, const void *, sceNetGlueSocklen_t);
int	sceNetGlueShutdown(int, int);
int	sceNetGlueSocket(int, int, int);

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define accept		sceNetGlueAccept
#define bind		sceNetGlueBind
#define connect		sceNetGlueConnect
#define getpeername	sceNetGlueGetpeername
#define getsockname	sceNetGlueGetsockname
#define getsockopt	sceNetGlueGetsockopt
#define listen		sceNetGlueListen
#define recv		sceNetGlueRecv
#define recvfrom	sceNetGlueRecvfrom
#define send		sceNetGlueSend
#define sendto		sceNetGlueSendto
#define setsockopt	sceNetGlueSetsockopt
#define shutdown	sceNetGlueShutdown
#define socket		sceNetGlueSocket
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __NETGLUE_SYS_SOCKET_H__ */
