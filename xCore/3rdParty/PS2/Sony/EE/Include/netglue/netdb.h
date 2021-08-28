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
 *                    <netglue - netglue/netdb.h>
 *                     <header for DNS functions>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Sep,21,2001     komaki      first version
 *       1.10           Apr,24,2003     ksh         prefix
 */

#ifndef __NETGLUE_NETDB_H__
#define __NETGLUE_NETDB_H__

#include <sys/types.h>

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef struct sceNetGlueHostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	sceNetGlue_h_addr	h_addr_list[0]
				/* address, for backward compatibility */
} sceNetGlueHostent_t;

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define hostent	sceNetGlueHostent
#define	h_addr	sceNetGlue_h_addr
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#define	SCE_NETGLUE_NETDB_INTERNAL	-1	/* see errno */
#define	SCE_NETGLUE_NETDB_SUCCESS	0	/* no problem */
#define	SCE_NETGLUE_HOST_NOT_FOUND	1
			/* Authoritative Answer Host not found */
#define	SCE_NETGLUE_TRY_AGAIN	2
			/* Non-Authoritative Host not found, or SERVERFAIL */
#define	SCE_NETGLUE_NO_RECOVERY	3
			/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	SCE_NETGLUE_NO_DATA		4
			/* Valid name, no data record of requested type */
#define	SCE_NETGLUE_NO_ADDRESS	SCE_NETGLUE_NO_DATA
			/* no address, look for MX record */

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define	NETDB_INTERNAL	SCE_NETGLUE_NETDB_INTERNAL
#define	NETDB_SUCCESS	SCE_NETGLUE_NETDB_SUCCESS
#define	HOST_NOT_FOUND	SCE_NETGLUE_HOST_NOT_FOUND
#define	TRY_AGAIN	SCE_NETGLUE_TRY_AGAIN
#define	NO_RECOVERY	SCE_NETGLUE_NO_RECOVERY
#define	NO_DATA		SCE_NETGLUE_NO_DATA
#define	NO_ADDRESS	SCE_NETGLUE_NO_ADDRESS
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

sceNetGlueHostent_t *sceNetGlueGethostbyaddr(const char *, int, int);
sceNetGlueHostent_t *sceNetGlueGethostbyname(const char *);

#if !defined(sceNetGlueDisableSocketSymbolAliases)
#define gethostbyaddr	sceNetGlueGethostbyaddr
#define gethostbyname	sceNetGlueGethostbyname
#endif	/* !sceNetGlueDisableSocketSymbolAliases */

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* __NETGLUE_NETDB_H__ */
