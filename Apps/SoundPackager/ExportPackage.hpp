#ifndef __EXPORTPACKAGE_HPP
#define __EXPORTPACKAGE_HPP

#include "PackageTypes.hpp"

void    ExportPackage( void );
u32     GetLipSyncSize( s32 nSamples, s32 SampleRate );
s32     GetBreakPointSize( xarray<aiff_file::breakpoint>& BreakPoints, s32& nBreakPoints );
void    WriteLipSyncData( aiff_file* Aiff, X_FILE* out );
void    WriteBreakPoints( xarray<aiff_file::breakpoint>& BreakPoints, X_FILE*out, xbool bReverseEndian );

extern package_info    s_Package;

#endif
