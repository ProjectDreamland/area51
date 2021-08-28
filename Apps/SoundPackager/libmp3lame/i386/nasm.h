
;	Copyright (C) 1999 URURI

;	nasm用マクロ
;	1999/08/21 作成
;	1999/10/10 幾つか追加
;	1999/10/27 aout対応
;	1999/11/07 pushf, popf のNASMのバグ対応
;	1999/12/02 for BCC ( Thanks to Miquel )

; for Windows Visual C++        -> define WIN32
;             Borland or cygwin ->        WIN32 and COFF
; for FreeBSD 2.x               ->        AOUT
; for TownsOS                   ->        __tos__
; otherwise                     ->   none

;名前の付け方

BITS 32

%ifdef WIN32
	%define _NAMING
	%define segment_code segment .text align=16 class=CODE use32
	%define segment_data segment .data align=16 class=DATA use32
%ifdef __BORLANDC__
	%define segment_bss  segment .data align=16 class=DATA use32
%else
	%define segment_bss  segment .bss align=16 class=DATA use32
%endif
%elifdef AOUT
	%define _NAMING
	%define segment_code segment .text
	%define segment_data segment .data
	%define segment_bss  segment .bss
%else
	%define segment_code segment .text align=16 class=CODE use32
	%define segment_data segment .data align=16 class=DATA use32
	%define segment_bss  segment .bss align=16 class=DATA use32
%endif

%ifdef __tos__
group CGROUP text
group DGROUP data
%endif

;単精度浮動小数点形式

%idefine float dword
%idefine fsize 4
%idefine fsizen(a) (fsize*(a))

;ワード形式

%idefine wsize 2
%idefine wsizen(a) (wsize*(a))
%idefine dwsize 4
%idefine dwsizen(a) (dwsize*(a))

;REG

%define r0 eax
%define r1 ebx
%define r2 ecx
%define r3 edx
%define r4 esi
%define r5 edi
%define r6 ebp
%define r7 esp

;MMX,3DNow!,SSE

%define pmov	movq
%define pmovd	movd

%define pupldq	punpckldq
%define puphdq	punpckhdq
%define puplwd	punpcklwd
%define puphwd	punpckhwd

%define xm0 xmm0
%define xm1 xmm1
%define xm2 xmm2
%define xm3 xmm3
%define xm4 xmm4
%define xm5 xmm5
%define xm6 xmm6
%define xm7 xmm7

;シャッフル用の4進マクロ

%define R4(a,b,c,d) (a*64+b*16+c*4+d)

;Cライクな簡易マクロ

%imacro globaldef 1
	%ifdef _NAMING
		%define %1 _%1
	%endif
	global %1
%endmacro

%imacro externdef 1
	%ifdef _NAMING
		%define %1 _%1
	%endif
	extern %1
%endmacro

%imacro proc 1
	%push	proc
	%ifdef _NAMING
		global _%1
	%else
		global %1
	%endif

	align 32
%1:
_%1:

	%assign %$STACK 0
	%assign %$STACKN 0
	%assign %$ARG 4
%endmacro

%imacro endproc 0
	%ifnctx proc
		%error expected 'proc' before 'endproc'.
	%else
		%if %$STACK > 0
			add esp, %$STACK
		%endif

		%if %$STACK <> (-%$STACKN)
			%error STACKLEVEL mismatch check 'local', 'alloc', 'pushd', 'popd'
		%endif

		ret
		%pop
	%endif
%endmacro

%idefine sp(a) esp+%$STACK+a

%imacro arg 1
	%00	equ %$ARG
	%assign %$ARG %$ARG+%1
%endmacro

%imacro local 1
	%assign %$STACKN %$STACKN-%1
	%00 equ %$STACKN
%endmacro

%imacro alloc 0
	sub esp, (-%$STACKN)-%$STACK
	%assign %$STACK (-%$STACKN)
%endmacro

%imacro pushd 1-*
	%rep %0
		push %1
		%assign %$STACK %$STACK+4
	%rotate 1
	%endrep
%endmacro

%imacro popd 1-*
	%rep %0
	%rotate -1
		pop %1
		%assign %$STACK %$STACK-4
	%endrep
%endmacro

; bug of NASM-0.98
%define pushf db 0x66, 0x9C
%define popf  db 0x66, 0x9D
