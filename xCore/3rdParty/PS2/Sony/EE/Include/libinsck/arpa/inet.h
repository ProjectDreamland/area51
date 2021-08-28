/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                 <libinsck - libinsck/arpa/inet.h>
 *                <header for socket address functions>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           May,17,2001     komaki      first version
 */

#ifndef __INSOCK_ARPA_INET_H__
#define	__INSOCK_ARPA_INET_H__

#include <sys/types.h>

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

u_int			sceInsockInetAddr(const char *);
int			sceInsockInetAton(const char *, sceInsockInAddr_t *);
u_int			sceInsockInetLnaof(sceInsockInAddr_t);
sceInsockInAddr_t	sceInsockInetMakeaddr(u_int , u_int);
u_int			sceInsockInetNetof(sceInsockInAddr_t);
u_int			sceInsockInetNetwork(const char *);
char *			sceInsockInetNtoa(sceInsockInAddr_t);
const char *	sceInsockInetNtop(int, const void *, char *, size_t);

#if !defined(sceInsockDisableSocketSymbolAliases)
#define inet_addr	sceInsockInetAddr
#define inet_aton	sceInsockInetAton
#define inet_lnaof	sceInsockInetLnaof
#define inet_makeaddr	sceInsockInetMakeaddr
#define inet_netof	sceInsockInetNetof
#define inet_network	sceInsockInetNetwork
#define inet_ntoa	sceInsockInetNtoa
#define inet_ntop	sceInsockInetNtop
#endif	/* !sceInsockDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __INSOCK_ARPA_INET_H__ */
