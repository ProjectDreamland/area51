#ifndef __IOPAUDIO_H
#define __IOPAUDIO_H

#include "ioptypes.h"
#include "iopaud_defines.h"
#include "iopaud_cfx.h"
#include "iopaud_voice.h"
#include "iopaud_container.h"
#include "iopaud_host.h"
#include "iopaud_stream.h"

#define ENABLE_SURROUND

#define CORES_IN_USE            2
#define VOICES_PER_CORE         24
#define AUD_MAX_PHYS_VOICES     (VOICES_PER_CORE * CORES_IN_USE)
#define AUD_PRIVATE_CHANNELS    16      // Used when we're processing complex effects
#define AUD_MAX_STREAMS         6       // Total # of streaming channels possible
#define AUD_STREAM_BUFFER_SIZE  (2048)  // Buffer size on spu for each stream (*2 for double buffer)
#define AUD_STREAM_READAHEAD    24

#define SET_KEY_ON(chn) SpuSetKey(chn,1)
#define SET_KEY_OFF(chn) SpuSetKey(chn,0)


typedef struct s_iop_audio_vars
{
    cfx_pool        *m_pCfxPool;
    cfx             *m_pRootCfx;
    audio_voice     *m_pVoices;
    iop_message_queue   m_qFreeVoices;
    container       *m_pContainers;
    s32             m_TimerTick;                    // Ticks in 10ms intervals with 0.1ms accuracy
    s32             m_ChannelCount;
    u32             m_LastVoiceId;
    s32             m_VoicesToKill;
    s32             m_VoicesInUse;
    s32             m_LowestKillPriority;
    s32             m_SysTimerId;
    u32             m_TopCfxId;
    u32             m_TopUniqueId;
    s32             m_UpdateThreadId;
    s32             m_Mutex;
    u8              *m_pInBuffer;
    u8              *m_pOutBuffer;
    spu_master_state m_MasterState;                 // State we want to write to hardware
    spu_master_state m_CurrentMasterState;          // State we just read from hardware on this update cycle
    s32             m_StreamThreadId;
    stream_request  m_StreamRequests[AUD_MAX_STREAMS * AUD_STREAM_READAHEAD];
    byte            *m_pStreamBuffers;
    iop_message_queue   m_qAvailableStreamRequests;
    iop_message_queue   m_qPendingStreamRequests;
    iop_message_queue   m_qFreeStreams;
    iop_message_queue   m_qLoadPending;
    char            m_LoadFilename[64];
    container_reply m_LoadReply;
    cfx_element     m_StreamerElement[2];
    cfx_element_attributes m_StreamerAttributes[2];
    cfx             *m_pStreamerCfx[2];
} iop_audio_vars;

s32 AudioDispatch(u32 Command,void *Data,s32 Size);

void    audio_DupState(cfx_state *pDest,cfx_state *pParent,cfx_state *pOurs);

extern iop_audio_vars g_Audio;
void *audio_Dispatch(u32 Command,void *Data,s32 Size);

xbool audio_IsLocked(void);
void audio_Lock(const char* Locker);
void audio_Unlock(void);

#endif // __IOPAUDIO_H