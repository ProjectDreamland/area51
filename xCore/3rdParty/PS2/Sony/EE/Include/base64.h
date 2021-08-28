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
 *                       <libhttp - base64.h>
 *                    <header for base64 library>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Apr,25,2003     komaki      first version
 */

#ifndef _BASE64_H
#define _BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

void *sceBASE64GetErxEntries(void);

/* base64 */
extern int sceBASE64Encoder(unsigned const char *in, unsigned char *out,
		int len);
extern int sceBASE64LineDecoder(unsigned const char *in, unsigned char *out,
		int len);

#ifdef __cplusplus
}
#endif

#endif /* _BASE64_H */
