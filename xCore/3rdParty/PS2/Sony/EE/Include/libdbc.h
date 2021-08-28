/* SCEI CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                 device basic control library
 *                          Version 1.0
 *                           Shift-JIS
 *
 *      Copyright (C) 2000,2001 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libdbc - libdbc.h
 *                     header file of libdbc
 *
 *      Version    Date        Design   Log
 *  --------------------------------------------------------------------
 *      0.00       2000-10- 1  makoto   the first version
 *      1.00       2001- 6-13  iwano    modify for release
 */

#ifndef _LIBDBC_H_
#define _LIBDBC_H_

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int sceDbcInit( void );
int sceDbcEnd( void );
void *sceDbcGetErxEntries(void);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _LIBDBC_H_ */
