/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 2001, 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * sk/sk.h
 *     Standard Kit --- header file
 */

#ifndef _STANDARDKIT_EE_PUBLIC_H_
#define _STANDARDKIT_EE_PUBLIC_H_

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int sceSkInit (unsigned int);
int sceSkQuit (unsigned int);
void *sceSkGetErxEntries(void);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _STANDARDKIT_EE_PUBLIC_H_ */
/* This file ends here, DON'T ADD STUFF AFTER THIS */
