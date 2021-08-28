/*--------------------------------------------------------------------------*
  Project: Dolphin DSP ADPCM encoder
  File:    endian.h

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *--------------------------------------------------------------------------*/
#include "x_types.hpp"

f32 reverse_endian_f32(f32 f);
u32 reverse_endian_32(u32 i);
u16 reverse_endian_16(u16 i);
void reverse_buffer_16(u16 *p, int samples);
