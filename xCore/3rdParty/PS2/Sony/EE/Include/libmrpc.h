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

#ifndef _MSIFRPC_H_DEFS
#define _MSIFRPC_H_DEFS

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

typedef struct _sifm_rpc_data {
	void		*paddr;	/* packet address */
	unsigned int	pid;	/* packet id */
	int		tid;	/* thread id */
	unsigned int	mode;	/* call mode */
} sceSifMRpcData;

typedef void (* sceSifMEndFunc)(void *);

typedef struct _sifm_client_data {
	struct _sifm_rpc_data	rpcd;
	unsigned int	command;
	void		*buff;
	void		*gp;
	sceSifMEndFunc	func;
	void		*para;
	void		*serve;
	int		sema;
	int		unbind;
	int		buffersize;
	int		stacksize;
	int		prio;
} sceSifMClientData;

/* call & bind mode */
#define SIF_RPCM_NOWAIT		0x01	/* not wait for end of function */
#define SIF_RPCM_NOWBDC		0x02	/* no write back d-cache .ee only */

/* error no */
#define SIF_RPCE_GETP	1	/* fail to get packet data */
#define SIF_RPCE_SENDP	2	/* fail to send dma packet */
#define SIF_RPCE_CSEMA	3	/* fail to get sema */

/* unbind result */
#define SIFM_UB_OK			1
#define SIFM_UB_NOT_DORMANT	2
#define SIFM_UB_NOT_EXIST	3
#define SIFM_UB_NOT_SLEEP	4

/* get member macro */
#define sceSifMGetBufferSize(x) (x)->buffersize
#define sceSifMGetStackSize(x) (x)->stacksize
#define sceSifMGetPriority(x) (x)->prio

/* functions */
void sceSifMInitRpc(unsigned int mode);
void sceSifMExitRpc(void);

int sceSifMBindRpc(sceSifMClientData *, unsigned int, unsigned int);
int sceSifMBindRpcParam(sceSifMClientData *, unsigned int, unsigned int,
			unsigned int,unsigned int, int);
int sceSifMUnBindRpc(sceSifMClientData *, unsigned int);
/* int sceSifMUnBindRpc(sceSifMClientData *, unsigned int, unsigned int);*/
int sceSifMCallRpc(sceSifMClientData *, unsigned int, unsigned int,void *,int, void *,int, sceSifMEndFunc, void *);

/* erx */
void *sceSifMGetErxEntries(void);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _MSIFRPC_H_DEFS */

/* End of File */
