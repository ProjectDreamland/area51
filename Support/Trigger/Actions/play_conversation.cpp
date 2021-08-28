///////////////////////////////////////////////////////////////////////////
//
//  play_conversation.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\play_conversation.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Sound\EventSoundEmitter.hpp"
#include "..\Support\Trigger\Trigger_Spatial_Object.hpp"
#include "..\Support\Objects\Event.hpp"
#include "..\Support\Objects\Player.hpp"
#include "Entropy.hpp"


//=========================================================================
// PLAY_SOUND
//=========================================================================

play_conversation::play_conversation ( guid ParentGuid ) : actions_base( ParentGuid )
{
    m_Descriptor[0] = 0;
    m_StartDelay    = 0.0f;
    m_Flags         = 0;
}


//=============================================================================

void play_conversation::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * play_conversation::Execute" );
    ASSERT( pParent );
    (void) pParent;
    
    //Play the sound..

//    object* pObj = pParent;

/////////////////
// TODO:    Need to fix this HACK!!!!!!!
/////////////////
    //
    // Create a new simple sound emitter object.
    //
    guid Guid = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
    object* pSndObj = g_ObjMgr.GetObjectByGuid( Guid );
    event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pSndObj );
    
    if( pParent->IsKindOf(  trigger_spatial_object::GetRTTI() ) )
    {
        trigger_spatial_object& Parent = trigger_spatial_object::GetSafeType( *pParent );
        const guid*             pActorGuid = Parent.GetTriggerActor();
        vector3                 Position = GetPositionOwner();
    
        if( pActorGuid && *pActorGuid )
        {
            EventEmitter.PlayEmitter( m_Descriptor, Position, pParent->GetZone1(), 
                                     event_sound_emitter::CONVERSATION, *pActorGuid, m_Flags, m_StartDelay ); 
        }
        else
        {
            EventEmitter.PlayEmitter( m_Descriptor, Position, pParent->GetZone1(), 
                                     event_sound_emitter::CONVERSATION, pParent->GetGuid(), m_Flags, m_StartDelay ); 
        }
    }
    else
    {
        // Player
        player* pPlayer = SMP_UTIL_GetActivePlayer();
        vector3 Position = pPlayer->GetPosition();

        EventEmitter.PlayEmitter( m_Descriptor, Position, 0, 
                                 event_sound_emitter::CONVERSATION, pPlayer->GetGuid(), m_Flags, m_StartDelay ); 
    }
}

//=============================================================================

void play_conversation::OnEnumProp ( prop_enum& rPropList )
{

    //object info
    //rPropList.AddString     ( "Sound Name" ,    "Name of sound file to play." );
    
    rPropList.AddExternal   ( "Sound Name", "Resource\0soundexternal","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );

    rPropList.AddExternal   ( "Audio Package",  "Resource\0audiopkg","The audio package associated with this trigger action." );
    
    rPropList.AddFloat      ( "Sound Start Delay", "The time delay till we start the sound. NOTE: It could take up to 1.5sec to play a stream sample" );
    rPropList.AddEnum       ( "Conversation Type", "DEFAULT\0VOICE\0", "Is this a dialog or a stream audio sample" );

    actions_base::OnEnumProp( rPropList );
    
}

//=============================================================================

xbool play_conversation::OnProperty ( prop_query& rPropQuery )
{

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
     
    // External
    if( rPropQuery.IsVar( "Sound Name" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_Descriptor, 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = rPropQuery.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

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

                x_strncpy( m_Descriptor, String, 64 );
            }
        }
        return( TRUE );
    }

    // External
    if( rPropQuery.IsVar( "Audio Package" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_hAudioPackage.SetName( pString );                

                // Load the audio package.
                if( m_hAudioPackage.IsLoaded() == FALSE )
                    m_hAudioPackage.GetPointer();
            }
        }
        return( TRUE );
    }

    if( rPropQuery.VarFloat( "Sound Start Delay", m_StartDelay ) )
        return( TRUE );
    
    if( rPropQuery.IsVar ( "Conversation Type" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_Flags == VOICE_EVENT )
                rPropQuery.SetVarEnum( "VOICE" );
            else
                rPropQuery.SetVarEnum( "DEFAULT" );
        }
        else
        {
            if( !x_stricmp( "VOICE", rPropQuery.GetVarEnum() ) )
                m_Flags = VOICE_EVENT;
            else
                m_Flags = 0;
        }
        return( TRUE );
    }

    return( FALSE );
}