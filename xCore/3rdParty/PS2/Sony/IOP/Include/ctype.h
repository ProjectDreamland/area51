/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: ctype.h,v 1.3 2001/03/01 07:07:10 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         ctype.h
 *
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 */

#ifndef	_CTYPE_H
#define	_CTYPE_H

#define	_U	0x01	/* upper case letter */
#define	_L	0x02	/* lower case letter */
#define	_N	0x04	/* digit */
#define	_S	0x08	/* space, tab, newline, vertical tab, formfeed, or
				carriage return */
#define _P	0x10	/* punctuation character */
#define _C	0x20	/* control character or delete */
#define _X	0x40	/* hexadecimal digit [0-9a-fA-F]*/
#define	_B	0x80	/* blank (space) */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
extern 	char	toupper(char);
extern 	char	tolower(char);
extern  int	look_ctype_table(unsigned int c);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#define	isalpha(c)	(look_ctype_table((unsigned int)c)&(_U|_L))
#define	isupper(c)	(look_ctype_table((unsigned int)c)&_U)
#define	islower(c)	(look_ctype_table((unsigned int)c)&_L)
#define	isdigit(c)	(look_ctype_table((unsigned int)c)&_N)
#define	isxdigit(c)	(look_ctype_table((unsigned int)c)&(_X|_N))
#define	isspace(c)	(look_ctype_table((unsigned int)c)&_S)
#define ispunct(c)	(look_ctype_table((unsigned int)c)&_P)
#define isalnum(c)	(look_ctype_table((unsigned int)c)&(_U|_L|_N))
#define isprint(c)	(look_ctype_table((unsigned int)c)&(_P|_U|_L|_N|_B))
#define isgraph(c)	(look_ctype_table((unsigned int)c)&(_P|_U|_L|_N))
#define iscntrl(c)	(look_ctype_table((unsigned int)c)&_C)
#define isascii(c)	((unsigned int)(c)<=0x7f)
#define toascii(c)	((unsigned char)(c)&0x7f)
#define _toupper(c)	((unsigned char)(c)-'a'+'A')
#define _tolower(c)	((unsigned char)(c)-'A'+'a')

#endif	/* _CTYPE_H */
