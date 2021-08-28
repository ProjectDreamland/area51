#ifndef _SETJMP
#define _SETJMP


typedef char jmp_buf[64];

#define setjmp(x)  (x),__eicsetjmp
#define longjmp(x,y) {int f = y; (x)+(f),__eiclongjmp;}


#endif






