/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
*/
/*
 *                      Emotion Engine Library
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         libhig - libhig.h
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *                    Sep,20,2000     kaneko
 */
/*	$Id: libhig.h,v 1.36 2003/10/08 09:52:53 xkazama Exp $	*/
#ifndef __HiG_H__
#define __HiG_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>	/* necessary header(for size_t) */
#endif
#ifndef _eetypes_h_
#include <eetypes.h>	/* necessary header */
#endif
#ifndef _eestruct_
#include <eestruct.h>	/* necessary header */
#endif
#ifndef _LIB_VU0_H_
#include <libvu0.h>	/* necessary header */
#endif
#ifndef _LIBGRAPH_H
#include <libgraph.h>	/* necessary header */
#endif

/*********************************************************************************
 * 				Error Codes
 *********************************************************************************/

/* HiG Error Code (number 0 - 127 reserved) */
typedef enum _sceHiErr{
  SCE_HIG_NO_ERR = 0,
  SCE_HIG_NO_HEAP,
  SCE_HIG_INVALID_VALUE,
  SCE_HIG_INVALID_DATA,
  SCE_HIG_FAILURE,

  HIG_NO_ERR = SCE_HIG_NO_ERR
}sceHiErr;

/*********************************************************************************
 *				TYPE DEFINES
 *********************************************************************************/

/* HiG Data Header structure */
typedef struct _sceHiHeader{
	u_int		ver;	/* 0-7bit is 'status' (same place of (sceHiPlug).type */
	u_int		reserve1;
	u_int		reserve2;
	u_int		qsize;
} sceHiHeader;

typedef struct _sceHiHeadData{
	char		plug_name[12];
	struct _sceHiPlug	*plug_blk_addr;
} sceHiHeadData;

/* plugin type structure */
typedef struct _sceHiType {
	unsigned long	repository:8;
	unsigned long	project:8;
	unsigned long	category:8;
	unsigned long	status:8;
	unsigned long	id:24;
	unsigned long	revision:8;
} sceHiType;

enum {
	SCE_HIG_REPOSITORY_O	=	0,
	SCE_HIG_PROJECT_O	=	8,
	SCE_HIG_CATEGORY_O	=	16,
	SCE_HIG_STATUS_O	=	24,
	SCE_HIG_ID_O		=	32,
	SCE_HIG_REVISION_O	=	56,
};

#define	SCE_HIG_REPOSITORY_M	(0x00000000000000ffUL)
#define	SCE_HIG_PROJECT_M	(0x000000000000ff00UL)
#define	SCE_HIG_CATEGORY_M	(0x0000000000ff0000UL)
#define	SCE_HIG_STATUS_M	(0x00000000ff000000UL)
#define	SCE_HIG_ID_M		(0x00ffffff00000000UL)
#define	SCE_HIG_REVISION_M	(0xff00000000000000UL)

enum {
    SCE_HIG_STATUS_MAP_O	=	0x01,
    SCE_HIG_STATUS_DATA_O	=	0x02,
    SCE_HIG_STATUS_REF_O	=	0x03,
    SCE_HIG_STATUS_STOP_O	=	0x04
};

/* plugin block handle structure */
typedef struct _sceHiPlug {
	sceHiType	type;
	void		*myapi;
	u_int		size;
	char		nplug;
	char		ndata;
	char		reserve[6];
	u_int		stack;
	u_int		args;
	qword		list;
} sceHiPlug;

/* plugin block & plugin data block list structure */
typedef struct _sceHiList {
	sceHiType	type;
	u_int		*addr;
	u_int		reserve;
} sceHiList;

/* plugin data block structure */
typedef struct _sceHiData {
	sceHiType	type;
	char		count;
	char		reserve[3];
	u_int		size;
	u_int		data[1];
} sceHiData;

/* plugin regist table structure */
typedef struct _sceHiPlugTable {
	sceHiType	type;
	void		*func;
} sceHiPlugTable;

/* sceHiG Error State Structure */
typedef struct _sceHiErrStateType {
    sceHiPlug	*top;	/* Root of plugin (if not in *plugin* then NULL */
    sceHiPlug	*plug;	/* Err plugin (if not in *plugin* then NULL*/
    int		process; /* Err plugin process (if not in *plugin then -1 */
    sceHiType	type;	/* plugin type (if not in *plugin* then 0 */
    const char	*mes;	/* error message (if no err then NULL) */    
} sceHiErrStateType;

/*********************************************************************************
 *				FUNCTIONS
 *********************************************************************************/

extern sceHiErr sceHiRegistTable(sceHiPlugTable *, u_int);
extern sceHiErr sceHiParseHeader(u_int *);
extern sceHiErr sceHiGetPlug(u_int *, char *, sceHiPlug **);
extern sceHiErr sceHiCallPlug(sceHiPlug *, int);
extern sceHiErrStateType *sceHiGetErrStatePtr(void);

extern sceHiErr sceHiGetInsPlug(sceHiPlug *, sceHiPlug **, sceHiType);
extern sceHiErr sceHiGetPlugList(sceHiPlug *, sceHiList **, sceHiType);

extern sceHiErr sceHiGetList(sceHiPlug *, sceHiList **, sceHiType);
extern sceHiErr sceHiGetData(sceHiPlug *, u_int **, sceHiType);

extern sceHiErr sceHiMakeType(sceHiType *, u_long *);
extern sceHiErr sceHiGetType(sceHiType *, sceHiType *);

extern sceHiErr sceHiNewPlugBlk(int, int, sceHiPlug **, sceHiType *);
extern sceHiErr sceHiCalcPlugBlkSize(int, int, size_t *);
extern sceHiErr sceHiCreatePlugBlk(sceHiPlug *, int, int, const sceHiType *);
extern sceHiErr sceHiSetPlugType(sceHiPlug *, sceHiType *);
extern sceHiErr sceHiSetDataType(sceHiData *, sceHiType *);
extern sceHiErr sceHiSetPluginApi(sceHiPlug *);
extern sceHiErr sceHiAddPlugBlk(sceHiPlug *, sceHiPlug *);
extern sceHiErr sceHiInsPlugBlk(sceHiPlug *, sceHiPlug *, int);
extern sceHiErr sceHiAddDataBlk(sceHiPlug *, sceHiData *);
extern sceHiErr sceHiInsDataBlk(sceHiPlug *, sceHiData *, int);
extern sceHiErr sceHiRmvPlugBlk(sceHiPlug *, sceHiPlug *);
extern sceHiErr sceHiRmvDataBlk(sceHiPlug *, sceHiData *);
extern sceHiErr sceHiMakeDataBlk(u_int *, sceHiData **, sceHiType *);
extern sceHiErr sceHiCreateDataBlk(sceHiData *, const u_int *, sceHiType *);
extern sceHiErr sceHiGetPlugPlace(sceHiPlug *, sceHiPlug *, int *);
extern sceHiErr sceHiGetDataPlace(sceHiPlug *, sceHiData *, int *);
extern sceHiErr sceHiStopPlugStatus(sceHiPlug *);
extern sceHiErr sceHiContPlugStatus(sceHiPlug *);
extern sceHiErr sceHiStopPlugListStatus(sceHiList *);
extern sceHiErr sceHiContPlugListStatus(sceHiList *);

/*********************************************************************************
 *				MACROS
 *********************************************************************************/
/* HiG Version */
#define	SCE_HIG_VERSION			0x000001

/*	default plugins process			*/
#define	SCE_HIG_INIT_PROCESS		1
#define	SCE_HIG_PRE_PROCESS		2
#define SCE_HIG_POST_PROCESS		3
#define	SCE_HIG_END_PROCESS		4

/* default converter variables */
#define SCE_HIG_HEADER_STATUS		0
#define SCE_HIG_PLUGIN_STATUS		0
#define SCE_HIG_DATA_STATUS		0

/*********************************************************************************
 *				Mem Services
 *********************************************************************************/
extern void *sceHiMemAlloc(size_t);
extern void *sceHiMemAlign(size_t, size_t);
extern void sceHiMemFree(void *);
extern void *sceHiMemRealloc(void *, size_t);
extern void *sceHiMemCalloc(size_t, size_t);

extern sceHiErr sceHiMemInit(void *, size_t);
extern sceHiErr sceHiMemGetUsedSize(int *);
extern sceHiErr sceHiMemGetNousedSize(int *);

/*********************************************************************************
 *				DMA Services
 *********************************************************************************/
typedef enum {
	SCE_UPF_S32		= 0x00,		/* 0b0000 */
	SCE_UPF_S16		= 0x01,		/* 0b0001 */
	SCE_UPF_S8		= 0x02,		/* 0b0010 */
	SCE_UPF_V2_32	= 0x04,		/* 0b0100 */
	SCE_UPF_V2_16	= 0x05,		/* 0b0101 */
	SCE_UPF_V2_8	= 0x06,		/* 0b0110 */
	SCE_UPF_V3_32	= 0x08,		/* 0b1000 */
	SCE_UPF_V3_16	= 0x09,		/* 0b1001 */
	SCE_UPF_V3_8	= 0x0a,		/* 0b1010 */
	SCE_UPF_V4_32	= 0x0c,		/* 0b1100 */
	SCE_UPF_V4_16	= 0x0d,		/* 0b1101 */
	SCE_UPF_V4_8	= 0x0e,		/* 0b1110 */
	SCE_UPF_V4_5	= 0x0f		/* 0b1111 */
} sceHiDMAUnpackFormat_t;
typedef int	sceHiDMAChainID_t;
typedef struct {
	sceHiDMAUnpackFormat_t	fmt;		/* unpack format */
	int			irq;		/* vif irq bit */
	int			pack_active;/* pack data quantity of activity */
	int			use_directhl;
} sceHiDMAState_t;

typedef enum {
    SCE_HIG_DMA_REGIST_CHAIN,
    SCE_HIG_DMA_DYNAMIC_CHAIN,
    SCE_HIG_DMA_STATIC_CHAIN
} sceHiDMAChainType_t;

extern sceHiErr sceHiDMAInit(void *(*)(size_t, size_t), void (*)(void *), size_t);
extern sceHiErr sceHiDMAInit_DBuf(int, int);
extern sceHiErr sceHiDMARegist(sceHiDMAChainID_t);
extern sceHiErr sceHiDMASend(void);
extern sceHiErr sceHiDMASendI(void);
extern sceHiErr sceHiDMAWait(void);
extern sceHiErr sceHiDMASwap(void);
extern sceHiErr sceHiDMAGetState(sceHiDMAState_t *);
extern sceHiErr sceHiDMASetState(const sceHiDMAState_t *);
extern sceHiErr sceHiDMAMake_ChainStart(void);
extern sceHiErr sceHiDMAMake_ChainEnd(sceHiDMAChainID_t *);
extern sceHiErr sceHiDMAMake_DynamicChainStart(void);
extern sceHiErr sceHiDMAMake_DynamicChainEnd(void);
extern sceHiErr sceHiDMADel_Chain(sceHiDMAChainID_t);
extern sceHiErr sceHiDMAPurge(void);
extern sceHiErr sceHiDMAMake_DBufStart(void);
extern sceHiErr sceHiDMAMake_DBufEnd(void);
extern sceHiErr sceHiDMAMake_LoadMicro(char *, size_t);
extern sceHiErr sceHiDMAMake_ExecMicro(void);
extern sceHiErr sceHiDMAMake_ExecMicroAddr(int);
extern sceHiErr sceHiDMAMake_ContinueMicro(void);
extern sceHiErr sceHiDMAMake_WaitMicro(void);
extern sceHiErr sceHiDMAMake_LoadImm(u_int *, qword);
extern sceHiErr sceHiDMAMake_LoadPtr(u_int *, u_int *, size_t);
extern sceHiErr sceHiDMAMake_CallID(sceHiDMAChainID_t);
extern sceHiErr sceHiDMAMake_CallPtr(u_int *);
extern sceHiErr sceHiDMAMake_LoadStep(u_int *, u_int *, size_t, int, int);
extern sceHiErr sceHiDMAMake_LumpStart(u_int *);
extern sceHiErr sceHiDMAMake_LumpEnd(void);
extern sceHiErr sceHiDMAMake_Lump(qword);
extern sceHiErr sceHiDMAMake_LoadGSLump(u_int *, size_t);
extern sceHiErr sceHiDMAMake_LoadGS(u_int *, size_t);
extern sceHiErr sceHiDMAGet_ChainAddr(sceHiDMAChainID_t, u_int **);
extern sceHiErr sceHiDMASet_BufferPtr(u_int *);
extern sceHiErr sceHiDMAGet_BufferPtr(u_int **);
extern qword *sceHiDMAGetChainHead(sceHiDMAChainID_t);
extern qword *sceHiDMAGetChainTail(sceHiDMAChainID_t);
extern void sceHiDMARegistClear(void);
extern void sceHiDMAMarkup(sceHiDMAChainType_t);
extern void sceHiDMADeleteMarked(void);

/*********************************************************************************
 *				Gs Services
 *********************************************************************************/
#define SCE_HIGS_MEM_TOP	(0x000000)	/* default top word address */
#define SCE_HIGS_MEM_END	(0x100000)	/* default end word address */

#define SCE_HIGS_PAGE_SIZE	(8192)		/* byte size 8*1024 */
#define SCE_HIGS_BLOCK_SIZE	(256)		/* byte size */
#define SCE_HIGS_COLUMN_SIZE	(64)		/* byte size */

#define SCE_HIGS_PAGE_ALIGN	(2048)		/* 2K word */
#define	SCE_HIGS_BLOCK_ALIGN	(64)		/* 64 word */
#define	SCE_HIGS_COLUMN_ALIGN	(16)		/* 16 word */

#define SCE_HIGS_ALIGN_ADDR(addr, align)	(((addr + (align-1))/align)*align)

typedef struct _sceHiGsMemTbl{
  u_int			align;	/* buffer alignment	*/
  u_int			addr;	/* buffer address	*/
  u_int			size;	/* buffer size	*/
  struct _sceHiGsMemTbl	*next;	/* next chunk table */
}sceHiGsMemTbl;

typedef enum _sceHiGsDisp_t{
  SCE_HIGS_NTSC,			/* NTSC non interlace */
  SCE_HIGS_NTSCI,			/* NTSC interlace */
  SCE_HIGS_PAL,				/* PAL non interlace */
  SCE_HIGS_PALI,				/* PAL interlace */
  SCE_HIGS_NTSCIH,			/* NTSC interlace field mode */
  SCE_HIGS_PALIH			/* PAL interlace field mode */
}sceHiGsDisp_t;

typedef enum _sceHiGsRGBA_t{
  SCE_HIGS_RGBA		= (0<<5),	/* PSMCT32 */
  SCE_HIGS_RGB		= (1<<5),	/* PSMCT24 */
  SCE_HIGS_RGBA16	= (2<<5),	/* PSMCT16 */
  SCE_HIGS_RGBA16S	= (3<<5)	/* PSMCT16S */
}sceHiGsRGBA_t;

typedef enum _sceHiGsReset_t{
  SCE_HIGS_RESET	= (0<<7),	/* reset GS */
  SCE_HIGS_FLUSH	= (1<<7)	/* flush GS */
}sceHiGsReset_t;

typedef enum _sceHiGsDEPTH_t{
  SCE_HIGS_DEPTH0	= (0<<8),	/* no Z */
  SCE_HIGS_DEPTH	= (1<<8),	/* PSMZ32 */
  SCE_HIGS_DEPTH24	= (2<<8),	/* PSMZ24 */
  SCE_HIGS_DEPTH16	= (3<<8),	/* PSMZ16 */
  SCE_HIGS_DEPTH16S	= (4<<8)	/* PSMZ16S */
}sceHiGsDEPTH_t;

typedef enum _sceHiGsBUFFERING_t{
  SCE_HIGS_DOUBLE	= (0<<11),	/* double buffer */
  SCE_HIGS_SINGLE	= (1<<11)	/* single buffer */
}sceHiGsBUFFERING_t;


#define SCE_HIGS_DISP_M		(0x001f)
#define SCE_HIGS_RGBA_M		(0x0060)
#define SCE_HIGS_RESET_M	(0x0080)
#define SCE_HIGS_DEPTH_M	(0x0700)
#define SCE_HIGS_BUFFERING_M	(0x1800)

#define SCE_HIGS_DISP_O		(0)
#define SCE_HIGS_RGBA_O		(5)
#define SCE_HIGS_RESET_O	(7)
#define SCE_HIGS_DEPTH_O	(8)
#define SCE_HIGS_BUFFERING_O	(8)

typedef enum _sceHiGsClear_t{
  SCE_HIGS_CLEAR_KEEP =		0,
  SCE_HIGS_CLEAR_COLOR =	1,
  SCE_HIGS_CLEAR_DEPTH =	2,
  SCE_HIGS_CLEAR_RGB =		3,
  SCE_HIGS_CLEAR_ALL =		4
}sceHiGsClear_t;

typedef struct _sceHiGsDisplay{
  int		swap;
  sceGsDBuff	dbuf;
}sceHiGsDisplay;

typedef struct _sceHiGsContext{
  sceGsFrame	frame;
  sceGsZbuf	zbuf;
  sceGsTex0	tex0;
  sceGsTex1	tex1;
  sceGsTex2	tex2;
  sceGsMiptbp1	miptbp1;
  sceGsMiptbp2	miptbp2;
  sceGsClamp	clamp;
  sceGsTest	test;
  sceGsAlpha	alpha;
  sceGsXyoffset	xyoffset;
  sceGsScissor	scissor;
  sceGsFba	fba;
}sceHiGsContext;

typedef struct _sceHiGsGeneral {
    sceGsColclamp	colclamp;
    sceGsDimx	dimx;
    sceGsDthe	dthe;
    sceGsFog	fog;
    sceGsFogcol	fogcol;
    sceGsPabe	pabe;
    sceGsTexa	texa;
    sceGsPrmode		prmode;
    sceGsPrmodecont	prmodecont;
} sceHiGsGeneral;

typedef struct _sceHiGsGifTag{
  unsigned long nloop:15;
  unsigned long eop:1;
  unsigned long id:30;
  unsigned long pre:1;
  unsigned long prim:11;
  unsigned long flg:2;
  unsigned long nreg:4;
  unsigned long regs:64;
}sceHiGsGiftag;

typedef struct _sceHiGsPacked_t{
  u_long	data;	/* :64 */
  u_char	addr;	/* :8 */
  unsigned long	padd:56;
}sceHiGsPacked_t;

typedef struct _sceHiGsPacked{
  sceHiGsGiftag		*giftag;
  sceHiGsPacked_t	*packed;
}sceHiGsPacked;

typedef enum _sceHiGsFbw_t {
    SCE_HIGS_FBW_KEEP = (-1),
    SCE_HIGS_FBW_DEFAULT = (-2)
} sceHiGsFbw_t;

typedef struct {
    sceHiGsGiftag   giftag;
    sceGsClear      clear;
} sceHiGsClearPacket  __attribute__((aligned(64)));

typedef struct {
    sceHiGsClearPacket clearp;
    sceHiGsPacked packed;		/* DMA packet area  */
    sceHiGsContext value;		/* context registers */
    u_short fbp[2];
    u_short validregs;
    u_char clearmode;
    u_char ctxt;			/* 設定するコンテキスト */
    u_char swap;			/* double buffer 時: 描画中のバッファ，他: 常に 0 */
    u_char field;
    u_char isDbuf;			/* double buffer するか */
    u_char isSync;			/* sceHiGsDisplaySwap() で swap するか */
    u_char isInterlace;
    u_char isZbuf;			/* Zbuf つきか */
    char ppos[2];				/* positions in packed[] of xyoffset, frame */
} sceHiGsCtx  __attribute__((aligned(64)));

typedef  struct {
    sceHiGsPacked packed;			/* DMA packet area */
    u_long  *value;
    u_int validregs;
} sceHiGsEnv  __attribute__((aligned(16)));

typedef enum _sceHiGsEnvValidRegs_t {  /* sceHiGsGeneral と同じならび順 */
	SCE_HIGS_VALID_COLCLAMP	= (1<<0),
	SCE_HIGS_VALID_DIMX	= (1<<1),
	SCE_HIGS_VALID_DTHE	= (1<<2),
	SCE_HIGS_VALID_FOG	= (1<<3),
	SCE_HIGS_VALID_FOGCOL	= (1<<4),
	SCE_HIGS_VALID_PABE	= (1<<5),
	SCE_HIGS_VALID_TEXA	= (1<<6),
	SCE_HIGS_VALID_PRMODE	= (1<<7),
	SCE_HIGS_VALID_PRMODECONT=(1<<8),
	/* -- */
	SCE_HIGS_VALID_TEXCLUT=(1<<9),
	SCE_HIGS_VALID_SCANMSK=(1<<10),
	SCE_HIGS_VALID_TEXFLUSH=(1<<11),
	SCE_HIGS_VALID_BITBLT=(1<<12),
	SCE_HIGS_VALID_TRXPOS=(1<<13),
	SCE_HIGS_VALID_TRXREG=(1<<14),
	SCE_HIGS_VALID_TRXDIR=(1<<15),
	SCE_HIGS_VALID_SIGNAL=(1<<16),
	SCE_HIGS_VALID_FINISH=(1<<17),
	SCE_HIGS_VALID_LABEL=(1<<18)
} sceHiGsEnvValidRegs_t;

typedef enum _sceHiGsCtxValidRegs_t {
    SCE_HIGS_VALID_FRAME    = (1<<0),
    SCE_HIGS_VALID_ZBUF	    = (1<<1),
    SCE_HIGS_VALID_TEX0	    = (1<<2),
    SCE_HIGS_VALID_TEX1	    = (1<<3),
    SCE_HIGS_VALID_TEX2	    = (1<<4),
    SCE_HIGS_VALID_MIPTBP1  = (1<<5),
    SCE_HIGS_VALID_MIPTBP2  = (1<<6),
    SCE_HIGS_VALID_CLAMP    = (1<<7),
    SCE_HIGS_VALID_TEST	    = (1<<8),
    SCE_HIGS_VALID_ALPHA    = (1<<9),
    SCE_HIGS_VALID_XYOFFSET = (1<<10),
    SCE_HIGS_VALID_SCISSOR  = (1<<11),
    SCE_HIGS_VALID_FBA      = (1<<12)
} sceHiGsCtxValidRegs_t;

extern sceHiGsCtx *sceHiGsStdCtx;
extern sceHiGsEnv *sceHiGsStdEnv;


extern sceHiErr sceHiGsMemInit(u_int addr, size_t size);
extern sceHiGsMemTbl *sceHiGsMemAlloc(u_int align, size_t size);
extern sceHiErr sceHiGsMemFree(sceHiGsMemTbl *tbl);
extern sceHiGsMemTbl *sceHiGsMemRealloc(sceHiGsMemTbl *tbl, u_int align, size_t size);
extern sceHiErr sceHiGsMemAddTbl(sceHiGsMemTbl *tbl);
extern size_t sceHiGsMemRestSize(void);
extern size_t sceHiGsMemRestSizePlus(void);
extern sceHiErr sceHiGsMemPrintTbl(void);

extern sceHiGsDisplay *sceHiGsDisplayStatus(void);
extern sceHiErr sceHiGsDisplaySet(u_int w, u_int h, u_int psm, u_int zpsm);
extern sceHiErr sceHiGsDisplayEnd(void);
extern sceHiErr sceHiGsDisplayMode(u_int mode);
extern sceHiErr sceHiGsDisplaySize(u_int width, u_int height);
extern sceHiErr sceHiGsDisplaySwap(int field);
extern sceHiErr sceHiGsClearColor(u_char red, u_char green, u_char blue, u_char alpha);
extern sceHiErr sceHiGsClearDepth(u_int z);
extern sceHiErr sceHiGsClear(u_int mode);

extern size_t sceHiGsBlockSize(u_int w, u_int h, u_int psm);
extern size_t sceHiGsPageSize(u_int w, u_int h, u_int psm);

extern sceHiErr sceHiGsContextID(int id);
extern sceHiGsContext *sceHiGsContextStatus(void);
extern sceHiGsGeneral *sceHiGsGeneralStatus(void);
extern sceHiErr sceHiGsPackedDelete(sceHiGsPacked *p);
extern sceHiGsPacked *sceHiGsPackedCreate(u_char *addr, u_short n);
extern sceHiErr sceHiGsPackedUpdate(sceHiGsPacked *p);

extern sceHiErr sceHiGsFrameRegs(int fbp, int fbw, int psm, int fbmsk);
extern sceHiErr sceHiGsZbufRegs(int zbp, int psm, int zmsk);
extern sceHiErr sceHiGsTex0Regs(int tbp0, int tbw, int psm, int tw, int th, int tcc, int tfx, int cbp, int cpsm, int csm, int csa, int cld);
extern sceHiErr sceHiGsTex1Regs(int lcm, int mxl, int mmag, int mmin, int mtba, int l, int k);
extern sceHiErr sceHiGsMiptbp1Regs(int tbp1, int tbw1, int tbp2, int tbw2, int tbp3, int tbw3);
extern sceHiErr sceHiGsMiptbp2Regs(int tbp4, int tbw4, int tbp5, int tbw5, int tbp6, int tbw6);
extern sceHiErr sceHiGsClampRegs(int wms, int wmt, int minu, int maxu, int minv, int maxv);
extern sceHiErr sceHiGsTestRegs(int ate, int atst, int aref, int afail, int date, int datm, int zte, int ztst);
extern sceHiErr sceHiGsAlphaRegs(int a, int b, int c, int d, int fix);
extern sceHiErr sceHiGsXyoffsetRegs(int ofx, int ofy);
extern sceHiErr sceHiGsFbaRegs(int fba);
extern sceHiErr sceHiGsColclampRegs(int clamp);
extern sceHiErr sceHiGsDimxRegs(int dm[16]);
extern sceHiErr sceHiGsDtheRegs(int dthe);
extern sceHiErr sceHiGsFogRegs(int f);
extern sceHiErr sceHiGsFogcolRegs(int fcr, int fcg, int fcb);
extern sceHiErr sceHiGsPabeRegs(int pabe);
extern sceHiErr sceHiGsTexaRegs(int ta0, int aem, int ta1);
extern sceHiErr sceHiGsPrmodeRegs(int iip, int tme, int fge, int abe, int aa1, int fst, int ctxt, int fix);
extern sceHiErr sceHiGsPrmodecontRegs(int ac);

extern sceHiErr sceHiGsCtxSetMax(int num);
extern sceHiErr sceHiGsCtxSetDefault(sceHiGsCtx *gsctx);
extern sceHiErr sceHiGsEnvSetDefault(sceHiGsEnv *gsenv);
extern sceHiGsCtx *sceHiGsCtxGetDefault(void);
extern sceHiGsEnv *sceHiGsEnvGetDefault(void);
extern sceHiErr sceHiGsEnvRegist(sceHiGsEnv *gsenv);
extern sceHiErr sceHiGsServiceInit(void);
extern sceHiErr sceHiGsServiceExit(void);
extern sceHiErr sceHiGsEnvUpdate(sceHiGsEnv *gsenv);
extern sceHiErr sceHiGsEnvDelete(sceHiGsEnv *gsenv);
extern sceHiGsEnv *sceHiGsEnvCreate(u_int validregs);
extern sceHiGsCtx *sceHiGsCtxCreate(int isDbuf);
extern sceHiErr sceHiGsCtxDelete(sceHiGsCtx *gsctx);
extern sceHiErr sceHiGsCtxSwapAll(int swap, int field);
extern sceHiErr sceHiGsCtxSwap(sceHiGsCtx *gsctx, int swap, int field);

extern sceHiErr sceHiGsCtxSetHalfOffset(sceHiGsCtx *gsctx, int field);

extern sceHiErr sceHiGsEnvCopy(sceHiGsEnv *dst, sceHiGsEnv *src);
extern sceHiErr sceHiGsCtxCopy(sceHiGsCtx *dst, sceHiGsCtx *src);
extern sceHiErr sceHiGsCtxRegist(sceHiGsCtx *gsctx, int clear);
extern sceHiErr sceHiGsCtxSend(sceHiGsCtx *gsctx, int clear);
extern sceHiErr sceHiGsCtxFcache(sceHiGsCtx *gsctx, int clear);
extern sceHiErr sceHiGsEnvSend(sceHiGsEnv *gsenv);
extern sceHiErr sceHiGsCtxUpdate(sceHiGsCtx *gsctx);
extern sceHiErr sceHiGsCtxSetClearColor(sceHiGsCtx *gsctx, u_char red, u_char green, u_char blue, u_char alpha);
extern sceHiErr sceHiGsCtxSetClearZ(sceHiGsCtx *gsctx, u_int z);
extern sceHiErr sceHiGsCtxSetClearMode(sceHiGsCtx *gsctx, u_int mode);
extern u_int sceHiGsCtxChkSize(sceHiGsCtx *gsctx);
extern sceHiErr sceHiGsCtxSetDepth(sceHiGsCtx *gsctx, int psm, int zpsm, int isZbuf);
extern sceHiErr sceHiGsCtxSetColorDepth(sceHiGsCtx *gsctx, int psm);
extern sceHiErr sceHiGsCtxSetZbufDepth(sceHiGsCtx *gsctx, int zpsm, int isZbuf);
extern sceHiErr sceHiGsCtxSetRect(sceHiGsCtx *gsctx, int x, int y, int w, int h, int fbw);
extern sceHiErr sceHiGsCtxSetLumpBuffer(sceHiGsCtx *gsctx, u_int bp);
extern sceHiErr sceHiGsCtxSetEachBuffer(sceHiGsCtx *gsctx, u_int fbp0, u_int fbp1, u_int zbp);
extern sceHiErr sceHiGsCtxShiftBuffers(sceHiGsCtx *gsctx, int fbpoffset);
extern sceHiErr sceHiGsCtxGetTex0(sceHiGsCtx *gsctx, u_long *tex0, int swap, int tcc, int tfx);
extern sceHiErr sceHiGsCtxSetContext(sceHiGsCtx *gsctx, int context);
extern sceHiErr sceHiGsCtxSetFrame(sceHiGsCtx *gsctx, int fbp0, int fbp1, int fbw, int psm, int fbmsk);
extern sceHiErr sceHiGsCtxSetRegZbuf(sceHiGsCtx *gsctx, int zbp, int psm, int zmsk);
extern sceHiErr sceHiGsCtxSetRegTex0(sceHiGsCtx *gsctx, int tbp0, int tbw, int psm, int tw, int th, int tcc, int tfx, int cbp, int cpsm, int csm, int csa, int cld);
extern sceHiErr sceHiGsCtxSetRegTex1(sceHiGsCtx *gsctx, int lcm, int mxl, int mmag, int mmin, int mtba, int l, int k);
extern sceHiErr sceHiGsCtxSetRegMiptbp1(sceHiGsCtx *gsctx, int tbp1, int tbw1, int tbp2, int tbw2, int tbp3, int tbw3);
extern sceHiErr sceHiGsCtxSetRegMiptbp2(sceHiGsCtx *gsctx, int tbp4, int tbw4, int tbp5, int tbw5, int tbp6, int tbw6);
extern sceHiErr sceHiGsCtxSetRegClamp(sceHiGsCtx *gsctx, int wms, int wmt, int minu, int maxu, int minv, int maxv);
extern sceHiErr sceHiGsCtxSetRegTest(sceHiGsCtx *gsctx, int ate, int atst, int aref, int afail, int date, int datm, int zte, int ztst);
extern sceHiErr sceHiGsCtxSetRegAlpha(sceHiGsCtx *gsctx, int a, int b, int c, int d, int fix);
extern sceHiErr sceHiGsCtxSetRegXyoffset(sceHiGsCtx *gsctx, int ofx, int ofy);
extern sceHiErr sceHiGsCtxSetRegFba(sceHiGsCtx *gsctx, int fba);
extern sceHiErr sceHiGsEnvSetRegColclamp(sceHiGsEnv *gsenv, int clamp);
extern sceHiErr sceHiGsEnvSetRegDimx(sceHiGsEnv *gsenv, int dm[16]);
extern sceHiErr sceHiGsEnvSetRegDthe(sceHiGsEnv *gsenv, int dthe);
extern sceHiErr sceHiGsEnvSetRegFog(sceHiGsEnv *gsenv, int f);
extern sceHiErr sceHiGsEnvSetRegFogcol(sceHiGsEnv *gsenv, int fcr, int fcg, int fcb);
extern sceHiErr sceHiGsEnvSetRegPabe(sceHiGsEnv *gsenv, int pabe);
extern sceHiErr sceHiGsEnvSetRegTexa(sceHiGsEnv *gsenv, int ta0, int aem, int ta1);
extern sceHiErr sceHiGsEnvSetRegPrmode(sceHiGsEnv *gsenv, int iip, int tme, int fge, int abe, int aa1, int fst, int ctxt, int fix);
extern sceHiErr sceHiGsEnvSetRegPrmodecont(sceHiGsEnv *gsenv, int ac);
extern sceHiErr sceHiGsCtxSetDefaultValues(sceHiGsCtx *gsctx, int psm, int zpsm, int isZbuf, int w, int h);
extern sceHiErr sceHiGsServiceSetRegistFunc(int (*func)(void *, int));
extern sceHiErr sceHiGsCtxGetRect(sceHiGsCtx *gsctx, int *xyzw);
extern sceHiErr sceHiGsCtxSetByDBuff(sceHiGsCtx *gsctx, sceGsDBuff *dbuf);
extern sceHiErr sceHiGsCtxFcacheI(sceHiGsCtx *gsctx, int clear);
extern sceHiErr sceHiGsEnvFcache(sceHiGsEnv *gsenv);
extern sceHiErr sceHiGsEnvFcacheI(sceHiGsEnv *gsenv);

extern sceHiErr sceHiGsCtxSendClear(sceHiGsCtx *gsctx);
extern sceHiErr sceHiGsCtxRegistClear(sceHiGsCtx *gsctx);
extern sceHiErr sceHiGsCtxGetClamp(sceHiGsCtx *gsctx, u_long *clamp, int w_repeat, int h_repeat);
extern sceHiErr sceHiGsEnvSetRegTexclut(sceHiGsEnv *gsenv, int cbw, int cou, int cov);
extern sceHiErr sceHiGsEnvSetRegScanmsk(sceHiGsEnv *gsenv, int msk);
extern sceHiErr sceHiGsEnvSetRegBitblt(sceHiGsEnv *gsenv, int sbp, int sbw, int spsm, int dbp, int dbw, int dpsm);
extern sceHiErr sceHiGsEnvSetRegTrxpos(sceHiGsEnv *gsenv, int ssax, int ssay, int dsax, int dsay, int dir);
extern sceHiErr sceHiGsEnvSetRegTrxreg(sceHiGsEnv *gsenv, int rrw, int rrh);
extern sceHiErr sceHiGsEnvSetRegTrxdir(sceHiGsEnv *gsenv, int xdr);
extern sceHiErr sceHiGsEnvSetRegSignal(sceHiGsEnv *gsenv, u_int id, u_int idmsk);
extern sceHiErr sceHiGsEnvSetRegLabel(sceHiGsEnv *gsenv, u_int id, u_int idmsk);


/* ERX */
extern void *sceHiGetErxEntries(void);

#ifdef __cplusplus
}
#endif
#endif /* !__HiG_H__ */
