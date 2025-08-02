//==============================================================================
// SOUND EMITTERS
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "SimpleSoundEmitter.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "gamelib\StatsMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "Audio\audio_stream_controller.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

extern audio_stream_controller g_ConversationStreamControl;

#define ACTIVATE        (1<<1)
#define DEACTIVATE      (1<<2)
#define DESTROY         (1<<3)
#define PLAY_SYNC       (1<<4)
#define PLAY_ASYNC      (1<<5)
#define PLAY_2D         (1<<6)
#define UPDATE_POS      (1<<7)

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
static struct simple_sound_emitter_desc : public object_desc
{
    simple_sound_emitter_desc( void ) : object_desc( 
        object::TYPE_SIMPLE_SND_EMITTER, 
        "Simple Sound Emitter", 
        "SOUND",

            object::ATTR_NEEDS_LOGIC_TIME            |
            object::ATTR_SOUND_SOURCE,
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC  ) {}         

    //---------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new simple_sound_emitter;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR
    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );

        if( (Object. GetAttrBits() & object::ATTR_EDITOR_SELECTED) || 
            (Object.GetAttrBits() & object::ATTR_EDITOR_PLACEMENT_OBJECT) )
            EditorIcon_Draw(EDITOR_ICON_SPEAKER, Object.GetL2W(), TRUE, XCOLOR_BLUE );
        else
            EditorIcon_Draw(EDITOR_ICON_SPEAKER, Object.GetL2W(), FALSE, XCOLOR_AQUA );
        
        Object.OnDebugRender();

        return -1;
    }
#endif // X_EDITOR

} s_SimpleSoundEmitter_Desc;

//=========================================================================

const object_desc&  simple_sound_emitter::GetTypeDesc( void ) const
{
    return s_SimpleSoundEmitter_Desc;
}

//=========================================================================

const object_desc&  simple_sound_emitter::GetObjectType( void )
{
    return s_SimpleSoundEmitter_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

simple_sound_emitter::simple_sound_emitter( void ) 
{
    m_DescriptorName[0] = 0;
    m_Flags             = 0;
    m_VoiceID           = 0;
    m_ReleaseTime       = 0.0f;
    m_bIsStreamed       = FALSE;

    m_Flags             |= PLAY_SYNC;
    m_Flags             |= DEACTIVATE;

    m_pAsyncVoiceID.Clear();
    m_pAsyncVoiceID.SetGrowAmount( 4 );
}

//==============================================================================

#ifdef X_EDITOR

s32 simple_sound_emitter::OnValidateProperties( xstring& ErrorMsg )
{
    // Audio package not found?
    const char* pPackage = m_hAudioPackage.GetName();
    if( ( !pPackage ) || ( pPackage[0] == 0 ) || ( x_stricmp( pPackage, "<null>" ) == 0 ) )
    {
        ErrorMsg += "No audio package assigned.\n";
        return 1;
    }        

    // Audio package not loaded?
    if( m_hAudioPackage.GetPointer() == NULL )
    {
        ErrorMsg += "Audio package [" + xstring( pPackage ) + "] not loaded.\n";
        return 1;
    }        

    // Invalid descriptor?
    if( g_AudioMgr.IsValidDescriptor( m_DescriptorName ) == FALSE )
    {
        ErrorMsg += "Sound [" + xstring( m_DescriptorName ) + "] does not exist.\n";
        return 1;
    }

    // Is it steamed?
    if( m_bIsStreamed )
    {
        if( !g_AudioMgr.IsCold( m_DescriptorName ) )
        {
            ErrorMsg += "Sound [" + xstring( m_DescriptorName ) + "] is NOT cold.\n";
            return 1;
        }
    }
    else
    {
        if( g_AudioMgr.IsCold( m_DescriptorName ) )
        {
            ErrorMsg += "Sound [" + xstring( m_DescriptorName ) + "] IS cold.\n";
            return 1;
        }
    }

    return 0;
}

#endif // X_EDITOR

//=========================================================================

void simple_sound_emitter::OnActivate( xbool Flag )
{
    if( Flag )
    {
        m_Flags |= ACTIVATE;

        SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );        

        // If its a asynchronous emitter play a sound immediatly.
        if( m_Flags & PLAY_ASYNC )
        {
            if( m_Flags & PLAY_2D )
                m_pAsyncVoiceID.Append( g_AudioMgr.Play( m_DescriptorName ) );
            else
                m_pAsyncVoiceID.Append( g_AudioMgr.PlayVolumeClipped( m_DescriptorName, GetPosition(), GetZone1(), TRUE ) );
        }
    }
    else
    {
        m_Flags &= ~ACTIVATE;
    }
}

//=========================================================================

void simple_sound_emitter::OnKill( void )
{
    // Clear the normal sound.
    if( m_VoiceID )
        g_AudioMgr.Release( m_VoiceID, m_ReleaseTime );
    
    // Clear the async sounds.
    for( s32 i = m_pAsyncVoiceID.GetCount()-1; i >=0 ; i-- )
        g_AudioMgr.Release( m_pAsyncVoiceID[i], m_ReleaseTime );

    m_pAsyncVoiceID.Clear();

    object::OnKill();
}

//=========================================================================

#ifndef X_RETAIL
void simple_sound_emitter::OnDebugRender( void )
{
    if( (GetAttrBits() & ATTR_EDITOR_SELECTED) || (GetAttrBits() & ATTR_EDITOR_PLACEMENT_OBJECT) )
    {
        draw_BBox( GetBBox(), XCOLOR_RED );
    }
    else
    {
        //draw_BBox( GetBBox(), XCOLOR_WHITE );
    }
}
#endif // X_RETAIL

//=========================================================================

void simple_sound_emitter::OnAdvanceLogic ( f32 DeltaTime )
{
    LOG_STAT(k_stats_Sound);
	
    CONTEXT( "sound_emitter::OnAdvanceLogic" );
    (void)DeltaTime;
    
    // Play synchronous.
    if( m_Flags & PLAY_SYNC )
    {
        if( m_Flags & ACTIVATE )
        {
            if( m_VoiceID == 0 )
            {
                if( m_bIsStreamed )
                {
                    if( m_Flags & PLAY_2D )
                        m_VoiceID =  g_ConversationStreamControl.Play( m_DescriptorName );
                    else
                        m_VoiceID =  g_ConversationStreamControl.PlayVolumeClipped( m_DescriptorName, GetPosition(), GetZone1() );
                }
                else
                {
                    if( m_Flags & PLAY_2D )
                        m_VoiceID =  g_AudioMgr.Play( m_DescriptorName );
                    else
                        m_VoiceID =  g_AudioMgr.PlayVolumeClipped( m_DescriptorName, GetPosition(), GetZone1(), TRUE );
                }
                return;
            }

            // Is the sound done playing.
            if( (!g_AudioMgr.IsValidVoiceId( m_VoiceID )) )
            {
                if( m_Flags & DEACTIVATE )
                {
                    m_Flags &= ~ACTIVATE;
                }
                else if( m_Flags & DESTROY )
                {
                    g_ObjMgr.DestroyObject( GetGuid() );
                }
            }
            else if( (m_Flags & UPDATE_POS) && g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
            {
                g_AudioMgr.SetPosition( m_VoiceID, GetPosition(), GetZone1() );
            }
        }
        else
        {
            if( m_VoiceID )
            {
                g_AudioMgr.Release( m_VoiceID, m_ReleaseTime );
                m_VoiceID = 0;
            }

            // Stop running logic.
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
        }
    }
    else // Play asynchronous.
    {
        if( m_Flags & ACTIVATE )
        {
            // Only play this sound if the emitter initially only started with start activate.
            if( m_VoiceID == 0 && m_pAsyncVoiceID.GetCount() == 0)
            {
                if( m_Flags & PLAY_2D )
                    m_VoiceID =  g_AudioMgr.Play( m_DescriptorName );
                else
                    m_VoiceID =  g_AudioMgr.PlayVolumeClipped( m_DescriptorName, GetPosition(), GetZone1(), TRUE );

                return;
            }

            // Check all of the async voice array.
            for( s32 i = m_pAsyncVoiceID.GetCount()-1; i >=0 ; i-- )
            {
                // Is the sound done playing.
                if( (!g_AudioMgr.IsValidVoiceId( m_pAsyncVoiceID[i] )) )
                {
                    m_pAsyncVoiceID.Delete( i );
                }
                else if( (m_Flags & UPDATE_POS) && g_AudioMgr.IsValidVoiceId( m_pAsyncVoiceID[i] ) )
                {
                    g_AudioMgr.SetPosition( m_pAsyncVoiceID[i], GetPosition(), GetZone1() );
                }
            }

            // Are all the sounds done playing.
            if( (m_pAsyncVoiceID.GetCount() == 0) && 
                (!g_AudioMgr.IsValidVoiceId( m_VoiceID )) )
            {
                if( m_Flags & DEACTIVATE )
                {
                    m_Flags &= ~ACTIVATE;

                    // Stop running logic.
                    SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
                }
                else if( m_Flags & DESTROY )
                {
                    g_ObjMgr.DestroyObject( GetGuid() );
                }            
            }
        }
        else
        {
            // Clear all of the sounds.
            if( m_VoiceID )
            {
                g_AudioMgr.Release( m_VoiceID, m_ReleaseTime );
                m_VoiceID = 0;
            }

            for( s32 i = m_pAsyncVoiceID.GetCount()-1; i >=0 ; i-- )
                g_AudioMgr.Release( m_pAsyncVoiceID[i], m_ReleaseTime );

            m_pAsyncVoiceID.Clear();

            // Stop running logic.
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );            
        }
    }
}

//=========================================================================

void simple_sound_emitter::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
    
    if( m_Flags & PLAY_2D )
        return;

    if( m_VoiceID )
        g_AudioMgr.SetPosition( m_VoiceID, NewPos, GetZone1() );

    for( s32 i = 0; i < m_pAsyncVoiceID.GetCount(); i++ )
    {
        g_AudioMgr.SetPosition( m_pAsyncVoiceID[i], NewPos, GetZone1() );
    }
}

//=========================================================================

void simple_sound_emitter::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );

    if( m_Flags & PLAY_2D )
        return;

    if( m_VoiceID )
        g_AudioMgr.SetPosition( m_VoiceID, L2W.GetTranslation(), GetZone1() );

    for( s32 i = 0; i < m_pAsyncVoiceID.GetCount(); i++ )
    {
        g_AudioMgr.SetPosition( m_pAsyncVoiceID[i], L2W.GetTranslation(), GetZone1() );
    }

}

//=========================================================================

bbox simple_sound_emitter::GetLocalBBox( void ) const
{
    bbox Box( vector3(0.0f, 0.0f, 0.0f), 50.0f );
    return Box;
}

//=========================================================================

void simple_sound_emitter::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "Simple Sound Emitter", "Sound Emitter Properties", 0 );
    List.PropEnumExternal( "Simple Sound Emitter\\Label", "Sound\0soundemitter\0","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    List.PropEnumExternal( "Simple Sound Emitter\\Audio Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumString  ( "Simple Sound Emitter\\Audio Package", "The audio package associated with this prop object.", PROP_TYPE_READ_ONLY );

    List.PropEnumBool    ( "Simple Sound Emitter\\Start Active", "Start playing sound from the start of the game or wait for an activate call", 0 );
    List.PropEnumEnum    ( "Simple Sound Emitter\\Play Style", "Synchronous\0Asynchronous",                       
                            "In Synchronous style it doesn't matter how many time the"
                            "the emitter is activated it will play the sample once and"
                            "then try to follow its ending routine.  The Aynchronous"
                            "style will play the sample everytime you activate the object"
                            "and when all of the sounds its playing are done then it will"
                            "continue to its ending routines.", PROP_TYPE_MUST_ENUM );
                      
    List.PropEnumEnum    ( "Simple Sound Emitter\\Ending Routine", "Deactivate\0Destroy",
                            "When the emitter deactivates it just stops running its logic "
                            "however when the emitter destories its self its gone for the "
                            "life of the game, which means it can't be used by another trigger", 0 );                       
    
    List.PropEnumBool    ( "Simple Sound Emitter\\Play 2D", "Play the sound in the player's ear, non-positional", 0 );
    List.PropEnumFloat   ( "Simple Sound Emitter\\Release Time", "How long to release the sound over", 0 );

    if( (m_Flags & PLAY_2D) == FALSE )
    {
        List.PropEnumBool    ( "Simple Sound Emitter\\Update Position", "Keep updating the sound's position while its being played."
                                "NOTE: This is expensive, only use on sounds that will last long enough for"
                                "you to notice its position updating.", 0 );
    }

    if( m_Flags & PLAY_SYNC )
    {
        List.PropEnumBool    ( "Simple Sound Emitter\\Stream Emitter", "Is this a streamed emitter", 0 );
    }
}

//=========================================================================

xbool simple_sound_emitter::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return TRUE;

    // External
    if( I.IsVar( "Simple Sound Emitter\\Label" ) )
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
        return( TRUE );
    }

    // External
    if( I.IsVar( "Simple Sound Emitter\\Audio Package External Resource" ) )
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
        return( TRUE );        
    }

    if( I.IsVar( "Simple Sound Emitter\\Audio Package" ) )
    {
        // You can only read.
        if( I.IsRead() )
        {
            I.SetVarString( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        return( TRUE );
    }
    
    // Do we a sound from the get go.
    if( I.IsVar( "Simple Sound Emitter\\Start Active" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( (m_Flags & ACTIVATE) );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= ACTIVATE;
            }
            else
            {
                m_Flags &= ~ACTIVATE;
            }
        }   

        if( m_Flags & ACTIVATE )
            SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
        else
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );

        return( TRUE );
    }

    // The Type.
    if( I.IsVar( "Simple Sound Emitter\\Play Style" ) )
    {
        if( I.IsRead () )
        {
            if( m_Flags & PLAY_SYNC )
                I.SetVarEnum( "Synchronous" );
            else if( m_Flags & PLAY_ASYNC )
                I.SetVarEnum( "Asynchronous" );
            else
                ASSERTS( FALSE, "Didn't set the type"  );
        }
        else
        {
            if( !x_stricmp( "Synchronous", I.GetVarEnum()) )
            {
                m_Flags |= PLAY_SYNC;
                m_Flags &= ~PLAY_ASYNC;
                m_bIsStreamed = FALSE;
            }
            else if( !x_stricmp( "Asynchronous", I.GetVarEnum() ) )
            {
                m_Flags |= PLAY_ASYNC;
                m_Flags &= ~PLAY_SYNC;
                m_bIsStreamed = FALSE;
            }
            else
            {
                ASSERTS( FALSE, "Didn't set the type"  );
            }
        }

        return( TRUE );
    }

    // What to do after finishing playing the sample.
    if( I.IsVar( "Simple Sound Emitter\\Ending Routine" ) )
    {
        if( I.IsRead () )
        {
            if( m_Flags & DEACTIVATE )
                I.SetVarEnum( "Deactivate" );
            else if( m_Flags & DESTROY )
                I.SetVarEnum( "Destroy" );
            else
                ASSERTS( FALSE, "Didn't set the type"  );
        }
        else
        {
            if( !x_stricmp( "Deactivate", I.GetVarEnum()) )
            {
                m_Flags |= DEACTIVATE;
                m_Flags &= ~DESTROY;
            }
            else if( !x_stricmp( "Destroy", I.GetVarEnum() ) )
            {
                m_Flags |= DESTROY;
                m_Flags &= ~DEACTIVATE;
            }
            else
            {
                ASSERTS( FALSE, "Didn't set the type"  );
            }
        }

        return( TRUE );
    }

    // Do we a sound from the get go.
    if( I.IsVar( "Simple Sound Emitter\\Play 2D" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( (m_Flags & PLAY_2D) );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= PLAY_2D;

                if( m_Flags & UPDATE_POS )
                    m_Flags &= ~UPDATE_POS;
            }
            else
            {
                m_Flags &= ~PLAY_2D;
            }
        }   

        return( TRUE );
    }


    // Do we a sound from the get go.
    if( I.IsVar( "Simple Sound Emitter\\Update Position" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( (m_Flags & UPDATE_POS) );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Flags |= UPDATE_POS;
            }
            else
            {
                m_Flags &= ~UPDATE_POS;
            }
        }   

        return( TRUE );
    }

    if( I.VarFloat( "Simple Sound Emitter\\Release Time", m_ReleaseTime ) )
    {
        m_ReleaseTime = MAX( 0.0f, m_ReleaseTime );
        return TRUE;
    }

    if( I.VarBool( "Simple Sound Emitter\\Stream Emitter", m_bIsStreamed ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( m_bIsStreamed );
        }
        else
        {
            m_bIsStreamed = I.GetVarBool();
        }
        return TRUE;
    }

    return( FALSE );
}

//=========================================================================
