;
;
; 	assembler routines to detect CPU-features
;
;	MMX / 3DNow! / SIMD / SIMD2
;
;	for the LAME project
;	Frank Klemm, Robert Hegemann 2000-10-12
;

%include "nasm.h"

	globaldef	has_MMX_nasm
	globaldef	has_3DNow_nasm
	globaldef	has_SIMD_nasm
	globaldef	has_SIMD2_nasm

        segment_code

testCPUID:
	pushfd	                        
	pop	eax
	mov	ecx,eax
	xor	eax,0x200000
	push	eax
	popfd
	pushfd
	pop	eax
	cmp	eax,ecx
	ret

;---------------------------------------
;	int  has_MMX_nasm (void)
;---------------------------------------

has_MMX_nasm:
        pushad
	call	testCPUID
	jz	return0		; no CPUID command, so no MMX

	mov	eax,0x1
	CPUID
	test	edx,0x800000
	jz	return0		; no MMX support
	jmp	return1		; MMX support
        
;---------------------------------------
;	int  has_SIMD_nasm (void)
;---------------------------------------

has_SIMD_nasm:
        pushad
	call	testCPUID
	jz	return0		; no CPUID command, so no SIMD
        
	mov	eax,0x1
	CPUID
	test	edx,0x02000000
	jz	return0		; no SIMD support
	jmp	return1		; SIMD support
        
;---------------------------------------
;	int  has_SIMD2_nasm (void)
;---------------------------------------

has_SIMD2_nasm:
        pushad
	call	testCPUID
	jz	return0		; no CPUID command, so no SIMD2
        
	mov	eax,0x1
	CPUID
	test	edx,0x04000000
	jz	return0		; no SIMD2 support
	jmp	return1		; SIMD2 support
        
;---------------------------------------
;	int  has_3DNow_nasm (void)
;---------------------------------------

has_3DNow_nasm:
        pushad
	call	testCPUID
	jz	return0		; no CPUID command, so no 3DNow!

	mov	eax,0x80000000
	CPUID
	cmp	eax,0x80000000
	jbe	return0		; no extended MSR(1), so no 3DNow!

	mov	eax,0x80000001
	CPUID
	test	edx,0x80000000
	jz	return0		; no 3DNow! support
				; 3DNow! support
return1:
	popad
	xor	eax,eax
	inc	eax
	ret

return0:
	popad
	xor	eax,eax
	ret
        
        end
