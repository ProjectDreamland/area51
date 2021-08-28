/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.1.0.0
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libgraph - libgraph.h
 *                      header file of libgraph
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.10            March.25.1999   okadaa      the first version
 *      0.2.0.0	        May.11.1999     okadaa      add functions for 2context
 *      1.10            Feb,  8, 2002   ywashizu    Added DTV480P macro
 * 
 * $Id: libgraph.h,v 1.18 2003/05/22 06:34:08 xkazama Exp $
 *
 */


/* libgraph.h - definitions to use GS. */

#ifndef _LIBGRAPH_H
#define _LIBGRAPH_H

#include <eeregs.h>
#include <eestruct.h>
#include <eetypes.h>

/* Functions for control GS */

typedef struct {
	tGS_PMODE	pmode;
	tGS_SMODE2	smode2;
	tGS_DISPFB2	dispfb;
	tGS_DISPLAY2	display;
	tGS_BGCOLOR	bgcolor;
} sceGsDispEnv;

typedef struct {
	sceGsFrame	frame1;
	u_long		frame1addr;
	sceGsZbuf	zbuf1;
	long		zbuf1addr;
	sceGsXyoffset	xyoffset1;
	long		xyoffset1addr;
	sceGsScissor	scissor1;
	long		scissor1addr;
	sceGsPrmodecont	prmodecont;
	long		prmodecontaddr;
	sceGsColclamp	colclamp;
	long		colclampaddr;
	sceGsDthe	dthe;
	long		dtheaddr;
	sceGsTest	test1;
	long		test1addr;
} sceGsDrawEnv1 __attribute__((aligned(16)));

typedef struct {
	sceGsFrame	frame2;
	u_long		frame2addr;
	sceGsZbuf	zbuf2;
	long		zbuf2addr;
	sceGsXyoffset	xyoffset2;
	long		xyoffset2addr;
	sceGsScissor	scissor2;
	long		scissor2addr;
	sceGsPrmodecont	prmodecont;
	long		prmodecontaddr;
	sceGsColclamp	colclamp;
	long		colclampaddr;
	sceGsDthe	dthe;
	long		dtheaddr;
	sceGsTest	test2;
	long		test2addr;
} sceGsDrawEnv2 __attribute__((aligned(16)));

typedef struct {
	sceGsTest	testa;
	long		testaaddr;
	sceGsPrim	prim;
	long		primaddr;
	sceGsRgbaq	rgbaq;
	long		rgbaqaddr;
	sceGsXyz	xyz2a;
	long		xyz2aaddr;
	sceGsXyz	xyz2b;
	long		xyz2baddr;
	sceGsTest	testb;
	long		testbaddr;
} sceGsClear __attribute__((aligned(16)));

typedef struct {
	sceGsDispEnv	disp[2];
	sceGifTag	giftag0;
	sceGsDrawEnv1	draw0;
	sceGsClear	clear0;
	sceGifTag	giftag1;
	sceGsDrawEnv1	draw1;
	sceGsClear	clear1;
} sceGsDBuff;

typedef struct {
	sceGsDispEnv	disp[2];
	sceGifTag	giftag0;
	sceGsDrawEnv1	draw01;
	sceGsDrawEnv2	draw02;
	sceGsClear	clear0;
	sceGifTag	giftag1;
	sceGsDrawEnv1	draw11;
	sceGsDrawEnv2	draw12;
	sceGsClear	clear1;
} sceGsDBuffDc;

typedef struct {
	sceGsTexflush	texflush;
	long		texflushaddr;
	sceGsTex1	tex11;
	long		tex11addr;
	sceGsTex0	tex01;
	long		tex01addr;
	sceGsClamp	clamp1;
	long		clamp1addr;
} sceGsTexEnv __attribute__((aligned(16)));

typedef struct {
	sceGsTexflush	texflush;
	long		texflushaddr;
	sceGsTex1	tex12;
	long		tex12addr;
	sceGsTex0	tex02;
	long		tex02addr;
	sceGsClamp	clamp2;
	long		clamp2addr;
} sceGsTexEnv2 __attribute__((aligned(16)));

typedef struct {
	sceGsAlpha	alpha1;
	long		alpha1addr;
	sceGsPabe	pabe;
	long		pabeaddr;
	sceGsTexa	texa;
	long		texaaddr;
	sceGsFba	fba1;
	long		fba1addr;
} sceGsAlphaEnv __attribute__((aligned(16)));

typedef struct {
	sceGsAlpha	alpha2;
	long		alpha2addr;
	sceGsPabe	pabe;
	long		pabeaddr;
	sceGsTexa	texa;
	long		texaaddr;
	sceGsFba	fba2;
	long		fba2addr;
} sceGsAlphaEnv2 __attribute__((aligned(16)));

typedef struct {
	sceGifTag	giftag0;
	sceGsBitbltbuf	bitbltbuf;
	long		bitbltbufaddr;
	sceGsTrxpos	trxpos;
	long		trxposaddr;
	sceGsTrxreg	trxreg;
	long		trxregaddr;
	sceGsTrxdir	trxdir;
	long		trxdiraddr;
	sceGifTag	giftag1;
} sceGsLoadImage __attribute__((aligned(16)));

typedef struct {
	u_int		vifcode[4];
	sceGifTag	giftag;
	sceGsBitbltbuf	bitbltbuf;
	long		bitbltbufaddr;
	sceGsTrxpos	trxpos;
	long		trxposaddr;
	sceGsTrxreg	trxreg;
	long		trxregaddr;
	sceGsFinish	finish;
	long		finishaddr;
	sceGsTrxdir	trxdir;
	long		trxdiraddr;
} sceGsStoreImage __attribute__((aligned(16)));

typedef struct {
	short		sceGsInterMode;
	short		sceGsOutMode;
	short		sceGsFFMode;
	short		sceGsVersion;
	int		(*sceGsVSCfunc)(int);
	int		sceGsVSCid;
} sceGsGParam __attribute__((aligned(16)));

#define SCE_GS_NOINTERLACE		(0)
#define SCE_GS_INTERLACE		(1)
#define	SCE_GS_FIELD			(0)
#define	SCE_GS_FRAME			(1)
#define SCE_GS_NTSC			(0x2)
#define SCE_GS_PAL			(0x3)
#define SCE_GS_VESA1A			(0x1a)    /* System Use */
#define SCE_GS_VESA1B			(0x1b)    /* System Use */
#define SCE_GS_VESA1C			(0x1c)    /* System Use */
#define SCE_GS_VESA1D			(0x1d)    /* System Use */
#define SCE_GS_VESA2A			(0x2a)    /* System Use */
#define SCE_GS_VESA2B			(0x2b)    /* System Use */
#define SCE_GS_VESA2C			(0x2c)    /* System Use */
#define SCE_GS_VESA2D			(0x2d)    /* System Use */
#define SCE_GS_VESA2E			(0x2e)    /* System Use */
#define SCE_GS_VESA3B			(0x3b)    /* System Use */
#define SCE_GS_VESA3C			(0x3c)    /* System Use */
#define SCE_GS_VESA3D			(0x3d)    /* System Use */
#define SCE_GS_VESA3E			(0x3e)    /* System Use */
#define SCE_GS_VESA4A			(0x4a)    /* System Use */
#define SCE_GS_VESA4B			(0x4b)    /* System Use */
#define SCE_GS_DTV480P			(0x50)
#define SCE_GS_PSMCT32			(0)
#define SCE_GS_PSMCT24			(1)
#define SCE_GS_PSMCT16			(2)
#define SCE_GS_PSMCT16S			(10)
#define SCE_GS_PSMT8			(19)
#define SCE_GS_PSMT4			(20)
#define SCE_GS_PSMT8H			(27)
#define SCE_GS_PSMT4HL			(36)
#define SCE_GS_PSMT4HH			(44)
#define SCE_GS_PSMZ32			(48)
#define SCE_GS_PSMZ24			(49)
#define SCE_GS_PSMZ16			(50)
#define SCE_GS_PSMZ16S			(58)
#define SCE_GS_ZNOUSE			(0)
#define SCE_GS_ZALWAYS			(1)
#define SCE_GS_ZGEQUAL			(2)
#define SCE_GS_ZGREATER			(3)
#define SCE_GS_NOCLEAR			(0)
#define SCE_GS_CLEAR			(1)
#define SCE_GS_MODULATE			(0)
#define SCE_GS_DECAL			(1)
#define SCE_GS_HILIGHT			(2)
#define SCE_GS_GHLIGHT2   		SCE_GS_HIGHLIGHT2
#define SCE_GS_HIGHLIGHT2		(3)
#define SCE_GS_NEAREST			(0)
#define SCE_GS_LINEAR			(1)
#define SCE_GS_NEAREST_MIPMAP_NEAREST	(2)
#define SCE_GS_NEAREST_MIPMAP_LINEAR	SCE_GS_NEAREST_MIPMAP_LENEAR
#define SCE_GS_NEAREST_MIPMAP_LENEAR	(3)
#define SCE_GS_LINEAR_MIPMAP_NEAREST	(4)
#define SCE_GS_LINEAR_MIPMAP_LINEAR	(5)
#define SCE_GS_PRIM_POINT		(0)
#define SCE_GS_PRIM_LINE		(1)
#define SCE_GS_PRIM_LINESTRIP		(2)
#define SCE_GS_PRIM_TRI			(3)
#define SCE_GS_PRIM_TRISTRIP		(4)
#define SCE_GS_PRIM_TRIFAN		(5)
#define SCE_GS_PRIM_SPRITE		(6)
#define SCE_GS_PRIM_IIP			(1<<3)
#define SCE_GS_PRIM_TME			(1<<4)
#define SCE_GS_PRIM_FGE			(1<<5)
#define SCE_GS_PRIM_ABE			(1<<6)
#define SCE_GS_PRIM_AA1			(1<<7)
#define SCE_GS_PRIM_FST			(1<<8)
#define SCE_GS_PRIM_CTXT1		(0)
#define SCE_GS_PRIM_CTXT2		(1<<9)
#define SCE_GS_PRIM_FIX			(1<<10)

#define	SCE_GS_FALSE			(0)
#define	SCE_GS_TRUE			(1)

#define SCE_GS_REPEAT			(0)
#define SCE_GS_CLAMP			(1)
#define SCE_GS_REGION_CLAMP		(2)
#define SCE_GS_REGION_REPEAT		(3)

#define SCE_GS_DEPTH_NEVER		(0)
#define SCE_GS_DEPTH_ALWAYS		(1)
#define SCE_GS_DEPTH_GEQUAL		(2)
#define SCE_GS_DEPTH_GREATER		(3)

#define SCE_GS_ALPHA_NEVER		(0)
#define SCE_GS_ALPHA_ALWAYS		(1)
#define SCE_GS_ALPHA_LESS		(2)
#define SCE_GS_ALPHA_LEQUAL		(3)
#define SCE_GS_ALPHA_EQUAL		(4)
#define SCE_GS_ALPHA_GEQUAL		(5)
#define SCE_GS_ALPHA_GREATER		(6)
#define SCE_GS_ALPHA_NOTEQUAL		(7)

#define	SCE_GS_AFAIL_KEEP		(0)
#define	SCE_GS_AFAIL_FB_ONLY		(1)
#define SCE_GS_AFAIL_ZB_ONLY		(2)
#define SCE_GS_AFAIL_RGB_ONLY		(3)

#define SCE_GS_ALPHA_CS			(0)
#define SCE_GS_ALPHA_CD			(1)
#define SCE_GS_ALPHA_ZERO		(2)
#define SCE_GS_ALPHA_AS			(0)
#define SCE_GS_ALPHA_AD			(1)
#define SCE_GS_ALPHA_FIX		(2)

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

void sceGsResetGraph(short mode, short inter, short omode, short ffmode);
sceGsGParam *sceGsGetGParam(void);
void sceGsResetPath(void);
void sceGsSetDefDispEnv(sceGsDispEnv *disp, short psm, short w, short h, short dx, short dy);
void sceGsPutDispEnv(sceGsDispEnv *disp);
int sceGsSetDefDrawEnv(sceGsDrawEnv1 *draw, short psm, short w, short h, short ztest, short zpsm);
int sceGsPutDrawEnv(sceGifTag *giftag);
void sceGsSetDefDBuff(sceGsDBuff *dp, short psm, short w, short h, short ztest, short zpsm,  short clear);
int sceGsSwapDBuff(sceGsDBuff *db, int id);
int sceGsSyncV(int mode);
int sceGsSyncPath(int mode, u_short timeout);
int sceGsSetDefTexEnv(sceGsTexEnv *tp, short flush, short tbp0, short tbw, short psm, short w, short h, short tfx, short cbp, short cpsm, short cld, short filter);
int sceGsSetDefAlphaEnv(sceGsAlphaEnv *ap, short pabe);
int sceGsSetDefClear(sceGsClear *cp, short ztest, short x, short y, short w, short h, u_char r, u_char g, u_char b, u_char a, u_int z);
int sceGsSetDefLoadImage(sceGsLoadImage *lp, short dbp, short dbw, short dpsm, short x, short y, short w, short h);
int sceGsSetDefStoreImage(sceGsStoreImage *sp, short sbp, short sbw, short spsm, short x, short y, short w, short h);
int sceGsExecLoadImage(sceGsLoadImage *lp, u_long128 *srcaddr);
int sceGsExecStoreImage(sceGsStoreImage *sp, u_long128 *dstaddr);
int *sceGsSyncVCallback(int (*func)(int));
u_long sceGsPutIMR(u_long imr);
u_long sceGsGetIMR(void);
u_long isceGsPutIMR(u_long imr);
u_long isceGsGetIMR(void);
void sceGsSetHalfOffset(sceGsDrawEnv1 *draw, short centerx, short centery, short halfoff);
int sceGsSetDefDrawEnv2(sceGsDrawEnv2 *draw, short psm, short w, short h, short ztest, short zpsm);
int sceGsSetDefClear2(sceGsClear *cp, short ztest, short x, short y, short w, short h, u_char r, u_char g, u_char b, u_char a, u_int z);
int sceGsSetDefTexEnv2(sceGsTexEnv2 *tp, short flush, short tbp0, short tbw, short psm, short w, short h, short tfx, short cbp, short cpsm, short cld, short filter);
int sceGsSetDefAlphaEnv2(sceGsAlphaEnv2 *ap, short pabe);
void sceGsSetHalfOffset2(sceGsDrawEnv2 *draw, short centerx, short cyntery, short halfoff);
void sceGsSetDefDBuffDc(sceGsDBuffDc *db, short psm, short w, short h, short ztest, short zpsm, short clear);
int sceGsSwapDBuffDc(sceGsDBuffDc *db, int id);

    extern void *sceGsGetErxEntries(void);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _LIBGRAPH_H */
