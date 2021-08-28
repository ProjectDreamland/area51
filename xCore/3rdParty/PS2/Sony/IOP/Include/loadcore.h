/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: loadcore.h,v 1.18 2003/06/03 08:48:04 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         loadcore.h
 *                         Module manager
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 *       1.01           2000/12/08      tei 	    add ReleaseLibraryEntries()
 */

#ifndef _LOADCORE_H
#define _LOADCORE_H

#define RESIDENT_END		(0)
#define REMOVABLE_RESIDENT_END	(2)
#define NO_RESIDENT_END		(1)
#define FAREWELL_END		(1)

typedef struct _libhead {
    struct _libhead	*next;
    struct _libcaller	*client;
    unsigned short	version;
    unsigned short	flags;
    char		name[8];
} libhead ;

#define LIBMAGIC	(0x41e00000)
typedef struct _libcaller {
    unsigned long	magic;
    struct _libcaller	*client;
    unsigned short	version;
    unsigned short	flags;
    char		name[8];
} libcaller;

typedef struct _moduleinfo {
    const char			*name;
    const unsigned short	version;
} ModuleInfo;

void FlushIcache(void);
void FlushDcache(void);

int RegisterLibraryEntries(libhead *lib);
int ReleaseLibraryEntries(libhead *lib);

int SetRebootTimeLibraryHandlingMode(libhead *lib, int mode);
/* mode define */
#define RTLH_MODE_di		(0x0<<1)
#define RTLH_MODE_ei		(0x1<<1)
#define RTLH_MODE_ei_di		(0x2<<1)

#endif /* _LOADCORE_H */
