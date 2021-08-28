/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 2001-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         eenetctl.h
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2002/07/01      komaki
 */

#ifndef _IOP_EENETCTL_H_
#define _IOP_EENETCTL_H_

#include <netcnf.h>

/* error code */
#define EENETCTL_ERROR_INVALID_ARG -1
#define EENETCTL_ERROR_NO_MEM -2
#define EENETCTL_ERROR_ALREADY_EXIST -3

int sceEENetCtlSetConfiguration(sceNetCnfEnv_t *env);
int sceEENetCtlSetDialingData(
	const char *dialcnf_filename,
	sceNetCnfDial_t *dial
	);
int sceEENetCtlClearDialingData(
	const char *dialcnf_filename
	);

#endif /* _IOP_EENETCTL_H_ */
