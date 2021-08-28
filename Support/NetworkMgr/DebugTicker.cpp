//==============================================================================
//
//  DebugTicker.cpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//  This provides us with a mechanism to halt all clients connected to a server
//  when (1) the server stops for any reason (breakpoint, exception etc) or (2)
//  when a client unexpectantly halts execution (breakpoint, exception, etc).
//  Very handy for multiplayer online games.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "DebugTicker.hpp"
#include "x_threads.hpp"
#include "x_log.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

static const char* REMOTE_ENABLED  = "ON";
static const char* REMOTE_DISABLED = "OFF";

//==============================================================================

void debug_ticker::Init( s32 Port )
{
    m_LocalSocket.Bind( Port );
    m_RemoteAddress.Clear();

    m_TickTimer         = 0.0f;
    m_LastTickTimer     = 0.0f;
    m_SkipTickCheck     = FALSE;
    m_RemoteIsListening = FALSE;
    m_WasNotified       = FALSE;

    LOG_WARNING( "debug_ticker::Init", "Debug ticker Initialized." );
}

//==============================================================================

void debug_ticker::Kill( void )
{
    s32 i;

    if( !m_RemoteAddress.IsEmpty() )
    {
        // Tell the other debug tick listener that we are no longer
        // going to send tick data to it. But do this several times just
        // in case the remote machine didn't catch the first packet.
        for( i=0; i<5; i++ )
        {
            m_LocalSocket.Send( m_RemoteAddress, REMOTE_DISABLED, 
                                x_strlen(REMOTE_DISABLED)+1 );
        }
        m_LocalSocket.Close();
        m_RemoteAddress.Clear();
        LOG_WARNING( "debug_ticker::Kill", "Debug ticker killed." );
    }
}

//==============================================================================

void debug_ticker::Update( f32 DeltaTime )
{
    // The Debug Tick functionality allows us to stall the client/server
    // if the server/client it is connected to is single stepping. The
    // DebugTick packets will be sent periodically.

    if( m_RemoteAddress.IsEmpty() )
        return;

    s32         Dummy;
    net_address Remote;
    char        TextBuff[ 128 + 1024 ];     // HACKOMOTRON *********************
    xbool       SayWeResumed;

    xbool       WaitForTick;
    f32         SendTimeout;
    s32         Count;

    Count        = 0;
    SayWeResumed = FALSE;

    xthread* pCurrent = x_GetCurrentThread();

    // We're going to send a packet every 100ms.  However, if we need to wait 
    // for a debug tick to come in from another source, then we wait a bit 
    // longer before sending another pulse.
    SendTimeout = 0.1f;
    do
    {  
        m_TickTimer     += DeltaTime;
        m_LastTickTimer += DeltaTime;

        Dummy = sizeof( TextBuff );
        if( m_LocalSocket.Receive( Remote, TextBuff, Dummy ) )
        {
            #ifdef VERBOSE_DEBUG_TICKS
            LOG_MESSAGE( "debug_ticker::Update", 
                         "Received debug tick from %s.", 
                         Remote.GetStrAddress() );
            #endif

            do
            {
                ASSERT( Dummy < 1024 );     // HACKOMOTRON *********************

                if( Remote == m_RemoteAddress )
                {
                    m_LastTickTimer = 0.0f;
                }

                if( x_strcmp( TextBuff, REMOTE_ENABLED ) == 0 )
                {
                    if( !m_RemoteIsListening )
                    {
                        LOG_WARNING( "debug_ticker::Update", 
                                     "Debug ticker enabled." );
                    }
                    m_RemoteIsListening = TRUE;
                }

                if( x_strcmp( TextBuff, REMOTE_DISABLED ) == 0 )
                {
                    if( m_RemoteIsListening )
                    {
                        LOG_WARNING( "debug_ticker::Update",
                                     "Debug ticker disabled.");
                    }
                    m_RemoteIsListening = FALSE;
                }

                Dummy = sizeof( TextBuff );
            } while( m_LocalSocket.Receive( Remote, TextBuff, Dummy ) );
        }

        if(    (m_RemoteAddress.IsEmpty() == FALSE)
            && (m_TickTimer > SendTimeout) )
        {
            // First, send a packet on the debug tick port.
            interface_info  Info;

            net_GetInterfaceInfo( -1, Info );

            // Only enable debug ticker if the remote machine is on the same 
            // network. This will allow us to debug machines outside of the 
            // local net.
#if defined(TARGET_PS2)
            if(    (m_RemoteAddress.GetIP() & Info.Netmask) 
                == (Info.Address & Info.Netmask) )
#endif
            {
                m_LocalSocket.Send( m_RemoteAddress, REMOTE_ENABLED, 
                                    x_strlen(REMOTE_ENABLED)+1 );
                #ifdef VERBOSE_DEBUG_TICKS
                LOG_MESSAGE( "debug_ticker::Update", 
                             "Sending debug tick to %s through %s.",
                             m_RemoteAddress.GetStrAddress(), 
                             m_LocalSocket.GetStrAddress() );
                #endif
            }
#if defined(TARGET_PS2)
            else
            {
                if( m_WasNotified == FALSE )
                {
                    LOG_WARNING( "debug_ticker::Update",
                                "Remote machine not on local subnet. Ticker disabled." );
                    m_WasNotified = TRUE;
                }
            }
#endif
            m_TickTimer = 0.0f;
        }
        WaitForTick = (m_LastTickTimer > 0.400f) && m_RemoteIsListening;
        if( WaitForTick )
        {
            if( m_SkipTickCheck )
                break;
            Count--;
            if( Count < 0 )
            {
                Count = 2000;
                LOG_WARNING( "debug_ticker::Update",
                             "Paused waiting for a debug tick from %s.",
                             m_RemoteAddress.GetStrAddress() );
                SayWeResumed = TRUE;
            }
            x_DelayThread( 10 );
            // DeltaTime will be 10ms.
            DeltaTime   = 0.01f;
            SendTimeout = 0.50f;
            LOG_FLUSH();
        }
        if( pCurrent->IsActive()==FALSE )
        {
            break;
        }
    } while( WaitForTick );

    if( SayWeResumed )
    {
        LOG_WARNING( "debug_ticker::Update", 
                     "Received debug tick. Resuming." );
    }
}

//==============================================================================

void debug_ticker::ProvideUpdate( bitstream& BitStream )
{
    BitStream.WriteU16( m_LocalSocket.GetPort() );
}

//==============================================================================

xbool debug_ticker::ReceiveUpdate( const net_address& Remote, 
                                   bitstream& BitStream )
{
    u16 Port;
    BitStream.ReadU16( Port );
    if( Port )
    {
        m_RemoteAddress.Setup( Remote.GetIP(), Port );
    }

    return( TRUE );
}

//==============================================================================
