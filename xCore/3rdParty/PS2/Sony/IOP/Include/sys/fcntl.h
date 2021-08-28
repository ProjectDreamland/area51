/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: fcntl.h,v 1.3 2002/03/27 05:10:16 xokano Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         sys/fcntl.h
 *                         file control defines
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2000/10/21      tei
 */

#ifndef _SYS_FCNTL_H
#define _SYS_FCNTL_H

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

/*
  OPEN file
    fd = open("host1:FILENAME", O_RDONLY); <-- host0: for EE, host1: for IOP 
    n = read(fd, buffer, readsize);

  Write file
    ファイルが存在しないとエラーになる
        fd = open("host1:FILENAME", O_WRONLY);
        n = write(fd, buffer, readsize);
    ファイルが存在しないと自動的にファイルをつくる
        fd = open("host1:FILENAME", O_WRONLY|O_CREAT);
        n = write(fd, buffer, readsize);
    オープン後にファイルのサイズを 0 に戻す。
        fd = open("host1:FILENAME", O_WRONLY|O_TRUNC);
        n = write(fd, buffer, readsize);
    ファイルが存在しないと自動的にファイルをつくり、オープン後に
    ファイルのサイズを 0 に戻す。
        fd = open("host1:FILENAME", O_WRONLY|O_TRUNC|O_CREAT);
        n = write(fd, buffer, readsize);

*/

/* flags */
#define	FREAD		(0x0001)  /* readable */
#define	FWRITE		(0x0002)  /* writable */
#define	FNBLOCK		(0x0004)  /*   Reserved: non-blocking reads */
#define	FDIRO		(0x0008)  /* internal use for dopen */
#define	FRLOCK		(0x0010)  /*   Reserved: read locked (non-shared) */
#define	FWLOCK		(0x0020)  /*   Reserved: write locked (non-shared) */
#define	FAPPEND		(0x0100)  /* append on each write */
#define	FCREAT		(0x0200)  /* create if nonexistant */
#define	FTRUNC		(0x0400)  /* truncate to zero length */
#define EXCL		(0x0800)  /* exclusive create */
#define	FSCAN		(0x1000)  /*   Reserved: scan type */
#define	FRCOM		(0x2000)  /*   Reserved: remote command entry */
#define	FNBUF		(0x4000)  /*   Reserved: no ring buf. and console interrupt */
#define	FASYNC		(0x8000)  /*   Reserved: asyncronous i/o */

/* Flag for open() */
#define O_RDONLY        (FREAD)
#define O_WRONLY        (FWRITE)
#define O_RDWR          (FREAD|FWRITE)
#define O_NBLOCK        (FNBLOCK) /*   Reserved: Non-Blocking I/O */
#define O_APPEND        (FAPPEND) /* append (writes guaranteed at the end) */
#define O_CREAT         (FCREAT)  /* open with file create */
#define O_TRUNC         (FTRUNC)  /* open with truncation */
#define O_EXCL		(EXCL)	  /* exclusive create */
#define O_NOBUF         (FNBUF)	  /*   Reserved: no device buffer and console interrupt */
#define O_NOWAIT        (FASYNC)  /*   Reserved: asyncronous i/o */

#ifndef SEEK_SET
#define SEEK_SET	(0)
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	(1)
#endif
#ifndef SEEK_END
#define SEEK_END	(2)
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /* _SYS_FCNTL_H */
