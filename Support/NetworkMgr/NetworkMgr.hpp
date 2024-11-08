//==============================================================================
//
//  NetworkMgr.hpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#ifndef NETWORK_MGR_HPP
#define NETWORK_MGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "e_Network.hpp"

//==============================================================================
//  ANNOUNCEMENTS
//==============================================================================
class   game_server;          // Announce game_server class
class   game_client;          // Announce game_client class
class   voice_proxy;

class   pain_queue;           // Announce the pain_queue class
struct  server_info;         // Announce server info structure

//==============================================================================
//  DEFINITIONS
//==============================================================================

//**NOTE** This port # is defined by IANA as the PlayStation AMS (Secure) port.
const s32 NET_GAME_PORT = 3658;

//==============================================================================
//  TYPES
//==============================================================================
enum osd_destination
{
    OSD_EXIT_MANAGE,            // Run network configuration tool or troubleshooter
    OSD_EXIT_NEW_USER,          // XBOX: New user account needed
    OSD_EXIT_UPDATE,            // XBOX: Required update available
    OSD_EXIT_MESSAGE,           // XBOX: Required message available
    OSD_EXIT_REBOOT,
};

//==============================================================================
//  FUNCTIONS
//==============================================================================
class network_mgr 
{
public:
                        network_mgr         ( void );
                       ~network_mgr         ( void );

        void            Init                ( void );
        void            Kill                ( void );
        void            Update              ( f32 DeltaTime );
        void            SetOnline           ( xbool IsOnline );
        xbool           IsOnline            ( void )                            { return m_IsOnline; };
        void            BecomeServer        ( void );
        void            BeginLogin          ( void );
        void            ReenterGame         ( void );
        void            Silence             ( void )                            { Silence(FALSE);                       }
        void            Disconnect          ( void );
        xbool           IsServer            ( void )                            { return (m_pServer != NULL);           }
        xbool           IsClient            ( void )                            { return (m_pClient != NULL);           }
        xbool           IsLocal             ( s32 NetSlot );
        xbool           IsLoggedIn          ( void );
        xbool           IsRequestingMission ( void );
        void            KickPlayer          ( s32 Index );
        s32             GetLocalPlayerCount ( void )                            { return m_LocalPlayerCount;            }
        void            SetLocalPlayerCount ( s32 Count )                       { m_LocalPlayerCount = Count;           }
        s32             GetLocalPlayerSlot  ( s32 Index );
        s32             GetLocalSlot        ( s32 NetSlot );
        s32             GetClientIndex      ( void );
        s32             GetClientIndex      ( s32 PlayerIndex );
        s32             GetClientPlayerSlot ( s32 ClientIndex, s32 PlayerIndex );
        s32             GetClientPlayerCount( s32 ClientIndex );
        f32             GetClientPing       ( s32 ClientIndex );
        f32             GetAveragePing      ( void );
        pain_queue*     GetPainQueue        ( void );

        net_socket&     GetSocket           ( void )                            { return m_LocalSocket;                 }
        const char*     GetStateName        ( void );
        void            SimpleDialog        ( const char* pText, f32 Timeout = 0.0f);
        void            LoadMissionComplete ( void );
        voice_proxy&    GetVoiceProxy       ( s32 ClientIndex );
        game_server&    GetServerObject     ( void )                            { ASSERT( m_pServer ); return *m_pServer;}
#ifdef TARGET_XBOX
        game_client&    GetClientObject     ( void )                            { ASSERT(m_pClient); return *m_pClient; }
#else
        game_client&    GetClientObject     ( void )                            { return *m_pClient; }
#endif
        xbool           IsServerABuddy      ( const char* pSearch );
        xbool           HasPlayerBuddy      ( const char* pSearch );

#ifdef TARGET_PS2
        void            OnRepollCB          ( void );
#endif

private:
        void            Silence             ( xbool ForceDisconnect );
        xbool           m_IsOnline;
        net_socket      m_LocalSocket;
        game_client*    m_pClient;
        game_server*    m_pServer;

        s32             m_LocalPlayerCount;
};

extern network_mgr g_NetworkMgr;

#endif // NETWORK_MGR_HPP






