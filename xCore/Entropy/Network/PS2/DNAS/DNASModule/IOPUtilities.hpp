#ifndef IOP_UTILITIES_HPP
#define IOP_UTILITIES_HPP

#include "x_types.hpp"

s32 iop_LoadModule   ( const char* pFilename, const char* pArg=NULL, s32 ArgLength = 0, xbool AllowFail = FALSE  );
void iop_UnloadModule( s32 Id );


#endif