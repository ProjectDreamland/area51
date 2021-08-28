/*	$NetBSD: in.h,v 1.47.4.2 2001/04/24 22:21:35 he Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)in.h	8.3 (Berkeley) 1/3/94
 */

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, and numerous additions.
 */

#ifndef SCE_EENET__NETINET_IN_H_
#define SCE_EENET__NETINET_IN_H_

/*
 * Protocols
 */
#define SCE_EENET_IPPROTO_IP 0 /* dummy for IP */
#define SCE_EENET_IPPROTO_HOPOPTS 0 /* IP6 hop-by-hop options */
#define SCE_EENET_IPPROTO_ICMP 1 /* control message protocol */
#define SCE_EENET_IPPROTO_IGMP 2 /* group mgmt protocol */
#define SCE_EENET_IPPROTO_GGP 3 /* gateway^2 (deprecated) */
#define SCE_EENET_IPPROTO_IPV4 4 /* IP header */
#define SCE_EENET_IPPROTO_IPIP 4 /* IP inside IP */
#define SCE_EENET_IPPROTO_TCP 6 /* tcp */
#define SCE_EENET_IPPROTO_EGP 8 /* exterior gateway protocol */
#define SCE_EENET_IPPROTO_PUP 12 /* pup */
#define SCE_EENET_IPPROTO_UDP 17 /* user datagram protocol */
#define SCE_EENET_IPPROTO_IDP 22 /* xns idp */
#define SCE_EENET_IPPROTO_TP 29 /* tp-4 w/ class negotiation */
#define SCE_EENET_IPPROTO_IPV6 41 /* IP6 header */
#define SCE_EENET_IPPROTO_ROUTING 43 /* IP6 routing header */
#define SCE_EENET_IPPROTO_FRAGMENT 44 /* IP6 fragmentation header */
#define SCE_EENET_IPPROTO_RSVP 46 /* resource reservation */
#define SCE_EENET_IPPROTO_GRE 47 /* GRE encaps RFC 1701 */
#define SCE_EENET_IPPROTO_ESP 50 /* encap. security payload */
#define SCE_EENET_IPPROTO_AH 51 /* authentication header */
#define SCE_EENET_IPPROTO_MOBILE 55 /* IP Mobility RFC 2004 */
#define SCE_EENET_IPPROTO_IPV6_ICMP 58 /* IPv6 ICMP */
#define SCE_EENET_IPPROTO_ICMPV6 58 /* ICMP6 */
#define SCE_EENET_IPPROTO_NONE 59 /* IP6 no next header */
#define SCE_EENET_IPPROTO_DSTOPTS 60 /* IP6 destination option */
#define SCE_EENET_IPPROTO_EON 80 /* ISO cnlp */
#define SCE_EENET_IPPROTO_ENCAP 98 /* encapsulation header */
#define SCE_EENET_IPPROTO_PIM 103 /* Protocol indep. multicast */
#define SCE_EENET_IPPROTO_IPCOMP 108 /* IP Payload Comp. Protocol */

#define SCE_EENET_IPPROTO_RAW 255 /* raw IP packet */
#define SCE_EENET_IPPROTO_MAX 256

/* last return value of *_input(), meaning "all job for this pkt is done".  */
#define SCE_EENET_IPPROTO_DONE 257


/*
 * Local port number conventions:
 *
 * Ports < IPPORT_RESERVED are reserved for privileged processes (e.g. root),
 * unless a kernel is compiled with IPNOPRIVPORTS defined.
 *
 * When a user does a bind(2) or connect(2) with a port number of zero,
 * a non-conflicting local port address is chosen.
 *
 * The default range is IPPORT_ANONMIX to IPPORT_ANONMAX, although
 * that is settable by sysctl(3); net.inet.ip.anonportmin and
 * net.inet.ip.anonportmax respectively.
 *
 * A user may set the IPPROTO_IP option IP_PORTRANGE to change this
 * default assignment range.
 *
 * The value IP_PORTRANGE_DEFAULT causes the default behavior.
 *
 * The value IP_PORTRANGE_HIGH is the same as IP_PORTRANGE_DEFAULT,
 * and exists only for FreeBSD compatibility purposes.
 *
 * The value IP_PORTRANGE_LOW changes the range to the "low" are
 * that is (by convention) restricted to privileged processes.
 * This convention is based on "vouchsafe" principles only.
 * It is only secure if you trust the remote host to restrict these ports.
 * The range is IPPORT_RESERVEDMIN to IPPORT_RESERVEDMAX.
 */

#define SCE_EENET_IPPORT_RESERVED 1024
#define SCE_EENET_IPPORT_ANONMIN 49152
#define SCE_EENET_IPPORT_ANONMAX 65535
#define SCE_EENET_IPPORT_RESERVEDMIN 600
#define SCE_EENET_IPPORT_RESERVEDMAX (SCE_EENET_IPPORT_RESERVED-1)

/*
 * Internet address (a structure for historical reasons)
 */

#if defined(__GNUC__)
#define SCE_EENET_GCC_PACKED  __attribute__((packed))
#endif  /* defined(__GNUC__) */
#if defined(__MWERKS__)
#define SCE_EENET_GCC_PACKED
#pragma options align=packed
#endif  /* defined(__MWERKS__) */

struct sceEENetInAddr {
	u_int32_t s_addr;
} SCE_EENET_GCC_PACKED;

#if defined(__MWERKS__)
#pragma options align=reset
#endif  /* defined(__MWERKS__) */
#undef SCE_EENET_GCC_PACKED

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 *
 * By byte-swapping the constants, we avoid ever having to byte-swap IP
 * addresses inside the kernel.  Unfortunately, user-level programs rely
 * on these macros not doing byte-swapping.
 */
#define SCE_EENET___IPADDR(x) ((u_int32_t)(x))

#define SCE_EENET_IN_CLASSA(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0x80000000)) == \
				 SCE_EENET___IPADDR(0x00000000))
#define SCE_EENET_IN_CLASSA_NET SCE_EENET___IPADDR(0xff000000)
#define SCE_EENET_IN_CLASSA_NSHIFT 24
#define SCE_EENET_IN_CLASSA_HOST SCE_EENET___IPADDR(0x00ffffff)
#define SCE_EENET_IN_CLASSA_MAX 128

#define SCE_EENET_IN_CLASSB(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0xc0000000)) == \
				 SCE_EENET___IPADDR(0x80000000))
#define SCE_EENET_IN_CLASSB_NET SCE_EENET___IPADDR(0xffff0000)
#define SCE_EENET_IN_CLASSB_NSHIFT 16
#define SCE_EENET_IN_CLASSB_HOST SCE_EENET___IPADDR(0x0000ffff)
#define SCE_EENET_IN_CLASSB_MAX 65536

#define SCE_EENET_IN_CLASSC(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0xe0000000)) == \
				 SCE_EENET___IPADDR(0xc0000000))
#define SCE_EENET_IN_CLASSC_NET SCE_EENET___IPADDR(0xffffff00)
#define SCE_EENET_IN_CLASSC_NSHIFT 8
#define SCE_EENET_IN_CLASSC_HOST SCE_EENET___IPADDR(0x000000ff)

#define SCE_EENET_IN_CLASSD(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0xf0000000)) == \
				 SCE_EENET___IPADDR(0xe0000000))
/* These ones aren't really net and host fields, but routing needn't know. */
#define SCE_EENET_IN_CLASSD_NET SCE_EENET___IPADDR(0xf0000000)
#define SCE_EENET_IN_CLASSD_NSHIFT 28
#define SCE_EENET_IN_CLASSD_HOST SCE_EENET___IPADDR(0x0fffffff)
#define SCE_EENET_IN_MULTICAST(i) IN_CLASSD(i)

#define SCE_EENET_IN_EXPERIMENTAL(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0xf0000000)) == \
				 SCE_EENET___IPADDR(0xf0000000))
#define SCE_EENET_IN_BADCLASS(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0xf0000000)) == \
				 SCE_EENET___IPADDR(0xf0000000))

#define SCE_EENET_IN_LOCAL_GROUP(i) (((u_int32_t)(i) & SCE_EENET___IPADDR(0xffffff00)) == \
				 SCE_EENET___IPADDR(0xe0000000))

#define SCE_EENET_INADDR_ANY SCE_EENET___IPADDR(0x00000000)
#define SCE_EENET_INADDR_LOOPBACK SCE_EENET___IPADDR(0x7f000001)
#define SCE_EENET_INADDR_BROADCAST SCE_EENET___IPADDR(0xffffffff) /* must be masked */
#define SCE_EENET_INADDR_NONE SCE_EENET___IPADDR(0xffffffff) /* -1 return */

#define SCE_EENET_INADDR_UNSPEC_GROUP SCE_EENET___IPADDR(0xe0000000) /* 224.0.0.0 */
#define SCE_EENET_INADDR_ALLHOSTS_GROUP SCE_EENET___IPADDR(0xe0000001) /* 224.0.0.1 */
#define SCE_EENET_INADDR_ALLRTRS_GROUP SCE_EENET___IPADDR(0xe0000002) /* 224.0.0.2 */
#define SCE_EENET_INADDR_MAX_LOCAL_GROUP SCE_EENET___IPADDR(0xe00000ff) /* 224.0.0.255 */

#define SCE_EENET_IN_LOOPBACKNET 127 /* official! */

/*
 * Socket address, internet style.
 */
struct sceEENetSockaddrIn {
	u_int8_t  sin_len;
	u_int8_t  sin_family;
	u_int16_t sin_port;
	struct	  sceEENetInAddr sin_addr;
	int8_t	  sin_zero[8];
};

#define SCE_EENET_INET_ADDRSTRLEN 16

/*
 * Structure used to describe IP options.
 * Used to store options internally, to pass them to a process,
 * or to restore options retrieved earlier.
 * The ip_dst is used for the first-hop gateway when using a source route
 * (this gets put into the header proper).
 */
struct sceEENetIpOpts {
	struct sceEENetInAddr	ip_dst;		/* first hop, 0 w/o src rt */
	int8_t		ip_opts[40];	/* actually variable in size */
};

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#define SCE_EENET_IP_OPTIONS 1 /* buf/ip_opts; set/get IP options */
#define SCE_EENET_IP_HDRINCL 2 /* int; header is included with data */
#define SCE_EENET_IP_TOS 3 /* int; IP type of service and preced. */
#define SCE_EENET_IP_TTL 4 /* int; IP time to live */
#define SCE_EENET_IP_RECVOPTS 5 /* bool; receive all IP opts w/dgram */
#define SCE_EENET_IP_RECVRETOPTS 6 /* bool; receive IP opts for response */
#define SCE_EENET_IP_RECVDSTADDR 7 /* bool; receive IP dst addr w/dgram */
#define SCE_EENET_IP_RETOPTS 8 /* ip_opts; set/get IP options */
#define SCE_EENET_IP_MULTICAST_IF 9 /* in_addr; set/get IP multicast i/f */
#define SCE_EENET_IP_MULTICAST_TTL 10 /* u_char; set/get IP multicast ttl */
#define SCE_EENET_IP_MULTICAST_LOOP 11 /* u_char; set/get IP multicast loopback */
#define SCE_EENET_IP_ADD_MEMBERSHIP 12 /* ip_mreq; add an IP group membership */
#define SCE_EENET_IP_DROP_MEMBERSHIP 13 /* ip_mreq; drop an IP group membership */
#define SCE_EENET_IP_PORTRANGE 19 /* int; range to use for ephemeral port */
#define SCE_EENET_IP_RECVIF 20 /* bool; receive reception if w/dgram */
#define SCE_EENET_IP_ERRORMTU 21 /* int; get MTU of last xmit = EMSGSIZE */
#if 1 /*IPSEC*/
#define SCE_EENET_IP_IPSEC_POLICY 22 /* struct; get/set security policy */
#endif

/*
 * Defaults and limits for options
 */
#define SCE_EENET_IP_DEFAULT_MULTICAST_TTL 1 /* normally limit m'casts to 1 hop */
#define SCE_EENET_IP_DEFAULT_MULTICAST_LOOP 1 /* normally hear sends if a member */
#define SCE_EENET_IP_MAX_MEMBERSHIPS 20 /* per socket; must fit in one mbuf */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct sceEENetIpMreq {
	struct	sceEENetInAddr imr_multiaddr;	/* IP multicast address of group */
	struct	sceEENetInAddr imr_interface;	/* local IP address of interface */
};

/*
 * Argument for IP_PORTRANGE:
 * - which range to search when port is unspecified at bind() or connect()
 */
#define SCE_EENET_IP_PORTRANGE_DEFAULT 0 /* default range */
#define SCE_EENET_IP_PORTRANGE_HIGH 1 /* same as DEFAULT (FreeBSD compat) */
#define SCE_EENET_IP_PORTRANGE_LOW 2 /* use privileged range */

/*
 * Definitions for inet sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */
#define SCE_EENET_IPPROTO_MAXID (SCE_EENET_IPPROTO_AH + 1) /* don't list to IPPROTO_MAX */

#define SCE_EENET_CTL_IPPROTO_NAMES { \
	{ "ip", CTLTYPE_NODE }, \
	{ "icmp", CTLTYPE_NODE }, \
	{ "igmp", CTLTYPE_NODE }, \
	{ "ggp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "tcp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ "egp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "pup", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "udp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "idp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "ipsec", CTLTYPE_NODE }, \
}

/*
 * Names for IP sysctl objects
 */
#define SCE_EENET_IPCTL_FORWARDING 1 /* act as router */
#define SCE_EENET_IPCTL_SENDREDIRECTS 2 /* may send redirects when forwarding */
#define SCE_EENET_IPCTL_DEFTTL 3 /* default TTL */
#ifdef notyet
#define SCE_EENET_IPCTL_DEFMTU 4 /* default MTU */
#endif
#define SCE_EENET_IPCTL_FORWSRCRT 5 /* forward source-routed packets */
#define SCE_EENET_IPCTL_DIRECTEDBCAST 6 /* default broadcast behavior */
#define SCE_EENET_IPCTL_ALLOWSRCRT 7 /* allow/drop all source-routed pkts */
#define SCE_EENET_IPCTL_SUBNETSARELOCAL 8 /* treat subnets as local addresses */
#define SCE_EENET_IPCTL_MTUDISC 9 /* allow path MTU discovery */
#define SCE_EENET_IPCTL_ANONPORTMIN 10 /* minimum ephemeral port */
#define SCE_EENET_IPCTL_ANONPORTMAX 11 /* maximum ephemeral port */
#define SCE_EENET_IPCTL_MTUDISCTIMEOUT 12 /* allow path MTU discovery */
#define SCE_EENET_IPCTL_MAXFLOWS 13 /* maximum ip flows allowed */
#define SCE_EENET_IPCTL_HOSTZEROBROADCAST 14 /* is host zero a broadcast addr? */
#define SCE_EENET_IPCTL_GIF_TTL 15 /* default TTL for gif encap packet */
#define SCE_EENET_IPCTL_LOWPORTMIN 16 /* minimum reserved port */
#define SCE_EENET_IPCTL_LOWPORTMAX 17 /* maximum reserved port */
#define SCE_EENET_IPCTL_MAXFRAGPACKETS 18 /* max packets in reassembly queue */
#define SCE_EENET_IPCTL_MAXID 19

#define SCE_EENET_IPCTL_NAMES { \
	{ 0, 0 }, \
	{ "forwarding", CTLTYPE_INT }, \
	{ "redirect", CTLTYPE_INT }, \
	{ "ttl", CTLTYPE_INT }, \
	{ "mtu", CTLTYPE_INT }, \
	{ "forwsrcrt", CTLTYPE_INT }, \
	{ "directed-broadcast", CTLTYPE_INT }, \
	{ "allowsrcrt", CTLTYPE_INT }, \
	{ "subnetsarelocal", CTLTYPE_INT }, \
	{ "mtudisc", CTLTYPE_INT }, \
	{ "anonportmin", CTLTYPE_INT }, \
	{ "anonportmax", CTLTYPE_INT }, \
	{ "mtudisctimeout", CTLTYPE_INT }, \
	{ "maxflows", CTLTYPE_INT }, \
	{ "hostzerobroadcast", CTLTYPE_INT }, \
	{ "gifttl", CTLTYPE_INT }, \
	{ "lowportmin", CTLTYPE_INT }, \
	{ "lowportmax", CTLTYPE_INT }, \
	{ "maxfragpackets", CTLTYPE_INT }, \
}

/* INET6 stuff */
#define __KAME_NETINET_IN_H_INCLUDED_
#include <netinet6/in6.h>
#undef __KAME_NETINET_IN_H_INCLUDED_

#endif /* !SCE_EENET__NETINET_IN_H_ */
