/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                         i.LINK Library
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                             ilsocksf.h
 *      header file of RPC client/server for socket for i.LINK
 *
 *	Version		Date		Design	Log
 *  --------------------------------------------------------------------
 *	0.91		2000/01/05	sim
 *	0.92		2000/05/12	sim	Rename sceILsock...
 */

#define SIFNUM_ILINK		    (0x80000b80)
#define SIFNUM_sceILsockModuleInit  (SIFNUM_ILINK |  0) /* System used */
#define SIFNUM_sceILsockModuleReset (SIFNUM_ILINK |  2) /* System used */
#define SIFNUM_sceILsockInit        (SIFNUM_ILINK |  4)
#define SIFNUM_sceILsockReset       (SIFNUM_ILINK |  5)
#define SIFNUM_sceILsockOpen        (SIFNUM_ILINK |  8)
#define SIFNUM_sceILsockClose       (SIFNUM_ILINK |  9)
#define SIFNUM_sceILsockBind        (SIFNUM_ILINK | 10)
#define SIFNUM_sceILsockConnect     (SIFNUM_ILINK | 11)
#define SIFNUM_sceILsockSend        (SIFNUM_ILINK | 12)
#define SIFNUM_sceILsockSendTo      (SIFNUM_ILINK | 13)
#define SIFNUM_sceILsockRecv        (SIFNUM_ILINK | 14)
#define SIFNUM_sceILsockRecvFrom    (SIFNUM_ILINK | 15)
#define SIFNUM_sce1394SbEui64       (SIFNUM_ILINK | 0x40)
#define SIFNUM_sce1394SbNodeId      (SIFNUM_ILINK | 0x41)
#define SIFNUM_sce1394CycleTimeV    (SIFNUM_ILINK | 0x42)
