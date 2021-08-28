//-----------------------------------------------------------------------------
// File: compress.c
// Author: Mark Breugelmans ( Sony Computer Entertainment Europe )
// Date: 19 January 2001
// Description: Compresses texture into 2 passes
//-----------------------------------------------------------------------------


#include <libgraph.h>
#include "tim.h"
#include "compress.h"

u_int   BilinearExpand          ( u_int colours[4], float x, float y );
void    Write2bitColourIndex    ( u_char *data, u_char index, int x, int y, int width );
void    Write4bitColourIndex    ( u_char *data, u_char index, int x, int y, int width );
void    Write8bitColourIndex    ( u_char *data, u_char index, int x, int y, int width );
void    Write24bitColour        ( u_char *data, u_int newColour );
u_int   Read24bitColour         ( u_char *data, int x, int y, int width, int height );
int     Bilinear                ( int num[4], float x, float y );
int     BilinearColour          ( u_char colour[4][4], float x, float y );
void    CreateAlphaMultiplyClut ( u_int *clutData, int twoBitCLUT );
int     ChooseNearestClutEntry  ( u_int value, u_int *clutData, int numEntries );
u_int   Blend16Pixels           ( u_int colours[16], int *totalMax, int *totalMin );
u_int   Blend4Pixels            ( u_int colours[4], int *totalMax, int *totalMin );

//=========================================================================

void CreateLowResColourMap(TIMInfo *inputTIM, TIMInfo *outputTIM)
{
    u_int   colours[16];
    u_int   newColour;
    int     sourceWidth, sourceHeight;
    int     destWidth, destHeight;
    int     x, y;
    int     i, j;
    int     maxLuminance;
    int     totalMax = 0;
    int     totalMin = 255;

    u_char* image    = (u_char*)inputTIM->textureData;
    u_char* newImage = (u_char*)outputTIM->textureData;

    sourceWidth     = inputTIM->timW;
    sourceHeight    = inputTIM->timH;
    destWidth       = outputTIM->timW;
    destHeight      = outputTIM->timH;

    for (x=0; x<destWidth; x++)
    {
        for (y=0; y<destHeight; y++)
        {
            for (j=0; j<4; j++)
            {
                for (i=0; i<4; i++)
                {
                    colours[i+j*4] = Read24bitColour( &image[0], x*4+i, y*4+j, sourceWidth, sourceHeight );
                }
            }

            newColour = Blend16Pixels( colours, &totalMax, &totalMin );

            Write24bitColour(&newImage[3*(x+(y*destWidth))], newColour);
        }
    }

  //printf("Max luminance = %d   Min luminance = %d\n", totalMax, totalMin);  
}

//=========================================================================

void CreateLuminanceMultiplyMap( 
    TIMInfo* colourMapTIM, 
    TIMInfo* originalTIM,
    TIMInfo* outputTIM, 
    int      twoBitCLUT )
{
    u_int   colours[16];
    u_int   newColour, colourMapColour, originalColour;
    int     x, y;
    int     i, j;
    int     maxLuminance;
    int     numClutEntries;
    int     colourMapLuminance;
    int     originalLuminance;
    int     totalMax=0;
    int     totalMin=255;
    int     clutValue;
    int     countError = 0;
    int     colourWidth, colourHeight, originalWidth, originalHeight;
    int     alphaMultiply;

    u_char* originalImage  = (u_char*)originalTIM->textureData;
    u_char* colourMapImage = (u_char*)colourMapTIM->textureData; 
    u_char* outputImage    = (u_char*)outputTIM->textureData;

    originalWidth  = originalTIM->timW;
    originalHeight = originalTIM->timH;
    colourWidth    = colourMapTIM->timW;
    colourHeight   = colourMapTIM->timH;

    if (twoBitCLUT) numClutEntries = 4;
    else            numClutEntries = 16;

    CreateAlphaMultiplyClut( outputTIM->clutData32, twoBitCLUT );

    for (x=0; x<originalWidth; x++)
    {
        for (y=0; y<originalHeight; y++)
        {
            // Read original colour
            originalColour = Read24bitColour( &originalImage[0], x, y, originalWidth, originalHeight );

            // Get original luminance ( Biggest of the 3 compnets r,g,b)
            originalLuminance = originalColour&0xff;

            if( ((originalColour>>8)&0xff) > originalLuminance )  
                originalLuminance = (originalColour>>8)&0xff;

            if( ((originalColour>>16)&0xff) > originalLuminance ) 
                originalLuminance = (originalColour>>16)&0xff;

            // Read colour map (since the luminace map falls inside this set)
            // 0 1
            // 2 3	  
            for (j=0; j<2; j++)
            {
                for (i=0; i<2; i++)
                {
                    colours[ i + j*2 ] = Read24bitColour( &colourMapImage[0],
			                                              ((x+2)>>2)+(i-1), 
                                                          ((y+2)>>2)+(j-1),
			                                              colourWidth, 
                                                          colourHeight );
                }
            }

            // Create bilinear expanded colour (tha actual color were the luminance has to apoarch)
            colourMapColour = BilinearExpand( colours, ((x-0)%4), ((y-0)%4) );

            // Get colour map luminance (base on the bilinear color)
            colourMapLuminance = (colourMapColour>>0) &0xff;

            if (((colourMapColour>>8)&0xff)>colourMapLuminance)  
                colourMapLuminance = (colourMapColour>>8) &0xff;

            if (((colourMapColour>>16)&0xff)>colourMapLuminance) 
                colourMapLuminance = (colourMapColour>>16) &0xff;	  

            // Don't want a Divide by zero bug here
            if( colourMapLuminance == 0 ) colourMapLuminance = 1;

            // Calculate multiply needed. 
            alphaMultiply = (128*originalLuminance) / colourMapLuminance;

            if( alphaMultiply < 0   ) printf("Alpha multiply underflow\n");
            if( alphaMultiply > 255 ) alphaMultiply = 255;

            // Map to 4bit alphaMultiply palette
            newColour = ChooseNearestClutEntry( alphaMultiply, outputTIM->clutData32, numClutEntries );

            if (twoBitCLUT)
                Write2bitColourIndex(outputImage, newColour, x, y, originalWidth);
            else 
                Write4bitColourIndex(outputImage, newColour, x, y, originalWidth);
        }
    }
}

//=========================================================================

int ChooseNearestClutEntry(u_int value, u_int *clutData, int numEntries)
{
    int     i=0;
    int     difference1, difference2;

    while (i<(numEntries-1))
    {
        if( value <= ((clutData[i+1]>>24)&0xff) ) 
        {
            difference1 = abs(((clutData[i]>>24)&0xff) - value);
            difference2 = abs(((clutData[i+1]>>24)&0xff) - value);

            if( difference1<difference2 )
                return i;
            else
                return i+1;
	    }

        i++;
    }

    return numEntries-1;
}

void FadeClut(u_int *clutData, int numEntries, float scale)
{
  int i;
  int value;

  scale = scale*scale*scale;
  for (i=0; i<numEntries; i++)
    {
      value = (clutData[i]&0xff)-128;
      value = (int) (((float)value)*scale);
      value += 128;

      if (value>255) value=255;
      if (value<0)   value=0;
      clutData[i] = value + (value<<8) + (value<<16) + (value<<24);
    }
}

//=========================================================================

void CreateAlphaMultiplyClut( u_int* clutData, int twoBitCLUT )
{
    int     x;
    u_int   clutValue;
    int     numEntries;

    int palette4bit[] = { -128, -84, -52, -32, -20, -12, -8, -4, 
		                   0, 5, 11, 17, 29, 47, 76, 123 };

    int palette2bit[] = { -42,  -8,   8,  42, -42, -8,  8, 42, // csa 0 (lower 2 bits)
	                      -42,  -8,   8,  42, -42, -8,  8, 42, // csa 0 (lower 2 bits)
	                      -42, -42, -42, -42,  -8, -8, -8, -8,
	                        8,   8,   8,   8,  42, 42, 42, 42, };

    if( twoBitCLUT ) numEntries = 32; // 2 4bit cluts
    else             numEntries = 16;

    for( x=0; x<numEntries; x++)
    {
        if( numEntries == 16 )  clutValue = (palette4bit[x]+128)&0xff;
        else 	                clutValue = (palette2bit[x]+128)&0xff;

        clutData[x] = clutValue + (clutValue<<8) + (clutValue<<16) + (clutValue<<24);
    }
}

//=========================================================================
// Bilinear expand 1:4
u_int BilinearExpand( u_int colours[4], float x, float y )
{
    u_int   resultColour;
    int     red[4];
    int     green[4];
    int     blue[4];
    int     alpha[4];
    int     i;

    // 0 1
    // 2 3
    x += 2.5f; y+= 2.5f;

    if( x>=4.0f ) x-= 4.0f;
    if( y>=4.0f ) y-= 4.0f;

    if( x<0.0f ) x+=4.0f;
    if( y<0.0f ) y+=4.0f;

  //printf("x=%f, y=%f\n", x, y);

    // This is for debug
    for( i=0; i<4; i++ )
    {
        // Read colour components out of array
        red[i]   = (colours[i] >> 0 ) & 0xff;
        green[i] = (colours[i] >> 8 ) & 0xff;
        blue[i]  = (colours[i] >> 16) & 0xff;
        alpha[i] = (colours[i] >> 24) & 0xff;
    }

    resultColour = BilinearColour( colours, x, y );

    return resultColour;
}

//=========================================================================

int BilinearColour( u_char colour[4][4], float x, float y )
{
    // Could use GS 4bit precission but float will do for now
    float   alpha, beta, oneMinusAlpha, oneMinusBeta;
    float   result[4];
    u_int   resultColour;
    int     i;

    // alpha, beta are (0.0f to 1.0f)
    alpha = ((float)x) * 0.25f;
    beta  = ((float)y) * 0.25f;

    oneMinusAlpha = 1.0f - alpha;
    oneMinusBeta  = 1.0f - beta;

    for (i=0; i<3; i++)
    {
        result[i] = 
                    oneMinusAlpha * oneMinusBeta * ((float)colour[0][i]) +
                    alpha         * oneMinusBeta * ((float)colour[1][i]) + 
                    oneMinusAlpha * beta         * ((float)colour[2][i]) +
                    alpha         * beta         * ((float)colour[3][i]);
    }

    resultColour = (int)result[0] + ((int)(result[1])<<8) + ((int)(result[2])<<16);
                    // + ( (int)(result[3])>>24); // alpha?

    return ((u_int)resultColour);
}

int Bilinear(int num[4], float x, float y)
{
  // Could use GS 4bit precission but float will do for now
  float result;
  float alpha, beta, oneMinusAlpha, oneMinusBeta;

  // alpha, beta are (0.0f to 1.0f)
  alpha = ((float)x) * 0.25f;
  beta  = ((float)y) * 0.25f;

  oneMinusAlpha = 1.0f - alpha;
  oneMinusBeta  = 1.0f - beta;

  result = 
    oneMinusAlpha * oneMinusBeta * ((float)num[0]) +
    alpha         * oneMinusBeta * ((float)num[1]) + 
    oneMinusAlpha * beta         * ((float)num[2]) +
    alpha         * beta         * ((float)num[3]);

  return ((int)result);
}

void Write2bitColourIndex(u_char *data, u_char index, int x, int y, int width)
{
  // Same as 4bit, but if > half width write bottom bits
  int byteValue;

  // Read byte
  byteValue = data[((x%(width>>1))>>1) + y*(width>>2)];

  // Mask input
  index = index&0x03;

  // Add input to masked value
  if ((x%2)==1)
    {
      if ((x<<1)>=width) // Upper 2bits
	byteValue = (byteValue&0x3f) + (index<<6); 
      else              // Lower 2bits
	byteValue = (byteValue&0xcf) + (index<<4);
    }
  else
    {
      if ((x<<1)>=width) // Upper 2bits
	byteValue = (byteValue&0xf3) + (index<<2); 
      else              // Lower 2bits
	byteValue = (byteValue&0xfc) + (index<<0);
    }

    data[((x%(width>>1))>>1) + y*(width>>2)] = byteValue;
}

void Write4bitColourIndex(u_char *data, u_char index, int x, int y, int width)
{
  if ((x%2)==1)
    {
      data[(x>>1) + y*(width>>1)] 
	= (data[(x>>1) + y*(width>>1)]&0x0f) + ((index&0x0f)<<4);
    }
  else
    {
      data[(x>>1) + y*(width>>1)] 
	= (data[(x>>1) + y*(width>>1)]&0xf0) + ((index&0x0f)<<0);
    }
}

void Write8bitColourIndex(u_char *data, u_char index, int x, int y, int width)
{
  // Swap bits 3 and 4
  data[x + y*width] = (index&0xe7) | ((index&0x08)<<1) | ((index&0x10)>>1);
}

void Write24bitColour(u_char *data, u_int newColour)
{
  data[0] = (newColour>>0) & 0xff; // red
  data[1] = (newColour>>8) & 0xff; // green
  data[2] = (newColour>>16) & 0xff; // blue
}

//=========================================================================

u_int Read24bitColour(u_char *data, int x, int y, int width, int height)
{
    u_char* ptr;
    u_int   colour;

    if( x < 0 )       x=0;
    if( y < 0 )       y=0;
    if( x >= width  ) x = width-1;
    if( y >= height ) y = height-1;

    ptr = &data[ 3*(x + y*width) ];

    colour = (ptr[0]<<0) +  // red
             (ptr[1]<<8) +  // green
             (ptr[2]<<16);  // blue

    return colour;
}

//=========================================================================

u_int Blend16Pixels(u_int colours[16], int *totalMax, int *totalMin)
{
    u_int   resultColour;
    float   sumRed, sumGreen, sumBlue, sumAlpha;
    int     intRed, intGreen, intBlue, intAlpha;
    int     x, y;
    int     i;
    float   red, green, blue, alpha;
    int     maxLuminance;
    float   luminanceAdjust;
    int     luminance = 0;

  /*
  float blendTable[16] = { 0.4f, 0.5f, 0.5f, 0.4f,
			   0.5f, 2.0f, 2.0f, 0.5f,
 			   0.5f, 2.0f, 2.0f, 0.5f,
	 		   0.4f, 0.5f, 0.5f, 0.4f };
  */
    
    float blendTable[16] = { 1.0f, 1.0f, 1.0f, 1.0f, 
  		                     1.0f, 1.0f, 1.0f, 1.0f,
 			                 1.0f, 1.0f, 1.0f, 1.0f,
 			                 1.0f, 1.0f, 1.0f, 1.0f };
  
    sumRed       = 0.0f; 
    sumGreen     = 0.0f; 
    sumBlue      = 0.0f; 
    sumAlpha     = 0.0f;
    maxLuminance = (colours[0] >> 0 ) & 0xff;

    for( y=0; y<4; y++ )
    {
        for( x=0; x<4; x++ )
        {
            // Read colour components out of array
            red   = (colours[x+y*4] >> 0 ) & 0xff;
            green = (colours[x+y*4] >> 8 ) & 0xff;
            blue  = (colours[x+y*4] >> 16) & 0xff;
            alpha = (colours[x+y*4] >> 24) & 0xff;

            // Get max, min luminance
            if ( red   > maxLuminance) maxLuminance = red;
            if ( green > maxLuminance) maxLuminance = green;
            if ( blue  > maxLuminance) maxLuminance = blue;

            // Add up colours
            sumRed   += red   * blendTable[x+y*4];
            sumGreen += green * blendTable[x+y*4];
            sumBlue  += blue  * blendTable[x+y*4];
            sumAlpha += alpha * blendTable[x+y*4];
        }
    }
  
    intRed      = (int)sumRed;
    intGreen    = (int)sumGreen;
    intBlue     = (int)sumBlue;
    intAlpha    = (int)sumAlpha;

    intRed   >>= 4; // Divide by 16
    intGreen >>= 4;
    intBlue  >>= 4;
    intAlpha >>= 4;
  
    // Get luminance of added up colours
    luminance = intRed;
    if( intGreen > luminance ) luminance = intGreen;
    if( intBlue  > luminance ) luminance = intBlue;

    /*
    // Check final colours (just in case)
    if (sumRed   > 255) printf("Warning: Colour overflow\n");
    if (sumGreen > 255) printf("Warning: Colour overflow\n");
    if (sumBlue  > 255) printf("Warning: Colour overflow\n");
    if (sumAlpha > 255) printf("Warning: Colour overflow\n");


    if (luminance>(*totalMax)) *totalMax = luminance;
    */

    resultColour = intRed + (intGreen<<8) + (intBlue<<16);

    return resultColour;
}


// Blend 4 pixels (25% each) and adjust luminance
u_int Blend4Pixels(u_int colours[4], int *totalMax, int *totalMin)
{
  u_int resultColour;
  int sumRed, sumGreen, sumBlue, sumAlpha;
  int luminance;

  int i, red, green, blue, alpha;
  int maxLuminance;
  float luminanceAdjust;

  sumRed = 0; sumGreen =0; sumBlue = 0; sumAlpha = 0;
  luminance = 0;

  maxLuminance = (colours[0] >> 0 ) & 0xff;
  for (i=0; i<4; i++)
    {
      // Read colour components out of array
      red   = (colours[i] >> 0 ) & 0xff;
      green = (colours[i] >> 8 ) & 0xff;
      blue  = (colours[i] >> 16) & 0xff;
      alpha = (colours[i] >> 24) & 0xff;
      
      // Get max, min luminance
      if ( red   > maxLuminance) maxLuminance = red;
      if ( green > maxLuminance) maxLuminance = green;
      if ( blue  > maxLuminance) maxLuminance = blue;

      // Add up colours
      sumRed   += red;
      sumGreen += green;
      sumBlue  += blue;
      sumAlpha += alpha;
    }

  sumRed   >>= 2;
  sumGreen >>= 2;
  sumBlue  >>= 2;
  sumAlpha >>= 2;

  // Get luminance of added up colours
  luminance = sumRed;
  if (sumGreen > luminance) luminance = sumGreen;
  if (sumBlue  > luminance) luminance = sumBlue;

  // Perhaps a check for saturated luminance?

  // Check maxLuminance < (luminance*2) as alpha multiply is only upto 2*
  if(  ((luminance<<1) < maxLuminance) )
    {
      // Increase luminance to 1/2 max luminence
      luminanceAdjust = (((float)maxLuminance)*0.5f) /  ((float)luminance );
      
      // Multiply through colour to match new luminance

      luminance = (int)( ((float)luminance) * luminanceAdjust);
      sumRed   = (int)( ((float)sumRed)   * luminanceAdjust );
      sumGreen = (int)( ((float)sumGreen) * luminanceAdjust );
      sumBlue  = (int)( ((float)sumBlue)  * luminanceAdjust );
    }
  

  /*
  // Check final colours (just in case)
  if (sumRed   > 255) printf("Warning: Colour overflow\n");
  if (sumGreen > 255) printf("Warning: Colour overflow\n");
  if (sumBlue  > 255) printf("Warning: Colour overflow\n");
  if (sumAlpha > 255) printf("Warning: Colour overflow\n");
  if (sumRed   < 0  ) printf("Warning: Colour overflow\n");
  if (sumGreen < 0  ) printf("Warning: Colour overflow\n");
  if (sumBlue  < 0  ) printf("Warning: Colour overflow\n");
  if (sumAlpha < 0  ) printf("Warning: Colour overflow\n");
  

  if (luminance>(*totalMax)) *totalMax = luminance;
  if (luminance<(*totalMin)) *totalMin = luminance;
  */


  /*    
  // 16bit colour simulation
  sumRed = sumRed & 0xf8;
  sumGreen = sumGreen & 0xf8;
  sumBlue = sumBlue & 0xf8;
  */
  resultColour = sumRed + (sumGreen<<8) + (sumBlue<<16);
  return resultColour;
}







