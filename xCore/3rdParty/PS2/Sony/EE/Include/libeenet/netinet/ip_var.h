/*	$NetBSD: ip_var.h,v 1.41.4.1 2000/08/26 16:38:33 tron Exp $	*/

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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 */

#ifndef _NETINET_IP_VAR_H_
#define _NETINET_IP_VAR_H_

struct	sceEENetIpstat {
	u_quad_t ips_total;		/* total packets received */
	u_quad_t ips_badsum;		/* checksum bad */
	u_quad_t ips_tooshort;		/* packet too short */
	u_quad_t ips_toosmall;		/* not enough data */
	u_quad_t ips_badhlen;		/* ip header length < data size */
	u_quad_t ips_badlen;		/* ip length < ip header length */
	u_quad_t ips_fragments;		/* fragments received */
	u_quad_t ips_fragdropped;	/* frags dropped (dups, out of space) */
	u_quad_t ips_fragtimeout;	/* fragments timed out */
	u_quad_t ips_forward;		/* packets forwarded */
	u_quad_t ips_fastforward;	/* packets fast forwarded */
	u_quad_t ips_cantforward;	/* packets rcvd for unreachable dest */
	u_quad_t ips_redirectsent;	/* packets forwarded on same net */
	u_quad_t ips_noproto;		/* unknown or unsupported protocol */
	u_quad_t ips_delivered;		/* datagrams delivered to upper level*/
	u_quad_t ips_localout;		/* total ip packets generated here */
	u_quad_t ips_odropped;		/* lost packets due to nobufs, etc. */
	u_quad_t ips_reassembled;	/* total packets reassembled ok */
	u_quad_t ips_fragmented;	/* datagrams sucessfully fragmented */
	u_quad_t ips_ofragments;	/* output fragments created */
	u_quad_t ips_cantfrag;		/* don't fragment flag was set, etc. */
	u_quad_t ips_badoptions;	/* error in option processing */
	u_quad_t ips_noroute;		/* packets discarded due to no route */
	u_quad_t ips_badvers;		/* ip version != 4 */
	u_quad_t ips_rawout;		/* total raw ip packets generated */
	u_quad_t ips_badfrags;		/* malformed fragments (bad length) */
	u_quad_t ips_rcvmemdrop;	/* frags dropped for lack of memory */
	u_quad_t ips_toolong;		/* ip length > max ip packet size */
	u_quad_t ips_nogif;		/* no match gif found */
};

struct sceEENetIpstat *__sceEENetIpstat(void);

#define sceEENetipstat	(__sceEENetIpstat())

#endif /* _NETINET_IP_VAR_H_ */
