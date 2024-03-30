///////////////////////////////////////////////////////////////////////////
//
//  action_music_intensity.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_music_intensity.hpp"

#include "Music_mgr\Music_mgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "MusicStateMgr\MusicStateMgr.hpp"
#include "Entropy.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"

//=========================================================================
// MUSIC_INTENSITY
//=========================================================================

action_music_intensity::action_music_intensity ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{        
    for( s32 i=0 ; i<NUM_MUSIC_STATES ; i++ )
    {
        m_Transitions[i].AudioPackage.SetName( "" );
     
        m_Transitions[i].TrackName[0]   = 0;
        m_Transitions[i].TransitionMode = music_mgr::SWITCH_END_OF_TRACK;
        m_Transitions[i].FadeInTime     = 0.0f;
        m_Transitions[i].FadeOutTime    = 0.0f;
        m_Transitions[i].DelayTime      = 0.0f;
    }
}

//=============================================================================

xbool action_music_intensity::Execute ( f32 DeltaTime )
{
    (void)DeltaTime;

x_try;
    
    // Default is looped.
    g_MusicStateMgr.ClearOneShot();
    
    // Note the transitions specified. 
    for( s32 i=0 ; i<NUM_MUSIC_STATES ; i++ )
    {
        // Is the track name specified?
        if( m_Transitions[i].TrackName[0] )
        {
            // Update the music state.
            g_MusicStateMgr.SetMusicEntry( i,
                                           m_Transitions[i].TrackName,
                                           m_Transitions[i].TransitionMode,
                                           m_Transitions[i].FadeOutTime,
                                           m_Transitions[i].DelayTime,
                                           m_Transitions[i].FadeInTime );

            if( i == ONESHOT_MUSIC )
                g_MusicStateMgr.SetOneShot();
        }
    }

    return TRUE;
    
x_catch_display;

    return (!RetryOnError());
}

//=============================================================================

void action_music_intensity::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.PropEnumString  ( "OneShot Package", "The audio package associated with this prop object.", PROP_TYPE_READ_ONLY );
    rPropList.PropEnumExternal( "OneShot Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumExternal( "OneShot Track", "Sound\0soundexternal\0","OneShot Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    rPropList.PropEnumEnum    ( "OneShot Segue", "END_OF_TRACK\0BREAKPOINT\0VOLUME_FADE\0IMMEDIATE\0", "The segue used when switching to this track.", PROP_TYPE_MUST_ENUM );  

    if( m_Transitions[ONESHOT_MUSIC].TransitionMode == music_mgr::SWITCH_VOLUME_FADE )
    {
        rPropList.PropEnumFloat( "OneShot Fade Out", "The fade out time for the current track", 0 );
        rPropList.PropEnumFloat( "OneShot Silence",  "The silence time before the fade in occurs", 0 );
        rPropList.PropEnumFloat( "OneShot Fade In",  "The fade in time for the new track", 0 );
    }

    rPropList.PropEnumString  ( "Ambient Package", "The audio package associated with this prop object.", PROP_TYPE_READ_ONLY );
    rPropList.PropEnumExternal( "Ambient Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumExternal( "Ambient Track", "Sound\0soundexternal\0","Ambient Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    rPropList.PropEnumEnum    ( "Ambient Segue", "END_OF_TRACK\0BREAKPOINT\0VOLUME_FADE\0IMMEDIATE\0", "The segue used when switching to this track.", PROP_TYPE_MUST_ENUM );  

    if( m_Transitions[AMBIENT_MUSIC].TransitionMode == music_mgr::SWITCH_VOLUME_FADE )
    {
        rPropList.PropEnumFloat( "Ambient Fade Out", "The fade out time for the current track", 0 );
        rPropList.PropEnumFloat( "Ambient Silence",  "The silence time before the fade in occurs", 0 );
        rPropList.PropEnumFloat( "Ambient Fade In",  "The fade in time for the new track", 0 );
    }

    rPropList.PropEnumString  ( "Alert Package", "The audio package associated with this prop object.", PROP_TYPE_READ_ONLY );
    rPropList.PropEnumExternal( "Alert Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumExternal( "Alert Track", "Sound\0soundexternal\0","Alert Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    rPropList.PropEnumEnum    ( "Alert Segue", "END_OF_TRACK\0BREAKPOINT\0VOLUME_FADE\0IMMEDIATE\0", "The segue used when switching to this track.", PROP_TYPE_MUST_ENUM );  

    if( m_Transitions[ALERT_MUSIC].TransitionMode == music_mgr::SWITCH_VOLUME_FADE )
    {
        rPropList.PropEnumFloat( "Alert Fade Out", "The fade out time for the current track", 0 );
        rPropList.PropEnumFloat( "Alert Silence",  "The silence time before the fade in occurs", 0 );
        rPropList.PropEnumFloat( "Alert Fade In",  "The fade in time for the new track", 0 );
    }

    rPropList.PropEnumString  ( "Combat Package", "The audio package associated with this prop object.", PROP_TYPE_READ_ONLY );
    rPropList.PropEnumExternal( "Combat Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumExternal( "Combat Track", "Sound\0soundexternal\0","Combat Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    rPropList.PropEnumEnum    ( "Combat Segue", "END_OF_TRACK\0BREAKPOINT\0VOLUME_FADE\0IMMEDIATE\0", "The segue used when switching to this track.", PROP_TYPE_MUST_ENUM );  

    if( m_Transitions[COMBAT_MUSIC].TransitionMode == music_mgr::SWITCH_VOLUME_FADE )
    {
        rPropList.PropEnumFloat( "Combat Fade Out", "The fade out time for the current track", 0 );
        rPropList.PropEnumFloat( "Combat Silence",  "The silence time before the fade in occurs", 0 );
        rPropList.PropEnumFloat( "Combat Fade In",  "The fade in time for the new track", 0 );
    }

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_music_intensity::OnProperty ( prop_query& rPropQuery )
{

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    // External
    if( rPropQuery.IsVar( "Ambient Track" ) || 
        rPropQuery.IsVar( "Combat Track"  ) || 
        rPropQuery.IsVar( "Alert Track"   ) || 
        rPropQuery.IsVar( "OneShot Track" ) )
    {
        s32 i;

        if( rPropQuery.IsVar( "Ambient Track" ) )
            i = AMBIENT_MUSIC;
        else if( rPropQuery.IsVar( "Alert Track" ) )
            i = ALERT_MUSIC;
        else if( rPropQuery.IsVar( "Combat Track" ) )
            i = COMBAT_MUSIC;
        else
            i = ONESHOT_MUSIC;

        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_Transitions[i].TrackName, 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = rPropQuery.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                if( String == "<null>" )
                {
                    m_Transitions[i].AudioPackage.SetName( "" );
                    m_Transitions[i].TrackName[0] = 0;
                }
                else
                {
                    s32 PkgIndex = String.Find( '\\', 0 );

                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );

                        m_Transitions[i].AudioPackage.SetName( Pkg );                

                        // Load the audio package.
                        if( m_Transitions[i].AudioPackage.IsLoaded() == FALSE )
                            m_Transitions[i].AudioPackage.GetPointer();
                    }

                    x_strncpy( m_Transitions[i].TrackName, String, 64 );
                }
            }
        }
        return( TRUE );
    }

    // External
    if( rPropQuery.IsVar( "Ambient Package External Resource" ) || 
        rPropQuery.IsVar( "Combat Package External Resource"  ) ||
        rPropQuery.IsVar( "Alert Package External Resource"   ) ||
        rPropQuery.IsVar( "OneShot Package External Resource" ) )
    {
        s32 i;

        if( rPropQuery.IsVar( "Ambient Package External Resource" ) )
            i = AMBIENT_MUSIC;
        else if( rPropQuery.IsVar( "Combat Package External Resource" ) )
            i = COMBAT_MUSIC;
        else if( rPropQuery.IsVar( "Alert Package External Resource" ) )
            i = ALERT_MUSIC;
        else
            i = ONESHOT_MUSIC;

        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_Transitions[i].AudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_Transitions[i].AudioPackage.SetName( "" );
                }
                else
                {
                    m_Transitions[i].AudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_Transitions[i].AudioPackage.IsLoaded() == FALSE )
                        m_Transitions[i].AudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );        
    }

    if( rPropQuery.IsVar( "Ambient Package" ) || 
        rPropQuery.IsVar( "Combat Package"  ) ||
        rPropQuery.IsVar( "Alert Package"   ) ||
        rPropQuery.IsVar( "OneShot Package" ) )
    {
        s32 i;
        if( rPropQuery.IsVar( "Ambient Package" ) )
            i = AMBIENT_MUSIC;
        else if( rPropQuery.IsVar( "Combat Package" ) )
            i = COMBAT_MUSIC;
        else if( rPropQuery.IsVar( "Alert Package" ) )
            i = ALERT_MUSIC;
        else
            i = ONESHOT_MUSIC;

        // You can only read.
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarString( m_Transitions[i].AudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        return TRUE;
    }

    // The Segue type.
    if( rPropQuery.IsVar( "Ambient Segue" ) || 
        rPropQuery.IsVar( "Combat Segue"  ) ||
        rPropQuery.IsVar( "Alert Segue"  ) ||
        rPropQuery.IsVar( "OneShot Segue" ) )
    {
        s32 i;
        if( rPropQuery.IsVar( "Ambient Segue" ) )
            i = AMBIENT_MUSIC;
        else if( rPropQuery.IsVar( "Combat Segue" ) )
            i = COMBAT_MUSIC;
        else if( rPropQuery.IsVar( "Alert Segue" ) )
            i = ALERT_MUSIC;
        else
            i = ONESHOT_MUSIC;

        if( rPropQuery.IsRead () )
        {
            switch( m_Transitions[i].TransitionMode )
            {
                case music_mgr::SWITCH_END_OF_TRACK:    rPropQuery.SetVarEnum( "END_OF_TRACK" ); break;
                case music_mgr::SWITCH_BREAKPOINT:      rPropQuery.SetVarEnum( "BREAKPOINT" );   break;
                case music_mgr::SWITCH_VOLUME_FADE:     rPropQuery.SetVarEnum( "VOLUME_FADE" );  break;
                case music_mgr::SWITCH_IMMEDIATE:       rPropQuery.SetVarEnum( "IMMEDIATE" );    break;
                default:                                ASSERTS( 0, "Didn't set the type" );     break;
            } 
        }
        else
        {
            if( !x_stricmp( "END_OF_TRACK", rPropQuery.GetVarEnum()) )
            {
                m_Transitions[i].TransitionMode = music_mgr::SWITCH_END_OF_TRACK;
            }
            else if( !x_stricmp( "BREAKPOINT", rPropQuery.GetVarEnum() ) )
            {
                m_Transitions[i].TransitionMode = music_mgr::SWITCH_BREAKPOINT;
            }
            else if( !x_stricmp( "VOLUME_FADE", rPropQuery.GetVarEnum() ) )
            {
                m_Transitions[i].TransitionMode = music_mgr::SWITCH_VOLUME_FADE;
            }
            else if( !x_stricmp( "IMMEDIATE", rPropQuery.GetVarEnum() ) )
            {
                m_Transitions[i].TransitionMode = music_mgr::SWITCH_IMMEDIATE;
            }
        }
        return TRUE;
    }    

    if( rPropQuery.VarFloat( "OneShot Fade Out", m_Transitions[ONESHOT_MUSIC].FadeOutTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "OneShot Silence", m_Transitions[ONESHOT_MUSIC].DelayTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "OneShot Fade In", m_Transitions[ONESHOT_MUSIC].FadeInTime ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Ambient Fade Out", m_Transitions[AMBIENT_MUSIC].FadeOutTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "Ambient Silence", m_Transitions[AMBIENT_MUSIC].DelayTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "Ambient Fade In", m_Transitions[AMBIENT_MUSIC].FadeInTime ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Alert Fade Out", m_Transitions[ALERT_MUSIC].FadeOutTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "Alert Silence", m_Transitions[ALERT_MUSIC].DelayTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "Alert Fade In", m_Transitions[ALERT_MUSIC].FadeInTime ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Combat Fade Out", m_Transitions[COMBAT_MUSIC].FadeOutTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "Combat Silence", m_Transitions[COMBAT_MUSIC].DelayTime ) )
        return TRUE;
    if( rPropQuery.VarFloat( "Combat Fade In", m_Transitions[COMBAT_MUSIC].FadeInTime ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_music_intensity::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Set Music Tracks");
    return Info.Get();
}

//=============================================================================


