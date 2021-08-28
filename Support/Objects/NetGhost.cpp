//==============================================================================
//
//  NetGhost.cpp
//
//==============================================================================

#if defined(X_EDITOR)
#error NetGhost.cpp should not be included in the editor.
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetGhost.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Objects\PlayerLoco.hpp"
#include "Characters\Grunt\GruntLoco.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Objects\HudObject.hpp"
#include "GrenadeProjectile.hpp"
#include "GravChargeProjectile.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Objects\Event.hpp"  
#include "Objects\JumpPad.hpp"  
#include "Characters\ActorEffects.hpp"
#include "Sound\EventSoundEmitter.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

f32 net_ghost::m_BlindLogic = 0.0f;
s32 net_ghost::m_NInstances = 0;

//==============================================================================

static struct net_ghost_tweak
{
    // Data
    s32     m_FramesToBlendPos;     // Frames to blend position
    s32     m_FramesToBlendRot;     // Frames to blend rotation
    s32     m_FramesToBlendLean;    // Frames to blend lean
    #ifndef X_RETAIL
    xbool   m_bDebugRender;         // Shows debugging info
    #endif

    // Functions
    net_ghost_tweak()
    {
        m_FramesToBlendPos  = 4;
        m_FramesToBlendRot  = 3;
        m_FramesToBlendLean = 2;
        #ifndef X_RETAIL
        m_bDebugRender      = FALSE;
        #endif
    }
} g_NetGhost;

xbool s_bDegradeAim = FALSE;

//==============================================================================
//  TYPES
//==============================================================================

struct net_ghost_desc : public object_desc
{
    net_ghost_desc( void )
        :  object_desc( object::TYPE_NET_GHOST,              
                        "NetGhost",
                        "NETWORK",
                        object::ATTR_NEEDS_LOGIC_TIME          |
                        object::ATTR_COLLIDABLE                |
                        object::ATTR_BLOCKS_ALL_PROJECTILES    | 
                        object::ATTR_RENDERABLE                |
                        object::ATTR_SPACIAL_ENTRY             |
                        object::ATTR_DAMAGEABLE                |
                        object::ATTR_LIVING                    |
                        object::ATTR_TRANSPARENT               |
                        object::ATTR_CAST_SHADOWS,
                        FLAGS_IS_DYNAMIC )            
    {
        // Nothing to do in here.
    }

    virtual object* Create( void ) 
    { 
        return( new net_ghost );
    }

#ifdef X_EDITOR

    s32 OnEditorRender( object& Object ) const
    {
        (void)Object;
        return( -1 );
    }

#endif // X_EDITOR

};

//==============================================================================
//  STORAGE
//==============================================================================

static net_ghost_desc s_Ghost;

//==============================================================================
//  FUNCTIONS
//==============================================================================

net_ghost::locale::locale() :
    Position( 0.0f, 0.0f, 0.0f  ),
    Yaw     ( R_0               ),
    Pitch   ( R_0               ),
    Lean    ( 0.0f              )
{
}

//==============================================================================

net_ghost::net::net() :
    LocoType      ( -1    )
{
}

//==============================================================================

const object_desc& net_ghost::GetTypeDesc( void ) const
{
    return( s_Ghost );
}

//==============================================================================

const object_desc& net_ghost::GetObjectType( void )
{
    return( s_Ghost );
}

//==============================================================================

net_ghost::net_ghost( void )  
{
    // Start out DEAD.  Then spawn.
    m_Health.Dead();
    m_bDead        = TRUE;
    m_bWantToSpawn = TRUE;

#ifndef X_EDITOR
    m_Net.Frames[0]  = 4;
    m_Net.Frames[1]  = 4;
    m_Net.FrameDelay = 4;

    m_BlindLogic     = 0.0f;
    m_NInstances    += 1;

    actor::m_Net.LifeSeq = -1;  // Odd number means 'dead'.
    actor::m_NetType     = netobj::TYPE_PLAYER;
#endif
}

//==============================================================================

net_ghost::~net_ghost( void )
{
    m_BlindLogic  = 0.0f;
    m_NInstances -= 1;
}

//==============================================================================

void net_ghost::OnInit( void )
{
    // Call base class
    actor::OnInit();

    // Create default hazmat ghost
    m_pLoco = new player_loco();

    // TO DO - Only do this for PLAYERS.  Not for other NPCs.
    PrepPlayerAvatar();

    // Add to active list so NPCS attack
    SetIsActive( TRUE );
}

//==============================================================================

void net_ghost::OnKill( void )
{
    if( GameMgr.GameInProgress() )
    {
        CreateCorpse();
    }
    
    // Call base class
    actor::OnKill();

    SetIsActive( FALSE );
    // Free loco
    if( m_pLoco )
    {
        delete m_pLoco;
        m_pLoco = NULL;
    }
}

//==============================================================================
// This function sucks bad.

f32 TweakHeadPain = 1.0f;
f32 TweakBodyPain = 1.0f;
f32 TweakLimbPain = 1.0f;

void net_ghost::OnPain( const pain& Pain )
{
    f32 TweakFactor = 1.0f;

    // If you are already dead, no pain!
    if( m_bDead )
        return;

    // If the same pain event as the last one, ignore it.
    if( (Pain.GetAnimEventID() != -1) && 
        (Pain.GetAnimEventID() == m_LastAnimPainID) )
    {    
        return;
    }

    // if we are neutral, we ignore pain
    if ( m_SpawnNeutralTime > 0.0f )
    {
        return;
    }
    
    // If we have a "wire pull", we ignore the pain.
    if( (m_BlindLogic / m_NInstances) > 0.5f )
    {
        LOG_ERROR( "net_ghost::OnPain", 
                   "WIRE PULL PAIN IGNORED" );
        return;
    }

    xstring StringPrefix = (const char*)xfs( "%s", GetLogicalName() );

    // Modify damage on mutated players.
    if( m_bIsMutated )
    {
        StringPrefix += "_MUTANT";
    }

    // Decide which health id to use
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        switch( GetHitLocation( Pain ) )
        {   
        case geom::bone::HIT_LOCATION_HEAD:
            TweakFactor = TweakHeadPain;
            StringPrefix += "_H";
            break;
        case geom::bone::HIT_LOCATION_LEGS:
            TweakFactor = TweakLimbPain;
            StringPrefix += "_L";
            break;
        default: // includes TORSO, ARMS
            TweakFactor = TweakBodyPain;
            StringPrefix += "_B";
            break;
        };
    }
    else
#endif
    {
        StringPrefix += "_B";
    }

    // turn into string pointer
    const char* pString = (const char*)StringPrefix;

    // Decide which health id to use
    health_handle HealthHandle;
    HealthHandle.SetName( pString );

    // Resolve Pain
    if( !Pain.ComputeDamageAndForce( HealthHandle, GetGuid(), GetBBox().GetCenter() ) )
        return; 

    ((pain)Pain).SetCustomScalar( TweakFactor * Pain.GetCustomScalar() );

    /*
#ifdef DATA_VAULT_KEEP_NAMES
    LOG_MESSAGE( "net_ghost::OnPain",
        "NetGhost %d taking %f damage in %s from weapon %s.",
        m_NetSlot, Pain.GetDamage(), pString, Pain.GetPainHealthHandle().GetName() );
#else
    LOG_MESSAGE( "net_ghost::OnPain",
        "NetGhost %d taking %f damage in %s from weapon %d.",
        m_NetSlot, Pain.GetDamage(), pString, Pain.GetHitType() );
#endif
    */

    // Apply the damage
    TakeDamage( Pain );

    // Do flinches, blood, impact sounds etc
    DoMultiplayerPainEffects( Pain );
}

//==============================================================================

void net_ghost::OnDeath( void )
{    
    //LOG_MESSAGE("net_ghost::OnDeath","Crap, I'm dead.");
    // Call base class
    actor::OnDeath();
}

//==============================================================================

void net_ghost::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "net_ghost::OnAdvanceLogic" );

    //
    // Do player relative aiming here.
    //
    if( m_TargetNetSlot != -1 )
    {
        // Figure out the relative pitch and yaw here.
        object* pObject = NetObjMgr.GetObjFromSlot( m_TargetNetSlot );

        if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
        {
            actor&  TargetActor = actor::GetSafeType( *pObject );
            vector3 HeadPos;
            vector3 RootPos;
            GetHeadAndRootPosition( HeadPos, RootPos );

            radian  PitchOffset;
            radian  YawOffset;
            m_AimOffset.GetPitchYaw( PitchOffset, YawOffset );

            radian YawActor = TargetActor.GetYaw();

            // Add back in the relative pitch and yaw.
            vector3 TargetPos = TargetActor.GetPosition() + 
                (m_AimOffset.Length() * 
                vector3( PitchOffset, YawOffset + YawActor ));

            vector3 AimVector = TargetPos - HeadPos;

            m_Net.Actual.Pitch = AimVector.GetPitch();
            m_Net.Actual.Yaw   = AimVector.GetYaw();

            if( m_FireState > 0 )
            {
                m_Net.BlendYaw.Teleport  ( AimVector.GetYaw()   );
                m_Net.BlendPitch.Teleport( AimVector.GetPitch() );

                if( m_FireState == 1 )
                {
                    m_FireState = 0;
                }
            }
        }
    }

    // Make sure we cast shadows.
    SetAttrBits( GetAttrBits() | object::ATTR_CAST_SHADOWS );

    OnAdvanceGhostLogic( DeltaTime );

    // Call base class.
    actor::OnAdvanceLogic( DeltaTime );    

    // Send out animation events (firing etc)
    SendAnimEvents();

    // Wake up doors
    WakeUpDoors();

    // Wanting to end primary fire and a single bullet has been fired?
    if( ( m_bEndPrimaryFire ) && ( m_bPrimaryFired ) )
    {
        // Stop effects
        net_EndFirePrimary();
    }

    // Make sure looping primary effects are turned off if not playing shooting anim
    if( m_pLoco )
    {
        // Lookup shooting animation info
        loco_additive_controller& Cont = m_pLoco->GetAdditiveController( actor::ANIM_FLAG_SHOOT_CONTROLLER );
        s32 iShootAnim = m_pLoco->GetAnimIndex( loco::ANIM_SHOOT );

        // Not playing shoot animation?
        if( Cont.GetAnimTypeIndex() != iShootAnim )
        {
            // Stop effects
            new_weapon* pWeapon = GetCurrentWeaponPtr();
            if( pWeapon )
                pWeapon->EndPrimaryFire();
        }            
    }
        
#ifndef X_RETAIL
    // Show debug info?
    loco* pLoco = GetLocoPointer();
    if( (pLoco) && 
        (g_NetGhost.m_bDebugRender) )
    {
        draw_Label( GetPosition(), 
                    XCOLOR_WHITE,
                    "\n\n\n\n\n\n\n\nState:%s\nStyle:%s\nAnim:%s\n",
                    pLoco->GetStateName(),
                    loco::GetMoveStyleName( pLoco->GetMoveStyle() ),
                    pLoco->m_Player.GetCurrAnim().GetAnimName() );
    }
#endif
}

//==============================================================================

void net_ghost::OnRender( void )
{
    CONTEXT( "net_ghost::OnRender" );

    // Skip if dead body is present.
    if( m_bDead )
        return;

    ASSERTS( (m_SkinInst.GetVMeshMask() != 0), 
             "net_ghost has 0 skin VMeshMask, see M.Reed or D.M.Traub" );

    // Call base class render.
    actor::OnRender();

#if defined( aharp ) && !defined( CONFIG_RETAIL )
    new_weapon *pWeapon = GetCurrentWeaponPtr();

    if( pWeapon )
    {
        vector3 FirePos; 
        pWeapon->GetFiringBonePosition(FirePos);
        vector3 AimPos;
        pWeapon->GetAimBonePosition(AimPos);

        vector3 EyePos;
        vector3 Rubbish;
        GetHeadAndRootPosition( EyePos, Rubbish );

        // A line from the barrel with the correct pitch and yaw.
        {
            vector3 TargetPos = FirePos + vector3( GetPitch(), GetYaw() ) * 1000.0f; 
            draw_Line(FirePos, TargetPos, XCOLOR_BLUE);
        }

        // Where the bullets will get drawn, and where it looks like the gun is pointing.
        {        
            vector3 FireDirection = FirePos - AimPos;
            FireDirection.Normalize();
            FireDirection = FireDirection;

            vector3 TargetPos = FirePos + FireDirection * 1000.0f;
            draw_Line(FirePos, TargetPos, XCOLOR_RED);
        }

        // Approximately where the player is looking.
        {
            vector3 LookPos   = EyePos + vector3( GetPitch(), GetYaw() ) * 1000.0f;
            draw_Line(EyePos, LookPos, XCOLOR_YELLOW);
        }
    }
#endif    

#ifndef X_RETAIL
    // Render loco?
    if( g_NetGhost.m_bDebugRender )
        m_pLoco->RenderInfo( TRUE );
#endif
}

//==============================================================================

void net_ghost::OnRenderTransparent( void )
{
   actor::OnRenderTransparent(); // needed for the actor effects (mutation)

   // render weapon
   new_weapon* pWeapon = GetCurrentWeaponPtr();
   if( pWeapon )
   {
       pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );
       pWeapon->OnRenderTransparent();
   }
}

//==============================================================================

 void net_ghost::OnEvent( const event& Event )
 {
     //
     // Do player relative aiming here.
     //
     vector3 TargetPos;

     if( m_TargetNetSlot != -1 )
     {
         // Figure out the relative pitch and yaw here.
         object* pObject = NetObjMgr.GetObjFromSlot( m_TargetNetSlot );

         if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
         {
             actor&  TargetActor = actor::GetSafeType( *pObject );

             radian  PitchOffset;
             radian  YawOffset;
             m_AimOffset.GetPitchYaw( PitchOffset, YawOffset );

             radian YawActor = TargetActor.GetYaw();

             // Add back in the relative pitch and yaw.
             TargetPos = TargetActor.GetPosition() + 
                 (m_AimOffset.Length() * 
                 vector3( PitchOffset, YawOffset + YawActor ));
         }
     }

     // Is this a fire weapon event?
     if( Event.Type == event::EVENT_WEAPON )
     {
         // Get weapon event
         const weapon_event& WeaponEvent = weapon_event::GetSafeType( Event );

         // Lookup the fire point
         s32 iFirePoint = new_weapon::FIRE_POINT_DEFAULT;
         switch( WeaponEvent.WeaponState )
         {
         case new_weapon::EVENT_FIRE:
         case new_weapon::EVENT_ALT_FIRE:
             iFirePoint = new_weapon::FIRE_POINT_DEFAULT;
             break;

         case new_weapon::EVENT_FIRE_LEFT: 
         case new_weapon::EVENT_ALT_FIRE_LEFT: 
             iFirePoint = new_weapon::FIRE_POINT_LEFT;
             break;

         case new_weapon::EVENT_FIRE_RIGHT: 
         case new_weapon::EVENT_ALT_FIRE_RIGHT: 
             iFirePoint = new_weapon::FIRE_POINT_RIGHT;
             break;
         }

         // Fire the weapon?        
         new_weapon* pWeapon = GetCurrentWeaponPtr();
         if( pWeapon )
         {
             if( (m_TargetNetSlot == -1) &&
                 (m_CurrentWeaponItem != INVEN_WEAPON_SNIPER_RIFLE) )
             {
                 s_bDegradeAim = TRUE;
             }
             
             switch( WeaponEvent.WeaponState )
             {
             case new_weapon::EVENT_FIRE:
             case new_weapon::EVENT_FIRE_LEFT: 
             case new_weapon::EVENT_FIRE_RIGHT: 
                 pWeapon->FireGhostPrimary( iFirePoint, (m_TargetNetSlot != -1) ? TRUE : FALSE, TargetPos );
                 // Flag that primary has been fired so it can now be stopped 
                 m_bPrimaryFired = TRUE;
                 break;

             case new_weapon::EVENT_ALT_FIRE:
             case new_weapon::EVENT_ALT_FIRE_LEFT: 
             case new_weapon::EVENT_ALT_FIRE_RIGHT: 
                 pWeapon->FireGhostSecondary( iFirePoint );
                 break;
             }
         }
     }
     else if( Event.Type == event::EVENT_PAIN )
     {
         const pain_event& PainSuperEvent = pain_event::GetSafeType( Event );

         // check if this is a melee pain and kick it off
         if( ( PainSuperEvent.PainType == pain_event::EVENT_PAIN_MELEE ) )
         {
             EmitMeleePain();
         }
     }
}
 
 //==============================================================================

 void net_ghost::EmitMeleePain( void )
 {
    // Lookup tweaks
    f32 MeleeReachDistance = GetTweakF32( "PLAYER_MeleeReachDistance" );
    f32 MeleeSphereRadius  = GetTweakF32( "PLAYER_MeleeSphereRadius" );

    // Compute direction of melee
    vector3 Dir( 0.0f, 0.0f, 1.0f );
    Dir.RotateX( GetPitch() );
    Dir.RotateY( GetYaw() );

    // Compute start and end position of melee
    vector3 StartPos = GetPosition() + vector3( 0.0f, GetCollisionHeight() * 0.75f, 0.0f );
    vector3 EndPos   = StartPos + ( MeleeReachDistance * Dir );

    // Fire a sphere out from the eye to the correct distance and see if we hit anything.
    g_CollisionMgr.SphereSetup( GetGuid(), StartPos, EndPos, MeleeSphereRadius );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );
    if( !g_CollisionMgr.m_nCollisions )
        return;

    // Lookup collision info
    guid    DirectHitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
    vector3 HitPosition   = g_CollisionMgr.m_Collisions[0].Point;
    if( DirectHitGuid == 0 )
        return;

    // Create an event sound emitter.
    guid                 SndEventGuid = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
    event_sound_emitter* pSndEvent    = (event_sound_emitter*)g_ObjMgr.GetObjectByGuid( SndEventGuid );
    ASSERT( pSndEvent );
    
    // Play impact sound
    char DescName[64];
    x_sprintf( DescName, "Melee_%s", pSndEvent->GetMaterialName( g_CollisionMgr.m_Collisions[0].Flags ) );
    pSndEvent->PlayEmitter( DescName, 
                            HitPosition, 
                            GetZone1(), 
                            event_sound_emitter::SINGLE_SHOT, 
                            m_WeaponGuids[ inventory2::ItemToWeaponIndex( m_CurrentWeaponItem ) ] );

    // Build pain
    pain Pain;
    Pain.Setup( "PLAYER_MELEE_0", GetGuid(), HitPosition );
    Pain.SetDirection( Dir );
    Pain.SetDirectHitGuid( DirectHitGuid );
    Pain.SetCollisionInfo( g_CollisionMgr.m_Collisions[0] );
    Pain.ApplyToObject( DirectHitGuid );

    // Create melee impact FX
    particle_emitter::CreateProjectileCollisionEffect( g_CollisionMgr.m_Collisions[0], GetGuid() );
 }

//==============================================================================

void net_ghost::OnSpawn( void )
{           
    //LOG_MESSAGE("net_ghost::OnSpawn","Spawning.");
    // Call base class.
    actor::OnSpawn();

#ifndef X_EDITOR

    vector3 Position = GetPosition();
    m_Net.BlendPosX .Teleport( Position.GetX() );
    m_Net.BlendPosY .Teleport( Position.GetY() );
    m_Net.BlendPosZ .Teleport( Position.GetZ() );
    m_Net.BlendPitch.Teleport( GetPitch()      );
    m_Net.BlendYaw  .Teleport( GetYaw()        );

#endif
}  

//==============================================================================

actor::eHitType net_ghost::OverrideFlinchType( actor::eHitType hitType )
{
    // Ghost 3rd person avatar can only play light (additive) hits
    switch( hitType )
    {
    case HITTYPE_HARD:
    case HITTYPE_LIGHT:
    case HITTYPE_IDLE:
    case HITTYPE_PLAYER_MELEE_1:
        return HITTYPE_LIGHT;
        ASSERTS( 0, "Need to add new hit type here..." );
    default:
        return hitType;
    }
}

//==============================================================================
//
//  NETWORK FUNCTIONS
//
//==============================================================================

void net_ghost::net_Activate( void )
{
    // Call base class.
    actor::net_Activate();

    m_Net.BlendPosX .Init( 1.000f, 500.0f, g_NetGhost.m_FramesToBlendPos,  FALSE );
    m_Net.BlendPosY .Init( 1.000f, 500.0f, g_NetGhost.m_FramesToBlendPos,  FALSE );
    m_Net.BlendPosZ .Init( 1.000f, 500.0f, g_NetGhost.m_FramesToBlendPos,  FALSE );
    m_Net.BlendPitch.Init( 0.001f,     PI, g_NetGhost.m_FramesToBlendRot,  TRUE  );
    m_Net.BlendYaw  .Init( 0.001f,     PI, g_NetGhost.m_FramesToBlendRot,  TRUE  );
    m_Net.BlendLean .Init( 0.001f,   3.0f, g_NetGhost.m_FramesToBlendLean, FALSE );

    m_Net.DoTeleport = FALSE;
    m_Net.DoWayPoint = FALSE;

    LOG_MESSAGE( "net_ghost::net_Activate",
                 "Addr:%08X - NetSlot:%d - Status:%s on %s",
                 this, m_NetSlot,
                 (m_NetModeBits & CONTROL_LOCAL) ? "LOCAL"  : "REMOTE",
                 (m_NetModeBits & ON_SERVER    ) ? "SERVER" : "CLIENT" );
}

//==============================================================================

void net_ghost::net_AcceptUpdate( const update& Update )
{
    xbool DoTeleport = FALSE;

    m_BlindLogic = 0.0f;

    //
    // Update the average number of frames between updates.
    //

    s32 BlendFrames = MAX( m_Net.Frames[0], m_Net.Frames[1] );
    BlendFrames     = MAX( BlendFrames, m_Net.FrameDelay );

    m_Net.Frames[0]  = m_Net.Frames[1];
    m_Net.Frames[1]  = m_Net.FrameDelay;
    m_Net.FrameDelay = 0;

    m_Net.BlendPosX.SetBlendFrames( BlendFrames );
    m_Net.BlendPosY.SetBlendFrames( BlendFrames );
    m_Net.BlendPosZ.SetBlendFrames( BlendFrames );

    //
    // Let the ancestor (actor) class handle most of the update work.
    //

    actor::net_AcceptUpdate( Update );

    //
    // Take care of position/orientation updates.
    //

    if( Update.LifeSeq != actor::m_Net.LifeSeq )
    {
        return;
    }

    if( Update.DirtyBits & POSITION_BIT )
    {
        xbool   SetTarget = TRUE;
        vector3 Target;

        // Consider the data that has arrived.

        // Receive a teleport effect move?
        if( (Update.DirtyBits     & WAYPOINT_BIT) && 
            (Update.WayPointFlags & WAYPOINT_TELEPORT) &&
            (Update.WayPointFlags & WAYPOINT_TELEPORT_FX) )
        {
            m_Net.DoTeleport  = TRUE;
            m_Net.DoWayPoint  = TRUE;
            Target            = Update.WayPoint[0];
            m_Net.WayPoint[0] = Update.WayPoint[0];
            m_Net.WayPoint[1] = Update.WayPoint[1];
            /*
            LOG_MESSAGE( "net_ghost::net_AcceptUpdate[TELEPORT]",
                         "[%02d] From(%d,%d,%d) - To(%d,%d,%d)", 
                         m_NetSlot,
                         (s32)(Update.WayPoint[0].GetX()),
                         (s32)(Update.WayPoint[0].GetY()),
                         (s32)(Update.WayPoint[0].GetZ()),
                         (s32)(Update.WayPoint[1].GetX()),
                         (s32)(Update.WayPoint[1].GetY()),
                         (s32)(Update.WayPoint[1].GetZ()) );
            */
        }
        else
        // Receive an effect free teleport move?
        if(  (Update.DirtyBits     & WAYPOINT_BIT) && 
             (Update.WayPointFlags & WAYPOINT_TELEPORT) &&
            !(Update.WayPointFlags & WAYPOINT_TELEPORT_FX) )
        {
            Target     = Update.Position;
            DoTeleport = TRUE;
        }
        else
        // Receive a jump pad effect move?
        if( (Update.DirtyBits     & WAYPOINT_BIT) && 
            (Update.WayPointFlags & WAYPOINT_JUMP_PAD_FX) )
        {
            m_Net.DoTeleport  = FALSE;
            m_Net.DoWayPoint  = TRUE;
            Target            = Update.WayPoint[0];
            m_Net.WayPoint[0] = Update.WayPoint[0];
        }
        else
        // Receive an ordinary move?
        {
            Target    = Update.Position;
            SetTarget = !m_Net.DoWayPoint;
        }
        
        // Now execute.

        if( DoTeleport )
        {
            // If this ghost was targeting another player when he teleported,
            // he won't have absolute pitch/yaw information.  Just recycle his 
            // current pitch/yaw.
            if( Update.TargetNetSlot != -1 )
            {
                actor::Teleport( Target, GetPitch(), GetYaw() );
            }
            else
            {
                actor::Teleport( Target, Update.Pitch, Update.Yaw );
            }

            /*
            CLOG_MESSAGE( m_NetSlot == 1, "net_ghost::net_AcceptUpdate",
                          "TeleportBlendZ:%d", (s32)Target.GetZ() );
            */                            
            m_Net.BlendPosX.Teleport( Target.GetX() );
            m_Net.BlendPosY.Teleport( Target.GetY() );
            m_Net.BlendPosZ.Teleport( Target.GetZ() );
        }
        else
        {
            // Give loco movement hint.
            if( m_pLoco )
                m_pLoco->SetGhostIsMoving( m_Net.Actual.Position != Target );

            if( SetTarget )
            {
                /*
                CLOG_MESSAGE( m_NetSlot == 1, "net_ghost::net_AcceptUpdate",
                              "SetBlendTargetZ:%d", (s32)Target.GetZ() );
                */
                m_Net.BlendPosX.SetTarget( Target.GetX() );
                m_Net.BlendPosY.SetTarget( Target.GetY() );
                m_Net.BlendPosZ.SetTarget( Target.GetZ() );
            }
        }

        // Record last reported position.
        m_Net.Actual.Position = Update.Position;
        /*
        CLOG_MESSAGE( m_NetSlot == 1, "net_ghost::net_AcceptUpdate",
                      "UpdateActualZ:%d", (s32)Update.Position.GetZ() );
        */
    }

    if( Update.DirtyBits & ORIENTATION_BIT )
    {
        m_TargetNetSlot = Update.TargetNetSlot;
        m_AimOffset     = Update.AimOffset;

        if( m_TargetNetSlot == -1 )
        {
            // Looks like we got an absolute pitch and yaw.
            m_Net.Actual.Pitch = Update.Pitch;
            m_Net.Actual.Yaw   = Update.Yaw;

            if( m_FireState == 1 )
            {
                m_FireState = 0;
            }
        }

        if( DoTeleport )
        {
            m_Net.BlendPitch.Teleport( m_Net.Actual.Pitch );
            m_Net.BlendYaw  .Teleport( m_Net.Actual.Yaw   );
        }
        else
        {
            m_Net.BlendPitch.SetTarget( m_Net.Actual.Pitch );
            m_Net.BlendYaw  .SetTarget( m_Net.Actual.Yaw   );
        }
    }
    
    if( Update.DirtyBits & LEAN_BIT )
    {
        m_Net.Actual.Lean = Update.Lean;
        m_Net.BlendLean.SetTarget( m_Net.Actual.Lean );
    }
}

//==============================================================================

void net_ghost::net_ProvideUpdate( update& Update, u32& DirtyBits )
{
    actor::net_ProvideUpdate( Update, DirtyBits );

    if( Update.DirtyBits & POSITION_BIT )
    {
        if( m_Net.DoWayPoint )
            Update.Position = m_Net.Actual.Position;
    }
}

//==============================================================================

void net_ghost::net_Logic( f32 DeltaTime )
{   
    ASSERT( m_NetModeBits & CONTROL_REMOTE );   // Must be a ghost!

    m_BlindLogic += DeltaTime;
    m_Net.FrameDelay++;

    if( m_WayPointTimeOut < 15 )
        m_WayPointTimeOut++;
    else
        m_WayPointFlags = 0;

    if( IsAlive() )
    {
        // Update blending.
        m_Net.BlendPosX .BlendLogic();
        m_Net.BlendPosY .BlendLogic();
        m_Net.BlendPosZ .BlendLogic();
        m_Net.BlendPitch.BlendLogic();
        m_Net.BlendYaw  .BlendLogic();
        m_Net.BlendLean .BlendLogic();
        
        // Way point logic.

        vector3 Offset;
        f32     Distance = 0.0f;

        if( m_Net.DoWayPoint )
        {
            Offset   = m_Net.Render.Position - m_Net.WayPoint[0];
            Distance = Offset.Length();
            /*
            LOG_MESSAGE( "net_ghost::net_Logic[TELEPORT]", 
                         "[%02d] Current(%d,%d,%d) - Distance:%d",
                         m_NetSlot,
                         (s32)(m_Net.Render.Position.GetX()),
                         (s32)(m_Net.Render.Position.GetY()),
                         (s32)(m_Net.Render.Position.GetZ()),
                         (s32)(Distance) );
            */
        }

        if( m_Net.DoWayPoint && (Distance < 10.0f) )
        {
            if( m_Net.DoTeleport )
            {
                /*
                LOG_MESSAGE( "net_ghost::net_Logic[TELEPORT]",
                             "[%02d] Target(%d,%d,%d)",
                             m_NetSlot,
                             (s32)(m_Net.WayPoint[1].GetX()),
                             (s32)(m_Net.WayPoint[1].GetY()),
                             (s32)(m_Net.WayPoint[1].GetZ()) );
                */

                m_Net.BlendPosX.Teleport( m_Net.WayPoint[1].GetX() );
                m_Net.BlendPosY.Teleport( m_Net.WayPoint[1].GetY() );
                m_Net.BlendPosZ.Teleport( m_Net.WayPoint[1].GetZ() );

                // If this ghost was targeting another player when he teleported,
                // he won't have absolute pitch/yaw information.  Just recycle 
                // his current pitch/yaw.
                if( m_TargetNetSlot != -1 )
                {
                    actor::Teleport( m_Net.WayPoint[1], 
                                     GetPitch(), GetYaw(), 
                                     FALSE, TRUE );
                }
                else
                {
                    actor::Teleport( m_Net.WayPoint[1], 
                                     m_Net.Actual.Pitch, m_Net.Actual.Yaw, 
                                     FALSE, TRUE );
                }

                m_NetDirtyBits &= ~WAYPOINT_BIT;
            }
            else
            {
                slot_id Slot = g_ObjMgr.GetFirst( TYPE_JUMP_PAD );
                while( Slot != SLOT_NULL )
                {
                    object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
                    ASSERT( pObject );
                    ASSERT( pObject->GetType() == TYPE_JUMP_PAD );
                    vector3 Gap = pObject->GetPosition() - GetPosition();
                    if( Gap.LengthSquared() < 250.0f )
                    {
                        jump_pad* pJumpPad = (jump_pad*)pObject;
                        pJumpPad->PlayJump();
                        break;
                    }
                    Slot = g_ObjMgr.GetNext( Slot );
                }   
            }
            
            /*
            CLOG_MESSAGE( m_NetSlot == 1, "net_ghost::net_Logic",
                          "SetBlendTargetZ:%d", (s32)m_Net.Actual.Position.GetZ() );
            */
            m_Net.BlendPosX.SetTarget( m_Net.Actual.Position.GetX() );
            m_Net.BlendPosY.SetTarget( m_Net.Actual.Position.GetY() );
            m_Net.BlendPosZ.SetTarget( m_Net.Actual.Position.GetZ() );

            if( m_TargetNetSlot != -1 )
            {
                m_Net.BlendPitch.SetTarget( m_Net.Actual.Pitch );
                m_Net.BlendYaw  .SetTarget( m_Net.Actual.Yaw   );
            }

            m_Net.DoTeleport = FALSE;
            m_Net.DoWayPoint = FALSE;
        }

        // SB: Force render lean to actual lean if it's really close,
        //     otherwise it never reaches zero and causes players to skate.
        if( x_abs( m_Net.Actual.Lean - m_Net.Render.Lean ) < 0.0001f )
            m_Net.Render.Lean = m_Net.Actual.Lean;

        m_Net.Render.Position.Set( m_Net.BlendPosX.GetValue(),
                                   m_Net.BlendPosY.GetValue(),
                                   m_Net.BlendPosZ.GetValue() );
        m_Net.Render.Pitch = m_Net.BlendPitch.GetValue();
        m_Net.Render.Yaw   = m_Net.BlendYaw  .GetValue();
        m_Net.Render.Lean  = m_Net.BlendLean .GetValue();

        // Update ghost.
        Teleport      ( m_Net.Render.Position );
        SetPitch      ( m_Net.Render.Pitch    );
        SetYaw        ( m_Net.Render.Yaw      );
        SetLeanAmount ( m_Net.Render.Lean     );
        /*
        CLOG_MESSAGE( (m_NetSlot == 1), "net_ghost::net_Logic", 
                      "RenderZ:%d - ActualZ:%d - ActorZ:%d - BlendZ:%d", 
                      (s32)m_Net.Render.Position.GetZ(),
                      (s32)m_Net.Actual.Position.GetZ(),
                      (s32)GetPosition().GetZ(),
                      (s32)m_Net.BlendPosZ.GetValue() );
        */
    }

    // Update any effects to be at current position.
    // NOTE: This function call happens after the object::OnAdvance.
    if( m_pEffects )
        m_pEffects->Update( this, DeltaTime );

    /*
    if( m_NetModeBits == GHOST_ON_SERVER )
    {
        LOG_MESSAGE( "net_ghost::net_Logic",
                     "Slot:%d - Mode:GHOST ON SERVER - DirtyBits:%08X", 
                     m_NetSlot, m_NetDirtyBits );
    }
    else
    {
    LOG_MESSAGE( "net_ghost::net_Logic",
                 "Slot:%d - Mode:GHOST ON CLIENT", m_NetSlot );
    }
    */
}

//==============================================================================

xbool net_ghost::net_EquipWeapon2( inven_item WeaponItem )
{
    // Make sure loco anims are updated
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        pLoco->SetWeapon( WeaponItem );
    }

    // Update?
    if( WeaponItem == m_CurrentWeaponItem )
        return( FALSE );

    // Delete current weapon objects
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        // Dispose of the current weapon.
        guid& Guid = m_WeaponGuids[ m_CurrentWeaponItem ];
        object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
        if( pObject )
        {
            g_ObjMgr.DestroyObject( Guid );
        }                                                                                                                                         
        Guid = NULL_GUID;
    }
    
    // Default to NULL incase not found or clearing weapon
    m_CurrentWeaponItem = INVEN_NULL;

    // Create the desired weapon.
    {
        const char* pBlueprintName = inventory2::ItemToBlueprintName( WeaponItem );

        if( pBlueprintName )
        {
            guid WeaponGUID = g_TemplateMgr.CreateSingleTemplate( pBlueprintName, 
                                                                  vector3(0,0,0), 
                                                                  radian3(0,0,0), 
                                                                  0, 0 );

            if( WeaponGUID )
            {
                ASSERT( m_WeaponGuids[WeaponItem] == NULL_GUID );

                new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( WeaponGUID );
                ASSERT( pWeapon );

                pWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_NPC, GetGuid() );
                pWeapon->BeginIdle();
                pWeapon->SetVisible( TRUE );

                m_CurrentWeaponItem       = WeaponItem;
                m_WeaponGuids[WeaponItem] = WeaponGUID;

                // Position ready for rendering.
                MoveWeapon( TRUE );

                // I thought that this would have happened somewhere else, 
                // but it seems not.  So...
                #ifndef X_EDITOR
                m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
                #endif // X_EDITOR 
            }
        }
    }

    return( TRUE );
}

//==============================================================================
