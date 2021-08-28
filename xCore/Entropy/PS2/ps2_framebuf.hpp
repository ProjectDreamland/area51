//===========================================================================
// File: framebuf.h
// Date: 10 May 2000
// Author: Mark Breugelmans (Sony Computer Entertainment Europe)
// Description: Simple functions to setup display/draw environments
//              and to draw filtered sprites
//===========================================================================

#ifndef PS2_FRAMEBUF_H
#define PS2_FRAMEBUF_H

#include "x_types.hpp"


#include <libgraph.h>


void SwapGSBuffers(int oddOrEven);

typedef struct 
{
    tGS_PMODE       pmode;
    tGS_SMODE2      smode2;
    tGS_DISPFB2     dispfb;
    tGS_DISPLAY2    display;
    tGS_BGCOLOR     bgcolor;
    tGS_DISPFB1     dispfb1;
    tGS_DISPLAY1    display1;
    tGS_DISPLAY1    pad;
} DispEnvTwoCircuits;

typedef struct 
{
  DispEnvTwoCircuits    disp0;        // GS Priviledged registers to set up disp buffer
  DispEnvTwoCircuits    disp1;        // GS Priviledged registers to set up disp buffer

  sceGifTag             giftagDrawSmall0; // Double buffer 0
  sceGsDrawEnv1         drawSmall0;   
  sceGsDrawEnv2         drawSmall0_1;   
  sceGsClear            clearSmall0;

  sceGifTag             giftagDrawSmall1; // Double buffer 1
  sceGsDrawEnv1         drawSmall1;   
  sceGsDrawEnv2         drawSmall1_1;   
  sceGsClear            clearSmall1;

  short drawW;
  short drawH;
  short drawPSM;
  short drawFBP;

  short dispW;
  short dispH;
  short dispPSM;
  short dispFBP0;
  short dispFBP1;

  short zPSM;
  short zFBP;

} fsAABuff __attribute__((aligned(64)));


void SetDispBuffers(fsAABuff *buff, short w, short h, short psm, short fbp0, short fbp1); 

void SetDrawBuffersSmall(fsAABuff *buff, short w, short h, short psm, short fbp0, short fbp1,
             short zbp, short ztest, short zpsm, short clear);

void PutDrawBufferSmall(fsAABuff *buff, int bufferNum, int clear);

void PutDispBuffer(fsAABuff *buff, int bufferNum, int both);

// Note half offset only to be used when copying to small buffer (if at all)
void SetHalfOffset(void);

void SetupFS_AA_buffer(fsAABuff *buff, short dispW, short dispH, short dispPSM,
               short ztest, short zPSM,
               short clear);


void CalcFrameBufferPositions(fsAABuff *buff, short dispW, short dispH, short dispPSM);

void FB_SetBackgroundColor( fsAABuff *buff, s32 R, s32 G, s32 B );

u64  FB_GetFRAMEReg(fsAABuff *buff,  int bufferNum );

u32  FB_GetFrameBufferAddr(fsAABuff *buff,  int bufferNum ) ;

#endif
