/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Controller Library
 *                          Version 1.2
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libpad - libpad.h
 *                     header file of libpad
 *
 *      Version    Date        Design   Log
 *  --------------------------------------------------------------------
 *      0.00       1999- 7-28  makoto   the first version
 *      1.10       1999-10-15  iwano    remove sce Prefix
 *      1.20       1999-11-22           added scePrefix
 *      1.30       1999-12-14           support cplus
 *      1.31       1999-12-20           cplus native support
 *      2.00       2000-08-15           remove "scePadStateFindCTP2"
 *      2.10       2000-10-18           added scePadStateClosed
 *      2.324      2001-07-25           added scePadInitGun()/EndGun()
 */

#ifndef _LIBPAD_H_
#define _LIBPAD_H_

#ifndef SCE_PADLup
/* keys config */
#define SCE_PADLup     (1<<12)
#define SCE_PADLdown   (1<<14)
#define SCE_PADLleft   (1<<15)
#define SCE_PADLright  (1<<13)
#define SCE_PADRup     (1<< 4)
#define SCE_PADRdown   (1<< 6)
#define SCE_PADRleft   (1<< 7)
#define SCE_PADRright  (1<< 5)
#define SCE_PADi       (1<< 9)
#define SCE_PADj       (1<<10)
#define SCE_PADk       (1<< 8)
#define SCE_PADl       (1<< 3)
#define SCE_PADm       (1<< 1)
#define SCE_PADn       (1<< 2)
#define SCE_PADo       (1<< 0)
#define SCE_PADh       (1<<11)
#define SCE_PADL1      SCE_PADn
#define SCE_PADL2      SCE_PADo
#define SCE_PADR1      SCE_PADl
#define SCE_PADR2      SCE_PADm
#define SCE_PADstart   SCE_PADh
#define SCE_PADselect  SCE_PADk
#endif /* SCE_PADLup */

#define scePadStateDiscon	(0)
#define scePadStateFindPad	(1)
#define scePadStateFindCTP1	(2)
#define scePadStateExecCmd	(5)
#define scePadStateStable	(6)
#define scePadStateError	(7)
#define scePadStateClosed	( 99 )

#define scePadReqStateComplete	(0)
#define scePadReqStateFaild	(1)
#define scePadReqStateFailed	(1)
#define scePadReqStateBusy	(2)

#define InfoModeCurID		(1)
#define InfoModeCurExID		(2)
#define InfoModeCurExOffs	(3)
#define InfoModeIdTable		(4)

#define InfoActFunc		(1)
#define InfoActSub		(2)
#define InfoActSize		(3)
#define InfoActCurr		(4)
#define InfoActSign		(5)

#define scePadDmaBufferMax	(16)
#define scePadError		(0)

#define SCE_PAD_DMA_BUFFER_SIZE		(256)
#define SCE_PAD_BUTTON_BUFFER_SIZE	(32)

typedef volatile void (*scePadDmaCB)(int frame);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
int scePadInit( int mode );
int scePadPortOpen( int port, int slot, void* addr );
int scePadRead( int port, int slot, unsigned char* rdata );
int scePadInfoAct( int port, int slot, int actno, int term );
int scePadInfoComb( int port, int slot, int listno, int offs );
int scePadInfoMode( int port, int slot, int term, int offs );
int scePadSetActDirect( int port, int slot, const unsigned char* data );
int scePadSetActAlign( int port, int slot, const unsigned char* data );
int scePadGetState( int port, int slot );
int scePadGetReqState(int port, int slot);
int scePadSetMainMode( int port, int slot, int offs, int lock );
int scePadInfoPressMode( int port, int slot );
int scePadEnterPressMode( int port, int slot );
int scePadExitPressMode( int port, int slot );
void scePadReqIntToStr(int state, char* str);
void scePadStateIntToStr(int state, char* str);
int scePadEnd(void);
int scePadPortClose( int port, int slot );
int scePadGetSlotMax( int port );
int scePadSetWarningLevel(int level);
int scePadInitGun(int mode);
int scePadEndGun(void);
int* scePadSetDmaCallback(int (*func)(int));
int scePadSetFlushRate(int rate );
void *scePadGetErxEntries(void);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /* _LIBPAD_H_ */
