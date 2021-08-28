/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       <libeenet - select.h>
 *                     <definitionx for select()>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Dec,10,2002     komaki      first version
 */

#ifndef _SYS_SELECT_H_
#define	_SYS_SELECT_H_

#include <sys/types.h>
#include <string.h>

#define sceEENetFD_SETSIZE	256

typedef int32_t sceEENetFdMask;

#define sceEENetNFDBITS		32	/* sizeof(sceEEnetFdMask) * 8 */
#define sceEENetNFDBITS_SHIFT	5	/* 2^5 = 32 */
#define sceEENetNFDBITS_MASK	0x1f	/* 5 bit mask */

typedef struct sceEENetFdSet {
	sceEENetFdMask		fds_bits[8 /*howmany(sceEENetFD_SETSIZE, sceEENetNFDBITS)*/];
} sceEENetFdSet;

#define sceEENetFD_SET(n, p) \
	((p)->fds_bits[(n)>>sceEENetNFDBITS_SHIFT] |= (1 << ((n) & sceEENetNFDBITS_MASK)))
#define sceEENetFD_CLR(n, p) \
	((p)->fds_bits[(n)>>sceEENetNFDBITS_SHIFT] &= ~(1 << ((n) & sceEENetNFDBITS_MASK)))
#define sceEENetFD_ISSET(n, p) \
	((p)->fds_bits[(n)>>sceEENetNFDBITS_SHIFT] & (1 << ((n) & sceEENetNFDBITS_MASK)))
#define sceEENetFD_ZERO(p) \
	(void)memset((p), 0, sizeof(*(p)))

/* for BSD API compatibility */
#undef FD_SETSIZE
#define FD_SETSIZE sceEENetFD_SETSIZE
#undef fd_mask
#define fd_mask sceEENetFdMask
#undef NFDBITS
#define NFDBITS sceEENetNFDBITS
#undef NFDBITS_SHIFT
#define NFDBITS_SHIFT sceEENetNFDBITS_SHIFT
#undef NFDBITS_MASK
#define NFDBITS_MASK sceEENetNFDBITS_MASK
#undef fd_set
#define fd_set sceEENetFdSet
#undef FD_SET
#define FD_SET sceEENetFD_SET
#undef FD_CLR
#define FD_CLR sceEENetFD_CLR
#undef FD_ISSET
#define FD_ISSET sceEENetFD_ISSET
#undef FD_ZERO
#define FD_ZERO sceEENetFD_ZERO

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int	sceEENetSelect(int nfds, sceEENetFdSet *readfds,
		sceEENetFdSet *writefds, sceEENetFdSet *exceptfds,
		struct timeval *timeout);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* !_SYS_SELECT_H_ */
