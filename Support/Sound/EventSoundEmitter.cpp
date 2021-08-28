//==============================================================================
// EVENT SOUND EMITTERS
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "EventSoundEmitter.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Animation\AnimData.hpp"
#include "Objects\Event.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "gamelib\StatsMgr.hpp"
#include "Characters\Character.hpp"
#include "Objects\Player.hpp"

#ifndef X_EDITOR
#include "Objects\NetGhost.hpp"
#endif

//=========================================================================
// GLOBALS
//=========================================================================

#define FOOTFALL_COLLISION_DEPTH    10.0f
#define MIN_INDEX   1
#define MAX_INDEX   4

xbool s_EventContactDebug = FALSE;
//#define NO_MATERIAL_CHECK

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
static struct event_sound_emitter_desc : public object_desc
{
    event_sound_emitter_desc( void ) : object_desc( 
        object::TYPE_EVENT_SND_EMITTER, 
        "Event Sound Emitter", 
        "SOUND",
            object::ATTR_NEEDS_LOGIC_TIME            |
            object::ATTR_SOUND_SOURCE,
            FLAGS_IS_DYNAMIC) {}         

    //---------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new event_sound_emitter;
    }

} s_EventSoundEmitter_Desc;

//=========================================================================

const object_desc&  event_sound_emitter::GetTypeDesc( void ) const
{
    return s_EventSoundEmitter_Desc;
}

//=========================================================================

const object_desc&  event_sound_emitter::GetObjectType( void )
{
    return s_EventSoundEmitter_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

event_sound_emitter::event_sound_emitter( void ) 
{
    m_DescriptorName[0] = 0;
    m_ActionName[0]     = 0;
    m_ObjectName[0]     = 0;
    m_VoiceID           = 0;
    m_ParentGuid        = 0;
    m_Active            = TRUE;
    m_Delay             = 0.0f;
    m_Flags             = 0;
    m_Radius            = 0.0f;
    m_SoundType         = SINGLE_SHOT;
    m_SphereTest        = FALSE;
    m_b2D               = FALSE;
}

//=========================================================================

void event_sound_emitter::OnAdvanceLogic ( f32 DeltaTime )
{
    LOG_STAT(k_stats_Sound);
	
    CONTEXT( "event_sound_emitter::OnAdvanceLogic" );
    (void)DeltaTime;
    
    // Check if its time for the sound to start yet.
    if( m_Delay > 0.0f )
    {
        m_Delay -= DeltaTime;
        
        if( m_Delay <= 0.0f )
        {
            if( m_b2D )
                StartEmitter2D();
            else
                StartEmitter();
        }
        else
        {
            return;
        }
    }

    // If its a single_shot sound then only destroy the object when its done playing.
    // If its a looped sound then keep the object alive till it get update from OnMove.
    // If its a conversation sound then don't kill the object untill the sound becomes inactive.
    
    else if( m_SoundType == SINGLE_SHOT || m_SoundType == CONTACT )
    {
        //if( (g_AudioManager.IsPlaying( m_VoiceID ) == FALSE) && (g_AudioManager.IsStarting( m_VoiceID ) ==  FALSE) )
        //    g_ObjMgr.DestroyObject( GetGuid() );

        if( !g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
            g_ObjMgr.DestroyObject( GetGuid() );
    }
    else if( m_SoundType == LOOPED )
    {
        if( m_Active == FALSE )
        {
            g_AudioMgr.Release( m_VoiceID, 0.0f );
            g_ObjMgr.DestroyObject( GetGuid() );
        }
    }
    else if( (m_SoundType == CONVERSATION) ||  (m_Flags == VOICE_EVENT) )
    {
        //object* pObject = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

        if( g_ConverseMgr.IsActive( m_VoiceID ) == FALSE )
            g_ObjMgr.DestroyObject( GetGuid() );
        //else
            //g_ConverseMgr.SetPosition( m_VoiceID, pObject->GetPosition(), 
    }

    m_Active = FALSE;
}

//=========================================================================

void event_sound_emitter::PlayEmitter( const char* pDescriptor, vector3& Position, u16 ZoneID, sound_type Type, 
                                        guid ParentGuid, u32 Flags, f32 Delay, xbool UseRadius, f32 Radius, xbool Play2D )
{
    (void)ZoneID;

    ASSERT( pDescriptor != NULL );
    ASSERT( ((Type >= FIRST_TYPE) && (Type <= LAST_TYPE)) );

    object* pObject = g_ObjMgr.GetObjectByGuid( ParentGuid );

    // Set the zone and the position.
    SetZone1( ZoneID );
    object::OnMove( Position );
    
    m_SphereTest    = UseRadius;
    m_Radius        = Radius;

    if( Type == CONTACT )
    {
        if( pObject && pObject->IsKindOf(character::GetRTTI()) )
        {
            character &ourCharacter = character::GetSafeType( *pObject );
            x_sprintf( m_DescriptorName, "%s_%s_%s", ourCharacter.GetDialogPrefix(), pDescriptor, GetMaterialTypeFromActor( ParentGuid ) );
        }
#ifndef X_EDITOR        
        else if( pObject && pObject->IsKindOf( net_ghost::GetRTTI() ) )
        {
            net_ghost& NetGhost = net_ghost::GetSafeType( *pObject );
        
            // Play heel sfx?
            if( x_stricmp( pDescriptor, "Footfall_Heel" ) == 0 )       
            {
                // Use player footfall heel sfx
                x_strcpy( m_DescriptorName, player::GetFootfallHeel( NetGhost.GetFloorMaterial() ) );
            }                
            else if( x_stricmp( pDescriptor, "Footfall_Toe" ) == 0 )       
            {            
                // Use player footfall toe sfx
                x_strcpy( m_DescriptorName, player::GetFootfallToe( NetGhost.GetFloorMaterial() ) );
            }                
            else
            {
                ASSERTS( 0, xfs( "Ghost audio \"%s\" not supported - grab SteveB!", pDescriptor ) );
                x_sprintf( m_DescriptorName, "%s_%s", pDescriptor, GetMaterialType() );
            }                
        }
#endif        
        else
        {
            x_sprintf( m_DescriptorName, "%s_%s", pDescriptor, GetMaterialType() );
        }
    }
    else if( pObject )
    {
        if( pObject->IsKindOf(character::GetRTTI()) )
        {
            character &ourCharacter = character::GetSafeType( *pObject );
            x_sprintf( m_DescriptorName, "%s_%s", ourCharacter.GetDialogPrefix(), pDescriptor );
            x_sprintf( m_ObjectName, "%s", ourCharacter.GetDialogPrefix() );
        }
        else if( pObject->GetType() == object::TYPE_WEAPON_SMP )
        {
            x_sprintf( m_DescriptorName, "SMP_%s", pDescriptor );
            x_sprintf( m_ObjectName, "SMP" );
        }
        else if( pObject->GetType() == object::TYPE_WEAPON_DUAL_SMP )
        {
            x_sprintf( m_DescriptorName, "2MP_%s", pDescriptor );
            x_sprintf( m_ObjectName, "2MP" );
        }
// KSS -- TO ADD NEW WEAPON
        else if( pObject->GetType() == object::TYPE_WEAPON_SHOTGUN )
        {
            x_sprintf( m_DescriptorName, "SHT_%s", pDescriptor );
            x_sprintf( m_ObjectName, "SHT" );
        }

        else if( pObject->GetType() == object::TYPE_WEAPON_DUAL_SHT )
        {
            x_sprintf( m_DescriptorName, "2SH_%s", pDescriptor );
            x_sprintf( m_ObjectName, "2SH" );
        }

        else if( pObject->GetType() == object::TYPE_WEAPON_SCANNER )
        {
            x_sprintf( m_DescriptorName, "SCN_%s", pDescriptor );
            x_sprintf( m_ObjectName, "SCN" );
        }

        else if( pObject->GetType() == object::TYPE_WEAPON_SNIPER )
        {
            x_sprintf( m_DescriptorName, "SNI_%s", pDescriptor );
            x_sprintf( m_ObjectName, "SNI" );
        }
    
        else if( pObject->GetType() == object::TYPE_WEAPON_GAUSS )
        {
            x_sprintf( m_DescriptorName, "GAS_%s", pDescriptor );
            x_sprintf( m_ObjectName, "GAS" );
        }

        else if( pObject->GetType() == object::TYPE_WEAPON_DESERT_EAGLE )
        {
            x_sprintf( m_DescriptorName, "EGL_%s", pDescriptor );
            x_sprintf( m_ObjectName, "EGL" );
        }

        else if( pObject->GetType() == object::TYPE_WEAPON_MHG )
        {
            x_sprintf( m_DescriptorName, "MHG_%s", pDescriptor );
            x_sprintf( m_ObjectName, "MHG" );
        }
        else if( pObject->GetType() == object::TYPE_WEAPON_MSN )
        {
            x_sprintf( m_DescriptorName, "MSN_%s", pDescriptor );
            x_sprintf( m_ObjectName, "MSN" );
        }
        else if( pObject->GetType() == object::TYPE_WEAPON_BBG )
        {
            x_sprintf( m_DescriptorName, "BBG_%s", pDescriptor );
            x_sprintf( m_ObjectName, "BBG" );
        }
        else if( pObject->GetType() == object::TYPE_WEAPON_TRA )
        {
            x_sprintf( m_DescriptorName, "TRA_%s", pDescriptor );
            x_sprintf( m_ObjectName, "TRA" );
        }
        else if( pObject->GetType() == object::TYPE_WEAPON_MUTATION )
        {
            x_sprintf( m_DescriptorName, "MUT_%s", pDescriptor );
            x_sprintf( m_ObjectName, "MUT" );
        }
        else
        {
            x_strncpy( m_DescriptorName, pDescriptor, 64 );
        }
    }

    x_strncpy( m_ActionName, pDescriptor, 64 );

    m_ParentGuid = ParentGuid;
    m_SoundType  = Type;
    m_Delay      = Delay;
    m_Flags      = Flags;
    
    //
    // NOTE: This should be a separate call to the event sound emitter but since 
    //       I don't have time, this will have to do.
    //
    
    m_b2D        = Play2D;
    
    if( m_Delay == 0.0f )
    {
        if( m_b2D )
            StartEmitter2D();
        else
            StartEmitter();
    }
}   

//=========================================================================

void event_sound_emitter::StartEmitter( void )
{
    u16 ZoneID = GetZone1();

    // TODO: Need to also give it the zoneid.
    if( m_Flags == GUN_SHOT_EVENT )
    {
        m_VoiceID = g_AudioMgr.PlayVolumeClipped( m_DescriptorName, GetPosition(), ZoneID, TRUE );
        g_AudioManager.NewAudioAlert( m_VoiceID, audio_manager::GUN_SHOT, GetPosition(), ZoneID, m_ParentGuid );
    }
    else if( m_Flags == EXPLOSION_EVENT )
    {
        m_VoiceID = g_AudioMgr.Play( m_DescriptorName, GetPosition(), ZoneID, TRUE );
        g_AudioManager.NewAudioAlert( m_VoiceID, audio_manager::EXPLOSION, GetPosition(), ZoneID, m_ParentGuid );
    }
    else if( m_Flags == VOICE_EVENT )
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

        // actors handle it this way
        if( pObj->IsKindOf(actor::GetRTTI()) )
        {
            dialog_event dialogEvent;
            dialogEvent.Type        = event::EVENT_DIALOG;
            
            x_strcpy( dialogEvent.DialogName, m_DescriptorName );//m_ActionName );
            dialogEvent.HotVoice    = m_SoundType != CONVERSATION;
            pObj->OnEvent( dialogEvent );       
        }
        // all others handle it this way
        else
        {
            if( m_SoundType == CONVERSATION )
            {
                m_VoiceID = g_ConverseMgr.PlayStream( m_ObjectName, m_ActionName, GetGuid(), ZoneID, GetPosition(), 1.0f );
            }
            else
            {
                m_VoiceID = g_ConverseMgr.PlayHotVoice( m_ObjectName, m_ActionName, GetPosition(), ZoneID, 1.0f );
            }

            voice_event VoiceEvent;
            VoiceEvent.Type     = event::EVENT_VOICE;
            VoiceEvent.VoiceID  = m_VoiceID;

            pObj->OnEvent( VoiceEvent );        
        }
    }
    else
    {
        if( m_SoundType == CONVERSATION )
            m_VoiceID = g_ConverseMgr.PlayStream( m_DescriptorName, GetPosition(), GetGuid(), GetZone1() );
        else
            m_VoiceID = g_AudioMgr.PlayVolumeClipped( m_DescriptorName, GetPosition(), GetZone1(), TRUE );
    }

    m_Active = TRUE;
}

//=========================================================================

void event_sound_emitter::StartEmitter2D( void )
{
    // TODO: Need to also give it the zoneid.
    if( m_Flags == GUN_SHOT_EVENT )
    {
        m_VoiceID = g_AudioMgr.Play( m_DescriptorName );
        g_AudioManager.NewAudioAlert( m_VoiceID, audio_manager::GUN_SHOT, m_ParentGuid );
    }
    else if( m_Flags == EXPLOSION_EVENT )
    {
        m_VoiceID = g_AudioMgr.Play( m_DescriptorName );
        g_AudioManager.NewAudioAlert( m_VoiceID, audio_manager::EXPLOSION, m_ParentGuid );
    }
    else if( m_Flags == VOICE_EVENT )
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

        // actors handle it this way
        if( pObj->IsKindOf(actor::GetRTTI()) )
        {
            dialog_event dialogEvent;
            dialogEvent.Type        = event::EVENT_DIALOG;
            
            x_strcpy( dialogEvent.DialogName, m_DescriptorName );//m_ActionName );
            dialogEvent.HotVoice    = m_SoundType != CONVERSATION;
            pObj->OnEvent( dialogEvent );       
        }
        // all others handle it this way
        else
        {
            if( m_SoundType == CONVERSATION )
            {
                m_VoiceID = g_ConverseMgr.PlayStream( m_ObjectName, m_ActionName, GetGuid(), GetZone1(),
                                                    GetPosition(), 1.0f, TRUE, PLAY_2D );
            }
            else
            {
                m_VoiceID = g_ConverseMgr.PlayHotVoice( m_ObjectName, m_ActionName, 
                                                    GetPosition(), GetZone1(), 1.0f, TRUE, PLAY_2D );
            }

            voice_event VoiceEvent;
            VoiceEvent.Type     = event::EVENT_VOICE;
            VoiceEvent.VoiceID  = m_VoiceID;

            pObj->OnEvent( VoiceEvent );        
        }    
    }
    else
    {
        if( m_SoundType == CONVERSATION )
            m_VoiceID = g_ConverseMgr.PlayStream( m_DescriptorName, GetPosition(), GetGuid(), GetZone1(), IMMEDIATE_PLAY,
                                                    TRUE, PLAY_2D );
        else
            m_VoiceID = g_AudioMgr.Play( m_DescriptorName );
    }

    m_Active = TRUE;
}

//=========================================================================

void event_sound_emitter::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );

    m_Active     = TRUE;

    if( m_b2D )
        return;

    g_AudioMgr.SetPosition( m_VoiceID, NewPos, GetZone1() );
}

//=========================================================================

bbox event_sound_emitter::GetLocalBBox( void ) const
{
    bbox Box( vector3(0.0f, 0.0f, 0.0f), 0.0f );
    return Box;
}

//=========================================================================

const char* event_sound_emitter::GetMaterialType( void )
{
    s32 CollisionMat = MAT_TYPE_NULL;    

#ifdef NO_MATERIAL_CHECK
    
    return GetMaterialName( CollisionMat );

#endif

    // We need to see what tri the player is colliding against so we can play the correct sound.
    vector3 Start ( GetPosition() ); 
    vector3 End   ( Start );
    Start.GetY() += FOOTFALL_COLLISION_DEPTH;
    End.GetY()   -= FOOTFALL_COLLISION_DEPTH*2.0f;


#if !defined( CONFIG_RETAIL )
    if( s_EventContactDebug )
    {
        draw_Marker( GetPosition(), XCOLOR_BLUE );
        draw_Marker( Start, XCOLOR_BLUE );
        draw_Line( Start, End, XCOLOR_BLUE );
        draw_Marker( End, XCOLOR_BLUE );
    }
#endif // !defined( CONFIG_RETAIL )

    if( m_SphereTest )
        g_CollisionMgr.SphereSetup( GetGuid(), GetPosition(), GetPosition(), m_Radius );
    else
        g_CollisionMgr.RaySetup( GetGuid(), Start, End );

    g_CollisionMgr.CheckCollisions( );

    if( g_CollisionMgr.m_nCollisions )
    {
        CollisionMat = g_CollisionMgr.m_Collisions[0].Flags;
        return GetMaterialName( CollisionMat );
    }
    else
    {
#ifdef sansari
        x_DebugMsg( "No collision reported for the footfall\n" );
        LOG_WARNING( "event_sound_emitter::GetMaterialType", "No collision reported" );
#endif
        return GetMaterialName( CollisionMat );
    }
}

//=========================================================================

const char* event_sound_emitter::GetMaterialTypeFromActor( guid Guid )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
    
    ASSERT( pObj->GetAttrBits() & object::ATTR_LIVING );

    actor& Actor = actor::GetSafeType( *pObj );

    return GetMaterialName( Actor.GetFloorMaterial() );
}

//=========================================================================

const char* event_sound_emitter::GetMaterialName( s32 MatType )
{
    switch( MatType )
    {        
        case MAT_TYPE_NULL:                 return "Null";                  break;
        case MAT_TYPE_EARTH:                return "Earth";                 break;
        case MAT_TYPE_ROCK:                 return "Rock";                  break;
        case MAT_TYPE_CONCRETE:             return "Concrete";              break;
        case MAT_TYPE_SOLID_METAL:          return "Metal";                 break;
        case MAT_TYPE_HOLLOW_METAL:         return "HollowMetal";           break;
        case MAT_TYPE_METAL_GRATE:          return "MetalGrate";            break;
        case MAT_TYPE_PLASTIC:              return "Plastic";               break;
        case MAT_TYPE_WATER:                return "Water";                 break;
        case MAT_TYPE_WOOD:                 return "Wood";                  break;
        case MAT_TYPE_ENERGY_FIELD:         return "EnergyField";           break;
        case MAT_TYPE_BULLET_PROOF_GLASS:   return "BulletProofGlass";      break;
        case MAT_TYPE_ICE:                  return "Ice";                   break;

        case MAT_TYPE_LEATHER:              return "Leather";               break;
        case MAT_TYPE_EXOSKELETON:          return "Exoskeleton";           break;
        case MAT_TYPE_FLESH:                return "Flesh";                 break;
        case MAT_TYPE_BLOB:                 return "Blob";                  break;
        
        case MAT_TYPE_FIRE:                 return "Fire";                  break;
        case MAT_TYPE_GHOST:                return "Ghost";                 break;
        case MAT_TYPE_FABRIC:               return "Fabric";                break;
        case MAT_TYPE_CERAMIC:              return "Ceramic";               break;
        case MAT_TYPE_WIRE_FENCE:           return "WireFence";             break;

        case MAT_TYPE_GLASS:                return "Glass";                 break;

        case MAT_TYPE_CARPET:               return "Carpet";                break;
        case MAT_TYPE_CLOTH:                return "Cloth";                 break;
        case MAT_TYPE_DRYWALL:              return "Drywall";               break;
        case MAT_TYPE_FLESHHEAD:            return "Flesh_head";            break;
        case MAT_TYPE_MARBLE:               return "Marble";                break;
        case MAT_TYPE_TILE:                 return "Tile";                  break;

        default:
                                            return "Null";
        break;
    }
}
