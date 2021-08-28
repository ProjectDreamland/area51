/*	$NetBSD: poll.h,v 1.6 1998/09/13 14:46:24 christos Exp $	*/

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum.
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

#ifndef SCE_EENET__SYS_POLL_H_
#define SCE_EENET__SYS_POLL_H_

typedef unsigned int	nfds_t;

struct sceEENetPollfd {
	int	fd;		/* file descriptor */
	short	events;		/* events to look for */
	short	revents;	/* events returned */
};

/*
 * Testable events (may be specified in events field).
 */
#define SCE_EENET_POLLIN 0x0001
#define SCE_EENET_POLLPRI 0x0002
#define SCE_EENET_POLLOUT 0x0004
#define SCE_EENET_POLLRDNORM 0x0040
#define SCE_EENET_POLLWRNORM SCE_EENET_POLLOUT
#define SCE_EENET_POLLRDBAND 0x0080
#define SCE_EENET_POLLWRBAND 0x0100

/*
 * Non-testable events (may not be specified in events field).
 */
#define SCE_EENET_POLLERR 0x0008
#define SCE_EENET_POLLHUP 0x0010
#define SCE_EENET_POLLNVAL 0x0020

/*
 * Infinite timeout value.
 */
#define SCE_EENET_INFTIM -1

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int	sceEENetPoll (struct sceEENetPollfd *, nfds_t, int);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !SCE_EENET__SYS_POLL_H_ */
