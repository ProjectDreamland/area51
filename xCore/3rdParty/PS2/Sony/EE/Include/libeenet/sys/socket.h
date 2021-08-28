/*	$NetBSD: socket.h,v 1.55.2.2 2001/05/01 10:29:06 he Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1985, 1986, 1988, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)socket.h	8.6 (Berkeley) 5/3/95
 */

#ifndef SCE_EENET__SYS_SOCKET_H_
#define SCE_EENET__SYS_SOCKET_H_

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Data types.
 */
#ifndef sceEENetSocklen_t
typedef unsigned int	sceEENetSocklen_t;
#endif

/*
 * Socket types.
 */
#define SCE_EENET_SOCK_STREAM 1 /* stream socket */
#define SCE_EENET_SOCK_DGRAM 2 /* datagram socket */
#define SCE_EENET_SOCK_RAW 3 /* raw-protocol interface */
#define SCE_EENET_SOCK_RDM 4 /* reliably-delivered message */
#define SCE_EENET_SOCK_SEQPACKET 5 /* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define SCE_EENET_SO_DEBUG 0x0001 /* turn on debugging info recording */
#define SCE_EENET_SO_ACCEPTCONN 0x0002 /* socket has had listen() */
#define SCE_EENET_SO_REUSEADDR 0x0004 /* allow local address reuse */
#define SCE_EENET_SO_KEEPALIVE 0x0008 /* keep connections alive */
#define SCE_EENET_SO_DONTROUTE 0x0010 /* just use interface addresses */
#define SCE_EENET_SO_BROADCAST 0x0020 /* permit sending of broadcast msgs */
#define SCE_EENET_SO_USELOOPBACK 0x0040 /* bypass hardware when possible */
#define SCE_EENET_SO_LINGER 0x0080 /* linger on close if data present */
#define SCE_EENET_SO_OOBINLINE 0x0100 /* leave received OOB data in line */
#define SCE_EENET_SO_REUSEPORT 0x0200 /* allow local address & port reuse */
#define SCE_EENET_SO_TIMESTAMP 0x0400 /* timestamp received dgram traffic */

/*
 * Additional options, not kept in so_options.
 */
#define SCE_EENET_SO_SNDBUF 0x1001 /* send buffer size */
#define SCE_EENET_SO_RCVBUF 0x1002 /* receive buffer size */
#define SCE_EENET_SO_SNDLOWAT 0x1003 /* send low-water mark */
#define SCE_EENET_SO_RCVLOWAT 0x1004 /* receive low-water mark */
#define SCE_EENET_SO_SNDTIMEO 0x1005 /* send timeout */
#define SCE_EENET_SO_RCVTIMEO 0x1006 /* receive timeout */
#define SCE_EENET_SO_ERROR 0x1007 /* get error status and clear */
#define SCE_EENET_SO_TYPE 0x1008 /* get socket type */
#define SCE_EENET_SO_NBIO 0x1009 /* non-blocking I/O */

/*
 * Structure used for manipulating linger option.
 */
struct	sceEENetLinger {
	int	l_onoff;		/* option on/off */
	int	l_linger;		/* linger time in seconds */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SCE_EENET_SOL_SOCKET 0xffff /* options for socket level */

/*
 * Address families.
 */
#define SCE_EENET_AF_UNSPEC 0 /* unspecified */
#define SCE_EENET_AF_LOCAL 1 /* local to host (pipes, portals) */
#define SCE_EENET_AF_UNIX SCE_EENET_AF_LOCAL /* backward compatibility */
#define SCE_EENET_AF_INET 2 /* internetwork: UDP, TCP, etc. */
#define SCE_EENET_AF_IMPLINK 3 /* arpanet imp addresses */
#define SCE_EENET_AF_PUP 4 /* pup protocols: e.g. BSP */
#define SCE_EENET_AF_CHAOS 5 /* mit CHAOS protocols */
#define SCE_EENET_AF_NS 6 /* XEROX NS protocols */
#define SCE_EENET_AF_ISO 7 /* ISO protocols */
#define SCE_EENET_AF_OSI SCE_EENET_AF_ISO
#define SCE_EENET_AF_ECMA 8 /* european computer manufacturers */
#define SCE_EENET_AF_DATAKIT 9 /* datakit protocols */
#define SCE_EENET_AF_CCITT 10 /* CCITT protocols, X.25 etc */
#define SCE_EENET_AF_SNA 11 /* IBM SNA */
#define SCE_EENET_AF_DECnet 12 /* DECnet */
#define SCE_EENET_AF_DLI 13 /* DEC Direct data link interface */
#define SCE_EENET_AF_LAT 14 /* LAT */
#define SCE_EENET_AF_HYLINK 15 /* NSC Hyperchannel */
#define SCE_EENET_AF_APPLETALK 16 /* Apple Talk */
#define SCE_EENET_AF_ROUTE 17 /* Internal Routing Protocol */
#define SCE_EENET_AF_LINK 18 /* Link layer interface */
#define SCE_EENET_pseudo_AF_XTP 19 /* eXpress Transfer Protocol (no AF) */
#define SCE_EENET_AF_COIP 20 /* connection-oriented IP, aka ST II */
#define SCE_EENET_AF_CNT 21 /* Computer Network Technology */
#define SCE_EENET_pseudo_AF_RTIP 22 /* Help Identify RTIP packets */
#define SCE_EENET_AF_IPX 23 /* Novell Internet Protocol */
#define SCE_EENET_AF_INET6 24 /* IP version 6 */
#define SCE_EENET_pseudo_AF_PIP 25 /* Help Identify PIP packets */
#define SCE_EENET_AF_ISDN 26 /* Integrated Services Digital Network*/
#define SCE_EENET_AF_E164 SCE_EENET_AF_ISDN /* CCITT E.164 recommendation */
#define SCE_EENET_AF_NATM 27 /* native ATM access */
#define SCE_EENET_AF_ARP 28 /* (rev.) addr. res. prot. (RFC 826) */
#define SCE_EENET_pseudo_AF_KEY 29 /* Internal key management protocol */
#define SCE_EENET_pseudo_AF_HDRCMPLT 30 /* Used by BPF to not rewrite hdrs
					   in interface output routine */
#define SCE_EENET_AF_MAX 31

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sceEENetSockaddr {
	u_char	sa_len;			/* total length */
	u_char	sa_family;		/* address family */
	char	sa_data[14];		/* actually longer; address value */
};

/*
 * RFC 2553: protocol-independent placeholder for socket addresses
  */
#define SCE_EENET__SS_MAXSIZE 128
#define SCE_EENET__SS_ALIGNSIZE (sizeof(int64_t))
#define SCE_EENET__SS_PAD1SIZE (SCE_EENET__SS_ALIGNSIZE - sizeof(u_char) * 2)
#define SCE_EENET__SS_PAD2SIZE (SCE_EENET__SS_MAXSIZE - sizeof(u_char) * 2 - \
				SCE_EENET__SS_PAD1SIZE - SCE_EENET__SS_ALIGNSIZE)

struct sceEENetSockaddrStorage {
	u_char  ss_len;     /* address length */
	u_char  ss_family;  /* address family */
	char    __ss_pad1[SCE_EENET__SS_PAD1SIZE];
	int64_t __ss_align; /* force desired structure storage alignment */
	char    __ss_pad2[SCE_EENET__SS_PAD2SIZE];
};

/*
 * Protocol families, same as address families for now.
 */
#define SCE_EENET_PF_UNSPEC SCE_EENET_AF_UNSPEC
#define SCE_EENET_PF_LOCAL SCE_EENET_AF_LOCAL
#define SCE_EENET_PF_UNIX SCE_EENET_PF_LOCAL /* backward compatibility */
#define SCE_EENET_PF_INET SCE_EENET_AF_INET
#define SCE_EENET_PF_IMPLINK SCE_EENET_AF_IMPLINK
#define SCE_EENET_PF_PUP SCE_EENET_AF_PUP
#define SCE_EENET_PF_CHAOS SCE_EENET_AF_CHAOS
#define SCE_EENET_PF_NS SCE_EENET_AF_NS
#define SCE_EENET_PF_ISO SCE_EENET_AF_ISO
#define SCE_EENET_PF_OSI SCE_EENET_AF_ISO
#define SCE_EENET_PF_ECMA SCE_EENET_AF_ECMA
#define SCE_EENET_PF_DATAKIT SCE_EENET_AF_DATAKIT
#define SCE_EENET_PF_CCITT SCE_EENET_AF_CCITT
#define SCE_EENET_PF_SNA SCE_EENET_AF_SNA
#define SCE_EENET_PF_DECnet SCE_EENET_AF_DECnet
#define SCE_EENET_PF_DLI SCE_EENET_AF_DLI
#define SCE_EENET_PF_LAT SCE_EENET_AF_LAT
#define SCE_EENET_PF_HYLINK SCE_EENET_AF_HYLINK
#define SCE_EENET_PF_APPLETALK SCE_EENET_AF_APPLETALK
#define SCE_EENET_PF_ROUTE SCE_EENET_AF_ROUTE
#define SCE_EENET_PF_LINK SCE_EENET_AF_LINK
#define SCE_EENET_PF_XTP SCE_EENET_pseudo_AF_XTP /* really just proto family, no AF */
#define SCE_EENET_PF_COIP SCE_EENET_AF_COIP
#define SCE_EENET_PF_CNT SCE_EENET_AF_CNT
#define SCE_EENET_PF_INET6 SCE_EENET_AF_INET6
#define SCE_EENET_PF_IPX SCE_EENET_AF_IPX /* same format as AF_NS */
#define SCE_EENET_PF_RTIP SCE_EENET_pseudo_AF_FTIP /* same format as AF_INET */
#define SCE_EENET_PF_PIP SCE_EENET_pseudo_AF_PIP
#define SCE_EENET_PF_ISDN SCE_EENET_AF_ISDN /* same as E164 */
#define SCE_EENET_PF_E164 SCE_EENET_AF_E164
#define SCE_EENET_PF_NATM SCE_EENET_AF_NATM
#define SCE_EENET_PF_ARP SCE_EENET_AF_ARP
#define SCE_EENET_PF_KEY SCE_EENET_pseudo_AF_KEY /* like PF_ROUTE, only for key mgmt */

#define SCE_EENET_PF_MAX SCE_EENET_AF_MAX

/*
 * Definitions for network related sysctl, CTL_NET.
 *
 * Second level is protocol family.
 * Third level is protocol number.
 *
 * Further levels are defined by the individual families below.
 */
#define SCE_EENET_NET_MAXID SCE_EENET_AF_MAX

#define SCE_EENET_CTL_NET_NAMES { \
	{ 0, 0 }, \
	{ "local", CTLTYPE_NODE }, \
	{ "inet", CTLTYPE_NODE }, \
	{ "implink", CTLTYPE_NODE }, \
	{ "pup", CTLTYPE_NODE }, \
	{ "chaos", CTLTYPE_NODE }, \
	{ "xerox_ns", CTLTYPE_NODE }, \
	{ "iso", CTLTYPE_NODE }, \
	{ "emca", CTLTYPE_NODE }, \
	{ "datakit", CTLTYPE_NODE }, \
	{ "ccitt", CTLTYPE_NODE }, \
	{ "ibm_sna", CTLTYPE_NODE }, \
	{ "decnet", CTLTYPE_NODE }, \
	{ "dec_dli", CTLTYPE_NODE }, \
	{ "lat", CTLTYPE_NODE }, \
	{ "hylink", CTLTYPE_NODE }, \
	{ "appletalk", CTLTYPE_NODE }, \
	{ "route", CTLTYPE_NODE }, \
	{ "link_layer", CTLTYPE_NODE }, \
	{ "xtp", CTLTYPE_NODE }, \
	{ "coip", CTLTYPE_NODE }, \
	{ "cnt", CTLTYPE_NODE }, \
	{ "rtip", CTLTYPE_NODE }, \
	{ "ipx", CTLTYPE_NODE }, \
	{ "inet6", CTLTYPE_NODE }, \
	{ "pip", CTLTYPE_NODE }, \
	{ "isdn", CTLTYPE_NODE }, \
	{ "natm", CTLTYPE_NODE }, \
	{ "arp", CTLTYPE_NODE }, \
	{ "key", CTLTYPE_NODE }, \
}

/*
 * PF_ROUTE - Routing table
 *
 * Three additional levels are defined:
 *	Fourth: address family, 0 is wildcard
 *	Fifth: type of info, defined below
 *	Sixth: flag(s) to mask with for NET_RT_FLAGS
 */
#define SCE_EENET_NET_RT_DUMP 1 /* dump; may limit to a.f. */
#define SCE_EENET_NET_RT_FLAGS 2 /* by flags, e.g. RESOLVING */
#define SCE_EENET_NET_RT_OIFLIST 3 /* old NET_RT_IFLIST (pre 1.5) */
#define SCE_EENET_NET_RT_IFLIST 4 /* survey interface list */
#define SCE_EENET_NET_RT_MAXID 5

#define SCE_EENET_CTL_NET_RT_NAMES { \
	{ 0, 0 }, \
	{ "dump", CTLTYPE_STRUCT }, \
	{ "flags", CTLTYPE_STRUCT }, \
	{ 0, 0 }, \
	{ "iflist", CTLTYPE_STRUCT }, \
}

/*
 * Maximum queue length specifiable by listen(2).
 */

#define SCE_EENET_SOMAXCONN 128

/*
 * Message header for recvmsg and sendmsg calls.
 * Used value-result for recvmsg, value only for sendmsg.
 */
struct sceEENetMsghdr {
	void		*msg_name;	/* optional address */
	sceEENetSocklen_t	msg_namelen;	/* size of address */
	struct sceEENetIovec	*msg_iov;	/* scatter/gather array */
	int		msg_iovlen;	/* # elements in msg_iov */
	void		*msg_control;	/* ancillary data, see below */
	sceEENetSocklen_t	msg_controllen;	/* ancillary data buffer len */
	int		msg_flags;	/* flags on received message */
};

#define SCE_EENET_MSG_OOB 0x1 /* process out-of-band data */
#define SCE_EENET_MSG_PEEK 0x2 /* peek at incoming message */
#define SCE_EENET_MSG_DONTROUTE 0x4 /* send without using routing tables */
#define SCE_EENET_MSG_EOR 0x8 /* data completes record */
#define SCE_EENET_MSG_TRUNC 0x10 /* data discarded before delivery */
#define SCE_EENET_MSG_CTRUNC 0x20 /* control data lost before delivery */
#define SCE_EENET_MSG_WAITALL 0x40 /* wait for full request or error */
#define SCE_EENET_MSG_DONTWAIT 0x80 /* this message should be nonblocking */
#define SCE_EENET_MSG_BCAST 0x100 /* this message was rcvd using link-level brdcst */
#define SCE_EENET_MSG_MCAST 0x200 /* this message was rcvd using link-level mcast */

/*
 * Header for ancillary data objects in msg_control buffer.
 * Used for additional information with/about a datagram
 * not expressible by flags.  The format is a sequence
 * of message elements headed by cmsghdr structures.
 */
struct sceEENetCmsghdr {
	sceEENetSocklen_t	cmsg_len;	/* data byte count, including hdr */
	int		cmsg_level;	/* originating protocol */
	int		cmsg_type;	/* protocol-specific type */
/* followed by	u_char  cmsg_data[]; */
};

/* given pointer to struct cmsghdr, return pointer to data */
#define SCE_EENET_CMSG_DATA(cmsg) \
	((u_char *)(cmsg) + SCE_EENET___CMSG_ALIGN(sizeof(struct sceEENetCmsghdr)))

/*
 * Alignment requirement for CMSG struct manipulation.
 * This basically behaves the same as ALIGN() ARCH/include/param.h.
 * We declare it separately for two reasons:
 * (1) avoid dependency between machine/param.h, and (2) to sync with kernel's
 * idea of ALIGNBYTES at runtime.
 * without (2), we can't guarantee binary compatibility in case of future
 * changes in ALIGNBYTES.
 */
#define SCE_EENET___CMSG_ALIGN(n) (((n) + __sceEENetCmsgAlignbytes()) & ~__sceEENetCmsgAlignbytes())

/* given pointer to struct cmsghdr, return pointer to next cmsghdr */
#define SCE_EENET_CMSG_NXTHDR(mhdr, cmsg) \
	(((caddr_t)(cmsg) + SCE_EENET___CMSG_ALIGN((cmsg)->cmsg_len) + \
			    SCE_EENET___CMSG_ALIGN(sizeof(struct sceEENetCmsghdr)) > \
	    (((caddr_t)(mhdr)->msg_control) + (mhdr)->msg_controllen)) ? \
	    (struct sceEENetCmsghdr *)NULL : \
	    (struct sceEENetCmsghdr *)((caddr_t)(cmsg) + SCE_EENET___CMSG_ALIGN((cmsg)->cmsg_len)))

#define SCE_EENET_CMSG_FIRSTHDR(mhdr) ((struct sceEENetCmsghdr *)(mhdr)->msg_control)

#define SCE_EENET_CMSG_SPACE(l) (SCE_EENET___CMSG_ALIGN(sizeof(struct sceEENetCmsghdr)) + __CMSG_ALIGN(l))
#define SCE_EENET_CMSG_LEN(l) (SCE_EENET___CMSG_ALIGN(sizeof(struct sceEENetCmsghdr)) + (l))

/* "Socket"-level control message types: */
#define SCE_EENET_SCM_RIGHTS 0x01 /* access rights (array of int) */
#define SCE_EENET_SCM_TIMESTAMP 0x02 /* timestamp (struct timeval) */
#define SCE_EENET_SCM_CREDS 0x04 /* credentials (struct sockcred) */

/*
 * Types of socket shutdown(2).
 */
#define SCE_EENET_SHUT_RD 0 /* Disallow further receives. */
#define SCE_EENET_SHUT_WR 1 /* Disallow further sends. */
#define SCE_EENET_SHUT_RDWR 2 /* Disallow further sends/receives. */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
int	__sceEENetCmsgAlignbytes (void);

int	sceEENetAccept (int, struct sceEENetSockaddr *, sceEENetSocklen_t *);
int	sceEENetBind (int, const struct sceEENetSockaddr *, sceEENetSocklen_t);
int	sceEENetConnect (int, const struct sceEENetSockaddr *, sceEENetSocklen_t);
int	sceEENetGetpeername (int, struct sceEENetSockaddr *, sceEENetSocklen_t *);
int	sceEENetGetsockname (int, struct sceEENetSockaddr *, sceEENetSocklen_t *);
int	sceEENetGetsockopt (int, int, int, void *, sceEENetSocklen_t *);
int	sceEENetListen (int, int);
ssize_t	sceEENetRecv (int, void *, size_t, int);
ssize_t	sceEENetRecvfrom (int, void *, size_t, int, struct sceEENetSockaddr *,
	    sceEENetSocklen_t *);
ssize_t	sceEENetRecvmsg (int, struct sceEENetMsghdr *, int);
ssize_t	sceEENetSend (int, const void *, size_t, int);
ssize_t	sceEENetSendto (int, const void *,
	    size_t, int, const struct sceEENetSockaddr *, sceEENetSocklen_t);
ssize_t	sceEENetSendmsg (int, const struct sceEENetMsghdr *, int);
int	sceEENetSetsockopt (int, int, int, const void *, sceEENetSocklen_t);
int	sceEENetShutdown (int, int);
int	sceEENetSocket (int, int, int);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !SCE_EENET__SYS_SOCKET_H_ */
