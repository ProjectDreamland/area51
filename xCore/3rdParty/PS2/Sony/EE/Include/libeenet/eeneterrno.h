/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <libeenet - eeneterrno.h>
 *                  <header for libeenet error code>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Jul,24,2002     komaki      first version
 */

#ifndef _EENETERRNO_H_
#define _EENETERRNO_H_

/* compatible with eenetctlerrno.h (-1 to -20 are reserved) */
#define sceEENET_E_NoMem		-1	/* No more memory */
#define sceEENET_E_Sema		-2	/* Semaphore error */
#define sceEENET_E_Thread	-3	/* Thread error */
#define sceEENET_E_Socket	-4	/* Socket operation error */
#define sceEENET_E_Bpf		-5	/* BPF operation error */
#define sceEENET_E_Ifconfig	-6	/* Interface configuration error */
#define sceEENET_E_DnsRoute	-7	/* DNS & Router configuration error */
#define sceEENET_E_GetStatus	-8	/* Can't get status */
#define sceEENET_E_Local		-9	/* Local operation error */
#define sceEENET_E_Fatal		-10	/* Fatal system error */

#define sceEENET_E_File		-21	/* File operation error */
#define sceEENET_E_Alarm		-22	/* Alarm error */
#define sceEENET_E_Busy		-23	/* Busy */
#define sceEENET_E_DevBusy	-24	/* Device Busy */
#define sceEENET_E_Exist		-25	/* Already Exist */
#define sceEENET_E_AFNoSupport	-26	/* Address family is not supported */
#define sceEENET_E_NoSpace	-27	/* No enough space */
#define sceEENET_E_NoEnt		-28	/* there is no entry */
#define sceEENET_E_Invalid	-29	/* invalid argument */
#define sceEENET_E_NotAvail	-30	/* not available */

#define sceEENET_E_Unknown	-999	/* unknown error */

#endif /* _EENETERRNO_H_ */
