/*	$NetBSD: if.h,v 1.50.4.1 2000/12/31 17:57:43 jhawk Exp $	*/

/*-
 * Copyright (c) 1999, 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by William Studnemund and Jason R. Thorpe.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1989, 1993
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
 *	@(#)if.h	8.3 (Berkeley) 2/9/95
 */

#ifndef _NET_IF_H_
#define _NET_IF_H_

/*
 * Length of interface external name, including terminating '\0'.
 * Note: this is the same size as a generic device's external name.
 */
#define	IFNAMSIZ	16
#define	IF_NAMESIZE	IFNAMSIZ

/*
 * Structure used to query names of interface cloners.
 */
struct sceEENetIfClonereq {
	int	ifcr_total;		/* total cloners (out) */
	int	ifcr_count;		/* room for this many in user buffer */
	char	*ifcr_buffer;		/* buffer for cloner names */
};

/*
 * Structure defining statistics and other data kept regarding a network
 * interface.
 */
struct sceEENetIfData {
	/* generic interface information */
	u_char	ifi_type;		/* ethernet, tokenring, etc. */
	u_char	ifi_addrlen;		/* media address length */
	u_char	ifi_hdrlen;		/* media header length */
	int	ifi_link_state;		/* current link state */
	u_quad_t ifi_mtu;		/* maximum transmission unit */
	u_quad_t ifi_metric;		/* routing metric (external only) */
	u_quad_t ifi_baudrate;		/* linespeed */
	/* volatile statistics */
	u_quad_t ifi_ipackets;		/* packets received on interface */
	u_quad_t ifi_ierrors;		/* input errors on interface */
	u_quad_t ifi_opackets;		/* packets sent on interface */
	u_quad_t ifi_oerrors;		/* output errors on interface */
	u_quad_t ifi_collisions;	/* collisions on csma interfaces */
	u_quad_t ifi_ibytes;		/* total number of octets received */
	u_quad_t ifi_obytes;		/* total number of octets sent */
	u_quad_t ifi_imcasts;		/* packets received via multicast */
	u_quad_t ifi_omcasts;		/* packets sent via multicast */
	u_quad_t ifi_iqdrops;		/* dropped on input, this interface */
	u_quad_t ifi_noproto;		/* destined for unsupported protocol */
	struct	timeval ifi_lastchange;	/* last updated */
};

/*
 * Values for if_link_state.
 */
#define	LINK_STATE_UNKNOWN	0	/* link invalid/unknown */
#define	LINK_STATE_DOWN		1	/* link is down */
#define	LINK_STATE_UP		2	/* link is up */


/*
 * Message format for use in obtaining information about interfaces
 * from sysctl and the routing socket.
 */
struct sceEENetIfMsghdr {
	u_short	ifm_msglen;	/* to skip over non-understood messages */
	u_char	ifm_version;	/* future binary compatability */
	u_char	ifm_type;	/* message type */
	int	ifm_addrs;	/* like rtm_addrs */
	int	ifm_flags;	/* value of if_flags */
	u_short	ifm_index;	/* index for associated ifp */
	struct	sceEENetIfData ifm_data;/* statistics and other data about if */
};

/*
 * Message format for use in obtaining information about interface addresses
 * from sysctl and the routing socket.
 */
struct sceEENetIfaMsghdr {
	u_short	ifam_msglen;	/* to skip over non-understood messages */
	u_char	ifam_version;	/* future binary compatability */
	u_char	ifam_type;	/* message type */
	int	ifam_addrs;	/* like rtm_addrs */
	int	ifam_flags;	/* value of ifa_flags */
	u_short	ifam_index;	/* index for associated ifp */
	int	ifam_metric;	/* value of ifa_metric */
};

/*
 * Message format announcing the arrival or departure of a network interface.
 */
struct sceEENetIfAnnouncemsghdr {
	u_short	ifan_msglen;	/* to skip over non-understood messages */
	u_char	ifan_version;	/* future binary compatibility */
	u_char	ifan_type;	/* message type */
	u_short	ifan_index;	/* index for associated ifp */
	char	ifan_name[IFNAMSIZ]; /* if name, e.g. "en0" */
	u_short	ifan_what;	/* what type of announcement */
};

#define	IFAN_ARRIVAL	0	/* interface arrival */
#define	IFAN_DEPARTURE	1	/* interface departure */

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	sceEENetIfreq {
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sceEENetSockaddr ifru_addr;
		struct	sceEENetSockaddr ifru_dstaddr;
		struct	sceEENetSockaddr ifru_broadaddr;
		short	ifru_flags;
		int	ifru_metric;
		int	ifru_mtu;
		u_int	ifru_value;
		caddr_t	ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu */
#define	ifr_value	ifr_ifru.ifru_value	/* generic value */
#define	ifr_media	ifr_ifru.ifru_metric	/* media options (overload) */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};

struct sceEENetIfaliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sceEENetSockaddr ifra_addr;
	struct	sceEENetSockaddr ifra_dstaddr;
#define	ifra_broadaddr	ifra_dstaddr
	struct	sceEENetSockaddr ifra_mask;
};

struct sceEENetIfmediareq {
	char	ifm_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	int	ifm_current;			/* current media options */
	int	ifm_mask;			/* don't care mask */
	int	ifm_status;			/* media status */
	int	ifm_active;			/* active options */
	int	ifm_count;			/* # entries in ifm_ulist
						   array */
	int	*ifm_ulist;			/* media words */
};

struct  sceEENetIfdrv {
	char		ifd_name[IFNAMSIZ];	/* if name, e.g. "en0" */
	unsigned long	ifd_cmd;
	size_t		ifd_len;
	void		*ifd_data;
}; 

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	sceEENetIfconf {
	int	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	sceEENetIfreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};

/*
 * Structure for SIOC[AGD]LIFADDR
 */
struct sceEENetIfLaddrreq {
	char iflr_name[IFNAMSIZ];
	unsigned int flags;
#define IFLR_PREFIX	0x8000	/* in: prefix given  out: kernel fills id */
	unsigned int prefixlen;		/* in/out */
	struct sceEENetSockaddrStorage addr;	/* in/out */
	struct sceEENetSockaddrStorage dstaddr; /* out */
};

#include <net/if_arp.h>

#if defined(EENET46)
struct sceEENetIfNameindex {
	unsigned int	if_index;	/* 1, 2, ... */
	char		*if_name;	/* null terminated name: "le0", ... */
};
#endif

struct sceEENetIfname {
	char		ifn_name[IFNAMSIZ];
};

#define sceEENET_IFINFO_VENDOR_NAME	1
#define sceEENET_IFINFO_PRODUCT_NAME	2
#define sceEENET_IFINFO_IFTYPE		3
#define sceEENET_IFINFO_IFFLAGS		4
#define sceEENET_IFINFO_MACADDR		5
#define sceEENET_IFINFO_MEDIA		6
#define sceEENET_IFINFO_MTU		7
#define sceEENET_IFINFO_ADDR		8
#define sceEENET_IFINFO_NETMASK		9
#define sceEENET_IFINFO_DSTADDR		10
#define sceEENET_IFINFO_BROADADDR	sceEENET_IFINFO_DSTADDR

/* sceEENET_IFINFO_IFTYPE */
#define sceEENET_IFTYPE_UNKNOWN	0
#define sceEENET_IFTYPE_LOOP	1	/* Local Loopback */
#define sceEENET_IFTYPE_ETHER	2	/* Ethernet */
#define sceEENET_IFTYPE_PPP	3	/* PPP */
#define sceEENET_IFTYPE_PPPOE	4	/* PPPoE */

/* sceEENET_IFINFO_IFFLAGS */
#define IFF_UP		0x1	/* interface is up */
#define IFF_BROADCAST	0x2	/* broadcast address valid */
#define IFF_LOOPBACK	0x8	/* is a loopback net */
#define IFF_POINTOPOINT	0x10	/* interface is point-to-point link */
#define IFF_RUNNING	0x40	/* resources allocated */
#define IFF_PROMISC	0x100	/* receive all packets */
#define IFF_ALLMULTI	0x200	/* receive all multicast packets */
#define IFF_MULTICAST	0x8000	/* supports multicast */

/* sceEENET_IFINFO_MEDIA */
#define sceEENET_MEDIA_10_T	0x0100	/* 10baseT */
#define sceEENET_MEDIA_100_TX	0x0200	/* 100baseTX */
#define sceEENET_MEDIA_HDX	0x0010	/* Half duplex */
#define sceEENET_MEDIA_FDX	0x0020	/* Full duplex */
#define sceEENET_MEDIA_FLOW	0x0040	/* Flow control */
#define sceEENET_MEDIA_AVALID	0x0001	/* ACTIVE bit is valid */
#define sceEENET_MEDIA_ACTIVE	0x0002	/* active or no carrier */

struct sceEENetIfstat {
	u_quad_t ifs_ipackets;		/* packets received on interface */
	u_quad_t ifs_ierrors;		/* input errors on interface */
	u_quad_t ifs_opackets;		/* packets sent on interface */
	u_quad_t ifs_oerrors;		/* output errors on interface */
	u_quad_t ifs_collisions;	/* collisions on csma interfaces */
	u_quad_t ifs_ibytes;		/* total number of octets received */
	u_quad_t ifs_obytes;		/* total number of octets sent */
	u_quad_t ifs_imcasts;		/* packets received via multicast */
	u_quad_t ifs_omcasts;		/* packets sent via multicast */
	u_quad_t ifs_iqdrops;		/* dropped on input, this interface */
	u_quad_t ifs_noproto;		/* destined for unsupported protocol */
};

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#if defined(EENET46)
unsigned int sceEENetIfNametoindex (const char *);
char *	sceEENetIfIndextoname (unsigned int, char *);
struct	sceEENetIfNameindex * sceEENetIfNameindex (void);
void	sceEENetIfFreenameindex (struct sceEENetIfNameindex *);
#endif

int sceEENetGetIfnames(struct sceEENetIfname *, int *);
int sceEENetGetIfinfo(const char *, int, void *, int *);
int sceEENetGetIfstat(const char *, struct sceEENetIfstat *);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !_NET_IF_H_ */
