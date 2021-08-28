#ifndef __XBOX_CHANNELMGR_HPP
#define __XBOX_CHANNELMGR_HPP

#include "x_files.hpp"
#include "channelio.hpp"

//bphold:***#if defined(TARGET_PS2_IOP) || !defined(TARGET_PS2)
//bphold:***#error "This should only be compiled for EE side IOP support. Check your dependencies"
//bphold:***#endif

struct xbox_au_channel_state
{
    f32                    m_Pan;
    f32                    m_Pitch;
    f32                    m_Volume;
    void*                  m_pSample;
    s32                    m_PlayPosition;
    s32                    m_Length;
    s32                    m_Rate;
    s32                    m_Priority;
    xbox_au_channel_status m_Status;
};

class xbox_au_channel_manager
{
public:
        void                    Init            ( void );
        void                    Init            ( s32 Length );
        void                    Kill            ( void );
        void                    Update          ( void );
        void                    SetVolume       ( s32 Channel, f32 Volume )                     { m_Channels[Channel].m_Volume  = Volume;   };
        void                    SetPan          ( s32 Channel, f32 Pan )                        { m_Channels[Channel].m_Pan     = Pan;      };
        void                    SetPitch        ( s32 Channel, f32 Pitch )                      { m_Channels[Channel].m_Pitch   = Pitch;    };
        void                    SetSample       ( s32 Channel, void* pSample )                  { m_Channels[Channel].m_pSample = pSample;  };
        void                    SetSampleRate   ( s32 Channel, s32 Rate )                       { m_Channels[Channel].m_Rate    = Rate;     };
        void                    SetLength       ( s32 Channel, s32 Length )                     { m_Channels[Channel].m_Length  = Length;   };
        void                    SetPriority     ( s32 Channel, s32 Priority )                   { m_Channels[Channel].m_Priority= Priority; };
        void                    SetState        ( s32 Channel, xbox_au_channel_status Status )  { m_Channels[Channel].m_Status  = Status;   };

        f32                     GetVolume       ( s32 Channel )                                 { return m_Channels[Channel].m_Volume;      };
        f32                     GetPan          ( s32 Channel )                                 { return m_Channels[Channel].m_Pan;         };
        f32                     GetPitch        ( s32 Channel )                                 { return m_Channels[Channel].m_Pitch;       };
        void*                   GetSample       ( s32 Channel )                                 { return m_Channels[Channel].m_pSample;     };
        s32                     GetSampleRate   ( s32 Channel )                                 { return m_Channels[Channel].m_Rate;        };
        s32                     GetLength       ( s32 Channel )                                 { return m_Channels[Channel].m_Length;      };
        s32                     GetPriority     ( s32 Channel )                                 { return m_Channels[Channel].m_Priority;    };
        s32                     GetPlayPosition ( s32 Channel )                                 { return m_Channels[Channel].m_PlayPosition;};
        xbox_au_channel_status  GetState        ( s32 Channel )                                 { return m_Channels[Channel].m_Status;      };

private:
        void*                   m_LocalData;
        xbox_au_channel_state   m_Channels[MAX_CHANNELS];
};

extern xbox_au_channel_manager g_ChannelManager;
#endif
