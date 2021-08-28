;############################################################################
;##                                                                        ##
;##  MSSA16.ASM                                                            ##
;##                                                                        ##
;##  Miles Sound System                                                    ##
;##                                                                        ##
;##  Digital Sound 16-bit protected-mode assembly API functions            ##
;##                                                                        ##
;##  Version 2.00 of 14-Aug-98: Initial, derived from MSSA32.ASM V2.00     ##
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

                ;
                ;Macros, internal equates
                ;

?PLM            = 1
?WIN            = 1
?MEDIUM         = 1

                INCLUDE SSmacros.inc     ;Windows C interface macros (modified by jkr)

                ;
                ;Internal data
                ;

                extern __AHINCR:FAR

DGROUP group _DATA1
_DATA1 segment para   public  'DATA'

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

cur_l           dd ?
cur_r           dd ?
cnt             dd ?

                ;
                ;Table of reciprocals of integers 0-255, in 1:16 format
                ;

recip_table     LABEL DWORD
                dd 65535                ;(should be 65536, but MASM bug causes
i               = 1                     ;truncation of DWORD divisor in 
                REPT 255                ;16-bit mode)
                dd 65535 / i
i               = i+1
                ENDM

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

callbackofs    dd ?           ; callback information
callbackds     dw ?,?

VTDaddr        dd 0

                PUBLIC DECODEADPCM_STEREO
                PUBLIC DECODEADPCM_MONO
                PUBLIC DECODEADPCM_MONO_8

_DATA1          ENDS

sBegin          CODE

                assumes CS,CODE
                assumes DS,_DATA1
                assumes ES,NOTHING

                .586
                .MMX

;#############################################################################
;##                                                                         ##
;## Utility macros                                                          ##
;##                                                                         ##
;#############################################################################

SAVE_REGS       MACRO
                push ebp
                push esi
                push edi
                push ebx
                push ds
                push es
                push fs
                push gs
                ENDM

RESTORE_REGS    MACRO
                pop gs
                pop fs
                pop es
                pop ds
                pop ebx
                pop edi
                pop esi
                pop ebp
                ENDM

;#############################################################################
;##                                                                         ##
;## Macro used to construct MSS_mixer_merge() handlers                      ##
;##                                                                         ##
;#############################################################################

MAKE_MERGE      MACRO Flg
MERGE_&Flg&     PROC

                push ebp

                mov ebx,operation
                and ebx,7
                jmp cs:__loop_vectors[ebx * SIZE WORD]

__loop_vectors: dw __0_loop, __1_loop, __2_loop, __3_loop
                dw __4_loop, __5_loop, __6_loop, __7_loop

                ;----------------------------------
                ;DEST_MONO + SRC_8
                ;----------------------------------

__0_loop:
                IF (OP AND MOP_SRC_STEREO)
                
                  xor eax,eax
                  xor ebx,ebx
                  mov ah,fs:[esi]
                  mov bh,fs:[esi+1]
                  xor eax,8000h
                  xor ebx,8000h
                  movsx ebx,bx
                  movsx eax,ax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  add ebx,eax

                ELSE

                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  movsx ebx,ax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul ebx,scale_left

                  ELSE
                  
                    shl ebx,11

                  ENDIF

                ENDIF

                add es:[edi],ebx

                IF (OP AND MOP_RESAMPLE)

                  add edx,step_fract

                  IF (OP AND MOP_SRC_STEREO)

                    sbb eax,eax
                    lea edi,[edi+4]
                    add esi,step_whole[4+eax*4]

                  ELSE

                    lea edi,[edi+4]
                    adc esi,step_whole[4]

                  ENDIF

                ELSE

                  lea edi,[edi+4]

                  IF (OP AND MOP_SRC_STEREO)

                    add esi,2

                  ELSE

                    inc esi

                  ENDIF

                ENDIF

                cmp edi,ecx
                jae __exit
                cmp esi,src_end
                jb __0_loop
                jmp __exit

                ;----------------------------------
                ;DEST_STEREO + SRC_8
                ;----------------------------------

__1_loop:       
                IF (OP AND MOP_SRC_STEREO)

                  xor eax,eax
                  xor ebx,ebx
                  mov ah,fs:[esi]
                  mov bh,fs:[esi+1]
                  xor eax,8000h
                  xor ebx,8000h
                  movsx eax,ax
                  movsx ebx,bx

                ELSE

                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h

                  movsx ebx,ax
                  mov eax,ebx

                ENDIF

                IF (OP AND MOP_VOL_SCALING)
                
                  imul eax,scale_left
                  imul ebx,scale_right

                ELSE

                  shl eax,11
                  shl ebx,11
                    
                ENDIF

                IF (OP AND MOP_ORDER)

                  add es:[edi],ebx
                  add es:[edi+4],eax

                ELSE

                  add es:[edi],eax
                  add es:[edi+4],ebx

                ENDIF

                IF (OP AND MOP_RESAMPLE)

                  add edx,step_fract

                  IF (OP AND MOP_SRC_STEREO)

                    sbb eax,eax
                    lea edi,[edi+8]
                    add esi,step_whole[4+eax*4]

                  ELSE

                    lea edi,[edi+8]
                    adc esi,step_whole[4]

                  ENDIF

                ELSE

                  lea edi,[edi+8]

                  IF (OP AND MOP_SRC_STEREO)

                    add esi,2

                  ELSE

                    inc esi

                  ENDIF

                ENDIF

                cmp edi,ecx
                jae __exit
                cmp esi,src_end
                jb __1_loop
                jmp __exit

                ;----------------------------------
                ;DEST_MONO + SRC_16
                ;----------------------------------

__2_loop:       
                IF (OP AND MOP_SRC_STEREO)

                  movsx eax,WORD PTR fs:[esi]
                  movsx ebx,WORD PTR fs:[esi+2]

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  add eax,ebx

                ELSE
                
                  movsx eax,WORD PTR fs:[esi]

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left

                  ELSE
                  
                    shl eax,11
                    
                  ENDIF

                ENDIF

                add es:[edi],eax

                IF (OP AND MOP_RESAMPLE)

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+4]
                  add esi,step_whole[4+eax*4]

                ELSE

                  lea edi,[edi+4]

                  IF (OP AND MOP_SRC_STEREO)

                    add esi,4

                  ELSE

                    add esi,2

                  ENDIF

                ENDIF

                cmp edi,ecx
                jae __exit
                cmp esi,src_end
                jb __2_loop
                jmp __exit

                ;----------------------------------
                ;DEST_STEREO + SRC_16
                ;----------------------------------

__3_loop:       
                IF (OP AND MOP_SRC_STEREO)

                  movsx eax,WORD PTR fs:[esi]
                  movsx ebx,WORD PTR fs:[esi+2]

                ELSE

                  movsx eax,WORD PTR fs:[esi]
                  mov ebx,eax

                ENDIF

                IF (OP AND MOP_VOL_SCALING)
                
                  imul eax,scale_left
                  imul ebx,scale_right

                ELSE

                  shl eax,11
                  shl ebx,11
                    
                ENDIF

                IF (OP AND MOP_ORDER)

                  add es:[edi],ebx
                  add es:[edi+4],eax

                ELSE

                  add es:[edi],eax
                  add es:[edi+4],ebx

                ENDIF

                IF (OP AND MOP_RESAMPLE)

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+8]
                  add esi,step_whole[4+eax*4]

                ELSE

                  lea edi,[edi+8]

                  IF (OP AND MOP_SRC_STEREO)

                    add esi,4

                  ELSE

                    add esi,2

                  ENDIF

                ENDIF

                cmp edi,ecx
                jae __exit
                cmp esi,src_end
                jb __3_loop
                jmp __exit

                ;----------------------------------
                ;DEST_MONO + SRC_8 + FILTER
                ;----------------------------------

                IF (OP AND MOP_SRC_STEREO)

__4_loop:         cmp playback_ratio,10000h
                  jle __4_rep

                  mov ecx,edx                     ;Pitch up: drop samples
                            
                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  movsx edx,ax

                  mov ah,fs:[esi+1]
                  xor eax,8000h
                  movsx ebp,ax

                  mov ebx,1

__4_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  mov eax,ebp
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_r,eax

                  mov edx,eax
                  mov eax,cur_l

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul edx,scale_right

                  ELSE

                    shl eax,11
                    shl edx,11
                    
                  ENDIF

                  add eax,edx
                  add es:[edi],eax

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx
                  xor ebp,ebp

                  lea edi,[edi+4]

__4_skip:         cmp ecx,65536
                  jl __4_drop

                  sub ecx,65536

                  add esi,2
                  cmp esi,src_end
                  jae __exit

                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde
                  add edx,eax

                  mov ah,fs:[esi+1]
                  xor eax,8000h
                  cwde
                  add ebp,eax

                  inc ebx
                  jmp __4_skip

__4_rep:          xor eax,eax                     ;Pitch down: replicate samples
                  xor ebx,ebx
                  mov ah,fs:[esi]
                  mov bh,fs:[esi+1]
                  xor eax,8000h
                  xor ebx,8000h
                  movsx eax,ax
                  movsx ebx,bx

                  mov ecx,cur_l
                  mov ebp,cur_r
                  mov cur_l,eax
                  mov cur_r,ebx

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  push eax
                  push ebx

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  pop ebx
                  pop ecx

                  imul ebx,ebp
                  imul ecx,cur_r
                  sar ebx,16
                  sar ecx,16
                  add ebx,ecx

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  add eax,ebx
                  add es:[edi],eax

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+4]
                  add esi,step_whole[4+eax*4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __4_rep
                  jmp __exit

                ELSE

__4_loop:         cmp playback_ratio,10000h
                  jle __4_rep

                  mov ecx,edx                     ;Pitch up: drop samples
                            
                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  movsx edx,ax
                  mov ebx,1

__4_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left

                  ELSE

                    shl eax,11
                    
                  ENDIF

                  add es:[edi],eax

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx

                  lea edi,[edi+4]

__4_skip:         cmp ecx,65536
                  jl __4_drop

                  sub ecx,65536

                  inc esi
                  cmp esi,src_end
                  jae __exit

                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde

                  add edx,eax
                  inc ebx
                  jmp __4_skip

__4_rep:          xor eax,eax                     ;Pitch down: replicate samples
                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde

                  mov ecx,cur_l
                  mov cur_l,eax

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left

                  ELSE

                    shl eax,11
                    
                  ENDIF

                  add es:[edi],eax

                  add edx,step_fract
                  lea edi,[edi+4]
                  adc esi,step_whole[4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __4_rep
                  jmp __exit

                ENDIF

                ;----------------------------------
                ;DEST_STEREO + SRC_8 + FILTER
                ;----------------------------------

                IF (OP AND MOP_SRC_STEREO)

__5_loop:         cmp playback_ratio,10000h
                  jle __5_rep

                  mov ecx,edx                     ;Pitch up: drop samples
                                                  
                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  movsx edx,ax

                  mov ah,fs:[esi+1]
                  xor eax,8000h
                  movsx ebp,ax

                  mov ebx,1

__5_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  mov eax,ebp
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_r,eax

                  mov edx,eax
                  mov eax,cur_l

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul edx,scale_right

                  ELSE

                    shl eax,11
                    shl edx,11
                    
                  ENDIF

                  IF (OP AND MOP_ORDER)

                    add es:[edi],edx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],edx

                  ENDIF

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx
                  xor ebp,ebp

                  lea edi,[edi+8]

__5_skip:         cmp ecx,65536
                  jl __5_drop

                  sub ecx,65536

                  add esi,2
                  cmp esi,src_end
                  jae __exit

                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde
                  add edx,eax

                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde
                  add ebp,eax

                  inc ebx
                  jmp __5_skip

__5_rep:          xor eax,eax                     ;Pitch down: replicate samples
                  xor ebx,ebx
                  mov ah,fs:[esi]
                  mov bh,fs:[esi+1]
                  xor eax,8000h
                  xor ebx,8000h
                  movsx eax,ax
                  movsx ebx,bx

                  mov ecx,cur_l
                  mov ebp,cur_r
                  mov cur_l,eax
                  mov cur_r,ebx

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  push eax
                  push ebx

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  pop ebx
                  pop ecx

                  imul ebx,ebp
                  imul ecx,cur_r
                  sar ebx,16
                  sar ecx,16
                  add ebx,ecx

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  IF (OP AND MOP_ORDER)

                    add es:[edi],ebx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],ebx

                  ENDIF

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+8]
                  add esi,step_whole[4+eax*4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __5_rep
                  jmp __exit

                ELSE

__5_loop:         cmp playback_ratio,10000h
                  jle __5_rep

                  mov ecx,edx                     ;Pitch up: drop samples
                                                  
                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  movsx edx,ax
                  mov ebx,1

__5_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  mov edx,eax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul edx,scale_right

                  ELSE

                    shl eax,11
                    shl edx,11
                    
                  ENDIF

                  IF (OP AND MOP_ORDER)

                    add es:[edi],edx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],edx

                  ENDIF

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx

                  lea edi,[edi+8]

__5_skip:         cmp ecx,65536
                  jl __5_drop

                  sub ecx,65536

                  inc esi
                  cmp esi,src_end
                  jae __exit

                  xor eax,eax
                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde

                  add edx,eax
                  inc ebx
                  jmp __5_skip

__5_rep:          xor eax,eax                     ;Pitch down: replicate samples
                  mov ah,fs:[esi]
                  xor eax,8000h
                  cwde

                  mov ecx,cur_l
                  mov cur_l,eax

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  mov ebx,eax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  IF (OP AND MOP_ORDER)

                    add es:[edi],ebx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],ebx

                  ENDIF

                  add edx,step_fract
                  lea edi,[edi+8]
                  adc esi,step_whole[4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __5_rep
                  jmp __exit

                ENDIF

                ;----------------------------------
                ;DEST_MONO + SRC_16 + FILTER
                ;----------------------------------

                IF (OP AND MOP_SRC_STEREO)

__6_loop:         cmp playback_ratio,10000h
                  jle __6_rep

                  mov ecx,edx                     ;Pitch up: drop samples
                                                  
                  movsx edx,WORD PTR fs:[esi]
                  movsx ebp,WORD PTR fs:[esi+2]
                  mov ebx,1

__6_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  mov eax,ebp
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_r,eax

                  mov edx,eax
                  mov eax,cur_l

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul edx,scale_right

                  ELSE

                    shl eax,11
                    shl edx,11
                    
                  ENDIF

                  add eax,edx
                  add es:[edi],eax

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx
                  xor ebp,ebp

                  lea edi,[edi+4]

__6_skip:         cmp ecx,65536
                  jl __6_drop

                  sub ecx,65536

                  add esi,4
                  cmp esi,src_end
                  jae __exit

                  movsx eax,WORD PTR fs:[esi]
                  add edx,eax

                  movsx eax,WORD PTR fs:[esi+2]
                  add ebp,eax

                  inc ebx
                  jmp __6_skip

__6_rep:          movsx eax,WORD PTR fs:[esi]        ;Pitch down: replicate samples
                  movsx ebx,WORD PTR fs:[esi+2]

                  mov ecx,cur_l
                  mov ebp,cur_r
                  mov cur_l,eax
                  mov cur_r,ebx

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  push eax
                  push ebx

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  pop ebx
                  pop ecx

                  imul ebx,ebp
                  imul ecx,cur_r
                  sar ebx,16
                  sar ecx,16
                  add ebx,ecx

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  add eax,ebx
                  add es:[edi],eax

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+4]
                  add esi,step_whole[4+eax*4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __6_rep
                  jmp __exit

                ELSE

__6_loop:         cmp playback_ratio,10000h
                  jle __6_rep

                  mov ecx,edx                     ;Pitch up: drop samples
                                                  
                  movsx edx,WORD PTR fs:[esi]
                  mov ebx,1

__6_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left

                  ELSE

                    shl eax,11
                    
                  ENDIF

                  add es:[edi],eax

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx

                  lea edi,[edi+4]

__6_skip:         cmp ecx,65536
                  jl __6_drop

                  sub ecx,65536

                  add esi,2
                  cmp esi,src_end
                  jae __exit

                  movsx eax,WORD PTR fs:[esi]
                  add edx,eax
                  inc ebx
                  jmp __6_skip

__6_rep:          movsx eax,WORD PTR fs:[esi]        ;Pitch down: replicate samples
                  mov ecx,cur_l
                  mov cur_l,eax

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left

                  ELSE

                    shl eax,11
                    
                  ENDIF

                  add es:[edi],eax

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+4]
                  add esi,step_whole[4+eax*4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __6_rep
                  jmp __exit

                ENDIF

                ;----------------------------------
                ;DEST_STEREO + SRC_16 + FILTER
                ;----------------------------------

                IF (OP AND MOP_SRC_STEREO)

__7_loop:         cmp playback_ratio,10000h
                  jle __7_rep

                  mov ecx,edx                      ;Pitch up: drop samples
                          
                  movsx edx,WORD PTR fs:[esi]
                  movsx ebp,WORD PTR fs:[esi+2]
                  mov ebx,1

__7_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  mov eax,ebp
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_r,eax

                  mov edx,eax
                  mov eax,cur_l

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul edx,scale_right

                  ELSE

                    shl eax,11
                    shl edx,11
                    
                  ENDIF

                  IF (OP AND MOP_ORDER)

                    add es:[edi],edx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],edx

                  ENDIF

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebp,ebp
                  xor ebx,ebx

                  lea edi,[edi+8]

__7_skip:         cmp ecx,65536
                  jl __7_drop

                  sub ecx,65536

                  add esi,4
                  cmp esi,src_end
                  jae __exit

                  movsx eax,WORD PTR fs:[esi]
                  add edx,eax
                  movsx eax,WORD PTR fs:[esi+2]
                  add ebp,eax

                  inc ebx
                  jmp __7_skip

__7_rep:          movsx eax,WORD PTR fs:[esi]        ;Pitch down: replicate samples
                  movsx ebx,WORD PTR fs:[esi+2]

                  mov ecx,cur_l
                  mov ebp,cur_r
                  mov cur_l,eax
                  mov cur_r,ebx

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  push eax
                  push ebx

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  pop ebx
                  pop ecx

                  imul ebx,ebp
                  imul ecx,cur_r
                  sar ebx,16
                  sar ecx,16
                  add ebx,ecx

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF


                  IF (OP AND MOP_ORDER)

                    add es:[edi],ebx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],ebx

                  ENDIF

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+8]
                  add esi,step_whole[4+eax*4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __7_rep

                ELSE

__7_loop:         cmp playback_ratio,10000h
                  jle __7_rep

                  mov ecx,edx                      ;Pitch up: drop samples
                          
                  movsx edx,WORD PTR fs:[esi]
                  mov ebx,1

__7_drop:         cmp edi,dest_end
                  jae __exit

                  mov eax,edx
                  imul recip_table[ebx*4]
                  shrd eax,edx,16
                  mov cur_l,eax

                  mov edx,eax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul edx,scale_right

                  ELSE

                    shl eax,11
                    shl edx,11
                    
                  ENDIF


                  IF (OP AND MOP_ORDER)

                    add es:[edi],edx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],edx

                  ENDIF

                  add ecx,playback_ratio

                  xor edx,edx
                  xor ebx,ebx

                  lea edi,[edi+8]

__7_skip:         cmp ecx,65536
                  jl __7_drop

                  sub ecx,65536

                  add esi,2
                  cmp esi,src_end
                  jae __exit

                  movsx eax,WORD PTR fs:[esi]
                  add edx,eax
                  inc ebx
                  jmp __7_skip

__7_rep:          movsx eax,WORD PTR fs:[esi]        ;Pitch down: replicate samples

                  mov ecx,cur_l
                  mov cur_l,eax

                  mov eax,edx
                  mov ebx,65535
                  shr eax,16
                  sub ebx,eax

                  imul ebx,ecx
                  imul eax,cur_l
                  sar ebx,16
                  sar eax,16
                  add eax,ebx

                  mov ebx,eax

                  IF (OP AND MOP_VOL_SCALING)
                  
                    imul eax,scale_left
                    imul ebx,scale_right

                  ELSE

                    shl eax,11
                    shl ebx,11
                    
                  ENDIF

                  IF (OP AND MOP_ORDER)

                    add es:[edi],ebx
                    add es:[edi+4],eax

                  ELSE

                    add es:[edi],eax
                    add es:[edi+4],ebx

                  ENDIF

                  add edx,step_fract
                  sbb eax,eax
                  lea edi,[edi+8]
                  add esi,step_whole[4+eax*4]

                  cmp edi,dest_end
                  jae __exit
                  cmp esi,src_end
                  jb __7_rep

                ENDIF

__exit:         pop ebp
                ret

MERGE_&Flg&     ENDP

                ;
                ;Save current assembly location, then insert vector to
                ;primitive at appropriate place in jump table
                ;

                ASM = $

                ORG OFFSET vector_table + ((OP) * SIZE WORD)
                dw MERGE_&Flg&

                ORG ASM

                ENDM

;#############################################################################
;##                                                                         ##
;## MAKE_MERGE procedures                                                   ##
;##                                                                         ##
;#############################################################################

PARM_SPACE      equ 16 * SIZE WORD

vector_table    LABEL WORD

                ;
                ;Reserve space for mixer vector table
                ;

                ORG ((OFFSET vector_table) + PARM_SPACE)

                ;
                ;Set up to build all 16 possible mixer loop routines
                ;

OP              = 0
                REPT 16

VALID           = 1

                ;
                ;Don't build MOP_ORDER routines for mono output
                ;

                IF (OP AND M_DEST_STEREO) EQ 0
                  IF (OP AND M_ORDER)
                    VALID = 0
                  ENDIF
                ENDIF

                IF VALID

                  MAKE_MERGE %OP

                ENDIF

OP            = OP + 1
                ENDM

;#############################################################################
;##                                                                         ##
;## Model-independent memset()                                              ##
;##                                                                         ##
;#############################################################################

cProc           AIL_memset,<PUBLIC,FAR>,<si,di,bx,ds,es>
                parmD lpDest
                parmD nCh
                parmD nLen
cBegin
                SAVE_REGS

                cld

                xor edi,edi
                les di,[lpDest]

                mov ecx,[nLen]

                mov eax,[nCh]
                mov ah,al
                mov ebx,eax
                shl eax,16
                mov ax,bx

                mov bx,cx
                shr ecx,2
                db 67h          ;Address size override: build buffer may be
                rep stosd       ;> 64K in size
                mov cx,bx
                and cx,3
                rep stosb

                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Return 1 if MMX available, else 0                                       ##
;##                                                                         ##
;#############################################################################

cProc           MSS_MMX_available,<PUBLIC,FAR>,<si,di,bx,ds,es>
cBegin
                SAVE_REGS

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
                jmp __bail

__no_MMX:       mov eax,0

__bail:         mov edx,0

                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Start up mixer services                                                 ##
;##                                                                         ##
;#############################################################################

cProc           MSS_mixer_startup,<PUBLIC,FAR>,<si,di,bx,ds,es>
cBegin
                SAVE_REGS

                mov eax,0

                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Shut down mixer services                                                ##
;##                                                                         ##
;#############################################################################

cProc           MSS_mixer_shutdown,<PUBLIC,FAR>,<si,di,bx,ds,es>
cBegin
                SAVE_REGS

                mov eax,0

                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Flush build buffer with zero-amplitude DC baseline signal               ##
;##                                                                         ##
;#############################################################################

cProc           MSS_mixer_flush,<PUBLIC,FAR>,<si,di,bx,ds,es>
                parmD lpDest
                parmD nLen
                parmD lpReverbBuff
                parmD dwReverbLevel
                parmD bMMXAvailable
cBegin          
                SAVE_REGS

                cld

                xor edi,edi
                les di,[lpDest]

                mov ecx,[nLen]

                cmp dwReverbLevel,0
                jne __apply_reverb

                mov eax,0

                push ecx
                and ecx,3
                rep stosb
                pop ecx
                shr ecx,2
                db 67h          ;Address size override: build buffer may be
                rep stosd       ;> 64K in size

                jmp __bail

__apply_reverb: shr ecx,2

                xor esi,esi
                lfs si,[lpReverbBuff]

__rev_loop:     mov eax,fs:[esi]
                imul dwReverbLevel
                shrd eax,edx,7
                mov es:[edi],eax

                add esi,4
                add edi,4

                dec ecx
                jnz __rev_loop

__bail:
                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Add data at *src to *dest                                               ##
;##                                                                         ##
;#############################################################################

cProc           MSS_reverb_merge,<PUBLIC,FAR>,<si,di,bx,ds,es>
                parmD lpSrc
                parmD lpDest
                parmD nLen
cBegin          
                SAVE_REGS

                cld

                xor edi,edi
                les di,[lpDest]

                xor esi,esi
                lfs si,[lpSrc]

                mov ecx,[nLen]
                shr ecx,2

__rev_loop:     mov eax,fs:[esi]
                add es:[edi],eax

                add esi,4
                add edi,4

                dec ecx
                jnz __rev_loop

                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Mix data from PCM source into 32-bit signed build buffer                ##
;##                                                                         ##
;#############################################################################

cProc           MSS_mixer_merge,<PUBLIC,FAR>,<si,di,bx,ds,es>
                parmD dwSrcSel
                parmD dwDestSel
                parmD lpSrcFract
                parmD lpSrcOffset
                parmD lpDestOffset
                parmD dwSrcEnd
                parmD dwDestEnd
                parmD lpLeftVal
                parmD lpRightVal
                parmD nPlaybackRatio
                parmD nScaleBoth
                parmD nOperation
cBegin
                SAVE_REGS

                ;
                ;Save static values -- this routine is used as both the DLS
                ;and main mixers in Win16, and reentry is possible
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
                ;FS:ESI->src
                ;ES:EDI->dest
                ;
                ;Non-filtered and downsample-filtered loops maintain the 
                ;source address fraction at 0:32-bit precision.  Upsample-
                ;filtered loops use 16:16-bit precision, to avoid processing
                ;skipped samples twice.  The fractional component is always 
                ;returned to the caller at 16:16 precision to allow smooth 
                ;transitions between the two loop types.
                ;

                xor ebx,ebx
                lgs bx,[lpSrcFract]

                mov edx,gs:[ebx]

                test [nOperation],M_FILTER
                jz __rest_fract
                cmp [nPlaybackRatio],10000h
                jg __load_src
__rest_fract:   shl edx,16

__load_src:     xor ebx,ebx
                lgs bx,[lpLeftVal]
                mov eax,gs:[ebx]
                mov cur_l,eax

                xor ebx,ebx
                lgs bx,[lpRightVal]
                mov eax,gs:[ebx]
                mov cur_r,eax

                xor ebx,ebx
                lgs bx,[lpSrcOffset]

                mov fs,[dwSrcSel]
                mov esi,gs:[ebx]
                cmp esi,[dwSrcEnd]
                jae __exit

                xor ebx,ebx
                lgs bx,[lpDestOffset]

                mov es,[dwDestSel]
                mov edi,gs:[ebx]
                mov ecx,[dwDestEnd]
                cmp edi,ecx
                jae __exit

                ;
                ;Make parameters accessible to macros
                ;

                mov eax,[nScaleBoth]
                mov ebx,eax
                shr eax,16
                mov scale_left,eax
                and ebx,0ffffh
                mov scale_right,ebx

                mov eax,[dwSrcEnd]
                mov ebx,[dwDestEnd]
                mov src_end,eax
                mov dest_end,ebx

                mov eax,[nPlaybackRatio]
                mov playback_ratio,eax

                mov eax,[nOperation]
                mov operation,eax

                ;
                ;Call merge handler
                ;
                ;Operations in lower 3 bits (DEST_STEREO, SRC_16, FILTER)
                ;are selected at runtime
                ;
                ;Operations in upper 3 bits (SRC_STEREO, VOL_SCALING, RESAMPLE)
                ;are constructed at assembly time
                ;

                shr eax,3               

                call cs:vector_table[eax * SIZE WORD]

                ;
                ;Update application src/dest pointers before returning
                ;

__exit:         test [nOperation],M_FILTER
                jz __shift_fract
                cmp [nPlaybackRatio],10000h
                jle __shift_fract
                mov edx,ecx
                jmp __store_src
__shift_fract:  shr edx,16

__store_src:    xor ebx,ebx
                lgs bx,[lpSrcOffset]
                mov gs:[ebx],esi

                xor ebx,ebx
                lgs bx,[lpDestOffset]
                mov gs:[ebx],edi

                xor ebx,ebx
                lgs bx,[lpSrcFract]
                mov gs:[ebx],edx

                mov eax,cur_l
                xor ebx,ebx
                lgs bx,[lpLeftVal]
                mov gs:[ebx],eax

                mov eax,cur_r
                xor ebx,ebx
                lgs bx,[lpRightVal]
                mov gs:[ebx],eax

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

                RESTORE_REGS
cEnd

;#############################################################################
;##                                                                         ##
;## Clip and copy build buffer data to output sample buffer                 ##
;##                                                                         ##
;#############################################################################

cProc           MSS_mixer_copy,<PUBLIC,FAR>,<si,di,bx,ds,es>
                parmD lpSrc
                parmD nSrcLen
                parmD lpDest
                parmD nOperation
                parmD bMMXAvailable
cBegin
                SAVE_REGS

                ;
                ;FS:ESI->src
                ;ES:EDI->dest
                ;ECX->src end
                ;

                xor esi,esi
                lfs si,[lpSrc]

                xor edi,edi
                les di,[lpDest]

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

__do_16_MMX:    movq mm0,fs:[esi]   ;read first 2 source DWORDs
                psrad mm0,11        ;SAR DWORD pair by 11

                movq mm1,fs:[esi+8] ;read next 2 source DWORDs
                psrad mm1,11        ;SAR DWORD pair by 11

                packssdw mm0,mm1    ;merge DWORDs into 4 WORDs w/saturation
                movq es:[edi],mm0   ;write 4 samples to output stream

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

__8_MMX_loop:   movq mm0,fs:[esi]   ;read first 2 source DWORDs
                psrad mm0,11        ;SAR DWORD pair by 11

                movq mm1,fs:[esi+8] ;read next 2 source DWORDs
                psrad mm1,11        ;SAR DWORD pair by 11

                packssdw mm0,mm1    ;merge DWORDs into 4 WORDs w/saturation

                psraw mm0,8         ;convert to bytes
                packsswb mm0,mm0

                pxor mm0,mm2        ;convert to unsigned bytes
                movd es:[edi],mm0   ;write 4 samples to output stream

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

__copy_16:      mov eax,fs:[esi]

                add edi,2
                sar eax,11
                add esi,4

                cmp eax,32767
                jg __clip_H16
                cmp eax,-32768
                jl __clip_L16

                mov es:[edi-2],ax
                cmp esi,ecx
                jb __copy_16
                jmp __exit

__clip_H16:     mov WORD PTR es:[edi-2],32767
                cmp esi,ecx
                jb __copy_16
                jmp __exit

__clip_L16:     mov WORD PTR es:[edi-2],-32768
                cmp esi,ecx
                jb __copy_16
                jmp __exit

                ;
                ;X86 clip and copy to 8-bit stream
                ;

__copy_8:       mov eax,fs:[esi]

                inc edi
                sar eax,11
                add esi,4

                cmp eax,32767
                jg __clip_H8
                cmp eax,-32768
                jl __clip_L8

                xor eax,8000h

                mov es:[edi-1],ah
                cmp esi,ecx
                jb __copy_8
                jmp __exit

__clip_H8:      mov BYTE PTR es:[edi-1],0ffh
                cmp esi,ecx
                jb __copy_8
                jmp __exit

__clip_L8:      mov BYTE PTR es:[edi-1],0
                cmp esi,ecx
                jb __copy_8
                jmp __exit

__exit:         
                RESTORE_REGS
cEnd

;#############################################################################
DECODE_ADPCM	MACRO Flg

IF (Flg AND 2)
DECODEADPCM_MONO_8 PROC
ELSE
IF (Flg AND 1)
DECODEADPCM_STEREO PROC
ELSE
DECODEADPCM_MONO PROC
ENDIF
ENDIF

destaddrptr equ ebp+38+16
srcaddrptr  equ ebp+38+12
destlen     equ ebp+38+8
srclen      equ ebp+38+4
adpcmptr    equ ebp+38+0

  push es
  push fs
  push gs
  push eax
  push ebx
  push ecx
  push edx
  push esi
  push edi
  push ebp

  movzx ebp,sp

  xor edi,edi
  xor ebx,ebx
  les di,[destaddrptr]
  lgs bx,[srcaddrptr]
  les di,es:[di]
  lgs bx,gs:[bx]

  mov esi,edi
  mov ecx,ebx
  add esi,[destlen]
  add ecx,[srclen]

  lfs bp,[adpcmptr]
  mov fs:[ebp.ADPCMDATA.destend],esi
  mov fs:[ebp.ADPCMDATA.srcend],ecx

  cmp fs:[ebp.ADPCMDATA.extrasamples],0
  ja  __doextrasamps

  cmp fs:[ebp.ADPCMDATA.blockleft],0
  ja __doadword

__startblock:
  IF (Flg AND 1)

  mov eax,gs:[ebx] ;get the first sample and step size
  mov esi,fs:[ebp.ADPCMDATA.blocksize]
  ror eax,16
  sub esi,8

  mov edx,gs:[ebx+4]
  mov fs:[ebp.ADPCMDATA.blockleft],esi
  xchg ax,dx

  ror eax,16
  add ebx,8

  shl edx,8
  mov es:[edi],eax

  add edi,4

  ELSE

  IF (Flg AND 2)

  mov edx,gs:[ebx] ;get the first sample and step size
  mov eax,edx
  mov esi,fs:[ebp.ADPCMDATA.blocksize]
  add dh,080h
  add ebx,4
  mov es:[edi],dh
  sub esi,4
  inc edi
  shr edx,8
  mov fs:[ebp.ADPCMDATA.blockleft],esi
  and edx,0ffffh

  ELSE

  mov edx,gs:[ebx] ;get the first sample and step size
  mov eax,edx
  mov esi,fs:[ebp.ADPCMDATA.blocksize]
  mov es:[edi],dx
  add ebx,4
  shr edx,8
  sub esi,4
  add edi,2
  mov fs:[ebp.ADPCMDATA.blockleft],esi
  and edx,0ffffh

  ENDIF
  ENDIF

__contsamps:
  cmp ebx,fs:[ebp.ADPCMDATA.srcend]
  jae __srcend

__donextdword:

  mov ecx,gs:[ebx]

  IF (Flg AND 1)
  add ebx,8
  mov dl,cl
  mov fs:[ebp.ADPCMDATA.savesrc],ebx
  shl dl,4
  mov ebx,gs:[ebx-4]
  sub fs:[ebp.ADPCMDATA.blockleft],8
  ror edx,16
  mov fs:[ebp.ADPCMDATA.extrasamples],8
  mov dl,bl
  shl dl,4
  ror edx,20
  ELSE
  mov dl,cl
  add ebx,4
  mov fs:[ebp.ADPCMDATA.extrasamples],8
  shl dl,4
  sub fs:[ebp.ADPCMDATA.blockleft],4
  ENDIF

__expanddword:

  cmp edi,fs:[ebp.ADPCMDATA.destend]
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
  mov es:[edi],eax

  shr ebx,4
  add edi,4

  ELSE

  shr edx,4
  test cl,8
  mov si,stepw[edx*2]
  mov dh,newadpcm[edx]
  jnz __minus

  add si,ax
  mov dl,cl
  cmp si,ax
  jge __handle
  mov ax,32767
  jmp __fixedsample

__minus:
  add si,ax
  mov dl,cl
  cmp si,ax
  jg __toolow

__handle:
  mov ax,si
__fixedsample:
  IF (Flg AND 2)

  ;8-bit mono
  add ah,080h
  shr ecx,4
  mov es:[edi],ah
  inc edi
  sub ah,080h

  ELSE

  ;16-bit mono
  shr ecx,4
  mov es:[edi],ax
  add edi,2

  ENDIF

  ENDIF

  dec fs:[ebp.ADPCMDATA.extrasamples]
  jnz __expanddword

  IF (Flg AND 1)
  mov ebx,fs:[ebp.ADPCMDATA.savesrc]
  ENDIF

  cmp edi,fs:[ebp.ADPCMDATA.destend]
  jae __destendloop

  IF (Flg AND 1)
  shl edx,4
  ENDIF

  cmp fs:[ebp.ADPCMDATA.blockleft],0
  jne __contsamps

  cmp ebx,fs:[ebp.ADPCMDATA.srcend]
  jb __startblock

__destendtop:
__srcend:
  mov fs:[ebp.ADPCMDATA.extrasamples],0
  mov fs:[ebp.ADPCMDATA.adpstep],edx
  jmp __done

__destend:
  mov fs:[ebp.ADPCMDATA.samplesL],ecx
  IF (Flg AND 1)
  mov fs:[ebp.ADPCMDATA.samplesR],ebx
  mov ebx,fs:[ebp.ADPCMDATA.savesrc]
  ENDIF

__destendloop:
  mov fs:[ebp.ADPCMDATA.lsample],eax
  mov fs:[ebp.ADPCMDATA.adpstep],edx

__done:
  movzx ebp,sp

__norm_out:             ;normalize pointers before returning -- jm
  cmp edi,10000h
  jb __norm_in
  sub edi,10000h
  mov ax,es
  add ax,__AHINCR
  mov es,ax
  jmp __norm_out

__norm_in:
  cmp ebx,10000h
  jb __return
  sub ebx,10000h
  mov ax,gs
  add ax,__AHINCR
  mov gs,ax
  jmp __norm_in

__return:
  xor esi,esi
  lfs si,[destaddrptr]
  mov WORD PTR fs:[esi],di
  mov WORD PTR fs:[esi+2],es

  xor ecx,ecx
  lfs cx,[srcaddrptr]
  mov WORD PTR fs:[ecx],bx
  mov WORD PTR fs:[ecx+2],gs

  pop ebp
  pop edi
  pop esi
  pop edx
  pop ecx
  pop ebx
  pop eax
  pop gs
  pop fs
  pop es
  retf 20

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
  mov edx,fs:[ebp.ADPCMDATA.adpstep]
  IF (Flg AND 1)
  shl edx,4
  ENDIF

  mov eax,fs:[ebp.ADPCMDATA.lsample]
  jmp __donextdword

__doextrasamps:

  mov ecx,fs:[ebp.ADPCMDATA.samplesL]
  IF (Flg AND 1)
  mov fs:[ebp.ADPCMDATA.savesrc],ebx
  mov ebx,fs:[ebp.ADPCMDATA.samplesR]
  ENDIF

  mov eax,fs:[ebp.ADPCMDATA.lsample]
  mov edx,fs:[ebp.ADPCMDATA.adpstep]
  jmp __expanddword

IF (Flg AND 2)
DECODEADPCM_MONO_8 ENDP
ELSE
IF (Flg AND 1)
DECODEADPCM_STEREO ENDP
ELSE
DECODEADPCM_MONO ENDP
ENDIF
ENDIF
                ENDM

;#############################################################################
DECODE_ADPCM 0	; Mono adpcm

DECODE_ADPCM 1	; Stereo adpcm

DECODE_ADPCM 2	; Mono 8-bit adpcm

;#############################################################################


public AIL_MEMCPYDB

AIL_MEMCPYDB proc

dstptr equ ebp+24+8
srcptr equ ebp+24+4
cpylen equ ebp+24+0

      push ecx
      push esi
      push edi
      push ebp
      push ds
      push es

      movzx ebp,sp
      mov ecx,[cpylen]
      xor edi,edi
      xor esi,esi
      lds si,[srcptr]
      les di,[dstptr]

      std
      mov al,cl
      lea esi,[esi+ecx-4]
      lea edi,[edi+ecx-4]
      shr ecx,2
      db 67h          ;Address size override
      rep movsd
      and al,3
      jz __dne
      add esi,3
      add edi,3
      mov cl,al
      rep movsb
    __dne:
      cld
      pop es
      pop ds
      pop ebp
      pop edi
      pop esi
      pop ecx
      retf 12
AIL_MEMCPYDB endp


public _Only16Push32s

_Only16Push32s proc                ; ***Save the 32 bit registers
                pop  ax            ; get the 16:16 ret address
                pop  dx            ; get the 16:16 ret address
                push esi
                push edi
                ror edx,16
                push dx            ;save the high word
                ror edx,16
                push ecx
                push ebx
                ror eax,16
                push ax            ;save the high word
                ror eax,16
                push fs
                push gs
                sub sp,108
                push bp
                mov bp,sp
                fnsaved ss:[bp+2]
                pop bp
                push dx            ; put the 16:16 ret address back on stack
                push ax            ; put the 16:16 ret address back on stack
               retf
_Only16Push32s endp


public _Only16Pop32s

_Only16Pop32s  proc                ; ***Restore the 32 bit registers
                pop  ax            ; get the 16:16 ret address
                pop  dx            ; get the 16:16 ret address
                push bp
                mov bp,sp
                frstord ss:[bp+2]
                pop bp
                add sp,108
                pop gs
                pop fs
                ror eax,16
                pop ax              ;restore the high word
                ror eax,16
                pop ebx
                pop ecx
                ror edx,16
                pop dx              ;restore the high word
                ror edx,16
                pop edi
                pop esi
                push dx            ; put the 16:16 ret address back on stack
                push ax            ; put the 16:16 ret address back on stack
               retf
_Only16Pop32s  endp


public _SetupMSSCall16

_SetupMSSCall16   proc
                push bp
                mov  bp,sp
                push es
                push di
                les di,ss:[bp+6]                ; Get the function address
                mov ax,es:[di]
                cmp ax,0d88ch                   ; Pointing to MOV AX,DS?
                je  _setadj
                cmp ax,0d08ch                   ; Pointing to MOV AX,SS?
                je  _setadj
                cmp ax,05816h                   ; Pointing to PUSH SS POP AX?
                je  _setadj
                cmp ax,0581eh                   ; Pointing to PUSH DS POP AX?
                jne _setdone
_setadj:
                add di,2                        ; Yup, skip over those bytes
_setdone:
                mov [word ptr callbackofs],di   ; Save the callback
                mov [word ptr callbackofs+2],es
                mov ax,[bp+10]                  ; Save the DS value to use
                mov [word ptr callbackds],ax
                pop di
                pop es
                pop bp
               retf
_SetupMSSCall16   endp


public _MSSCall16

_MSSCall16     proc

                mov ax,[callbackds]   ; Setup up ax with correct DS
                jmp [callbackofs]     ; Setup the callback address

_MSSCall16    endp

public AIL_US_COUNT

AIL_US_COUNT proc
  push ebx
  push edx
  push eax
  mov eax,0100h
  call dword ptr [VTDaddr]
  and edx,07ffh
  mov ebx,1000000
  mul ebx
  mov ebx,1193181
  div ebx
  mov ebx,eax
  pop eax
  mov ax,bx
  shr ebx,16
  pop edx
  mov dx,bx
  pop ebx
  retf
AIL_US_COUNT ENDP

public WIN16INIT

WIN16INIT proc
  push eax
  push ebx
  push es
  push di
  xor di,di
  mov es,di
  mov eax,01684h
  mov ebx,5
  int 02fh
  mov word ptr [VTDaddr],di
  mov word ptr [VTDaddr+2],es
  pop di
  pop es
  pop ebx
  pop eax
  retf
WIN16INIT ENDP

db 13,10,13,10
db 'Miles Sound System',13,10
db 'Copyright (C) 1991-98 RAD Game Tools, Inc.',13,10,13,10

sEnd            CODE
                END
