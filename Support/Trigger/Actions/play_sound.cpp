///////////////////////////////////////////////////////////////////////////
//
//  play_sound.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\play_sound.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "..\Support\Sound\EventSoundEmitter.hpp"

#include "Entropy.hpp"

//=========================================================================
// PLAY_SOUND
//=========================================================================

play_sound::play_sound ( guid ParentGuid ) : actions_base( ParentGuid )
{
    m_Descriptor[0] = 0;
    m_StartDelay    = 0.0f;
}

//=============================================================================

void play_sound::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * play_sound::Execute" );
    
    (void) pParent;
    
    //Play the sound..

    vector3 Position = GetPositionOwner();

    //
    // Create a new simple sound emitter object.
    //
    guid Guid = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
    object* pSndObj = g_ObjMgr.GetObjectByGuid( Guid );
    event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pSndObj );
    
    EventEmitter.PlayEmitter( m_Descriptor, Position, pParent->GetZone1(), 
                             event_sound_emitter::SINGLE_SHOT, pParent->GetGuid(), 0, m_StartDelay ); 

}

//=============================================================================

void play_sound::OnEnumProp ( prop_enum& rPropList )
{
    //object info
    //rPropList.AddString     ( "Sound Name" ,    "Name of sound file to play." );
    rPropList.AddExternal   ( "Sound Descriptor",     "Resource\0soundexternal","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    rPropList.AddExternal   ( "Audio Package",  "Resource\0audiopkg",    "The audio package associated with this trigger action." );
    rPropList.AddFloat      ( "Sound Start Delay", "The time delay till we start the sound." );

    actions_base::OnEnumProp( rPropList );
    
}

//=============================================================================

xbool play_sound::OnProperty ( prop_query& rPropQuery )
{

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
     
//    if ( rPropQuery.VarString ( "Sound Name"  , m_Sound.Get(),  m_Sound.MaxLen()) )
//        return TRUE;
    // External
    if( rPropQuery.IsVar( "Sound Descriptor" ) )
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
        return TRUE;
      
    return FALSE;
}



