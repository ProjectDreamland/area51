#ifndef __IOPVOICE_H
#define __IOPVOICE_H

#include "ioptypes.h"
#include "iopaud_cfx.h"
#include "iopaud_spu.h"
#include "iopaud_element.h"
#include "iopaud_stream.h"

#define VOICE_ALIGNMENT (16)

struct s_cfx_element;
struct s_voice_stream;

typedef struct s_audio_voice
{
    s16                     m_Id;                   // Current voice number 0..23
    s16                     m_Type;                 // Element, stream or hybrid.
    voicestat               m_Status;
    s16                     m_ExpireDelay;
    s16                     m_Age;
    s32                     m_SpuCurrent;           // Current SPU address
    struct s_cfx_element    *m_pOwner;
    struct s_audio_voice    *m_pSibling;            // Surround voice
    spu_state               m_Hardware;             // Current status of the hardware for this channel
} audio_voice;


void        voice_Init  (void);
void        voice_Kill  (void);
audio_voice *voice_Alloc(cfx_element *pElement);
void        voice_Free  (audio_voice *pVoice);
s32         voice_Cull  (s32 NumberToCull);
xbool       voice_Update(audio_voice *pVoice,cfx_state *pState,s32 DeltaTime);

#endif

