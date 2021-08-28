;############################################################################
;##                                                                        ##
;##  MSSA32.ASM                                                            ##
;##                                                                        ##
;##  Miles Sound System                                                    ##
;##                                                                        ##
;##  Digital Sound 32-bit flat-model assembly API functions                ##
;##                                                                        ##
;##  Version 2.00 of 11-Jun-98: Initial, derived from DLS1A32.ASM V1.00    ##
;##                                                                        ##
;##  80386 ASM source compatible with Microsoft Assembler v6.12 or later   ##
;##                                                                        ##
;##  Author: John Miles, Jeff Roberts                                      ##
;##                                                                        ##
;############################################################################
;##                                                                        ##
;##  Copyright (C) RAD Game Tools, Inc.                                    ##
;##                                                                        ##
;##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
;##                                                                        ##
;############################################################################

                OPTION SCOPED           ;Enable local labels
                .586                    ;Enable Pentium instructions
                .MMX                    ;Enable MMX instructions

                IFDEF DPMI
                  .MODEL FLAT,C
                ELSE
                  .MODEL FLAT,STDCALL   ;(RIB functions use __stdcall in Win32)
                ENDIF

                .DATA

data_start      LABEL BYTE

DIG_F_16BITS_MASK equ 1                 ;Copy options passed from caller

M_DEST_STEREO   equ 1                   ;Mixer options passed from caller
M_SRC_16        equ 2
M_FILTER        equ 4
M_SRC_STEREO    equ 8
M_VOL_SCALING   equ 16
M_RESAMPLE      equ 32
M_ORDER         equ 64

MOP_SRC_STEREO  equ 1                   ;Mixer options built at assembly time
MOP_VOL_SCALING equ 2
MOP_RESAMPLE    equ 4
MOP_ORDER       equ 8

step_fract      dd ?
step_whole      dd 2 dup (?)

operation       dd ?
scale_left      dd ?
scale_right     dd ?
src_end         dd ?
dest_end        dd ?
playback_ratio  dd ?

divider_l       dd ?
divider_r       dd ?
cur_l           dd ?
cur_r           dd ?
cur_r2          dd ?
src_end_early   dd ?

cnt             dd ?

                ;
                ;Table of reciprocals of integers 0-255, in 1:16 format
                ;

                INCLUDE mssadp.inc

ADPCMDATA    STRUC
  blocksize    dd ?
  extrasamples db ?,?,?,?
  blockleft    dd ?
  adpstep      dd ?
  savesrc      dd ?
  lsample      dd ? ; the rest of the structure also doubles as a decomp buffer
  destend      dd ?
  srcend       dd ?
  samplesL     dd ?
  samplesR     dd ?
  moresamples  dw 16 dup (?)
ADPCMDATA    ENDS

data_end        LABEL BYTE

                .CODE MSSMIXER

                ;
                ;RIB-exported functions
                ;

                PUBLIC MSS_mixer_startup
                PUBLIC MSS_mixer_shutdown
                PUBLIC MSS_mixer_flush
                PUBLIC MSS_mixer_merge
                PUBLIC MSS_mixer_copy

                ;
                ;Utility functions (called directly)
                ;

                PUBLIC MSS_MMX_available

                PUBLIC DecodeADPCM_STEREO
                PUBLIC DecodeADPCM_MONO
                PUBLIC DecodeADPCM_MONO_8

                IFDEF DPMI
                  PUBLIC MSSA32_VMM_lock
                ENDIF

code_start      LABEL BYTE


include ..\..\src\shared\mssmixer.asm

;#############################################################################
;##                                                                         ##
;## Return 1 if MMX available, else 0                                       ##
;##                                                                         ##
;#############################################################################

MSS_MMX_available PROC \
                USES ebx esi edi ds es

                ;
                ;If bit 21 of the eflags register is changeable, the CPU
                ;supports the "CPUID" instruction -- this includes all P5's,
                ;P6's, some 486s, and all future CPUs
                ;

                pushfd
                pop eax                 ;EAX = original eflags word

                mov ebx,eax             ;save it in EBX

                xor eax,1 SHL 21        ;toggle bit 21
                push eax
                popfd

                pushfd                  ;examine eflags again
                pop eax

                push ebx                ;restore original eflags value
                popfd

                cmp eax,ebx             ;bit changed?
                je __no_MMX             ;no CPUID, ergo no MMX

                mov eax,1               ;else OK to perform CPUID
                cpuid                   
                test edx,00800000h      ;test bit 23
                jz __no_MMX

                mov eax,1
                ret

__no_MMX:       mov eax,0
                ret

MSS_MMX_available ENDP

;#############################################################################
;##                                                                         ##
;## Start up mixer services                                                 ##
;##                                                                         ##
;#############################################################################

MSS_mixer_startup PROC \
                USES ebx esi edi ds es

                mov eax,0
                ret

MSS_mixer_startup ENDP

;#############################################################################
;##                                                                         ##
;## Shut down mixer services                                                ##
;##                                                                         ##
;#############################################################################

MSS_mixer_shutdown PROC \
                USES ebx esi edi ds es

                mov eax,0
                ret

MSS_mixer_shutdown ENDP

;#############################################################################
;##                                                                         ##
;## Flush build buffer with zero-amplitude DC baseline signal               ##
;##                                                                         ##
;#############################################################################

MSS_mixer_flush PROC \
                USES ebx esi edi ds es \
                lpDest:PTR,\
                nLen:DWORD,\
                lpReverbBuff:PTR,\
                dwReverbLevel:DWORD,\
                bMMXAvailable:DWORD

                cld

                push ds
                pop es 

                mov edi,[lpDest]
                mov ecx,[nLen]

                cmp dwReverbLevel,0
                jne __apply_reverb

                mov eax,0

                push ecx
                and ecx,3
                rep stosb
                pop ecx
                shr ecx,2
                rep stosd

                ret

__apply_reverb: shr ecx,2
                mov esi,[lpReverbBuff]

__rev_loop:     mov eax,[esi]
                imul dwReverbLevel
                shrd eax,edx,7
                mov [edi],eax

                add esi,4
                add edi,4

                dec ecx
                jnz __rev_loop

                ret

MSS_mixer_flush ENDP

;#############################################################################
;##                                                                         ##
;## Mix data from PCM source into 32-bit signed build buffer                ##
;##                                                                         ##
;#############################################################################

MSS_mixer_merge PROC \
                USES ebx esi edi ds es \
                lplpSrc:PTR,\
                lpSrcFract:PTR,\
                lpSrcEnd:PTR,\
                lplpDest:PTR,\
                lpDestEnd:PTR,\
                lpLeftVal:PTR,\
                lpRightVal:PTR,\
                nPlaybackRatio:DWORD,\
                nScaleLeft:DWORD,\
                nScaleRight:DWORD,\
                nOperation:DWORD,\
                bMMXAvailable:DWORD

                ;
                ;Save static values -- this routine is used as both the DLS
                ;and main mixers in Win32, and reentry is possible
                ;

                push step_fract
                push step_whole[0]
                push step_whole[1]
                push operation
                push scale_left
                push scale_right
                push src_end
                push dest_end
                push playback_ratio
                push cur_l
                push cur_r
                push cnt

                ;
                ;Configure step tables for frequency resampling, and adjust
                ;for sample size in bytes
                ;

                mov eax,[nPlaybackRatio]
                mov ebx,eax
                shr ebx,16      ;EBX = whole part
                shl eax,16      ;EAX = fract part

                mov step_fract,eax

                mov eax,ebx     ;EAX = base step when fract does not overflow
                inc ebx         ;EBX = base+1 step when fract overflows

                test [nOperation],M_SRC_STEREO
                jz __chk_16_bits

                shl eax,1
                shl ebx,1

__chk_16_bits:  test [nOperation],M_SRC_16
                jz __write_step

                shl eax,1       ;multiply whole step size by # of bytes/sample
                shl ebx,1

__write_step:   mov step_whole[0*4],ebx
                mov step_whole[1*4],eax

                ;
                ;Register usage throughout all loops:
                ;
                ;EAX->scratch
                ;EBX->scratch
                ;ECX->dest end (or scratch)
                ;EDX->src fraction (or scratch)
                ;ESI->src
                ;EDI->dest
                ;
                ;Non-filtered and downsample-filtered loops maintain the
                ;source address fraction at 0:32-bit precision.  Upsample-
                ;filtered loops use 16:16-bit precision, to avoid processing
                ;skipped samples twice.  The fractional component is always
                ;returned to the caller at 16:16 precision to allow smooth
                ;transitions between the two loop types.
                ;

                mov ebx,[lpSrcFract]
                mov edx,[ebx]

__load_src:     mov ebx,[lpLeftVal]
                mov eax,[ebx]
                mov cur_l,eax

                mov ebx,[lpRightVal]
                mov eax,[ebx]
                mov cur_r,eax

                mov ebx,[lplpSrc]
                mov esi,[ebx]
                cmp esi,[lpSrcEnd]
                mov edi,0
                jae __exit

                mov ebx,[lplpDest]
                mov edi,[ebx]
                mov ecx,[lpDestEnd]
                cmp edi,ecx
                jae __exit

                ;
                ;Make parameters accessible to macros
                ;

                mov eax,[nScaleLeft]
                mov ebx,[nScaleRight]
                mov scale_left,eax
                mov scale_right,ebx

                mov eax,[lpSrcEnd]
                mov ebx,[lpDestEnd]
                mov src_end,eax
                mov dest_end,ebx

                mov eax,[nPlaybackRatio]
                mov playback_ratio,eax

                mov eax,[nOperation]
                mov operation,eax

                ;
                ;Call merge handler
                ;

                call cs:vector_table[eax * SIZE DWORD]

                ;
                ;Update application src/dest pointers before returning
                ;

__exit:         mov ebx,[lplpSrc]
                mov [ebx],esi

                mov ebx,[lplpDest]
                mov [ebx],edi

                mov ebx,[lpSrcFract]
                mov [ebx],edx

                mov eax,cur_l
                mov ebx,[lpLeftVal]
                mov [ebx],eax

                mov eax,cur_r
                mov ebx,[lpRightVal]
                mov [ebx],eax

                pop cnt
                pop cur_r
                pop cur_l
                pop playback_ratio
                pop dest_end
                pop src_end
                pop scale_right
                pop scale_left
                pop operation
                pop step_whole[1]
                pop step_whole[0]
                pop step_fract

                ret

MSS_mixer_merge ENDP

;#############################################################################
;##                                                                         ##
;## Clip and copy build buffer data to output sample buffer                 ##
;##                                                                         ##
;#############################################################################

MSS_mixer_copy  PROC \
                USES ebx esi edi ds es \
                lpSrc:PTR,\
                nSrcLen:DWORD,\
                lpDest:PTR,\
                nOperation:DWORD,\
                bMMXAvailable:DWORD

                ;
                ;ESI->src
                ;EDI->dest
                ;ECX->src end
                ;

                mov esi,[lpSrc]
                mov edi,[lpDest]
                mov ecx,[nSrcLen]
                cmp ecx,0
                je __exit
                add ecx,esi

                ;
                ;If MMX requested, do as many blocks as possible
                ;before falling through to x86 remnant case
                ;

                cmp [bMMXAvailable],0
                je __x86

                mov eax,[nSrcLen]   ;get # of source bytes
                shr eax,4           ;divide by 128-bit MMX block size
                jz __x86            ;if < 1 block, fall through to remnant

                test [nOperation],DIG_F_16BITS_MASK
                jz __do_8_MMX

                ;
                ;MMX clip and copy to 16-bit stream
                ;

__do_16_MMX:    movq mm0,[esi]      ;read first 2 source DWORDs
                psrad mm0,11        ;SAR DWORD pair by 11

                movq mm1,[esi+8]    ;read next 2 source DWORDs
                psrad mm1,11        ;SAR DWORD pair by 11

                packssdw mm0,mm1    ;merge DWORDs into 4 WORDs w/saturation
                movq [edi],mm0      ;write 4 samples to output stream

                add edi,8
                add esi,16
                dec eax
                jnz __do_16_MMX

                emms                ;exit MMX mode
                jmp __x86

                ;
                ;MMX clip and copy to 8-bit stream
                ;

__do_8_MMX:     mov ebx,80808080h   ;prepare XOR constant for 
                movd mm2,ebx        ;signed-to-unsigned conversion

__8_MMX_loop:   movq mm0,[esi]      ;read first 2 source DWORDs
                psrad mm0,11        ;SAR DWORD pair by 11

                movq mm1,[esi+8]    ;read next 2 source DWORDs
                psrad mm1,11        ;SAR DWORD pair by 11

                packssdw mm0,mm1    ;merge DWORDs into 4 WORDs w/saturation

                psraw mm0,8         ;convert to bytes
                packsswb mm0,mm0

                pxor mm0,mm2        ;convert to unsigned bytes
                movd [edi],mm0      ;write 4 samples to output stream

                add edi,4
                add esi,16
                dec eax
                jnz __8_MMX_loop

                emms                ;exit MMX mode

                ;
                ;Non-MMX version also handles remnant case
                ;

__x86:          cmp esi,ecx
                jae __exit
                
                test [nOperation],DIG_F_16BITS_MASK
                jz __copy_8

                ;
                ;X86 clip and copy to 16-bit stream
                ;

__copy_16:      mov eax,[esi]

                add edi,2
                sar eax,11
                add esi,4

                cmp eax,32767
                jg __clip_H16
                cmp eax,-32768
                jl __clip_L16

                mov [edi-2],ax
                cmp esi,ecx
                jb __copy_16
                jmp __exit

__clip_H16:     mov WORD PTR [edi-2],32767
                cmp esi,ecx
                jb __copy_16
                jmp __exit

__clip_L16:     mov WORD PTR [edi-2],-32768
                cmp esi,ecx
                jb __copy_16
                jmp __exit

                ;
                ;X86 clip and copy to 8-bit stream
                ;

__copy_8:       mov eax,[esi]

                inc edi
                sar eax,11
                add esi,4

                cmp eax,32767
                jg __clip_H8
                cmp eax,-32768
                jl __clip_L8

                xor eax,8000h

                mov [edi-1],ah
                cmp esi,ecx
                jb __copy_8
                jmp __exit

__clip_H8:      mov BYTE PTR [edi-1],0ffh
                cmp esi,ecx
                jb __copy_8
                jmp __exit

__clip_L8:      mov BYTE PTR [edi-1],0
                cmp esi,ecx
                jb __copy_8
                jmp __exit

__exit:         ret

MSS_mixer_copy  ENDP

;#############################################################################
DECODE_ADPCM	MACRO Flg

IF (Flg AND 2)
DecodeADPCM_MONO_8 PROC C
ELSE
IF (Flg AND 1)
DecodeADPCM_STEREO PROC C
ELSE
DecodeADPCM_MONO PROC C
ENDIF
ENDIF

destaddrptr equ ebp+28+0
srcaddrptr  equ ebp+28+4
destlen     equ ebp+28+8
srclen      equ ebp+28+12
adpcmptr    equ ebp+28+16

  push ebx
  push ecx
  push edx
  push esi
  push edi
  push ebp

  mov ebp,esp
  mov edi,[destaddrptr]
  mov ebx,[srcaddrptr]
  mov edi,[edi]
  mov ebx,[ebx]
  mov esi,edi
  mov ecx,ebx
  add esi,[destlen]
  add ecx,[srclen]
  mov ebp,[adpcmptr]
  mov [ebp.ADPCMDATA.destend],esi
  mov [ebp.ADPCMDATA.srcend],ecx
  
  cmp [ebp.ADPCMDATA.extrasamples],0
  ja  __doextrasamps

  cmp [ebp.ADPCMDATA.blockleft],0
  ja __doadword

__startblock:
  IF (Flg AND 1)

  mov eax,[ebx] ;get the first sample and step size
  mov esi,[ebp.ADPCMDATA.blocksize]
  ror eax,16
  sub esi,8

  mov edx,[ebx+4]
  mov [ebp.ADPCMDATA.blockleft],esi
  xchg ax,dx

  ror eax,16
  add ebx,8

  shl edx,8
  mov [edi],eax

  add edi,4

  ELSE

  IF (Flg AND 2)

  mov edx,[ebx] ;get the first sample and step size
  mov eax,edx
  mov esi,[ebp.ADPCMDATA.blocksize]
  add dh,080h
  add ebx,4
  mov [edi],dh
  sub esi,4
  inc edi
  shr edx,8
  mov [ebp.ADPCMDATA.blockleft],esi
  and edx,0ffffh

  ELSE

  mov edx,[ebx] ;get the first sample and step size
  mov eax,edx
  mov esi,[ebp.ADPCMDATA.blocksize]
  mov [edi],dx
  add ebx,4
  shr edx,8
  sub esi,4
  add edi,2
  mov [ebp.ADPCMDATA.blockleft],esi
  and edx,0ffffh

  ENDIF
  ENDIF

__contsamps:
  cmp ebx,[ebp.ADPCMDATA.srcend]
  jae __srcend

__donextdword:

  mov ecx,[ebx]

  IF (Flg AND 1)
  add ebx,8
  mov dl,cl
  mov [ebp.ADPCMDATA.savesrc],ebx
  shl dl,4
  mov ebx,[ebx-4]
  sub [ebp.ADPCMDATA.blockleft],8
  ror edx,16
  mov [ebp.ADPCMDATA.extrasamples],8
  mov dl,bl
  shl dl,4
  ror edx,20
  ELSE
  mov dl,cl
  add ebx,4
  mov [ebp.ADPCMDATA.extrasamples],8
  shl dl,4
  sub [ebp.ADPCMDATA.blockleft],4
  ENDIF

__expanddword:

  cmp edi,[ebp.ADPCMDATA.destend]
  jae __destend

  IF (Flg AND 1)
  mov esi,0fffh
  and esi,edx
  mov dl,cl
  mov dh,newadpcm[esi]
  test cl,8
  mov si,stepw[esi*2]
  jnz __minusL

  add si,ax
  cmp si,ax
  jge __handleL

  mov ax,32767
  jmp __fixedsampleL

__minusL:
  add si,ax
  cmp si,ax
  jg __toolowL

__handleL:
  mov ax,si
__fixedsampleL:
  ror edx,16
  mov esi,0fffh
  ror eax,16
  and esi,edx
  mov dl,bl
  mov dh,newadpcm[esi]
  test bl,8
  mov si,stepw[esi*2]
  jnz __minusR

  add si,ax
  cmp si,ax
  jge __handleR

  mov ax,32767
  jmp __fixedsampleR

__minusR:
  add si,ax
  cmp si,ax
  jg __toolowR

__handleR:
  mov ax,si
__fixedsampleR:
  ror edx,20
  ror eax,16
  shr ecx,4
  mov [edi],eax

  shr ebx,4
  add edi,4

  ELSE

  shr edx,4

  test cl,8
  mov esi,edx
  movzx edx,cl
  mov dh,newadpcm[esi]
  movzx esi,stepw[esi*2]
  jnz __minus

  add si,ax
  cmp si,ax
  jge __handle
  mov ax,32767
  jmp __fixedsample

__minus:
  add si,ax
  cmp si,ax
  jg __toolow

__handle:
  mov ax,si
__fixedsample:
  IF (Flg AND 2)

  ;8-bit mono
  add ah,080h
  shr ecx,4
  mov [edi],ah
  inc edi
  sub ah,080h

  ELSE

  ;16-bit mono
  shr ecx,4
  mov [edi],ax
  add edi,2

  ENDIF

  ENDIF

  dec [ebp.ADPCMDATA.extrasamples]
  jnz __expanddword

  IF (Flg AND 1)
  mov ebx,[ebp.ADPCMDATA.savesrc]
  ENDIF

  cmp edi,[ebp.ADPCMDATA.destend]
  jae __destendloop

  IF (Flg AND 1)
  shl edx,4
  ENDIF

  cmp [ebp.ADPCMDATA.blockleft],0
  jne __contsamps

  cmp ebx,[ebp.ADPCMDATA.srcend]
  jb __startblock

__destendtop:
__srcend:
  mov [ebp.ADPCMDATA.extrasamples],0
  mov [ebp.ADPCMDATA.adpstep],edx
  jmp __done

__destend:
  mov [ebp.ADPCMDATA.samplesL],ecx
  IF (Flg AND 1)
  mov [ebp.ADPCMDATA.samplesR],ebx
  mov ebx,[ebp.ADPCMDATA.savesrc]
  ENDIF

__destendloop:
  mov [ebp.ADPCMDATA.lsample],eax
  mov [ebp.ADPCMDATA.adpstep],edx

__done:
  mov ebp,esp
  mov esi,[destaddrptr]
  mov ecx,[srcaddrptr]
  mov [esi],edi
  mov [ecx],ebx

  pop ebp
  pop edi
  pop esi
  pop edx
  pop ecx
  pop ebx
  ret

  IF (Flg AND 1)

__toolowL:
  mov ax,-32768
  jmp __fixedsampleL

__toolowR:
  mov ax,-32768
  jmp __fixedsampleR

  ELSE

__toolow:
  mov ax,-32768
  jmp __fixedsample

  ENDIF

__doadword:
  mov edx,[ebp.ADPCMDATA.adpstep]
  IF (Flg AND 1)
  shl edx,4
  ENDIF

  mov eax,[ebp.ADPCMDATA.lsample]
  jmp __donextdword

__doextrasamps:

  mov ecx,[ebp.ADPCMDATA.samplesL]
  IF (Flg AND 1)
  mov [ebp.ADPCMDATA.savesrc],ebx
  mov ebx,[ebp.ADPCMDATA.samplesR]
  ENDIF

  mov eax,[ebp.ADPCMDATA.lsample]
  mov edx,[ebp.ADPCMDATA.adpstep]
  jmp __expanddword

IF (Flg AND 2)
DecodeADPCM_MONO_8 ENDP
ELSE
IF (Flg AND 1)
DecodeADPCM_STEREO ENDP
ELSE
DecodeADPCM_MONO ENDP
ENDIF
ENDIF

                ENDM

;#############################################################################
DECODE_ADPCM 0	; Mono adpcm decode

DECODE_ADPCM 1	; Stereo adpcm decode

DECODE_ADPCM 2	; Mono, 8-bit adpcm decode


db 13,10,13,10
db 'Miles Sound System',13,10
db 'Copyright (C) 1991-98 RAD Game Tools, Inc.',13,10,13,10

                ;
                ;End of locked code
                ;

code_end        LABEL BYTE

;#############################################################################
;##                                                                         ##
;## Lock all code and data in AILSSA module                                 ##
;##                                                                         ##
;#############################################################################

                IFDEF DPMI

AIL_vmm_lock_range PROTO C,P1:NEAR PTR,P2:NEAR PTR

MSSA32_VMM_lock PROC C \
                USES ebx esi edi ds es

                invoke AIL_vmm_lock_range,OFFSET data_start,OFFSET data_end
                invoke AIL_vmm_lock_range,OFFSET code_start,OFFSET code_end

                ret

MSSA32_VMM_lock ENDP

                ENDIF

                END


