// MakePS2DebugFont.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "x_files.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"

int main(int argc, char* argv[])
{
    // Load the bitmap
    xbitmap BMP ;
    if (auxbmp_Load(BMP, "beebfont.psd"))
    {
        // Convert to PS2
        BMP.ConvertFormat(xbitmap::FMT_P8_ABGR_8888) ;
    
        // Swizzle
        auxbmp_ConvertToPS2(BMP) ;

        // Dump out file
        BMP.DumpSourceCode("font.txt") ;
    }

	return 0;
}
