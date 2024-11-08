 /*
GameSpy GHTTP SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
*/

#ifndef _GHTTPMAIN_H_
#define _GHTTPMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "ghttp.h"
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__mips64) && !defined(_WIN32)
    #include "::nonport.h"
#else
    #include "../nonport.h"
    #include "../stringutil.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
