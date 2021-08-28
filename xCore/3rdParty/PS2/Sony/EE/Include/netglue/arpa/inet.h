/* SCE CONFIDENTIAL
"PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                 <netglue - netglue/arpa/inet.h>
 *                <header for socket address functions>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Sep,21,2001     komaki      first version
 */

#ifndef __NETGLUE_ARPA_INET_H__
#define	__NETGLUE_ARPA_INET_H__

#include <sys/types.h>

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

u_int			sceNetGlueInetAddr(const char *);
int			sceNetGlueInetAton(const char *, sceNetGlueInAddr_t *);
u_int			sceNetGlueInetLnaof(sceNetGlueInAddr_t);
sceNetGlueInAddr_t	sceNetGlueInetMakeaddr(u_int , u_int);
u_int			sceNetGlueInetNetof(sceNetGlueInAddr_t);
u_int			sceNetGlueInetNetwork(const char *);
char *			sceNetGlueInetNtoa(sceNetGlueInAddr_t);
const char *	sceNetGlueInetNtop(int, const void *, char *, size_t);

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define inet_addr	sceNetGlueInetAddr
#define inet_aton	sceNetGlueInetAton
#define inet_lnaof	sceNetGlueInetLnatof
#define inet_makeaddr	sceNetGlueInetMakeaddr
#define inet_netof	sceNetGlueInetNetof
#define inet_network	sceNetGlueInetNetwork
#define inet_ntoa	sceNetGlueInetNtoa
#define inet_ntop	sceNetGlueInetNtop
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __NETGLUE_ARPA_INET_H__ */
