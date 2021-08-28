/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: libmsin.h,v 1.7 2003/05/29 06:48:35 kaol Exp $
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 1998-1999, 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * libmsin - libmsin.h
 *    EE Midi Stream Message Input
 */
#ifndef _SCE_LIBMSIN_H
#define _SCE_LIBMSIN_H
#include <csl.h>
#include <modmsin.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

void *sceMSIn_GetErxEntries(void);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /* !_SCE_LIBMSIN_H */
