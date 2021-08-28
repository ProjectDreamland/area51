/*	$NetBSD: in6.h,v 1.15.2.4 2000/10/18 03:14:45 tv Exp $	*/
/*	$KAME: in6.h,v 1.57 2000/08/26 10:00:45 itojun Exp $	*/

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

#ifndef __KAME_NETINET_IN_H_INCLUDED_
#error "do not include netinet6/in6.h directly, include netinet/in.h"
#endif

#ifndef _NETINET6_IN6_H_
#define _NETINET6_IN6_H_

/*
 * Identification of the network protocol stack
 */
#define __KAME__
#define __KAME_VERSION		"19991213/NetBSD-current"

/*
 * Local port number conventions:
 *
 * Ports < IPPORT_RESERVED are reserved for privileged processes (e.g. root),
 * unless a kernel is compiled with IPNOPRIVPORTS defined.
 *
 * When a user does a bind(2) or connect(2) with a port number of zero,
 * a non-conflicting local port address is chosen.
 *
 * The default range is IPPORT_ANONMIN to IPPORT_ANONMAX, although
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

#define	IPV6PORT_RESERVED	1024
#define	IPV6PORT_ANONMIN	49152
#define	IPV6PORT_ANONMAX	65535
#define	IPV6PORT_RESERVEDMIN	600
#define	IPV6PORT_RESERVEDMAX	(IPV6PORT_RESERVED-1)

/*
 * IPv6 address
 */
struct sceEENetIn6Addr {
	union {
		u_int8_t   __u6_addr8[16];
		u_int16_t  __u6_addr16[8];
		u_int32_t  __u6_addr32[4];
	} __u6_addr;			/* 128-bit IP6 address */
};

#define s6_addr   __u6_addr.__u6_addr8

#define INET6_ADDRSTRLEN	46

/*
 * Socket address for IPv6
 */
struct sceEENetSockaddrIn6 {
	u_int8_t	sin6_len;	/* length of this struct(sa_family_t)*/
	u_int8_t	sin6_family;	/* AF_INET6 (sa_family_t) */
	u_int16_t	sin6_port;	/* Transport layer port # (in_port_t)*/
	u_int32_t	sin6_flowinfo;	/* IP6 flow information */
	struct sceEENetIn6Addr	sin6_addr;	/* IP6 address */
	u_int32_t	sin6_scope_id;	/* intface scope id */
};

/*
 * Equality
 */
#define IN6_ARE_ADDR_EQUAL(a, b)			\
    (memcmp(&(a)->s6_addr[0], &(b)->s6_addr[0], sizeof(struct sceEENetIn6Addr)) == 0)

/*
 * Unspecified
 */
#define IN6_IS_ADDR_UNSPECIFIED(a)	\
	((*(u_int32_t *)(&(a)->s6_addr[0]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[4]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[8]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[12]) == 0))

/*
 * Loopback
 */
#define IN6_IS_ADDR_LOOPBACK(a)		\
	((*(u_int32_t *)(&(a)->s6_addr[0]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[4]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[8]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[12]) == ntohl(1)))

/*
 * IPv4 compatible
 */
#define IN6_IS_ADDR_V4COMPAT(a)		\
	((*(u_int32_t *)(&(a)->s6_addr[0]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[4]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[8]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[12]) != 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[12]) != ntohl(1)))

/*
 * Mapped
 */
#define IN6_IS_ADDR_V4MAPPED(a)		      \
	((*(u_int32_t *)(&(a)->s6_addr[0]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[4]) == 0) &&	\
	 (*(u_int32_t *)(&(a)->s6_addr[8]) == ntohl(0x0000ffff)))

/*
 * KAME Scope Values
 */

#define __IPV6_ADDR_SCOPE_NODELOCAL	0x01
#define __IPV6_ADDR_SCOPE_LINKLOCAL	0x02
#define __IPV6_ADDR_SCOPE_SITELOCAL	0x05
#define __IPV6_ADDR_SCOPE_ORGLOCAL	0x08	/* just used in this file */
#define __IPV6_ADDR_SCOPE_GLOBAL	0x0e

/*
 * Unicast Scope
 * Note that we must check topmost 10 bits only, not 16 bits (see RFC2373).
 */
#define IN6_IS_ADDR_LINKLOCAL(a)	\
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0x80))
#define IN6_IS_ADDR_SITELOCAL(a)	\
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0xc0))

/*
 * Multicast
 */
#define IN6_IS_ADDR_MULTICAST(a)	((a)->s6_addr[0] == 0xff)

#define __IPV6_ADDR_MC_SCOPE(a)		((a)->s6_addr[1] & 0x0f)

/*
 * Multicast Scope
 */
#define IN6_IS_ADDR_MC_NODELOCAL(a)	\
	(IN6_IS_ADDR_MULTICAST(a) &&	\
	 (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_NODELOCAL))
#define IN6_IS_ADDR_MC_LINKLOCAL(a)	\
	(IN6_IS_ADDR_MULTICAST(a) &&	\
	 (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_LINKLOCAL))
#define IN6_IS_ADDR_MC_SITELOCAL(a)	\
	(IN6_IS_ADDR_MULTICAST(a) && 	\
	 (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_SITELOCAL))
#define IN6_IS_ADDR_MC_ORGLOCAL(a)	\
	(IN6_IS_ADDR_MULTICAST(a) &&	\
	 (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_ORGLOCAL))
#define IN6_IS_ADDR_MC_GLOBAL(a)	\
	(IN6_IS_ADDR_MULTICAST(a) &&	\
	 (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_GLOBAL))

/*
 * Options for use with [gs]etsockopt at the IPV6 level.
 * First word of comment is data type; bool is stored in int.
 */
#define IPV6_OPTIONS		1  /* buf/ip6_opts; set/get IP6 options */
/* no hdrincl */
#define IPV6_SOCKOPT_RESERVED1	3  /* reserved for future use */
#define IPV6_UNICAST_HOPS	4  /* int; IP6 hops */
#define IPV6_RECVOPTS		5  /* bool; receive all IP6 opts w/dgram */
#define IPV6_RECVRETOPTS	6  /* bool; receive IP6 opts for response */
#define IPV6_RECVDSTADDR	7  /* bool; receive IP6 dst addr w/dgram */
#define IPV6_RETOPTS		8  /* ip6_opts; set/get IP6 options */
#define IPV6_MULTICAST_IF	9  /* u_char; set/get IP6 multicast i/f  */
#define IPV6_MULTICAST_HOPS	10 /* u_char; set/get IP6 multicast hops */
#define IPV6_MULTICAST_LOOP	11 /* u_char; set/get IP6 multicast loopback */
#define IPV6_JOIN_GROUP		12 /* ip6_mreq; join a group membership */
#define IPV6_LEAVE_GROUP	13 /* ip6_mreq; leave a group membership */
#define IPV6_PORTRANGE		14 /* int; range to choose for unspec port */
#define ICMP6_FILTER		18 /* icmp6_filter; icmp6 filter */
#define IPV6_PKTINFO		19 /* bool; send/rcv if, src/dst addr */
#define IPV6_HOPLIMIT		20 /* bool; hop limit */
#define IPV6_NEXTHOP		21 /* bool; next hop addr */
#define IPV6_HOPOPTS		22 /* bool; hop-by-hop option */
#define IPV6_DSTOPTS		23 /* bool; destination option */
#define IPV6_RTHDR		24 /* bool; routing header */
#define IPV6_PKTOPTIONS		25 /* buf/cmsghdr; set/get IPv6 options */
#define IPV6_CHECKSUM		26 /* int; checksum offset for raw socket */
#define IPV6_BINDV6ONLY		27 /* bool; only bind INET6 at null bind */

#if 1 /*IPSEC*/
#define IPV6_IPSEC_POLICY	28 /* struct; get/set security policy */
#endif
#define IPV6_FAITH		29 /* bool; accept FAITH'ed connections */
/* to define items, should talk with KAME guys first, for *BSD compatibility */

#define IPV6_RTHDR_LOOSE     0 /* this hop need not be a neighbor. XXX old spec */
#define IPV6_RTHDR_STRICT    1 /* this hop must be a neighbor. XXX old spec */
#define IPV6_RTHDR_TYPE_0    0 /* IPv6 routing header type 0 */

/*
 * Defaults and limits for options
 */
#define IPV6_DEFAULT_MULTICAST_HOPS 1	/* normally limit m'casts to 1 hop  */
#define IPV6_DEFAULT_MULTICAST_LOOP 1	/* normally hear sends if a member  */

/*
 * Argument structure for IPV6_JOIN_GROUP and IPV6_LEAVE_GROUP.
 */
struct sceEENetIpv6Mreq {
	struct sceEENetIn6Addr	ipv6mr_multiaddr;
	unsigned int	ipv6mr_interface;
};

/*
 * IPV6_PKTINFO: Packet information(RFC2292 sec 5)
 */
struct sceEENetIn6Pktinfo {
	struct sceEENetIn6Addr	ipi6_addr;	/* src/dst IPv6 address */
	unsigned int	ipi6_ifindex;	/* send/recv interface index */
};

/*
 * Argument for IPV6_PORTRANGE:
 * - which range to search when port is unspecified at bind() or connect()
 */
#define	IPV6_PORTRANGE_DEFAULT	0	/* default range */
#define	IPV6_PORTRANGE_HIGH	1	/* "high" - request firewall bypass */
#define	IPV6_PORTRANGE_LOW	2	/* "low" - vouchsafe security */

/*
 * Definitions for inet6 sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */
#define IPV6PROTO_MAXID	(IPPROTO_PIM + 1)	/* don't list to IPV6PROTO_MAX */

#define CTL_IPV6PROTO_NAMES { \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ "tcp6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "udp6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ "ip6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ "ipsec6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "icmp6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "pim6", CTLTYPE_NODE }, \
}

/*
 * Names for IP sysctl objects
 */
#define IPV6CTL_FORWARDING	1	/* act as router */
#define IPV6CTL_SENDREDIRECTS	2	/* may send redirects when forwarding*/
#define IPV6CTL_DEFHLIM		3	/* default Hop-Limit */
#ifdef notyet
#define IPV6CTL_DEFMTU		4	/* default MTU */
#endif
#define IPV6CTL_FORWSRCRT	5	/* forward source-routed dgrams */
#define IPV6CTL_STATS		6	/* stats */
#define IPV6CTL_MRTSTATS	7	/* multicast forwarding stats */
#define IPV6CTL_MRTPROTO	8	/* multicast routing protocol */
#define IPV6CTL_MAXFRAGPACKETS	9	/* max packets reassembly queue */
#define IPV6CTL_SOURCECHECK	10	/* verify source route and intf */
#define IPV6CTL_SOURCECHECK_LOGINT 11	/* minimume logging interval */
#define IPV6CTL_ACCEPT_RTADV	12
#define IPV6CTL_KEEPFAITH	13
#define IPV6CTL_LOG_INTERVAL	14
#define IPV6CTL_HDRNESTLIMIT	15
#define IPV6CTL_DAD_COUNT	16
#define IPV6CTL_AUTO_FLOWLABEL	17
#define IPV6CTL_DEFMCASTHLIM	18
#define IPV6CTL_GIF_HLIM	19	/* default HLIM for gif encap packet */
#define IPV6CTL_KAME_VERSION	20
#define IPV6CTL_USE_DEPRECATED	21	/* use deprecated addr (RFC2462 5.5.4) */
#define IPV6CTL_RR_PRUNE	22	/* walk timer for router renumbering */
#define IPV6CTL_BINDV6ONLY	24
/* 25 to 27: reserved */
#define IPV6CTL_ANONPORTMIN	28	/* minimum ephemeral port */
#define IPV6CTL_ANONPORTMAX	29	/* maximum ephemeral port */
#define IPV6CTL_LOWPORTMIN	30	/* minimum reserved port */
#define IPV6CTL_LOWPORTMAX	31	/* maximum reserved port */
/* New entries should be added here from current IPV6CTL_MAXID value. */
/* to define items, should talk with KAME guys first, for *BSD compatibility */
#define IPV6CTL_STAT		32	/* IPv6 statistics */
#define IPV6CTL_MAXID		33

#define IPV6CTL_NAMES { \
	{ 0, 0 }, \
	{ "forwarding", CTLTYPE_INT }, \
	{ "redirect", CTLTYPE_INT }, \
	{ "hlim", CTLTYPE_INT }, \
	{ "mtu", CTLTYPE_INT }, \
	{ "forwsrcrt", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "mrtproto", CTLTYPE_INT }, \
	{ "maxfragpackets", CTLTYPE_INT }, \
	{ "sourcecheck", CTLTYPE_INT }, \
	{ "sourcecheck_logint", CTLTYPE_INT }, \
	{ "accept_rtadv", CTLTYPE_INT }, \
	{ "keepfaith", CTLTYPE_INT }, \
	{ "log_interval", CTLTYPE_INT }, \
	{ "hdrnestlimit", CTLTYPE_INT }, \
	{ "dad_count", CTLTYPE_INT }, \
	{ "auto_flowlabel", CTLTYPE_INT }, \
	{ "defmcasthlim", CTLTYPE_INT }, \
	{ "gifhlim", CTLTYPE_INT }, \
	{ "kame_version", CTLTYPE_STRING }, \
	{ "use_deprecated", CTLTYPE_INT }, \
	{ "rr_prune", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ "bindv6only", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "anonportmin", CTLTYPE_INT }, \
	{ "anonportmax", CTLTYPE_INT }, \
	{ "lowportmin", CTLTYPE_INT }, \
	{ "lowportmax", CTLTYPE_INT }, \
	{ "stat", CTLTYPE_STRUCT }, \
}

#endif /* !_NETINET6_IN6_H_ */
