#ifndef _CTYPEH
#define _CTYPEH

#pragma push_safeptr

#define _C_SPA 1                       /* space */
#define _C_DIG 2                       /* digit indicator */
#define _C_UPP 4                       /* upper case */
#define _C_LOW 8                       /* lower case */
#define _C_HEX 16                      /* [A-F or [a-f] */
#define _C_CTL 32                      /* Control */
#define _C_PUN 64                      /* punctuation */
#define _C_OTH 128                     /* other */


#define isalnum(c)  (_CtYpE[(int)(c)] & (_C_DIG|_C_UPP|_C_LOW))
#define isalpha(c)  (_CtYpE[(int)(c)] & (_C_UPP|_C_LOW))
#define iscntrl(c)  (_CtYpE[(int)(c)] & (_C_CTL|_C_OTH))
#define isdigit(c)  (_CtYpE[(int)(c)] & _C_DIG)
#define isgraph(c)  (_CtYpE[(int)(c)] & (_C_DIG|_C_LOW|_C_UPP|_C_PUN))
#define islower(c)  (_CtYpE[(int)(c)] & _C_LOW)
#define isprint(c)  (_CtYpE[(int)(c)] & (_C_DIG|_C_LOW|_C_UPP|_C_PUN|_C_SPA))
#define ispunct(c)  (_CtYpE[(int)(c)] & _C_PUN)
#define isspace(c)  (_CtYpE[(int)(c)] & (_C_SPA|_C_CTL))
#define isupper(c)  (_CtYpE[(int)(c)] & _C_UPP)
#define isxdigit(c) (_CtYpE[(int)(c)] & _C_HEX)

/*
 * The next two marcos work, but they are unsafe.
 * They must eventually be REPLACED.
 */
#define tolower(c)       (isupper(c) ? (c)+32:c)
#define toupper(c)       (islower(c) ? (c)-32:c)


#ifndef _EiC
extern unsigned short  *_CtYpE;
#else
unsigned short *_CtYpE;
unsigned short *_get_ctype(void);
_CtYpE = _get_ctype();
#endif

#pragma pop_ptr

#endif




