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
 *                         sifcmd.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.1.0
 */

#ifndef _SIFCMD_H_DEFS
#define _SIFCMD_H_DEFS

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
  
#define SIF_CMDI_SYSTEM 0x80000000 /* system func call , not user func*/

/* commnad packet header (128bit) */
typedef  struct {
	unsigned int	psize:8; /* packet size(min 16(hdr only),max 16*7)*/
	unsigned int	dsize:24;/* attended data size */
	unsigned int	daddr;   /* attended data address */
  	unsigned int	fcode;	 /* called function code */
	unsigned int	opt;	 /* system no use */
} sceSifCmdHdr;

/* commnad packet handler */
typedef void (* sceSifCmdHandler)(void *, void *);

/* structure of command packet handler & data */
typedef struct {
	sceSifCmdHandler	func;
	void			*data;
	void			*gp;
} sceSifCmdData;

/* system function (defalut) */
#define SIF_CMDC_CHANGE_SADDR	( SIF_CMDI_SYSTEM | 0x00000000)
#define SIF_CMDC_SET_SREG	( SIF_CMDI_SYSTEM | 0x00000001)
#define SIF_CMDC_INIT_CMD	( SIF_CMDI_SYSTEM | 0x00000002)
#define SIF_CMDC_RESET_CMD	( SIF_CMDI_SYSTEM | 0x00000003)

/* data structure for telling to change packet buffer address */
typedef struct {
	sceSifCmdHdr	chdr;
	void		*newaddr;
} sceSifCmdCSData; 		/* System use */

/* data structure for telling to set software register[0 - 31] */
/* software register[0 - 7] used by system */
typedef struct {
	sceSifCmdHdr	chdr;
	int		rno;
	unsigned int	value;
} sceSifCmdSRData;

/* data structure for reset iop modules */
typedef struct {
	sceSifCmdHdr    chdr;
	int             size;
	int             flag;
	char            arg[80];
} sceSifCmdResetData; 		/* System use */

/* */
void sceSifInitCmd(void);
void sceSifExitCmd(void);

/* get & set software register value */
unsigned int sceSifGetSreg(int);
unsigned int sceSifSetSreg(int,unsigned int);

sceSifCmdData * sceSifSetCmdBuffer(sceSifCmdData *, int);
sceSifCmdData * sceSifSetSysCmdBuffer(sceSifCmdData *, int); /* System use */

void sceSifAddCmdHandler(unsigned int,sceSifCmdHandler,void *);
void sceSifRemoveCmdHandler(unsigned int);

unsigned int sceSifSendCmd(unsigned int,void *,int,void *, void *, int);
unsigned int isceSifSendCmd(unsigned int,void *,int,void *, void *, int);

void sceSifWriteBackDCache(const void *, int); /* EE only */

/* send mode */
#define SIF_CMDM_INTR	0x01	/* called in no intr area */
#define SIF_CMDM_TAG	0x02	/* */
#define SIF_CMDM_WBDC	0x04	/* write back atended data */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _SIFCMD_H_DEFS */

/* End of File */
