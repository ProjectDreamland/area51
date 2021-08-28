/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: libm_export.h,v 1.2 2003/04/11 17:28:22 shibata Exp $
 */
/* 
 *                          libm_export
 *                          Version 1.00
 *                           Shift-JIS
 *
 *         Copyright (C) 2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                   libm_export - libm_export.h
 *                   header file of libm_export
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2003/03/26      shibata     First version
 *
 */
#ifndef _LIBM_EXPORT_H_
#define _LIBM_EXPORT_H_

#include <liberx.h>

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


void * sceLibmFloatGetErxEntries (void);

void * sceLibmDoubleGetErxEntries (void);

void * sceLibmFloatExtGetErxEntries (void);

void * sceLibmDoubleExtGetErxEntries (void);

void * sceLibmErrGetErxEntries (void);

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _LIBM_EXPORT_H_ */
