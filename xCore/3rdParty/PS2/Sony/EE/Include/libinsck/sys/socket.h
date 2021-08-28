/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.20
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                      <libinsck - sys/socket.h>
 *                      <libinsck general header>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           May,17,2001     komaki      first version
 *       1.10           Apr,24,2003     mka, ksh    prefix
 *       1.20           Mar,03,2004     mka         add some socket options
 */

#ifndef __INSOCK_SYS_SOCKET_H__
#define	__INSOCK_SYS_SOCKET_H__

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef u_char		sceInsockSaFamily_t;
typedef u_int		sceInsockSocklen_t;

#if !defined(sceInsockDisableSocketSymbolAliases)
#define sa_family_t	sceInsockSaFamily_t
#define socklen_t	sceInsockSocklen_t
#endif	/* !sceInsockDisableSocketSymbolAliases */
 
#define	SCE_INSOCK_SOCK_STREAM	1	/* stream socket */
#define	SCE_INSOCK_SOCK_DGRAM	2	/* datagram socket */
#define	SCE_INSOCK_SOCK_RAW	3	/* raw-protocol interface */

#define	SCE_INSOCK_AF_INET	2	/* internetwork: UDP, TCP, etc. */

#if !defined(sceInsockDisableSocketSymbolAliases)
#define	SOCK_STREAM	SCE_INSOCK_SOCK_STREAM	
#define	SOCK_DGRAM	SCE_INSOCK_SOCK_DGRAM	
#define	SOCK_RAW	SCE_INSOCK_SOCK_RAW	
#define	AF_INET		SCE_INSOCK_AF_INET	
#endif	/* !sceInsockDisableSocketSymbolAliases */

#define SCE_INSOCK_SO_SNDBUF	0x1001		/* send buffer size */
#define SCE_INSOCK_SO_RCVBUF	0x1002		/* receive buffer size */
#define SCE_INSOCK_SO_SNDTIMEO	0x1005		/* send timeout */
#define SCE_INSOCK_SO_RCVTIMEO	0x1006		/* receive timeout */
#define SCE_INSOCK_SO_ERROR	0x1007		/* get error status & clear */
#define	SCE_INSOCK_SO_TYPE	0x1008		/* get socket type */
#define SCE_INSOCK_SO_NBIO	0x1009		/* non-blocking I/O */

#define	SCE_INSOCK_SOL_SOCKET	0xffff		/* options for socket level */

#if !defined(sceInsockDisableSocketSymbolAliases)
#define SO_SNDBUF	SCE_INSOCK_SO_SNDBUF	
#define SO_RCVBUF	SCE_INSOCK_SO_RCVBUF	
#define SO_SNDTIMEO	SCE_INSOCK_SO_SNDTIMEO	
#define SO_RCVTIMEO	SCE_INSOCK_SO_RCVTIMEO	
#define SO_ERROR	SCE_INSOCK_SO_ERROR
#define	SO_TYPE		SCE_INSOCK_SO_TYPE
#define	SO_NBIO		SCE_INSOCK_SO_NBIO
#define	SOL_SOCKET	SCE_INSOCK_SOL_SOCKET	
#endif	/* !sceInsockDisableSocketSymbolAliases */

typedef struct sceInsockSockaddr {
	u_char		sa_len;		/* total length */
	sceInsockSaFamily_t	sa_family;	/* address family */
	char		sa_data[14];	/* actually longer; address value */
} sceInsockSockaddr_t;

#if !defined(sceInsockDisableSocketSymbolAliases)
#define sockaddr	sceInsockSockaddr
#endif	/* !sceInsockDisableSocketSymbolAliases */

#define SCE_INSOCK_SHUT_RD	0		/* shutdown the reading side */
#define SCE_INSOCK_SHUT_WR	1		/* shutdown the writing side */
#define SCE_INSOCK_SHUT_RDWR	2		/* shutdown both sides */

#if !defined(sceInsockDisableSocketSymbolAliases)
#define SHUT_RD		SCE_INSOCK_SHUT_RD
#define SHUT_WR		SCE_INSOCK_SHUT_WR
#define SHUT_RDWR	SCE_INSOCK_SHUT_RDWR
#endif	/* !sceInsockDisableSocketSymbolAliases */

#define	SCE_INSOCK_MSG_OOB	0x1		/* out of band data */
#define	SCE_INSOCK_MSG_WAITALL	0x40		/* wait for all request */

#if !defined(sceInsockDisableSocketSymbolAliases)
#define	MSG_OOB		SCE_INSOCK_MSG_OOB
#define	MSG_WAITALL	SCE_INSOCK_MSG_WAITALL
#endif	/* !sceInsockDisableSocketSymbolAliases */

int	sceInsockAccept(int, sceInsockSockaddr_t *, sceInsockSocklen_t *);
int	sceInsockBind(int, const sceInsockSockaddr_t *, sceInsockSocklen_t);
int	sceInsockConnect(int, const sceInsockSockaddr_t *, sceInsockSocklen_t);
int	sceInsockGetpeername(int, sceInsockSockaddr_t *, sceInsockSocklen_t *);
int	sceInsockGetsockname(int, sceInsockSockaddr_t *, sceInsockSocklen_t *);
int	sceInsockGetsockopt(int, int, int, void *, sceInsockSocklen_t *);
int	sceInsockListen(int, int);
ssize_t	sceInsockRecv(int, void *, size_t, int);
ssize_t	sceInsockRecvfrom(int, void *, size_t, int, sceInsockSockaddr_t *,
		sceInsockSocklen_t *);
ssize_t	sceInsockSend(int, const void *, size_t, int);
ssize_t	sceInsockSendto(int, const void *,
		size_t, int, const sceInsockSockaddr_t *, sceInsockSocklen_t);
int	sceInsockSetsockopt(int, int, int, const void *, sceInsockSocklen_t);
int	sceInsockShutdown(int, int);
int	sceInsockSocket(int, int, int);

#if !defined(sceInsockDisableSocketSymbolAliases)
#define accept		sceInsockAccept
#define bind		sceInsockBind
#define connect		sceInsockConnect
#define getpeername	sceInsockGetpeername
#define getsockname	sceInsockGetsockname
#define getsockopt	sceInsockGetsockopt
#define listen		sceInsockListen
#define recv		sceInsockRecv
#define recvfrom	sceInsockRecvfrom
#define send		sceInsockSend
#define sendto		sceInsockSendto
#define setsockopt	sceInsockSetsockopt
#define shutdown	sceInsockShutdown
#define socket		sceInsockSocket
#endif	/* !sceInsockDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __INSOCK_SYS_SOCKET_H__ */
