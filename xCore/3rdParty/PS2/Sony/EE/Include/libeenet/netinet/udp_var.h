/*	$NetBSD: udp_var.h,v 1.17 1999/11/20 00:38:00 thorpej Exp $	*/

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
 *	@(#)udp_var.h	8.1 (Berkeley) 6/10/93
 */

#ifndef SCE_EENET__NETINET_UDP_VAR_H_
#define SCE_EENET__NETINET_UDP_VAR_H_

struct	sceEENetUdpstat {
					/* input statistics: */
	u_quad_t udps_ipackets;		/* total input packets */
	u_quad_t udps_hdrops;		/* packet shorter than header */
	u_quad_t udps_badsum;		/* checksum error */
	u_quad_t udps_badlen;		/* data length larger than packet */
	u_quad_t udps_noport;		/* no socket on port */
	u_quad_t udps_noportbcast;	/* of above, arrived as broadcast */
	u_quad_t udps_fullsock;		/* not delivered, input socket full */
	u_quad_t udps_pcbhashmiss;	/* input packets missing pcb hash */
					/* output statistics: */
	u_quad_t udps_opackets;		/* total output packets */
};

struct sceEENetUdpstat *__sceEENetUdpstat(void);

#define sceEENetudpstat	(__sceEENetUdpstat())

struct sceEENetUdpcbstat {
	u_long		us_so_snd_sb_cc;/* actual chars in send buffer */
	u_long		us_so_rcv_sb_cc;/* actual chars in recv buffer */
	struct sceEENetInAddr	us_inp_laddr;	/* local address */
	struct sceEENetInAddr	us_inp_faddr;	/* foreign address */
	u_int16_t	us_inp_lport;	/* local port */
	u_int16_t	us_inp_fport;	/* foreign port */
};

int sceEENetGetUdpcbstat(struct sceEENetUdpcbstat *, int *);

/*
 * Names for UDP sysctl objects
 */
#define SCE_EENET_UDPCTL_CHECKSUM 1 /* checksum UDP packets */
#define SCE_EENET_UDPCTL_SENDSPACE 2 /* default send buffer */
#define SCE_EENET_UDPCTL_RECVSPACE 3 /* default recv buffer */
#define SCE_EENET_UDPCTL_STAT 4 /* UDP statistics */
#define SCE_EENET_UDPCTL_MAXID 5

#define SCE_EENET_UDPCTL_NAMES { \
	{ 0, 0 }, \
	{ "checksum", CTLTYPE_INT }, \
	{ "sendspace", CTLTYPE_INT }, \
	{ "recvspace", CTLTYPE_INT }, \
	{ "stat", CTLTYPE_STRUCT }, \
}

#endif /* SCE_EENET__NETINET_UDP_VAR_H_ */
