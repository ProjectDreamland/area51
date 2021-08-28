/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                    <libinsck - libinsck/netdb.h>
 *                     <header for DNS functions>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           May,17,2001     komaki      first version
 *       1.10           Apr,24,2003     mka, ksh    prefix
 */
				
#ifndef __INSOCK_NETDB_H__
#define __INSOCK_NETDB_H__

#include <sys/types.h>

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef struct sceInsockHostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	sceInsock_h_addr	h_addr_list[0]
				/* address, for backward compatibility */
} sceInsockHostent_t;

#if !defined(sceInsockDisableSocketSymbolAliases)
#define hostent	sceInsockHostent
#define h_addr	sceInsock_h_addr
#endif	/* !sceInsockDisableSocketSymbolAliases */

#define	SCE_INSOCK_NETDB_INTERNAL	-1	/* see errno */
#define	SCE_INSOCK_NETDB_SUCCESS	0	/* no problem */
#define	SCE_INSOCK_HOST_NOT_FOUND	1
			/* Authoritative Answer Host not found */
#define	SCE_INSOCK_TRY_AGAIN		2
			/* Non-Authoritative Host not found, or SERVERFAIL */
#define	SCE_INSOCK_NO_RECOVERY		3
			/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	SCE_INSOCK_NO_DATA		4
			/* Valid name, no data record of requested type */
#define	SCE_INSOCK_NO_ADDRESS		SCE_INSOCK_NO_DATA
			/* no address, look for MX record */

#if !defined(sceInsockDisableSocketSymbolAliases)
#define NETDB_INTERNAL	SCE_INSOCK_NETDB_INTERNAL
#define NETDB_SUCCESS	SCE_INSOCK_NETDB_SUCCESS
#define HOST_NOT_FOUND	SCE_INSOCK_HOST_NOT_FOUND
#define TRY_AGAIN	SCE_INSOCK_TRY_AGAIN
#define NO_RECOVERY	SCE_INSOCK_NO_RECOVERY
#define NO_DATA		SCE_INSOCK_NO_DATA	
#define NO_ADDRESS	SCE_INSOCK_NO_ADDRESS
#endif	/* !sceInsockDisableSocketSymbolAliases */

sceInsockHostent_t *sceInsockGethostbyaddr(const char *, int, int);
sceInsockHostent_t *sceInsockGethostbyname(const char *);

#if !defined(sceInsockDisableSocketSymbolAliases)
#define gethostbyaddr	sceInsockGethostbyaddr
#define gethostbyname	sceInsockGethostbyname
#endif	/* !sceInsockDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __INSOCK_NETDB_H__ */
