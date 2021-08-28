/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.2.0.0
 *                           Shift-JIS
 *
 *      Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libgp - libgp.h
 *                      header file of libgp
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      1.00            
 */

/* libgp.h  */

#ifndef _LIBGP_H
#define _LIBGP_H

#include <eeregs.h>
#include <eestruct.h>
#include <eetypes.h>
#include <libdma.h>
#include <libgraph.h>
#include <libvu0.h>

typedef struct _sceGpChain {
    u_long128 *ot;      /* chain のアドレス */
    u_long128 *pKick; 	/* DMA 転送開始アドレス */
    u_long128 *pEnd;
    int resolution;
} sceGpChain;

/***********************************************
 * prim packet
 ***********************************************/

typedef struct _sceGpAdc{  /* dummy */
    int   ADC;
    int   pad;
}sceGpAdc;

typedef union _sceGpReg{  /* registers for reglist mode */
    sceGsPrim  prim;
    sceGsRgbaq rgbaq;
    sceGsSt    st;
    sceGsUv    uv;
    sceGsXyzf  xyzf;
    sceGsXyz   xyz;
    sceGsTex0  tex0;
    sceGsClamp clamp;
    sceGsFog   fog;
    sceGpAdc   adc;
    u_long   ul;
    u_int    ui[2];
} sceGpReg;

typedef union _sceGpPack{  /* registers for packed mode */
    sceGifPackRgbaq rgbaq;
    sceGifPackSt    st;
    sceGifPackUv    uv;
    sceGifPackXyzf  xyzf;
    sceGifPackXyz   xyz;
    sceGifPackFog   fog;
    sceGifPackAd    ad;
    sceGifPackNop   nop;
    sceGifPackXyzf  adc;
    sceVu0FVECTOR fv;
    sceVu0IVECTOR iv;
    u_long128 ul128;
    u_long    ul[2];
    u_int     ui[4];
    float     f[4];
} sceGpPack;


/***********************************
 * Packet structures
 ***********************************/
 
typedef struct { /* reglist mode  */
    sceDmaTag dmanext;
    sceGifTag giftag1;
    sceGifPackAd userreg;
    sceGifTag giftag2;
    sceGpReg reg[1];
} sceGpPrimR;

typedef struct { /* packed mode */
    sceDmaTag dmanext;
    sceGifTag giftag1;
    sceGifPackAd userreg;
    sceGifTag giftag2;
    sceGpPack reg[1];
} sceGpPrimP;


typedef struct { /* load 1 texture( texel or clut ) */
    sceDmaTag       dmacnt;
    sceGifTag       giftag1;
    sceGsBitbltbuf  bitbltbuf;  long bitbltbufaddr;
    sceGsTrxpos     trxpos;     long trxposaddr;
    sceGsTrxreg     trxreg;     long trxregaddr;
    sceGsTrxdir     trxdir;     long trxdiraddr;
    sceGifTag       giftag2;
    sceDmaTag       dmaref;
    sceDmaTag       dmanext;
    sceGifTag       giftag3;
    sceGsTexflush   texflush;   long texflushaddr;
} sceGpLoadImage __attribute__((aligned(16)));

typedef struct { /* load texel and clut */
    struct {
        sceDmaTag       dmacnt;
        sceGifTag       giftag1;
        sceGsBitbltbuf  bitbltbuf;  long bitbltbufaddr;
        sceGsTrxpos     trxpos;     long trxposaddr;
        sceGsTrxreg     trxreg;     long trxregaddr;
        sceGsTrxdir     trxdir;     long trxdiraddr;
        sceGifTag       giftag2;
        sceDmaTag       dmaref;
    } trans[2];
    sceDmaTag       dmanext;
    sceGifTag       giftag3;
    sceGsTexflush   texflush;   long texflushaddr;
} sceGpLoadTexelClut __attribute__((aligned(16)));

typedef struct { /* send texenv */
    sceDmaTag   dmanext;
    sceGifTag   giftag;
    sceGsTex1   tex1;           long tex1addr;
    sceGsTex0   tex0;           long tex0addr;
    sceGsClamp  clamp;          long clampaddr;
} sceGpTexEnv __attribute__((aligned(16)));


typedef struct { /* send texenv & mipmapenv */
    sceDmaTag       dmanext;
    sceGifTag       giftag;
    sceGsTex1       tex1;       long tex1addr;
    sceGsTex0       tex0;       long tex0addr;
    sceGsClamp      clamp;      long clampaddr;
    sceGsMiptbp1    miptbp1;    long miptbp1addr;
    sceGsMiptbp2    miptbp2;    long miptbp2addr;
} sceGpTexEnvMipmap __attribute__((aligned(16)));


typedef struct { /* send alphaenv */
    sceDmaTag   dmanext;
    sceGifTag   giftag;
    sceGsAlpha  alpha;  long  alphaaddr;
    sceGsPabe   pabe;   long  pabeaddr;
    sceGsTexa   texa;   long  texaaddr;
    sceGsFba    fba;    long  fbaaddr;
} sceGpAlphaEnv __attribute__((aligned(16)));


typedef struct { /* call other chain */
    sceDmaTag dmacall;
    sceDmaTag dmanext;
} sceGpCall __attribute__((aligned(16)));


typedef struct { /* ref mem region */
    sceDmaTag dmaref;
    sceDmaTag dmanext;
} sceGpRef __attribute__((aligned(16)));


typedef struct { /* general-purpose */
    sceDmaTag dmanext;
    sceGifTag giftag;
    struct {
        u_long value;
        u_long addr;
    }reg[1];
} sceGpAd __attribute__((aligned(16)));


/* Argument structures */

typedef struct{
    short tbp, tbw, tpsm;
    short tx, ty, tw, th;
    short cbp, cpsm;
} sceGpTextureArg;


enum {
    SCE_GP_PATH1,
    SCE_GP_PATH2,
    SCE_GP_PATH3
};


/* Packet ID */

#define SCE_GP_UNKNOWN          0x0000
#define SCE_GP_CHAIN            0x1000

#define SCE_GP_PRIM_R           0x2000
#define SCE_GP_PRIM_P           0x3000
#define SCE_GP_ALPHAENV         0x4000
#define SCE_GP_TEXENV           0x5000
#define SCE_GP_TEXENVMIPMAP     0x6000
#define SCE_GP_LOADIMAGE        0x7000
#define SCE_GP_LOADTEXELCLUT    0x8000
#define SCE_GP_AD               0x9000
#define SCE_GP_REF              0xa000
#define SCE_GP_CALL             0xb000


/* Prim type */

#define SCE_GP_MONOCHROME       (1<<11)

#define SCE_GP_POINT_FM  ( SCE_GS_PRIM_POINT | SCE_GP_MONOCHROME )
#define SCE_GP_POINT_FMTU  ( SCE_GS_PRIM_POINT | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_POINT_F  ( SCE_GS_PRIM_POINT )
#define SCE_GP_POINT_FTU  ( SCE_GS_PRIM_POINT | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_POINT_FTS  ( SCE_GS_PRIM_POINT | SCE_GS_PRIM_TME )

#define SCE_GP_LINE_FM  ( SCE_GS_PRIM_LINE | SCE_GP_MONOCHROME )
#define SCE_GP_LINE_FMTU  ( SCE_GS_PRIM_LINE | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_LINE_F  ( SCE_GS_PRIM_LINE )
#define SCE_GP_LINE_FTU  ( SCE_GS_PRIM_LINE | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_LINE_FTS  ( SCE_GS_PRIM_LINE | SCE_GS_PRIM_TME )
#define SCE_GP_LINE_G  ( SCE_GS_PRIM_LINE | SCE_GS_PRIM_IIP)
#define SCE_GP_LINE_GTU  ( SCE_GS_PRIM_LINE | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_LINE_GTS  ( SCE_GS_PRIM_LINE | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME )

#define SCE_GP_LINESTRIP_FM  ( SCE_GS_PRIM_LINESTRIP | SCE_GP_MONOCHROME )
#define SCE_GP_LINESTRIP_FMTU  ( SCE_GS_PRIM_LINESTRIP | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_LINESTRIP_F  ( SCE_GS_PRIM_LINESTRIP )
#define SCE_GP_LINESTRIP_FTU  ( SCE_GS_PRIM_LINESTRIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_LINESTRIP_FTS  ( SCE_GS_PRIM_LINESTRIP | SCE_GS_PRIM_TME )
#define SCE_GP_LINESTRIP_G  ( SCE_GS_PRIM_LINESTRIP | SCE_GS_PRIM_IIP)
#define SCE_GP_LINESTRIP_GTU  ( SCE_GS_PRIM_LINESTRIP | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_LINESTRIP_GTS  ( SCE_GS_PRIM_LINESTRIP | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME )

#define SCE_GP_TRI_FM  ( SCE_GS_PRIM_TRI | SCE_GP_MONOCHROME )
#define SCE_GP_TRI_FMTU  ( SCE_GS_PRIM_TRI | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRI_F  ( SCE_GS_PRIM_TRI )
#define SCE_GP_TRI_FTU  ( SCE_GS_PRIM_TRI | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRI_FTS  ( SCE_GS_PRIM_TRI | SCE_GS_PRIM_TME )
#define SCE_GP_TRI_G  ( SCE_GS_PRIM_TRI | SCE_GS_PRIM_IIP)
#define SCE_GP_TRI_GTU  ( SCE_GS_PRIM_TRI | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRI_GTS  ( SCE_GS_PRIM_TRI | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME )

#define SCE_GP_TRISTRIP_FM  ( SCE_GS_PRIM_TRISTRIP | SCE_GP_MONOCHROME )
#define SCE_GP_TRISTRIP_FMTU  ( SCE_GS_PRIM_TRISTRIP | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRISTRIP_F  ( SCE_GS_PRIM_TRISTRIP )
#define SCE_GP_TRISTRIP_FTU  ( SCE_GS_PRIM_TRISTRIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRISTRIP_FTS  ( SCE_GS_PRIM_TRISTRIP | SCE_GS_PRIM_TME )
#define SCE_GP_TRISTRIP_G  ( SCE_GS_PRIM_TRISTRIP | SCE_GS_PRIM_IIP)
#define SCE_GP_TRISTRIP_GTU  ( SCE_GS_PRIM_TRISTRIP | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRISTRIP_GTS  ( SCE_GS_PRIM_TRISTRIP | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME )

#define SCE_GP_TRIFAN_FM  ( SCE_GS_PRIM_TRIFAN | SCE_GP_MONOCHROME )
#define SCE_GP_TRIFAN_FMTU  ( SCE_GS_PRIM_TRIFAN | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRIFAN_F  ( SCE_GS_PRIM_TRIFAN )
#define SCE_GP_TRIFAN_FTU  ( SCE_GS_PRIM_TRIFAN | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRIFAN_FTS  ( SCE_GS_PRIM_TRIFAN | SCE_GS_PRIM_TME )
#define SCE_GP_TRIFAN_G  ( SCE_GS_PRIM_TRIFAN | SCE_GS_PRIM_IIP)
#define SCE_GP_TRIFAN_GTU  ( SCE_GS_PRIM_TRIFAN | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_TRIFAN_GTS  ( SCE_GS_PRIM_TRIFAN | SCE_GS_PRIM_IIP | SCE_GS_PRIM_TME )

#define SCE_GP_SPRITE_FM  ( SCE_GS_PRIM_SPRITE | SCE_GP_MONOCHROME )
#define SCE_GP_SPRITE_FMTU  ( SCE_GS_PRIM_SPRITE | SCE_GP_MONOCHROME | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_SPRITE_F  ( SCE_GS_PRIM_SPRITE )
#define SCE_GP_SPRITE_FTU  ( SCE_GS_PRIM_SPRITE | SCE_GS_PRIM_TME | SCE_GS_PRIM_FST )
#define SCE_GP_SPRITE_FTS  ( SCE_GS_PRIM_SPRITE | SCE_GS_PRIM_TME )

/* for sceGpSetAlphaEnv() */

#define SCE_GP_ALPHA_NOP            (0)
#define SCE_GP_ALPHA_INTER_AS       (1)
#define SCE_GP_ALPHA_INTER_AD       (2)
#define SCE_GP_ALPHA_INTER_FIX      (3)
#define SCE_GP_ALPHA_RINTER_AS      (4)
#define SCE_GP_ALPHA_RINTER_AD      (5)
#define SCE_GP_ALPHA_RINTER_FIX     (6)
#define SCE_GP_ALPHA_ADD            (7)
#define SCE_GP_ALPHA_ADD_CS_FIX     (8)
#define SCE_GP_ALPHA_ADD_CD_FIX     (9)
#define SCE_GP_ALPHA_ADD_CS_AS      (10)
#define SCE_GP_ALPHA_ADD_CD_AS      (11)
#define SCE_GP_ALPHA_ADD_CS_AD      (12)
#define SCE_GP_ALPHA_ADD_CD_AD      (13)
#define SCE_GP_ALPHA_SUB_CS         (14)
#define SCE_GP_ALPHA_SUB_CD         (15)
#define SCE_GP_ALPHA_SUB_CS_FIX     (16)
#define SCE_GP_ALPHA_SUB_CD_FIX     (17)
#define SCE_GP_ALPHA_SUB_CS_AS      (18)
#define SCE_GP_ALPHA_SUB_CD_AS      (19)
#define SCE_GP_ALPHA_SUB_CS_AD      (20)
#define SCE_GP_ALPHA_SUB_CD_AD      (21)
#define SCE_GP_ALPHA_MUL_CS_AS      (22)
#define SCE_GP_ALPHA_MUL_CS_AD      (23)
#define SCE_GP_ALPHA_MUL_CS_FIX     (24)
#define SCE_GP_ALPHA_MUL_CD_AS      (25)
#define SCE_GP_ALPHA_MUL_CD_AD      (26)
#define SCE_GP_ALPHA_MUL_CD_FIX     (27)


#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern void *sceGpGetErxEntries(void);


/*   chain(dma) operate functions   */

extern int sceGpInitChain(sceGpChain *chain, void *addr, int resolution);
extern void sceGpResetChain(sceGpChain *chain);
extern void sceGpResetChainRev(sceGpChain *chain);
extern int sceGpKickChain(sceGpChain *chain, int path);
extern void sceGpKickChain2(sceGpChain *chain, int path);

extern void sceGpPrintChain(sceGpChain *chain, int verbosity, int from, int num);
extern void sceGpTermChain(sceGpChain *chain, int level, int isret);
extern void sceGpSetStartLevel(sceGpChain *chain, int level);
extern void sceGpSetEndLevel(sceGpChain *chain, int level);
extern void sceGpAddChain(sceGpChain* chain, int level, sceGpChain *chain2);
extern void sceGpCallChain(sceGpChain* chain, int level, sceGpChain* chain2, sceGpCall* calltag);

extern sceDmaTag* sceGpSearchTailToRemove(sceGpChain* chain, void* p);
extern void* sceGpRemoveNextPacket(sceDmaTag* tail2remv);
extern sceGpChain* sceGpRemoveNextChain(sceDmaTag* tail2remv, sceGpChain* chain);

extern void sceGpInsertPacket(void* b, void* p);
extern void sceGpAddPacket(sceGpChain* chain, int level, void* p);
extern int sceGpCopyPacket(void* dp, void* sp);

extern sceDmaTag *sceGpAddPacket2(sceDmaTag *tail2remv, void* pPacket);
extern sceDmaTag *sceGpAddChain2(sceDmaTag *tail2remv, sceGpChain *chain2);
extern sceDmaTag *sceGpCallChain2(sceDmaTag *tail2remv, sceGpChain* chain2, sceGpCall* calltag);
extern sceDmaTag *sceGpGetTailChain(sceGpChain *chain, int level);
extern sceDmaTag *sceGpGetTail(void *pPacket);

extern int sceGpKickPacket(void* p, int path);
extern void sceGpKickPacket2(void* p, int path);
extern void sceGpSyncPacket(void* p);
extern void sceGpSyncPacketI(void* p);
extern void sceGpSetPacketMode(void* p, int mode);


#define sceGpChkChainOtSize(r)  ((r)+1)


/*    packet operate functions    */

extern void sceGpSetDefaultCtxt(int ctxt);
extern void sceGpSetDefaultAa1(int aa1);
extern void sceGpSetDefaultAbe(int abe);
extern void sceGpSetDefaultFog(int fge);
extern void sceGpSetDefaultDirectHL(int on);
extern void sceGpSetDefaultZ32(int on);
extern int sceGpChkNumPtoV(u_int type, int pnum);
extern int sceGpChkPacketSize(u_int type, int pnum);

extern int sceGpInitPacket(void *p, u_int type, int num);

extern int sceGpInitPrimR(sceGpPrimR *p, u_int type, int pnum);
extern int sceGpInitPrimP(sceGpPrimP *p, u_int type, int pnum);
extern int sceGpInitLoadImage(sceGpLoadImage *p);
extern int sceGpInitLoadTexelClut(sceGpLoadTexelClut *p);
extern int sceGpInitTexEnv(sceGpTexEnv *p, int ctxt);
extern int sceGpInitTexEnvMipmap(sceGpTexEnvMipmap *p, int ctxt);
extern int sceGpInitAlphaEnv(sceGpAlphaEnv *p, int ctxt);
extern int sceGpInitAd(sceGpAd *p, int size);
extern int sceGpInitRef(sceGpRef *p);
extern int sceGpInitCall(sceGpCall *p);

extern void sceGpSetLoadImageByTim2(sceGpLoadImage *p, const void *ptim2, int picture, int miplevel, int isClut);

extern void sceGpSetLoadImageByArgTim2(sceGpLoadImage *p, const sceGpTextureArg *arg, const void *ptim2, int picture, int miplevel, int isClut);

extern void sceGpSetLoadTexelClutByTim2(sceGpLoadTexelClut *p, const void *ptim2, int picture, int miplevel);

extern void sceGpSetLoadTexelClutByArgTim2(sceGpLoadTexelClut *p, const sceGpTextureArg *arg, const void *ptim2, int picture, int miplevel);

extern void sceGpSetTexEnvByTim2(sceGpTexEnv *p, const void *ptim2, int picture, int miplevel);

extern void sceGpSetTexEnvByArgTim2(sceGpTexEnv *p, const sceGpTextureArg *arg, const void *ptim2, int picture, int miplevel);

extern void sceGpSetLoadImage(sceGpLoadImage *p, sceGpTextureArg *texarg, void *srcaddr, int isClut);


extern void sceGpSetLoadTexelClut(sceGpLoadTexelClut *p, sceGpTextureArg *texarg, void *tsrcaddr, void *csrcaddr);

extern void sceGpSetTexEnv(sceGpTexEnv *p, sceGpTextureArg *tex, int tfx, int filter);


extern void sceGpSetTexEnvByDrawEnv(sceGpTexEnv *p, sceGsDrawEnv1 *draw, int tfx, int filter);

extern void sceGpSetAlphaEnv(sceGpAlphaEnv* p, int func, int fix);
extern void sceGpSetAd(sceGpAd *p, int level, u_long addr, u_long value);
extern void sceGpSetRef(sceGpRef *p, void* addr, int size, int path);
extern void sceGpSetCall(sceGpCall *p, void* addr);

extern void sceGpSetAlphaEnvFunc(sceGpAlphaEnv* p, int func, int fix);



/**************  GetIndex functions  **************/

/* point */
static __inline__ unsigned int sceGpIndexXyzfPointFM(unsigned int n)      { return n;        }
static __inline__ unsigned int sceGpIndexUvPointFMTU(unsigned int n)      { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfPointFMTU(unsigned int n)    { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaPointF(unsigned int n)       { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfPointF(unsigned int n)       { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvPointFTU(unsigned int n)       { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaPointFTU(unsigned int n)     { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfPointFTU(unsigned int n)     { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqPointFTS_P(unsigned int n)    { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaPointFTS(unsigned int n)     { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfPointFTS(unsigned int n)     { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStPointFTS_R(unsigned int n)     { return n*3;      }
static __inline__ unsigned int sceGpIndexQPointFTS_R(unsigned int n)      { return n*3 + 1;  }

/* line */
static __inline__ unsigned int sceGpIndexXyzfLineFM(unsigned int n)       { return n;        }
static __inline__ unsigned int sceGpIndexUvLineFMTU(unsigned int n)       { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfLineFMTU(unsigned int n)     { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaLineF(unsigned int n)        { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfLineF(unsigned int n)        { return (n*3+1)/2;}
static __inline__ unsigned int sceGpIndexUvLineFTU(unsigned int n)        { return n*5/2;    }
static __inline__ unsigned int sceGpIndexRgbaLineFTU(unsigned int n)      { return n*5 + 3;  }
static __inline__ unsigned int sceGpIndexXyzfLineFTU(unsigned int n)      { return (n*5+3)/2;}
static __inline__ unsigned int sceGpIndexStqLineFTS_P(unsigned int n)     { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineFTS(unsigned int n)      { return n*6 + 4;  }
static __inline__ unsigned int sceGpIndexXyzfLineFTS(unsigned int n)      { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStLineFTS_R(unsigned int n)      { return n*3;      }
static __inline__ unsigned int sceGpIndexQLineFTS_R(unsigned int n)       { return n*3 + 1;  }

static __inline__ unsigned int sceGpIndexRgbaLineG(unsigned int n)        { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfLineG(unsigned int n)        { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvLineGTU(unsigned int n)        { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineGTU(unsigned int n)      { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfLineGTU(unsigned int n)      { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqLineGTS_P(unsigned int n)     { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineGTS(unsigned int n)      { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfLineGTS(unsigned int n)      { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStLineGTS_R(unsigned int n)      { return n*3;      }
static __inline__ unsigned int sceGpIndexQLineGTS_R(unsigned int n)       { return n*3 + 1;  }

/* line strip */
static __inline__ unsigned int sceGpIndexXyzfLineStripFM(unsigned int n)  { return n;        }
static __inline__ unsigned int sceGpIndexUvLineStripFMTU(unsigned int n)  { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfLineStripFMTU(unsigned int n){ return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaLineStripF(unsigned int n)   { return n*2 + 2;  }
static __inline__ unsigned int sceGpIndexXyzfLineStripF(unsigned int n)   { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvLineStripFTU(unsigned int n)   { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineStripFTU(unsigned int n) { return n*3 + 4;  }
static __inline__ unsigned int sceGpIndexXyzfLineStripFTU(unsigned int n) { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqLineStripFTS_P(unsigned int n){ return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineStripFTS(unsigned int n) { return n*3 + 4;  }
static __inline__ unsigned int sceGpIndexXyzfLineStripFTS(unsigned int n) { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStLineStripFTS_R(unsigned int n) { return n*3;      }
static __inline__ unsigned int sceGpIndexQLineStripFTS_R(unsigned int n)  { return n*3 + 1;  }

static __inline__ unsigned int sceGpIndexRgbaLineStripG(unsigned int n)   { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfLineStripG(unsigned int n)   { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvLineStripGTU(unsigned int n)   { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineStripGTU(unsigned int n) { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfLineStripGTU(unsigned int n) { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqLineStripGTS_P(unsigned int n){ return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaLineStripGTS(unsigned int n) { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfLineStripGTS(unsigned int n) { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStLineStripGTS_R(unsigned int n) { return n*3;      }
static __inline__ unsigned int sceGpIndexQLineStripGTS_R(unsigned int n)  { return n*3 + 1;  }

/* tri */
static __inline__ unsigned int sceGpIndexXyzfTriFM(unsigned int n)        { return n;        }
static __inline__ unsigned int sceGpIndexUvTriFMTU(unsigned int n)        { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfTriFMTU(unsigned int n)      { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaTriF(unsigned int n)         { return n*4 + 2;  }
static __inline__ unsigned int sceGpIndexXyzfTriF(unsigned int n)         { return (n*4+1)/3;}
static __inline__ unsigned int sceGpIndexUvTriFTU(unsigned int n)         { return n*7/3;    }
static __inline__ unsigned int sceGpIndexRgbaTriFTU(unsigned int n)       { return n*7 + 5;  }
static __inline__ unsigned int sceGpIndexXyzfTriFTU(unsigned int n)       { return (n*7+4)/3;}
static __inline__ unsigned int sceGpIndexStqTriFTS_P(unsigned int n)      { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriFTS(unsigned int n)       { return n*9 + 7;  }
static __inline__ unsigned int sceGpIndexXyzfTriFTS(unsigned int n)       { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStTriFTS_R(unsigned int n)       { return n*3;      }
static __inline__ unsigned int sceGpIndexQTriFTS_R(unsigned int n)        { return n*3 + 1;  }

static __inline__ unsigned int sceGpIndexRgbaTriG(unsigned int n)         { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfTriG(unsigned int n)         { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvTriGTU(unsigned int n)         { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriGTU(unsigned int n)       { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfTriGTU(unsigned int n)       { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqTriGTS_P(unsigned int n)      { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriGTS(unsigned int n)       { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfTriGTS(unsigned int n)       { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStTriGTS_R(unsigned int n)       { return n*3;      }
static __inline__ unsigned int sceGpIndexQTriGTS_R(unsigned int n)        { return n*3 + 1;  }

/* tri strip */
static __inline__ unsigned int sceGpIndexXyzfTriStripFM(unsigned int n)   { return n;        }
static __inline__ unsigned int sceGpIndexUvTriStripFMTU(unsigned int n)   { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfTriStripFMTU(unsigned int n) { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaTriStripF(unsigned int n)    { return n*2 + 4;  }
static __inline__ unsigned int sceGpIndexXyzfTriStripF(unsigned int n)    { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvTriStripFTU(unsigned int n)    { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriStripFTU(unsigned int n)  { return n*3 + 7;  }
static __inline__ unsigned int sceGpIndexXyzfTriStripFTU(unsigned int n)  { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqTriStripFTS_P(unsigned int n) { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriStripFTS(unsigned int n)  { return n*3 + 7;  }
static __inline__ unsigned int sceGpIndexXyzfTriStripFTS(unsigned int n)  { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStTriStripFTS_R(unsigned int n)  { return n*3;      }
static __inline__ unsigned int sceGpIndexQTriStripFTS_R(unsigned int n)   { return n*3 + 1;  }

static __inline__ unsigned int sceGpIndexRgbaTriStripG(unsigned int n)    { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfTriStripG(unsigned int n)    { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvTriStripGTU(unsigned int n)    { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriStripGTU(unsigned int n)  { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfTriStripGTU(unsigned int n)  { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqTriStripGTS_P(unsigned int n) { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriStripGTS(unsigned int n)  { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfTriStripGTS(unsigned int n)  { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStTriStripGTS_R(unsigned int n)  { return n*3;      }
static __inline__ unsigned int sceGpIndexQTriStripGTS_R(unsigned int n)   { return n*3 + 1;  }

/* tri fan */
static __inline__ unsigned int sceGpIndexXyzfTriFanFM(unsigned int n)     { return n;        }
static __inline__ unsigned int sceGpIndexUvTriFanFMTU(unsigned int n)     { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfTriFanFMTU(unsigned int n)   { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaTriFanF(unsigned int n)      { return n*2 + 4;  }
static __inline__ unsigned int sceGpIndexXyzfTriFanF(unsigned int n)      { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvTriFanFTU(unsigned int n)      { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriFanFTU(unsigned int n)    { return n*3 + 7;  }
static __inline__ unsigned int sceGpIndexXyzfTriFanFTU(unsigned int n)    { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqTriFanFTS_P(unsigned int n)   { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriFanFTS(unsigned int n)    { return n*3 + 7;  }
static __inline__ unsigned int sceGpIndexXyzfTriFanFTS(unsigned int n)    { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStTriFanFTS_R(unsigned int n)    { return n*3;      }
static __inline__ unsigned int sceGpIndexQTriFanFTS_R(unsigned int n)     { return n*3 + 1;  }

static __inline__ unsigned int sceGpIndexRgbaTriFanG(unsigned int n)      { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfTriFanG(unsigned int n)      { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexUvTriFanGTU(unsigned int n)      { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriFanGTU(unsigned int n)    { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfTriFanGTU(unsigned int n)    { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStqTriFanGTS_P(unsigned int n)   { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaTriFanGTS(unsigned int n)    { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfTriFanGTS(unsigned int n)    { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStTriFanGTS_R(unsigned int n)    { return n*3;      }
static __inline__ unsigned int sceGpIndexQTriFanGTS_R(unsigned int n)     { return n*3 + 1;  }

/* sprite */
static __inline__ unsigned int sceGpIndexXyzfSpriteFM(unsigned int n)     { return n;        }
static __inline__ unsigned int sceGpIndexUvSpriteFMTU(unsigned int n)     { return n*2;      }
static __inline__ unsigned int sceGpIndexXyzfSpriteFMTU(unsigned int n)   { return n*2 + 1;  }
static __inline__ unsigned int sceGpIndexRgbaSpriteF(unsigned int n)      { return n*3 + 1;  }
static __inline__ unsigned int sceGpIndexXyzfSpriteF(unsigned int n)      { return (n*3+1)/2;}
static __inline__ unsigned int sceGpIndexUvSpriteFTU(unsigned int n)      { return n*5/2;    }
static __inline__ unsigned int sceGpIndexRgbaSpriteFTU(unsigned int n)    { return n*5 + 3;  }
static __inline__ unsigned int sceGpIndexXyzfSpriteFTU(unsigned int n)    { return (n*5+3)/2;}
static __inline__ unsigned int sceGpIndexStqSpriteFTS_P(unsigned int n)   { return n*3;      }
static __inline__ unsigned int sceGpIndexRgbaSpriteFTS(unsigned int n)    { return n*6 + 4;  }
static __inline__ unsigned int sceGpIndexXyzfSpriteFTS(unsigned int n)    { return n*3 + 2;  }
static __inline__ unsigned int sceGpIndexStSpriteFTS_R(unsigned int n)    { return n*3;      }
static __inline__ unsigned int sceGpIndexQSpriteFTS_R(unsigned int n)     { return n*3 + 1;  }


/* prim type compatible */
extern unsigned int sceGpIndexUv(u_int type, unsigned int n);

static __inline__ unsigned int sceGpIndexStq_P(u_int type, unsigned int n)
{
    (void)type; /* suppress warning */
    return n*3;
}

static __inline__ unsigned int sceGpIndexSt_R(u_int type, unsigned int n)
{
    (void)type; /* suppress warning */
    return n*3;
}

static __inline__ unsigned int sceGpIndexQ_R(u_int type, unsigned int n)
{
    (void)type; /* suppress warning */
    return n*3 + 1;
}

static __inline__ void sceGpSetRgbaFM(void* p, u_long r, u_long g, u_long b, u_long a)
{
    ((sceGpPrimR*)p)->userreg.ADDR = SCE_GS_RGBAQ;
    ((sceGpPrimR*)p)->userreg.DATA = SCE_GS_SET_RGBAQ(r,g,b,a,0x3f800000);
}

extern unsigned int sceGpIndexRgba(u_int type, unsigned int n);
extern unsigned int sceGpIndexXyzf(u_int type, unsigned int n);


#define sceGpSetRgb(p, k, r, g, b) ((p)->reg[k].rgbaq.R=(r),(p)->reg[k].rgbaq.G=(g),(p)->reg[k].rgbaq.B=(b))
#define sceGpSetRgba(p, k, r, g, b, a) ((p)->reg[k].rgbaq.R=(r),(p)->reg[k].rgbaq.G=(g),(p)->reg[k].rgbaq.B=(b),(p)->reg[k].rgbaq.A=(a))

#define sceGpSetXy(p, k, x, y) ((p)->reg[k].adc.ADC=0,(p)->reg[k].xyzf.X=(x),(p)->reg[k].xyzf.Y=(y))
#define sceGpSetXyz(p, k, x, y, z) ((p)->reg[k].adc.ADC=0,(p)->reg[k].xyzf.X=(x),(p)->reg[k].xyzf.Y=(y),(p)->reg[k].xyzf.Z=(z))
#define sceGpSetXyzf(p, k, x, y, z, f) ((p)->reg[k].adc.ADC=0,(p)->reg[k].xyzf.X=(x),(p)->reg[k].xyzf.Y=(y),(p)->reg[k].xyzf.Z=(z),(p)->reg[k].xyzf.F=(f))
#define sceGpSetXyz32(p, k, x, y, z) ((p)->reg[k].adc.ADC=0,(p)->reg[k].xyz.X=(x),(p)->reg[k].xyz.Y=(y),(p)->reg[k].xyz.Z=(z))

#define sceGpSetUv(p, k, u, v) ((p)->reg[k].uv.U=(u),(p)->reg[k].uv.V=(v))
#define sceGpSetStq_R(p, k, s, t, q) ((p)->reg[k].st.S=(s),(p)->reg[k].st.T=(t),(p)->reg[k+1].rgbaq.Q=(q))
#define sceGpSetStq_P(p, k, s, t, q) ((p)->reg[k].st.S=(s),(p)->reg[k].st.T=(t),(p)->reg[k].st.Q=(q))


#define sceGpSetFog(p,v) ((p)->giftag1.PRIM = ((v)?((p)->giftag1.PRIM|GS_PRIM_FGE_M):((p)->giftag1.PRIM&(~GS_PRIM_FGE_M))))

#define sceGpSetAa1(p,v) ((p)->giftag1.PRIM = ((v)?((p)->giftag1.PRIM|GS_PRIM_AA1_M):((p)->giftag1.PRIM&(~GS_PRIM_AA1_M))))

#define sceGpSetCtxt(p,v) ((p)->giftag1.PRIM = ((v)?((p)->giftag1.PRIM|GS_PRIM_CTXT_M):((p)->giftag1.PRIM&(~GS_PRIM_CTXT_M))))

#define sceGpSetAbe(p,v) ((p)->giftag1.PRIM = ((v)?((p)->giftag1.PRIM|GS_PRIM_ABE_M):((p)->giftag1.PRIM&(~GS_PRIM_ABE_M))))

extern void sceGpSetZ32(void* p, int on);


#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _LIBGP_H */
