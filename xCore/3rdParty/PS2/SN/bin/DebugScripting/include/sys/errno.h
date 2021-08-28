/*  This header file is machine generated. 
Modify EiC/config/genstdio.c, or the target independent source 
files it reads, in order to modify this file.  Any 
direct modifications to this file will be lost. 
*/

#ifndef SYSERRNOH_
#define SYSERRNOH_

/* ISO C STUFF */
#define EDOM		33	/* Math argument out of domain of func */
#define ERANGE		34	/* Math result not representable */

/* POSIX.1 STUFF */
#ifdef _POSIX_SOURCE
#define E2BIG	7
#define EACCES	13
#define EAGAIN	11
#define EBADF	9
#define EBUSY	16
#define ECHILD	10
#define EDEADLK	36
#define EEXIST	17
#define EFAULT	14
#define EFBIG	27
#define EINTR	4
#define EINVAL	22
#define EIO	5
#define EISDIR	21
#define EMFILE	24
#define EMLINK	31
#define ENAMETOOLONG	38
#define ENFILE	23
#define ENODEV	19
#define ENOENT	2
#define ENOEXEC	8
#define ENOLCK	39
#define ENOMEM	12
#define ENOSPC	28
#define ENOSYS	40
#define ENOTDIR	20
#define ENOTEMPTY	41
#define ENOTTY	25
#define ENXIO	6
#define EPERM	1
#define EPIPE	32
#define EROFS	30
#define ESPIPE	29
#define ESRCH	3
#define EXDEV	18
#endif /* end  _POSIX_SOURCE */

#endif
