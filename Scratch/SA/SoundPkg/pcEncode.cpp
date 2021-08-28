#include "x_types.hpp"
#include "x_files.hpp"
#include "encode.hpp"
#include "library/encvag.h"
#include "memory.h"
#include "../../support/audiomgr/audio.hpp"
#include "../../support/audiomgr/pc/pcaud_audio.hpp"
#include "main.hpp"

s32 pc_Encode(s32 Length, u8* pOutBuffer, s8* pFilename)
{
    // The length of data.
    s32 len = strlen((const char *)pFilename);
    
    u8* bytes;
    u8* Buffer;
    Buffer = pOutBuffer;
    bytes = (u8 *)pFilename;
    
    // Copy the name of fill where the data is supposed to be.
    for (s32 i = 0; i < len; i++)
        Buffer[i] = bytes[i];

    return len;
}