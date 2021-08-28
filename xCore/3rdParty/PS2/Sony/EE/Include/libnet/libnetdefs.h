/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                          Libnet Library
 *
 *      Copyright (C) 2000-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *  This header is used in Application(EE) & Libnet(EE) & Libnet(IOP).
 *
 *       Version      Date        Design      Log
 *  --------------------------------------------------------------------
 *       1.1.0        2002/01/24  ksh         sample to library
 *       1.2.0        2002/05/28  ksh         add sceLIBNETE_CALL_CANCELLED
 *       1.3.0        2003/04/15  ksh         add sceLIBNET_MIN_NETBUF
 *       1.3.1        2003/04/18  ksh         add sceLIBNETF_USE_WAITSEMA
 *       1.3.2        2003/04/30  ksh         add sceLIBNETF_IFC
 */

#if ( !defined __LIBNET_LIBNETDEFS_H )
#define	__LIBNET_LIBNETDEFS_H

#if defined(__cplusplus)
extern "C" {
#endif

#define	sceLIBNET_PAD_BASE( x, base )	( ( ( x ) + ( ( base ) - 1 ) ) & ~( ( base ) - 1 ) )
#define	sceLIBNET_PAD( x )	( sceLIBNET_PAD_BASE( ( x ), 64 ) )

#define	sceLIBNET_BUFFERSIZE	( 2048 )
#define	sceLIBNET_STACKSIZE 	( 8192 )
#define	sceLIBNET_PRIORITY  	( 32 )
#define	sceLIBNET_PRIORITY_SIF 	( sceLIBNET_PRIORITY )
#define	sceLIBNET_PRIORITY_IOP	( 64 )

#define	sceLIBNET_MAX_INTERFACE	( 2 )
#define	sceLIBNET_CTRL_DATA_SIZE	( sizeof( u_int ) * 9 )
#define	sceLIBNET_MIN_NETBUF	( 256 )

#define	sceLIBNETE_OK	( 0 )
#define	sceLIBNETE_NG	( -540 )
#define	sceLIBNETE_NO_RPC_CALL	( -541 )
#define	sceLIBNETE_RPC_CALL_FAILED	( -542 )
#define	sceLIBNETE_INSUFFICIENT_RESOURCES	( -543 )
#define	sceLIBNETE_CALL_CANCELLED	( -544 )
#define	sceLIBNETE_RESERVED2	( -545 )
#define	sceLIBNETE_USER 	( -546 )
#define	sceLIBNETE_USER1	( -546 )
#define	sceLIBNETE_USER2	( -547 )
#define	sceLIBNETE_USER3	( -548 )
#define	sceLIBNETE_USEREND	( -548 )
#define	sceLIBNETE_RESERVED1	( -549 )

#define	sceLIBNETF_DECODE	( 0x00000001 )
#define	sceLIBNETF_AUTO_UPIF	( 0x00000002 )
#define	sceLIBNETF_AUTO_LOADMODULE	( 0x00000004 )
#define	sceLIBNETF_USE_WAITSEMA	( 0x00000008 )
#define	sceLIBNETF_IFC	( 0x00000010 )

#if defined(__cplusplus)
}
#endif

#endif	/*** __LIBNET_LIBNETDEFS_H ***/

/*** End of file ***/

