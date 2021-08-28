/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                          Libnet Library
 *
 *      Copyright (C) 2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *       Version   Date        Design   Log
 *  --------------------------------------------------------------------
 *       1.0.0     2003/04/24  ksh      first
 */

#if ( !defined __LIBNET_IN_H )
#define	__LIBNET_IN_H

#if defined(__cplusplus)
extern "C" {
#endif

u_int	sceLibnetHtonl(u_int);
u_short	sceLibnetHtons(u_short);
u_int	sceLibnetNtohl(u_int);
u_short	sceLibnetNtohs(u_short);

#if !defined(sceLibnetDisableSocketSymbolAliases)
#undef  htonl
#undef  htons
#undef  ntohl
#undef  ntohs
#define htonl   sceLibnetHtonl
#define htons   sceLibnetHtons
#define ntohl   sceLibnetNtohl
#define ntohs   sceLibnetNtohs
#endif	/* !sceLibnetDisableSocketSymbolAliases */

#if defined(__cplusplus)
}
#endif

#endif	/*** __LIBNET_IN_H ***/

