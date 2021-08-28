/*	$NetBSD: ip6_var.h,v 1.12.4.2 2000/08/27 01:25:08 itojun Exp $	*/
/*	$KAME: ip6_var.h,v 1.33 2000/06/11 14:59:20 jinmei Exp $	*/

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
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)ip_var.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _NETINET6_IP6_VAR_H_
#define _NETINET6_IP6_VAR_H_

struct	sceEENetIp6stat {
	u_quad_t ip6s_total;		/* total packets received */
	u_quad_t ip6s_tooshort;		/* packet too short */
	u_quad_t ip6s_toosmall;		/* not enough data */
	u_quad_t ip6s_fragments;	/* fragments received */
	u_quad_t ip6s_fragdropped;	/* frags dropped(dups, out of space) */
	u_quad_t ip6s_fragtimeout;	/* fragments timed out */
	u_quad_t ip6s_fragoverflow;	/* fragments that exceeded limit */
	u_quad_t ip6s_forward;		/* packets forwarded */
	u_quad_t ip6s_cantforward;	/* packets rcvd for unreachable dest */
	u_quad_t ip6s_redirectsent;	/* packets forwarded on same net */
	u_quad_t ip6s_delivered;	/* datagrams delivered to upper level*/
	u_quad_t ip6s_localout;		/* total ip packets generated here */
	u_quad_t ip6s_odropped;		/* lost packets due to nobufs, etc. */
	u_quad_t ip6s_reassembled;	/* total packets reassembled ok */
	u_quad_t ip6s_fragmented;	/* datagrams sucessfully fragmented */
	u_quad_t ip6s_ofragments;	/* output fragments created */
	u_quad_t ip6s_cantfrag;		/* don't fragment flag was set, etc. */
	u_quad_t ip6s_badoptions;	/* error in option processing */
	u_quad_t ip6s_noroute;		/* packets discarded due to no route */
	u_quad_t ip6s_badvers;		/* ip6 version != 6 */
	u_quad_t ip6s_rawout;		/* total raw ip packets generated */
	u_quad_t ip6s_badscope;		/* scope error */
	u_quad_t ip6s_notmember;	/* don't join this multicast group */
	u_quad_t ip6s_nxthist[256];	/* next header history */
	u_quad_t ip6s_m1;		/* one mbuf */
	u_quad_t ip6s_m2m[32];		/* two or more mbuf */
	u_quad_t ip6s_mext1;		/* one ext mbuf */
	u_quad_t ip6s_mext2m;		/* two or more ext mbuf */
	u_quad_t ip6s_exthdrtoolong;	/* ext hdr are not continuous */
	u_quad_t ip6s_nogif;		/* no match gif found */
	u_quad_t ip6s_toomanyhdr;	/* discarded due to too many headers */
};

struct sceEENetIp6stat *__sceEENetIp6stat(void);

#define sceEENetip6stat	(__sceEENetIp6stat())

#endif /* !_NETINET6_IP6_VAR_H_ */
