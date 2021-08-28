/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                          Libnet Library
 *
 *      Copyright (C) 2000-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *       This header is used in Libnet(EE) & Libnet(IOP).
 *
 *       Version      Date        Design      Log
 *  --------------------------------------------------------------------
 *       1.1.0        2002/01/24  ksh         sample to library
 *       1.2.0        2002/05/28  ksh         add RPC_WAITGETIEV
 */

#if ( !defined __LIBNET_COMMON_H )
#define	__LIBNET_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#define LIBNET_SIFNUMBER	( 0x80001201 )

#define	DUMP( p, size )  ( { \
	int 	i; \
	for ( i = 0; i < ( size ); i++ ) { \
		printf( "%02x ", *( ( unsigned char* )( p ) + i ) & 0xff ); \
		if ( i % 16 == 15 ) { \
			printf( "\n" ); \
		} \
	} \
	printf( "\n" ); \
} )

#define	PAD_INETADDRESS	( sceLIBNET_PAD_BASE( sizeof( sceInetAddress_t ), sizeof( u_int ) ) )

#define RPC_SCECREATE	( 1 )
#define RPC_SCEOPEN	( 2 )
#define RPC_SCECLOSE	( 3 )
#define RPC_SCERECV	( 4 )
#define RPC_SCESEND	( 5 )
#define RPC_SCENAME2ADDR	( 6 )
#define RPC_SCEADDR2STR	( 7 )
#define RPC_SCEGIFLIST	( 8 )
#define RPC_SCEIFCTL	( 9 )
#define RPC_SCEGETRT	( 10 )
#define RPC_SCEGETNS	( 11 )
#define RPC_SCECHPRI	( 12 )
#define RPC_SCERECVF	( 13 )
#define RPC_SCESENDT	( 14 )
#define RPC_SCEABORT	( 15 )
#define	RPC_SCEABORTLOG	( 16 )
#define RPC_SCEGETLOG	( 17 )
#define RPC_SCEADDR2NAME	( 18 )
#define RPC_SCECTL	( 19 )
#define RPC_SCEPOLL	( 20 )

#define RPC_REGHANDLER	( 30 )
#define RPC_UNREGHANDLER	( 31 )
#define RPC_SETCONFIG	( 32 )
#define RPC_WAITIFATTACHED	( 33 )
#define	RPC_WAITIFSTARTED	( 34 )
#define	RPC_WAITGETIEV	( 35 )

#define	RPC_UPIF	( 50 )
#define	RPC_DOWNIF	( 51 )
#define	RPC_SETAUTOMODE	( 52 )
#define	RPC_GETSTATE	( 53 )

#define	RPC_USER	( 64 )

#if defined(__cplusplus)
}
#endif

#endif	/*** __LIBNET_COMMON_H ***/

/*** End of file ***/

