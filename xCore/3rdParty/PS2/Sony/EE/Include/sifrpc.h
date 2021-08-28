/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 0.1.0
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         sifrpc.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.1.0
 */

#ifndef _SIFRPC_H_DEFS
#define _SIFRPC_H_DEFS

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef struct _sif_rpc_data {
	void		*paddr;	/* packet address */
	unsigned int	pid;	/* packet id */
	int		tid;	/* thread id */
	unsigned int	mode;	/* call mode */
} sceSifRpcData;

typedef void (* sceSifEndFunc)(void *);

typedef struct _sif_client_data {
	struct _sif_rpc_data	rpcd;
	unsigned int	command;
	void		*buff;
	void		*gp;
	sceSifEndFunc	func;
	void		*para;
	struct _sif_serve_data		*serve;
} sceSifClientData;

typedef struct _sif_receive_data {
	struct _sif_rpc_data	rpcd;
	void		*src;
	void		*dest;
	int		size;
} sceSifReceiveData;

typedef void * (* sceSifRpcFunc)(unsigned int,void *,int);

typedef struct _sif_serve_data {
	unsigned int    command;
	sceSifRpcFunc	func;
	void            *buff;
	int		size;	
	sceSifRpcFunc	cfunc;
	void            *cbuff;
	int		csize;	
	sceSifClientData *client;
	void		*paddr;
	unsigned int	fno;
	void		*receive;
	int		rsize;
	int		rmode;
	unsigned int	rid;
	struct _sif_serve_data	*link;
	struct _sif_serve_data	*next;
	struct _sif_queue_data	*base;
} sceSifServeData;

typedef struct _sif_queue_data {
	int             key;
	int             active;
	struct _sif_serve_data	*link;
	struct _sif_serve_data	*start;
	struct _sif_serve_data	*end;
	struct _sif_queue_data	*next;  
}sceSifQueueData; 

/* call & bind mode */
#define SIF_RPCM_NOWAIT		0x01	/* not wait for end of function */
#define SIF_RPCM_NOWBDC		0x02	/* no write back d-cache .ee only */

/* error no */
#define SIF_RPCE_GETP	1	/* fail to get packet data */
#define SIF_RPCE_SENDP	2	/* fail to send dma packet */
#define SIF_RPCE_CSEMA	3	/* fail to get sema */

/* functions */
void sceSifInitRpc(unsigned int mode);

int sceSifBindRpc(sceSifClientData *, unsigned int, unsigned int);
int sceSifCallRpc(sceSifClientData *, unsigned int, unsigned int,void *,int, void *,int, sceSifEndFunc, void *);

int sceSifCheckStatRpc(sceSifRpcData *);

void sceSifSetRpcQueue( sceSifQueueData *, int);
sceSifServeData * sceSifGetNextRequest(sceSifQueueData *);
void sceSifExecRequest(sceSifServeData *);
void sceSifRegisterRpc(sceSifServeData *, unsigned int,sceSifRpcFunc,void *,sceSifRpcFunc,void *,sceSifQueueData *);
void sceSifRpcLoop(sceSifQueueData *);
int sceSifGetOtherData(sceSifReceiveData *,void *,void *,int size, unsigned int mode);
sceSifServeData * sceSifRemoveRpc(sceSifServeData *,sceSifQueueData *);
sceSifQueueData * sceSifRemoveRpcQueue(sceSifQueueData *);


#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _SIFRPC_H_DEFS */

/* End of File */
