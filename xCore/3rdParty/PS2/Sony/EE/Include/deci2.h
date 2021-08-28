/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine Library
 * Version 0.02
 * Copyright (C) 1999 Sony Computer Entertainment Inc., All Rights Reserved.
 *
 * libkernel - deci2.h
 *             user side DECI2 API library
 *
 * Version        Date        Design        Log
 * --------------------------------------------------------------------
 * 0.00           03/26/99    koji          1st version
 * 0.01           04/26/99    koji          fucntion table to bss section
 * 0.02           06/16/99    koji          changed for new specification
 * 0.03           07/05/99    koji          changed function prototypes
 */

#ifndef _DECI2_H
#define _DECI2_H

#ifdef __cplusplus
extern "C" {
#endif

/* deci2 header */
typedef struct {
	unsigned short len;
	unsigned short rsvd;
	unsigned short proto;
	unsigned char  src;
	unsigned char  dest;
} sceDeci2Hdr;

/* events value for protocol handler */
#define DECI2_READ	1
#define	DECI2_READDONE	2
#define	DECI2_WRITE	3
#define	DECI2_WRITEDONE	4
#define	DECI2_CHSTATUS	5
#define	DECI2_ERROR	6

/* error codes */
#define DECI2_ERR_INVALID	-1 	/* invalid argument */
#define DECI2_ERR_INVALSOCK	-2	/* invalid socket descriptor */
#define DECI2_ERR_ALREADYUSE	-3	/* protocol number already used */
#define DECI2_ERR_MFILE		-4	/* too many open protocols */
#define DECI2_ERR_INVALADDR	-5	/* invalid address for buffer */
#define DECI2_ERR_PKTSIZE	-6	/* buffer is too small */
#define DECI2_ERR_WOULDBLOCK	-7	/* blocks inspite of asynchronous */
#define DECI2_ERR_ALREADYLOCK	-8	/* already lock */
#define DECI2_ERR_NOTLOCKED	-9	/* not locked */
#define DECI2_ERR_NOROUTE	-10	/* no route to host */
#define DECI2_ERR_NOSPACE	-11	/* no room left on manager */
#define DECI2_ERR_INVALHEAD	-12	/* invalid deci2 header */

/* function prototypes */
int  sceDeci2Open(unsigned short protocol, void *opt,
                 void (*handler)(int event, int param, void *opt));
int  sceDeci2Close(int s);
int  sceDeci2ReqSend(int s, char dest);
void sceDeci2Poll(int s);
int  sceDeci2ExRecv(int s, void *buf, unsigned short len);
int  sceDeci2ExSend(int s, void *buf, unsigned short len);
int  sceDeci2ExReqSend(int s, char dest);
int  sceDeci2ExLock(int s);
int  sceDeci2ExUnLock(int s);

#ifdef __cplusplus
}
#endif

#endif /* _DECI2_H */

