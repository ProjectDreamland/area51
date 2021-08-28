/*  This header file is machine generated. 
Modify EiC/config/genstdio.c, or the target independent source 
files it reads, in order to modify this file.  Any 
direct modifications to this file will be lost. 
*/

#if defined(_need_size_t) && !defined(_SIZE_T)
#define _SIZE_T
typedef unsigned int size_t;
#endif

#if defined( _need_ptrdiff_t) && !defined(_PTRDIFF_T)
#define _PTRDIFF_T
typedef int ptrdiff_t;
#endif

#if  defined( _need_wchar_t) &&  !defined(_WCHAR_T)
#define _WCHAR_T
typedef short wchar_t;
#endif

#if  defined( _need_NULL) && !defined (NULL)
#define NULL (void*)0
#endif

#if defined( _need_clock_t) && !defined(_CLOCK_T)
#define _CLOCK_T
typedef unsigned long  clock_t;        /* units=ticks (typically 60/sec) */
#endif

#if defined(_need_time_t) && !defined(_TIME_T)
#define _TIME_T
typedef long  time_t;         /* value = secs since epoch */
#endif

#if defined(_need_eic_ptr) && !defined(_eic_ptr) && !defined(_EiC)
#define _eic_ptr
typedef struct {void *p, *sp, *ep;} ptr_t;
#endif

