//===========================================================================
// File: framebuf.c
// Date: 10 May 2000
// Author: Mark Breugelmans (Sony Computer Entertainment Europe)
// Description: Simple functions to setup display/draw environments
//              and to draw filtered sprites
//===========================================================================

#include "libgraph.h"
#include "ps2_framebuf.hpp"
#include "x_plus.hpp"
#include "e_engine.hpp"


// Note: coord system will always be 2048,2048 centre (use this for scissor)
// Note: don't forget to set the half offset when swapping buffers

/*

Need to set the following registers (USE struct sceGsDrawEnv1):

  FRAME_1     - Frame buffer (drawing buffer)
  ZBUF_1      - Z buffer address
  XY_OFFSET   - Half pixel offset
  SCISSOR     - Scissor region
  PRMODECONT  - Use / don't use prmode register
  COLCLAMP    - Clamp or mask 8bit colour values
  DTHE        - Dither on / off
  TEST_1      - Z test / Alpha test modes


Need to clear buffers( USE struct sceGSClear)

  TEST        - Set draw mode
  PRIM        - Set prim type(sprite)
  RGBAQ       - Set clear colour
  XYZ2        - Top left pos
  XYZ2        - Bottom right pos
  TEST        - Reset draw mode

Also need to set the following priviliged registers( USE struct sceGsDispEnv)

  PMODE       - Choose read circuits
  SMODE2      - Interlace, field/frame mode
  DISPFB1     - Sets display buffer settings (format, addr, x, y)
  DISPLAY1    - Sets display buffer position on TV (x, y, width, height)
  BGCOLOR     - Sets background colour of PCRTC

And need to reset/sync the GS (USE sceGsResetGraph and sceGsSyncV functions)
  CSR         - Resets the GS / syncV


*/
//=========================================================================

void CalcFrameBufferPositions(fsAABuff *buff, short dispW, short dispH, short dispPSM)
{
    dispW = 0;
    dispH = 0;
    dispPSM = 0;
  //int textureAddr   = 490;

  buff->dispFBP0    = 0;   // Max disp  size 140 pages (640x448x32bit)
  buff->dispFBP1    = 128;//140;
  buff->zFBP        = 256;//280; // Z-buf size 140 pages
}

//=========================================================================

void SetupFS_AA_buffer(fsAABuff *buff, short dispW, short dispH, short dispPSM,
               short ztest, short zPSM,
               short clear)
{

    x_memset( buff, 0, sizeof(fsAABuff) );
  buff->dispW   = dispW;
  buff->dispH   = dispH;
  buff->dispPSM = dispPSM;
  buff->zPSM    = zPSM;

  CalcFrameBufferPositions( buff, dispW, dispH, dispPSM );

  SetDispBuffers( buff, dispW, dispH, dispPSM, buff->dispFBP0, buff->dispFBP1);

  SetDrawBuffersSmall( buff, dispW, dispH, dispPSM, buff->dispFBP0, buff->dispFBP1,
              buff->zFBP, ztest, zPSM, clear);
}

//=========================================================================

void SetDrawBuffersSmall(fsAABuff *buff, short w, short h, short psm, short fbp0, short fbp1,
             short zbp, short ztest, short zpsm, short clear) 
{
    clear = 0;
  // --- Buffer0 ----------------------------------------------------------------------------

  sceGsSetDefDrawEnv(&buff->drawSmall0, psm, w, h, ztest, zpsm); // Last 2 param = ztest, zpsm
  sceGsSetDefDrawEnv2(&buff->drawSmall0_1, psm, w, h, ztest, zpsm); // Last 2 param = ztest, zpsm

  // Fix fbp value for the draw buffer
  buff->drawSmall0.frame1.FBP = fbp0;
  buff->drawSmall0_1.frame2.FBP = fbp0;

  if (ztest == 0)
  {
    *(u_long *)&buff->drawSmall0.zbuf1 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 1); 
    *(u_long *)&buff->drawSmall0_1.zbuf2 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 1); 
  }
  else
  {
    *(u_long *)&buff->drawSmall0.zbuf1 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 0); 
    *(u_long *)&buff->drawSmall0_1.zbuf2 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 0); 
  }


  // Setup the giftag
  SCE_GIF_CLEAR_TAG(&buff->giftagDrawSmall0);
  buff->giftagDrawSmall0.NLOOP = 6+8+8; 
  buff->giftagDrawSmall0.EOP   = 1;
  buff->giftagDrawSmall0.NREG  = 1;
  buff->giftagDrawSmall0.REGS0 = 0xe; // A_plus_D

  sceGsSetDefClear(&buff->clearSmall0, ztest, 2048-(w>>1), 2048-(h>>1), w, h, 32,32,32, 0, 0);   

  // --- Buffer1 ----------------------------------------------------------------------------

  sceGsSetDefDrawEnv(&buff->drawSmall1, psm, w, h, ztest, zpsm); // Last 2 param = ztest, zpsm
  sceGsSetDefDrawEnv2(&buff->drawSmall1_1, psm, w, h, ztest, zpsm); // Last 2 param = ztest, zpsm

  // Fix fbp value for the draw buffer
  buff->drawSmall1.frame1.FBP = fbp1;
  buff->drawSmall1_1.frame2.FBP = fbp1;

  if (ztest == 0)
  {
    *(u_long *)&buff->drawSmall1.zbuf1 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 1); 
    *(u_long *)&buff->drawSmall1_1.zbuf2 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 1); 
  }
  else
  {
    *(u_long *)&buff->drawSmall1.zbuf1 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 0); 
    *(u_long *)&buff->drawSmall1_1.zbuf2 = SCE_GS_SET_ZBUF(zbp, zpsm&0xf, 0); 
  }


  // Setup the giftag
  SCE_GIF_CLEAR_TAG(&buff->giftagDrawSmall1);
  buff->giftagDrawSmall1.NLOOP = 6+8+8; // Add 6 registers for clearing
  buff->giftagDrawSmall1.EOP   = 1;
  buff->giftagDrawSmall1.NREG  = 1;
  buff->giftagDrawSmall1.REGS0 = 0xe; // A_plus_D

  sceGsSetDefClear(&buff->clearSmall1, ztest, 2048-(w>>1), 2048-(h>>1), w, h, 32,32,32, 0, 0);   
}

//=========================================================================

void PutDrawBufferSmall(fsAABuff *buff, int bufferNum, int clear)
{
    if (clear)
    {
        buff->giftagDrawSmall0.NLOOP = 8+8+6; // Add 6 registers for clearing
        buff->giftagDrawSmall1.NLOOP = 8+8+6; // Add 6 registers for clearing

        if( clear == 2 )
        {
            // JP: run this code to disable writes to the RGBA buffer (Z-Buffer is still written to)
        
            buff->clearSmall0.testa.ATE   = 1;
            buff->clearSmall0.testa.ATST  = 0;
            buff->clearSmall0.testa.AFAIL = 2;
            buff->clearSmall0.testb.ATE   = 0;
            buff->clearSmall0.testb.ATST  = 0;
            buff->clearSmall0.testb.AFAIL = 0;
            
            buff->clearSmall1.testa.ATE   = 1;
            buff->clearSmall1.testa.ATST  = 0;
            buff->clearSmall1.testa.AFAIL = 2;
            buff->clearSmall1.testb.ATE   = 0;
            buff->clearSmall1.testb.ATST  = 0;
            buff->clearSmall1.testb.AFAIL = 0;
        }
    }
    else
    {
        buff->giftagDrawSmall0.NLOOP = 8+8;
        buff->giftagDrawSmall1.NLOOP = 8+8;
    }

    if (bufferNum==0)
        sceGsPutDrawEnv(&buff->giftagDrawSmall0);
    else
        sceGsPutDrawEnv(&buff->giftagDrawSmall1);
}

//=========================================================================

void SetDispBuffers(fsAABuff *buff, short w, short h, short psm, short fbp0, short fbp1)
{

  // Buffer 0 ==================================================
  // No giftags needed (all memory mapped registers to set)
  sceGsSetDefDispEnv((sceGsDispEnv*)(&buff->disp0), psm, w, h, 0, 0); // dx,dy = 0
  
  buff->disp0.dispfb.FBP            = fbp0; 

  // 2nd circuit
  *((u_long*)&buff->disp0.dispfb1)  = *(u_long*)&buff->disp0.dispfb;
  *((u_long*)&buff->disp0.display1) = *(u_long*)&buff->disp0.display;  
  buff->disp0.dispfb.DBY            = 1;                            // dispfb2 has DBY field of 1
  buff->disp0.display.DH            = (buff->disp0.display1.DH)-1;
  buff->disp0.display.DX            = (buff->disp0.display.DX)+4;    // display2 has magh/2
  buff->disp0.pmode.ALP             = 128;
  buff->disp0.pmode.EN1             = 1;
  buff->disp0.pmode.EN2             = 1;


  // Buffer 1 ==================================================
  sceGsSetDefDispEnv((sceGsDispEnv*)(&buff->disp1), psm, w, h, 0, 0); // dx,dy = 0
  
  buff->disp1.dispfb.FBP            = fbp1; 

  // 2nd circuit
  *((u_long*)&buff->disp1.dispfb1)  = *(u_long*)&buff->disp1.dispfb;
  *((u_long*)&buff->disp1.display1) = *(u_long*)&buff->disp1.display;
  buff->disp1.dispfb.DBY            = 1; // dispfb2 has DBY field of 1
  buff->disp1.display.DH            = (buff->disp1.display1.DH)-1;
  buff->disp1.display.DX            = (buff->disp1.display.DX)+4; // display2 has magh/2  
  buff->disp1.pmode.ALP             = 128;
  buff->disp1.pmode.EN1             = 1;
  buff->disp1.pmode.EN2             = 1;

}

//=========================================================================

void PutDispBuffer(fsAABuff *buff, int bufferNum, int both)
{
    if (bufferNum==0)
    {
        if (both)
        {
            buff->disp0.pmode.EN1 = 1;
            buff->disp0.pmode.EN2 = 1;
        }
        else
        {
            buff->disp0.pmode.EN1 = 0;
            buff->disp0.pmode.EN2 = 1;
        }

        DPUT_GS_PMODE(*(u_long *)&buff->disp0.pmode);   //PMODE
        DPUT_GS_SMODE2(*(u_long *)&buff->disp0.smode2);  //SMODE2
        DPUT_GS_DISPFB2(*(u_long *)&buff->disp0.dispfb);  //DISPFB2
        DPUT_GS_DISPLAY2(*(u_long *)&buff->disp0.display); //DISPLAY2
        DPUT_GS_BGCOLOR(*(u_long *)&buff->disp0.bgcolor); //BGCOLOR
        DPUT_GS_DISPLAY1( *(u_long *)&buff->disp0.display1); //DISPLAY1
        DPUT_GS_DISPFB1(*(u_long *)&buff->disp0.dispfb1);  //DISPFB1
    }
    else
    {
        if (both)
        {
            buff->disp1.pmode.EN1 = 1;
            buff->disp1.pmode.EN2 = 1;
        }
        else
        {
            buff->disp1.pmode.EN1 = 0;
            buff->disp1.pmode.EN2 = 1;
        }
      
        DPUT_GS_PMODE(*(u_long *)&buff->disp1.pmode);   //PMODE
        DPUT_GS_SMODE2(*(u_long *)&buff->disp1.smode2);  //SMODE2
        DPUT_GS_DISPFB2(*(u_long *)&buff->disp1.dispfb);  //DISPFB2
        DPUT_GS_DISPLAY2(*(u_long *)&buff->disp1.display); //DISPLAY2
        DPUT_GS_BGCOLOR(*(u_long *)&buff->disp1.bgcolor); //BGCOLOR
        DPUT_GS_DISPLAY1( *(u_long *)&buff->disp1.display1); //DISPLAY1
        DPUT_GS_DISPFB1(*(u_long *)&buff->disp1.dispfb1);  //DISPFB1
    }

    asm __volatile__ ( "sync.l" );
    asm __volatile__ ( "sync.p" );
}

//=========================================================================

void FB_SetBackgroundColor( fsAABuff *buff, s32 R, s32 G, s32 B )
{
    buff->clearSmall0.rgbaq.R = (byte)R;
    buff->clearSmall0.rgbaq.G = (byte)G;
    buff->clearSmall0.rgbaq.B = (byte)B;
    buff->clearSmall1.rgbaq.R = (byte)R;
    buff->clearSmall1.rgbaq.G = (byte)G;
    buff->clearSmall1.rgbaq.B = (byte)B;
}

//=========================================================================

u64  FB_GetFRAMEReg(fsAABuff *buff,  int bufferNum )
{
    if (bufferNum==0)
        return *((u64*)(&buff->drawSmall0.frame1));
    else
        return *((u64*)(&buff->drawSmall1.frame1));
}

//=========================================================================

u32  FB_GetFrameBufferAddr(fsAABuff *buff,  int bufferNum )
{
    if (bufferNum==0)
        return buff->drawSmall0.frame1.FBP * 2048 ;
    else
        return buff->drawSmall1.frame1.FBP * 2048 ;
}

//=========================================================================

void FB_GetFrameBufferPositions( s32& FB0, s32& FB1, s32& ZB )
{
    FB0 = 0;
    FB1 = 128;
    ZB  = 256;
}

//=========================================================================




