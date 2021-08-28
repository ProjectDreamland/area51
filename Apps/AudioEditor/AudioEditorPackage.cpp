//=========================================================================
// AUDIO EDITOR PACKAGE.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "AudioEditorPackage.hpp"
#include "AudioEditor.hpp"
#include "Resource.h"
#include "FaderDialog.h"
#include "..\EDRscDesc\RSCDesc.hpp"
#include "..\Editor\Project.hpp"
#include "..\Support\ResourceMgr\ResourceMgr.hpp"

//=========================================================================
// LOCAL
//=========================================================================
DEFINE_RSC_TYPE( s_Desc, editor_audio_package, "audiopkg", "Sound Packages Files (*.audiopkg)|*.audiopkg", "Add editor_descriptors to audio packages." );

//=========================================================================
// EXTERNALS
//=========================================================================

char* g_pFaderList;

//=========================================================================
// FUNCTIONS
//=========================================================================

void WriteBaseParameters(   f32 Volume, f32 VolumeVar, f32 VolumeCenter, f32 VolumeLFE, f32 Pitch, f32 PitchVar, 
                            f32 Pan, s32 Priority, f32 EffectSend, f32 NearFalloff, f32 FarFalloff, 
                            u8 RollOff, f32 m_NearDiffuse, f32 m_FarDiffuse, s32 m_PlayPercent, f32 m_ReplayDelay,
                            xstring& WriteBuffer)
{
    // We do this check so if we don't have any variables to add then don't add the braces ( [] ) to the string.
    if( (Volume != 1.0f) || (VolumeVar != 0.0f) || (VolumeCenter != 0.0f ) || (VolumeLFE != 0.0f ) || (Pitch != 1.0f) || 
        (PitchVar != 0.0f) || (Pan != 0.0f) || (Priority != 128) || (EffectSend != 1.0f) || (NearFalloff != 1.0f) || 
        (FarFalloff != 1.0f) || !(RollOff & LINEAR_ROLLOFF) || (m_NearDiffuse != 1.0f) || (m_FarDiffuse != 1.0f) ||
        ( m_PlayPercent != 100 ) || (m_ReplayDelay != 0.0f) )
    {
        WriteBuffer += " [";
    
        if( Volume != 1.0f )
        {
            WriteBuffer += " volume=";
            WriteBuffer += (const char*)xfs("%g ", Volume);
        }

        if( VolumeVar != 0.0f )
        {
            WriteBuffer += " volumevar=";
            WriteBuffer += (const char*)xfs("%g ", VolumeVar);
        }

        if( VolumeCenter != 0.0f )
        {
            WriteBuffer += " volcenter=";
            WriteBuffer += (const char*)xfs("%g ", VolumeCenter);
        }

        if( VolumeLFE != 0.0f )
        {
            WriteBuffer += " vollfe=";
            WriteBuffer += (const char*)xfs("%g ", VolumeLFE);
        }

        if( Pitch != 1.0f )
        {
            WriteBuffer += " pitch=";
            WriteBuffer += (const char*)xfs("%g ", Pitch);
        }

        if( PitchVar != 0.0f )
        {
            WriteBuffer += " pitchvar=";
            WriteBuffer += (const char*)xfs("%g ", PitchVar);
        }

        if( Pan != 0.0f )
        {
            WriteBuffer += " pan=";
            WriteBuffer += (const char*)xfs("%g ", Pan);
        }

        if( Priority != 128 )
        {
            WriteBuffer += " priority=";
            WriteBuffer += (const char*)xfs("%d ", Priority);
        }

        if( EffectSend != 1.0f )
        {
            WriteBuffer += " effect=";
            WriteBuffer += (const char*)xfs("%g ", EffectSend);
        }

        if( NearFalloff != 1.0f )
        {
            WriteBuffer += " nearclip=";
            WriteBuffer += (const char*)xfs("%g ", NearFalloff);
        }

        if( FarFalloff != 1.0f )
        {
            WriteBuffer += " farclip=";
            WriteBuffer += (const char*)xfs("%g ", FarFalloff);
        }
        
        if( !(RollOff & LINEAR_ROLLOFF) )
        {
            WriteBuffer += " rolloff=";

            if( RollOff & FAST_ROLLOFF )
                WriteBuffer += "fast ";
            else if( RollOff & SLOW_ROLLOFF )
                WriteBuffer += "slow ";
            else
                ASSERT( 0 );
        }

        if( m_NearDiffuse != 1.0f )
        {
            WriteBuffer += " neardiff=";
            WriteBuffer += (const char*)xfs("%g ", m_NearDiffuse);
        }

        if( m_FarDiffuse != 1.0f )
        {
            WriteBuffer += " fardiff=";
            WriteBuffer += (const char*)xfs("%g ", m_FarDiffuse);
        }

        if( m_PlayPercent != 100 )
        {
            WriteBuffer += " playpercent=";
            WriteBuffer += (const char*)xfs("%d ", m_PlayPercent);
        }

        if( m_ReplayDelay != 0.0f )
        {
            WriteBuffer += " replaydelay=";
            WriteBuffer += (const char*)xfs("%g ", m_ReplayDelay);
        }

        WriteBuffer += "] ";
    }
}
    
//=========================================================================
// AUDIO PACKAGE
//=========================================================================

editor_audio_package::editor_audio_package( void ) : rsc_desc( s_Desc ) 
{    
    m_ElementSelected       = -1;
    m_DescriptorSelected    = -1;
    m_Selected              = 0;
    m_PrevSelected          = 0;
    m_IntensitySelected     = -1;

    m_Volume                = 1.0f;
    m_VolumeVar             = 0.0f;
    m_Pitch                 = 1.0f;
    m_PitchVar              = 0.0f;
    m_Pan                   = 0.0f;
    m_Priority              = 128;
    m_EffectSend            = 1.0f;
    m_NearFalloff           = 1.0f;
    m_FarFalloff            = 1.0f;
    m_DescCount             = 0;
    m_VolumeLfe             = 0.0f;
    m_VolumeCenter          = 0.0f;
    m_RollOff               |= LINEAR_ROLLOFF;
    m_NearDiffuse           = 1.0f;
    m_FarDiffuse            = 1.0f;
    m_PlayPercent           = 100;
    m_ReplayDelay           = 0.0f;

    m_pVirtualDirectory[0]  = 0;
    m_pType[0]              = 0;

    x_strncpy( m_pVolumeFader,      "NONE\0", 128 );
    x_strncpy( m_pPitchFader,       "NONE\0", 128 );
    x_strncpy( m_pNearFalloffFader, "NONE\0", 128 );
    x_strncpy( m_pFarFalloffFader,  "NONE\0", 128 );
    x_strncpy( m_pEffectSendFader,  "NONE\0", 128 );

    m_pDescriptorList.SetGrowAmount( 16 );
    m_pIntensity.SetGrowAmount( 4 );

    m_RebuildPackage = FALSE;
    m_PackageLoaded  = FALSE;

    g_pFaderList = g_String.GetBuffer( g_String.GetLength() );
}

//=========================================================================

editor_audio_package::~editor_audio_package( void )
{

}

//=========================================================================

void editor_audio_package::OnEnumProp( prop_enum& List)
{
    rsc_desc::OnEnumProp( List );

    // The packages properties.
    List.PropEnumFloat ( "ResDesc\\Volume",          "Volume of the audio package, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Volume Var",      "Volume Variance of the audio package, Range( 0.0 -- 1.0 )", 0 );

    List.PropEnumFloat ( "ResDesc\\Volume LFE",      "Volume LFE of the audio package, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Volume Center",   "Volume Center of the audio package, Range( 0.0 -- 1.0 )", 0 );

    List.PropEnumFloat ( "ResDesc\\Pitch",           "The pitch of the editor_descriptor, Range( 2^-6 -- 2^2 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Pitch Var",       "Pitch variance of the audio package, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Pan",             "Pan of the audio package, Range( -1.0 -- 1.0 )", 0 );
    List.PropEnumInt   ( "ResDesc\\Priority",        "Priority of the editor_descriptor, Range( 0 -- 255)", 0 );
    List.PropEnumFloat ( "ResDesc\\EffectSend",      "Reverb of the audio package, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Near Falloff",    "Near falloff of the audio package, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Far falloff",     "Far falloff of the audio package, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Near Diffuse",    "Near diffuse of the audio package, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( "ResDesc\\Far Diffuse",     "Far diffuse of the audio package, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumInt   ( "ResDesc\\Play Percent",    "Percent descriptor will actually play, Range( 0[never] -- 100[always] )", 0 );
    List.PropEnumFloat ( "ResDesc\\Replay Delay",    "Minimum time in seconds before playing again, Range( 0.1 -- 100.0 )", 0 );
    List.PropEnumEnum  ( "ResDesc\\Rolloff Curve",   "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );


    List.PropEnumInt   ( "ResDesc\\DescCount",       "The count of the editor_descriptor inside the package.", PROP_TYPE_READ_ONLY );

    List.PropEnumInt    ( "ResDesc\\IntensitySaveCount",  "", PROP_TYPE_DONT_SHOW );
    List.PropEnumInt    ( "ResDesc\\DescSaveCount",       "", PROP_TYPE_DONT_SHOW );
    List.PropEnumString ( "ResDesc\\Virtual Dir",         "", PROP_TYPE_DONT_SHOW );
        
    List.PropEnumEnum   ( "ResDesc\\Volume Fader",         g_pFaderList, "Volume fader for the package.", 0 );
    List.PropEnumEnum   ( "ResDesc\\Pitch Fader",          g_pFaderList, "Pitch fader for the package.", 0 );
    List.PropEnumEnum   ( "ResDesc\\Near Falloff Fader",   g_pFaderList, "NearFalloff fader for the package.", 0 );
    List.PropEnumEnum   ( "ResDesc\\Far Falloff Fader",    g_pFaderList, "FarFaroff fader for the package.", 0 );
    List.PropEnumEnum   ( "ResDesc\\EffectSend Fader",     g_pFaderList, "EffectSend fader for the package.", 0 );
    
    List.PropEnumString ( "ResDesc\\Type",           "The type of package (Used for music)", 0 );
    
    s32 i = 0;
    for( i = 0; i < m_pDescriptorList.GetCount(); i++ )
    {
        m_pDescriptorList[i].OnEnumProp( List, (const char*)xfs( "ResDesc\\Descriptor[%d]", i) );
    }   

    for( i = 0; i < m_pIntensity.GetCount(); i++ )
    {
        m_pIntensity[i].OnEnumProp( List, (const char*)xfs( "ResDesc\\Intensity[%d]", i) );
    }   
}

//=========================================================================

void editor_audio_package::OnEditorEnumProp( prop_enum& List)
{
    if( m_Selected & PACKAGE_SELECTED )
    {
		rsc_desc::OnEnumProp( List );

        // The packages properties.
        List.PropEnumFloat ( "ResDesc\\Volume",          "Volume of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Volume Var",      "Volume Variance of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );

        List.PropEnumFloat ( "ResDesc\\Volume LFE",      "Volume LFE of the audio package, Range( 0.0 -- 1.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Volume Center",   "Volume Center of the audio package, Range( 0.0 -- 1.0 )", 0 );

        List.PropEnumFloat ( "ResDesc\\Pitch",           "The pitch of the editor_descriptor, Range( 2^-6 -- 2^2 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Pitch Var",       "Pitch variance of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Pan",             "Pan of the sound editor_descriptor, Range( -1.0 -- 1.0 )", 0 );
        List.PropEnumInt   ( "ResDesc\\Priority",        "Priority of the editor_descriptor, Range( 0 -- 255)", 0 );
        List.PropEnumFloat ( "ResDesc\\EffectSend",      "Reverb of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Near Falloff",    "Near falloff of the sound editor_descriptor, Range( 0.0 -- 10.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Far falloff",     "Far falloff of the sound editor_descriptor, Range( 0.0 -- 10.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Near Diffuse",    "Near diffuse of the audio package, Range( 0.0 -- 10.0 )", 0 );
        List.PropEnumFloat ( "ResDesc\\Far Diffuse",     "Far diffuse of the audio package, Range( 0.0 -- 10.0 )", 0 );
        List.PropEnumInt   ( "ResDesc\\Play Percent",    "Percent descriptor will actually play, Range( 0[never] -- 100[always] )", 0 );
        List.PropEnumFloat ( "ResDesc\\Replay Delay",    "Minimum time in seconds before playing again, Range( 0.1 -- 100.0 )", 0 );
        List.PropEnumEnum  ( "ResDesc\\Rolloff Curve",   "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );

        List.PropEnumInt   ( "ResDesc\\DescCount",       "The count of the editor_descriptor inside the package.", PROP_TYPE_READ_ONLY );
    
        List.PropEnumInt    ( "ResDesc\\IntensitySaveCount",  "", PROP_TYPE_DONT_SHOW );
        List.PropEnumInt    ( "ResDesc\\DescSaveCount",       "", PROP_TYPE_DONT_SHOW );
        List.PropEnumString ( "ResDesc\\Virtual Dir",         "", PROP_TYPE_DONT_SHOW );
    
        List.PropEnumEnum   ( "ResDesc\\Volume Fader",         g_pFaderList, "Volume fader for the package.", 0 );
        List.PropEnumEnum   ( "ResDesc\\Pitch Fader",          g_pFaderList, "Pitch fader for the package.", 0 );
        List.PropEnumEnum   ( "ResDesc\\Near Falloff Fader",   g_pFaderList, "NearFalloff fader for the package.", 0 );
        List.PropEnumEnum   ( "ResDesc\\Far Falloff Fader",    g_pFaderList, "FarFaroff fader for the package.", 0 );
        List.PropEnumEnum   ( "ResDesc\\EffectSend Fader",     g_pFaderList, "EffectSend fader for the package.", 0 );

        List.PropEnumString ( "ResDesc\\Type",           "The type of package (Used for music)", 0 );

//        for( s32 i = 0; i < m_pDescriptorList.GetCount(); i++ )
//        {
//            m_pDescriptorList[i].OnEnumProp( List, (const char*)xfs( "ResDesc\\Descriptor[%d]", i) );
//        }       
    
    }   
    else if( m_Selected & DESCRIPTOR_SELECTED )        
    {
        if( m_DescriptorSelected == -1 )
            return;

        m_pDescriptorList[m_DescriptorSelected].OnEnumProp( List, xfs("Descriptor[%d]", m_DescriptorSelected) );
    }
    else if( m_Selected & ELEMENT_SELECTED )
    {
        if( m_ElementSelected == -1 || m_DescriptorSelected == -1 )
            return;
        m_pDescriptorList[m_DescriptorSelected].m_pElements[m_ElementSelected].OnEnumProp( List, "", m_ElementSelected, m_pDescriptorList[m_DescriptorSelected].m_Type );
    }
    else if( m_Selected & INTENSITY_SELECTED )
    {
        if( m_IntensitySelected == -1 )
            return;

        m_pIntensity[ m_IntensitySelected ].OnEnumProp( List, xfs("Intensity[%d]", m_IntensitySelected) );
    }
}

//=========================================================================

xbool editor_audio_package::OnProperty( prop_query& I)
{
    if( rsc_desc::OnProperty( I ) )
        return TRUE;

    if( I.IsRead() == FALSE )
    {
        if( I.GetQueryType() == PROP_TYPE_FLOAT )
        {
            f32 Value = I.GetVarFloat();
            if( Value == -2.0f )
                return TRUE;
        }   
        else if( I.GetQueryType() == PROP_TYPE_INT )
        {
            s32 iVal = I.GetVarInt();
            if( iVal == -2 )
                return TRUE;
        }

        if( I.GetIndexCount() == 0 )
        {
            const char* pPropName = I.GetName();
            (void)pPropName;
        }
    }


    // The Volume.
    if( I.VarFloat ( "ResDesc\\Volume"          , m_Volume      ) )
        return TRUE;

    // The Volume Var.
    if( I.VarFloat ( "ResDesc\\Volume Var"      , m_VolumeVar   ) )
        return TRUE;

    // The Volume LFE.
    if( I.VarFloat ( "ResDesc\\Volume LFE"      , m_VolumeLfe   ) )
        return TRUE;

    // The Volume Center.
    if( I.VarFloat ( "ResDesc\\Volume Center"   , m_VolumeCenter ) )
        return TRUE;

    // The Pitch.
    if( I.VarFloat ( "ResDesc\\Pitch"           , m_Pitch       ) )
    {
        if( (m_Pitch < 0.015625) )//&& (m_Pitch != -2) )
        {
            m_Pitch = 0.015625;
        }
        else if( m_Pitch > 4 )
        {
            m_Pitch = 4;
        }
        return TRUE;
    }

    // The Pitch Var.
    if( I.VarFloat ( "ResDesc\\Pitch Var"       , m_PitchVar    ) )
        return TRUE;

    // The Pan.
    if( I.VarFloat ( "ResDesc\\Pan"             , m_Pan         ) )
        return TRUE;

    // The Priority.
    if( I.VarInt   ( "ResDesc\\Priority"        , m_Priority    ) )
        return TRUE;

    // The EffectSend.
    if( I.VarFloat ( "ResDesc\\EffectSend"      , m_EffectSend  ) )
        return TRUE;

    // The Near Falloff.
    if( I.VarFloat ( "ResDesc\\Near Falloff"    , m_NearFalloff ) )
        return TRUE;

    // The Far falloff.
    if( I.VarFloat ( "ResDesc\\Far falloff"     , m_FarFalloff  ) )
        return TRUE;

    // The Near Diffuse.
    if( I.VarFloat ( "ResDesc\\Near Diffuse"    , m_NearDiffuse ) )
        return TRUE;

    // The Far Diffuse.
    if( I.VarFloat ( "ResDesc\\Far Diffuse"     , m_FarDiffuse  ) )
        return TRUE;

    // The Play Percent.
    if( I.VarInt   ( "ResDesc\\Play Percent"    , m_PlayPercent    ) )
        return TRUE;

    // The Replay Delay.
    if( I.VarFloat ( "ResDesc\\Replay Delay"    , m_ReplayDelay  ) )
        return TRUE;

    // The rolloff curve.
    if( I.IsVar( "ResDesc\\Rolloff Curve" ) )
    {
        if( I.IsRead () )
        {
            if( m_RollOff & LINEAR_ROLLOFF )
                I.SetVarEnum( "Linear" );
            else if( m_RollOff & FAST_ROLLOFF )
                I.SetVarEnum( "Fast" );
            else if( m_RollOff & SLOW_ROLLOFF )
                I.SetVarEnum( "Slow" );
            else
                ASSERT( 0 );
        }
        else
        {
            m_RollOff = 0;

            if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                m_RollOff |= LINEAR_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                m_RollOff |= FAST_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                m_RollOff |= SLOW_ROLLOFF;
            else
                ASSERT( 0 );
        }
        return TRUE;
    }

    // The Descriptor count.
    if( I.VarInt   ( "ResDesc\\DescCount"       , m_DescCount ) )
        return TRUE;

    // The number of editor_descriptors saved.
    if( I.IsVar( "ResDesc\\DescSaveCount" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarInt( m_pDescriptorList.GetCount() );
        }
        else
        {
            m_pDescriptorList.SetCount( I.GetVarInt() );

            /*
            //
            // HACKED: We need a setcount in the xharray
            //
            for( s32 i=0; i<I.GetVarInt(); i++ )
            {
                m_pDescriptorList.AddNode();
            }
            */
            
            // Since this is used with a read only flag it can't be saved so we have to load it from a file using the
            // DescSave count.
            m_DescCount = I.GetVarInt();
        }
        return TRUE;
    }        

    // The number of intensity saved.
    if( I.IsVar( "ResDesc\\IntensitySaveCount" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarInt( m_pIntensity.GetCount() );
        }
        else
        {
            m_pIntensity.SetCount( I.GetVarInt() );            
        }
        return TRUE;
    }        

    // The Volume Fader.
    if( I.IsVar( "ResDesc\\Volume Fader" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarEnum( m_pVolumeFader );
        }
        else
        {
            x_strsavecpy( m_pVolumeFader, I.GetVarEnum(), 128 );
        }
        return TRUE;
    }


    // The Pitch Fader.
    if( I.IsVar( "ResDesc\\Pitch Fader" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarEnum( m_pPitchFader );
        }
        else
        {
            x_strsavecpy( m_pPitchFader, I.GetVarEnum(), 128 );
        }
        return TRUE;
    }

    // The Near Falloff Fader.
    if( I.IsVar( "ResDesc\\Near Falloff Fader" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarEnum( m_pNearFalloffFader );
        }
        else
        {
            x_strsavecpy( m_pNearFalloffFader, I.GetVarEnum(), 128 );
        }
        return TRUE;
    }

    // The Far Falloff Fader.
    if( I.IsVar( "ResDesc\\Far Falloff Fader" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarEnum( m_pFarFalloffFader );
        }
        else
        {
            x_strsavecpy( m_pFarFalloffFader, I.GetVarEnum(), 128 );
        }
        return TRUE;
    }

    // The EffectSend Fader.
    if( I.IsVar( "ResDesc\\EffectSend Fader" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarEnum( m_pEffectSendFader );
        }
        else
        {
            x_strsavecpy( m_pEffectSendFader, I.GetVarEnum(), 128 );
        }
        return TRUE;
    }

    if( I.VarString ("ResDesc\\Virtual Dir",    m_pVirtualDirectory, 128 ) )
        return TRUE;

    if( I.VarString ("ResDesc\\Type",           m_pType, 128 ) )
        return TRUE;

    if( I.GetIndex(0) < m_pDescriptorList.GetCount() )
    {
        if( m_pDescriptorList[I.GetIndex( 0 )].OnProperty( I, "ResDesc\\Descriptor[]" ) )
        return TRUE;
    }

    if( I.GetIndex(0) < m_pIntensity.GetCount() )
    {
        if( m_pIntensity[I.GetIndex( 0 )].OnProperty( I, "ResDesc\\Intensity[]" ) )
        return TRUE;
    }
        
    return FALSE;
}

//=========================================================================

xbool editor_audio_package::OnEditorProperty( prop_query& I)
{
    if( rsc_desc::OnProperty( I ) )
        return TRUE;

    if( I.IsRead() == FALSE )
    {
        if( I.GetQueryType() == PROP_TYPE_FLOAT )
        {
            f32 Value = I.GetVarFloat();
            if( Value == -2.0f )
                return TRUE;
        }   
        else if( I.GetQueryType() == PROP_TYPE_INT )
        {
            s32 iVal = I.GetVarInt();
            if( iVal == -2 )
                return TRUE;
        }
    }

    if( m_Selected & PACKAGE_SELECTED )
    {
        if( I.IsRead() == FALSE )
        {
            const char* pPropName = I.GetName();
            (void)pPropName;
        }
        // The Volume.
        if( I.VarFloat ( "ResDesc\\Volume"          , m_Volume      ) )
            return TRUE;

        // The Volume Var.
        if( I.VarFloat ( "ResDesc\\Volume Var"      , m_VolumeVar   ) )
            return TRUE;

        // The Volume LFE.
        if( I.VarFloat ( "ResDesc\\Volume LFE"      , m_VolumeLfe   ) )
            return TRUE;

        // The Volume Center.
        if( I.VarFloat ( "ResDesc\\Volume Center"   , m_VolumeCenter ) )
            return TRUE;

        // The Pitch.
        if( I.VarFloat ( "ResDesc\\Pitch"           , m_Pitch       ) )
        {
            if( (m_Pitch < 0.015625) )//&& (m_Pitch != -2) )
            {
                m_Pitch = 0.015625;
            }
            else if( m_Pitch > 4 )
            {
                m_Pitch = 4;
            }
            return TRUE;
        }

        // The Pitch Var.
        if( I.VarFloat ( "ResDesc\\Pitch Var"       , m_PitchVar    ) )
            return TRUE;

        // The Pan.
        if( I.VarFloat ( "ResDesc\\Pan"             , m_Pan         ) )
            return TRUE;

        // The Priority.
        if( I.VarInt   ( "ResDesc\\Priority"        , m_Priority    ) )
            return TRUE;

        // The EffectSend.
        if( I.VarFloat ( "ResDesc\\EffectSend"      , m_EffectSend  ) )
            return TRUE;

        // The Near Falloff.
        if( I.VarFloat ( "ResDesc\\Near Falloff"    , m_NearFalloff ) )
            return TRUE;

        // The Far falloff.
        if( I.VarFloat ( "ResDesc\\Far falloff" ,    m_FarFalloff   ) )
            return TRUE;

        // The Near Diffuse.
        if( I.VarFloat ( "ResDesc\\Near Diffuse"    , m_NearDiffuse ) )
            return TRUE;

        // The Far Diffuse.
        if( I.VarFloat ( "ResDesc\\Far Diffuse"     , m_FarDiffuse  ) )
            return TRUE;

        // The Play Percent.
        if( I.VarInt   ( "ResDesc\\Play Percent"    , m_PlayPercent    ) )
            return TRUE;

        // The Replay Delay.
        if( I.VarFloat ( "ResDesc\\Replay Delay"    , m_ReplayDelay  ) )
            return TRUE;

        // The rolloff curve.
        if( I.IsVar( "ResDesc\\Rolloff Curve" ) )
        {
            if( I.IsRead () )
            {
                if( m_RollOff & LINEAR_ROLLOFF )
                    I.SetVarEnum( "Linear" );
                else if( m_RollOff & FAST_ROLLOFF )
                    I.SetVarEnum( "Fast" );
                else if( m_RollOff & SLOW_ROLLOFF )
                    I.SetVarEnum( "Slow" );
                else
                    ASSERT( 0 );
            }
            else
            {
                m_RollOff = 0;

                if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                    m_RollOff |= LINEAR_ROLLOFF;
                else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                    m_RollOff |= FAST_ROLLOFF;
                else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                    m_RollOff |= SLOW_ROLLOFF;
                else
                    ASSERT( 0 );
            }
            return TRUE;
        }

        // The Descriptorcount.
        if( I.VarInt   ( "ResDesc\\DescCount"       , m_DescCount ) )
            return TRUE;


        // The Type.
        if( I.IsVar( "ResDesc\\DescSaveCount" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarInt( m_pDescriptorList.GetCount() );
            }
            else
            {
                m_pDescriptorList.SetCount( I.GetVarInt() );
            }
            return TRUE;
        }        

        // The number of intensity saved.
        if( I.IsVar( "ResDesc\\IntensitySaveCount" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarInt( m_pIntensity.GetCount() );
            }
            else
            {
                m_pIntensity.SetCount( I.GetVarInt() );            
            }
            return TRUE;
        }        

        // The Volume Fader.
        if( I.IsVar( "ResDesc\\Volume Fader" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarEnum( m_pVolumeFader );
            }
            else
            {
                x_strsavecpy( m_pVolumeFader, I.GetVarEnum(), 128 );
            }
            return TRUE;
        }


        // The Pitch Fader.
        if( I.IsVar( "ResDesc\\Pitch Fader" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarEnum( m_pPitchFader );
            }
            else
            {
                x_strsavecpy( m_pPitchFader, I.GetVarEnum(), 128 );
            }
            return TRUE;
        }

        // The Near Falloff Fader.
        if( I.IsVar( "ResDesc\\Near Falloff Fader" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarEnum( m_pNearFalloffFader );
            }
            else
            {
                x_strsavecpy( m_pNearFalloffFader, I.GetVarEnum(), 128 );
            }
            return TRUE;
        }

        // The Far Falloff Fader.
        if( I.IsVar( "ResDesc\\Far Falloff Fader" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarEnum( m_pFarFalloffFader );
            }
            else
            {
                x_strsavecpy( m_pFarFalloffFader, I.GetVarEnum(), 128 );
            }
            return TRUE;
        }

        // The EffectSend Fader.
        if( I.IsVar( "ResDesc\\EffectSend Fader" ) )
        {
            if( I.IsRead () )
            {
                I.SetVarEnum( m_pEffectSendFader );
            }
            else
            {
                x_strsavecpy( m_pEffectSendFader, I.GetVarEnum(), 128 );
            }
            return TRUE;
        }

        if( I.VarString ("ResDesc\\Virtual Dir",    m_pVirtualDirectory, 128 ) )
            return TRUE;

        if( I.VarString ("ResDesc\\Type",           m_pType, 128 ) )
            return TRUE;

//        if( I.GetIndex(0) < m_pDescriptorList.GetCount() )
//        {
//            if( m_pDescriptorList[I.GetIndex( 0 )].OnProperty( I, "ResDesc\\Descriptor[]" ) )
//            return TRUE;
//        }    
    }   
    else if( m_Selected & DESCRIPTOR_SELECTED )        
    {
        if( m_DescriptorSelected == -1 )
            return FALSE;

        return m_pDescriptorList[m_DescriptorSelected].OnProperty( I, "Descriptor[]" );
    }
    else if( m_Selected & ELEMENT_SELECTED )
    {
        if( m_ElementSelected == -1 || m_DescriptorSelected == -1 )
            return FALSE;
        return m_pDescriptorList[m_DescriptorSelected].m_pElements[m_ElementSelected].OnProperty( I, "", m_ElementSelected, m_pDescriptorList[m_DescriptorSelected].m_Type );
    }
    else if( m_Selected & INTENSITY_SELECTED )        
    {
        if( m_IntensitySelected == -1 )
            return FALSE;

        return m_pIntensity[m_IntensitySelected].OnProperty( I, "Intensity[]" );
    }

    return FALSE;
}

//=========================================================================

void editor_audio_package::OnGetCompilerDependencies( xarray<xstring>& List )
{
    // We want to make sure that we don't append that same file name twice.
    xstring SampleBuffer;

    for( s32 i = 0; i < m_pDescriptorList.GetCount(); i++ )
    {
        for( s32 j = 0; j < m_pDescriptorList[i].m_pElements.GetCount(); j++ )   
        {
            // Make sure that its not a refrence.
            if( x_strlen( m_pDescriptorList[i].m_pElements[j].m_pSampleName ) )
            {
                // If the sample label doesn't exist then add it to the string.
                if( SampleBuffer.Find( m_pDescriptorList[i].m_pElements[j].m_pSamplePathName ) == -1 )
                {
                    SampleBuffer += m_pDescriptorList[i].m_pElements[j].m_pSamplePathName;
                    SampleBuffer += "\n";
                    
                    List.Append() = m_pDescriptorList[i].m_pElements[j].m_pSamplePathName;
                }
            }
        }
    }
}

//=========================================================================

void editor_audio_package::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    // This resouce doesn't have any post compile dependencies 
}

//=========================================================================

void editor_audio_package::OnGetCompilerRules( xstring& CompilerRules )
{
    CompilerRules.Clear();

    // Check if this package was already loaded by the editor, if it was then
    // unlaod it and reset the loaded flag so the next time someone want to play this
    // package they load the newly undated package.
    if( m_PackageLoaded )
    {
        //char FilePath[256];
        //sprintf( FilePath, "%s\\PC\\%s", g_Settings.GetReleasePath(), GetName() );

        g_RscMgr.Unload( GetName() );
        //g_AudioMgr.UnloadPackage( FilePath );
        
        m_PackageLoaded = FALSE;
    }

    xstring         WriteBuffer;
    xstring         SampleLabelBuffer;
    xarray<xstring> SampleNameBuffer;
    s32 i = 0;
    s32 j = 0;
    
    WriteBuffer += "\n package: \n";
    xstring PackageName = GetName();
    PackageName = PackageName.Left( PackageName.Find( '.' ) );

    WriteBuffer += PackageName;
    
    WriteBaseParameters( m_Volume, 
                        m_VolumeVar, 
                        m_VolumeCenter, 
                        m_VolumeLfe, 
                        m_Pitch, 
                        m_PitchVar, 
                        m_Pan, 
                        m_Priority, 
                        m_EffectSend, 
                        m_NearFalloff, 
                        m_FarFalloff, 
                        m_RollOff, 
                        m_NearDiffuse,
                        m_FarDiffuse, 
                        m_PlayPercent,
                        m_ReplayDelay,
                        WriteBuffer );

    WriteBuffer += "\nfiles:\n";
    
    // We want to make sure that we don't append the same file name twice.
    for( i = 0; i < m_pDescriptorList.GetCount(); i++ )
    {
        for( j = 0; j < m_pDescriptorList[i].m_pElements.GetCount(); j++ )   
        {
            // Make sure that its not a refrence.
            if( x_strlen( m_pDescriptorList[i].m_pElements[j].m_pSampleName ) )
            {
                
                s32     iBuff;
                xbool   bFound = FALSE;
                for( iBuff = 0; iBuff < SampleNameBuffer.GetCount(); iBuff++ )
                {
                    if( SampleNameBuffer[iBuff] == m_pDescriptorList[i].m_pElements[j].m_pSampleName )
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                
                // If the sample label doesn't exist then add it to the string.
                if( bFound == FALSE )//if( SampleLabelBuffer.Find( m_pDescriptorList[i].m_pElements[j].m_pSampleName ) == -1 )
                {
                    
                    SampleLabelBuffer += m_pDescriptorList[i].m_pElements[j].m_pSampleName;
                    switch( m_pDescriptorList[i].m_pElements[j].m_Temperature )
                    {
                        case HOT     : SampleLabelBuffer +=  " HOT "; break;
                        case WARM    : SampleLabelBuffer += " WARM "; break;
                        case COLD    : SampleLabelBuffer += " COLD "; break;
                        default:    
                        {
                            ASSERTS( FALSE, "Didn't set the type"  );
                        }
                    }
                    SampleLabelBuffer +=   "NoLipSync ";
                    SampleLabelBuffer += m_pDescriptorList[i].m_pElements[j].m_pSamplePathName;
                    SampleLabelBuffer += "\n";

                    SampleNameBuffer.Append( xstring(m_pDescriptorList[i].m_pElements[j].m_pSampleName) );
                }
            }
        }
    }
    
    WriteBuffer += SampleLabelBuffer;
    WriteBuffer += "descriptors:\n";

    // Start writing the descriptors.
    for( i = 0; i < m_pDescriptorList.GetCount(); i++ )
    {
        WriteBuffer += m_pDescriptorList[i].m_Label;
        WriteBaseParameters( m_pDescriptorList[i].m_Volume,         
                            m_pDescriptorList[i].m_VolumeVar, 
                            m_pDescriptorList[i].m_VolumeCenter, 
                            m_pDescriptorList[i].m_VolumeLfe,       
                            m_pDescriptorList[i].m_Pitch,           
                            m_pDescriptorList[i].m_PitchVar, 
                            m_pDescriptorList[i].m_Pan,             
                            m_pDescriptorList[i].m_Priority, 
                            m_pDescriptorList[i].m_EffectSend,      
                            m_pDescriptorList[i].m_NearFalloff, 
                            m_pDescriptorList[i].m_FarFalloff,      
                            m_pDescriptorList[i].m_RollOff,
                            m_pDescriptorList[i].m_NearDiffuse,     
                            m_pDescriptorList[i].m_FarDiffuse,
                            m_pDescriptorList[i].m_PlayPercent,
                            m_pDescriptorList[i].m_ReplayDelay,
                            WriteBuffer );
        
        switch( m_pDescriptorList[i].m_Type )
        {
            case EDITOR_SIMPLE     : WriteBuffer += " simple "; break;
            case EDITOR_COMPLEX    : WriteBuffer += " complex "; break;
            case EDITOR_RANDOM     : WriteBuffer += " rlist "; break;
            case EDITOR_WEIGHTED   : WriteBuffer += " wlist "; break;
            default:    
            {
            ASSERTS( FALSE, "Didn't set the type"  );
            }
        } 
        
        if( m_pDescriptorList[i].m_Type != EDITOR_SIMPLE )
            WriteBuffer += "\n{\n";

        // Store all the elements
        for( j = 0; j < m_pDescriptorList[i].m_pElements.GetCount(); j++ )   
        {
            if( x_strlen(m_pDescriptorList[i].m_pElements[j].m_pSampleName) )
            {
                WriteBuffer += m_pDescriptorList[i].m_pElements[j].m_pSampleName;
                
                WriteBaseParameters(    m_pDescriptorList[i].m_pElements[j].m_Volume,  
                                        m_pDescriptorList[i].m_pElements[j].m_VolumeVar, 
                                        m_pDescriptorList[i].m_pElements[j].m_VolumeCenter,             
                                        m_pDescriptorList[i].m_pElements[j].m_VolumeLfe, 
                                        m_pDescriptorList[i].m_pElements[j].m_Pitch,                    
                                        m_pDescriptorList[i].m_pElements[j].m_PitchVar, 
                                        m_pDescriptorList[i].m_pElements[j].m_Pan,                      
                                        m_pDescriptorList[i].m_pElements[j].m_Priority, 
                                        m_pDescriptorList[i].m_pElements[j].m_EffectSend,               
                                        m_pDescriptorList[i].m_pElements[j].m_NearFalloff,
                                        m_pDescriptorList[i].m_pElements[j].m_FarFalloff,               
                                        m_pDescriptorList[i].m_pElements[j].m_RollOff,
                                        m_pDescriptorList[i].m_pElements[j].m_NearDiffuse,              
                                        m_pDescriptorList[i].m_pElements[j].m_FarDiffuse,
                                        m_pDescriptorList[i].m_pElements[j].m_PlayPercent,
                                        m_pDescriptorList[i].m_pElements[j].m_ReplayDelay,
                                        WriteBuffer );

                
                if( m_pDescriptorList[i].m_Type == EDITOR_COMPLEX )
                    WriteBuffer += (const char*)xfs(" %5.2f ", m_pDescriptorList[i].m_pElements[j].m_Delta);    
                else if( m_pDescriptorList[i].m_Type == EDITOR_WEIGHTED ) 
                    WriteBuffer += (const char*)xfs(" %g ", m_pDescriptorList[i].m_pElements[j].m_Delta);    

                WriteBuffer += "\n";
            }
            else
            {
                // Its a refrencing descriptor so just put the label of the descriptor there.
                WriteBuffer += m_pDescriptorList[i].m_pElements[j].m_pReferenceLabel;

                WriteBaseParameters(    m_pDescriptorList[i].m_pElements[j].m_Volume,  
                                        m_pDescriptorList[i].m_pElements[j].m_VolumeVar, 
                                        m_pDescriptorList[i].m_pElements[j].m_VolumeCenter,             
                                        m_pDescriptorList[i].m_pElements[j].m_VolumeLfe, 
                                        m_pDescriptorList[i].m_pElements[j].m_Pitch,                    
                                        m_pDescriptorList[i].m_pElements[j].m_PitchVar, 
                                        m_pDescriptorList[i].m_pElements[j].m_Pan,                      
                                        m_pDescriptorList[i].m_pElements[j].m_Priority, 
                                        m_pDescriptorList[i].m_pElements[j].m_EffectSend,               
                                        m_pDescriptorList[i].m_pElements[j].m_NearFalloff,
                                        m_pDescriptorList[i].m_pElements[j].m_FarFalloff,               
                                        m_pDescriptorList[i].m_pElements[j].m_RollOff,
                                        m_pDescriptorList[i].m_pElements[j].m_NearDiffuse,              
                                        m_pDescriptorList[i].m_pElements[j].m_FarDiffuse,
                                        m_pDescriptorList[i].m_pElements[j].m_PlayPercent,
                                        m_pDescriptorList[i].m_pElements[j].m_ReplayDelay,
                                        WriteBuffer );

                if( m_pDescriptorList[i].m_Type == EDITOR_COMPLEX )
                    WriteBuffer += (const char*)xfs(" %5.2f ", m_pDescriptorList[i].m_pElements[j].m_Delta);    
                else if( m_pDescriptorList[i].m_Type == EDITOR_WEIGHTED ) 
                    WriteBuffer += (const char*)xfs(" %g ", m_pDescriptorList[i].m_pElements[j].m_Delta);    

                WriteBuffer += "\n";
            }
        }

        if( m_pDescriptorList[i].m_Type != EDITOR_SIMPLE )
            WriteBuffer += "}\n";

    }

    if( m_pIntensity.GetCount() )
    {
        
        WriteBuffer += "music:\n";
        
        WriteBuffer += "type: ";
        WriteBuffer += m_pType;
        
        WriteBuffer += "\nintensity:\n";
        WriteBuffer += "{\n";
        // Start writing the descriptors.
        for( i = 0; i < m_pIntensity.GetCount(); i++ )
        {
            for( s32 j = 0; j < m_pIntensity[i].m_pDescriptors.GetCount(); j++ )
            {
                WriteBuffer += m_pIntensity[i].m_pDescriptors[j];
                WriteBuffer += (const char*)xfs(" %d \n", m_pIntensity[i].m_Level);
            }
        }
        WriteBuffer += "}\n";
    }
           
    WriteBuffer += "output:\n";

    CompilerRules = "A51SoundPkg.exe ";
    CompilerRules += xfs( "-TempPath \"%s\" ", g_Settings.GetTempPath() );
    CompilerRules += " -cmdline ";
    CompilerRules += WriteBuffer;

}

//=========================================================================

void editor_audio_package::OnCheckIntegrity ( void )
{
    rsc_desc::OnCheckIntegrity();
    s32 i = 0;

    if( ((m_Volume < 0.0f) || (m_Volume > 1.0f)) && (m_Volume != -2.0f) )
        x_throw( xfs( "Volume parameter out of range for the package [%s]", GetName() ) );

    if( ((m_VolumeVar < 0.0f) || (m_VolumeVar > 1.0f)) && (m_VolumeVar != -2.0f) )
        x_throw( xfs( "Volume Var parameter out of range for the package [%s]", GetName() ) );

    if( ((m_Pitch < 0.015625f) || (m_Pitch > 4.0f)) && (m_Pitch != -2.0f) )
        x_throw( xfs( "Pitch parameter out of range for the package [%s]", GetName() ) );

    if( ((m_PitchVar < 0.0f) || (m_PitchVar > 1.0f)) && (m_PitchVar != -2.0f) )
        x_throw( xfs( "Pitch Var parameter out of range for the package [%s]", GetName() ) );

    if( ((m_Pan < -1.0f) || (m_Pan > 1.0f)) && (m_Pan != -2.0f) )
        x_throw( xfs( "Pan parameter out of range for the package [%s]", GetName() ) );

    if( ((m_Priority < 0) || (m_Priority > 255)) && (m_Priority != -2) )
        x_throw( xfs( "Priority parameter out of range for the package [%s]", GetName() ) );

    if( ((m_EffectSend < 0.0f) || (m_EffectSend > 1.0f)) && (m_EffectSend != -2.0f) )
        x_throw( xfs( "EffectSend parameter out of range for the package [%s]", GetName() ) );
    
    if( ((m_NearFalloff < 0.0f) || (m_NearFalloff > 10.0f)) && (m_NearFalloff != -2.0f) )
        x_throw( xfs( "NearFalloff parameter out of range for the package [%s]", GetName() ) );

    if( ((m_FarFalloff < 0.0f) || (m_FarFalloff > 10.0f)) && (m_FarFalloff != -2.0f) )
        x_throw( xfs( "FarFalloff parameter out of range for the package [%s]", GetName() ) );

    if( ((m_VolumeLfe < 0.0f) || (m_VolumeLfe > 1.0f)) && (m_VolumeLfe != -2.0f) )
        x_throw( xfs( "VolumeLfe parameter out of range for the package [%s]", GetName() ) );

    if( ((m_VolumeCenter < 0.0f) || (m_VolumeCenter > 1.0f)) && (m_VolumeCenter != -2.0f) )
        x_throw( xfs( "Volume Center parameter out of range for the package [%s]", GetName() ) );

    if( ((m_NearDiffuse < 0.0f) || (m_NearDiffuse > 10.0f)) )
        x_throw( xfs( "NearDiffuse parameter out of range for the package [%s]", GetName() ) );

    if( ((m_FarDiffuse < 0.0f) || (m_FarDiffuse > 10.0f)) )
        x_throw( xfs( "FarDiffuse parameter out of range for the package [%s]", GetName() ) );
    
    if( !(m_RollOff & LINEAR_ROLLOFF) && !(m_RollOff & FAST_ROLLOFF) && !(m_RollOff & SLOW_ROLLOFF) )
        x_throw( xfs( "Curruption in the rolloff type, consult a programmer asap, package [%s]", GetName() ) );

    if( ((m_PlayPercent < 0) || (m_PlayPercent > 100)) )
        x_throw( xfs( "PlayPercent parameter out of range for the package [%s]", GetName() ) );

    if( ((m_ReplayDelay < 0.0f) || (m_ReplayDelay > 100.0f)) )
        x_throw( xfs( "ReplayDelay parameter out of range for the package [%s]", GetName() ) );

    for( i = 0; i < m_pDescriptorList.GetCount(); i++ )
    {
        m_pDescriptorList[i].OnCheckIntegrity( GetName() );        
    }

    for( i = 0; i < m_pIntensity.GetCount(); i++ )
    {
        for( s32 j = i+1; j < m_pIntensity.GetCount(); j++ )
        {
            if( m_pIntensity[i].m_Level == m_pIntensity[j].m_Level )
                x_throw( xfs( "There are intensities with the same level in package [%s]", GetName() ) );
        }
    }
}

//=========================================================================

editor_audio_package& editor_audio_package::operator = ( const editor_audio_package& Package   )
{
    
    // Copy all the parameters.
    char FullName[128];
    Package.GetFullName( FullName );
    SetFullName( FullName );

    m_Volume                = Package.m_Volume;
    m_VolumeVar             = Package.m_VolumeVar;
    m_VolumeLfe             = Package.m_VolumeLfe;
    m_VolumeCenter          = Package.m_VolumeCenter;
    m_Pitch                 = Package.m_Pitch;
    m_PitchVar              = Package.m_PitchVar;
    m_Pan                   = Package.m_Pan;
    m_Priority              = Package.m_Priority;
    m_EffectSend            = Package.m_EffectSend;
    m_NearFalloff           = Package.m_NearFalloff;
    m_FarFalloff            = Package.m_FarFalloff;
    m_RollOff               = Package.m_RollOff;
    m_NearDiffuse           = Package.m_NearDiffuse;
    m_FarDiffuse            = Package.m_FarFalloff;
    m_PlayPercent           = Package.m_PlayPercent;
    m_ReplayDelay           = Package.m_ReplayDelay;

    m_DescCount             = Package.m_DescCount;

    x_strncpy( m_pVirtualDirectory, Package.m_pVirtualDirectory, 128 );
    
    x_strsavecpy( m_pVolumeFader,      Package.m_pVolumeFader,         128 );
    x_strsavecpy( m_pPitchFader,       Package.m_pPitchFader,          128 );
    x_strsavecpy( m_pNearFalloffFader, Package.m_pNearFalloffFader,    128 );
    x_strsavecpy( m_pFarFalloffFader,  Package.m_pFarFalloffFader,     128 );
    x_strsavecpy( m_pEffectSendFader,  Package.m_pEffectSendFader,     128 );
    x_strsavecpy( m_pType,             Package.m_pType,                128 );

    // Reset the editor_element and editor_descriptor select variables.
    m_ElementSelected = -1;
    m_DescriptorSelected = -1;
    m_Selected = 0;
    m_Selected |= PACKAGE_SELECTED;

    m_pDescriptorList.Clear();
    m_pIntensity.Clear();
    s32 i = 0;

    // Copy all the editor_descriptors and the editor_elements over.
    for( i = 0; i < Package.m_pDescriptorList.GetCount(); i++ )
    {
        m_pDescriptorList.Append( Package.m_pDescriptorList[i] );
        
        // Get the elements.
        m_pDescriptorList[i].m_pElements.Clear();
        for( s32 j = 0; j < Package.m_pDescriptorList[i].m_pElements.GetCount(); j++ )
        {
            m_pDescriptorList[i].m_pElements.Append( Package.m_pDescriptorList[i].m_pElements[j] );
        }

        // Get the refrencing elements.
        m_pDescriptorList[i].m_pReferencingElement.Clear();
        for( j = 0; j < Package.m_pDescriptorList[i].m_pReferencingElement.GetCount(); j++ )
        {
            m_pDescriptorList[i].m_pReferencingElement.Append( Package.m_pDescriptorList[i].m_pReferencingElement[j] );
        }
    }
    
    for( i = 0; i < Package.m_pIntensity.GetCount(); i ++ )
    {
        m_pIntensity.Append( Package.m_pIntensity[i] );
    }

    return (*this);
}


//=========================================================================
// INTENSITY
//=========================================================================

editor_intensity::editor_intensity()
{
    m_Level = 0;
    m_pDescriptors.Clear();
    m_pDescriptors.SetGrowAmount( 4 );
}

//=========================================================================

void editor_intensity::OnEnumProp( prop_enum& List, const char* pParent )
{
    VERIFYS( pParent, "Null parent found in intensity enum prop\n" );
    if( pParent == NULL )
        return;

    // Add all the editor_descriptor items to the property.
    List.PropEnumHeader( xfs( "%s", pParent), "", 0 );
    
    List.PropEnumInt( xfs( "%s\\Level", pParent ), "Intensity Level of the descriptors", 0 );    
    List.PropEnumInt( xfs( "%s\\Desc Count", pParent ), "", 0 );//, PROP_TYPE_DONT_SHOW );
    
    for( s32 i = 0; i < m_pDescriptors.GetCount(); i++ )
    {
        List.PropEnumString( xfs( "%s\\Desc Name[%d]", pParent, i ), "Names of the descriptors that are associtated with the intensity", 0 );
    }
}

//=========================================================================

xbool editor_intensity::OnProperty( prop_query& I, const char* pParent )
{
    VERIFYS( pParent, "Null parent found in intensity property\n" );
    if( pParent == NULL )
        return FALSE;

    if( I.VarInt( xfs( "%s\\Level", pParent ), m_Level ) )
        return TRUE;

    // Descriptor Count on laod.
    if( I.IsVar(  xfs( "%s\\Desc Count", pParent ) ) )
    {
        if( I.IsRead () )
        {
            I.SetVarInt( m_pDescriptors.GetCount() );
        }
        else
        {
            m_pDescriptors.SetCount( I.GetVarInt() );
        }
        return TRUE;
    }        

    if( (I.GetIndex(1) < m_pDescriptors.GetCount()) && (I.GetIndex(1) >= 0) )
    {

        if( I.IsVar( xfs( "%s\\Desc Name[]", pParent ) ))
        {
            if( I.IsRead () )
            {
                I.SetVarString( m_pDescriptors[ I.GetIndex(1) ], 128 );
            }
            else
            {
                m_pDescriptors[ I.GetIndex(1) ] = I.GetVarString();
            }
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

void editor_intensity::OnCheckIntegrity( const char* pRscName )
{
    (void)pRscName;
}

//=========================================================================
// DESCRIPTORS
//=========================================================================

editor_descriptor::editor_descriptor()
{
    m_Label[0]      = 0;
    m_Volume        = 1.0f;
    m_VolumeVar     = 0.0f;
    m_Pitch         = 1.0f;
    m_PitchVar      = 0.0f;
    m_Pan           = 0.0f;
    m_Priority      = 128;
    m_EffectSend    = 1.0f;
    m_NearFalloff   = 1.0f;
    m_FarFalloff    = 1.0f;
    m_VolumeLfe     = 0.0f;
    m_VolumeCenter  = 0.0f;
    m_RollOff       |= LINEAR_ROLLOFF;
    m_NearDiffuse   = 1.0f;
    m_FarDiffuse    = 1.0f;
    m_PlayPercent   = 100;
    m_ReplayDelay   = 0.0f;

    m_Type = EDITOR_SIMPLE;

    m_pElements.SetGrowAmount( 8 );
}

//=========================================================================

void editor_descriptor::Clear( void )
{
    m_Label[0]      = 0;
    m_Volume        = 1.0f;
    m_VolumeVar     = 0.0f;
    m_Pitch         = 1.0f;
    m_PitchVar      = 0.0f;
    m_Pan           = 0.0f;
    m_Priority      = 128;
    m_EffectSend    = 1.0f;
    m_NearFalloff   = 1.0f;
    m_FarFalloff    = 1.0f;
    m_VolumeLfe     = 0.0f;
    m_VolumeCenter  = 0.0f;
    m_RollOff       |= LINEAR_ROLLOFF;
    m_NearDiffuse   = 1.0f;
    m_FarDiffuse    = 1.0f;
    m_PlayPercent   = 100;
    m_ReplayDelay   = 0.0f;

    m_Type = EDITOR_SIMPLE;
}

//=========================================================================

xbool editor_descriptor::OnProperty( prop_query& I, const char* pParent  )
{
    VERIFYS( pParent, "Null parent found in descriptor OnProperty\n" );
    if( pParent == NULL )
        return FALSE;

    // The Label.
    if( I.VarString( xfs( "%s\\Label", pParent          ), m_Label, 128 ) )
        return TRUE;

    // The Volume.
    if( I.VarFloat ( xfs( "%s\\Volume", pParent         ), m_Volume     ) )
        return TRUE;

    // The Volume Var.
    if( I.VarFloat ( xfs( "%s\\Volume Var", pParent     ), m_VolumeVar  ) )
        return TRUE;

    // The Volume LFE.
    if( I.VarFloat ( xfs( "%s\\Volume LFE", pParent     ), m_VolumeLfe   ) )
        return TRUE;

    // The Volume Center.
    if( I.VarFloat ( xfs( "%s\\Volume Center", pParent  ), m_VolumeCenter ) )
        return TRUE;

    // The Pitch.
    if( I.VarFloat ( xfs( "%s\\Pitch", pParent          ), m_Pitch      ) )
    {
        if( (m_Pitch < 0.015625) )//&& (m_Pitch != -2) )
        {
            m_Pitch = 0.015625;
        }
        else if( m_Pitch > 4 )
        {
            m_Pitch = 4;
        }
        return TRUE;
    }

    // The Pitch Var.
    if( I.VarFloat ( xfs( "%s\\Pitch Var", pParent      ), m_PitchVar   ) )
        return TRUE;

    // The Pan.
    if( I.VarFloat ( xfs( "%s\\Pan", pParent            ), m_Pan        ) )
        return TRUE;

    // The Priority.
    if( I.VarInt   ( xfs( "%s\\Priority", pParent       ), m_Priority   ) )
        return TRUE;

    // The EffectSend.
    if( I.VarFloat ( xfs( "%s\\EffectSend", pParent     ), m_EffectSend ) )
        return TRUE;

    // The Near Falloff.
    if( I.VarFloat ( xfs( "%s\\Near Falloff", pParent   ), m_NearFalloff ) )
        return TRUE;

    // The Far falloff.
    if( I.VarFloat ( xfs( "%s\\Far falloff", pParent    ), m_FarFalloff ) )
        return TRUE;

    // The Near Diffuse.
    if( I.VarFloat ( xfs( "%s\\Near Diffuse", pParent    )    , m_NearDiffuse ) )
        return TRUE;

    // The Far Diffuse.
    if( I.VarFloat ( xfs( "%s\\Far Diffuse", pParent    )     , m_FarDiffuse  ) )
        return TRUE;

    // The Play Percent.
    if( I.VarInt   ( xfs( "%s\\Play Percent", pParent       ), m_PlayPercent   ) )
        return TRUE;

    // The Replay Delay.
    if( I.VarFloat ( xfs( "%s\\Replay Delay", pParent    )   , m_ReplayDelay  ) )
        return TRUE;

    // The Rolloff Curve.
    if( I.IsVar( xfs( "%s\\Rolloff Curve", pParent    ) ) )
    {
        if( I.IsRead () )
        {
            if( m_RollOff & LINEAR_ROLLOFF )
                I.SetVarEnum( "Linear" );
            else if( m_RollOff & FAST_ROLLOFF )
                I.SetVarEnum( "Fast" );
            else if( m_RollOff & SLOW_ROLLOFF )
                I.SetVarEnum( "Slow" );
            else
                ASSERT( 0 );
        }
        else
        {
            m_RollOff = 0;

            if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                m_RollOff |= LINEAR_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                m_RollOff |= FAST_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                m_RollOff |= SLOW_ROLLOFF;
            else
                ASSERT( 0 );
        }
        return TRUE;
    }

    // The Type.
    if( I.IsVar( xfs( "%s\\Type", pParent    ) ) )
    {
        if( I.IsRead () )
        {
            switch( m_Type )
            {
                case EDITOR_SIMPLE     : I.SetVarEnum( "SIMPLE" );   break;
                case EDITOR_COMPLEX    : I.SetVarEnum( "COMPLEX" );  break;
                case EDITOR_RANDOM     : I.SetVarEnum( "RANDOM" );   break;
                case EDITOR_WEIGHTED   : I.SetVarEnum( "WEIGHTED" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "SIMPLE", I.GetVarEnum()) )
            {
                m_Type = EDITOR_SIMPLE;
            }
            else if( !x_stricmp( "COMPLEX", I.GetVarEnum() ) )
            {
                m_Type = EDITOR_COMPLEX;
            }
            else if( !x_stricmp( "RANDOM", I.GetVarEnum() ) )
            {
                m_Type = EDITOR_RANDOM;
            }
            else if( !x_stricmp( "WEIGHTED", I.GetVarEnum() ) )
            {
                m_Type = EDITOR_WEIGHTED;
            }
        }
        return TRUE;
    }

    // Element Count on laod.
    if( I.IsVar( xfs( "%s\\ElementCount", pParent    ) ) )
    {
        if( I.IsRead () )
        {
            I.SetVarInt( m_pElements.GetCount() );
        }
        else
        {
            m_pElements.SetCount( I.GetVarInt() );
        }
        return TRUE;
    }        

    if( (I.GetIndex(1) < m_pReferencingElement.GetCount()) && (I.GetIndex(1) >= 0) )
    {

        if( I.VarString( xfs( "%s\\ReferencingElement[]\\ParentDescriptor\\Label", pParent ), m_pReferencingElement[ I.GetIndex(1) ]->m_ParentDesc.m_Label, 128 ) )
            return TRUE;
    }

    if( (I.GetIndex(1) < m_pElements.GetCount()) && (I.GetIndex(1) >= 0) )
    {
        // Element.
        if( I.GetIndex(1) < m_pElements.GetCount() )
             return m_pElements[I.GetIndex(1)].OnProperty( I, (const char*)xfs("%s\\", pParent, m_Label), I.GetIndex(1), m_Type );
    }

    return FALSE;
}

//=========================================================================

void editor_descriptor::OnEnumProp( prop_enum& List, const char* pParent )
{
    VERIFYS( pParent, "Null parent found in descriptor OnEnumProp\n" );
    if( pParent == NULL )
        return;

    // Add all the editor_descriptor items to the property.
    List.PropEnumHeader( xfs( "%s", pParent         ), "Label for a sound editor_descriptor", 0 );
    
    List.PropEnumString( xfs( "%s\\Label",           pParent  ), "Sound Descriptor (Label)", 0 );
    List.PropEnumFloat ( xfs( "%s\\Volume",          pParent  ), "Volume of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Volume Var",      pParent  ), "Volume Variance of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Volume LFE",      pParent  ), "Volume LFE of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Volume Center",   pParent  ), "Volume center of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Pitch",           pParent  ), "The pitch of the editor_descriptor, Range( 2^-6 -- 2^2 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Pitch Var",       pParent  ), "Pitch variance of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Pan",             pParent  ), "Pan of the sound editor_descriptor, Range( -1.0 -- 1.0 )", 0 );
    List.PropEnumInt   ( xfs( "%s\\Priority",        pParent  ), "Priority of the editor_descriptor, Range( 0 -- 255)", 0 );
    List.PropEnumFloat ( xfs( "%s\\EffectSend",      pParent  ), "Reverb of the sound editor_descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Near Falloff",    pParent  ), "Near falloff of the sound editor_descriptor, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Far falloff",     pParent  ), "Far falloff of the sound editor_descriptor, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Near Diffuse",    pParent  ), "Near diffuse of the sound editor_descriptor, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( xfs( "%s\\Far Diffuse",     pParent  ), "Far diffuse of the sound editor_descriptor, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumInt   ( xfs( "%s\\Play Percent",    pParent  ), "Percent the sound editor_descriptor will actually play, Range( 0[never] -- 100[always]", 0 );
    List.PropEnumFloat ( xfs( "%s\\Replay Delay",    pParent  ), "Minimum time in seconds before the sound editor_descriptor plays again, Range( 0.1 -- 100.0 )", 0 );
    List.PropEnumEnum  ( xfs( "%s\\Rolloff Curve",   pParent  ), "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );

    List.PropEnumInt   ( xfs( "%s\\ElementCount", pParent  ), "", PROP_TYPE_DONT_SHOW );

    List.PropEnumEnum ( xfs( "%s\\Type", pParent  ), "SIMPLE\0COMPLEX\0RANDOM\0WEIGHTED\0", "The type of editor_descriptor (Simple, Complex, Random, Weighted)", PROP_TYPE_MUST_ENUM );

    for( s32 i = 0; i < m_pElements.GetCount(); i++ )
    {
        m_pElements[i].OnEnumProp( List, (const char*)xfs("%s\\", pParent ), i, m_Type);
    }

    for( i = 0; i < m_pReferencingElement.GetCount(); i++ )
    {
        List.PropEnumHeader( xfs( "%s\\ReferencingElement[%d]\\ParentDescriptor", pParent, i  ), "", PROP_TYPE_DONT_SHOW );
        List.PropEnumString( xfs( "%s\\ReferencingElement[%d]\\ParentDescriptor\\Label", pParent, i  ), "", PROP_TYPE_DONT_SHOW );
    }

}

//=========================================================================

void editor_descriptor::OnCheckIntegrity ( const char* pRscName )
{
    if( (m_pElements.GetCount() > 1) && (m_Type == EDITOR_SIMPLE) )
        x_throw( xfs( "You have a simple descriptor[%s] in package %s with more that one element", m_Label, pRscName ) );
    
    if( ((m_Volume < 0.0f) || (m_Volume > 1.0f)) && (m_Volume != -2.0f) )
        x_throw( xfs( "Volume parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_VolumeVar < 0.0f) || (m_VolumeVar > 1.0f)) && (m_VolumeVar != -2.0f) )
        x_throw( xfs( "Volume Var parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_Pitch < 0.015625f) || (m_Pitch > 4.0f)) && (m_Pitch != -2.0f) )
        x_throw( xfs( "Pitch parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_PitchVar < 0.0f) || (m_PitchVar > 1.0f)) && (m_PitchVar != -2.0f) )
        x_throw( xfs( "Pitch Var parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_Pan < -1.0f) || (m_Pan > 1.0f)) && (m_Pan != -2.0f) )
        x_throw( xfs( "Pan parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_Priority < 0) || (m_Priority > 255)) && (m_Priority != -2) )
        x_throw( xfs( "Priority parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_EffectSend < 0.0f) || (m_EffectSend > 1.0f)) && (m_EffectSend != -2.0f) )
        x_throw( xfs( "EffectSend parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );
    
    if( ((m_NearFalloff < 0.0f) || (m_NearFalloff > 10.0f)) && (m_NearFalloff != -2.0f) )
        x_throw( xfs( "NearFalloff parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_FarFalloff < 0.0f) || (m_FarFalloff > 10.0f)) && (m_FarFalloff != -2.0f) )
        x_throw( xfs( "FarFalloff parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_VolumeLfe < 0.0f) || (m_VolumeLfe > 1.0f)) && (m_VolumeLfe != -2.0f) )
        x_throw( xfs( "Volume Lfe parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_VolumeCenter < 0.0f) || (m_VolumeCenter > 1.0f)) && (m_VolumeCenter != -2.0f) )
        x_throw( xfs( "Volume Center parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_NearDiffuse < 0.0f) || (m_NearDiffuse > 10.0f)) )
        x_throw( xfs( "NearDiffuse parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_FarDiffuse < 0.0f) || (m_FarDiffuse > 10.0f)) )
        x_throw( xfs( "FarDiffuse parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );
    
    if( !(m_RollOff & LINEAR_ROLLOFF) && !(m_RollOff & FAST_ROLLOFF) && !(m_RollOff & SLOW_ROLLOFF) )
        x_throw( xfs( "Curruption in the rolloff type, consult a programmer asap, descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_PlayPercent < 0) || (m_PlayPercent > 100)) )
        x_throw( xfs( "PlayPercent parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    if( ((m_ReplayDelay < 0.0f) || (m_ReplayDelay > 100.0f)) )
        x_throw( xfs( "ReplayDelay parameter out of range for the descriptor [%s] in package %s", m_Label, pRscName  ) );

    s32 WarmSampleCount = 0;        
    s32 ColdSampleCount = 0;      
    s32 j = 0;  

    for( j = 0; j < m_pElements.GetCount(); j++ )
    {        
        if( m_pElements[j].m_Temperature == WARM )                    
            WarmSampleCount++;
        
        if( m_pElements[j].m_Temperature == COLD )
            ColdSampleCount++;
        
        m_pElements[j].OnCheckIntegrity( pRscName, m_Label, j );
    }

    // We have 2 stream dedecated to music and 2 to dialog.
    if( (ColdSampleCount > 2) && (m_Type == EDITOR_COMPLEX) )
        x_throw( xfs("Descriptor [%s] has more than 2 stream attached as elements", m_Label) );
    
//
// TODO: Check with Marc to see if he want more warnings for descriptors with cold and hot samples.
//
}

//=========================================================================
// ELEMENTS
//=========================================================================

editor_element::editor_element()
{
//    m_pDescriptor               = NULL;
    m_Temperature               = HOT;
    m_pSampleName[0]            = 0;        // We are going to use this as the label name for the samples.
    m_pSamplePathName[0]        = 0;
    m_ReferenceDescIndex        = -1;
    m_pReferenceLabel[0]        = 0;

    m_Volume        = 1.0f;
    m_VolumeVar     = 0.0f;
    m_Pitch         = 1.0f;
    m_PitchVar      = 0.0f;
    m_Pan           = 0.0f;
    m_Priority      = 128;
    m_EffectSend    = 1.0f;
    m_NearFalloff   = 1.0f;
    m_FarFalloff    = 1.0f;
    m_VolumeLfe     = 0.0f;
    m_VolumeCenter  = 0.0f;
    m_RollOff       |= LINEAR_ROLLOFF;
    m_NearDiffuse   = 1.0f;
    m_FarDiffuse    = 1.0f;
    m_PlayPercent   = 100;
    m_ReplayDelay   = 0.0f;

    m_Delta         = 0.0f;
}

//=========================================================================

void editor_element::Clear( void )
{
//    m_pDescriptor               = NULL;
    m_Temperature               = HOT;
    m_pSampleName[0]            = 0;        // We are going to use this as the label name for the samples.
    m_pSamplePathName[0]        = 0;    
    m_ReferenceDescIndex        = -1;
    m_pReferenceLabel[0]        = 0;

    m_Volume        = 1.0f;
    m_VolumeVar     = 0.0f;
    m_Pitch         = 1.0f;
    m_PitchVar      = 0.0f;
    m_Pan           = 0.0f;
    m_Priority      = 128;
    m_EffectSend    = 1.0f;
    m_NearFalloff   = 1.0f;
    m_FarFalloff    = 1.0f;
    m_VolumeLfe     = 0.0f;
    m_VolumeCenter  = 0.0f;
    m_RollOff       |= LINEAR_ROLLOFF;
    m_NearDiffuse   = 1.0f;
    m_FarDiffuse    = 1.0f;
    m_PlayPercent   = 100;
    m_ReplayDelay   = 0.0f;

    m_Delta         = 0.0f;
}

//=========================================================================

void editor_element::OnEnumProp( prop_enum& List, const char* pParent, s32 Count, s32 Type)
{  
    VERIFYS( pParent, "Null parent found in element OnEnumProp\n" );
    if( pParent == NULL )
        return;

    // Add all the editor_descriptor items to the property.
    List.PropEnumHeader( xfs( "%sElement[%d]", pParent, Count), "Element", 0 );

    // Add all the editor_element items to the proterty
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Volume",       pParent, Count  ), "Volume of the editor_element, Range( 0.0 -- 1.0 )", 0          );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Volume Var",   pParent, Count  ), "Volume Variance of the editor_element, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Volume LFE",   pParent, Count  ), "Volume LFE of the editor_element, Range( 0.0 -- 1.0 )", 0      );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Volume Center",pParent, Count  ), "Volume Center of the editor_element, Range( 0.0 -- 1.0 )", 0   );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Pitch",        pParent, Count  ), "The pitch of the editor_element, Range( 2^-6 -- 2^2 )", 0   );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Pitch Var",    pParent, Count  ), "Pitch variance of the editor_element, Range( 0.0 -- 1.0 )", 0  );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Pan",          pParent, Count  ), "Pan of the editor_element, Range( -1.0 -- 1.0 )", 0            );
    List.PropEnumInt   ( xfs( "%sElement[%d]\\Priority",     pParent, Count  ), "Priority of the editor_descriptor, Range( 0 -- 255)", 0        );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\EffectSend",   pParent, Count  ), "Reverb of the editor_element, Range( 0.0 -- 1.0 )", 0          );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Near Falloff", pParent, Count  ), "Near falloff of the editor_element, Range( 0.0 -- 10.0 )", 0    );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Far falloff",  pParent, Count  ), "Far falloff of the editor_element, Range( 0.0 -- 10.0 )", 0     );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Near Diffuse", pParent, Count  ), "Near diffuse of the editor_element, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Far Diffuse",  pParent, Count  ), "Far diffuse of the editor_element, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumInt   ( xfs( "%sElement[%d]\\Play Percent", pParent, Count  ), "Percantage the editor_descriptor will play, Range( 0[never] -- 100[always])", 0        );
    List.PropEnumFloat ( xfs( "%sElement[%d]\\Replay Delay", pParent, Count  ), "Replay delay of the editor_element, Range( 0.0 -- 100.0 )", 0 );
    List.PropEnumEnum  ( xfs( "%sElement[%d]\\Rolloff Curve",pParent, Count  ), "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );

    
    if( Type == EDITOR_COMPLEX )
    {
        List.PropEnumFloat ( xfs( "%sElement[%d]\\Time Delta", pParent, Count  ), "The displacment of time from the start for the sample.", 0 );
    }
    else if( Type == EDITOR_WEIGHTED )
    {
        List.PropEnumFloat ( xfs( "%sElement[%d]\\Weight", pParent, Count  ), "The weight of the sample.", 0 );
    }

    List.PropEnumInt   ( xfs( "%sElement[%d]\\RefDescriptor",            pParent, Count  ), "", PROP_TYPE_DONT_SHOW );
    List.PropEnumHeader( xfs( "%sElement[%d]\\ParentDescriptor",         pParent, Count  ), "", PROP_TYPE_DONT_SHOW );
    List.PropEnumString( xfs( "%sElement[%d]\\ParentDescriptor\\Label",  pParent, Count  ), "", PROP_TYPE_DONT_SHOW );

    // Is this editor_element refrencing another editor_descriptor.
    if( m_ReferenceDescIndex != -1 )
    {
        List.PropEnumHeader( xfs( "%sElement[%d]\\Descriptor",                 pParent, Count   ), "Label for a sound editor_descriptor", PROP_TYPE_READ_ONLY);  
        List.PropEnumString( xfs( "%sElement[%d]\\Descriptor\\Label",          pParent, Count   ), "Sound Descriptor (Label)", PROP_TYPE_READ_ONLY);

        List.PropEnumString( xfs( "%sElement[%d]\\ReferenceDescriptor\\Label", pParent, Count   ), "", PROP_TYPE_DONT_SHOW);
    }
    else
    {
        List.PropEnumEnum ( xfs( "%sElement[%d]\\Temperature",   pParent, Count  ), "HOT\0WARM\0COLD\0", "The temperature of the sample (Descriptor, Hot, Warm, Cold)", 0 );
        List.PropEnumString  ( xfs( "%sElement[%d]\\SampleName", pParent, Count  ), "Name of the sound file with the path", 0 );
        
        // We are going to use this as the label name for the samples.
        List.PropEnumString  ( xfs( "%sElement[%d]\\ShortSampleName", pParent, Count      ), "", PROP_TYPE_DONT_SHOW );
    }
}

//=========================================================================

xbool editor_element::OnProperty( prop_query& I, const char* pParent, s32 Count, s32 Type )
{
    VERIFYS( pParent, "Null parent found in element OnProperty\n" );
    if( pParent == NULL )
        return FALSE;

    // The Volume.
    if( I.VarFloat ( xfs( "%sElement[]\\Volume",        pParent ), m_Volume     ) )
        return TRUE;

    // The Volume var .
    if( I.VarFloat ( xfs( "%sElement[]\\Volume Var",    pParent ), m_VolumeVar  ) )
        return TRUE;

    // The Volume LFE.
    if( I.VarFloat ( xfs( "%sElement[]\\Volume LFE",    pParent ), m_VolumeLfe  ) )
        return TRUE;

    // The Volume center.
    if( I.VarFloat ( xfs( "%sElement[]\\Volume Center", pParent ), m_VolumeCenter ) )
        return TRUE;

    // The Pitch.
    if( I.VarFloat ( xfs( "%sElement[]\\Pitch",         pParent ), m_Pitch      ) )
    {
        if( (m_Pitch < 0.015625) )//&& (m_Pitch != -2) )
        {
            m_Pitch = 0.015625;
        }
        else if( m_Pitch > 4 )
        {
            m_Pitch = 4;
        }
        return TRUE;
    }

    // The Pitch Var.
    if( I.VarFloat ( xfs( "%sElement[]\\Pitch Var",     pParent ), m_PitchVar   ) )
        return TRUE;

    // The Pan.
    if( I.VarFloat ( xfs( "%sElement[]\\Pan",           pParent ), m_Pan        ) )
        return TRUE;

    // The Priority.
    if( I.VarInt   ( xfs( "%sElement[]\\Priority",      pParent ), m_Priority   ) )
        return TRUE;

    // The EffectSend.
    if( I.VarFloat ( xfs( "%sElement[]\\EffectSend",    pParent ), m_EffectSend ) )
        return TRUE;

    // The Near Falloff.
    if( I.VarFloat ( xfs( "%sElement[]\\Near Falloff",  pParent ), m_NearFalloff ) )
        return TRUE;

    // The Far falloff.
    if( I.VarFloat ( xfs( "%sElement[]\\Far falloff",   pParent ), m_FarFalloff ) )
        return TRUE;

    // The Near Diffuse.
    if( I.VarFloat ( xfs( "%sElement[]\\Near Diffuse",  pParent ), m_NearDiffuse ) )
        return TRUE;

    // The Far Diffuse.
    if( I.VarFloat ( xfs( "%sElement[]\\Far Diffuse",   pParent ),  m_FarDiffuse  ) )
        return TRUE;

    // The Play Percent.
    if( I.VarInt   ( xfs( "%sElement[]\\Play Percent", pParent  ), m_PlayPercent   ) )
        return TRUE;

    // The Replay Delay.
    if( I.VarFloat ( xfs( "%sElement[]\\Replay Delay", pParent  ), m_ReplayDelay  ) )
        return TRUE;

    // The rolloff curve.
    if( I.IsVar( xfs( "%sElement[]\\Rolloff Curve",   pParent ) ) )
    {
        if( I.IsRead () )
        {
            if( m_RollOff & LINEAR_ROLLOFF )
                I.SetVarEnum( "Linear" );
            else if( m_RollOff & FAST_ROLLOFF )
                I.SetVarEnum( "Fast" );
            else if( m_RollOff & SLOW_ROLLOFF )
                I.SetVarEnum( "Slow" );
            else
                ASSERT( 0 );
        }
        else
        {
            m_RollOff = 0;

            if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                m_RollOff |= LINEAR_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                m_RollOff |= FAST_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                m_RollOff |= SLOW_ROLLOFF;
            else
                ASSERT( 0 );
        }
        return TRUE;
    }

    // If its a complex or weighted sound display the delta which could be eigher time or weight.
    if( Type == EDITOR_COMPLEX )
    {
        if( I.VarFloat ( xfs( "%sElement[]\\Time Delta", pParent ), m_Delta ) )
            return TRUE;
    }
    else if( Type == EDITOR_WEIGHTED )
    {
        if( I.VarFloat ( xfs( "%sElement[]\\Weight",     pParent ), m_Delta ) )
            return TRUE;
    }
    
    // The temperature of the sample.
    if( I.IsVar( xfs( "%sElement[]\\Temperature", pParent   ) ) )
    {
        if( I.IsRead () )
        {
            switch( m_Temperature )
            {
                case HOT     : I.SetVarEnum( "HOT" ); break;
                case WARM    : I.SetVarEnum( "WARM" ); break;
                case COLD    : I.SetVarEnum( "COLD" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "HOT", I.GetVarEnum()) )
            {
                m_Temperature = HOT;
            }
            else if( !x_stricmp( "WARM", I.GetVarEnum() ) )
            {
                m_Temperature = WARM;
            }
            else if( !x_stricmp( "COLD", I.GetVarEnum() ) )
            {
                m_Temperature = COLD;
            }
        }
        return TRUE;
    }

    // Element Count on laod.
    if( I.VarInt   ( xfs( "%sElement[]\\RefDescriptor", pParent ), m_ReferenceDescIndex ) )
        return TRUE;

    // Store the parents label.
    if( I.VarString( xfs( "%sElement[]\\ParentDescriptor\\Label", pParent   ), m_ParentDesc.m_Label, 128 ) )
        return TRUE;

    // If the editor_element is refrencing a editor_descriptor then only show its label.
    //if( m_ReferenceDescIndex != -1 )
    {
        if( I.VarString( xfs( "%sElement[]\\Descriptor\\Label", pParent   ), m_pReferenceLabel, 128 ) )
            return TRUE;

        if( I.VarString( xfs( "%sElement[]\\ReferenceDescriptor\\Label", pParent   ), m_pReferenceLabel, 128 ) )
            return TRUE;
    }

    // If there is no editor_descriptor then show the sample path name.
    if( I.VarString  ( xfs( "%sElement[]\\SampleName", pParent      ), m_pSamplePathName, 128 ) )
    {
        // Truncate the string so only the the sample name shows.
        CString PathName = m_pSamplePathName;
        s32 LastHashMark = PathName.ReverseFind( '\\' );
        LastHashMark++;
        PathName = PathName.Right( PathName.GetLength() - LastHashMark );    
        s32 ExtIndex = PathName.Find( '.' );
        if( ExtIndex == -1 )
            x_throw( xfs( "Unable to find .aif extension in [%s]", pParent) );

        PathName.Delete( ExtIndex, PathName.GetLength() - ExtIndex );
        PathName = "f_" + PathName;
        x_strsavecpy( m_pSampleName, (LPCTSTR)PathName, 128 );
        
        return TRUE;
    }

    // If there is no editor_descriptor then add the short sample name.
    if( I.VarString  ( xfs( "%sElement[]\\ShortSampleName", pParent      ), m_pSampleName, 128 ) )
    {
        // If they are using old element name then change it so that they fit the new schem.
        CString SampleName( m_pSampleName );

        // We can't use Find here because some of the footfalls have "f_" in the middle of the sample name.
        if( (SampleName[0] != 'f') || (SampleName[1] != '_') )
        {
            SampleName = "f_" + SampleName;
            x_strsavecpy( m_pSampleName, (LPCTSTR)SampleName, 128 );
        }

        return TRUE;
    }
    
    return FALSE;
}

//=========================================================================

void editor_element::OnCheckIntegrity( const char* pRscName, const char* pDescName, s32 Index )
{
    if( ((m_Volume < 0.0f) || (m_Volume > 1.0f)) && (m_Volume != -2.0f) )
        x_throw( xfs( "Volume parameter out of range for the element element index %d in descriptor [%s] in package %s", Index, pDescName,pRscName ) );

    if( ((m_VolumeVar < 0.0f) || (m_VolumeVar > 1.0f)) && (m_VolumeVar != -2.0f) )
        x_throw( xfs( "Volume Var parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_Pitch < 0.015625f) || (m_Pitch > 4.0f)) && (m_Pitch != -2.0f) )
        x_throw( xfs( "Pitch parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_PitchVar < 0.0f) || (m_PitchVar > 1.0f)) && (m_PitchVar != -2.0f) )
        x_throw( xfs( "Pitch Var parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_Pan < -1.0f) || (m_Pan > 1.0f)) && (m_Pan != -2.0f) )
        x_throw( xfs( "Pan parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_Priority < 0) || (m_Priority > 255)) && (m_Priority != -2) )
        x_throw( xfs( "Priority parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_EffectSend < 0.0f) || (m_EffectSend > 1.0f)) && (m_EffectSend != -2.0f) )
        x_throw( xfs( "EffectSend parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_NearFalloff < 0.0f) || (m_NearFalloff > 10.0f)) && (m_NearFalloff != -2.0f) )
        x_throw( xfs( "NearFalloff parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_FarFalloff < 0.0f) || (m_FarFalloff > 10.0f)) && (m_FarFalloff != -2.0f) )
        x_throw( xfs( "FarFalloff parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_VolumeLfe < 0.0f) || (m_VolumeLfe > 1.0f)) && (m_VolumeLfe != -2.0f) )
        x_throw( xfs( "Volume Lfe parameter out of range for the element element index %d in descriptor [%s] in package %s", Index, pDescName,pRscName ) );

    if( ((m_VolumeCenter < 0.0f) || (m_VolumeCenter > 1.0f)) && (m_VolumeCenter != -2.0f) )
        x_throw( xfs( "Volume Center parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_NearDiffuse < 0.0f) || (m_NearDiffuse > 10.0f)) )
        x_throw( xfs( "NearDiffuse parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );

    if( ((m_FarDiffuse < 0.0f) || (m_FarDiffuse > 10.0f)) )
        x_throw( xfs( "FarDiffuse parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );
    
    if( ((m_PlayPercent < 0) || (m_PlayPercent > 100)) )
        x_throw( xfs( "PlayPercent parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName  ) );

    if( ((m_ReplayDelay < 0.0f) || (m_ReplayDelay > 100.0f)) )
        x_throw( xfs( "ReplayDelay parameter out of range for the element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName  ) );

    if( !(m_RollOff & LINEAR_ROLLOFF) && !(m_RollOff & FAST_ROLLOFF) && !(m_RollOff & SLOW_ROLLOFF) )
        x_throw( xfs( "Curruption in the rolloff type, consult a programmer asap, element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );
    
    if( m_ReferenceDescIndex == -1 )
        if( x_strlen(m_pSamplePathName) == 0 )
            x_throw( xfs( "Unreferenced element with no sample name detected, element index %d in descriptor [%s] in package %s", Index, pDescName, pRscName ) );
}
