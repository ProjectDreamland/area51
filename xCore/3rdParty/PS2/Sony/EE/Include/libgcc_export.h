/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: libgcc_export.h,v 1.3 2003/04/14 10:15:20 shibata Exp $
 */
/* 
 *                          libgcc_export
 *                          Version 1.00
 *                           Shift-JIS
 *
 *         Copyright (C) 2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                   libgcc_export - libgcc_export.h
 *                   header file of libgcc_export
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2003/03/26      shibata     First version
 *
 */
#ifndef _LIBGCC_EXPORT_H_
#define _LIBGCC_EXPORT_H_

#include <liberx.h>

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if (__GNUC__==2) && (__GNUC_MINOR__<96)
#define sceLibgccGetErxEntries	sceLibgcc29GetErxEntries
#elif (__GNUC__==2) && (__GNUC_MINOR__==96)
#define sceLibgccGetErxEntries	sceLibgcc296GetErxEntries
#elif (__GNUC__==3) && (__GNUC_MINOR__==2)
#define sceLibgccGetErxEntries	sceLibgcc32GetErxEntries
#endif

void * sceLibgccGetErxEntries (void);

void * sceLibgccCommonGetErxEntries (void);

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _LIBGCC_EXPORT_H_ */
