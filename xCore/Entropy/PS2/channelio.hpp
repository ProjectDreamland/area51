#ifndef __CHANNELIO_HPP
#define __CHANNELIO_HPP

// This defines all the identical information required by IOP and EE
// communication for audio.

#define CHANNEL_RPC_DEV     0x12340000
#define MAX_CHANNELS        48

#define AUDIO_MODULE_VERSION_MAJOR (1)
#define AUDIO_MODULE_VERSION_MINOR (5)
#define AUDIO_MODULE_VERSION    ( (AUDIO_MODULE_VERSION_MAJOR << 8) | AUDIO_MODULE_VERSION_MINOR)

// Note: GET_VERSION is right after init so that if an old version of the module is attempted to be initialized, this
// will return a bad version id.
enum
{
    CHANCMD_INIT        = CHANNEL_RPC_DEV,
    CHANCMD_GET_VERSION,
    CHANCMD_PRE_INIT,
    CHANCMD_KILL,
    CHANCMD_UPDATE,
    CHANCMD_PUSHDATA,
    CHANCMD_RESIZE,
    CHANCMD_RELEASEMEMORY,
    CHANCMD_ENABLE_STATS,
    CHANCMD_PUSH_PCM,
};

enum iop_reverb_type
{
    REVERB_NONE,
    REVERB_ROOM,
    REVERB_STUDIO_A,
    REVERB_STUDIO_B,
    REVERB_STUDIO_C,
    REVERB_HALL,
    REVERB_SPACE,
    REVERB_ECHO,
    REVERB_DELAY,
    REVERB_PIPE,
};

enum iop_channel_status
{
    CHANSTAT_IDLE,
    CHANSTAT_STOPPED,
    CHANSTAT_PLAYING,
    CHANSTAT_WARMING,
    CHANSTAT_COOLING,
    CHANSTAT_STEALING,
    CHANSTAT_PAUSED
};

enum iop_channel_flags
{
    CHANFLAG_DIRTY  =   (1<<0),
};

struct iop_channel
{
    u16             m_Pitch;
    s16             m_LeftVolume;
    s16             m_RightVolume;
    u8              m_Flags;
    u8              m_Priority;
    u8              m_Reverb;
    u8              m_Status;
    u16             m_SampleRate;
    byte*           m_Base;
    byte*           m_LoopStart;
    byte*           m_LoopEnd;
    s32             m_Length;
    byte*           m_Current;
};

struct iop_audio_stats
{
    s16             SampleHighest;
    s16             SampleLowest;
};

union iop_channel_buffer
{
    struct
    {
        s16             MasterVolume;
        s16             PCMVolume;
        s16             ReverbVolume;
        s16             ReverbType;
        iop_channel     Channels[MAX_CHANNELS];
        iop_audio_stats Stats;
    } Update;
    u32     Longs[64];
};

#define CHAN_BUFFER_SIZE    (sizeof(iop_channel_buffer))

class xmesgq;

struct iop_push_request
{
    void*       pSource;
    void*       pDest;
    s32         Length;
    xmesgq*     pReplyQ;
};

#endif