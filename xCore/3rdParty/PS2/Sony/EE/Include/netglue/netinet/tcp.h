/* SCE CONFIDENTIAL
"PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                 <netglue - netglue/netinet/tcp.h>
 *                          <header for TCP>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Sep,21,2001     komaki      first version
 *       1.10           Apr,24,2003     ksh         prefix
 */

#ifndef __NETGLUE_NETINET_TCP_H__
#define __NETGLUE_NETINET_TCP_H__

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#define	SCE_NETGLUE_TCP_NODELAY	0x01

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define	TCP_NODELAY	SCE_NETGLUE_TCP_NODELAY
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __NETGLUE_NETINET_TCP_H__ */
