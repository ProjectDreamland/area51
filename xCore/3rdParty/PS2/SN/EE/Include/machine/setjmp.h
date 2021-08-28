

#if defined(__mips__) && !defined(__R5900__)
#define _JBLEN 11
#endif

#ifdef __R5900__
/* allow for 11 128-bit registers and 8 32-bit FP regs */
#undef _JBLEN
#define _JBLEN 14
typedef int __jmp_buf_elt __attribute__ ((mode(TI)));
#define _JBTYPE __jmp_buf_elt
#endif

#ifdef _JBLEN
#ifdef _JBTYPE
typedef	_JBTYPE jmp_buf[_JBLEN];
#else
typedef	int jmp_buf[_JBLEN];
#endif

#endif
