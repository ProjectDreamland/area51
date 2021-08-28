//=========================================================================
//
//  BMPUtil.hpp
//
//=========================================================================

#ifndef __BMPUTIL_HPP_INCLUDED__
#define __BMPUTIL_HPP_INCLUDED__

#include "x_files.hpp"

namespace bmp_util
{
    void    ConvertToPalettized     ( xbitmap&          Bitmap,
                                      s32               BPP );
    void    ConvertToPS2            ( xbitmap&          Bitmap,
                                      xbool             ReduceAlpha );
    xbool   CopyIntensityToDiffuse  ( xbitmap&          D,
                                      const xbitmap&    M );
    xbool   CopyIntensityToAlpha    ( xbitmap&          Bitmap );
    void    ProcessDetailMap        ( xbitmap&          Bitmap,
                                      xbool             bPalettize );
    xbool   SetPunchThrough         ( xbitmap&          D,
                                      const xbitmap&    M );
    void    ConvertToColoredMips    ( xbitmap&          Bitmap );
};

#endif // __BMPUTIL_HPP_INCLUDED__