/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       <libhttp - libqp.h>
 *                <header for quoted printable library>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Apr,25,2003     komaki      first version
 */

#ifndef _LIBQP_H
#define _LIBQP_H

#ifdef __cplusplus
extern "C" {
#endif

void *sceQPrintableGetErxEntries(void);

/* quoted-printable */
extern int sceQPrintableEncoder(unsigned const char *in, unsigned char *out,
		int len);
extern int sceQPrintableLineDecoder(unsigned const char *in, unsigned char *out,
		int len);

#ifdef __cplusplus
}
#endif

#endif /* _LIBQP_H */
