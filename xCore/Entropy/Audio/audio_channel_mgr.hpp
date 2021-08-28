#ifndef AUDIO_CHANNEL_MGR_HPP
#define AUDIO_CHANNEL_MGR_HPP

#include "audio_private.hpp"

class audio_channel_mgr
{

//------------------------------------------------------------------------------
// Public functions.

public:

                        audio_channel_mgr   ( void );
                       ~audio_channel_mgr   ( void );
        
        void            Init                ( void );
        void            Kill                ( void );

        xbool           Acquire             ( element*      pElement );
        void            Release             ( channel*      pChannel );
                                             
        void            Start               ( channel*      pChannel );
        void            Pause               ( channel*      pChannel );
        void            Resume              ( channel*      pChannel );
        xbool           IsPlaying           ( channel*      pChannel );

        s32             GetPriority         ( channel*      pChannel );
        void            SetPriority         ( channel*      pChannel,
                                              s32           Priority );  

        f32             GetVolume           ( channel*      pChannel );
        void            SetVolume           ( channel*      pChannel,
                                              f32           Volume );

        void            GetPan              ( channel*      pChannel,
                                              vector4&      Pan);
        void            SetPan              ( channel*      pChannel,
                                              vector4&      Pan );

        f32             GetPitch            ( channel*      pChannel );
        void            SetPitch            ( channel*      pChannel,
                                              f32           Pitch );

        f32             GetEffectSend       ( channel*      pChannel );
        void            SetEffectSend       ( channel*      pChannel,
                                              f32           EffectSend );

        void            Update              ( void );

inline  channel*        FreeList            ( void )        { return &m_FreeChannels; }
inline  channel*        UsedList            ( void )        { return &m_UsedChannels; }
//------------------------------------------------------------------------------
// Private functions.

private:

        void            UpdatePriorityList  ( channel* pChannel, xbool RemoveFromFreeList );
        void            Free                ( channel* pChannel, xbool PutInFreeList, xbool FreeParent );

//------------------------------------------------------------------------------
// Private variables.
        
        channel         m_FreeChannels;
        channel         m_UsedChannels;
};

extern audio_channel_mgr g_AudioChannelMgr;

#endif
