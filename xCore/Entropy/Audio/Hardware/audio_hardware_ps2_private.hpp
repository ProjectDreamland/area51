#ifndef AUDIO_HARDWARE_PS2_PRIVATE_HPP
#define AUDIO_HARDWARE_PS2_PRIVATE_HPP
#include "x_types.hpp"
#if !defined(TARGET_PS2)
#error This is only for the PS2 target
#endif

#include "ps2/channelio.hpp"
#include "x_threads.hpp"

#define INVALID_CHANNEL (-1)
struct iop_channel_state
{
    f32                 m_LeftVolume;
    f32                 m_RightVolume;
    f32                 m_Pitch;
    f32                 m_Volume;
    f32                 m_Reverb;
    byte*               m_pSample;
    byte*               m_pLoopStart;
    byte*               m_pLoopEnd;
    byte*               m_PlayPosition;
    byte*               m_RelativePlayPosition;
    s32                 m_Length;
    u16                 m_Rate;
    u8                  m_Priority;
    u8                  m_Flags;
    xbool               m_StateDirty;
    iop_channel_status  m_Status;
    void*               m_pOwner;
    xbool               m_bPitchLock;
};

class iop_channel_manager
{
public:
                            iop_channel_manager(void) : m_qPushPending(MAX_DEFAULT_MESSAGES),m_qPushAvail(MAX_DEFAULT_MESSAGES)
                            {
                            }
        void                Init            ( void );
        s32                 Init            ( s32 Length );
        void                Kill            ( void );
        s32                 ResizeMemory    ( s32 NewSize );
        void                SetVolume       ( s32 Channel, f32 Left, f32 Right )        { m_Channels[Channel].m_LeftVolume  = Left;
                                                                                          m_Channels[Channel].m_RightVolume = Right;};
        void                SetReverb       ( s32 Channel, f32 Reverb )                 { m_Channels[Channel].m_Reverb  = Reverb;   };
        void                SetPitch        ( s32 Channel, f32 Pitch )                  { m_Channels[Channel].m_Pitch   = Pitch;    };
        void                SetSample       ( s32 Channel, byte* pSample )              { m_Channels[Channel].m_pSample = pSample;  };
        void                SetSampleRate   ( s32 Channel, s32 Rate )                   { m_Channels[Channel].m_Rate    = Rate;     };
        void                SetLength       ( s32 Channel, s32 Length )                 { m_Channels[Channel].m_Length  = Length;   };
        void                SetPriority     ( s32 Channel, s32 Priority )               { m_Channels[Channel].m_Priority= Priority; };
        void                SetLoopStart    ( s32 Channel, byte* pLoopStart)            { m_Channels[Channel].m_pLoopStart=pLoopStart;};
        void                SetLoopEnd      ( s32 Channel, byte* pLoopEnd)              { m_Channels[Channel].m_pLoopEnd = pLoopEnd;};
        void                SetState        ( s32 Channel, iop_channel_status Status );
        void                SetOwner        ( s32 Channel, void* pOwner )               { m_Channels[Channel].m_pOwner = pOwner;};  
        void                SetPitchLock    ( s32 Channel, xbool bPitchLock )           { m_Channels[Channel].m_bPitchLock = bPitchLock;};  

        void                SetMasterVolume ( f32 Volume )                              { m_MasterVolume = Volume;                  };
        f32                 GetMasterVolume ( void )                                    { return m_MasterVolume;                    };
        f32                 GetPCMVolume    ( void )                                    { return m_PCMVolume;                       };
        void                SetPCMVolume    ( f32 Volume )                              { m_PCMVolume = Volume;                     };
        iop_reverb_type     GetReverbType   ( void )                                    { return m_ReverbType;                      };
        void                SetReverbType   ( iop_reverb_type Reverb, f32 Volume )      { m_ReverbType = Reverb; m_ReverbVolume = Volume; };

        f32                 GetVolume       ( s32 Channel )                             { return (m_Channels[Channel].m_LeftVolume + m_Channels[Channel].m_RightVolume)/2.0f;      };
        f32                 GetPitch        ( s32 Channel )                             { return m_Channels[Channel].m_Pitch;       };
        byte*               GetSample       ( s32 Channel )                             { return m_Channels[Channel].m_pSample;     };
        s32                 GetSampleRate   ( s32 Channel )                             { return m_Channels[Channel].m_Rate;        };
        s32                 GetLength       ( s32 Channel )                             { return m_Channels[Channel].m_Length;      };
        s32                 GetPriority     ( s32 Channel )                             { return m_Channels[Channel].m_Priority;    };
        byte*               GetPlayPosition ( s32 Channel )                             { return m_Channels[Channel].m_PlayPosition;};
        byte*               GetRelativePlayPosition ( s32 Channel )                     { return m_Channels[Channel].m_RelativePlayPosition;};
        iop_channel_status  GetState        ( s32 Channel )                             { return m_Channels[Channel].m_Status;      };
        void*               GetOwner        ( s32 Channel )                             { return m_Channels[Channel].m_pOwner;      };
        xbool               GetPitchLock    ( s32 Channel )                             { return m_Channels[Channel].m_bPitchLock;  };  

        void                PushData        ( void* SrcAddr, void* DstAddr, s32 Length,xmesgq* pReplyQ);
        void                PushPCMData     ( void* pSource, s32 Length, s32 Frequency );
        s32                 PushAvail       ( void );

        void                GetStats        ( s32& Lowest, s32& Highest );
        void                EnableStats     ( xbool IsEnabled );
private:
        void                Update          ( void );
        void*               m_LocalData;
        f32                 m_MasterVolume;
        f32                 m_PCMVolume;
        f32                 m_ReverbVolume;
        iop_reverb_type     m_ReverbType;
        iop_channel_state   m_ChannelNegativeOne; // TODO: E3: CJ: This is here just to catch -1 index overwrite to the array below
        iop_channel_state   m_Channels[MAX_CHANNELS];
        iop_audio_stats     m_Stats;
        xmesgq              m_qPushPending;
        xmesgq              m_qPushAvail;
        iop_push_request    m_PushRequests[MAX_DEFAULT_MESSAGES];
#if !defined(X_RETAIL)
        s32                 m_ActiveChannels;
        s32                 m_ChannelsStealing;
#endif

        friend  void        s_ChannelPeriodicUpdate(void);
        friend  class       audio_hardware;

};

extern iop_channel_manager  s_ChannelManager;


#endif
