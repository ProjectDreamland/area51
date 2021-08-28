/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: mount.h,v 1.2 2002/03/27 05:10:16 xokano Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         sys/mount.h
 *                         file system mount
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2000/10/21      tei
 */

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern int format(const char *path, const char *blkdevname,
		  void *arg, int arglen);

extern int mount(const char *fsdevname, const char *blkdevname, int flag,
		 void *arg, int arglen);
/* mount() flag */
#define SCE_MT_RDWR	0x00
#define SCE_MT_RDONLY	0x01
#define SCE_MT_ROBUST	0x02
#define SCE_MT_ERRCHECK	0x04

extern int umount(const char *fsdevname);
extern int devctl (const char *devname, int cmd, void *arg, int arglen,
		   void *bufp, size_t buflen);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /* _SYS_MOUNT_H */
