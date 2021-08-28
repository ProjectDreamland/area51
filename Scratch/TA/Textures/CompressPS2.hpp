//-----------------------------------------------------------------------------
// File: compress.h
// Author: Mark Breugelmans ( Sony Computer Entertainment Europe )
// Date: 19 January 2001
// Description: Compresses texture into 2 passes
//-----------------------------------------------------------------------------

#ifndef COMPRESS_H
#define COMPRESS_H

#include "tim.h"

void    FadeClut                    ( u_int *clutData, int numEntries, float scale );
u_int   Blend4Pixels                ( u_int colours[4], int *totalMax, int *totalMin );
u_int   Read24bitColour             ( u_char *data, int x, int y, int width, int height );
void    Write24bitColour            ( u_char *data, u_int newColour );
void    CreateLowResColourMap       ( TIMInfo* inputTIM, TIMInfo* outputTIM );
void    CreateLuminanceMultiplyMap  ( TIMInfo* colourMapTIM,
				                      TIMInfo* originalTIM,
				                      TIMInfo* outputTIM,
				                      int      twoBitCLUT );
void CreateAlphaMultiplyClut        ( u_int *clutData, int twoBitCLUT );

#endif


