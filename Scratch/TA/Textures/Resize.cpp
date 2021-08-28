//=========================================================================
//
// Bicubic scale sweet!
//
// http://codeguru.earthweb.com/bitmap/SmoothBitmapResizing.html
//
//=========================================================================
// INCLUDES
//=========================================================================

#include "Resize.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================
static
s32* CreateCoeffInt( s32 nLen, s32 nNewLen, xbool bShrink )
{
    s32     nSum   = 0, nSum2;
    s32*    pRes   = new s32[2 * nLen];
    s32*    pCoeff = pRes;
    s32     nNorm  = (bShrink) ? (nNewLen << 12) / nLen : 0x1000;
    s32	    nDenom = (bShrink) ? nLen : nNewLen;

    x_memset( pRes, 0, 2 * nLen * sizeof(s32) );
 
    for( s32 i = 0; i < nLen; i++, pCoeff += 2 )
    {
        nSum2 = nSum + nNewLen;
        if(nSum2 > nLen)
        {
            *pCoeff = ((nLen - nSum) << 12) / nDenom;
            pCoeff[1] = ((nSum2 - nLen) << 12) / nDenom;
            nSum2 -= nLen;
        }
        else
        {
            *pCoeff = nNorm;
            if(nSum2 == nLen)
            {
                pCoeff[1] = -1;
                nSum2     = 0;
            }
        }

        nSum = nSum2;
    }
 
    return pRes;
}

//=========================================================================
static
void ShrinkDataInt( 
    xcolor*     pInBuff, 
    s32         Width, 
    s32         Height,
    xcolor*     pOutBuff, 
    s32         NewWidth, 
    s32         NewHeight )
{
    s32     x, y, i, ii;
    xbool   bCrossRow, bCrossCol;

    byte*   pLine       = (byte*)pInBuff;
    byte*   pOutLine    = (byte*)pOutBuff;
    byte*   pPix;
    s32     InLn        = 4 * Width;
    s32     OutLn       = 4 * NewWidth;

    s32*    pRowCoeff   = CreateCoeffInt( Width, 
                                          NewWidth, 
                                          TRUE );

    s32*    pColCoeff   = CreateCoeffInt( Height, 
                                          NewHeight, 
                                          TRUE );

    s32*    pXCoeff;
    s32*    pYCoeff     = pColCoeff;

    s32     BuffLn      = 4 * NewWidth * sizeof(s32);
    s32*    pBuff       = new s32[8 * NewWidth];
    s32*    pCurrLn     = pBuff;
    s32*    pCurrPix; 
    s32*    pNextLn     = pBuff + 4 * NewWidth;
    s32     Tmp;
    s32*    pNextPix;

    x_memset( pBuff, 0, 2 * BuffLn );

    y = 0;
    while( y < NewHeight )
    {
        pPix   = pLine;
        pLine += InLn;

        pCurrPix = pCurrLn;
        pNextPix = pNextLn;

        x = 0;
        pXCoeff   = pRowCoeff;
        bCrossRow = pYCoeff[1] > 0;

        while( x < NewWidth )
        {
            Tmp = (*pXCoeff) * (*pYCoeff);

            for(i = 0; i < 4; i++)
                pCurrPix[i] += Tmp * pPix[i];

            bCrossCol = pXCoeff[1] > 0;
            if( bCrossCol )
            {
                Tmp = pXCoeff[1] * (*pYCoeff);
                for(i = 0, ii = 4; i < 4; i++, ii++)
                    pCurrPix[ii] += Tmp * pPix[i];
            }

            if( bCrossRow )
            {
                Tmp = (*pXCoeff) * pYCoeff[1];
                for(i = 0; i < 4; i++)
                    pNextPix[i] += Tmp * pPix[i];

                if( bCrossCol )
                {
                    Tmp = pXCoeff[1] * pYCoeff[1];
                    for(i = 0, ii = 4; i < 4; i++, ii++)
                        pNextPix[ii] += Tmp * pPix[i];
                }
            }

            if(pXCoeff[1])
            {
                x++;
                pCurrPix += 4;
                pNextPix += 4;
            }

            pXCoeff += 2;
            pPix    += 4;
        }

        if(pYCoeff[1])
        {
            // set result line
            pCurrPix = pCurrLn;
            pPix     = pOutLine;

            for( i = 4 * NewWidth; i > 0; i--, pCurrPix++, pPix++)
                *pPix = ((byte*)pCurrPix)[3];

            // prepare line buffers
            pCurrPix = pNextLn;
            pNextLn  = pCurrLn;
            pCurrLn  = pCurrPix;

            x_memset( pNextLn, 0, BuffLn );

            y++;
            pOutLine += OutLn;
        }

        pYCoeff += 2;
    }

    delete [] pRowCoeff;
    delete [] pColCoeff;
    delete [] pBuff;
} 

//=========================================================================
static
void EnlargeDataInt( 
    xcolor*     pInBuff, 
    s32         Width, 
    s32         Height,
    xcolor*     pOutBuff, 
    s32         NewWidth, 
    s32         NewHeight )
{
    byte*   pLine = (byte*)pInBuff;
    byte*   pPix  = pLine;
    byte*   pPixOld; 
    byte*   pUpPix;
    byte*   pUpPixOld;

    byte*   pOutLine = (byte*)pOutBuff;
    byte*   pOutPix;

    s32     InLn  = 4 * Width;
    s32     OutLn = 4 * NewWidth;
    s32     x, y, i;
    xbool   bCrossRow, bCrossCol;

    s32*    pRowCoeff = CreateCoeffInt( NewWidth, 
                                        Width, 
                                        FALSE);

    s32*    pColCoeff = CreateCoeffInt( NewHeight, 
                                        Height, 
                                        FALSE);
    s32*    pXCoeff;
    s32*    pYCoeff = pColCoeff;
    s32     Tmp, PtTmp[4];

    y = 0;
    while( y < Height )
    {
        bCrossRow = pYCoeff[1] > 0;

        x = 0;
        pXCoeff   = pRowCoeff;
        pOutPix   = pOutLine;
        pOutLine += OutLn;
        pUpPix    = pLine;

        if(pYCoeff[1])
        {
            y++;
            pLine += InLn;
            pPix   = pLine;
        }

        while(x < Width)
        {
            bCrossCol = pXCoeff[1] > 0;
            pUpPixOld = pUpPix;
            pPixOld   = pPix;

            if( pXCoeff[1] )
            {
                x++;
                pUpPix += 4;
                pPix   += 4;
            }
   
            Tmp = (*pXCoeff) * (*pYCoeff);
   
            for(i = 0; i < 4; i++)
                PtTmp[i] = Tmp * pUpPixOld[i];
   
            if(bCrossCol)
            {
                Tmp = pXCoeff[1] * (*pYCoeff);

                for(i = 0; i < 4; i++)
                    PtTmp[i] += Tmp * pUpPix[i];
            }

            if(bCrossRow)
            {
                Tmp = (*pXCoeff) * pYCoeff[1];

                for(i = 0; i < 4; i++)
                    PtTmp[i] += Tmp * pPixOld[i];

                if( bCrossCol )
                {
                    Tmp = pXCoeff[1] * pYCoeff[1];

                    for(i = 0; i < 4; i++)
                        PtTmp[i] += Tmp * pPix[i];
                }
            }
   
            for( i = 0; i < 4; i++, pOutPix++ )
                *pOutPix = ((byte*)(PtTmp + i))[3];
   
            pXCoeff += 2;
        }

        pYCoeff += 2;
    }

    delete [] pRowCoeff;
    delete [] pColCoeff;
} 

//=========================================================================

void ResizeBitmap(
    const xbitmap&    SourceBMP, 
    xbitmap&          DestBMP, 
    s32               DestW, 
    s32               DestH )
{
    s32 SrcW = SourceBMP.GetWidth();
    s32 SrcH = SourceBMP.GetHeight();

    // check for valid size. It can't scale and resize at the same time.
    ASSERT( !(SrcW > DestW && SrcH < DestH) );
    ASSERT( !(SrcW < DestW && SrcH > DestH) );

    // Make sure that the source has the right format
    ASSERT( SourceBMP.GetFormat() == xbitmap::FMT_32_ARGB_8888 );

    // Setup the new bitmap
    xcolor* pData = new xcolor[ DestW*DestH ];
    ASSERT( pData );

    DestBMP.Setup( xbitmap::FMT_32_ARGB_8888, 
                   DestW, DestH, 
                   TRUE, 
                   (byte*)pData );


    // Do scaling
    if( SrcW >= DestW && SrcH >= DestH )
    {
        ShrinkDataInt( (xcolor*)SourceBMP.GetPixelData(), 
                       SrcW, 
                       SrcH,
                       pData, 
                       DestW, 
                       DestH );
    }
    else
    {
        EnlargeDataInt( (xcolor*)SourceBMP.GetPixelData(), 
                       SrcW, 
                       SrcH,
                       pData, 
                       DestW, 
                       DestH );
    }
}

