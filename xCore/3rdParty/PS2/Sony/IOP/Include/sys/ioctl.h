/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: ioctl.h,v 1.13 2002/11/29 02:40:12 kashiwa Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         ioctl.h
 *                         IO manager interface error codes
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/11/09      hakama
 *       1.01           2000/11/06      koji        added for hdd and pfs
 *       1.02           2001/11/26      tei         added for 'host'
 */

#ifndef IOCTL_H

#ifndef NULL
#define NULL 0
#endif

#ifndef EOF
#define	EOF	(-1)			/* EOF from getc() */
#endif

/* general */
#define	FIOCNBLOCK	(('f'<<8)|1)	/* set non-blocking io */
#define	FIOCSCAN	(('f'<<8)|2)	/* scan for input */

/* tty and sio */
#define	TIOCRAW		(('t'<<8)|1)	/* disable xon/xoff control */
#define	TIOCFLUSH	(('t'<<8)|2)	/* flush input buffer */
#define	TIOCREOPEN	(('t'<<8)|3)	/* reopen */
#define	TIOCBAUD	(('t'<<8)|4)	/* set baud rate */
#define	TIOCEXIT	(('t'<<8)|5)	/* console interrup */
#define	TIOCDTR		(('t'<<8)|6)	/* control DTR line */
#define	TIOCRTS		(('t'<<8)|7)	/* control RTS line */
#define	TIOCLEN		(('t'<<8)|8)	/* character length */
					/* stop<<16 | char */
					/* stop 0:none 1:1 2:1.5 3:2bit */
					/* char 0:5 1:6 2:7 3:8bit */
#define	TIOCPARITY	(('t'<<8)|9)	/* parity 0:none 1:e 3:o */


/* hdd */
#define HIOCADDSUB		(('h'<<8)|1)
#define HIOCDELSUB		(('h'<<8)|2)
#define HIOCNSUB		(('h'<<8)|3)
#define HIOCFLUSH		(('h'<<8)|4)


#define HDIOC_MAXSECTOR		(('H'<<8)|1)
#define HDIOC_TOTALSECTOR	(('H'<<8)|2)
#define HDIOC_IDLE		(('H'<<8)|3)
#define HDIOC_FLUSH		(('H'<<8)|4)
#define HDIOC_SWAPTMP		(('H'<<8)|5)
#define HDIOC_DEV9OFF		(('H'<<8)|6)
#define HDIOC_STATUS		(('H'<<8)|7)
#define HDIOC_FORMATVER		(('H'<<8)|8)
#define HDIOC_SMARTSTAT		(('H'<<8)|9)
#define HDIOC_FREESECTOR	(('H'<<8)|10)
#define HDIOC_IDLEIMM		(('H'<<8)|11)


/* pfs */
#define PIOCALLOC		(('p'<<8)|1)
#define PIOCFREE		(('p'<<8)|2)
#define PIOCATTRADD		(('p'<<8)|3)
#define PIOCATTRDEL		(('p'<<8)|4)
#define PIOCATTRLOOKUP		(('p'<<8)|5)
#define PIOCATTRREAD		(('p'<<8)|6)

#define PDIOC_ZONESZ		(('P'<<8)|1)
#define PDIOC_ZONEFREE		(('P'<<8)|2)
#define PDIOC_CLOSEALL		(('P'<<8)|3)
#define PDIOC_GETFSCKSTAT	(('P'<<8)|4)
#define PDIOC_CLRFSCKSTAT	(('P'<<8)|5)

/* dev9 */
#define DDIOC_MODEL		(('D'<<8)|1)
#define DDIOC_OFF		(('D'<<8)|2)

/* CD/DVD Ioctl           */
#define CIOCSTREAMPAUSE         (('c'<<8)|13)
#define CIOCSTREAMRESUME        (('c'<<8)|14)
#define CIOCSTREAMSTAT          (('c'<<8)|15)

/* CD/DVD Devctl          */
#define CDIOC_READCLOCK         (('C'<<8)|12)
#define CDIOC_GETDISKTYP        (('C'<<8)|31)
#define CDIOC_GETERROR          (('C'<<8)|32)
#define CDIOC_TRAYREQ           (('C'<<8)|33)
#define CDIOC_STATUS            (('C'<<8)|34)
#define CDIOC_POWEROFF          (('C'<<8)|35)
#define CDIOC_MMODE             (('C'<<8)|36)
#define CDIOC_DISKRDY           (('C'<<8)|37)
#define CDIOC_STREAMINIT        (('C'<<8)|39)
#define CDIOC_BREAK             (('C'<<8)|40)

#define CDIOC_SPINNOM           (('C'<<8)|128)
#define CDIOC_SPINSTM           (('C'<<8)|129)
#define CDIOC_TRYCNT            (('C'<<8)|130)
#define CDIOC_STANDBY           (('C'<<8)|132)
#define CDIOC_STOP              (('C'<<8)|133)
#define CDIOC_PAUSE             (('C'<<8)|134)
#define CDIOC_GETTOC            (('C'<<8)|135)
#define CDIOC_SETTIMEOUT        (('C'<<8)|136)
#define CDIOC_READDVDDUALINFO	(('C'<<8)|137)
#define CDIOC_INIT              (('C'<<8)|138)
#define CDIOC_FSCACHEINIT       (('C'<<8)|149)
#define CDIOC_FSCACHEDELETE     (('C'<<8)|151)

/* drfp 'host:' */
#define DRFP_GETRDFLGSZ		(('r'<<8)|1)
#define DRFP_SETRDFLGSZ		(('r'<<8)|2)
#define DRFP_GETWTFLGSZ		(('r'<<8)|3)
#define DRFP_SETWTFLGSZ		(('r'<<8)|4)

#endif
