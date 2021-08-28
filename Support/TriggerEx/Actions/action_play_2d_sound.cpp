///////////////////////////////////////////////////////////////////////////
//
//  action_play_2d_sound.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_play_2d_sound.hpp"
#include "..\TriggerEx_Object.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Audio\audio_stream_controller.hpp"

extern audio_stream_controller g_ConversationStreamControl;

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_play_2d_sound::action_play_2d_sound ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{
    m_DescriptorName[0] = 0;
    m_bBlock            = FALSE;
    m_VoiceID           = 0;
    m_Streamed          = -1;
}

//=============================================================================

void action_play_2d_sound::OnActivate ( xbool Flag )
{
    (void)Flag;

    m_VoiceID           = 0;
}

//=============================================================================

xbool action_play_2d_sound::Execute ( f32 DeltaTime )
{
    (void)DeltaTime;

    if (m_VoiceID == 0)
    {
        if( m_Streamed == 1 )
        {
            m_VoiceID = g_ConversationStreamControl.Play( m_DescriptorName, TRUE );
        }
        else
        {
            //no voice, so play a new one
            m_VoiceID = g_AudioMgr.Play( m_DescriptorName );
        }
    }
    else
    {
        //already playing a voice, if its done set its voice id to 0
        if( !g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
        {
            // sound done playing.
            m_VoiceID = 0;
        }
    }

    if (m_bBlock && (m_VoiceID != 0))
    {
        //Blocking and still have valid voice id
        return FALSE;
    }
        
    return TRUE;
}

//=============================================================================

void action_play_2d_sound::OnEnumProp	( prop_enum& List )
{
    List.PropEnumExternal( "Sound Name", "Sound\0soundemitter\0","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    List.PropEnumExternal( "Audio Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumString  ( "Audio Package", "The audio package associated with this sound.", PROP_TYPE_READ_ONLY );
    List.PropEnumBool    ( "Block", "Do we block on this action?", 0);
    List.PropEnumEnum    ( "Streamed", "INVALID\0NORMAL\0STREAMED\0", "Is this streamed?", 0 );

    actions_ex_base::OnEnumProp( List );
}

//=============================================================================

xbool action_play_2d_sound::OnProperty	( prop_query& I )
{
    if( actions_ex_base::OnProperty( I ) )
        return TRUE;
  
    if ( I.VarBool("Block", m_bBlock))
    {
        return TRUE;
    }

    if( I.IsVar("Streamed") )
    {
        if( I.IsRead() )
        {
            if( m_Streamed == -1 )
                I.SetVarEnum( "INVALID" );
            else if( m_Streamed == 0 )
                I.SetVarEnum( "NORMAL" );
            else if( m_Streamed == 1 )
                I.SetVarEnum( "STREAMED" );
            else
                ASSERT( 0 );
        }
        else
        {
            if( x_stricmp( I.GetVarEnum(), "INVALID" ) == 0 )
                m_Streamed = -1;
            else if( x_stricmp( I.GetVarEnum(), "NORMAL" ) == 0 )
                m_Streamed = 0;
            else if( x_stricmp( I.GetVarEnum(), "STREAMED" ) == 0 )
                m_Streamed = 1;
            else
                ASSERT( 0 );
        }
        return TRUE;
    }

    // External
    if( I.IsVar( "Sound Name" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_DescriptorName, 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {

                xstring String( ExtString );

                if( String == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                    m_DescriptorName[0] = 0;
                }
                else
                {
                    s32 PkgIndex = String.Find( '\\', 0 );

                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );

                        m_hAudioPackage.SetName( Pkg );                

                        // Load the audio package.
                        if( m_hAudioPackage.IsLoaded() == FALSE )
                            m_hAudioPackage.GetPointer();
                    }

                    x_strncpy( m_DescriptorName, String, 64 );
                }
            }
        }
        return TRUE;
    }

    // External
    if( I.IsVar( "Audio Package External Resource" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return TRUE;        
    }

    if( I.IsVar( "Audio Package" ) )
    {
        // You can only read.
        if( I.IsRead() )
        {
            I.SetVarString( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

const char* action_play_2d_sound::GetDescription( void )
{
    if (m_DescriptorName[0])
    {
        static big_string   Info;
        if (m_bBlock)
        {
            Info.Set(xfs("* Play %s",m_DescriptorName));
        }
        else
        {
            Info.Set(xfs("Play %s",m_DescriptorName));
       }
        return Info.Get();    
    }
    else
    {
        return "Play Unknown 2D Sound";
    }
}


