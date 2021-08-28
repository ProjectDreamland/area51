/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 2.8
 */
/*
 *                      Emotion Engine Library
 *                          Version 3.0
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                            crt0.s
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.10           Oct.12.1999     horikawa    renewal
 *       1.50           May.16.2000     horikawa
 *       2.50           Feb.18.2002     kumagae
 *	 2.60		Sep.30.2002	shibata	    add calls to _init,_fini
 *	 2.70		Sep.30.2002	shibata	    call _fini via atexit()
 *       2.80		Nov.28.2002	shibata     change local label name
 *	 3.00		May.05.2003	shibata	    fix bss-clear bug
 *                                                  and keep the compatiblity
 *                                                  with previous crt0.s
 *  	 3.01		May.14.2003	shibata	    add #if __GNUC__
 */

#define SN_PROVIEW

	.text
	.align	2
#ifdef __GNUC__	
	.globl	_init
	.type	_init, @function
	.weak	_init
	.globl	_fini
	.type	_fini, @function
	.weak	_fini
#endif /* #ifdef __GNUC__ */
	
#ifdef __mips16
	.set nomips16	/* This file contains 32 bit assembly code. */
#endif

#define	ARG_SIZ     256 + 16*4 + 1*4

	.set noat
    	.set noreorder
	.global ENTRYPOINT
	.global _start
	.ent	_start
	.text				# 0x00200000
	nop
	nop
ENTRYPOINT:
_start:

#if defined(SN_PROVIEW)
	/* $4 = 第 1 引数（レジスタ渡し）*/
	lui	$2, %hi(_args_ptr)
	addiu	$2, $2, %lo(_args_ptr)
	/* ProView only:
	 * - ProView ELF loader replaces this next instruction with: sw $4, ($2)
	 * - If not loading ELF on a ProView target, then _args_ptr remains NULL.
	 *   This is to prevent _args_ptr being set to $4 which isn't initialised.
	 * - NB. T10000s will use the regular _args value, so args will be as expected.
	 */
	sw	$0, ($2)
#endif

/* Clear GPR , FPR */
        padduw  $1, $0, $0
        padduw  $2, $0, $0
        padduw  $3, $0, $0
        padduw  $4, $0, $0
        padduw  $5, $0, $0
        padduw  $6, $0, $0
        padduw  $7, $0, $0
        padduw  $8, $0, $0
        padduw  $9, $0, $0
        padduw  $10, $0, $0
        padduw  $11, $0, $0
        padduw  $12, $0, $0
        padduw  $13, $0, $0
        padduw  $14, $0, $0
        padduw  $15, $0, $0
        padduw  $16, $0, $0
        padduw  $17, $0, $0
        padduw  $18, $0, $0
        padduw  $19, $0, $0
        padduw  $20, $0, $0
        padduw  $21, $0, $0
        padduw  $22, $0, $0
        padduw  $23, $0, $0
        padduw  $24, $0, $0
        padduw  $25, $0, $0

        padduw  $28, $0, $0
        padduw  $29, $0, $0	
        padduw  $30, $0, $0
        padduw  $31, $0, $0

	mthi    $0
	mthi1   $0
	mtlo    $0
	mtlo1   $0
	mtsah   $0, 0
	
    	mtc1    $0, $f0
   	mtc1    $0, $f1
    	mtc1    $0, $f2
    	mtc1    $0, $f3
    	mtc1    $0, $f4
    	mtc1    $0, $f5
    	mtc1    $0, $f6
    	mtc1    $0, $f7
    	mtc1    $0, $f8
    	mtc1    $0, $f9
    	mtc1    $0, $f10
    	mtc1    $0, $f11
    	mtc1    $0, $f12
    	mtc1    $0, $f13
    	mtc1    $0, $f14
    	mtc1    $0, $f15
    	mtc1    $0, $f16
    	mtc1    $0, $f17
    	mtc1    $0, $f18
    	mtc1    $0, $f19
    	mtc1    $0, $f20
    	mtc1    $0, $f21
    	mtc1    $0, $f22
    	mtc1    $0, $f23
    	mtc1    $0, $f24
    	mtc1    $0, $f25
    	mtc1    $0, $f26
    	mtc1    $0, $f27
    	mtc1    $0, $f28
    	mtc1    $0, $f29
    	mtc1    $0, $f30
    	mtc1    $0, $f31
    	adda.s  $f0, $f1
    	sync.p
    	ctc1    $0, $31
	
/*
 * clear .bss
 */
zerobss:
	lui	$2, %hi(_fbss)
	lui	$3, %hi(_end)
	addiu	$2, $2, %lo(_fbss)
	addiu	$3, $3, %lo(_end)
	#
1:	andi	$4, $2, 0x000f
	beqz	$4, 2f
	nop
	sb	$0, ($2)
	addiu	$2, $2, 0x1
	nop
	j	1b
	nop
	#
2:	lui	$4, 0xffff
	ori	$4, $4, 0xfff0
	and	$4, $3, $4
3:	beq	$2, $4, 4f
	nop
	sq	$0, ($2)
	addiu	$2, $2, 16
	nop
	nop
	j	3b
	nop
	#
4:	beq	$2, $3, 5f	
	nop
	sb	$0, ($2)
	addiu	$2, $2, 0x1
	nop
	nop
	j	4b
	nop
5:
/*
 * initialize main thread
 */
	lui	$4, %hi(_gp)
	lui	$5, %hi(_stack)
	lui	$6, %hi(_stack_size)
	lui	$7, %hi(_args)
	lui	$8, %hi(_root)
	addiu	$4, $4, %lo(_gp)
	addiu	$5, $5, %lo(_stack)
	addiu	$6, $6, %lo(_stack_size)
	addiu	$7, $7, %lo(_args)
	addiu	$8, $8, %lo(_root)
	move	$28, $4
	addiu	$3, $0, 60
	syscall
	move	$29, $2

/*
 * initialize heap area
 */
	lui	$4, %hi(_end)
	lui	$5, %hi(_heap_size)
	addiu	$4, $4, %lo(_end)
	addiu	$5, $5, %lo(_heap_size)
	addiu	$3, $0, 61
	syscall

/*
 * initialize System
 */
	jal	_InitSys
	nop

/*
 * flush data cache
 */
	jal	FlushCache
	move	$4, $0
#ifdef __GNUC__
/*
 * call _init and regist pointer of _fini into atexit table.
 */
#if 0
	lui	$16, %hi(_init)
	addiu	$16, $16, %lo(_init)
	beqz	$16, 8f
	nop
	jalr	$16		# call constructors for global objects
	nop
8:
	lui	$4, %hi(_fini)
	addiu	$4, $4, %lo(_fini)
	beqz	$4, 9f
	nop
	jal	atexit		# atexit (&_fini); 
	nop
    #endif
#endif /* #ifdef __GNUC__ */	
/*
 * call main program
 */
9:
	ei
#if defined(SN_PROVIEW)
 	lui	$2, %hi(_args_ptr)
 	addiu	$2, $2, %lo(_args_ptr)

 	lw	$3, ($2)
	
 	beq	$3, $0, _skipArgV
 	nop

 	addiu	$2, $3, 4
	b	_run	
	nop

_skipArgV:
	lui	$2, %hi(_args)
	addiu	$2, $2, %lo(_args)

#else
	lui	$2, %hi(_args)
	addiu	$2, $2, %lo(_args)
#endif

_run:
	lw	$4, ($2)
	jal	main
	addiu	$5, $2, 4
/*
 * call exit
 */
#if defined(SN_PROVIEW)
	j	_root
	nop
#else
	j	exit
	move	$4, $2		# save a return value of main
#endif
	.end	_start

/**************************************/
	.align	3
	.global	_exit
	.ent	_exit
_exit:
#if defined(SN_PROVIEW)
	j 	_root
	nop
#else
	j	Exit
	nop
#endif
	.end	_exit
    
	.align	3
	.ent	_root
_root:
#if defined(SN_PROVIEW)
	lui	$2, %hi(_args_ptr)
	addiu	$2, $2, %lo(_args_ptr)
	lw	$3, ($2)
	jal	SignalSema
	lw	$4, ($3)
#endif
	addiu	$3, $0, 35		# ExitThread();
	syscall
	.end	_root

/**************************************/

	.bss
	.align	6
_args: .space	ARG_SIZ

#if defined(SN_PROVIEW)
	.data
_args_ptr:
	.space 4
#endif
