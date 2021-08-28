/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: msifrpc.h,v 1.7 2003/06/12 01:28:40 ksh Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         sifrpc.h
 *                         sif remote proc basic header.
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12
 */

#ifndef _MSIFRPC_H_DEFS
#define _MSIFRPC_H_DEFS


#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef struct _sifm_rpc_data {	/* System used */
	void		*paddr;	/* packet address */
	unsigned int	pid;	/* packet id */
	int		tid;	/* thread id */
	unsigned int	mode;	/* call mode */
} sceSifMRpcData;

typedef void (* sceSifMEndFunc)(void *);	/* System used */

typedef struct _sifm_client_data {	/* System used */
	struct _sifm_rpc_data	rpcd;
	unsigned int	command;
	void		*buff;
	void		*cbuff;
	sceSifMEndFunc	func;
	void		*para;
	struct _sifm_serve_data		*serve;
} sceSifMClientData;

typedef struct _sifm_receive_data {	/* System used */
	struct _sifm_rpc_data	rpcd;
	void		*src;
	void		*dest;
	int		size;
} sceSifMReceiveData;

typedef void * (* sceSifMRpcFunc)(unsigned int,void *,int);

typedef struct _sifm_serve_data {
	void            *func_buff;
	int		size;	
	void            *cfunc_buff;
	int		csize;	
	sceSifMClientData *client;
	void		*paddr;
	unsigned int	fno;
	void		*receive;
	int		rsize;
	int		rmode;
	unsigned int	rid;
	struct _sifm_serve_data	*link;
	struct _sifm_serve_data	*next;
	struct _sifm_queue_data	*base;
	struct _sifm_serve_entry *sentry;
} sceSifMServeData;

typedef struct _sifm_serve_entry {
	unsigned int	mbxid;
	unsigned int	command;
	sceSifMRpcFunc	func;
	sceSifMRpcFunc	cfunc;
	sceSifMServeData *serve_list;
	struct _sifm_serve_entry *next;
} sceSifMServeEntry;

struct _sifm_smsg_data {
	void *cl_paddr;
	sceSifMRpcData *client;
	void *local;
	sceSifMServeEntry *sentry;
	unsigned int buffersize;
	unsigned int stacksize;
	int prio;
#if 1 /* for indirect action in _bind or _unbind */
	sceSifMServeData *sd;
#endif
};

typedef struct _sifm_serve_msg {	/* System used */
	struct MsgPacket header;
#if 1 /* for indirect action in _bind or _unbind */
	int type;
#endif
	struct _sifm_smsg_data data;
} sceSifMServeMsg;

typedef struct _sifm_queue_data {
	int             key;
	int             active;
#if 1 /* tmp */
	int sleep;
#endif
	struct _sifm_serve_data	*link;
	struct _sifm_serve_data	*start;
	struct _sifm_serve_data	*end;
	struct _sifm_queue_data	*next;  
}sceSifMQueueData; 

/* call & bind mode */
#define SIF_RPCM_NOWAIT		0x01	/* not wait for end of function */
#define SIF_RPCM_NOWBDC		0x02	/* no write back d-cache .ee only */

/* error no */
#define SIF_RPCE_GETP	1	/* fail to get packet data */
#define SIF_RPCE_SENDP	2	/* fail to send dma packet */

/* unbind result */
#define SIFM_UB_OK			1
#define SIFM_UB_NOT_DORMANT	2
#define SIFM_UB_NOT_EXIST	3
#define SIFM_UB_NOT_SLEEP	4

/* functions */

extern void sceSifMInitRpc(unsigned int mode);
extern void sceSifMEntryLoop(sceSifMServeEntry *, int, sceSifMRpcFunc, sceSifMRpcFunc);
extern int sceSifMTermRpc(int request, int flags);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _MSIFRPC_H_DEFS */

/* End of File */
