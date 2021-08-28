/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *
 *  C lang start support routine for IOP native kernel
 *
 *  Copyright 1998 Sony Computer Entertainment Inc.
 *  All Rights Reserved
 * 
 * $Id: crt0.c,v 1.3 2002/03/27 05:13:01 xokano Exp $
 */

#include <setjmp.h>
#include <kernel.h>

static jmp_buf exitbuf;

extern int main(int argc, char *argv[]);

int start(int argc, char *argv[])
{
    if( setjmp(exitbuf) == 0 ) {
	main(argc, argv);
    }
    return NO_RESIDENT_END;
}

void _exit()
{
    longjmp(exitbuf, 1);
}

void exit()
{
    longjmp(exitbuf, 1);
}

void __main()
{
}
