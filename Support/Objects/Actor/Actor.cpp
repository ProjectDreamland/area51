//==============================================================================
//
//  Actor.cpp
//
//==============================================================================

#if defined(bwatson)
#define X_SUPPRESS_LOGS
#endif

#include "EventMgr\EventMgr.hpp"
#include "Actor.hpp"
#include "Objects\Corpse.hpp"
#include "Characters\Character.hpp"
#include "Characters\GenericNPC\GenericNPC.hpp"
#include "GameLib\RenderContext.hpp"
#include "Objects\NewWeapon.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Debris\debris_rigid.hpp"
#include "Objects\door.hpp"
#include "Decals\DecalMgr.hpp"
#include "Objects\object.hpp"
#include "Objects\CokeCan.hpp"
#include "Objects\Player.hpp"
#include "objects\turret.hpp"
#include "Characters\ActorEffects.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Render\LightMgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Objects\ForceField.hpp"
#include "Objects\Teleporter.hpp"
#include "StateMgr\MapList.hpp"


#ifndef X_EDITOR
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "Objects\NetGhost.hpp"
#include "NetworkMgr\MsgMgr.hpp"
#endif

//=========================================================================
// DEFINES
//=========================================================================

#define ACTOR_DATA_VERSION      1000

//=========================================================================
// CONSTANTS
//=========================================================================

const f32 k_CloakTransitionTime         = 1.0f;
const f32 k_MinTimeBetweenBigStaggers   = 0.25f;
const f32 k_MinTimeBetweenSmallStaggers = 0.75f;
const f32 k_MinTimeBetweenBigHits       = 2.0f;
const f32 k_TimeBetweenContagionTicks   = 1.0f;
const f32 k_ContagionSpreadDistanceSqr  = 1000.0f * 1000.0f;
      f32 g_SpawnFadeTime               = 1.5f;

static const f32 DISPLACE_PARTICLE_EXTENT = 20.0f;

//=========================================================================
// EXTERNALS
//=========================================================================

extern s32 g_Difficulty;
extern const char* DifficultyText[];

extern xbool   g_bBloodEnabled;
extern xbool   g_RenderBoneBBoxes;

//=========================================================================
// TWEAKS
//=========================================================================

static tweak_handle s_TWEAK_ContagionDuration          ("CONTAGION_Duration");       
static tweak_handle s_MPContagion_TouchDistance        ("ContagionRadius");
static tweak_handle s_AlamoTurretDamagePct             ("PLAYER_AlamoTurretDamagePct");
static tweak_handle s_ExcavationTurretDamagePct        ("PLAYER_ExcavationTurretDamagePct");
       
struct avatar_tweaks
{
    avatar_tweaks( void );
    f32 m_NormalingTime;
    f32 m_MutatingTime;
};

avatar_tweaks::avatar_tweaks( void ) :
m_NormalingTime     ( 1.0f ),
m_MutatingTime      ( 1.0f )
{

}

avatar_tweaks g_AvatarTweaks;

//=========================================================================
// DATA
//=========================================================================

actor* actor::m_pFirstActive = NULL;
s32    actor::m_nActive      = 0;


//=========================================================================
// Actor class
//=========================================================================

actor::actor( void ) :
    m_pNextActive                   ( NULL                       ),
    m_pPrevActive                   ( NULL                       ),
    m_bIsActive                     ( FALSE                      ),
    m_Faction                       ( FACTION_NONE               ),
    m_FriendFlags                   ( 0                          ),
    m_MaxHealth                     ( 100.0f                     ),
    m_bDead                         ( FALSE                      ),
    m_bCanDie                       ( TRUE                       ),
    m_DeathType                     ( DEATH_BY_ANIM              ),
    m_CorpseGuid                    ( 0                          ),
    m_bWantToSpawn                  ( FALSE                      ),
    m_SpawnFadeTime                 ( 0.0f                       ),
    m_SpawnNeutralTime              ( 0.0f                       ),
    m_WeaponsCreated                ( FALSE                      ),
    m_CurrentWeaponItem             ( INVEN_NULL                 ),
    m_LastAnimPainID                ( 0                          ),
    m_LastMeleeEventID              ( 0                          ),
    m_CurrentPainEventID            ( 0                          ),
    m_TimeSinceLastPain             ( 0.0f                       ),
    m_SafeFallAltitude              ( 800.0f                     ),
    m_DeathFallAltitude             ( 1400.0f                    ),
    m_FellFromAltitude              ( -(F32_MAX/2.0f)            ), /* a REALLY negative number */
    m_VoiceID                       ( 0                          ),
    m_PreferedVoiceActor            ( 0                          ),
    m_PainSfxInterval               ( 0.0f                       ),
    m_PainSfxDelay                  ( 0.0f                       ),
    m_CloakVoiceID                  ( 0                          ),
    m_CloakShieldPainTimer          ( 0.0f                       ),
    m_CloakState                    ( CLOAKING_OFF               ),
    m_CollidedActor                 ( 0                          ),
    m_TimeActorColliding            ( 0.0f                       ),
    m_TimeObjectColliding           ( 0.0f                       ),
    m_BloodDecalGroup               ( 0                          ),
    m_LocoAllocated                 ( FALSE                      ),
    m_pLoco                         ( NULL                       ),
    m_bIgnoreLocoInTransform        ( FALSE                      ),
    m_LastTimeStaggered             ( 0.0f                       ),
    m_TimeSinceLastRender           ( 10.0f                      ),
    m_LeanAmount                    ( 0.0f                       ),
    m_LeanState                     ( LEAN_NONE                  ),
    m_FriendlyGlowColor             (  50, 255, 0, 255           ),
    m_EnemyGlowColor                ( 255,  50, 0, 255           ),
    m_PulseGlowDelta                ( 0.0f                       ),
    m_CurrentGlowPulse              ( 1.0f                       ),
    m_bAllowedToGlow                ( TRUE                       ),
    m_ContagionTimer                ( 0.0f                       ),
    m_ContagionDOTTimer             ( 0.0f                       ),
    m_CanCloak                      ( FALSE                      ),
    m_MustCloak                     ( FALSE                      ),
    m_bIsCrouching                  ( FALSE                      ),
    m_bIsAirborn                    ( FALSE                      ),
    m_bIsMutated                    ( FALSE                      ),
    m_bCanToggleMutation            ( TRUE                       ),
    m_bDumbAndFast                  ( FALSE                      ),
    m_bPrimaryFired                 ( FALSE                      ),
    m_bEndPrimaryFire               ( FALSE                      ),
    m_bContagious                   ( FALSE                      ),
    m_FireState                     ( 0                          ),
    m_VoteCanStartKick              ( 0                          ),
    m_VoteAction                    ( 0                          ),
    m_VoteArgument                  ( 0                          ),
    m_Pitch                         ( R_0                        ),
    m_Yaw                           ( R_0                        ),
    m_MutagenBurnMode               ( MBM_NORMAL_CAMPAIGN        ),
    m_TargetNetSlot                 ( -1                         ),
    m_AimOffset                     ( vector3( 0.0f, 0.0f, 0.0f) ),
    m_bLockedDoors                  ( FALSE                      )
{
    // Initialize the inventory data
    m_Inventory2.Init();

    // Clear the weapon guids
    x_memset( &m_WeaponGuids, 0, sizeof(m_WeaponGuids) );

    // Not inactive.
    ClearInactiveTime();

    m_FloorProperties.Init( 100.0f, 0.5f );

    m_BigPainTakenTime = (f32)x_GetTimeSec();
    m_pEffects = NULL;    

    SetAvatarMutationState( AVATAR_NORMAL );

#ifndef X_EDITOR
    m_NetType      = netobj::TYPE_PLAYER;
    m_pMPContagion = NULL;
#endif
}

//=========================================================================

actor::~actor( void )
{
    // Make sure we are removed from the static actor list
    SetIsActive( FALSE );

    if( m_pEffects )
    {
        delete m_pEffects;
        m_pEffects = NULL;
    }

    if( m_LocoAllocated )
    {
        delete m_pLoco;
        m_pLoco = NULL;
    }

    s32 c;
    for(c=0;c<INVEN_NUM_WEAPONS;c++)
    {
        object *weaponObject = g_ObjMgr.GetObjectByGuid(m_WeaponGuids[c]);
        if( weaponObject )
        {
            g_ObjMgr.DestroyObject( m_WeaponGuids[c] );
        }
    }
}

//=========================================================================

void actor::OnInit( void )
{
}

//=========================================================================

void actor::OnKill( void )
{
    netobj::OnKill();
    if( m_CloakVoiceID )
    {
        g_AudioMgr.Release(m_CloakVoiceID,0.0f);
        m_CloakVoiceID = 0;
    }

#ifndef X_EDITOR
    if( m_pMPContagion )
    {
        for( s32 i = 0; i < 32; i++ )
        {
            // Release contagion effect handles.
            ASSERT( m_pMPContagion->Arc[i].Validate() );
            m_pMPContagion->Arc[i].KillInstance();
        }
        delete m_pMPContagion;
        m_pMPContagion = NULL;
    }
#endif
}

//=========================================================================

f32 actor::GetActorCollisionRadius()
{
    return GetLocoPointer()->GetActorCollisionRadius();    
}

//=========================================================================

f32 actor::GetFloorIntensity()
{
    xcolor Color = GetFloorColor();
    vector3 Vector( Color.R, Color.G, Color.B );
    return Vector.Length();
}

//=========================================================================

radian actor::GetSightYaw( void ) const
{
    if( m_pLoco )
        return m_pLoco->GetSightYaw();
    else
        return R_0;
}

//=========================================================================

vector3 actor::GetPositionWithOffset( eOffsetPos offset )
{
    // SB: Fix meson debris crash in multi-player
    loco* pLoco = GetLocoPointer();
    if( pLoco == NULL )
    {
        // Just use npc/player center
        return GetBBox().GetCenter();
    }

    switch( offset )
    {
    case OFFSET_NONE:
        {
            return GetPosition();
        }
    case OFFSET_CENTER:
        {
            vector3 halfEyeOffset = pLoco->GetEyeOffset();
            halfEyeOffset.Scale(0.5f);
            return GetPosition() + halfEyeOffset;
        }
    case OFFSET_AIM_AT:
    case OFFSET_EYES:
        {
            return GetPosition() + pLoco->GetEyeOffset();
        }
    case OFFSET_TOP_OF_BBOX:
        {
            return GetPosition() + vector3( 0.0f, pLoco->m_Physics.GetBBox().Max.GetY(), 0.0f );
        }
    default:
        {                
            return GetPosition();
        }
    }
}

//=========================================================================

f32 actor::GetMovementNoiseLevel()
{
    ASSERT( GetLocoPointer() );

    if( IsMoving() )
    {
        switch( GetLocoPointer()->GetMoveStyle() )
        {
        case loco::MOVE_STYLE_PROWL:
            return 2.0f;
            break;
        case loco::MOVE_STYLE_WALK:
            return 4.0f;
            break;
        case loco::MOVE_STYLE_RUN:
            return 10.0f;
            break;
        }
    }
    return 0.0f;
}

//=============================================================================

void actor::GetHeadAndRootPosition( vector3& HeadPos, vector3& RootPos )
{
    loco* pLoco = GetLocoPointer();
    if( !pLoco )
    {
        RootPos = GetPosition();
        HeadPos = RootPos + vector3( 0, 200.0f, 0.0f ) ;
    }
    else
    {
        RootPos = pLoco->m_Player.GetBonePosition( 0 );
        HeadPos = pLoco->GetEyePosition();  // Center of eyes is a good spot to aim for
    }
}

//=============================================================================

void actor::ReloadAllWeapons( void )
{
    for( s32 i=0; i < INVEN_WEAPON_LAST; i++ )
    {
        new_weapon* pWeapon = GetWeaponPtr( inventory2::WeaponIndexToItem( i ) );

        if( pWeapon )
        {
            pWeapon->Reload(new_weapon::AMMO_PRIMARY);
            pWeapon->Reload(new_weapon::AMMO_SECONDARY);
        }
    }
}

//=============================================================================

xbool actor::AddItemToInventory2( inven_item Item )
{
    // if we're dead, don't add this to our inventory.
    if( IsDead() )
    {
        return FALSE;
    }

    if( m_Inventory2.CanHoldMore( Item ) )
    {
        m_Inventory2.AddAmount( Item, 1.0f );
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

xbool actor::RemoveItemFromInventory2( inven_item Item, xbool bRemoveAll )
{
    if( m_Inventory2.HasItem( Item ) )
    {
        f32 NbrToRemove = 1.0f;
        if( bRemoveAll )
        {
            NbrToRemove = m_Inventory2.GetAmount( Item );
        }

        m_Inventory2.RemoveAmount( Item, NbrToRemove );
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void actor::AddAmmoToInventory2( inven_item Item, s32 Amount )
{
    m_Inventory2.AddAmount( Item, (f32)Amount );

    if( IN_RANGE( INVEN_AMMO_FIRST, Item, INVEN_AMMO_LAST) )
    {
        new_weapon* pWeapon = GetWeaponPtr( inventory2::WeaponIndexToItem( m_Inventory2.AmmoToWeapon( Item ) ) );
        if( pWeapon )
        {
            pWeapon->AddAmmoToWeapon( Amount, 0 );
        }
    }
}

//=========================================================================

void actor::ClearInventory2( void )
{
    m_Inventory2.Clear();

    for( s32 i = 0; i < INVEN_NUM_WEAPONS; i++ )
    {
        new_weapon* pWeapon = GetWeaponPtr( inventory2::WeaponIndexToItem(i) );
        if( pWeapon )
        {
            pWeapon->ClearAmmo();
        }
    }
}

//=========================================================================

void actor::OnEnumProp( prop_enum&  List )
{
    object::OnEnumProp(List);
    List.PropEnumHeader( "Actor", "Actor Info.", 0 );

    // Locomotion
    if (m_pLoco)
        m_pLoco->OnEnumProp(List);

    // Fall damage
    List.PropEnumHeader( "FallDamage", "Damage taken from falling a given altitude.", 0 );
    List.PropEnumFloat( "FallDamage\\SafeAltitude", "Falling from this altitude or less is safe -- no damage.", PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat( "FallDamage\\DeathAltitude", "Falling from this altitude or more is guaranteed fatal.", PROP_TYPE_MUST_ENUM );

    // Decals
    List.PropEnumHeader  ( "BloodDecals",          "Which blood decals this actor will leave.", 0 );
    List.PropEnumExternal( "BloodDecals\\Package", "Decal Resource\0decalpkg\0", "Which blood decal package this actor uses.", 0 );
    List.PropEnumInt     ( "BloodDecals\\Group",   "Within the decal package, which group of bloud this actor uses.", 0 );

    // mutation glow effect
    List.PropEnumHeader  ( "MutantVision",                "Properties for the mutation vision effect.", 0 );
    List.PropEnumBool    ( "MutantVision\\Allowed",       "Allow this guy to glow?", 0 );
    List.PropEnumColor   ( "MutantVision\\FriendlyColor", "Ambient when you are a friendly in mutant vision. Alpha determines how much glow.", 0 );
    List.PropEnumColor   ( "MutantVision\\EnemyColor",    "Ambient when you are an enemy in mutant vision. Alpha determines how much glow.", 0 );

    List.PropEnumHeader  ( "Effects",              "Additional effects added to actors.", 0 );
    //List.PropEnumBool    ( "Effects\\UseEffects",  "Turn on effects.",  PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE);
    List.PropEnumBool    ( "Effects\\FlamesOn",    "Is Actor flaming?", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );
    List.PropEnumBool    ( "Effects\\FlashlightOn",  "Flashlight effect information", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );
    List.PropEnumFloat   ( "Effects\\Lifespan",    "How long can actor remain alive?", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );

}

//=========================================================================

xbool actor::OnProperty( prop_query& rPropQuery )
{
    // Call base class
    if ( object::OnProperty( rPropQuery ) )
    {
        // Initialize zone tracker?
        if( rPropQuery.IsVar( "Base\\Position" ) )
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }
        
        // Initialize locomotion?
        if( rPropQuery.IsVar( "Base\\Rotation" ) || rPropQuery.IsVar( "Base\\Position" ))
        {
            // Init the loco!
            if (GetLocoPointer())
            {
                // Get transform info
                const matrix4& L2W = GetL2W();
                vector3 Pos = L2W.GetTranslation();
                radian  Yaw = L2W.GetRotation().Yaw;

                // Update loco
                GetLocoPointer()->SetPosition(Pos);
                GetLocoPointer()->SetYaw(Yaw + R_180); // SB +R_180 is legacy for old anim system
            }
        }

        return TRUE;
    }

    // Skin?
    skin_inst& SkinInst = GetSkinInst();
    if( SkinInst.OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    // Animation?
    if( rPropQuery.IsVar( "RenderInst\\Anim" ) )
    {
        if( rPropQuery.IsRead() )
            rPropQuery.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
        else
        {
            // Anim changed?
            if( rPropQuery.GetVarExternal()[0] )
            {
                m_hAnimGroup.SetName( rPropQuery.GetVarExternal() );
                if( m_hAnimGroup.GetPointer() )
                {
                    // Tell loco
                    if( m_pLoco )
                        m_pLoco->OnInit( SkinInst.GetGeom(), m_hAnimGroup.GetName(), GetGuid() );
                    
                    InitLoco();
                }
            }
        }
        return TRUE;
    }

    // Loco?
    if(( m_pLoco ) && ( m_pLoco->OnProperty( rPropQuery ) ) )
        return TRUE;

    // Fall damage
    if ( rPropQuery.VarFloat( "FallDamage\\SafeAltitude", m_SafeFallAltitude, 0, F32_MAX ) )
    {
        if ( !rPropQuery.IsRead() && (m_SafeFallAltitude > m_DeathFallAltitude) )
        {
            m_DeathFallAltitude = m_SafeFallAltitude + 1.0f;
        }
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "FallDamage\\DeathAltitude", m_DeathFallAltitude, 1.0f, F32_MAX ) )
    {
        if ( !rPropQuery.IsRead() && (m_DeathFallAltitude < m_SafeFallAltitude) )
        {
            m_SafeFallAltitude = m_DeathFallAltitude - 1.0f;
        }
        return TRUE;
    }

    // Decals
    if ( rPropQuery.IsVar( "BloodDecals\\Package" ) )
    {
        if ( rPropQuery.IsRead() )
            rPropQuery.SetVarExternal( m_hBloodDecalPackage.GetName(), RESOURCE_NAME_SIZE );
        else
            m_hBloodDecalPackage.SetName( rPropQuery.GetVarExternal() );

        return TRUE;
    }

    if ( rPropQuery.VarInt( "BloodDecals\\Group", m_BloodDecalGroup ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarBool( "MutantVision\\Allowed", m_bAllowedToGlow ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarColor( "MutantVision\\FriendlyColor", m_FriendlyGlowColor ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarColor( "MutantVision\\EnemyColor", m_EnemyGlowColor ) )
    {
        return TRUE;
    }

    // Handle additional effects
    if( rPropQuery.IsBasePath("Effects") )
    {
        /*
        if( rPropQuery.IsVar( "Effects\\UseEffects" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarBool( (m_pEffects) ? (TRUE):(FALSE) );
            }
            else
            {
                if( rPropQuery.GetVarBool() )
                    GetActorEffects(TRUE);
                else
                {
                    if( m_pEffects )
                    {
                        delete m_pEffects;
                        m_pEffects = NULL;
                    }
                }
            }

            return TRUE;
        }
        */
        if( rPropQuery.IsVar( "Effects\\FlamesOn" ) )
        {
            if( rPropQuery.IsRead() )
            {
                if( GetActorEffects() )
                    rPropQuery.SetVarBool( GetActorEffects()->IsEffectOn( actor_effects::FX_FLAME ) );
                else
                    rPropQuery.SetVarBool( FALSE );
            }
            else
            {
                if( rPropQuery.GetVarBool() )
                    GetActorEffects(TRUE)->InitEffect( actor_effects::FX_FLAME, this );
                else
                {
                    if( GetActorEffects() )
                        GetActorEffects()->KillEffect( actor_effects::FX_FLAME );
                }
            }

            return TRUE;
        }


        if( rPropQuery.IsVar( "Effects\\FlashlightOn" ) )
        {
            if( rPropQuery.IsRead() )
            {
                if( GetActorEffects() )
                    rPropQuery.SetVarBool( GetActorEffects()->IsEffectOn( actor_effects::FX_FLASHLIGHT ) );
                else
                    rPropQuery.SetVarBool( FALSE );
            }
            else
            {
                if( rPropQuery.GetVarBool() )
                    GetActorEffects(TRUE)->InitEffect( actor_effects::FX_FLASHLIGHT, this );
                else
                {
                    if( GetActorEffects() )
                        GetActorEffects()->KillEffect( actor_effects::FX_FLASHLIGHT );
                }
            }

            return TRUE;
        }

        if( rPropQuery.IsVar( "Effects\\Lifespan" ) )
        {
            if( rPropQuery.IsRead() )
            {
                if( GetActorEffects() )
                    rPropQuery.SetVarFloat( GetActorEffects()->GetDeathTimer() );
                else
                    rPropQuery.SetVarFloat( 100000.0f );
            }
            else
            {
                if( GetActorEffects() )
                    GetActorEffects()->SetDeathTimer( rPropQuery.GetVarFloat() );
            }

            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

#ifdef X_EDITOR
s32 actor::OnValidateProperties( xstring& ErrorMsg )
{
    s32 nErrors = 0;
    
    // Check loco properties?
    if( m_pLoco )
        nErrors += m_pLoco->OnValidateProperties( m_SkinInst, ErrorMsg );
    
    return nErrors;
}
#endif

//=========================================================================

actor_effects* actor::GetActorEffects( xbool bCreate )
{
    if( !m_pEffects && bCreate )
    {
        // create the actor effects
        m_pEffects = new actor_effects;
        m_pEffects->Init();
    }

    return m_pEffects;
}

//=========================================================================

#ifdef ENABLE_DEBUG_MENU
static xbool s_DumpInventory = 0;
#endif

void actor::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "actor::OnAdvanceLogic" );

    // update the skin instance so actors can fade based on a super-event
    m_SkinInst.OnAdvanceLogic( DeltaTime );

    m_TimeSinceLastPain = x_min( m_TimeSinceLastPain + DeltaTime, 1.0f );

    // Update pain sound effects.
    UpdatePainSfx( DeltaTime );
    
    // Safe to clear the actor effects?
    if( m_pEffects )
    {
        if( !m_pEffects->IsActive() )
        {
            delete m_pEffects;
            m_pEffects = NULL;
        }
    }

    // Update our spawn timers
    m_SpawnFadeTime    = MAX( 0.0f, m_SpawnFadeTime    - DeltaTime );
    m_SpawnNeutralTime = MAX( 0.0f, m_SpawnNeutralTime - DeltaTime );

    // Special case logic
    if( !m_bDead )
    {
        OnAliveLogic( DeltaTime );
    }
    else
    {
        OnDeathLogic( DeltaTime );
    }

    // contagion logic
    ContagionLogic( DeltaTime );

    // Track inactivity.
    UpdateInactive( DeltaTime );

    // Update avatar mutation
    UpdateAvatarMutation( DeltaTime );

    // Update staggering
    if( IsStaggering() )
    {
        m_LastTimeStaggered -= DeltaTime;
    }

    // Update rendering
    m_TimeSinceLastRender += DeltaTime;

    // Debug to dump inventory
#ifdef ENABLE_DEBUG_MENU
    if( s_DumpInventory )
    {
        m_Inventory2.LogInventory();
        s_DumpInventory = 0;
    }
#endif

    //
    // If actor is below some threshold then shut off his logic
    //
    if( !IsPlayer() && !IsNetGhost() && !g_ObjMgr.GetSafeBBox().Intersect(GetPosition()) )
    {
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    // get floor information for ambient lighting and footfall sounds
    m_FloorProperties.Update( GetPosition(), DeltaTime );

    UpdateFellFromAltitude();

    // Handled by player and characters
    //WakeUpDoors();

    // Update our current weapon
    UpdateWeapon( DeltaTime );
} 

//=============================================================================
// This function should only be used when driving a ghost version of an actor.

void actor::OnAdvanceGhostLogic( f32 DeltaTime )
{
    // Update animation rate and move style to try match movement?
    loco* pLoco = GetLocoPointer();
    if( !pLoco )
        return;
    
    // Loco is controlled by ghost, not animations.
    pLoco->SetGhostMode( TRUE );

    // If this is a player loco, set the weapon to update the anims...
    if( pLoco->IsKindOf( player_loco::GetRTTI() ) )
    {
        // Update player loco
        player_loco* pPlayerLoco = (player_loco*)pLoco;
        pPlayerLoco->SetWeapon( m_CurrentWeaponItem );
        pPlayerLoco->UpdateAnims( DeltaTime, m_bIsAirborn, m_bIsCrouching, m_LeanAmount );
    }        
    else
    {
        // This is a non-player npc avatar.
        
        if( IsCrouching() )
        {
            pLoco->SetMoveStyle( loco::MOVE_STYLE_CROUCHAIM );
        }
        else
        {
            // TO DO: Send over current move style
            pLoco->SetMoveStyle( loco::MOVE_STYLE_RUNAIM );
        }
    }

    // Update loco.
    pLoco->OnAdvance( DeltaTime );
}

//=============================================================================

void actor::OnAliveLogic( f32 DeltaTime )
{
    (void)DeltaTime;
}

//=============================================================================

void actor::OnDeathLogic( f32 DeltaTime )
{
    (void)DeltaTime;

#ifndef X_EDITOR
    // Is this a character?
    if ( IsKindOf( character::GetRTTI() ) )
    {
        // Characters (bots) only respawn for on-line games
        if( g_NetworkMgr.IsOnline() == FALSE )
            return;
    }
#endif

    // DMT HACK - Until the GameLogic gets going.
#ifdef X_EDITOR
    if( m_bWantToSpawn )
        OnSpawn();
#else
    if( m_bWantToSpawn )
        pGameLogic->RequestSpawn( m_NetSlot );
#endif // X_EDITOR
}

//=============================================================================

void actor::OnRender( void )
{
    #ifndef X_EDITOR
        // If this is a network game, skip if dead body has been created
        if( ( m_CorpseGuid ) && ( g_NetworkMgr.IsOnline() ) )
            return;
    #endif

    // Reset timer
    m_TimeSinceLastRender = 0.0f;
    
    // Lookup skin geometry
    skin_geom* pSkinGeom = GetSkinInst().GetSkinGeom();
    if (!pSkinGeom)
        return;

#ifndef X_RETAIL
    if( g_RenderBoneBBoxes )
    {
        RenderHitLocations();
    }
#endif

    // Compute LOD mask
    u64 LODMask = m_SkinInst.GetLODMask(GetL2W());
    if( m_CloakState == CLOAKING_ON )
    {
        s32 CloakVMesh = pSkinGeom->GetVMeshIndex( "CLOAK" );
        if( CloakVMesh != -1 )
        {
            u16 ScreenSize = (u16)eng_GetView()->CalcScreenSize( GetPosition(), m_SkinInst.GetBBox().GetRadius() );
            LODMask = pSkinGeom->GetLODMask( 1<<CloakVMesh, ScreenSize );
        }
    }

    if (LODMask == 0)
        return ;

    // compute bones
    s32 nActiveBones = 0;
    const matrix4* pMatrices = GetBonesForRender( LODMask, nActiveBones );
    if( !pMatrices )
        return;

    // Setup render flags and get ambient color
    xcolor Ambient;
    u32    Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
    if ( g_RenderContext.m_bIsMutated && !g_RenderContext.m_bIsPipRender && m_bAllowedToGlow )
    {
        Flags  |= render::GLOWING;

#ifndef X_EDITOR
        // Get the score.
        const game_score& Score = GameMgr.GetScore();
        xbool ForceEnemy = FALSE;

        if( GameMgr.IsGameMultiplayer() )
        {
            actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( g_RenderContext.NetPlayerSlot );
            if( pActor && pActor->IsContagious() )
                ForceEnemy = TRUE;
        }

        if( (!ForceEnemy) && 
            (1 << Score.Player[ g_RenderContext.NetPlayerSlot ].Team) & net_GetTeamBits() )
        {
            Ambient = m_FriendlyGlowColor;
        }
        else 
#endif
        {
            Ambient = m_EnemyGlowColor;
        }
        Ambient.A = (u8)(Ambient.A*m_CurrentGlowPulse);
    }
    else
    {
        Ambient = GetFloorColor();
    }

    // Render
    if ( (m_CloakState == CLOAKING_ON) ||
         (m_CloakState == CLOAKING_TURNING_OFF) )
    {
        // TODO: Set up this pipeline so that we can use a lower poly distortion mesh
        // rather than the high poly one
        radian3 NormalRot( R_0, R_0, R_0 );
        if ( m_CloakShieldPainTimer > 0.0f )
        {
            // if our cloaking shield needs to settle down a bit, add some randomness
            // to the normal rotations to show we're in an unstable state.
            radian MaxRot = m_CloakShieldPainTimer * R_360;
            NormalRot.Yaw   += x_frand( R_0, MaxRot );
            NormalRot.Pitch += x_frand( R_0, MaxRot );
            NormalRot.Roll  += x_frand( R_0, MaxRot );
        }
        GetSkinInst().RenderDistortion(&GetL2W(), 
            pMatrices, 
            nActiveBones,
            Flags,
            LODMask,
            NormalRot,
            xcolor(128,128,128,Ambient.A));
    }
    else
    {
        // Are we fading in?
        if ( m_SpawnFadeTime > 0.0f )
        {
            Flags |= render::FADING_ALPHA;
            f32 Alpha = 1.0f - (m_SpawnFadeTime / g_SpawnFadeTime);
            Alpha = MIN( Alpha, 1.0f );
            Alpha = MAX( Alpha, 0.0f );
            Ambient.A  = (u8)(Alpha*255.0f);
        }
        else
        if( m_SkinInst.GetAlpha() != 255 )
        {
            Flags |= render::FADING_ALPHA;
        }

        GetSkinInst().Render(&GetL2W(), 
            pMatrices, 
            nActiveBones,
            Flags,
            LODMask,
            Ambient);
    }

    // render our weapon if alive
    if( IsAlive() )
    {    
        OnRenderWeapon();  
    }

    // Render effects
    if( m_pEffects )
        m_pEffects->Render( this );
}

//=============================================================================

void actor::OnRenderShadowCast( u64 ProjMask )
{
    #ifndef X_EDITOR
        // If this is a multiplayer game, skip if dead body has been created.
        if( ( m_CorpseGuid ) && ( GameMgr.IsGameMultiplayer() ) )
            return;
    #endif

    // Reset timer
    m_TimeSinceLastRender = 0;    
        
    // don't render a shadow if we're cloaked
    if ( m_CloakState == CLOAKING_ON )
        return;

    // Lookup skin geometry
    skin_geom* pSkinGeom = GetSkinInst().GetSkinGeom();
    if (!pSkinGeom)
        return;

    // Compute LOD mask for the normal render
    u64 LODMask = GetSkinInst().GetLODMask(GetL2W()) ;
    if (LODMask == 0)
        return ;

    // Compute LOD mask for the shadow render (by forcing 0 for the screen size
    // we are sure to get the lowest LOD)
    u64 ShadLODMask = GetSkinInst().GetLODMask(0);
    if (ShadLODMask == 0)
        return ;

    // Setup render flags
    u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

    // compute bones
    s32 nActiveBones = 0;
    const matrix4* pMatrices = GetBonesForRender( LODMask|ShadLODMask, nActiveBones );

    // Render
    GetSkinInst().RenderShadowCast(&GetL2W(), 
                                pMatrices, 
                                nActiveBones,
                                Flags,
                                ShadLODMask,
                                ProjMask );
}

//=============================================================================

void actor::OnRenderTransparent( void )
{
    if( m_pEffects )
        m_pEffects->RenderTransparent( this );

#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() && m_pMPContagion )
    {
        s32 i;
        for( i = 0; i < 32; i++ )
        {
            m_pMPContagion->Arc[i].Render();
        }
    }
#endif
}

//=============================================================================

void actor::OnRenderWeapon ( void )
{
    //render weapon
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
        return;

    // Always 3rd person for actor
    pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );

#ifdef X_EDITOR
    if (GetAttrBits() & ATTR_EDITOR_SELECTED)
    {
        pWeapon->RenderWeapon( character::s_bDebugLoco, GetFloorColor(), (m_CloakState == CLOAKING_ON) );
    }
    else
#endif // X_EDITOR
    {
        pWeapon->RenderWeapon( FALSE, GetFloorColor(), (m_CloakState == CLOAKING_ON) );
    }
}

//==============================================================================

xbool ALLOW_COLCHECK_BBOX = TRUE;

void actor::OnColCheck( void )
{
    // Skip if dead - so that player does not get in the way of corpse ragdoll in MP
    if( IsDead() )
        return;

    // If not loco pointer then just bail.
    if( !GetLocoPointer() )
        return;

    // Get the object that's trying to collide with us.
    object* pObject = g_ObjMgr.GetObjectByGuid( g_CollisionMgr.GetMovingObjGuid() );


    //
    // If this is a ray of some sort then give it special consideration.
    //
    if( (g_CollisionMgr.GetDynamicPrimitive() == PRIMITIVE_DYNAMIC_RAY) ||
        (g_CollisionMgr.GetDynamicPrimitive() == PRIMITIVE_DYNAMIC_LOS) )
    {
        // Do trivial reject with tighter bbox
        f32 T;

        // inflate the collision bbox a bit to encompass all of the head and extremeties
        bbox ColBBox = GetColBBox();
        ColBBox.Inflate(50.0f, 50.0f, 50.0f);
        
        const collision_mgr::dynamic_ray&  Ray = g_CollisionMgr.GetDynamicRay();
        if( ColBBox.Intersect(T,Ray.Start,Ray.End) == FALSE )
            return;
        

        xbool bProvideBBoxAsCollision = FALSE;

        //
        // Check if this is a bullet. 
        //
        if( pObject && pObject->IsKindOf( base_projectile::GetRTTI() ))
        {
            base_projectile* pProjectile = ( base_projectile* ) pObject;
            guid OwnerGuid = pProjectile->GetOwnerID();
            object* pOwner = g_ObjMgr.GetObjectByGuid( OwnerGuid );

            // Check if we are being shot at.
            if( pOwner )
            {
                if( IsEnemyFaction(GetFactionForGuid(OwnerGuid)) )
                {
                    loco* pLoco = GetLocoPointer();
                    bbox Box = pLoco->m_Physics.GetBBox();
                    Box.Translate( GetPosition() );
                    Box.Inflate(50,50,50);

                    if (Box.Intersect( T, Ray.Start, Ray.End ))
                    {                        
                        if (T>=0 && T<=1)
                        {
                            // Handle shot at stuff here.
                            OnBeingShotAt( pProjectile->GetType() , pProjectile->GetOwnerID() );                                    
                        }
                    }
                }
            }

            // Check if shooter is a character
            if( pOwner && pOwner->IsKindOf( character::GetRTTI() ) )
            {
                bProvideBBoxAsCollision = TRUE;
            }
        }

        //
        // Check if caller is a character
        //
        if( pObject && pObject->IsKindOf( character::GetRTTI() ) )
        {
            bProvideBBoxAsCollision = TRUE;
        }

        //
        // Should we just send our bbox?
        //
        if( bProvideBBoxAsCollision && ALLOW_COLCHECK_BBOX)
        {
            g_CollisionMgr.StartApply( GetGuid() );
            bbox BBox = GetLocoPointer()->m_Physics.GetBBox();
            BBox.Transform( GetL2W() );
            g_CollisionMgr.ApplyAABBox( BBox, object::MAT_TYPE_FLESH, (geom::bone::HIT_LOCATION_TORSO)<<24 );
            g_CollisionMgr.EndApply();
            return;
        }
    }

    //
    // Apply individual bone locations if requesting high poly
    // else feed in bbox
    //
    {
        //  If hit locations have been added then let's use those
        if( g_CollisionMgr.IsUsingHighPoly() )
        {
            // Get the juicy info about the mesh and the skeleton
            geom* pGeom = GetSkinInst().GetGeom();
            ASSERT( pGeom );
            if (!pGeom)
                return;

            // Get loco
            loco* pLoco = GetLocoPointer();
            if( !pLoco )
                return;

            // Must have an animation
            if( !pLoco->IsAnimLoaded() )                
                return;
                
            // Lookup # of bones since LOD system may not be using all of them
            s32 nBones = x_min( pLoco->GetNActiveBones(), (s32)pGeom->m_nBones );

            // Lookup animation matrices
            const matrix4* pMatrices = pLoco->ComputeL2W();
            if (!pMatrices)
                return;

            s32 i;
            s32 nStoredCollisions = g_CollisionMgr.m_nCollisions;

            // Loop over all bones
            ASSERT( nBones <= pGeom->m_nBones);
            
            // look for headshots first
            g_CollisionMgr.StartApply( GetGuid() );
            for (i=0;i<nBones;i++)
            {
                // Lookup bone and skip if no hit location specified
                const geom::bone& Bone = pGeom->m_pBone[i];

                // unknown, skip it
                if( Bone.HitLocation == geom::bone::HIT_LOCATION_UNKNOWN )
                    continue;

                // not a head bone, skip it
                if( Bone.HitLocation != geom::bone::HIT_LOCATION_HEAD )
                    continue;

                // Lookup bone info
                const bbox&    LocalBBox = Bone.BBox;
                const matrix4& L2W       = pMatrices[i];

                // Apply oriented bbox
                g_CollisionMgr.ApplyOOBBox( LocalBBox,
                                            L2W,
                                            object::MAT_TYPE_FLESH,                                                               
                                            ( Bone.HitLocation << 24 ) | i ); // Type | Index
            }
            g_CollisionMgr.EndApply();

            // hit a head bone, this is a headshot get out
            if( g_CollisionMgr.m_nCollisions != nStoredCollisions )
            {
                // x_printfxy(2,2, "*** HEAD SHOT ****" );

                return;
            }

            g_CollisionMgr.StartApply( GetGuid() );
            for (i=0;i<nBones;i++)
            {
                // Lookup bone and skip if no hit location specified
                const geom::bone& Bone = pGeom->m_pBone[i];

                if( Bone.HitLocation == geom::bone::HIT_LOCATION_UNKNOWN )
                    continue;

                // head bone, skip it
                if( Bone.HitLocation == geom::bone::HIT_LOCATION_HEAD )
                    continue;

                // Lookup bone info
                const bbox&    LocalBBox = Bone.BBox;
                const matrix4& L2W       = pMatrices[i];

                // Apply oriented bbox
                g_CollisionMgr.ApplyOOBBox( LocalBBox,
                    L2W,
                    object::MAT_TYPE_FLESH,                                                               
                    ( Bone.HitLocation << 24 ) | i ); // Type | Index
            }
            g_CollisionMgr.EndApply();
        }
        else
        {
            // Let physics do it's stuff
            GetLocoPointer()->m_Physics.ApplyCollision();
        }
    }
}

//=============================================================================

#ifndef X_RETAIL
void  actor::OnColRender( xbool bRenderHigh )
{
    (void)bRenderHigh;

    // Using locomotion?
    if (GetLocoPointer())
    {
        RenderHitLocations();
        GetLocoPointer()->m_Physics.RenderCollision();
        bbox BBox(GetLocoPointer()->m_Physics.GetBBox() );
        BBox.Transform( GetL2W() );
        draw_BBox( BBox );
    }
    else
    {
        // Just your regular bounding box
        draw_BBox( GetBBox() );
    }
}
#endif // X_RETAIL

//=============================================================================

void actor::OnMove( const vector3& NewPos )
{
    // Skip?
    if( GetAttrBits() & object::ATTR_DESTROY )
        return;

    // Tell loco (this needs to happen event if the position hasn't changed so
    // that ghost locos work correctly)
    if (GetLocoPointer())
        GetLocoPointer()->SetPosition(NewPos);

    // TODO: mreed: This condition should be in here (for network optimization), 
    // but for some strange reason, it breaks player crouch. Crouch doesn't update until
    // NewPos has actually changed.
    // 
    // Skip if nothing to update
    //if( NewPos == GetPosition() )
    //    return;

    #ifndef X_EDITOR
    m_NetDirtyBits |= POSITION_BIT;
    #endif X_EDITOR

    // Call base class
    object::OnMove(NewPos);

    // move our weapon
    MoveWeapon( TRUE );

    // Update zone tracker
    UpdateZoneTrack();
}

//=========================================================================

void actor::OnTransform( const matrix4& L2W )
{
    // Skip?
    if( GetAttrBits() & object::ATTR_DESTROY )
        return;

    // Call base class
    object::OnTransform(L2W);

    #ifndef X_EDITOR
    m_NetDirtyBits |= POSITION_BIT;
    m_NetDirtyBits |= ORIENTATION_BIT;
    #endif X_EDITOR
 
    // Setup loco?
    if( ( GetLocoPointer() ) && ( !m_bIgnoreLocoInTransform ) )
    {
        // Get info
        vector3 Pos = L2W.GetTranslation();
        radian  Yaw = L2W.GetRotation().Yaw;

        // Update loco
        GetLocoPointer()->SetPosition(Pos);
        GetLocoPointer()->SetYaw(Yaw + R_180); // SB +R_180 is legacy for old anim system
    }

    // move our weapon
    MoveWeapon( TRUE );

    // Update zone tracker
    UpdateZoneTrack();
}

//===========================================================================

bbox actor::GetColBBox( void )
{
    if ( m_pLoco )
    {
        // Start with physics bbox
        bbox BBox = m_pLoco->m_Physics.GetBBox();
        
        // Take lean into account so leaning ghosts/players can be hit in MP
        f32  LeanDist = x_abs( GetLeanAmount() * 100.0f );
        BBox.Inflate( LeanDist, 0.0f, LeanDist );
        
        // Convert into world space
        BBox.Transform( GetL2W() );
        return BBox;
    }
    else
    {
        return object::GetColBBox();
    }
}

//===========================================================================

bbox actor::GetLocalBBox( void ) const
{
    if( m_pLoco )
    {
        // Start with physics bbox
        bbox BBox = m_pLoco->m_Physics.GetBBox();

        // Take lean into account so leaning ghosts/players can be hit in MP
        f32  LeanDist = x_abs( GetLeanAmount() * 100.0f );
        BBox.Inflate( LeanDist, 0.0f, LeanDist );
        
        return BBox;
    }                
    else
    {
        bbox BBox;
        BBox.Min.Set( -50.0f, 0.0f,   -50.0f );
        BBox.Max.Set(  50.0f, 200.0f,  50.0f );
        return BBox;
    }
}      

//===========================================================================
// HEALTH FUNCTIONS
//===========================================================================

xbool actor::AddHealth( f32 DeltaHealth )
{
    // If player can not die, leave his health otherwise it screws with the AI targeting!
    if( (DeltaHealth < 0.0f) && (!m_bCanDie) )
    {
        return FALSE;
    }
    
    f32 OldHealth = m_Health.GetHealth();

    m_Health.Add( DeltaHealth, IsCharacter() );

    #ifndef X_EDITOR
    if( OldHealth != m_Health.GetHealth() )
    {
        m_NetDirtyBits |= HEALTH_BIT;
    }
    #endif

    return( TRUE );
}

//===========================================================================
// FACTION/FRIEND FUNCTIONS
//===========================================================================

factions actor::GetFactionForGuid( guid Guid ) const
{
    object *tempObject = g_ObjMgr.GetObjectByGuid(Guid);

    if( tempObject && tempObject->IsKindOf(actor::GetRTTI()) )
    {
        actor &actorObject = actor::GetSafeType( *tempObject ); 
        return actorObject.GetFaction();
    }
    else if( tempObject && tempObject->IsKindOf(turret::GetRTTI()) )
    {
        turret &turretObject = turret::GetSafeType( *tempObject );
        return turretObject.GetFaction();
    }

    return FACTION_NOT_SET;
}

//===========================================================================
// INVENTORY FUNCTIONS
//===========================================================================

void actor::InitInventory( void )
{
}

//===========================================================================

void actor::ReInitInventory( void )
{
}

//===========================================================================

/*
xbool actor::HasWeaponInInventory( inventory_item::inv_type weaponType )
{
    if( weaponType < inventory_item::INV_FIRST_WEAPON || weaponType > inventory_item::INV_LAST_WEAPON )
    {
        return FALSE;
    }

    for ( s32 i = 0; i < m_WeaponInventory.GetTotalPotentialCount(); i++ )
    {
        if ( m_WeaponInventory.GetCollapsedListPtr(i)->m_ItemCount > 0 )
        {
            guid WeaponGuid = m_WeaponInventory.GetCollapsedListPtr(i)->m_GuidList[0];
            ASSERT( WeaponGuid != NULL );

            object_ptr<inventory_item> WeaponObj( WeaponGuid );
            ASSERT( WeaponObj.IsValid() );
            if( WeaponObj.m_pObject->GetInvType() == weaponType )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
*/

//=============================================================================

xbool actor::HasItemInInventory2( inven_item Item )
{
    return m_Inventory2.HasItem( Item );
}

//=============================================================================

xbool actor::OnPickup( pickup& )
{
    return( FALSE );
}

//===========================================================================
// KILL/DAMAGE FUNCTIONS
//===========================================================================

xbool actor::OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags )
{
    // Lookup loco
    loco* pLoco = GetLocoPointer();
    if( !pLoco )
        return FALSE;

    // Try play anim
    return pLoco->PlayAnim( AnimType, BlendTime, Flags );
}

//===========================================================================

void actor::OnDeath( void )
{
    #ifndef X_EDITOR
//  LOG_MESSAGE( "actor::OnDeath", "Slot:%d", m_NetSlot );
    #endif

//  LOG_MESSAGE( "actor::OnDeath", "------------------------" );

    m_Health.Dead();
    m_bDead        = TRUE;
    m_bWantToSpawn = FALSE;  // Clear it for now just to be safe.
    m_LeanAmount   = 0.0f;

    // Stop any weapon looping sfx
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->ReleaseAudio();
        pWeapon->KillAllMuzzleFX();
        pWeapon->EndPrimaryFire();
        pWeapon->EndAltFire();

#ifndef X_EDITOR        
        net_EndFirePrimary();
#endif        
    }

    // Create the corpse
    xbool bCreateCorpse = TRUE;
#ifdef X_EDITOR
    // Do not create player corpse in single player games!
    if( ( GetType() == object::TYPE_PLAYER ) )
    {
        bCreateCorpse = FALSE;
    }
#else
    // Do not create player corpse in single player games!
    if( ( GetType() == object::TYPE_PLAYER ) && ( GameMgr.GetGameType() == GAME_CAMPAIGN ) )
    {
        bCreateCorpse = FALSE;
    }

    // Drop a weapon.
    if( GameMgr.IsGameMultiplayer() && g_NetworkMgr.IsServer() )
    {
        net_DropWeapon();
    }
#endif
    // Create corpse?
    if( bCreateCorpse )
    {
        CreateCorpse();
    }

    #ifndef X_EDITOR
    {
        if( m_Net.LifeSeq & 0x01 )
        {
            // LifeSeq was already 'odd' (meaning DEAD).  Strange.
            LOG_WARNING( "actor::OnDeath", "Unexpected value for LifeSeq." );
        }
        else
        {
            // LifeSeq was 'even' (alive).  Make it 'odd' (DEAD!).
            m_Net.LifeSeq += 1;
        }

        m_Net.LifeSeq  &= 0x07;
        m_NetDirtyBits |= LIFE_BIT;
    }

    // Shut off the tendrils
    if (m_pMPContagion)
    {
        m_pMPContagion->PlayerMask = 0;
    }
    #endif // X_EDITOR

    // Turn off collision when you die...
    SetAttrBits( (GetAttrBits() & ~((u32)0 | ATTR_COLLIDABLE | ATTR_LIVING)) );
}

//===========================================================================

void actor::OnSpawn( void )
{
    #ifndef X_EDITOR
    /*
    LOG_MESSAGE( "actor::OnSpawn", "Slot:%d - Position:%g,%g,%g", 
                                   m_NetSlot,
                                   GetPosition().GetX(), 
                                   GetPosition().GetY(), 
                                   GetPosition().GetZ() );
    */
    #endif
   
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
        ClearInventory2();
#endif

    m_Health.Reset();
    m_bDead        = FALSE;
    m_bWantToSpawn = FALSE;

    m_bIsCrouching = FALSE;
    m_bIsAirborn   = FALSE;

    if( m_CorpseGuid )
    {
        corpse* pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid( m_CorpseGuid );
        if( pCorpse )
            pCorpse->SetPermanent( FALSE );

        m_CorpseGuid = 0;   // NET HACK
    }

    #ifndef X_EDITOR
    {
        if( m_Net.LifeSeq & 0x01 )
        {
            // LifeSeq was 'odd' (dead).  Make it 'even' (ALIVE!).
            m_Net.LifeSeq += 1;
        }
        else
        {
            // LifeSeq was already 'even' (meaning ALIVE).  Strange.
            LOG_WARNING( "actor::OnSpawn", "Unexpected value for LifeSeq." );
        }

        m_Net.LifeSeq     &= 0x07;
        m_NetDirtyBits    |= LIFE_BIT;
        m_WayPointFlags    = 0;
        m_WayPointTimeOut  = 0;
    }

    {
        if ( GameMgr.GetGameType() != GAME_CAMPAIGN )
        {
            // set the spawn timers
            m_SpawnFadeTime     = g_SpawnFadeTime;
            if( IsPlayer() )
                m_SpawnNeutralTime  = 1.0f;
            else
                m_SpawnNeutralTime  = 3.0f;
        }
    }
    #endif X_EDITOR

    // Bring loco back alive
    if( GetLocoPointer() )
        GetLocoPointer()->SetDead( FALSE );

    // This assumes that the player is about 2.5 meters tall.  Then is about 2 
    // meters around.  The bbox is center around the eyes of the player.
    m_ZoneTracker.SetBBox( bbox( vector3( -100, -200, -100 ), 
                                 vector3(  100,   50,  100 ) ) );

    // Turn on collision when you spawn.
    SetAttrBits( GetAttrBits() | ATTR_COLLIDABLE | ATTR_LIVING );

    // Spawning is NOT a sign of activity.
    m_RecentPosition = GetPosition();

    // Shut down any actor effects
    if (GetActorEffects())
    {
        GetActorEffects()->Kill();
    }

#ifdef X_EDITOR
    xbool bIsMultiplayer = FALSE;
#else
    xbool bIsMultiplayer = GameMgr.IsGameMultiplayer();
#endif

    if( bIsMultiplayer )
    {
        GetActorEffects( TRUE )->InitEffect( actor_effects::FX_SPAWN, this );
    }
}

//==============================================================================

corpse* actor::CreateCorpseObject( xbool BodyFades )
{
    // create a dead body object
    m_CorpseGuid = g_ObjMgr.CreateObject( corpse::GetObjectType() );
    ASSERT( m_CorpseGuid != 0 );

    // initialize the dead body object
    corpse* pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid( m_CorpseGuid );
    ASSERT( pCorpse );
    pCorpse->Initialize( *this, BodyFades, GetActorEffects() );
    
    // Because we've passed the actor effects over to the dead body, it will
    // be responsible for destroying it. So set our pointer back to NULL.
    m_pEffects = NULL;

    return pCorpse;
}

//==============================================================================

void actor::CreateCorpse( void )
{
    // Must have loco
    if( !m_pLoco )
        return;
        
    // Skip if already created a corpse        
    if( m_CorpseGuid )
        return;

//  LOG_MESSAGE( "actor::CreateCorpse", "DEATH_BY_ANIM" );

    // Should body fade?
    xbool BodyFades = TRUE;
    if ( IsKindOf( player::GetRTTI() ) )
    {
        BodyFades = FALSE;
    }

    // Create dead body
    corpse* pCorpse = CreateCorpseObject( BodyFades );
    if( !pCorpse )
        return;
    
    // Only do pain to the corpse if the m_PainThatKilledUs 
    // is recent enough to be valid.
    if( m_TimeSinceLastPain < 0.12f )
    {
        pCorpse->OnPain( m_PainThatKilledUs );
    }
    else
    {
        // Apply pain from net?    
        if( pCorpse )
        {
            m_CorpseDeathPain.Apply( *pCorpse );
        }            
    }
}

//=============================================================================

#ifdef X_EDITOR
void actor::EditorPreGame( void )
{
    // TODO: CJ: WEAPONS: Need code to create all the weapons for actors in the editor
    //m_WeaponInventory.EditorPreGame();
}
#endif // X_EDITOR

//=============================================================================
//  GetHitLocation
//
//      returns hit location based on the last collision check.
//
//=============================================================================
geom::bone::hit_location actor::GetHitLocation ( const pain& Pain )
{   
    // Be sure this is a direct hit and we have collision info
    if( Pain.HasCollision()==FALSE )
        return geom::bone::HIT_LOCATION_UNKNOWN_WRONG_GUID;

    const collision_mgr::collision& Coll = Pain.GetCollision();
    if( Coll.ObjectHitGuid != GetGuid() )
        return geom::bone::HIT_LOCATION_UNKNOWN_WRONG_GUID;

    s32 Key = (Coll.PrimitiveKey);
    Key >>= 24;

    geom::bone::hit_location Loc = (geom::bone::hit_location)Key;

    if (Loc < geom::bone::HIT_LOCATION_START)
        return geom::bone::HIT_LOCATION_UNKNOWN;

    if (Loc >= geom::bone::HIT_LOCATION_COUNT)
        return geom::bone::HIT_LOCATION_UNKNOWN;

    return Loc;
}


//=============================================================================
//  GetHitLocationName
//
//  Converts hit location enum into a string
//
//=============================================================================

const char* actor::GetHitLocationName( geom::bone::hit_location Loc )
{
    switch( Loc )
    {        
        case geom::bone::HIT_LOCATION_HEAD:
            return "HIT_LOCATION_HEAD";
        case geom::bone::HIT_LOCATION_SHOULDER_LEFT:
            return "HIT_LOCATION_SHOULDER_LEFT";
        case geom::bone::HIT_LOCATION_SHOULDER_RIGHT:
            return "HIT_LOCATION_SHOULDER_RIGHT";
        case geom::bone::HIT_LOCATION_TORSO:
            return "HIT_LOCATION_TORSO";
        case geom::bone::HIT_LOCATION_LEGS:
            return "HIT_LOCATION_LEGS";
        case geom::bone::HIT_LOCATION_COUNT:
            return "HIT_LOCATION_COUNT";
        case geom::bone::HIT_LOCATION_UNKNOWN:
            return "HIT_LOCATION_UNKNOWN";
        case geom::bone::HIT_LOCATION_UNKNOWN_WRONG_GUID:
            return "HIT_LOCATION_UNKNOWN_WRONG_GUID";
        default:
            return "INVALID_HIT_LOCATION";
    }
}

//=============================================================================

void actor::PlayImpactAnim( const pain& Pain, eHitType hitType )
{
    geom::bone::hit_location hitLocation = GetHitLocation( Pain );

//  LOG_MESSAGE( "actor::PlayImpactAnim", "Hit location: %s", GetHitLocationName(hitLocation) );

    radian FaceYaw  = GetLocoPointer()->m_Player.GetFacingYaw();
    radian PainYaw  = Pain.GetDirection().GetYaw();
    radian PainDeltaYaw = x_MinAngleDiff(FaceYaw, PainYaw);

    xbool bFromFront = FALSE;
    if (ABS(PainDeltaYaw) > R_90)
    {
        bFromFront = TRUE;
    }

    if( IsAlive() )
    {
        if (HandleSpecialImpactAnim(hitType))
        {
            //did special handling for impact anim
            return;
        }
    }
    

    if ( hitType == HITTYPE_PLAYER_MELEE_1 ) //light hit
    {
        OnPlayFullBodyImpactAnim(   loco::ANIM_DAMAGE_PLAYER_MELEE_0,     // AnimType
                                    DEFAULT_BLEND_TIME,                   // Blend time
                                    loco::ANIM_FLAG_INTERRUPT_BLEND  |    // Flags
                                    loco::ANIM_FLAG_TURN_OFF_AIMER   |
                                    loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
    //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_DAMAGE_PLAYER_MELEE_0");
    }
    else if ( hitType == HITTYPE_LIGHT ) //light hit
    {
        // first figure out the additive impact anim to play
        loco::anim_type ImpactAnimType = loco::ANIM_NULL ;
        switch(hitLocation)
        {
            case geom::bone::HIT_LOCATION_HEAD:
                if (bFromFront)
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_HEAD_FRONT ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_HEAD_FRONT");
                }
                else
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_HEAD_BACK ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_HEAD_BACK");
                }
                break ;

            case geom::bone::HIT_LOCATION_SHOULDER_RIGHT:
                if (bFromFront)
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_SHOULDER_RIGHT_FRONT ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_SHOULDER_RIGHT_FRONT");
                }
                else
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_SHOULDER_RIGHT_BACK ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_SHOULDER_RIGHT_BACK");
                }
                break ;

            case geom::bone::HIT_LOCATION_SHOULDER_LEFT:
                if (bFromFront)
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_SHOULDER_LEFT_FRONT ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_SHOULDER_LEFT_FRONT");
                }
                else
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_SHOULDER_LEFT_BACK ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_SHOULDER_LEFT_BACK");
                }
                break ;
        
            case geom::bone::HIT_LOCATION_TORSO:
            case geom::bone::HIT_LOCATION_LEGS:
                if (bFromFront)
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_TORSO_FRONT ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_TORSO_FRONT");
                }
                else
                {
                    ImpactAnimType = loco::ANIM_ADD_IMPACT_TORSO_BACK ;
                //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_ADD_IMPACT_TORSO_BACK");
                }
                break ;
        }    

        if (ImpactAnimType != loco::ANIM_NULL)
        {    
            m_LastTimeStaggered = k_MinTimeBetweenSmallStaggers;

            GetLocoPointer()->PlayAdditiveAnim(ImpactAnimType,                  // AnimType
                                               0.1f,                            // BlendInTime
                                               0.1f,                            // BlendOutTime
                                               ANIM_FLAG_IMPACT_CONTROLLER) ;   // Flags
        }
    }
    else if ( hitType == HITTYPE_IDLE && !IgnoreFullBodyFlinches() )
    {
        loco::anim_type ImpactAnimType = loco::ANIM_NULL ;
        if( bFromFront )
        {
            ImpactAnimType = loco::ANIM_PAIN_IDLE_FRONT;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_PAIN_IDLE_FRONT");
        }
        else
        {
            ImpactAnimType = loco::ANIM_PAIN_IDLE_BACK;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_PAIN_IDLE_BACK");
        }

        //lets also apply a random yaw change
        // messing with someone's yaw can do nasty things like screw up anim playback. 
        // removing this part.
/*        if (x_irand(0,1))
            GetLocoPointer()->SetYawFacingTarget( GetLocoPointer()->GetYaw() + x_frand(R_20, R_40), R_360 );
        else
            GetLocoPointer()->SetYawFacingTarget( GetLocoPointer()->GetYaw() - x_frand(R_20, R_40), R_360 );
*/
        if( OnPlayFullBodyImpactAnim( ImpactAnimType,                       // AnimType
                                      DEFAULT_BLEND_TIME,                   // Blend time
                                      loco::ANIM_FLAG_INTERRUPT_BLEND  |    // Flags
                                      loco::ANIM_FLAG_TURN_OFF_AIMER   |
                                      loco::ANIM_FLAG_RESTART_IF_SAME_ANIM ) )
        {
            m_BigPainTakenTime = (f32)x_GetTimeSec();
            m_LastTimeStaggered = k_MinTimeBetweenBigStaggers;
        }
    }
    else if ( !IgnoreFullBodyFlinches() ) // hard hit
    {
        // next figure out our stagger anim, if any
        loco::anim_type StagerAnimType = loco::ANIM_NULL ;

        object *originObject = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
        if( originObject && 
            originObject->IsKindOf(genericNPC::GetRTTI()) &&
            HasAnim(loco::ANIM_DAMAGE_PARASITE) )
        {
            StagerAnimType = loco::ANIM_DAMAGE_PARASITE;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_DAMAGE_PARASITE");
        }
        else if ((PainDeltaYaw >= R_45) && (PainDeltaYaw <= R_135))
        {
            StagerAnimType = loco::ANIM_DAMAGE_STEP_RIGHT;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_DAMAGE_STEP_RIGHT");
        }
        else if ((PainDeltaYaw <= -R_45) && (PainDeltaYaw >= -R_135))
        {
            StagerAnimType = loco::ANIM_DAMAGE_STEP_LEFT;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_DAMAGE_STEP_LEFT+");
        }
        else if ((PainDeltaYaw >= -R_45) && (PainDeltaYaw <= R_45))
        {
            StagerAnimType = loco::ANIM_DAMAGE_STEP_FORWARD;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_DAMAGE_STEP_FORWARD");
        }
        else
        {
            StagerAnimType = loco::ANIM_DAMAGE_STEP_BACK;
        //  LOG_MESSAGE("actor::PlayImpactAnim","ANIM_DAMAGE_STEP_BACK");
        }

        if( StagerAnimType != loco::ANIM_NULL )
        {
            //lets also apply a random yaw change
            // messing with someone's yaw can do nasty things like screw up anim playback. 
            // removing this part.
/*            if (x_irand(0,1))
                GetLocoPointer()->SetYawFacingTarget( GetLocoPointer()->GetYaw() + x_frand(R_20, R_40), R_360 );
            else
                GetLocoPointer()->SetYawFacingTarget( GetLocoPointer()->GetYaw() - x_frand(R_20, R_40), R_360 );*/


            if ( OnPlayFullBodyImpactAnim( StagerAnimType,                       // AnimType
                                           DEFAULT_BLEND_TIME,                   // Blend time
                                           loco::ANIM_FLAG_INTERRUPT_BLEND  |    // Flags
                                           loco::ANIM_FLAG_TURN_OFF_AIMER   |
                                           loco::ANIM_FLAG_RESTART_IF_SAME_ANIM ) )
            {
                m_BigPainTakenTime = (f32)x_GetTimeSec();
                m_LastTimeStaggered = k_MinTimeBetweenBigStaggers;
            }
        }
    }
}

//==============================================================================
// Locomotion functions
//==============================================================================

void actor::InitLoco( void )
{
    // Setup loco transform?
    if ( m_pLoco )
    {
        // Get transform info
        const matrix4& L2W = GetL2W();
        vector3 Pos = L2W.GetTranslation();
        radian  Yaw = L2W.GetRotation().Yaw;

        // Update loco
        m_pLoco->SetPosition( Pos );
        m_pLoco->SetYaw( Yaw + R_180 ); // SB +R_180 is legacy for old anim system
        m_pLoco->SetArriveDistSqr( 1.0f );
        m_pLoco->SetUseAimMoveStyles( TRUE );
    }    
}

//=============================================================================

xbool actor::IsRunning( void )
{
    // Get loco
    loco* pLoco = GetLocoPointer();
    if( !pLoco )
        return FALSE;

    // Must be running
    if( pLoco->GetState() != loco::STATE_MOVE )
        return FALSE;

    // In a run style?
    switch( pLoco->GetMoveStyle() )
    {
        case loco::MOVE_STYLE_RUN:
        case loco::MOVE_STYLE_RUNAIM:
        case loco::MOVE_STYLE_CHARGE:
            return TRUE;
    }

    return FALSE;
}

//=============================================================================

xbool actor::IsMoving( void )
{
    return !GetLocoPointer()->IsAtDestination();
}

//=============================================================================

xbool actor::IsStaggering( void )
{
    return ( m_LastTimeStaggered > 0.0f );
}

//=============================================================================

f32 actor::GetPitch( void )
{
    return m_Pitch;
}

//=============================================================================

f32 actor::GetYaw( void )
{
    return m_Yaw;
}

//=============================================================================

void actor::SetPitch( radian Pitch )
{
    if( m_pLoco )
    {
        // Update loco
        m_pLoco->SetPitch( Pitch );

        // Update weapon
        MoveWeapon( TRUE );
    }

    m_Pitch = Pitch;

    #ifndef X_EDITOR
    m_NetDirtyBits |= ORIENTATION_BIT;  // Network
    #endif
}

//=============================================================================

void actor::SetYaw( radian Yaw )
{
    if( m_pLoco )
    {
        // Update loco
        m_pLoco->SetYaw( Yaw );
        
        // Update weapon
        MoveWeapon( TRUE );
    }

    m_Yaw = Yaw;

    #ifndef X_EDITOR
    m_NetDirtyBits |= ORIENTATION_BIT;  // Network
    #endif
}

//==============================================================================
// ANIMATION FUNCTIONS
//=============================================================================

xbool actor::HasAnim( loco::anim_type animType )
{
    return( GetLocoPointer() && (GetLocoPointer()->GetAnimIndex( animType ) >= 0) );
}

//=============================================================================

xbool actor::IsAnimInPackage( const char* pAnimGroup, const char* pName )
{
    if (!GetLocoPointer() ||
        x_strlen(pName) <= 0)
    {
        return FALSE;
    }

    anim_group::handle hAnimGroup;
    s32 AnimIndex = -1;
    if (x_strlen(pAnimGroup) > 0)
    {
        hAnimGroup.SetName(pAnimGroup);
        if (!hAnimGroup.GetPointer())
        {
            return FALSE;
        }
        AnimIndex = hAnimGroup.GetPointer()->GetAnimIndex(pName);
    }
    else
    {
        return FALSE;
    }    
    if (AnimIndex == -1)
    {
        return FALSE;
    }
    return TRUE;
}

//==============================================================================

xbool actor::IsPlayingFullBodyLipSync( void )
{
    // Loco?
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        // Is full body lip sync anim playing?
        loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
        if( ( LipSyncCont.IsPlaying() ) && ( LipSyncCont.IsFullBody() ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

//==============================================================================

xbool actor::IsPlayingFullBodyCinema( void )
{
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        // Is full body lip sync anim playing?
        loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
        if( ( LipSyncCont.IsPlaying() ) && 
            ( LipSyncCont.IsFullBody() ) && 
            ( LipSyncCont.GetAnimFlags() & loco::ANIM_FLAG_CINEMA ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//==============================================================================

xbool actor::IsPlayingLipSync( void )
{
    // Loco?
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        // Is lip sync anim playing?
        loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
        if( LipSyncCont.IsPlaying() )
        {
            return TRUE;
        }
    }

    return FALSE;
}

//==============================================================================

xbool actor::IsPlayingCinema( void )
{
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        // Is lip sync cinema anim playing?
        loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
        if( ( LipSyncCont.IsPlaying() ) && 
            ( LipSyncCont.GetAnimFlags() & loco::ANIM_FLAG_CINEMA ) )
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

//==============================================================================
// ZONE FUNCTIONS
//==============================================================================

void actor::Teleport( const vector3& Position, xbool DoBlend, xbool DoEffect )
{
    /*
    LOG_MESSAGE( "actor::Teleport", "Before:%.3f,%.3f,%.3f", 
                                    GetPosition().GetX(), 
                                    GetPosition().GetY(), 
                                    GetPosition().GetZ() );
    */

    #ifndef X_EDITOR
    if( !DoBlend )
    {
        m_NetDirtyBits    |= WAYPOINT_BIT;
        m_WayPointFlags   |= WAYPOINT_TELEPORT;
        m_WayPointTimeOut  = 0;
    }

    if( DoEffect )
    {        
        f32     Best  = F32_MAX;
        object* pBest = NULL;
        slot_id Slot  = g_ObjMgr.GetFirst( TYPE_TELEPORTER );
        while( Slot != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
            ASSERT( pObject );
            ASSERT( pObject->GetType() == TYPE_TELEPORTER );
            vector3 Gap = pObject->GetPosition() - GetPosition();
            f32     L2  = Gap.LengthSquared();
            if( L2 < Best )
            {
                Best  = L2;
                pBest = pObject;
            }
            Slot = g_ObjMgr.GetNext( Slot );
        }
        if( pBest )
        {
            teleporter* pTeleporter = (teleporter*)pBest;
            pTeleporter->PlayTeleportOut();
        }

        m_NetDirtyBits    |= WAYPOINT_BIT;
        m_WayPointFlags   |= WAYPOINT_TELEPORT_FX;
        m_WayPointTimeOut  = 0;
    }
    #endif

    // Move into position.
    OnMove( Position );

    #ifndef X_EDITOR
    if( DoEffect )
    {
        slot_id Slot = g_ObjMgr.GetFirst( TYPE_TELEPORTER );
        while( Slot != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
            ASSERT( pObject );
            ASSERT( pObject->GetType() == TYPE_TELEPORTER );
            vector3 Gap = pObject->GetPosition() - Position;
            if( Gap.LengthSquared() < 250.0f )
            {
                teleporter* pTeleporter = (teleporter*)pObject;
                pTeleporter->PlayTeleportIn();
                break;
            }
            Slot = g_ObjMgr.GetNext( Slot );
        }   
    }
    #endif

    // Fire a ray down from the given position.  Use the zone information from
    // the play surface hit by the ray.

    // HACK HACK HACK
    //
    // As fate would have it, the player often likes to spawn directly 
    // over the seam between two floor pieces.  There is a slight chance
    // that a ray fired directly down from such a location may MISS all 
    // of the floor pieces due to edge conditions in the math.
    //
    // Hack solution: Do not fire a ray straight down.  We move the start
    // position 5cm on X, and we angle the shot slightly as well.  This is
    // designed to minimize the chance of such math surprises.
    //
    // HACK HACK HACK

    vector3 Start = Position + vector3( 5.0f, 10.0f, 0.0f );    // HACK - Offset start postion
    vector3 Ray( 10.0f, -500.0f, 11.0f );                       // HACK - Don't fire straight down.

    g_CollisionMgr.RaySetup( GetGuid(), Start, Start + Ray );
    g_CollisionMgr.SetMaxCollisions( 5 );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                    object::ATTR_COLLIDABLE, 
                                    (object::object_attr)
                                        (object::ATTR_COLLISION_PERMEABLE | 
                                         object::ATTR_LIVING) );

    // Loop over collisions
    for( s32 i = 0; i < g_CollisionMgr.m_nCollisions; i++ )
    {
        // Found a surface below the player.
        guid    HitGuid = g_CollisionMgr.m_Collisions[i].ObjectHitGuid;
        object* pObject = g_ObjMgr.GetObjectByGuid( HitGuid );
        ASSERT( pObject );
        
        // Skip invisible walls because they can span more than one zone
        if( pObject->GetType() == object::TYPE_INVISIBLE_WALL_OBJ )
            continue;
        
        // Copy zones from object (position has already been updated)
        SetZone1( pObject->GetZone1() );
        SetZone2( pObject->GetZone2() );
        
        // Update the zone tracker
        g_ZoneMgr.InitZoneTracking( *pObject, m_ZoneTracker );
        
        /*
        LOG_MESSAGE( "actor::Teleport", "Zone1:%d - Zone2:%d", 
                    pObject->GetZone1(), pObject->GetZone2() );
        */
        
        // Exit for loop
        break;
    }
    
    /*
    LOG_MESSAGE( "actor::Teleport", "After:%.3f,%.3f,%.3f", 
                                    GetPosition().GetX(), 
                                    GetPosition().GetY(), 
                                    GetPosition().GetZ() );
    */
}

//==============================================================================

void actor::Teleport( const vector3& Position, 
                            radian   Pitch, 
                            radian   Yaw, 
                            xbool    DoBlend, 
                            xbool    DoEffect )
{
    Teleport( Position, DoBlend, DoEffect );
    SetPitch( Pitch );
    SetYaw  ( Yaw   );
}

//==============================================================================

void actor::InitZoneTracking( void )
{
    // Update the zone tracker
    g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
}

//==============================================================================

void actor::UpdateZoneTrack ( void )
{ 
    // Update tracker
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, GetPosition() );
}

//==============================================================================

void actor::UpdateZone( u8 Zone )
{
    SetZone1(Zone);
    SetZone2(0);
}

//==============================================================================
// Contagion FUNCTIONS
//==============================================================================

void actor::ContagionLogic( f32 DeltaTime )
{
    if( !IsContagious() )
    {
        #ifndef X_EDITOR
        // If we still have a MPContagion structure, then we are waiting for
        // some tendrils to die out.
        if( m_pMPContagion )
        {
            ASSERT( GameMgr.IsGameMultiplayer() );
            s32 AllDone = TRUE;
            for( s32 i = 0; i < 32; i++ )
            {
                ASSERT( m_pMPContagion->Arc[i].Validate() );                
                if( !m_pMPContagion->Arc[i].IsFinished() )
                {
                    m_pMPContagion->Arc[i].AdvanceLogic( DeltaTime );
                    AllDone = FALSE;
                }
            }
            if( AllDone )
                KillMPContagion();
        }
        #endif

        // Nothing to see here.  Move along.
        return;
    }

    // Update the timers.
    m_ContagionDOTTimer -= DeltaTime;
    m_ContagionTimer    -= DeltaTime;

    // End of contagion?
    if( IsDead() || (m_ContagionTimer <= 0.0f) )
    {
        if( IsAlive() && IsCharacter() )
        {
            pain Pain;
            Pain.Setup( "CONTAGION_FINAL", 0, GetPosition() );
            Pain.SetDirectHitGuid( GetGuid() );
            Pain.ApplyToObject( GetGuid() );
        }

        #ifndef X_EDITOR
        m_NetDirtyBits |= CONTAGION_OFF_BIT;
        
        /*
        if( m_pMPContagion )
        {
            LOG_MESSAGE( "actor::ContagionLogic", 
                         "End -- Origin:%d",
                         m_pMPContagion->Origin );
        }
        */
        
        if( GameMgr.IsGameMultiplayer() )
        {
            ASSERT( m_pMPContagion );
            m_pMPContagion->PlayerMask = 0x00000000;
            for( s32 i = 0; i < 32; i++ )
            {
                m_pMPContagion->Arc[i].SetSuspended( TRUE );
            }
        }
        #endif

        m_bContagious = FALSE;
        if( m_pEffects )
            m_pEffects->KillEffect( actor_effects::FX_CONTAIGON );

        // Show's over, people.  Go on home.
        return;
    }   

    #ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        ASSERT( m_pMPContagion );

        // Reacquire all targets.

        m_pMPContagion->PlayerMask = 0x00000000;

        f32 RangeSqr = s_MPContagion_TouchDistance.GetF32();
        if( RangeSqr == 0.0f )
            RangeSqr = 1000.0f;
        RangeSqr *= RangeSqr;

        for( s32 i = 0; i < 32; i++ )
        {
            if( i == m_NetSlot )
                continue;

            actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
            if( pActor && pActor->IsAlive() )
            {
                vector3 Offset( 0, 100, 0 );
                vector3 Here  =         GetPosition() + Offset;
                vector3 There = pActor->GetPosition() + Offset;
                vector3 Gap   = Here - There;
                if( Gap.LengthSquared() < RangeSqr )
                {
                    if( g_ObjMgr.HasLOS( GetGuid(), Here, 
                                        pActor->GetGuid(), There ) )
                    {
                        m_pMPContagion->PlayerMask |= (1<<i);   
                    }
                }
            }

            // Make sure origin is still around.
            if( (m_pMPContagion->Origin == i) && (!pActor) )
                m_pMPContagion->Origin = -1;
        }

        // Update the arcs.
        vector3 Center = GetPosition() + vector3(0,100,0);
        for( s32 i = 0; i < 32; i++ )
        {
            if( m_pMPContagion->PlayerMask & (1<<i) )
            {
                m_pMPContagion->Arc[i].SetSuspended( FALSE );
                object* pObject = (object*)NetObjMgr.GetObjFromSlot( i );
                ASSERT( pObject );

                vector3 Point = pObject->GetPosition();
                Point += vector3(0,100,0);

                vector3 Axis = (Center - Point);
                radian  Pitch;
                radian  Yaw;

                Axis.GetPitchYaw( Pitch, Yaw );

                matrix4 L2W( vector3( 1.0f, 1.0f, (Axis.Length() / 500.0f) ), 
                             radian3( Pitch, Yaw, R_0 ), 
                             Center );

                m_pMPContagion->Arc[i].SetTransform( L2W );
            }
            else
            {
                m_pMPContagion->Arc[i].SetSuspended( TRUE );
            }
        }

        // Advance the effect logic on the tendril arcs.
        for( s32 i = 0; i < 32; i++ )
        {
            ASSERT( m_pMPContagion->Arc[i].Validate() );
            m_pMPContagion->Arc[i].AdvanceLogic( DeltaTime );
        }
    }
    #endif

    // Time for contagion DOT?
    if( m_ContagionDOTTimer <= 0.0f )
    {
        m_ContagionDOTTimer += k_TimeBetweenContagionTicks;
        ContagionDOT();
    }
}

//==============================================================================

void actor::ContagionDOT( void )
{
//  LOG_MESSAGE( "actor::ContagionDOT", "No effect." );
}

//==============================================================================

void actor::InitContagion( s32 Origin )
{
    (void)Origin;

//  LOG_MESSAGE( "actor::InitContagion", "Origin:%d", Origin );

    f32 Duration = s_TWEAK_ContagionDuration.GetF32();
    if( Duration == 0.0f )
        Duration = 7.5f;

    GetActorEffects(TRUE)->InitEffect( actor_effects::FX_CONTAIGON, this );

#ifndef X_EDITOR
    if( !m_bContagious )
        m_NetDirtyBits |= CONTAGION_ON_BIT;

     if( m_pMPContagion && (m_pMPContagion->Origin != Origin) )
        m_NetDirtyBits |= CONTAGION_ON_BIT;
#endif

    m_bContagious       = TRUE;
    m_ContagionTimer    = Duration;
    m_ContagionDOTTimer = k_TimeBetweenContagionTicks;

#ifndef X_EDITOR

    if( GameMgr.IsGameMultiplayer() )
    {
        if( !m_pMPContagion )
        {
            m_pMPContagion = new mp_contagion;
            ASSERT( m_pMPContagion );

            for( s32 i = 0; i < 32; i++ )
            {
                SMP_UTIL_InitFXFromString( "ContagionTether.fxo", 
                                           m_pMPContagion->Arc[i] );
                ASSERT( m_pMPContagion->Arc[i].Validate() );
                m_pMPContagion->Arc[i].Restart();
                m_pMPContagion->Arc[i].SetSuspended( TRUE );
                m_pMPContagion->Arc[i].SetColor( XCOLOR_WHITE );
            }
            
            m_pMPContagion->PlayerMask = 0x00000000;
        }

        m_pMPContagion->Origin = Origin;
    }

#endif
}

//==============================================================================

void actor::KillMPContagion( void )
{
//  LOG_MESSAGE( "actor::KillMPContagion", "" );

    ASSERT( !m_bContagious );

#ifndef X_EDITOR

    ASSERT( GameMgr.IsGameMultiplayer() );
    if( m_pMPContagion )
    {
        for( s32 i = 0; i < 32; i++ )
        {
            ASSERT( m_pMPContagion->Arc[i].IsFinished() );
            m_pMPContagion->Arc[i].KillInstance();
        }

        delete m_pMPContagion;
        m_pMPContagion = NULL;
    }

#endif
}

//=============================================================================

void actor::RenderContagion( void )
{
    if( !m_bContagious )
        return;

#ifndef X_EDITOR

    if( !m_pMPContagion )
        return;

    ASSERT( !IsCharacter() );
    ASSERT( GameMgr.IsGameMultiplayer() );

    const view& View = *(eng_GetView());

    f32 Radius = s_MPContagion_TouchDistance.GetF32();
    if( Radius == 0.0f )
        Radius = 1000.0f;

    bbox BBox( GetPosition(), Radius );

    if( View.BBoxInView( BBox ) )
    {
        for( s32 i = 0; i < 32; i++ )
        {
            if( View.BBoxInView( m_pMPContagion->Arc[i].GetBounds() ) )
                m_pMPContagion->Arc[i].Render();
        }
    }

#endif
}        

//=============================================================================
// WEAPON FUNCTIONS
//=============================================================================

new_weapon* actor::GetCurrentWeaponPtr( void )
{
    s32 Index = inventory2::ItemToWeaponIndex( m_CurrentWeaponItem );
    return (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[Index] );
}

//==============================================================================

new_weapon* actor::GetWeaponPtr( inven_item WeaponItem )
{
    s32 Index = inventory2::ItemToWeaponIndex( WeaponItem );
    return (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[Index] );
}

//==============================================================================

void actor::MoveWeapon( xbool UpdateWeaponRenderState ) 
{
    //move the weapon using the prop points position... from the character animiation player
    CONTEXT("actor::MoveWeapon");
    
    //
    // move our 3rd person weapon?
    // This is a necessary test to that we don't mess up the player's weapon
    // pullback calculation by putting the weapon in an avatar's hands. So
    // we move the weapon if either we aren't a player, or we're playing split
    // screen.
    //
    const xbool bIsPlayer = (GetType() == object::TYPE_PLAYER);
#ifdef X_EDITOR
    const xbool bIsSplitScreen = FALSE;
#else
    const xbool bIsSplitScreen = (g_NetworkMgr.GetLocalPlayerCount() > 1);
#endif
    const xbool bShouldMove = (!bIsPlayer || (bIsPlayer && bIsSplitScreen));

    if( !bShouldMove || !m_pLoco )
        return;

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if ( !pWeapon || !GetLocoPointer()->IsAnimLoaded() )
        return;

    // Update weapons NPC (3rd person) state, but keep original state afterwards so split screen players work
    new_weapon::render_state RenderState = pWeapon->GetRenderState();
    pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );
    pWeapon->OnTransform( GetLocoPointer()->GetWeaponL2W() );
    pWeapon->SetZone1( GetZone1() );
    pWeapon->SetRenderState( RenderState );

    if( UpdateWeaponRenderState )
        pWeapon->SetRenderState( (new_weapon::render_state)GetWeaponRenderState() );
}

//=============================================================================

void actor::UpdateWeapon( f32 DeltaTime )
{
    CONTEXT( "actor::UpdateWeapon" );

    if( !IsAlive() )
    {
        return;
    }
    
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && ( pWeapon->GetRenderState() != new_weapon::RENDER_STATE_PLAYER ) )
    {
        //advance currently held weapon's logic.
        pWeapon->OnAdvanceLogic( DeltaTime );    
    }
}

//=============================================================================

xbool actor::EquipWeapon2( inven_item WeaponItem )
{
    ASSERT( (WeaponItem >= INVEN_WEAPON_FIRST) && (WeaponItem <= INVEN_WEAPON_LAST) );

    new_weapon* pNewWeapon = GetWeaponPtr( WeaponItem );
    if( pNewWeapon )
    {
        pNewWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_NPC, GetGuid() ); 
        m_CurrentWeaponItem = WeaponItem;
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

void actor::UnequipCurrentWeapon()
{   
    #ifndef X_EDITOR
    ASSERT( GameMgr.GetGameType() == GAME_CAMPAIGN );
    #endif

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->SetVisible(FALSE);
    }
    m_CurrentWeaponItem = INVEN_NULL;
}

//=============================================================================

s32 actor::GetWeaponRenderState( void )
{
    return new_weapon::RENDER_STATE_NPC;
}

//=============================================================================

xbool actor::IsFlashlightOn( void )
{
    return( m_pEffects && m_pEffects->IsEffectOn( actor_effects::FX_FLASHLIGHT ) );
}

//=============================================================================

xbool actor::IsBloodEnabled( void ) const
{
    // TO DO: Add property and detect german version if this needs to be on
    //        a per character basis
    return g_bBloodEnabled;
}

//=============================================================================

void actor::CreateDamageEffects( const pain& Pain, xbool bDoLargeEffects, xbool bDoDebris )
{
    (void)bDoDebris;

    // Skip?
    if( !IsBloodEnabled() )
        return;

    // Create Large Blood Splats
    if( bDoLargeEffects )
    {
        CreateSplatDecalOnWall( Pain );  

        if( IsDead() )
        {
            CreateSplatDecalOnGround();        
        }
    }

    // If pain is a direct hit then use pain position for blood effect
    if( Pain.IsDirectHit() )
    {
        // Create blood impact if blood decals are assigned
        const decal_package* pBloodDecalPackage = m_hBloodDecalPackage.GetPointer();
        if( pBloodDecalPackage )
        {
            // Create blood based on pain type and use color of assigned blood decal group
            particle_emitter::CreateOnPainEffect( Pain, 
                                                  0.0f, 
                                                  particle_emitter::UNINITIALIZED_PARTICLE, 
                                                  pBloodDecalPackage->GetGroupColor( m_BloodDecalGroup ) );
        }                                                  
    }
}

//===========================================================================

void actor::CreateSplatDecalOnGround( void )
{
    // Skip?
    if( !IsBloodEnabled() )
        return;

    decal_package* pPackage = m_hBloodDecalPackage.GetPointer();
    if ( !pPackage )
        return;

    if ( (m_BloodDecalGroup<0) || (m_BloodDecalGroup>=pPackage->GetNGroups()) )
        return;

    s32 nDecalDefs = pPackage->GetNDecalDefs( m_BloodDecalGroup );
    if ( nDecalDefs == 0 )
        return;

    // choose a random decal to paste
    s32 DecalIndex = (nDecalDefs == 1) ? 0 : x_rand() % (nDecalDefs-1);
    decal_definition& DecalDef = pPackage->GetDecalDef( m_BloodDecalGroup, DecalIndex );

    // create a ray that is biased towards the ground
    radian3 BloodOrient;
    BloodOrient.Pitch = x_frand(R_80, R_100);
    BloodOrient.Yaw   = x_frand(R_0,  R_360);
    BloodOrient.Roll  = R_0;
    
    vector3 RayStart = GetPosition();
    RayStart.GetY() += 10.0f;

    vector3 RayEnd( 0.0f, 0.0f, 1.0f );
    RayEnd.Rotate( BloodOrient );
    RayEnd  = 500.0f * RayEnd;
    RayEnd += RayStart;

    // generate the splat decal
    g_DecalMgr.CreateDecalFromRayCast( DecalDef,
                                       RayStart,
                                       RayEnd );
}

//===========================================================================

void actor::CreateSplatDecalOnWall( const pain& Pain )
{
    // Skip?
    if( !IsBloodEnabled() )
        return;

    decal_package* pPackage = m_hBloodDecalPackage.GetPointer();
    if ( !pPackage )
        return;

    if ( (m_BloodDecalGroup<0) || (m_BloodDecalGroup>=pPackage->GetNGroups()) )
        return;

    s32 nDecalDefs = pPackage->GetNDecalDefs( m_BloodDecalGroup );
    if ( nDecalDefs == 0 )
        return;

    // choose a random decal to paste
    s32 DecalIndex = (nDecalDefs == 1) ? 0 : x_rand() % (nDecalDefs-1);
    decal_definition& DecalDef = pPackage->GetDecalDef( m_BloodDecalGroup, DecalIndex );

    // create a ray that is biased towards the ground
    radian3 BloodOrient;
    Pain.GetDirection().GetPitchYaw( BloodOrient.Pitch, BloodOrient.Yaw );
    BloodOrient.Pitch += x_frand( R_0, R_45 );
    BloodOrient.Yaw   += x_frand( -R_30, R_30 );
    BloodOrient.Roll   = R_0;
    
    vector3 RayEnd( 0.0f, 0.0f, 1.0f );
    RayEnd.Rotate( BloodOrient );
    RayEnd  = 500.0f * RayEnd;
    RayEnd += Pain.GetImpactPoint();

    // generate the splat decal
    g_DecalMgr.CreateDecalFromRayCast( DecalDef,
                                       Pain.GetImpactPoint(),
                                       RayEnd );
}

//=============================================================================

loco::anim_type actor::GetDeathAnim( const pain& painThatKilledUs )
{
    general_hit_location    GenLocation = HIT_INVALID;
    death_motion_direction  MoveDir     = DEATH_MOVE_INVALID;
    geom::bone::hit_location hitLocation = GetHitLocation( painThatKilledUs );

//  LOG_MESSAGE( "actor::GetDeathAnim", "Hit location:%s", GetHitLocationName(hitLocation) );

    // How high did we get hit?
    switch( hitLocation)
    {
        case geom::bone::HIT_LOCATION_HEAD:
            GenLocation = HIT_HIGH;
            break;

        case geom::bone::HIT_LOCATION_SHOULDER_RIGHT:
        case geom::bone::HIT_LOCATION_SHOULDER_LEFT:
        case geom::bone::HIT_LOCATION_TORSO:
            GenLocation = HIT_MIDDLE;
            break;

        case geom::bone::HIT_LOCATION_LEGS:
            GenLocation = HIT_LOW;
            break;

        default:
            GenLocation = HIT_MIDDLE;
            break;
           
    }

    // Since middle is so common do a random chance of high or low
    if( GenLocation == HIT_MIDDLE )
    {
        s32 I = x_irand(0,100);
        if( I < 50 )
        {
            if( I<25 )
                GenLocation = HIT_LOW;
            else
                GenLocation = HIT_HIGH;
        }
    }
    
    // Get the facing and force direction
    radian FaceYaw = GetLocoPointer()->m_Player.GetFacingYaw();
    radian ForceYaw = painThatKilledUs.GetForceDirection().GetYaw();
    radian DeltaYaw = x_MinAngleDiff(FaceYaw, ForceYaw);    
    if (ABS(DeltaYaw) > R_90)
    {
        MoveDir = DEATH_MOVE_BACK;     
    }
    else
    {
        MoveDir = DEATH_MOVE_FORWARD;
    }

    // What animation do we play?
    loco::anim_type AnimType = loco::ANIM_NULL ;
    eHitType HitType = GetHitType(painThatKilledUs);
    loco::move_style MoveStyle = GetLocoPointer()->GetMoveStyle();

    if (  (HitType == HITTYPE_LIGHT) &&
          ((MoveStyle == loco::MOVE_STYLE_CROUCH) || (MoveStyle == loco::MOVE_STYLE_CROUCHAIM) ) && 
          (GetLocoPointer()->GetAnimIndex(loco::ANIM_DEATH_CROUCH) != -1) )
    {
        //light hits while crouching cause us to play crouching death
        AnimType = loco::ANIM_DEATH_CROUCH;
    }
    else
    {
        switch( GenLocation )
        {
            case HIT_HIGH:
            {
                switch( MoveDir )
                {
                    case DEATH_MOVE_FORWARD:
                        switch( HitType )
                        {
                        case HITTYPE_HARD:                      
                            AnimType = loco::ANIM_DEATH_HARD_SHOT_IN_BACK_HIGH;
                            break;
                        case HITTYPE_LIGHT:                      
                            AnimType = loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_HIGH;
                            break;
                        }
                        break;
                    case DEATH_MOVE_BACK:
                        switch( HitType )
                        {
                        case HITTYPE_HARD:                      
                            AnimType = loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_HIGH;
                            break;
                        case HITTYPE_LIGHT:                      
                            AnimType = loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_HIGH;
                            break;
                        }
                        break;
                    default:
                        break;
                }
            }
            break;

            case HIT_MIDDLE:
            {
                switch( MoveDir )
                {
                    case DEATH_MOVE_FORWARD:
                        switch( HitType )
                        {
                        case HITTYPE_HARD:                      
                            AnimType = loco::ANIM_DEATH_HARD_SHOT_IN_BACK_MED;
                            break;
                        case HITTYPE_LIGHT:                      
                            AnimType = loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_MED;
                            break;
                        }
                        break;
                    case DEATH_MOVE_BACK:
                        switch( HitType )
                        {
                        case HITTYPE_HARD:                      
                            AnimType = loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_MED;
                            break;
                        case HITTYPE_LIGHT:                      
                            AnimType = loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_MED;
                            break;
                        }
                        break;
                    default:
                        break;
                }
            }
            break;

            case HIT_LOW:
            {
                switch( MoveDir )
                {
                    case DEATH_MOVE_FORWARD:
                        switch( HitType )
                        {
                        case HITTYPE_HARD:                      
                            AnimType = loco::ANIM_DEATH_HARD_SHOT_IN_BACK_LOW;
                            break;
                        case HITTYPE_LIGHT:                      
                            AnimType = loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_LOW;
                            break;
                        }
                        break;
                    case DEATH_MOVE_BACK:
                        switch( HitType )
                        {
                        case HITTYPE_HARD:                      
                            AnimType = loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_LOW;
                            break;
                        case HITTYPE_LIGHT:                      
                            AnimType = loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_LOW;
                            break;
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        }
    }
    return AnimType;
}

//=============================================================================
// ANIMATION RELATED FUNCTIONS
//=============================================================================

// Takes care of all anim events
void actor::SendAnimEvents( void )
{
    s32 i ;

    // Any loco?
    if (!GetLocoPointer())
        return;
    
    g_EventMgr.HandleSuperEvents( GetLocoPointer()->m_Player, this );
    g_EventMgr.HandleSuperEvents( GetLocoPointer()->m_Player, GetLocoPointer()->GetMaskController(), this );
    g_EventMgr.HandleSuperEvents( GetLocoPointer()->m_Player, GetLocoPointer()->GetAdditiveController(ANIM_FLAG_IMPACT_CONTROLLER), this );
    g_EventMgr.HandleSuperEvents( GetLocoPointer()->m_Player, GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER), this );

    // Check main anim controller events
    for ( i = 0; i < GetLocoPointer()->m_Player.GetNEvents(); i++)
    {
        // Send this event?
        if (GetLocoPointer()->m_Player.IsEventActive(i))
        {
            // Lookup event and world position
            const anim_event& Event = GetLocoPointer()->m_Player.GetEvent(i);
            vector3           Pos   = GetLocoPointer()->m_Player.GetEventPosition(i);

            // Call event handler
            OnAnimEvent(Event, Pos);
        }
    }

    // OLD EVENT SYSTEM:
    // TO DO - Remove when all events are updated to super events!
    // Check reload shoot anim controller events
    for ( i = 0; i < GetLocoPointer()->GetMaskController().GetNEvents(); i++)
    {
        // Send this event?
        if (GetLocoPointer()->GetMaskController().IsEventActive(i))
        {
            // Lookup event
            const anim_event& Event = GetLocoPointer()->GetMaskController().GetEvent(i) ;
    
            // Compute world pos
            const matrix4& BoneM = GetLocoPointer()->m_Player.GetBoneL2W( Event.GetInt( anim_event::INT_IDX_BONE ) );
                  vector3  Pos   = BoneM * Event.GetPoint( anim_event::POINT_IDX_OFFSET );

            // Call event handler
            OnAnimEvent(Event, Pos);
        }
    }

    // OLD EVENT SYSTEM:
    // TO DO - Remove when all events are updated to super events!
    // Check reload shoot anim controller events
    for ( i = 0; i < GetLocoPointer()->GetAdditiveController(ANIM_FLAG_IMPACT_CONTROLLER).GetNEvents(); i++)
    {
        // Send this event?
        if (GetLocoPointer()->GetAdditiveController(ANIM_FLAG_IMPACT_CONTROLLER).IsEventActive(i))
        {
            // Lookup event
            const anim_event& Event = GetLocoPointer()->GetAdditiveController(ANIM_FLAG_IMPACT_CONTROLLER).GetEvent(i) ;
    
            // Compute world pos
            const matrix4& BoneM = GetLocoPointer()->m_Player.GetBoneL2W( Event.GetInt( anim_event::INT_IDX_BONE ) );
                  vector3  Pos   = BoneM * Event.GetPoint( anim_event::POINT_IDX_OFFSET );

            // Call event handler
            OnAnimEvent(Event, Pos);
        }
    }

    // OLD EVENT SYSTEM:
    // TO DO - Remove when all events are updated to super events!
    // Check reload shoot anim controller events
    for ( i = 0; i < GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).GetNEvents(); i++)
    {
        // Send this event?
        if (GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).IsEventActive(i))
        {
            // Lookup event
            const anim_event& Event = GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).GetEvent(i) ;
    
            // Compute world pos
            const matrix4& BoneM = GetLocoPointer()->m_Player.GetBoneL2W( Event.GetInt( anim_event::INT_IDX_BONE ) );
                  vector3  Pos   = BoneM * Event.GetPoint( anim_event::POINT_IDX_OFFSET );

            // Call event handler
            OnAnimEvent(Event, Pos);
        }
    }
}

//=============================================================================

xbool actor::OnAnimEvent( const anim_event& Event, const vector3& WorldPos )
{
    // TO DO - Move weapon firing into here?

    (void)Event;
    (void)WorldPos;

    return FALSE;
}

//=============================================================================

#ifndef X_EDITOR

void actor::DoMultiplayerPainEffects( const pain& Pain )
{
    // Should only happen in multi-player
    ASSERT( GameMgr.IsGameMultiplayer() );

    // Play flinch.  
    PlayFlinch( Pain );

    // Create all the effects.
    CreateDamageEffects( Pain, TRUE, TRUE );

    // Crate audio
    PlayPainSfx();
}

#endif

//=============================================================================

void actor::PlayFlinch( const pain& Pain )
{
    // get the hit type. 

    eHitType HitType = GetHitType(Pain);

    xbool isIdle = ( GetLocoPointer()->GetState() == loco::STATE_IDLE );
    object *painSource = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );


    // If playing a full body lip sync cinema, then ignore pain
    if( IsPlayingFullBodyCinema() )
    {
        HitType = HITTYPE_LIGHT;
    }
    // first player melee always plays special stun
    else if( Pain.GetHitType() == 6 && HasAnim(loco::ANIM_DAMAGE_PLAYER_MELEE_0))
    {
    //  LOG_MESSAGE( "actor::PlayFlinch", "HITTYPE_PLAYER_MELEE_1" );
        HitType = HITTYPE_PLAYER_MELEE_1;
    }
    // damage from a friendly player is always light
    else if( painSource &&
             painSource->IsKindOf(player::GetRTTI()) &&
             IsFriendlyFaction(GetFactionForGuid(Pain.GetOriginGuid())) )
    {
        HitType = HITTYPE_LIGHT;
    }
    // lights can become full body idles if we are idle.
    else if( HitType == HITTYPE_LIGHT )
    {
        // convert up to a full body hit
        if( isIdle && 
            x_GetTimeSec() - m_BigPainTakenTime >= k_MinTimeBetweenBigHits &&
            HasAnim(loco::ANIM_PAIN_IDLE_FRONT) )
        {
        //  LOG_MESSAGE( "actor::PlayFlinch", "HITTYPE_LIGHT->HITTYPE_IDLE" );
            HitType = HITTYPE_IDLE;
        }
        else
        {
        //  LOG_MESSAGE( "actor::PlayFlinch", "HITTYPE_LIGHT" );
        }
    }
    else
    {
        // downgrade to light if timer to low.
        if( (x_GetTimeSec() - m_BigPainTakenTime < k_MinTimeBetweenBigHits) ||
            IsDead() )
        {
        //  LOG_MESSAGE("actor::PlayFlinch","HITTYPE_BIG->HITTYPE_LIGHT");
            HitType = HITTYPE_LIGHT;
        }
        else
        {
        //  LOG_MESSAGE("actor::PlayFlinch","HITTYPE_BIG");
        }
    }

    // place to put any special hit_type override code.
    HitType = OverrideFlinchType( HitType );

    // play an impact anim.
    if( !IgnoreFlinches() )
    {    
        PlayImpactAnim(Pain, HitType);
    }
    else
    {
    //  LOG_MESSAGE("actor::PlayFlinch","IgnoreFlinches()==TRUE");
    }
}

//=============================================================================

void actor::PlayPainSfx( void )
{
    // Trigger impact sfx immediately
    if( IsDead() )
    {
        // Play after this delay
        m_PainSfxDelay = x_frand( 0.2f, 0.25f );
    }
    else
    {
        // Only play grunt if interval has finished
        if( m_PainSfxInterval == 0.0f )
        {
            // Play after this delay
            m_PainSfxDelay = x_frand( 0.1f, 0.2f );
            
            // Wait this time before playing another grunt
            m_PainSfxInterval = x_frand( 0.2f, 0.4f );
        }    
    }
}

//=============================================================================

void actor::UpdatePainSfx( f32 DeltaTime )
{
    // Update interval
    m_PainSfxInterval = x_max( m_PainSfxInterval - DeltaTime, 0.0f );

    // Waiting to play a pain?
    if( m_PainSfxDelay > 0.0f )
    {
        // Update delay
        m_PainSfxDelay = x_max( m_PainSfxDelay - DeltaTime, 0.0f );

        // Trigger the sfx?
        if( m_PainSfxDelay == 0.0f )
        {
            // Stop current pain grunt
            g_AudioMgr.Release( m_VoiceID, 0.0f );

#ifndef X_EDITOR
            // Lookup audio strings
            const char* pPrefix = GameMgr.GetSkinAudioPrefix( m_Net.Skin, m_PreferedVoiceActor, m_bIsMutated );
            const char* pType = IsDead() ? "DEATH" : "PAINGRUNT";
                            
            // Is this player local?
            if( g_NetworkMgr.IsLocal( net_GetSlot() ) )
            {
                // 2d sound
                m_VoiceID = g_AudioMgr.Play( xfs( "%s_%s", pPrefix, pType ) );
            }
            else
            {
                // 3d sound
                m_VoiceID = g_AudioMgr.PlayVolumeClipped( xfs( "%s_%s", pPrefix, pType ), GetPosition(), GetZone1(), TRUE );
            }            
#endif            
        }        
    }
}

//=============================================================================

#if !defined(CONFIG_RETAIL)
void actor::DebugSuicide( void )
{
    m_CorpseDeathPain.Clear();
    pain_handle PainHandle("GENERIC_LETHAL");
    pain Pain;
    Pain.Setup( "GENERIC_LETHAL", 0, GetPosition() );
    Pain.SetDirectHitGuid( GetGuid() );
    Pain.ApplyToObject( this );
}
#endif

//=============================================================================

xbool actor::TakeDamage( const pain& Pain )
{            
    ASSERT( Pain.ComputeDamageAndForceCalled() );
    //--------------------------------------------------------------------------
    #ifndef X_EDITOR
    //--------------------------------------------------------------------------
    // If this pain originated from a ghost (that is, a player on another 
    // machine on the network), then ignore the pain.  Except for the two
    // special cases of Contagion DOT.
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
        if( (pObject) && 
            (pObject->GetType() == object::TYPE_NET_GHOST) &&
            (Pain.GetHitType() !=  98) &&  //  98 = Contagion DOT to self.
            (Pain.GetHitType() != 100) )   // 100 = Contagion DOT to other.
        {
            m_TimeSinceLastPain = 0.0f;
            m_PainThatKilledUs = Pain;
            return( FALSE );
        }
    }
    //--------------------------------------------------------------------------
    #endif // X_EDITOR
    //--------------------------------------------------------------------------

    xbool bIsPlayer = IsKindOf(player::GetRTTI());

    // Invincible?
    if( m_bCanDie == FALSE )
    {
        // if this isn't a player, just don't let them take damage at all
        if( !bIsPlayer )
        {
            return( FALSE );
        }
    }

    f32 Damage = Pain.GetDamage();

    Damage = ModifyDamageByDifficulty(Damage);

    if ( InTurret() )
    {
        Damage = ModifyDamageByTurret( Damage );
    }

#if defined( USE_LEAN_PAIN_REDUCTION ) // mreed: this is placeholder, and may be introduced eventually 9/1/04
    if ( bIsPlayer && (x_abs( m_LeanAmount ) > 0) )
    {
        tweak_handle LeanPainPct("PLAYER_Lean_Pain_Percent");
        Damage *= LeanPainPct.GetF32();
    }
#endif

    // Update health.
    m_Health.Sub( Damage, !bIsPlayer );
    #ifndef X_EDITOR
    m_NetDirtyBits |= HEALTH_BIT;  // Network
    #endif

    // Dead?
    if( m_Health.GetHealth() <= 0.0f )
    {
        // if we are a player and are invulnerable and we are about to die... refill health
        if( (m_bCanDie == FALSE) && bIsPlayer )
        {
            m_Health.Add(GetMaxHealth());
        }
        else
        {
            m_TimeSinceLastPain = 0.0f;
            m_PainThatKilledUs = Pain;
            OnDeath(); // You are DEAD!
            Damage = 1000.0f;    // "Spike" the pain for the net kill.
        }
    }

    //----------------------------------------------------------------------
    #ifndef X_EDITOR
    //----------------------------------------------------------------------
    {
        s32 Origin = -1;
        object* pObject = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
        if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
        {
            actor* pActor = (actor*)pObject;
            Origin = pActor->net_GetSlot();
        }

        // Notify the game logic.  This gives kill credit in multiplayer.
        if( IsDead() && g_NetworkMgr.IsServer() )
        {  
            pGameLogic->PlayerDied( m_NetSlot, Origin, Pain.GetHitType() );
        }

        // We shouldn't be here if this pain did not originate locally.
        // When on the client, locally generated pain must be sent to the server
        // via the PainQueue as net pain.
        if( g_NetworkMgr.IsClient() )
        {
            net_ReportNetPain( m_NetSlot, 
                               Origin, 
                               Pain.GetHitType(),
                               m_Net.LifeSeq,
                               IsDead(),
                               Damage );
        }
        else
        {
            // Special case: contagion infection!
            if( Pain.GetHitType() == 101 )
            {
            //  LOG_MESSAGE( "actor::TakeDamage", "InitContagion" );
                InitContagion( Origin );
            }
        }
    }
    //----------------------------------------------------------------------
    #endif
    //----------------------------------------------------------------------

    // Damage taken
    return TRUE;
}

//==============================================================================

actor::eHitType actor::GetHitType( const pain& Pain )
{
    // Get HitType as specified in the tweak tables
    s32 HT = Pain.GetHitType();
    switch( HT )
    {
    case 0:     return HITTYPE_LIGHT;
    case 1:     return HITTYPE_HARD;
    case 3:     return HITTYPE_HARD;
    case 4:     return HITTYPE_HARD;
    case 2:
        {
            if( Pain.GetOriginGuid() )
            {
                object* pOriginOfPain = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
                if( pOriginOfPain )
                {
                    tweak_handle DistTweak("DistForCharacterHitType2");
                    vector3 vToTarget = pOriginOfPain->GetPosition() - GetPosition();
                    if (vToTarget.LengthSquared() <= x_sqr(DistTweak.GetF32()))
                    {
                        return character::HITTYPE_HARD;
                    }
                }
            }
            return HITTYPE_LIGHT;
        }
    default:
         return HITTYPE_LIGHT;
    }
}

//=============================================================================

void actor::UpdateCloak( f32 DeltaTime )
{
    // are we allowed to cloak?
    if( !m_CanCloak )
    {
        Decloak();
        return;
    }

    // are we forced to be cloaked all the time?
    if( m_MustCloak )
    {
        Cloak();
        return;
    }

    // update the shield status for a cloaked guy
    if( (m_CloakState == CLOAKING_ON) && (m_CloakShieldPainTimer > 0.0f) )
    {
        m_CloakShieldPainTimer -= DeltaTime;
        if ( m_CloakShieldPainTimer < 0.0f )
            m_CloakShieldPainTimer = 0.0f;
    }
    
    // update the transition state
    if( (m_CloakState == CLOAKING_TURNING_ON) ||
        (m_CloakState == CLOAKING_TURNING_OFF) )
    {
        m_CloakTransitionTime += DeltaTime;
        if( m_CloakTransitionTime > k_CloakTransitionTime )
        {
            if( m_CloakState == CLOAKING_TURNING_ON )
                m_CloakState = CLOAKING_ON;
            else
                m_CloakState = CLOAKING_OFF;
        }
        else
        {
            // add a light flash to draw the user's attention
            g_LightMgr.AddFadingLight( GetPosition(), xcolor(57, 230, 246), 200.0f, 3.0f * m_CloakTransitionTime / k_CloakTransitionTime, 0.1f );
        }
    }
}

//=============================================================================

void actor::Cloak( void )
{
    // if we are already cloaked, then nothing to do
    if( (m_CloakState == CLOAKING_ON) ||
        (m_CloakState == CLOAKING_TURNING_ON) )
    {
        return;
    }

    // we need to transition into cloaked
    m_CloakTransitionTime = 0.0f;
    m_CloakState          = CLOAKING_TURNING_ON;

    // play a cloaking sound effect
    g_AudioMgr.Play("Cloak_On",GetPosition(),GetZone1(),TRUE);
    m_CloakVoiceID = g_AudioMgr.Play("Cloak_Loop");

    // kick off the particle effect
    actor_effects* pActorEffects = GetActorEffects( TRUE );
    if( pActorEffects )
    {
        pActorEffects->InitEffect( actor_effects::FX_CLOAK, this );
    }
}

//=============================================================================

void actor::Decloak( void )
{
    // if we are already uncloaked, then nothing to do
    if( (m_CloakState == CLOAKING_OFF) ||
        (m_CloakState == CLOAKING_TURNING_OFF) )
    {
        return;
    }

    // we need to transition into not cloaked
    m_CloakTransitionTime = 0.0f;
    m_CloakState          = CLOAKING_TURNING_OFF;

    // play a decloaking sound effect
    g_AudioMgr.Play("Cloak_Off",GetPosition(),GetZone1(),TRUE);
    g_AudioMgr.Release(m_CloakVoiceID,0.0f);
    m_CloakVoiceID = 0;

    // kick off the particle effect
    actor_effects* pActorEffects = GetActorEffects( TRUE );
    if( pActorEffects )
    {
        pActorEffects->InitEffect( actor_effects::FX_DECLOAK, this );
    }
}

//=============================================================================

void actor::WakeUpDoors( void )
{
    xbool bLockedDoors = FALSE;
    
    // Dead men wake no doors.
    if( IsDead() )
    {
        return;
    }
    
    //
    // Doors.
    //
    {
        s32 ObjectSlot;

#ifndef X_EDITOR
        u32 TeamBits = net_GetTeamBits();
#else
        u32 TeamBits = 0;
#endif
        
        // Scan the box for objects.
        for( ObjectSlot = g_ObjMgr.GetFirst( object::TYPE_DOOR );
             ObjectSlot != SLOT_NULL; 
             ObjectSlot = g_ObjMgr.GetNext( ObjectSlot ) )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( ObjectSlot );
            if( pObj )
            {
                door* pDoor = (door*)pObj;

                bbox BBox = GetBBox();

                if( !pDoor->UsesProxBox() )
                {
                    BBox.Inflate( 350.0f, 50.0f, 350.0f );
                }

#ifndef X_EDITOR   
                // Inflate the net_ghost BB a little more so that players can't take 
                // advantage of small anomalies to open the door only on their client.
                if( IsKindOf( net_ghost::GetRTTI() ) ) 
                {
                    BBox.Inflate( 5.0f, 5.0f, 5.0f );
                }
#endif
                
                if( pDoor->GetDoorBBox().Intersect( BBox ) )
                {
                    u32 CircuitBits = pDoor->GetCircuit().GetTeamBits();
#ifndef X_EDITOR
                    // Wake up the door if it's open so it 
                    // doesn't close on somebody's head.
                    if( pDoor->GetState() != door::CLOSED )
                    {
                        pDoor->WakeUp();
                    }

                    // Team based check.
                    else if( (CircuitBits & TeamBits) )
                    {
                        u16 Zone1 = pDoor->GetZone1();
                        u16 Zone2 = pDoor->GetZone2();
                        xbool bZone1Unlocked = !GameMgr.IsZoneLocked( Zone1 );
                        xbool bZone2Unlocked = !GameMgr.IsZoneLocked( Zone2 );
                        
                        // Zone based check.
                        if( bZone1Unlocked && bZone2Unlocked )
                        {
                            pDoor->WakeUp();
                        }
                        else 
                        {
                            bLockedDoors = TRUE;                               
                        }
                    }

                    else if( GameMgr.IsGameMultiplayer() )
                    {
                        bLockedDoors = TRUE;   
                    }

                    // Let the editor and campaign do their own thing.
                    else if( !GameMgr.IsGameMultiplayer() )
#endif
                    {
                        pDoor->WakeUp();
                    }

                }
            }
        }
    }


    // Second check, to make buzzer sound.
    if( bLockedDoors && IsKindOf( player::GetRTTI() ) )
    {
        s32 ObjectSlot;

#ifndef X_EDITOR
        u32 TeamBits = net_GetTeamBits();
#else
        u32 TeamBits = 0;
#endif
        
        bLockedDoors = FALSE;

        // Scan the box for objects.
        for( ObjectSlot = g_ObjMgr.GetFirst( object::TYPE_DOOR );
            ObjectSlot != SLOT_NULL; 
            ObjectSlot = g_ObjMgr.GetNext( ObjectSlot ) )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( ObjectSlot );
            if( pObj )
            {
                door* pDoor = (door*)pObj;

                bbox BBox = GetBBox();

                if( !pDoor->UsesProxBox() )
                {
                    BBox.Inflate( 25.0f, 0.0f, 25.0f );
                }

                if( pDoor->GetDoorBBox().Intersect( BBox ) )
                {
                    u32 CircuitBits = pDoor->GetCircuit().GetTeamBits();
#ifndef X_EDITOR
                    // Wake up the door if it's open so it 
                    // doesn't close on somebody's head.
                    if( pDoor->GetState() != door::CLOSED )
                    {
                        ;
                    }

                    // Team based check.
                    else if( (CircuitBits & TeamBits) )
                    {
                        u16 Zone1 = pDoor->GetZone1();
                        u16 Zone2 = pDoor->GetZone2();
                        xbool bZone1Unlocked = !GameMgr.IsZoneLocked( Zone1 );
                        xbool bZone2Unlocked = !GameMgr.IsZoneLocked( Zone2 );

                        // Zone based check.
                        if( bZone1Unlocked && bZone2Unlocked )
                        {
                            ;
                        }
                        else 
                        {
                            bLockedDoors = TRUE;                               
                        }
                    }

                    else if( GameMgr.IsGameMultiplayer() )
                    {
                        bLockedDoors = TRUE;   
                    }
#endif
                }
            }
        }
    }

    //
    // Forcefields.
    //
    {
        s32 ObjectSlot;

#ifndef X_EDITOR
        u32 TeamBits = net_GetTeamBits();
#else
        u32 TeamBits = 0;
#endif

        // Scan the box for objects.
        for( ObjectSlot = g_ObjMgr.GetFirst( object::TYPE_FORCE_FIELD );
            ObjectSlot != SLOT_NULL; 
            ObjectSlot = g_ObjMgr.GetNext( ObjectSlot ) )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( ObjectSlot );

            if( pObj )
            {
                force_field* pField = (force_field*)pObj;

                bbox BBox = GetBBox();

                BBox.Inflate( 350.0f, 50.0f, 350.0f );

#ifndef X_EDITOR   
                // Inflate the net_ghost BB a little more so that players can't take 
                // advantage of small anomalies to open the door only on their client.
                if( IsKindOf( net_ghost::GetRTTI() ) ) 
                {
                    BBox.Inflate( 5.0f, 5.0f, 5.0f );
                }
#endif

                if( pField->GetBBox().Intersect( BBox ) )
                {
                    u32 CircuitBits = pField->GetCircuit().GetTeamBits();
#ifndef X_EDITOR
                    // Wake up the field if it's open so it 
                    // doesn't close with somebody inside of it.
                    if( pField->GetState() != force_field::STATE_CLOSED )
                    {
                        pField->Open();
                    }

                    // Team based check.
                    else if( (CircuitBits & TeamBits) )
                    {

                        // Zone based check.
                        if( !GameMgr.IsZoneLocked( pField->GetZone1() ) && 
                            !GameMgr.IsZoneLocked( pField->GetZone2() ) )
                        {
                            pField->Open();
                        }
                    }

                    // Let the editor and campaign do their own thing.
                    else if( !GameMgr.IsGameMultiplayer() )
#endif
                    {
                        pField->Open();
                    }
                }
            }
        }
    }

#ifndef X_EDITOR
    if( g_NetworkMgr.IsOnline() && 
        IsKindOf( player::GetRTTI() ) )
    {
        if( bLockedDoors && !m_bLockedDoors )
        {
            if( ((player*)this)->InvalidSound() )
            {
                MsgMgr.Message( MSG_DOOR_LOCKED_PPZ_TOO_FEW, net_GetSlot() );
            }
        }
    }
#endif

    m_bLockedDoors = bLockedDoors;
}

//=============================================================================

void actor::Push( const vector3& PushVector )
{
    GetLocoPointer()->m_Physics.SetPosition( GetPosition() );
    GetLocoPointer()->m_Physics.Push(PushVector);
    OnMove( GetLocoPointer()->m_Physics.GetPosition() );
}

//========================================================================

f32 actor::GetCollisionHeight( void )
{
    if ( m_pLoco )
    {
        return m_pLoco->m_Physics.GetColHeight();
    }
    else
    {
        return 0.0f;
    }
}

//========================================================================

f32 actor::GetCollisionRadius( void )
{
    if ( m_pLoco )
    {
        return m_pLoco->m_Physics.GetColRadius();
    }
    else
    {
        return 0.0f;
    }
}

//=============================================================================

void  actor::SetIsActive( xbool bIsActive )
{
    if( m_bIsActive == bIsActive )
        return;

    if( bIsActive )
    {
        m_bIsActive = TRUE;
        m_nActive++;

        m_pNextActive = m_pFirstActive;
        m_pPrevActive = NULL;
        if( m_pFirstActive ) m_pFirstActive->m_pPrevActive = this;
        m_pFirstActive = this;
    }
    else
    {
        m_bIsActive = FALSE;
        m_nActive--;

        if( m_pNextActive ) m_pNextActive->m_pPrevActive = m_pPrevActive;
        if( m_pPrevActive ) m_pPrevActive->m_pNextActive = m_pNextActive;
        if( m_pFirstActive == this )
            m_pFirstActive = m_pNextActive;

        m_pNextActive = NULL;
        m_pPrevActive = NULL;
    }

}

//=============================================================================

//=============================================================================

void actor::EnumAttachPoints( xstring& String ) const
{
    String = "BaseObject~"; 
    
    s32 i;

    if (m_pLoco)
    {
        s32 nBones = m_pLoco->m_Player.GetNBones();
        
        for (i=0;i<nBones;i++)
        {
            const anim_bone& Bone = m_pLoco->m_Player.GetBone( i );

            String += Bone.Name;
            String += "~";
        }        
    }

    for( i=0; String[i]; i++ )
    {
        if( String[i] == '~' )
            String[i] = 0;
    }
}

//=============================================================================

s32 actor::GetAttachPointIDByName( const char* pName ) const
{
    if (x_stricmp(pName,"BaseObject")==0)
        return 0;
    
    if (m_pLoco)
    {
        s32 nBones = m_pLoco->m_Player.GetNBones();
        s32 i;
        for (i=0;i<nBones;i++)
        {
            const anim_bone& Bone = m_pLoco->m_Player.GetBone( i );

            if (x_stricmp( pName, Bone.Name ) == 0)
            {
                return i+1;
            }            
        }        
    }


    return -1;
}

//=============================================================================

xstring actor::GetAttachPointNameByID( s32 iAttachPt ) const
{
    if (iAttachPt == 0)
        return xstring("BaseObject");
    

    if (m_pLoco)
    {
        // Decrement by one to bring it into the range [0,nbones)
        iAttachPt -= 1;
        s32 nBones = m_pLoco->m_Player.GetNBones();
        if ( (iAttachPt >= 0) &&
             (iAttachPt < nBones ))
        {
            const anim_bone& Bone = m_pLoco->m_Player.GetBone( iAttachPt );

            return Bone.Name;
        }        
    }

    return xstring("INVALID\0");
}

//=============================================================================

void actor::OnAttachedMove( s32             iAttachPt,
                            const matrix4&  L2W )
{
    if (iAttachPt == 0)
    {                
        OnTransform( L2W );
    }
    else
    if (m_pLoco)
    {
        // Decrement by one to bring it into the range [0,nbones)
        iAttachPt -= 1;
        s32 nBones = m_pLoco->m_Player.GetNBones();
        if ( (iAttachPt >= 0) &&
             (iAttachPt < nBones ))
        {
            matrix4 BoneL2W = m_pLoco->m_Player.GetBoneL2W( iAttachPt );            
            BoneL2W.PreTranslate( m_pLoco->m_Player.GetBoneBindPosition( iAttachPt ) );

            vector3 BonePos = BoneL2W.GetTranslation();
            BonePos -= GetPosition();

            matrix4 NewL2W = L2W;
            NewL2W.Translate( -BonePos );

            OnTransform( NewL2W );
        }        
    }

}

//=============================================================================

xbool actor::GetAttachPointData( s32      iAttachPt,
                                  matrix4& L2W,
                                  u32      Flags )
{
    if (iAttachPt == 0)
    {
        L2W = GetL2W();
        return TRUE;
    }
    if (m_pLoco)
    {
        // Decrement by one to bring it into the range [0,nbones)
        iAttachPt -= 1;
        s32 nBones = m_pLoco->m_Player.GetNBones();
        if ( (iAttachPt >= 0) &&
             (iAttachPt < nBones ))
        {
            L2W = m_pLoco->m_Player.GetBoneL2W( iAttachPt );
            if ( Flags & ATTACH_USE_WORLDSPACE)
                L2W.PreTranslate( m_pLoco->m_Player.GetBoneBindPosition( iAttachPt ) );
            
            return TRUE;
        }        
    }

    return FALSE;
}

//=============================================================================
// RENDER FUNCTIONS
//=============================================================================

const matrix4* actor::GetBonesForRender( u64 LODMask, s32& nActiveBones )
{
    s32 i;

    // Lookup loco
    loco* pLoco = GetLocoPointer();
    if( !pLoco )
        return NULL;
        
    // SB 6/26/03 -
    // Multi-view optimization: Compute the nActiveBones for all views and keep the max count

    // Compute # of bones that the skin rendering will need
    skin_inst& SkinInst = GetSkinInst();
    nActiveBones = SkinInst.GetNActiveBones( LODMask );

    // SB: 2/22/05
    // Always include the weapon bone for correct positioning
    nActiveBones = x_max( nActiveBones, pLoco->GetWeaponBoneIndex() + 1 );

#ifndef X_EDITOR

    // SB: 2/22/05
    // For MP "capture the flag", always compute the flag bone for correct positioning
    nActiveBones = x_max( nActiveBones, pLoco->GetFlagBoneIndex() + 1 );

#endif

    // Should never have more active bones than the geometry - if so the anim package bind pose is a
    // different hierarchy than the geometry bind pose file
    #ifdef X_ASSERT
    skin_geom* pSkinGeom = SkinInst.GetSkinGeom();
    ASSERTS( !pSkinGeom || nActiveBones <= pSkinGeom->m_nBones, "Skin bones counts are messed up - see Steve!") ;
    #endif

    // No bones?!
    if( nActiveBones == 0 )
        return NULL ;

    // Use loco to compute bones?
    const matrix4* pMatrices = NULL;
    if( pLoco->IsAnimLoaded() )
    {
        // Tell the animation player the # of bones to compute
        pLoco->m_Player.SetNActiveBones( nActiveBones );

        // Compute matrices
        ASSERT( pLoco->GetNActiveBones() == nActiveBones );
        pMatrices = pLoco->ComputeL2W();
    }

    // If animation not loaded, just render with bind pose 
    // (this can only happen in the editor when an anim is not yet assigned to a new actor)
    if ( pMatrices == NULL )
    {
        // Allocate matrices
        matrix4* pMat = (matrix4*)smem_BufferAlloc( nActiveBones * sizeof( matrix4 ) );
        if( !pMat )
            return NULL;

        // Get object transform
        const matrix4& L2W = GetL2W();
        
        // Use bind pose at current transform
        for( i = 0; i < nActiveBones ; i++ )
            pMat[i] = L2W;

        pMatrices = pMat;
    }

    return pMatrices;
}

//=============================================================================
//  RenderHitLocations
//  
//      Render hit locations for debugging purposes.  Renders the spheres for
//      collisions for the characters
//=============================================================================

#if !defined( CONFIG_RETAIL )

void actor::RenderHitLocations( void )
{
    // Lookup geometry
    skin_inst& SkinInst = GetSkinInst();
    skin_geom* pSkinGeom = SkinInst.GetSkinGeom();
    if( !pSkinGeom )
        return;

    // Compute LOD mask
    u64 LODMask = SkinInst.GetLODMask(GetL2W());
    if ( LODMask == 0 )
        return;

    // Compute bones
    s32 nActiveBones = 0;
    const matrix4* pMatrices = GetBonesForRender( LODMask, nActiveBones );
    if ( !pMatrices )
        return;

    // Render all bone bboxes
    s32 i;
    for (i=0;i<nActiveBones;i++)
    {
        // Skip?
        geom::bone::hit_location HitLoc = (geom::bone::hit_location)pSkinGeom->m_pBone[i].HitLocation;
        if( HitLoc == geom::bone::HIT_LOCATION_UNKNOWN )
            continue;
    
        // Select color
        xcolor Clr = XCOLOR_YELLOW;
        switch ( HitLoc )
        {
        case geom::bone::HIT_LOCATION_HEAD:
            Clr = xcolor(0,0,255);
            break;
        case geom::bone::HIT_LOCATION_SHOULDER_LEFT:
            Clr = xcolor(255,255,255);
            break;
        case geom::bone::HIT_LOCATION_SHOULDER_RIGHT:
            Clr = xcolor(255,0,0);
            break;
        case geom::bone::HIT_LOCATION_TORSO:
            Clr.Random();
            break;
        case geom::bone::HIT_LOCATION_LEGS:
            Clr = xcolor(255,0,255);
            break;
        }

        // Lookup bone info
        const bbox&    LocalBBox = pSkinGeom->m_pBone[i].BBox;
        const matrix4& L2W       = pMatrices[i];
        
        // Draw bone aligned bbox
        draw_SetL2W( L2W );
        draw_BBox( LocalBBox, Clr );
    }
    draw_ClearL2W();            
}

#endif // !defined( CONFIG_RETAIL )

//=============================================================================

void actor::ResetRidingPlatforms( void )
{
    if( GetLocoPointer() )
    {
        GetLocoPointer()->m_Physics.ResetRidingPlatforms();
    }
}

//=============================================================================

void actor::SetSkinVMesh( xbool Mutant )
{
// We should only be doing this in multi-player!
#ifdef X_EDITOR
    
    // Do nothing in the editor since it's always single player...
    (void)Mutant;
    return;

#else

    // Exit if this is a single player game otherwise characters literally loose their heads!
    if( GameMgr.IsGameMultiplayer() == FALSE )
        return;

    // Update skin mesh
    virtual_mesh_mask VMeshMask = m_SkinInst.GetVMeshMask();
    u32 Mask = VMeshMask.VMeshMask;
    //static const u32 EvenBits = 1 | 4 | 16 | 64;  // normal bits
    static const u32  OddBits = 2 | 8 | 32 | 128; // mutated bits

    const xbool IsMutated = Mask & OddBits;

    if ( Mutant && !IsMutated )
    {
        Mask *= 2;
    }
    else if ( !Mutant && IsMutated )
    {
        Mask /= 2;
    }

    m_SkinInst.SetVMeshMask( Mask );
    
#endif  //#ifdef X_EDITOR
}

//=============================================================================

void actor::SetAvatarMutationState( avatar_mutation_state State )
{
    if ( State != m_AvatarMutationState )
    {
        m_AvatarMutationState = State;

        if ( !m_pEffects )
        {
            GetActorEffects( TRUE );
        }

        switch ( m_AvatarMutationState )
        {
        case AVATAR_NORMAL:
            m_TimeLeftInAvatarMutationState = F32_MAX;
            break;

        case AVATAR_NORMALING:
            {
                m_TimeLeftInAvatarMutationState = g_AvatarTweaks.m_NormalingTime;
                if( !m_bDead ) m_pEffects->InitEffect( actor_effects::FX_UNMUTATE, this );
            }
            break;

        case AVATAR_MUTATING:
            {
                m_TimeLeftInAvatarMutationState = g_AvatarTweaks.m_MutatingTime;
                if( !m_bDead ) m_pEffects->InitEffect( actor_effects::FX_MUTATE, this );
            }
            break;

        case AVATAR_MUTANT:
            m_TimeLeftInAvatarMutationState = F32_MAX;
            break;  

        default:
            ASSERT( FALSE );
            break;
        }
        
#ifndef X_EDITOR        
        // Send across net
        m_NetDirtyBits |= MUTATE_BIT;
#endif            
    }
}

//=============================================================================

static const f32 TIME_IN_MUTATING_STATE = 0.0f;
static const f32 TIME_IN_NORMALING_STATE = 0.0f;

void actor::UpdateAvatarMutation( f32 DeltaTime )
{
    m_TimeLeftInAvatarMutationState -= DeltaTime;

    switch ( m_AvatarMutationState )
    {
    case AVATAR_NORMAL:
        
        // Show normal skin
        SetSkinVMesh( FALSE );
        
        // Switch to mutate?
        if( m_bIsMutated )
        {
            SetAvatarMutationState( AVATAR_MUTATING );
        }
        break;
        
    case AVATAR_MUTATING:
        if ( m_TimeLeftInAvatarMutationState <= 0.0f )
        {
            m_AvatarMutationState           = AVATAR_MUTANT;
            m_TimeLeftInAvatarMutationState = TIME_IN_MUTATING_STATE;
        }
        break;

    case AVATAR_MUTANT:
    
        // Show mutant skin
        SetSkinVMesh( TRUE );
    
        // Switch to normal?
        if( !m_bIsMutated )
        {
            SetAvatarMutationState( AVATAR_NORMALING );
        }
        break;

    case AVATAR_NORMALING:
        if ( m_TimeLeftInAvatarMutationState <= 0.0f )
        {
            m_AvatarMutationState           = AVATAR_NORMAL;
            m_TimeLeftInAvatarMutationState = TIME_IN_NORMALING_STATE;
        }
        break;

    default:
        ASSERTS( FALSE, xfs( "Invalid avatar mutation state: %d", m_AvatarMutationState ) );
    }

}

//=============================================================================
 
void actor::PrepPlayerAvatar( void )
{
    m_hAudioPackage.SetName( "AI_Multiplayer_Player.audiopkg" );

    m_SkinInst.SetUpSkinGeom( PRELOAD_MP_FILE( "MP_AVATAR_BIND.skingeom" ) );

    m_hAnimGroup.SetName( PRELOAD_MP_FILE( "MP_AVATAR.anim" ) );

    m_hBloodDecalPackage.SetName( PRELOAD_MP_FILE( "Blood.decalpkg" ) );
    m_BloodDecalGroup = 0;  // 0 = Human, 1 = Gray, 2 = Blackops

#ifndef X_EDITOR
    // If these fire off, your data needs building...
    ASSERTS( m_hAudioPackage.GetPointer(),  "You need to build AI_Multiplayer_Player.audiopkg" );
    ASSERTS( m_SkinInst.GetGeom(),          "You need to build MP_AVATAR_BIND.skingeom" );
    ASSERTS( m_hAnimGroup.GetPointer(),     "You need to build MP_AVATAR.anim" );
#endif
    
    if( ( m_SkinInst.GetGeom() ) && ( m_hAnimGroup.GetPointer() ) && ( m_pLoco ) )
    {
        // Tell loco
        m_pLoco->OnInit( m_SkinInst.GetGeom(), m_hAnimGroup.GetName(), GetGuid() );

        InitLoco();
    }
}

//=============================================================================

xbool actor::IsAlly( const actor* pActor ) const
{
#ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
        return( (m_NetTeamBits & pActor->net_GetTeamBits()) != 0 );
    else
#endif
        return( IsFriendlyFaction( pActor->GetFaction() ) );
}

//=============================================================================

xbool actor::IsEnemy( const actor* pActor ) const
{
    (void) pActor;
    ASSERTS(0, "OBSOLETE - Please use IsEnemyFaction()");
    return TRUE;

/* KSS -- I don't believe this is used anymore
#ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
        return( (m_NetTeamBits & pActor->net_GetTeamBits()) == 0 );
    else
#endif
        return( IsEnemyFaction( pActor->GetFaction() ) );
*/
}

//=============================================================================

f32 actor::ModifyDamageByDifficulty( f32 Damage )
{
    // if this is campaign mode, do health stuff
#ifndef X_EDITOR
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        // make sure the one taking damage is a player before we scale the damage
        if( IsKindOf( player::GetRTTI()) )
        {
            tweak_handle DamageScalarTweak( xfs( "%s_Damage_%s", 
                                                GetLogicalName(), 
                                                DifficultyText[g_Difficulty] ) );

            f32 DamageScalar = 0.0f;

            if( DamageScalarTweak.Exists() )
            {
                DamageScalar = DamageScalarTweak.GetF32();
            }
            else
            {
                // missing tweak for this type, just load in default/generic damage modifier
                tweak_handle DefaultDamageScalarTweak( xfs( "GENERIC_Damage_%s", 
                                                            DifficultyText[g_Difficulty] ) );

                DamageScalar = DefaultDamageScalarTweak.GetF32();
            }

            // Scale damage based on difficulty level
            // the scalar could be +/- and is a whole percentage i.e. -20
            f32 NewDamage = Damage + ( Damage * (DamageScalar/100.0f) );  

            return NewDamage;
        }
    }
#endif
 
    return Damage;
}

f32 actor::ModifyDamageByTurret( f32 Damage )
{
#if 1
    s_AlamoTurretDamagePct.GetF32();
    
    s32 LevelID = g_ActiveConfig.GetLevelID();
    f32 DamageModifier = 1.0f;
    if( LevelID == LEVELID_THE_LAST_STAND )
    {
        DamageModifier = s_AlamoTurretDamagePct.GetF32();
    }
    else if( ( LevelID == LEVELID_BURIED_SECRETS ) || ( LevelID == LEVELID_NOW_BOARDING ) )
    {
        DamageModifier = s_ExcavationTurretDamagePct.GetF32();
    }

    Damage *= DamageModifier;

    return Damage;
#else
    (void)Damage;
    // for precert, we'll be invulnerable in turrets.
    return( 0.0f );
#endif
}

//=============================================================================

xbool actor::SetMutated( xbool bMutate )
{ 
//  LOG_MESSAGE( "actor::SetMutated", "Mutate:%d", bMutate );

    m_bIsMutated = bMutate;

    #ifndef X_EDITOR
    m_NetDirtyBits |= MUTATION_FLAGS_BIT;
    #endif

    return TRUE; 
}

//=============================================================================

void actor::SetupMutationChange( xbool bMutate )
{
//  LOG_MESSAGE( "actor::SetupMutationChange", "Mutate:%d", bMutate );

    const xbool bWasMutated = m_bIsMutated;
    SetMutated( bMutate );
    const xbool bIsMutated = m_bIsMutated;

    if( (bWasMutated != bIsMutated) && GetActorEffects( TRUE ) && m_pEffects )
    {
        if( bMutate )
            m_pEffects->InitEffect( actor_effects::FX_MUTATE, this );
        else
            m_pEffects->InitEffect( actor_effects::FX_UNMUTATE, this );
    }
}

//=============================================================================

void actor::ForceMutationChange( xbool bMutate )
{
    #ifndef X_EDITOR
    ASSERT( GameMgr.GetGameType() != GAME_CAMPAIGN );
    #endif

//  LOG_MESSAGE( "actor::ForceMutationChange", "Mutate:%d", bMutate );

    SetMutated      ( bMutate );
    SetSkinVMesh    ( bMutate );

    if( bMutate )
    {
        #ifndef X_EDITOR
        net_EquipWeapon2( INVEN_WEAPON_MUTATION );
        #endif
    }
}

//=============================================================================

void actor::SetCanToggleMutation( xbool bCanToggleMutation ) 
{ 
    m_bCanToggleMutation = bCanToggleMutation; 

    #ifndef X_EDITOR
    m_NetDirtyBits |= MUTATION_FLAGS_BIT;
    #endif
}

//=============================================================================

void actor::SetMutagenBurnMode( mutagen_burn_mode MutagenBurnMode ) 
{ 
    m_MutagenBurnMode = MutagenBurnMode;
}

//=============================================================================

vector3 actor::GetBonePos( s32 BoneIndex )
{ 
    return m_pLoco->m_Player.GetBonePosition(BoneIndex); 
}

//=============================================================================

void actor::UpdateInactive( f32 DeltaTime )
{
    m_InactiveTime += DeltaTime;

    vector3 Movement = GetPosition() - m_RecentPosition;
    if( Movement.LengthSquared() > 250000.0f )
    {
        m_RecentPosition = GetPosition();
        m_InactiveTime   = 0.0f;
    }
}

//=============================================================================
