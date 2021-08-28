Merge_DestMono_SrcMono_Src8_NoVolume_NoResample PROC ; 0

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_NoVolume_NoResample ENDP


Merge_DestStereo_SrcMono_Src8_NoVolume_NoResample PROC ; 1

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_NoVolume_NoResample ENDP


Merge_DestMono_SrcMono_Src16_NoVolume_NoResample PROC ; 2

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_NoVolume_NoResample ENDP


Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample PROC ; 3

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample ENDP


Merge_DestMono_SrcStereo_Src8_NoVolume_NoResample PROC ; 8

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_NoVolume_NoResample ENDP


Merge_DestStereo_SrcStereo_Src8_NoVolume_NoResample PROC ; 9

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_NoVolume_NoResample ENDP


Merge_DestMono_SrcStereo_Src16_NoVolume_NoResample PROC ; 10

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_NoVolume_NoResample ENDP


Merge_DestStereo_SrcStereo_Src16_NoVolume_NoResample PROC ; 11

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_NoVolume_NoResample ENDP


Merge_DestMono_SrcMono_Src8_Volume_NoResample PROC ; 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_Volume_NoResample ENDP


Merge_DestStereo_SrcMono_Src8_Volume_NoResample PROC ; 17

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_Volume_NoResample ENDP


Merge_DestMono_SrcMono_Src16_Volume_NoResample PROC ; 18

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  imul  eax, [scale_left]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  imul  eax, [scale_left]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_Volume_NoResample ENDP


Merge_DestStereo_SrcMono_Src16_Volume_NoResample PROC ; 19

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_Volume_NoResample ENDP


Merge_DestMono_SrcStereo_Src8_Volume_NoResample PROC ; 24

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_Volume_NoResample ENDP


Merge_DestStereo_SrcStereo_Src8_Volume_NoResample PROC ; 25

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_Volume_NoResample ENDP


Merge_DestMono_SrcStereo_Src16_Volume_NoResample PROC ; 26

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_Volume_NoResample ENDP


Merge_DestStereo_SrcStereo_Src16_Volume_NoResample PROC ; 27

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_Volume_NoResample ENDP


Merge_DestMono_SrcMono_Src8_NoVolume_Resample PROC ; 32

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_NoVolume_Resample ENDP


Merge_DestStereo_SrcMono_Src8_NoVolume_Resample PROC ; 33

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_NoVolume_Resample ENDP


Merge_DestMono_SrcMono_Src16_NoVolume_Resample PROC ; 34

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_NoVolume_Resample ENDP


Merge_DestStereo_SrcMono_Src16_NoVolume_Resample PROC ; 35

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_NoVolume_Resample ENDP


Merge_DestMono_SrcMono_Src8_NoVolume_DownFiltered PROC ; 36

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcMono_Src8_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  add   ecx, eax
  sub   edx, 65536
  inc   esi
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  inc   esi
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_NoVolume_DownFiltered ENDP


Merge_DestStereo_SrcMono_Src8_NoVolume_DownFiltered PROC ; 37

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcMono_Src8_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  add   ecx, eax
  sub   edx, 65536
  inc   esi
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  inc   esi
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_NoVolume_DownFiltered ENDP


Merge_DestMono_SrcMono_Src16_NoVolume_DownFiltered PROC ; 38

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcMono_Src16_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  movsx eax, word ptr [esi]
  add   ecx, eax
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  movsx eax, word ptr [esi]
  add   esi, 2
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_NoVolume_DownFiltered ENDP


Merge_DestStereo_SrcMono_Src16_NoVolume_DownFiltered PROC ; 39

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcMono_Src16_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  movsx eax, word ptr [esi]
  add   ecx, eax
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  movsx eax, word ptr [esi]
  add   esi, 2
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_NoVolume_DownFiltered ENDP


Merge_DestMono_SrcStereo_Src8_NoVolume_Resample PROC ; 40

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_NoVolume_Resample ENDP


Merge_DestStereo_SrcStereo_Src8_NoVolume_Resample PROC ; 41

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_NoVolume_Resample ENDP


Merge_DestMono_SrcStereo_Src16_NoVolume_Resample PROC ; 42

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_NoVolume_Resample ENDP


Merge_DestStereo_SrcStereo_Src16_NoVolume_Resample PROC ; 43

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_NoVolume_Resample ENDP


Merge_DestMono_SrcStereo_Src8_NoVolume_DownFiltered PROC ; 44

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcStereo_Src8_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   ecx, eax
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   esi, 2
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_NoVolume_DownFiltered ENDP


Merge_DestStereo_SrcStereo_Src8_NoVolume_DownFiltered PROC ; 45

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcStereo_Src8_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_l], eax
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h
  add   esi, 2
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_NoVolume_DownFiltered ENDP


Merge_DestMono_SrcStereo_Src16_NoVolume_DownFiltered PROC ; 46

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcStereo_Src16_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   ecx, eax
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   esi, 4
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_NoVolume_DownFiltered ENDP


Merge_DestStereo_SrcStereo_Src16_NoVolume_DownFiltered PROC ; 47

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcStereo_Src16_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16
  add   esi, 4
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_NoVolume_DownFiltered ENDP


Merge_DestMono_SrcMono_Src8_Volume_Resample PROC ; 48

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_Volume_Resample ENDP


Merge_DestStereo_SrcMono_Src8_Volume_Resample PROC ; 49

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_Volume_Resample ENDP


Merge_DestMono_SrcMono_Src16_Volume_Resample PROC ; 50

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  imul  eax, [scale_left]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Apply volume
  imul  eax, [scale_left]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_Volume_Resample ENDP


Merge_DestStereo_SrcMono_Src16_Volume_Resample PROC ; 51

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_Volume_Resample ENDP


Merge_DestMono_SrcMono_Src8_Volume_DownFiltered PROC ; 52

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcMono_Src8_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  add   ecx, eax
  sub   edx, 65536
  inc   esi
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  inc   esi
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_Volume_DownFiltered ENDP


Merge_DestStereo_SrcMono_Src8_Volume_DownFiltered PROC ; 53

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcMono_Src8_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  add   ecx, eax
  sub   edx, 65536
  inc   esi
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h
  inc   esi
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx

  ; Duplicate the left channel into the right
  mov   ebx, eax
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_Volume_DownFiltered ENDP


Merge_DestMono_SrcMono_Src16_Volume_DownFiltered PROC ; 54

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcMono_Src16_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  movsx eax, word ptr [esi]
  add   ecx, eax
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  movsx eax, word ptr [esi]
  add   esi, 2
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_Volume_DownFiltered ENDP


Merge_DestStereo_SrcMono_Src16_Volume_DownFiltered PROC ; 55

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcMono_Src16_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  movsx eax, word ptr [esi]
  add   ecx, eax
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  movsx eax, word ptr [esi]
  add   esi, 2
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx

  ; Duplicate the left channel into the right
  mov   ebx, eax
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_Volume_DownFiltered ENDP


Merge_DestMono_SrcStereo_Src8_Volume_Resample PROC ; 56

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_Volume_Resample ENDP


Merge_DestStereo_SrcStereo_Src8_Volume_Resample PROC ; 57

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_Volume_Resample ENDP


Merge_DestMono_SrcStereo_Src16_Volume_Resample PROC ; 58

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_Volume_Resample ENDP


Merge_DestStereo_SrcStereo_Src16_Volume_Resample PROC ; 59

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_Volume_Resample ENDP


Merge_DestMono_SrcStereo_Src8_Volume_DownFiltered PROC ; 60

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcStereo_Src8_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h
  add   esi, 2
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_Volume_DownFiltered ENDP


Merge_DestStereo_SrcStereo_Src8_Volume_DownFiltered PROC ; 61

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcStereo_Src8_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h
  add   esi, 2
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_Volume_DownFiltered ENDP


Merge_DestMono_SrcStereo_Src16_Volume_DownFiltered PROC ; 62

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcStereo_Src16_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16
  add   esi, 4
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_Volume_DownFiltered ENDP


Merge_DestStereo_SrcStereo_Src16_Volume_DownFiltered PROC ; 63

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcStereo_Src16_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16
  add   esi, 4
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_Volume_DownFiltered ENDP


Merge_DestMono_SrcFlipped_Src8_NoVolume_NoResample PROC ; 72

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_NoVolume_NoResample ENDP


Merge_DestStereo_SrcFlipped_Src8_NoVolume_NoResample PROC ; 73

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_NoVolume_NoResample ENDP


Merge_DestMono_SrcFlipped_Src16_NoVolume_NoResample PROC ; 74

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_NoVolume_NoResample ENDP


Merge_DestStereo_SrcFlipped_Src16_NoVolume_NoResample PROC ; 75

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_NoVolume_NoResample ENDP


Merge_DestMono_SrcFlipped_Src8_Volume_NoResample PROC ; 88

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_Volume_NoResample ENDP


Merge_DestStereo_SrcFlipped_Src8_Volume_NoResample PROC ; 89

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_Volume_NoResample ENDP


Merge_DestMono_SrcFlipped_Src16_Volume_NoResample PROC ; 90

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_Volume_NoResample ENDP


Merge_DestStereo_SrcFlipped_Src16_Volume_NoResample PROC ; 91

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_Volume_NoResample ENDP


Merge_DestMono_SrcFlipped_Src8_NoVolume_Resample PROC ; 104

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_NoVolume_Resample ENDP


Merge_DestStereo_SrcFlipped_Src8_NoVolume_Resample PROC ; 105

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_NoVolume_Resample ENDP


Merge_DestMono_SrcFlipped_Src16_NoVolume_Resample PROC ; 106

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Apply volume
  shl   eax, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_NoVolume_Resample ENDP


Merge_DestStereo_SrcFlipped_Src16_NoVolume_Resample PROC ; 107

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_NoVolume_Resample ENDP


Merge_DestMono_SrcFlipped_Src8_NoVolume_DownFiltered PROC ; 108

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcFlipped_Src8_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   ecx, eax
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   esi, 2
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_NoVolume_DownFiltered ENDP


Merge_DestStereo_SrcFlipped_Src8_NoVolume_DownFiltered PROC ; 109

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcFlipped_Src8_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_l], eax
  mov   edx, 8
  xor   eax, eax  ; 100000000f/32*256
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h
  add   esi, 2
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_NoVolume_DownFiltered ENDP


Merge_DestMono_SrcFlipped_Src16_NoVolume_DownFiltered PROC ; 110

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcFlipped_Src16_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_l], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  imul  ecx, ebx
  sar   ecx, 16
  cmp   edx, 65536*2
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   ecx, eax
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  last_continue:
  cmp   esi, [src_end]
  jae   src_last_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx
  add   esi, 4
  mov   ebx, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebx
  imul  eax, [divider_l]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_NoVolume_DownFiltered ENDP


Merge_DestStereo_SrcFlipped_Src16_NoVolume_DownFiltered PROC ; 111

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcFlipped_Src16_NoVolume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 08000000h  ; 100000000f/32
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16
  add   esi, 4
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_NoVolume_DownFiltered ENDP


Merge_DestMono_SrcFlipped_Src8_Volume_Resample PROC ; 120

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_Volume_Resample ENDP


Merge_DestStereo_SrcFlipped_Src8_Volume_Resample PROC ; 121

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_Volume_Resample ENDP


Merge_DestMono_SrcFlipped_Src16_Volume_Resample PROC ; 122

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_Volume_Resample ENDP


Merge_DestStereo_SrcFlipped_Src16_Volume_Resample PROC ; 123

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  sbb   eax, eax
  add   esi, step_whole[4+eax*4]

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_Volume_Resample ENDP


Merge_DestMono_SrcFlipped_Src8_Volume_DownFiltered PROC ; 124

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcFlipped_Src8_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h
  add   esi, 2
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_Volume_DownFiltered ENDP


Merge_DestStereo_SrcFlipped_Src8_Volume_DownFiltered PROC ; 125

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcFlipped_Src8_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536*256  ; 100000000f/2048/32*256
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 2
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h
  add   esi, 2
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_Volume_DownFiltered ENDP


Merge_DestMono_SrcFlipped_Src16_Volume_DownFiltered PROC ; 126

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestMono_SrcFlipped_Src16_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16
  add   esi, 4
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_Volume_DownFiltered ENDP


Merge_DestStereo_SrcFlipped_Src16_Volume_DownFiltered PROC ; 127

  ; check to see if we have to call the upsampling version
  cmp   [playback_ratio], 10000h
  jle   Merge_DestStereo_SrcFlipped_Src16_Volume_UpFiltered

  ; build average dividers
  mov   ecx, edx
  mov   ebx, [playback_ratio]
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_left]
  div   ebx
  mov   [divider_l], eax
  xor   edx, edx
  mov   eax, 65536  ; 100000000f/2048/32
  imul  [scale_right]
  div   ebx
  mov   [divider_r], eax
  mov   edx, ecx

  ; load initial sample
  mov   ecx, [cur_l]

  ; Save registers
  push  ebp

  ; handle start up loop management
  mov  eax, edx
  and  edx, 03fffffffh
  test eax, 080000000h
  jnz  whole_continue
  test eax, 040000000h
  jnz  last_continue

  ; Merge sample data loop
  ALIGN 16
  merge_loop:
  mov   ebx, 65536
  sub   ebx, edx
  add   edx, [playback_ratio]

  ; weight the initial sample
  mov   eax, [cur_r]
  imul  ecx, ebx
  imul  eax, ebx
  sar   ecx, 16
  sar   eax, 16
  cmp   edx, 65536*2
  mov   [cur_r], eax
  jl    skip_loop

  ; loop to load all of the full sample points
  whole_loop:
  cmp   esi, [src_end]
  jae   src_whole_exit
  whole_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16
  add   ecx, eax
  add   [cur_r], ebx
  sub   edx, 65536
  add   esi, 4
  cmp   edx, 65536*2
  jae   whole_loop

  skip_loop:
  and   edx, 0ffffh
  cmp   esi, [src_end]
  jae   src_last_exit
  last_continue:

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16
  add   esi, 4
  mov   ebp, ecx
  mov   ecx, eax

  ; weight the final sample
  imul  eax, edx
  sar   eax, 16
  add   eax, ebp
  mov   ebp, [cur_r]
  mov   [cur_r], ebx
  imul  ebx, edx
  sar   ebx, 16
  add   ebx, ebp
  imul  eax, [divider_l]
  imul  ebx, [divider_r]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jb    merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  jmp   src_save_value

  ; jump out when src is exceed, but save our current loop position
  src_whole_exit:
  or   edx, 080000000h
  src_last_exit:
  or   edx, 040000000h
  src_save_value:
  mov  [cur_l], ecx

  ; Restore registers
  pop   ebp

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_Volume_DownFiltered ENDP


Merge_DestMono_SrcMono_Src8_NoVolume_UpFiltered PROC ; 164

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  dec   esi

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_NoVolume_UpFiltered ENDP


Merge_DestStereo_SrcMono_Src8_NoVolume_UpFiltered PROC ; 165

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  dec   esi

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_NoVolume_UpFiltered ENDP


Merge_DestMono_SrcMono_Src16_NoVolume_UpFiltered PROC ; 166

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_NoVolume_UpFiltered ENDP


Merge_DestStereo_SrcMono_Src16_NoVolume_UpFiltered PROC ; 167

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], eax
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_NoVolume_UpFiltered ENDP


Merge_DestMono_SrcStereo_Src8_NoVolume_UpFiltered PROC ; 172

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_NoVolume_UpFiltered ENDP


Merge_DestStereo_SrcStereo_Src8_NoVolume_UpFiltered PROC ; 173

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_NoVolume_UpFiltered ENDP


Merge_DestMono_SrcStereo_Src16_NoVolume_UpFiltered PROC ; 174

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_NoVolume_UpFiltered ENDP


Merge_DestStereo_SrcStereo_Src16_NoVolume_UpFiltered PROC ; 175

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_NoVolume_UpFiltered ENDP


Merge_DestMono_SrcMono_Src8_Volume_UpFiltered PROC ; 180

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  imul  eax, [scale_left]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  dec   esi

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src8_Volume_UpFiltered ENDP


Merge_DestStereo_SrcMono_Src8_Volume_UpFiltered PROC ; 181

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  inc   esi
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  mov   al, byte ptr [esi]
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  inc   esi
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  dec   esi

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src8_Volume_UpFiltered ENDP


Merge_DestMono_SrcMono_Src16_Volume_UpFiltered PROC ; 182

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  imul  eax, [scale_left]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcMono_Src16_Volume_UpFiltered ENDP


Merge_DestStereo_SrcMono_Src16_Volume_UpFiltered PROC ; 183

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Duplicate the left channel into the right
  mov   ebx, eax

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  movsx eax, word ptr [esi]

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcMono_Src16_Volume_UpFiltered ENDP


Merge_DestMono_SrcStereo_Src8_Volume_UpFiltered PROC ; 188

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Save registers
  push  ebp

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src8_Volume_UpFiltered ENDP


Merge_DestStereo_SrcStereo_Src8_Volume_UpFiltered PROC ; 189

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Save registers
  push  ebp

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   ebx, ebx
  xor   eax, eax
  mov   ax, word ptr [esi]
  mov   bl, ah
  and   eax, 0ffh
  sub   ebx, 080h
  sub   eax, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src8_Volume_UpFiltered ENDP


Merge_DestMono_SrcStereo_Src16_Volume_UpFiltered PROC ; 190

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcStereo_Src16_Volume_UpFiltered ENDP


Merge_DestStereo_SrcStereo_Src16_Volume_UpFiltered PROC ; 191

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   ebx, dword ptr [esi]
  movsx eax, bx
  sar   ebx, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcStereo_Src16_Volume_UpFiltered ENDP


Merge_DestMono_SrcFlipped_Src8_NoVolume_UpFiltered PROC ; 236

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 19

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_NoVolume_UpFiltered ENDP


Merge_DestStereo_SrcFlipped_Src8_NoVolume_UpFiltered PROC ; 237

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  shl   eax, 19
  shl   ebx, 19

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_NoVolume_UpFiltered ENDP


Merge_DestMono_SrcFlipped_Src16_NoVolume_UpFiltered PROC ; 238

  ; adjust fractional
  shl   edx, 16

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebx, edx
  mov   eax, [cur_l]
  shr   ebx, 16
  sub   eax, ecx
  imul  eax, ebx
  sar   eax, 16
  add   eax, ecx

  ; Apply volume
  shl   eax, 11

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_NoVolume_UpFiltered ENDP


Merge_DestStereo_SrcFlipped_Src16_NoVolume_UpFiltered PROC ; 239

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  shl   eax, 11
  shl   ebx, 11

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_NoVolume_UpFiltered ENDP


Merge_DestMono_SrcFlipped_Src8_Volume_UpFiltered PROC ; 252

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Save registers
  push  ebp

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src8_Volume_UpFiltered ENDP


Merge_DestStereo_SrcFlipped_Src8_Volume_UpFiltered PROC ; 253

  ; adjust fractional
  shl   edx, 16

  ; incorporate 8 to 16 upscale into volume
  shl  [scale_left], 8
  shl   [scale_right], 8

  ; Save registers
  push  ebp

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 2
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  xor   eax, eax
  xor   ebx, ebx
  mov   bx, word ptr [esi]
  mov   al, bh
  and   ebx, 0ffh
  sub   eax, 080h
  sub   ebx, 080h

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 2
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 2

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src8_Volume_UpFiltered ENDP


Merge_DestMono_SrcFlipped_Src16_Volume_UpFiltered PROC ; 254

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge left and right channels for mono dest
  add   eax, ebx

  ; Merge sample data into output buffer
  add   [edi], eax
  add   edi, 4
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestMono_SrcFlipped_Src16_Volume_UpFiltered ENDP


Merge_DestStereo_SrcFlipped_Src16_Volume_UpFiltered PROC ; 255

  ; adjust fractional
  shl   edx, 16

  ; Save registers
  push  ebp

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; Merge sample data loop
  ALIGN 16
  merge_loop:

  ; Upsample the data points
  mov   ebp, edx
  mov   eax, [cur_l]
  mov   ebx, [cur_r]
  shr   ebp, 16
  sub   eax, ecx
  sub   ebx, [cur_r2]
  imul  eax, ebp
  imul  ebx, ebp
  sar   eax, 16
  sar   ebx, 16
  add   eax, ecx
  add   ebx, [cur_r2]

  ; Apply volume
  imul  eax, [scale_left]
  imul  ebx, [scale_right]

  ; Merge sample data into output buffer
  add   [edi], eax
  add   [edi+4], ebx
  add   edi, 8
  cmp   edi, dest_end
  jae   dest_end_exit

  ; Add to accumulator and advance the source correctly
  add   edx, [step_fract]
  jnc   merge_loop

  ; Move the source pointer
  add   esi, 4
  cmp   esi, src_end
  jae   src_end_exit

  ; Load sample data
  mov   eax, dword ptr [esi]
  movsx ebx, ax
  sar   eax, 16

  ; rotate filter values
  mov   ecx, [cur_l]
  mov   [cur_l], eax
  mov   eax, [cur_r]
  mov   [cur_r], ebx
  mov   [cur_r2], eax

  ; End loop
  jmp   merge_loop

  ; Jump out point if end of dest is reached
  dest_end_exit:
  add   esi, 4
  add   edx, [step_fract]
  jc    skip_filter_adjust

  ; rotate filter values
  mov   [cur_l], ecx
  mov   ecx, [cur_r2]
  mov   [cur_r], ecx

  ; un-increment the source to skip the early source adjustment
  sub   esi, 4

  skip_filter_adjust:

  ; Jump out point if end of src is reached
  src_end_exit:

  ; Restore registers
  pop   ebp

  ; adjust fractional
  shr   edx, 16

  ; Routine end
  ret

Merge_DestStereo_SrcFlipped_Src16_Volume_UpFiltered ENDP


vector_table    LABEL DWORD
  dd Merge_DestMono_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcStereo_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcStereo_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcStereo_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcStereo_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcStereo_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcStereo_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcStereo_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcStereo_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_Volume_NoResample
  dd Merge_DestMono_SrcStereo_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcStereo_Src8_Volume_NoResample
  dd Merge_DestMono_SrcStereo_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcStereo_Src16_Volume_NoResample
  dd Merge_DestMono_SrcStereo_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcStereo_Src8_Volume_NoResample
  dd Merge_DestMono_SrcStereo_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcStereo_Src16_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src8_NoVolume_Resample
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_Resample
  dd Merge_DestMono_SrcMono_Src16_NoVolume_Resample
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_Resample
  dd Merge_DestMono_SrcMono_Src8_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_DownFiltered
  dd Merge_DestMono_SrcMono_Src16_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_DownFiltered
  dd Merge_DestMono_SrcStereo_Src8_NoVolume_Resample
  dd Merge_DestStereo_SrcStereo_Src8_NoVolume_Resample
  dd Merge_DestMono_SrcStereo_Src16_NoVolume_Resample
  dd Merge_DestStereo_SrcStereo_Src16_NoVolume_Resample
  dd Merge_DestMono_SrcStereo_Src8_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcStereo_Src8_NoVolume_DownFiltered
  dd Merge_DestMono_SrcStereo_Src16_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcStereo_Src16_NoVolume_DownFiltered
  dd Merge_DestMono_SrcMono_Src8_Volume_Resample
  dd Merge_DestStereo_SrcMono_Src8_Volume_Resample
  dd Merge_DestMono_SrcMono_Src16_Volume_Resample
  dd Merge_DestStereo_SrcMono_Src16_Volume_Resample
  dd Merge_DestMono_SrcMono_Src8_Volume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src8_Volume_DownFiltered
  dd Merge_DestMono_SrcMono_Src16_Volume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src16_Volume_DownFiltered
  dd Merge_DestMono_SrcStereo_Src8_Volume_Resample
  dd Merge_DestStereo_SrcStereo_Src8_Volume_Resample
  dd Merge_DestMono_SrcStereo_Src16_Volume_Resample
  dd Merge_DestStereo_SrcStereo_Src16_Volume_Resample
  dd Merge_DestMono_SrcStereo_Src8_Volume_DownFiltered
  dd Merge_DestStereo_SrcStereo_Src8_Volume_DownFiltered
  dd Merge_DestMono_SrcStereo_Src16_Volume_DownFiltered
  dd Merge_DestStereo_SrcStereo_Src16_Volume_DownFiltered
  dd Merge_DestMono_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcFlipped_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcFlipped_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcFlipped_Src8_NoVolume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src8_NoVolume_NoResample
  dd Merge_DestMono_SrcFlipped_Src16_NoVolume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src16_NoVolume_NoResample
  dd Merge_DestMono_SrcMono_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src8_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcMono_Src16_Volume_NoResample
  dd Merge_DestMono_SrcFlipped_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src8_Volume_NoResample
  dd Merge_DestMono_SrcFlipped_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src16_Volume_NoResample
  dd Merge_DestMono_SrcFlipped_Src8_Volume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src8_Volume_NoResample
  dd Merge_DestMono_SrcFlipped_Src16_Volume_NoResample
  dd Merge_DestStereo_SrcFlipped_Src16_Volume_NoResample
  dd Merge_DestMono_SrcMono_Src8_NoVolume_Resample
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_Resample
  dd Merge_DestMono_SrcMono_Src16_NoVolume_Resample
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_Resample
  dd Merge_DestMono_SrcMono_Src8_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src8_NoVolume_DownFiltered
  dd Merge_DestMono_SrcMono_Src16_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src16_NoVolume_DownFiltered
  dd Merge_DestMono_SrcFlipped_Src8_NoVolume_Resample
  dd Merge_DestStereo_SrcFlipped_Src8_NoVolume_Resample
  dd Merge_DestMono_SrcFlipped_Src16_NoVolume_Resample
  dd Merge_DestStereo_SrcFlipped_Src16_NoVolume_Resample
  dd Merge_DestMono_SrcFlipped_Src8_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcFlipped_Src8_NoVolume_DownFiltered
  dd Merge_DestMono_SrcFlipped_Src16_NoVolume_DownFiltered
  dd Merge_DestStereo_SrcFlipped_Src16_NoVolume_DownFiltered
  dd Merge_DestMono_SrcMono_Src8_Volume_Resample
  dd Merge_DestStereo_SrcMono_Src8_Volume_Resample
  dd Merge_DestMono_SrcMono_Src16_Volume_Resample
  dd Merge_DestStereo_SrcMono_Src16_Volume_Resample
  dd Merge_DestMono_SrcMono_Src8_Volume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src8_Volume_DownFiltered
  dd Merge_DestMono_SrcMono_Src16_Volume_DownFiltered
  dd Merge_DestStereo_SrcMono_Src16_Volume_DownFiltered
  dd Merge_DestMono_SrcFlipped_Src8_Volume_Resample
  dd Merge_DestStereo_SrcFlipped_Src8_Volume_Resample
  dd Merge_DestMono_SrcFlipped_Src16_Volume_Resample
  dd Merge_DestStereo_SrcFlipped_Src16_Volume_Resample
  dd Merge_DestMono_SrcFlipped_Src8_Volume_DownFiltered
  dd Merge_DestStereo_SrcFlipped_Src8_Volume_DownFiltered
  dd Merge_DestMono_SrcFlipped_Src16_Volume_DownFiltered
  dd Merge_DestStereo_SrcFlipped_Src16_Volume_DownFiltered
