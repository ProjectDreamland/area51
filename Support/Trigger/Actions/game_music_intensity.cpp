///////////////////////////////////////////////////////////////////////////
//
//  game_music_intensity.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\game_music_intensity.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Objects\MusicLogic.hpp"

#include "Music_mgr\Music_mgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Entropy.hpp"

//=========================================================================
// MUSIC_INTENSITY
//=========================================================================

game_music_intensity::game_music_intensity ( guid ParentGuid ) : actions_base( ParentGuid )
{
    m_SuiteName[0]      = 0;
    m_TrackName[0]      = 0;
    m_IntensityLevel    = 0;
    m_SwitchMode        = music_mgr::SWITCH_NORMAL;
    m_VolumeFade        = 0.0f;
    m_FadeInTime        = 0.0f;
    MusicTriggerAction  = SET_INTENSITY;
    m_RunLogic          = TRUE;
}

//=============================================================================

void game_music_intensity::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * game_music_intensity::Execute" );
    
    (void) pParent;

x_try;
    
    //Play the sound..

#ifdef AUDIO_ENABLE
    
    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_MUSIC_LOGIC );

    if( SlotID == SLOT_NULL )
    {
#ifdef TARGET_PC
        x_throw( "No music logic object found\n" );
#endif
        return;
    }

    object* pObj            = g_ObjMgr.GetObjectBySlot( SlotID );    
    music_logic& MusicLogic = music_logic::GetSafeType( *pObj );

    if( x_strlen(m_SuiteName) != 0 )
    {
        xstring SuiteName( m_SuiteName );
        MusicLogic.SetDesiredNextSuiteByName( SuiteName, m_FadeInTime );
    }

    if( MusicTriggerAction == SWITCH_TO_TRACK )
    {
        xstring TrackName( m_TrackName );
        MusicLogic.SetDesiredNextTrackByName( TrackName, (music_mgr::SWITCHTYPE)m_SwitchMode, m_FadeInTime );
    }
    else if( MusicTriggerAction == INCREASE_INTENSITY )
    {
        MusicLogic.IncreaseIntensity( (music_mgr::SWITCHTYPE)m_SwitchMode );
    }
    else if( MusicTriggerAction == DECREASE_INTENSITY )
    {
        MusicLogic.DecreaseIntensity( (music_mgr::SWITCHTYPE)m_SwitchMode );
    }
    else if( MusicTriggerAction == SET_INTENSITY )
    {
        MusicLogic.SetTrackByIntensity( m_IntensityLevel, (music_mgr::SWITCHTYPE)m_SwitchMode );

        if( m_RunLogic )
            MusicLogic.EnableLogic();
        else
            MusicLogic.DisableLogic();
    }
    
    if( m_SwitchMode == music_mgr::SWITCH_WITH_VOLUME_FADE )
        MusicLogic.SetVolumeChangeTime( m_VolumeFade );
#endif

x_catch_display;

}

//=============================================================================

void game_music_intensity::OnEnumProp ( prop_enum& rPropList )
{

    //object info
    rPropList.AddString ( "Suite Name" ,        "Name of the suite in which the intensity will be played." );

    rPropList.AddEnum   ( "Music Action",      "SWITCH TO TRACK\0INCREASE INTENSITY\0DECREASE INTENSITY\0SET INTENSITY\0", 
                                                "The type action to use to switch between music intensity.", PROP_TYPE_MUST_ENUM );  
    

    if( MusicTriggerAction == SWITCH_TO_TRACK )
    {
        rPropList.AddString ( "Track Name" ,        "Name of the track with in the suite." );
    }
    else if( MusicTriggerAction == INCREASE_INTENSITY )
    {

    }
    else if( MusicTriggerAction == DECREASE_INTENSITY )
    {
    
    }
    else if( MusicTriggerAction == SET_INTENSITY )
    {
        rPropList.AddInt ( "Intensity Level",    "The level of intensity to play." );
        rPropList.AddBool( "Run Music Logic",    "Is it ok to run the music logic" );
    }
        
    rPropList.AddEnum   ( "Switch Mode",        "NORMAL\0BREAKPOINT\0VOLUME_FADE\0IMMEDIATE\0", 
                                                "The type of switch to use to switch between the music.", PROP_TYPE_MUST_ENUM );  
    
    rPropList.AddFloat(   "Fade In Time",        "The Fade in time for the next intensity or track" );
    
    if( m_SwitchMode == music_mgr::SWITCH_WITH_VOLUME_FADE )
        rPropList.AddFloat( "Volume Fade",      "The volume fade for the VOLUME_FADE switch" );
    
    actions_base::OnEnumProp( rPropList );
    
}

//=============================================================================

xbool game_music_intensity::OnProperty ( prop_query& rPropQuery )
{

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
     
    if ( rPropQuery.VarString ( "Suite Name"  , m_SuiteName,  64 ) )
        return TRUE;

    if( rPropQuery.IsVar( "Music Action" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( MusicTriggerAction )
            {
                case SWITCH_TO_TRACK    : rPropQuery.SetVarEnum( "SWITCH TO TRACK"      ); break;
                case INCREASE_INTENSITY : rPropQuery.SetVarEnum( "INCREASE INTENSITY"   ); break;
                case DECREASE_INTENSITY : rPropQuery.SetVarEnum( "DECREASE INTENSITY"   ); break;
                case SET_INTENSITY      : rPropQuery.SetVarEnum( "SET INTENSITY"        ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "SWITCH TO TRACK", rPropQuery.GetVarEnum()) )
            {
                MusicTriggerAction = SWITCH_TO_TRACK;
            }
            else if( !x_stricmp( "INCREASE INTENSITY", rPropQuery.GetVarEnum() ) )
            {
                MusicTriggerAction = INCREASE_INTENSITY;
            }
            else if( !x_stricmp( "DECREASE INTENSITY", rPropQuery.GetVarEnum() ) )
            {
                MusicTriggerAction = DECREASE_INTENSITY;
            }
            else if( !x_stricmp( "SET INTENSITY", rPropQuery.GetVarEnum() ) )
            {
                MusicTriggerAction = SET_INTENSITY;
            }
        }
        return TRUE;
    }    


    // The Type.
    if( rPropQuery.IsVar( "Switch Mode" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( m_SwitchMode )
            {
                case music_mgr::SWITCH_NORMAL           : rPropQuery.SetVarEnum( "NORMAL" ); break;
                case music_mgr::SWITCH_AT_BREAK         : rPropQuery.SetVarEnum( "BREAKPOINT" ); break;
                case music_mgr::SWITCH_WITH_VOLUME_FADE : rPropQuery.SetVarEnum( "VOLUME_FADE" ); break;
                case music_mgr::SWITCH_IMMEDIATE        : rPropQuery.SetVarEnum( "IMMEDIATE" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "NORMAL", rPropQuery.GetVarEnum()) )
            {
                m_SwitchMode = music_mgr::SWITCH_NORMAL;
            }
            else if( !x_stricmp( "BREAKPOINT", rPropQuery.GetVarEnum() ) )
            {
                m_SwitchMode = music_mgr::SWITCH_AT_BREAK;
            }
            else if( !x_stricmp( "VOLUME_FADE", rPropQuery.GetVarEnum() ) )
            {
                m_SwitchMode = music_mgr::SWITCH_WITH_VOLUME_FADE;
            }
            else if( !x_stricmp( "IMMEDIATE", rPropQuery.GetVarEnum() ) )
            {
                m_SwitchMode = music_mgr::SWITCH_IMMEDIATE;
            }
        }
        return TRUE;
    }    

    if ( rPropQuery.VarInt (  "Intensity Level"  , m_IntensityLevel ) )
        return TRUE;

    if ( rPropQuery.VarBool (  "Run Music Logic"  , m_RunLogic ) )
        return TRUE;
    
    if ( rPropQuery.VarString (  "Track Name"  , m_TrackName, 64 ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Fade In Time", m_FadeInTime ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Volume Fade", m_VolumeFade ) )
        return TRUE;
          
    return FALSE;
}


