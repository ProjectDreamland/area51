/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
#ifndef _STANDARD_KIT_COMMON_ERRNO_H_
#define _STANDARD_KIT_COMMON_ERRNO_H_
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 2001, 2002 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * sk/errno.h
 *     Standard Kit / Sound System --- error codes
 */

/* Common error codes for Standard Kit */

#define SCESK_OK			0

#define SCESK_EERROR_IN_ARGUMENT	(-100)
#define SCESK_EINDEX_EXCEEDED		(-101)
#define SCESK_EINVALID_ARGUMENT		(-102)

#define SCESK_EERROR_IN_STATUS		(-200)
#define SCESK_ENOT_BOUND		(-201)
#define SCESK_EALREADY_BOUND		(-202)
#define SCESK_EINVALID_STATUS		(-299)

#define SCESK_EERROR_IN_RESOURCE	(-300)
#define SCESK_ENOT_ALLOCATE		(-301)
#define SCESK_ENOT_FREE			(-302)
#define SCESK_ENOT_START		(-303)
#define SCESK_ENOT_STOP			(-304)
#define SCESK_ENOT_PRELOAD		(-305)
#define SCESK_ECOMMAND_NOT_FOUND	(-310)

#define SCESK_EERROR_IN_CSL		(-400)
#define SCESK_ECSL_INIT			(-401)
#define SCESK_ECSL_LOAD			(-402)

/* ----------------------------------------------------------------
 *	End on File
 * ---------------------------------------------------------------- */
#endif /* _STANDARD_KIT_COMMON_ERRNO_H_ */
/* This file ends here, DON'T ADD STUFF AFTER THIS */
