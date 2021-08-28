/* SCE CONFIDENTIAL
 $PSLibId$
 $Id: erxcrt0.s,v 1.2 2003/08/01 05:17:39 kono Exp $
*/
/*
*                      Emotion Engine Library
*                          Version 3.00
*                           Shift-JIS
*
*      Copyright (C) 1998-2002 Sony Computer Entertainment Inc.
*                        All Rights Reserved.
*
*                       ee-gcc         crt0.s
*
*       Version        Date            Design      Log
*  --------------------------------------------------------------------
*       3.00           Mar.12.2003     kono        for ERX
*/



#define zero	$0	/* wired zero */
#define at		$1	/* assembler temp */
#define v0		$2	/* return value */
#define v1		$3
#define a0		$4	/* argument registers */
#define a1		$5
#define a2		$6
#define a3		$7
#define t0		$8	/* caller saved */
#define t1		$9
#define t2		$10
#define t3		$11
#define t4		$12
#define t5		$13
#define t6		$14
#define t7		$15
#define s0		$16	/* callee saved */
#define s1		$17
#define s2		$18
#define s3		$19
#define s4		$20
#define s5		$21
#define s6		$22
#define s7		$23
#define t8		$24	/* code generator */
#define t9		$25
#define k0		$26	/* kernel temporary */
#define k1		$27
#define gp		$28	/* global pointer */
#define sp		$29	/* stack pointer */
#define fp		$30	/* frame pointer */
#define ra		$31	/* return address */



#ifdef __mips16
.set nomips16	/* This file contains 32 bit assembly code. */
#endif

	.global		ENTRYPOINT

	.extern		_init
	.type		_init, @function
	.weak		_init
	.extern		_fini
	.type		_fini, @function
	.weak		_fini

	.extern		start
	.type		start, @function
	.weak		start

#if (__GNUC__==2)
	.extern		start__FiiPCPCc
	.type		start__FiiPCPCc, @function
	.weak		start__FiiPCPCc
#elif (__GNUC__==3)
	.extern		_Z5startiiPKPKc
	.type		_Z5startiiPKPKc, @function
	.weak		_Z5startiiPKPKc
#endif

	.extern		__sceKernlGetEhSemaId

	.set		noreorder
	.set		noat


	.text
	.p2align	5

erxcrt0:
		nop
		nop
ENTRYPOINT:


// int start(int reason, int argc, const char *const argv[])
	.ent	_erxcrt0
_erxcrt0:
		addiu		sp, sp, -80
		sd			a0,  0(sp)				// a0 save
		sd			a1,  8(sp)				// a1 save
		sd			a2, 16(sp)				// a2 save
		sd			a3, 24(sp)				// a3 save
		sd			t0, 32(sp)				// t0 save
		sd			t1, 40(sp)				// t1 save
		sd			t2, 48(sp)				// t2 save
		sd			t3, 56(sp)				// t3 save
		sd			s0, 64(sp)				// s0 save
		sd			ra, 72(sp)				// ra save

		beqz		a0, reason_start		// if(reason==0) goto reason_start
		addiu		t9, a0, -1				// t9 = a0 - 1
		beqz		t9, reason_stop			// if(reason==1) goto reason_stop
		nop

	// unknown reason code
		b			erx_end
		addiu		v0, zero, 1				// v0 = 1 = SCE_ERX_NO_RESIDENT_END


reason_start:
		jal			__sceKernlGetEhSemaId
		nop
		la			t0, __sce_eh_sema_id
		sw			v0, 0(t0)

#if (__GNUC__==2)
		bal			erx_setup_typeinfo
		nop
#endif	// (__GNUC__==2)

	// call .init functions
		lui			t9, %hi(_init)
		addiu		t9, t9, %lo(_init)
		beqz		t9, skip_call_init
		nop
		jalr		t9						// call constructors for global objects
		nop
skip_call_init:
#if (__GNUC__==2)
		bal			erx_global_constructor
		nop
#endif	// (__GNUC__==2)

		ld			a0,  0(sp)				// a0 restore
		ld			a1,  8(sp)				// a1 restore
		ld			a2, 16(sp)				// a2 restore
		ld			a3, 24(sp)				// a3 restore
		ld			t0, 32(sp)				// t0 restore
		ld			t1, 40(sp)				// t1 restore
		ld			t2, 48(sp)				// t2 restore
		bal			erx_main
		ld			t3, 56(sp)				// t3 restore

		b			erx_end
		nop

reason_stop:
		ld			a0,  0(sp)				// a0 restore
		ld			a1,  8(sp)				// a1 restore
		ld			a2, 16(sp)				// a2 restore
		ld			a3, 24(sp)				// a3 restore
		ld			t0, 32(sp)				// t0 restore
		ld			t1, 40(sp)				// t1 restore
		ld			t2, 48(sp)				// t2 restore
		bal			erx_main
		ld			t3, 56(sp)				// t3 restore

		addiu		t9, v0, -1				// t9 = v0 - 1
		bne			t9, zero, erx_end		// if(v0!=SCE_ERX_NO_RESIDENT_END) goto erx_end
		move		s0, v0					// s0 = v0

#if (__GNUC__==2)
		bal			erx_global_destructor
		nop
#endif	// (__GNUC__==2)

	// call .fini functions
		lui			t9, %hi(_fini)
		addiu		t9, t9, %lo(_fini)
		beqz		t9, skip_call_fini
		nop
		jalr		t9						// call destructors for global objects
		nop
skip_call_fini:
		move		v0, s0					// v0 = s0

erx_end:
		ld			s0, 64(sp)				// s0 restore
		ld			ra, 72(sp)				// ra restore
		jr			ra
		addiu		sp, sp, +80
	.end		_erxcrt0



// call erx module's start() function
	.ent		erx_main
erx_main:
		addiu		sp, sp, -144
		sq			s0,   0(sp)
		sq			s1,  16(sp)
		sq			s2,  32(sp)
		sq			s3,  48(sp)
		sq			s4,  64(sp)
		sq			s5,  80(sp)
		sq			s6,  96(sp)
		sq			s7, 112(sp)
		sd			ra, 128(sp)

		// clear GPR
		por			at,  zero, zero			// at
		por			v0,  zero, zero			// v0
		por			v1,  zero, zero			// v1
		por			t4,  zero, zero			// t4
		por			t5,  zero, zero			// t5
		por			t6,  zero, zero			// t6
		por			t7,  zero, zero			// t7
		por			t8,  zero, zero			// t8
		por			s0,  zero, zero			// s0
		por			s1,  zero, zero			// s1
		por			s2,  zero, zero			// s2
		por			s3,  zero, zero			// s3
		por			s4,  zero, zero			// s4
		por			s5,  zero, zero			// s5
		por			s6,  zero, zero			// s6
		por			s7,  zero, zero			// s7
		pmthi		zero					// hi
		pmtlo		zero					// lo
		mtsab		zero, 0					// sa

/*
* call main program
*/
		la			t9, start
		bne			t9, zero, exec_erx_entry
		nop
#if (__GNUC__==2)
		la			t9, start__FiiPCPCc
#elif (__GNUC__==3)
		la			t9, _Z5startiiPKPKc
#endif	// __GNUC__
		bne			t9, zero, exec_erx_entry
		nop

		li			v0, 2
		b			erx_main_end
		movn		v0, zero, a0			// v0 = (a0!=0) ? 0 : v0
											//    = (reason==SCE_ERX_REASON_START) ? SCE_ERX_REMOVABLE_RESIDENT_END : SCE_ERX_NO_RESIDENT_END

exec_erx_entry:
		jalr		t9						// call 'start' function
		nop

erx_main_end:
		lq			s0,   0(sp)
		lq			s1,  16(sp)
		lq			s2,  32(sp)
		lq			s3,  48(sp)
		lq			s4,  64(sp)
		lq			s5,  80(sp)
		lq			s6,  96(sp)
		lq			s7, 112(sp)
		ld			ra, 128(sp)
		jr			ra
		addiu		sp, sp, +144
	.end		erx_main


	.section	.data
	.global		__sce_eh_sema_id
__sce_eh_sema_id:
	.dc.w		0


#if (__GNUC__==2)
// copying type_info from resident libgcc.a
// for ee-gcc 2.96-ee-001003_1
	.section	.text

	.ent		erx_setup_typeinfo
erx_setup_typeinfo:
		addiu		sp, sp, -16
		sd			s0, 0(sp)
		sd			ra, 8(sp)
		la			s0, ti_setup_table
ti_setup_lop:
		lw			t0, 0(s0)
		beqz		t0, ti_setup_end
		nop
		jalr		t0
		nop
		lw			t0, 4(s0)
		ldr			t1, 0(v0)
		ldl			t1, 7(v0)
		sd			t1, 0(t0)
		b			ti_setup_lop
		addiu		s0, s0, 8

ti_setup_end:
		ld			ra, 8(sp)
		ld			s0, 0(sp)
		jr			ra
		addiu		sp, sp, +16
	.end		erx_setup_typeinfo

#define TYPE_INFO(TN)				\
	.extern		__tf##TN			; \
	.type		__tf##TN, @function	; \
	.global		__ti##TN			; \
	.data							; \
	.p2align	3					; \
__ti##TN:							; \
	.dc.d		0					; \
	.section	.rodata				; \
	.word		__tf##TN			; \
	.word		__ti##TN

	.section	.rodata
	.p2align	3
ti_setup_table:
TYPE_INFO(v)					// void
TYPE_INFO(x)					// long long
TYPE_INFO(l)					// long
TYPE_INFO(i)					// int
TYPE_INFO(s)					// short
TYPE_INFO(b)					// bool
TYPE_INFO(c)					// char
TYPE_INFO(w)					// ???
TYPE_INFO(r)					// ???
TYPE_INFO(d)					// double
TYPE_INFO(f)					// float
TYPE_INFO(Ui)					// unsigned int
TYPE_INFO(Ul)					// unsigned long
TYPE_INFO(Ux)					// unsigned long lon
TYPE_INFO(Us)					// unsigned short
TYPE_INFO(Uc)					// unsigned char
TYPE_INFO(Sc)					// signed char
TYPE_INFO(I80)					// long128
TYPE_INFO(UI80)					// u_long128
TYPE_INFO(9type_info)			// type_info
	.word		0
	.word		0


// global constructor and destructor for ee-gcc 2.96-ee-001003_1
	.extern		__CTOR_LIST__
	.extern		__DTOR_LIST__
	.extern		__EH_FRAME_BEGIN__

	.section	.text
	.ent		erx_global_constructor
erx_global_constructor:
		addiu		sp, sp, -32
		sd			s0,  0(sp)
		sd			s1,  8(sp)
		sd			ra, 16(sp)

		la			a0, __EH_FRAME_BEGIN__
		la			a1, dummy_object
		jal			__register_frame_info
		nop

		la			s0, __CTOR_LIST__
		lw			t0, 0(s0)
		addiu		t0, t0, +1
		bne			t0, zero, erx_ctor_end			// global constructor isn't exist
		move		s1, zero
erx_ctor_lop1:
		addiu		s0, s0, +4
		lw			t0, 0(s0)
		nop
		nop
		nop
		bnel		t0, zero, erx_ctor_lop1
		addiu		s1, s1, +1

erx_ctor_lop2:
		addiu		s0, s0, -4
		beqz		s1, erx_ctor_end
		lw			t0, 0(s0)
		jalr		t0
		addiu		s1, s1, -1
		b			erx_ctor_lop2
		nop
erx_ctor_end:
		ld			s0,  0(sp)
		ld			s1,  8(sp)
		ld			ra, 16(sp)
		jr			ra
		addiu		sp, sp, +32
	.end		erx_global_constructor


	.ent		erx_global_destructor
erx_global_destructor:
		addiu		sp, sp, -16
		sd			s0,  0(sp)
		sd			ra,  8(sp)

		la			s0, __DTOR_LIST__
erx_dtor_lop:
		addiu		s0, s0, +4
		lw			t0, 0(s0)
		beqz		t0, erx_dtor_end
		nop
		jal			t0
		nop
		b			erx_dtor_lop
		nop
erx_dtor_end:
		la			a0, __EH_FRAME_BEGIN__
		jal			__deregister_frame_info
		nop
		ld			s0,  0(sp)
		ld			ra,  8(sp)
		jr			ra
		addiu		sp, sp, +16
	.end		erx_global_destructor


	.section	.data
	.p2align	4
dummy_object:
	.space		32

#endif	// (__GNUC__==2)
