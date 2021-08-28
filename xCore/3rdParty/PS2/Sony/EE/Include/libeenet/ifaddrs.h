/*	$NetBSD: ifaddrs.h,v 1.4 2000/06/13 01:21:53 simonb Exp $	*/

/*
 * Copyright (c) 1995, 1999
 *	Berkeley Software Design, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY Berkeley Software Design, Inc. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Berkeley Software Design, Inc. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	BSDI ifaddrs.h,v 2.5 2000/02/23 14:51:59 dab Exp
 */

#ifndef	_IFADDRS_H_
#define	_IFADDRS_H_

struct sceEENetIfaddrs {
	struct sceEENetIfaddrs  *ifa_next;
	char		*ifa_name;
	u_int		 ifa_flags;
	struct sceEENetSockaddr	*ifa_addr;
	struct sceEENetSockaddr	*ifa_netmask;
	struct sceEENetSockaddr	*ifa_dstaddr;
	void		*ifa_data;
};

/*
 * This may have been defined in <net/if.h>.  Note that if <net/if.h> is
 * to be included it must be included before this header file.
 */
#ifndef	ifa_broadaddr
#define	ifa_broadaddr	ifa_dstaddr	/* broadcast address interface */
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int sceEENetGetifaddrs (struct sceEENetIfaddrs **);
void sceEENetFreeifaddrs (struct sceEENetIfaddrs *);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !_IFADDRS_H_ */
