/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                          Libnet Library
 *
 *      Copyright (C) 2000-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *       This header is used in Application(EE) & Libnet(EE).
 *
 *       Version   Date        Design   Log
 *  --------------------------------------------------------------------
 *       1.1.0     2002/01/24  ksh      sample to library
 *       1.2.0     2002/05/28  ksh      add inetctl.h
 *       1.2.1     2003/04/10  ksh      char * to const char *
 *       1.3.0     2003/04/15  ksh      u_int *net_buf to void *net_buf
 */

#if ( !defined __LIBNET_LIBNET_H )
#define	__LIBNET_LIBNET_H

#if 0

/* default -> disabled */
#if !defined(sceLibnetDisableSocketSymbolAliases) \
	&& !defined(sceLibnetEnableSocketSymbolAliases)
#define sceLibnetDisableSocketSymbolAliases
#endif

#if defined(sceLibnetEnableSocketSymbolAliases)
#undef	sceLibnetDisableSocketSymbolAliases
#endif	/* sceLibnetEnableSocketSymbolAliases */

#endif /* 0 */

/*** Header ***/
#include <libmrpc.h>

/*** Libnet header ***/
#include <libnet/libnetdefs.h>

/*** IOP header ***/
#define	__LIBNET__
#include <../../iop/include/inet/inet.h>
#include <../../iop/include/inet/inetctl.h>
#include <libnet/in.h>
#include <../../iop/include/inet/netdev.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*** Libnet ***/
int 	sceLibnetInitialize( sceSifMClientData *cd, int buffersize, int stacksize, int priority );
int 	sceLibnetTerminate( sceSifMClientData *cd );
int 	sceLibnetRegisterHandler( sceSifMClientData *cd, void *net_buf );
int 	sceLibnetUnregisterHandler( sceSifMClientData *cd, void *net_buf );
int 	sceLibnetSetConfiguration( sceSifMClientData *cd, void *net_buf, u_int env_addr );
int 	sceLibnetWaitGetAddress( sceSifMClientData *cd, void *net_buf, int *if_id, int n, struct sceInetAddress *addr, u_int flags );
int 	sceLibnetWaitGetInterfaceID( sceSifMClientData *cd, void *net_buf, int *if_id, int n );
int 	sceLibnetWaitGetInterfaceEvent( sceSifMClientData *cd, void *net_buf, int *id, int *type );

/*** INET ***/
int 	sceInetPoll( sceSifMClientData *cd, void *net_buf, sceInetPollFd_t *fds, int nfds, int ms );
int 	sceInetName2Address( sceSifMClientData *cd, void *net_buf, int flags, struct sceInetAddress *paddr, const char *name, int ms, int nretry, ... );
int 	sceInetAddress2String( sceSifMClientData *cd, void *net_buf, char *name, int len, struct sceInetAddress *paddr );
int 	sceInetAddress2Name( sceSifMClientData *cd, void *net_buf, int flags, char *name, int len, struct sceInetAddress *paddr, int ms, int nretry, ... );
int 	sceInetCreate( sceSifMClientData *cd, void *net_buf, struct sceInetParam *param );
int 	sceInetOpen( sceSifMClientData *cd, void *net_buf, int cid, int ms );
int 	sceInetClose( sceSifMClientData *cd, void *net_buf, int cid, int ms );
int 	sceInetRecv( sceSifMClientData *cd, void *net_buf, int cid, void *ptr, int count, int *pflags, int ms );
int 	sceInetRecvFrom( sceSifMClientData *cd, void *net_buf, int cid, void *ptr, int count, int *pflags, struct sceInetAddress *remote_adr, int *premote_port, int ms );
int 	sceInetSend( sceSifMClientData *cd, void *net_buf, int cid, const void *ptr, int count, int *pflags, int ms );
int 	sceInetSendTo( sceSifMClientData *cd, void *net_buf, int cid, void *ptr, int count, int *pflags, struct sceInetAddress *remote_adr, int remote_port, int ms );
int 	sceInetAbort( sceSifMClientData *cd, void *net_buf, int cid, int flags );
int 	sceInetControl( sceSifMClientData *cd, void *net_buf, int cid, int code, void *ptr, int len );
int 	sceInetGetInterfaceList( sceSifMClientData *cd, void *net_buf, int *interface_id_list, int n );
int 	sceInetInterfaceControl( sceSifMClientData *cd, void *net_buf, int interface_id, int code, void *ptr, int len );
int 	sceInetGetRoutingTable( sceSifMClientData *cd, void *net_buf, struct sceInetRoutingEntry *p, int n );
int 	sceInetGetNameServers( sceSifMClientData *cd, void *net_buf, struct sceInetAddress *paddr, int n );
int 	sceInetChangeThreadPriority( sceSifMClientData *cd, void *net_buf, int prio );
int 	sceInetGetLog( sceSifMClientData *cd, void *net_buf, char *log_buf, int len, int ms );
int 	sceInetAbortLog( sceSifMClientData *cd, void *net_buf );

/*** INETCTL ***/
int 	sceInetCtlUpInterface( sceSifMClientData *cd, void *net_buf, int id );
int 	sceInetCtlDownInterface( sceSifMClientData *cd, void *net_buf, int id );
int 	sceInetCtlSetAutoMode( sceSifMClientData *cd, void *net_buf, int f_auto );
int 	sceInetCtlGetState( sceSifMClientData *cd, void *net_buf, int id, int *pstate );

/*** erx ***/
void *sceLibnetGetErxEntries(void);
void *sceLibnetifGetErxEntries(void);

/*** disable ***/
#if	defined	sceLibnetDisableAll
#undef	sceLibnetDisableCompatible
#define	sceLibnetDisableCompatible
#undef	sceLibnetDisableNoExtra
#define	sceLibnetDisableNoExtra
#undef	sceLibnetDisableAliases
#define	sceLibnetDisableAliases
#endif	/*** sceLibnetDisableAll ***/

/*** compatible ***/
#if ( !defined sceLibnetDisableCompatible )
int 	load_set_conf_extra( sceSifMClientData *cd, void *net_buf, const char *fname, const char *usr_name, u_int flags );
#endif	/*** sceLibnetDisableCompatible ***/

/*** extra( reserved ) ***/
#if ( !defined sceLibnetDisableNoExtra )
#endif	/*** sceLibnetDisableNoExtra ***/

/*** alias ***/
#if ( !defined sceLibnetDisableAliases )
#define	libnet_init( xcd, x1, x2, x3 )	sceLibnetInitialize( xcd, x1, x2, x3 )
#define	libnet_term( xcd )	sceLibnetTerminate( xcd )
#define	reg_handler( xcd, xnet_buf )	sceLibnetRegisterHandler( xcd, xnet_buf )
#define	unreg_handler( xcd, xnet_buf )	sceLibnetUnregisterHandler( xcd, xnet_buf )
#define	load_set_conf( xcd, xnet_buf, x1, x2 )	load_set_conf_extra( xcd, xnet_buf, x1, x2, sceLIBNETF_AUTO_LOADMODULE | sceLIBNETF_AUTO_UPIF )
#define	load_set_conf_only( xcd, xnet_buf, x1, x2 )	load_set_conf_extra( xcd, xnet_buf, x1, x2, sceLIBNETF_AUTO_LOADMODULE )
#define	wait_get_addr( xcd, xnet_buf, x1, x2 )	sceLibnetWaitGetAddress( xcd, xnet_buf, x1, 1, x2, sceLIBNETF_AUTO_UPIF )
#define	wait_get_addr_only( xcd, xnet_buf, x1, x2 )	sceLibnetWaitGetAddress( xcd, xnet_buf, x1, 1, x2, 0 )
#define	get_interface_id( xcd, xnet_buf, x1 )	sceLibnetWaitGetInterfaceID( xcd, xnet_buf, x1, 1 )
#define	up_interface( xcd, xnet_buf, x1 )	sceInetCtlUpInterface( xcd, xnet_buf, x1 )
#define	down_interface( xcd, xnet_buf, x1 )	sceInetCtlDownInterface( xcd, xnet_buf, x1 )
#endif	/*** sceLibnetDisableAliases ***/

#if defined(__cplusplus)
}
#endif

#endif	/*** __LIBNET_LIBNET_H ***/

/*** End of file ***/

