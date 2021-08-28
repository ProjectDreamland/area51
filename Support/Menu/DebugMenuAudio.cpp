//==============================================================================
//  DebugMenuAudio.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu gameplay page.
//  
//==============================================================================

#include "DebugMenu2.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

extern xbool SHOW_STREAM_INFO;
extern xbool SHOW_AUDIO_LEVELS;
extern xbool SHOW_AUDIO_CHANNELS;
extern xbool DEBUG_PLAY_IDENTIFIER_NOT_FOUND;
extern xbool DEBUG_PLAY_ACQUIRE_VOICE_FAILED;
extern xbool DEBUG_PLAY_VOLUME_CLIPPED;
extern xbool DEBUG_PLAY_SUCCESS;
extern xbool DEBUG_ACQUIRE_CHANNEL_FAIL;
extern xbool DEBUG_FILTER_FOOTFALL;
extern xbool DEBUG_FILTER_VOX;
#ifndef X_RETAIL
extern xbool AUDIO_TWEAK;
#endif

//==============================================================================

debug_menu_audio::debug_menu_audio( ) : debug_menu_page()
{
    m_pTitle = "Audio";

    AddItemBool( "Stream Debug",                    SHOW_STREAM_INFO    );
#ifdef TARGET_PS2
    m_pEnableVUMeter = AddItemBool( "Audio Level Meter", SHOW_AUDIO_LEVELS   );
    AddItemBool( "Channel Monitor",                 SHOW_AUDIO_CHANNELS );
#endif // TARGET_PS2
#ifndef X_RETAIL
    AddItemBool( "Enable Audio Tweaks",             AUDIO_TWEAK   );
#endif
    AddItemSeperator( );
    AddItemBool( "Debug 'Identifier Not Found'",    DEBUG_PLAY_IDENTIFIER_NOT_FOUND );
    AddItemBool( "Debug 'Voice Acquire Failure'",   DEBUG_PLAY_ACQUIRE_VOICE_FAILED );
    AddItemBool( "Debug 'Volume Clipped'",          DEBUG_PLAY_VOLUME_CLIPPED       );
    AddItemBool( "Debug 'Success'",                 DEBUG_PLAY_SUCCESS              );
    AddItemBool( "Debug 'Channel Acquire Failure'", DEBUG_ACQUIRE_CHANNEL_FAIL      );
    AddItemSeperator( );
    AddItemBool( "'Footfall' Filter",               DEBUG_FILTER_FOOTFALL );
    AddItemBool( "'VOX & PainGrunt' Filter",        DEBUG_FILTER_VOX );
    AddItemSeperator( );
    m_pDumpLoadedPackages = AddItemButton( "Dump Loaded Packages to TTY" );    
}

//==============================================================================

void debug_menu_audio::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pDumpLoadedPackages )
    {
        g_AudioMgr.DisplayPackages();
    }

    if( pItem == m_pEnableVUMeter )
    {
#ifdef TARGET_PS2
        extern void EnableAudioLevels( xbool IsEnabled );
        EnableAudioLevels( SHOW_AUDIO_LEVELS );
#endif // TARGET_PS2
    }
}

//==============================================================================
#if 1
void debug_menu_audio::OnPreRenderActive( void )
{
    extern void AudioStats( f32 DeltaTime );
    AudioStats( 0.033f );
}
#endif
#endif // defined( ENABLE_DEBUG_MENU )
