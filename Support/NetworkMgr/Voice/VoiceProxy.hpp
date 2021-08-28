//==============================================================================
//
//  VoiceProxy.hpp
// Client instance depository for voice data going out to this specific
// client.
//
//==============================================================================

#ifndef VOICEPROXY_HPP
#define VOICEPROXY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "Network/NetStream.hpp"
#include "Network/FiFo.hpp"
#include "x_threads.hpp"
#include "VoiceMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================


//==============================================================================
//  TYPES
//==============================================================================
class voice_proxy
{
public:
                            voice_proxy         ( void );
                           ~voice_proxy         ( void );

        void                Init                ( s32 PlayerNetSlot );
        void                Kill                ( void );
        void                Update              ( f32 DeltaTime );

        void                ProvideUpdate       ( netstream& BitStream );
        void                AcceptUpdate        ( netstream& BitStream );
        s32                 Read                ( byte* pBuffer, s32 MaxLength );
        void                Write               ( const byte* pBuffer, s32 Length );
        void                SetPlayerSlot       ( s32 PlayerSlot );
        s32                 GetPlayerSlot       ( void );
        void                SetTalkMode         ( s32 Mode );
        void                SetVoiceOwner       ( s32 PlayerSlot, actual_talk_mode TalkMode );
        s32                 GetVoiceOwner       ( void );
        f32                 GetOwnerTimeout     ( void );
        desired_talk_mode   GetDesiredTalkMode  ( void ) { return m_DesiredTalkMode; };
        u32                 GetMutedPlayers     ( void ) { return m_MutedPlayers; };

private:
        xbool               m_Initialized;

        s32                 m_VoiceOwner;
        actual_talk_mode    m_VoiceTalkMode;
        u32                 m_MutedPlayers;

        s32                 m_PlayerNetSlot;

        desired_talk_mode   m_DesiredTalkMode;

        fifo                m_Outgoing;
        fifo                m_Incoming;
        byte                m_OutgoingBuffer[256];
        byte                m_IncomingBuffer[256];
};


//==============================================================================
#endif // VOICEMGR_HPP
//==============================================================================
