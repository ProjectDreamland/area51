/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * libspu2m.h
 *     SPU2 local memory manager
 */

#ifndef _SPU2_MEMORY_ALLOCATE_H_
#define _SPU2_MEMORY_ALLOCATE_H_

#define SCESPU2MEM_TABLE_UNITSIZE 12

#define SCESPU2MEM_NO_EFFECT  1
#define SCESPU2MEM_USE_EFFECT 0

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int  sceSpu2MemInit(void *, unsigned int, unsigned int);
int  sceSpu2MemQuit(void);

int  sceSpu2MemAllocate(unsigned int);
void sceSpu2MemFree(unsigned int);

void *sceSpu2MemGetErxEntries(void);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _SPU2_MEMORY_ALLOCATE_H_ */
/* This file ends here, DON'T ADD STUFF AFTER THIS */
