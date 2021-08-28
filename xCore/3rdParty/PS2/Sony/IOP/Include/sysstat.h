/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: sysstat.h,v 1.2 1999/11/20 07:11:37 hakama Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         sysstat.h
 *                         system status event flag define
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/11/03      tei
 */

#ifndef _SYSSTAT_H_
#define _SYSSTAT_H_

/* get SystemStatus EventFlag ID */
extern int GetSystemStatusFlag();

/* SystemStatus EventFlag bit assign */
#define SSF_HARD_BOOT			(1<<0)
#define SSF_SOFT_BOOT			(1<<1)
#define SSF_UPDATE_BOOT_PROGRESS	(1<<2)
#define SSF_UPDATE_BOOT			(1<<3)
#define SSF_DECI1			(1<<4)
#define SSF_DECI2			(1<<5)
#define SSF_DECI_AVAILABLE		(1<<6)
#define SSF_SIF_AVAILABLE		(1<<7)
#define SSF_SIFCMD_AVAILABLE		(1<<8)
#define SSF_VBLANK_AVAILABLE		(1<<9)
#define SSF_IOPREBOOT_REQUESTED		(1<<10)
#define SSF_SIFRPC_AVAILABLE		(1<<11)

#endif /* _SYSSTAT_H_ */
