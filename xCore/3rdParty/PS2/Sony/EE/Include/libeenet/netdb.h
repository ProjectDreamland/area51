/*	$NetBSD: netdb.h,v 1.18.2.3 2001/05/01 10:28:45 he Exp $	*/

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

/*-
 * Copyright (c) 1980, 1983, 1988, 1993
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
 *      @(#)netdb.h	8.1 (Berkeley) 6/2/93
 *	Id: netdb.h,v 4.9.1.2 1993/05/17 09:59:01 vixie Exp
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

#ifndef SCE_EENET__NETDB_H_
#define SCE_EENET__NETDB_H_

extern int h_errno;

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct	sceEENetHostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	SCE_EENET_h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

#if defined(EENET46)
struct sceEENetAddrinfo {
	int	ai_flags;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
	int	ai_family;	/* PF_xxx */
	int	ai_socktype;	/* SOCK_xxx */
	int	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
	size_t	ai_addrlen;	/* length of ai_addr */
	char	*ai_canonname;	/* canonical name for hostname */
	struct sceEENetSockaddr *ai_addr;	/* binary address */
	struct sceEENetAddrinfo *ai_next;	/* next structure in linked list */
};
#endif

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	SCE_EENET_NETDB_INTERNAL	-1	/* see errno */
#define	SCE_EENET_NETDB_SUCCESS	0	/* no problem */
#define	SCE_EENET_HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	SCE_EENET_TRY_AGAIN	2 /* Non-Authoritative Host not found, or SERVERFAIL */
#define	SCE_EENET_NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	SCE_EENET_NO_DATA		4 /* Valid name, no data record of requested type */
#define	SCE_EENET_NO_ADDRESS	NO_DATA		/* no address, look for MX record */

#if defined(EENET46)
/*
 * Error return codes from getaddrinfo()
 */
#define	SCE_EENET_EAI_ADDRFAMILY	 1	/* address family for hostname not supported */
#define	SCE_EENET_EAI_AGAIN	 2	/* temporary failure in name resolution */
#define	SCE_EENET_EAI_BADFLAGS	 3	/* invalid value for ai_flags */
#define	SCE_EENET_EAI_FAIL	 4	/* non-recoverable failure in name resolution */
#define	SCE_EENET_EAI_FAMILY	 5	/* ai_family not supported */
#define	SCE_EENET_EAI_MEMORY	 6	/* memory allocation failure */
#define	SCE_EENET_EAI_NODATA	 7	/* no address associated with hostname */
#define	SCE_EENET_EAI_NONAME	 8	/* hostname nor servname provided, or not known */
#define	SCE_EENET_EAI_SERVICE	 9	/* servname not supported for ai_socktype */
#define	SCE_EENET_EAI_SOCKTYPE	10	/* ai_socktype not supported */
#define	SCE_EENET_EAI_SYSTEM	11	/* system error returned in errno */
#define SCE_EENET_EAI_BADHINTS	12
#define SCE_EENET_EAI_PROTOCOL	13
#define SCE_EENET_EAI_MAX		14

/*
 * Flag values for getaddrinfo()
 */
#define	SCE_EENET_AI_PASSIVE	0x00000001 /* get address to use bind() */
#define	SCE_EENET_AI_CANONNAME	0x00000002 /* fill ai_canonname */
#define	SCE_EENET_AI_NUMERICHOST	0x00000004 /* prevent name resolution */
/* valid flags for addrinfo */
#define	SCE_EENET_AI_MASK		(SCE_EENET_AI_PASSIVE | SCE_EENET_AI_CANONNAME | SCE_EENET_AI_NUMERICHOST)

#define	SCE_EENET_AI_ALL		0x00000100 /* IPv6 and IPv4-mapped (with AI_V4MAPPED) */
#define	SCE_EENET_AI_V4MAPPED_CFG	0x00000200 /* accept IPv4-mapped if kernel supports */
#define	SCE_EENET_AI_ADDRCONFIG	0x00000400 /* only if any address is assigned */
#define	SCE_EENET_AI_V4MAPPED	0x00000800 /* accept IPv4-mapped IPv6 address */
/* special recommended flags for getipnodebyname */
#define	SCE_EENET_AI_DEFAULT	(SCE_EENET_AI_V4MAPPED_CFG | SCE_EENET_AI_ADDRCONFIG)

/*
 * Constants for getnameinfo()
 */
#define	SCE_EENET_NI_MAXHOST	1025
#define	SCE_EENET_NI_MAXSERV	32

/*
 * Flag values for getnameinfo()
 */
#define	SCE_EENET_NI_NOFQDN	0x00000001
#define	SCE_EENET_NI_NUMERICHOST	0x00000002
#define	SCE_EENET_NI_NAMEREQD	0x00000004
#define	SCE_EENET_NI_NUMERICSERV	0x00000008
#define	SCE_EENET_NI_DGRAM	0x00000010
#define SCE_EENET_NI_WITHSCOPEID	0x00000020	/*KAME extension*/

/*
 * Scope delimit character
 */
#define SCE_EENET_SCOPE_DELIMITER '%'		/*KAME extension*/
#endif

/*
 * Data types
 */
#include <sys/ansi.h>
#ifndef sceEENetSocklen_t
typedef __socklen_t	sceEENetSocklen_t;
#define sceEENetSocklen_t	__socklen_t
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

struct sceEENetHostent	*sceEENetGethostbyaddr (const char *, int, int);
struct sceEENetHostent	*sceEENetGethostbyname (const char *);
struct sceEENetHostent	*sceEENetGethostbyname2 (const char *, int);
void		sceEENetHerror (const char *);
const char	*sceEENetHstrerror (int);

#if defined(EENET46)
int		sceEENetGetaddrinfo (const char *, const char *,
				 const struct sceEENetAddrinfo *, struct sceEENetAddrinfo **);
int		sceEENetGetnameinfo (const struct sceEENetSockaddr *, sceEENetSocklen_t, char *,
				 size_t, char *, size_t, int);
void		sceEENetFreeaddrinfo (struct sceEENetAddrinfo *);
char		*sceEENetGaiStrerror (int);
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !SCE_EENET__NETDB_H_ */
