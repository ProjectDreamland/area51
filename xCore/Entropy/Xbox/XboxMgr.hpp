#ifndef _XBOXMGR_HPP_
#define _XBOXMGR_HPP_

    #define kPOOL_GENERAL 0
    #define kPOOL_TILED   1
    #define kPOOL_TEMP    2
    #define kPOOL_RECORD  0
    #define kPOOL_PUSH    1

    #include "x_files.hpp"
    #include "QuikHeap.h"
    #include "e_Singleton.hpp"
    #include "xbox\xbox_Private.hpp"

    #ifndef TARGET_XBOX
    #error not xbox target
    #endif

#endif
