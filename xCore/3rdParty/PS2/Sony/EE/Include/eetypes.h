/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 0.40
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                            eetypes.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.1.0
 *       0.3.0          1999/07/05      horikawa    required of M.W.
 *       0.4.0          2003/08/22      hana        #include <sys/types.h>
 */

#ifndef _eetypes_h_
#define _eetypes_h_

#include <sys/types.h>

#ifdef  __GNUC__
typedef int long128 __attribute__ ((mode (TI)));
typedef unsigned int u_long128 __attribute__ ((mode (TI)));
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#ifndef NULL
#define NULL    0
#endif

#endif /* _eetypes_h_ */

