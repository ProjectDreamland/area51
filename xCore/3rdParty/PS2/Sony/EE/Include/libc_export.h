/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: libc_export.h,v 1.4 2003/04/15 16:20:33 shibata Exp $
 */
/* 
 *                          libc_export
 *                          Version 1.00
 *                           Shift-JIS
 *
 *         Copyright (C) 2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                   libc_export - libc_export.h
 *                   header file of libc_export
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2003/03/26      shibata     First version
 *
 */
#ifndef _LIBC_EXPORT_H_
#define _LIBC_EXPORT_H_

#include <liberx.h>
#include <stdio.h>

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


void * sceLibcAssertGetErxEntries (void);

void * sceLibcCtypeGetErxEntries (void);
const char * __sceErxGetCtype (void);

void * sceLibcErrnoGetErxEntries (void);

void * sceLibcMallocGetErxEntries (void);

void * sceLibcSetjmpGetErxEntries (void);

void * sceLibcStdioGetErxEntries (void);
FILE * __sceErxGetStdin (void); 
FILE * __sceErxGetStdout (void); 
FILE * __sceErxGetStderr (void); 

void * sceLibcStdlibGetErxEntries (void);

void * sceLibcStringGetErxEntries (void);

void * sceLibcTimeGetErxEntries (void);

void * sceLibcUnistdGetErxEntries (void);

void * sceLibcCtypeExtGetErxEntries (void);

void * sceLibcStdioExtGetErxEntries (void);

void * sceLibcStdlibExtGetErxEntries (void);

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _LIBC_EXPORT_H_ */
