/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: deci2.h,v 1.3 2003/04/15 11:33:43 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         deci2.h
 *                         IOP deci2 manager define
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2000/1/24       tei
 */

#ifndef _DECI2_H
#define _DECI2_H

#ifdef __cplusplus
extern "C" {
#endif

/*-=-=-=-=-=-=-=-=-=-= deci2 header -=-=-=-=-=-=-=-=-=-=-=-=*/
typedef struct {
    unsigned short len;
    unsigned short rsvd;
    unsigned short proto;
    unsigned char  src;
    unsigned char  dest;
} sceDeci2Hdr;

#define DECI2_MAX_LEN	0xffff

#define DECI2_NODE_HOST		'H'
#define DECI2_NODE_IOP		'I'
#define DECI2_NODE_EE		'E'

/*-=-=-=-=-=-=-=-=-=- SCEI standard protocol numbers -=-=-=-=-=-=-=-=-=-=*/
#define DECI2_PROTO_DCMP	0x0001	/* Control Message */
#define DECI2_PROTO_I0TTYP	0x0110	/* IOP TTY base */
#define DECI2_PROTO_IKTTYP	0x011f	/* IOP Kernel TTY */
#define DECI2_PROTO_DRFP0	0x0120	/* IOP File (host0:) for EE */
#define DECI2_PROTO_DRFP1	0x0121	/* IOP File (host1:) for IOP */
#define DECI2_PROTO_ISDBGP	0x0130	/* IOP System Debugging */
#define DECI2_PROTO_ILOADP	0x0150	/* IOP load control */
#define DECI2_PROTO_E0TTYP	0x0210	/* EE  TTY base */
#define DECI2_PROTO_EKTTYP	0x021f	/* EE  Kernel TTY */
#define DECI2_PROTO_ESDBGP	0x0230	/* EE  System Debugging */
/*
 *   0x0000          : reserved(never use)
 *   0x0001          : DCMP
 *   0x0002 - 0x0fff : SCEI use
 *   0x1000 - 0xdfff : Tool licensee use
 *   0xe000 - 0xefff : local use for licensee
 *   0xf000 - 0xffff : reserved
 */

/*-=-=-=-=-=-=-=-=-=-= DCMP header -=-=-=-=-=-=-=-=-=-=-=-=*/
typedef struct {
    unsigned char type;
    unsigned char code;
    unsigned short unused;
} sceDcmpHdr;

/* DCMP Message Types */
typedef enum {
    DCMP_TYPE_CONNECT,
    DCMP_TYPE_ECHO,
    DCMP_TYPE_STATUS,
    DCMP_TYPE_ERROR,
} DCMP_TYPEs;

/*  DCMP_TYPE_CONNECT message code */
typedef enum {
    DCMP_CODE_CONNECT,
    DCMP_CODE_CONNECTR,
    DCMP_CODE_DISCONNECT,
    DCMP_CODE_DISCONNECTR,
} DCMP_CONNECT_CODEs;

/* Result Code for DCMP_TYPE_CONNECT */
typedef enum {
    DCMP_ERR_GOOD,		/* good */
    DCMP_ERR_INVALDEST,		/* destination invalid */
    DCMP_ERR_ALREADYCONN,	/* already connected */
    DCMP_ERR_NOTCONNECT,	/* not connected */
} DCMP_ERR_CODEs;

/*  DCMP_TYPE_ECHO message code */
typedef enum {
    DCMP_CODE_ECHO,
    DCMP_CODE_ECHOR,
} DCMP_ECHO_CODEs;

/*  DCMP_TYPE_STATUS message code */
typedef enum {
    DCMP_CODE_CONNECTED,
    DCMP_CODE_PROTO,
    DCMP_CODE_UNLOCKED,
    DCMP_CODE_SPACE,
    DCMP_CODE_ROUTE
} DCMP_STATUS_CODEs;

/*  DCMP_TYPE_ERROR message */
typedef enum {
    DCMP_CODE_NOROUTE,		/* no route to node */
    DCMP_CODE_NOPROTO,		/* protocol unreachable */
    DCMP_CODE_LOCKED,		/* locked */
    DCMP_CODE_NOSPACE,		/* deci2 manager/deci2d buffer full */
    DCMP_CODE_INVALHEAD,	/* invalid header */
    DCMP_CODE_NOCONNECT		/* not connected */
} DCMP_ERROR_CODEs;


/*-=-=-=-=-=-=-=-=-=-= DECI2 manager APIs -=-=-=-=-=-=-=-=-=-=*/
extern int sceDeci2Open( unsigned short protocol, void *opt,
			 void (*handler)(int event, int param, void *opt) );
extern int sceDeci2Close( int s );
extern int sceDeci2ReqSend( int s, char dest );
extern void sceDeci2Poll(void);

extern int sceDeci2ExRecv( int s, void *buf, unsigned short len );
extern int sceDeci2ExSend( int s, void *buf, unsigned short len );
extern int sceDeci2ExReqSend( int s, char dest );
extern int sceDeci2ExLock( int s );
extern int sceDeci2ExUnLock( int s );

/* IOP only */
extern int sceDeci2ExWakeupThread( int s, int thid );
extern int sceDeci2ExSignalSema( int s, int semid );
extern int sceDeci2ExSetEventFlag( int s, int evfid, unsigned long bitpattern);

/* IOP DECI2 debug print function */
extern int sceDeci2ExPanic(const char *fmt, ...);

/* DECI2 manager API retrun value (error code) */
#define	DECI2_ERR_INVALID     -1  /* invalid argument */
#define	DECI2_ERR_INVALSOCK   -2  /* invalid socket descriptor */
#define	DECI2_ERR_ALREADYUSE  -3  /* protocol number already used */
#define	DECI2_ERR_MFILE       -4  /* too many open protocols */
#define	DECI2_ERR_INVALADDR   -5  /* invalid address for buffer */
#define	DECI2_ERR_PKTSIZE     -6  /* buffer is too small */
#define	DECI2_ERR_WOULDBLOCK  -7  /* blocks inspite of asynchronous */
#define	DECI2_ERR_ALREADYLOCK -8  /* already locked */
#define	DECI2_ERR_NOTLOCKED   -9  /* not locked */
#define	DECI2_ERR_NOROUTE    -10  /* no route to host */
#define	DECI2_ERR_NOSPACE    -11  /* no room left on manager */
#define	DECI2_ERR_INVALHEAD  -12  /* invalid deci2 header */
#define	DECI2_ERR_NOHOSTIF   -13  /* No interface to Host */

/* events value for protocol driver */
typedef enum {
    DECI2_READ = 1,
    DECI2_READDONE,
    DECI2_WRITE,
    DECI2_WRITEDONE,
    DECI2_CHSTATUS,
    DECI2_ERROR,
} Deci2ProtocolDriverEvent;

#ifdef __cplusplus
}
#endif

#endif /* _DECI2_H */
