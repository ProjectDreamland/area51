/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: stdio.h,v 1.12 2001/12/01 08:31:53 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         stdio.h
 *                         like ansi stdio
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 *       1.10           2000/10/21      tei
 */

#include <sys/types.h>

#ifndef	_STDIO_H
#define	_STDIO_H
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#ifndef EOF
#define EOF (-1)
#endif

extern void  StdioInit( int mode );

extern int   open(const char *filename, int flag, /* u_int mode */ ...);
extern int   close(int fd);
extern long  lseek(int fd, long offset, int whence);
extern int   read(int fd, void *buf, size_t nbyte);
extern int   write(int fd, const void *buf, size_t nbyte);
extern int   ioctl(int fd, long request, long arg);
extern int   ioctl2(int fd, long request, void *argp, size_t arglen,
		     void *bufp, size_t buflen);
extern long long lseek64(int fd, long long offset, int whence);

extern int   remove(const char *filename);
extern int   mkdir(const char *dirname, int mode);
extern int   rmdir(const char *dirname);
extern int   rename(const char *oldname, const char *newname);
extern int   symlink(const char *existing, const char *newfile);
extern int   chdir(const char *dirname);
extern int   sync(const char *devname, int flag);
extern int   readlink(const char *path, char *buf, size_t bufsize);

extern int   sprintf(char *buf, const char *fmt, ...);
extern int   printf(const char *fmt, ...);
extern int   getchar();
extern char *gets(char *s);
extern int   putchar(int c);
extern int   puts(const char *s);

extern int   fdprintf(int fd, const char *format, ...);
extern int   fdgetc(int fd);
extern char *fdgets(char *buf, int fd);
extern int   fdputc(int c, int fd);
extern int   fdputs(const char *s, int fd);

#if defined(_STDARG_H)
extern int   vsprintf(char *buf, const char *fmt, va_list ap);
extern int   vfdprintf(int fd, const char *format, va_list ap);
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif	/* _STDIO_H */


