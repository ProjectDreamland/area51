/*	$NetBSD: tcp_fsm.h,v 1.10 1998/07/09 05:49:56 mycroft Exp $	*/

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
 *	@(#)tcp_fsm.h	8.1 (Berkeley) 6/10/93
 */

#ifndef SCE_EENET__NETINET_TCP_FSM_H_
#define SCE_EENET__NETINET_TCP_FSM_H_

/*
 * TCP FSM state definitions.
 * Per RFC793, September, 1981.
 */

#define SCE_EENET_TCP_NSTATES 11

#define SCE_EENET_TCPS_CLOSED 0 /* closed */
#define SCE_EENET_TCPS_LISTEN 1 /* listening for connection */
#define SCE_EENET_TCPS_SYN_SENT 2 /* active, have sent syn */
#define SCE_EENET_TCPS_SYN_RECEIVED 3 /* have send and received syn */
/* states < TCPS_ESTABLISHED are those where connections not established */
#define SCE_EENET_TCPS_ESTABLISHED 4 /* established */
#define SCE_EENET_TCPS_CLOSE_WAIT 5 /* rcvd fin, waiting for close */
/* states > TCPS_CLOSE_WAIT are those where user has closed */
#define SCE_EENET_TCPS_FIN_WAIT_1 6 /* have closed, sent fin */
#define SCE_EENET_TCPS_CLOSING 7 /* closed xchd FIN; await ACK */
#define SCE_EENET_TCPS_LAST_ACK 8 /* had fin and close; await FIN ACK */
/* states > TCPS_CLOSE_WAIT && < TCPS_FIN_WAIT_2 await ACK of FIN */
#define SCE_EENET_TCPS_FIN_WAIT_2 9 /* have closed, fin is acked */
#define SCE_EENET_TCPS_TIME_WAIT 10 /* in 2*msl quiet wait after close */

#endif /* SCE_EENET__NETINET_TCP_FSM_H_ */
