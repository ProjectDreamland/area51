//==============================================================================
//
//  Ghost.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Ghost.hpp"

#include "Objects\Event.hpp"
#include "Objects\DeadBody.hpp"
#include "Objects\GrenadeProjectile.hpp"
#include "Objects\GravChargeProjectile.hpp"

#include "Ragdoll\Ragdoll.hpp"
#include "GameLib\StatsMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Sound\EventSoundEmitter.hpp"

#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\NetObjMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"       

//==============================================================================
//  TYPES
//==============================================================================

struct ghost_desc : public object_desc
{
    ghost_desc( void )
     :  object_desc( object::TYPE_GHOST,              
                     "ghost",                        
                     "ACTOR",                         
                     object::ATTR_PLAYER            |
                     object::ATTR_NEEDS_LOGIC_TIME  |
                     object::ATTR_COLLIDABLE        |
                     object::ATTR_RENDERABLE        |
                     object::ATTR_SPACIAL_ENTRY     |
                     object::ATTR_DAMAGEABLE        |
                     object::ATTR_LIVING            |
                     object::ATTR_TRANSPARENT,
                     FLAGS_IS_DYNAMIC )            
    {
        // Nothing to do in here.
    }

    virtual object* Create( void ) 
    { 
        return( new ghost );
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

static ghost_desc s_Ghost;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& ghost::GetTypeDesc( void ) const
{
    return( s_Ghost );
}

//==============================================================================

const object_desc& ghost::GetObjectType( void )
{
    return( s_Ghost );
}

//==============================================================================

ghost::ghost( void )
 :  m_Mutagen                   ( 75.0f ),
    m_MaxMutagen                ( 100.0f ),
    m_MaxHealth                 ( 300.0f ),
    m_EyesPosition              ( 0.0f, 0.0f, 0.0f ),
    m_EyesPitch                 ( 0.0f ),
    m_EyesYaw                   ( 0.0f ),
    m_Pitch                     ( 0.0f ),
    m_Yaw                       ( 0.0f ),
    m_bRenderSkeleton           ( FALSE ),
    m_bRenderSkeletonNames      ( TRUE ),
    m_bRenderBBox               ( TRUE ),
    m_bIsCrouching              ( FALSE ),
    m_fCrouchChangeRate         ( 10.0f ),
    m_pCurrentWeapon            ( NULL ),
    m_CurrentVirtualWeapon      ( WEAPON_UNDEFINED ),
    m_CurrentWeaponObj          ( WEAPON_UNDEFINED_OBJ ),
    m_PreviousVirtualWeapon     ( WEAPON_UNDEFINED ),
    m_NextVirtualWeapon         ( WEAPON_UNDEFINED ),
    m_CurrentStrain             ( STRAIN_HUMAN ),
    m_NextStrain                ( STRAIN_INVALID ),
    m_bJustLanded               ( FALSE ),
    m_DeltaPos                  ( 0.0f, 0.0f, 0.0f ),
    m_DeltaTime                 ( 0.0f )
{
    s32 i;

    m_Faction     = FACTION_PLAYER_NORMAL;
    m_FriendFlags = FACTION_WORKERS;   

    for( i = 0; i < WEAPON_MAX_OBJ; i++ )
    {
        m_GuidWeaponArray[i] = NULL;
    }

    x_memset( m_bWeaponInInventory, FALSE, sizeof( xbool ) * WEAPON_VIRTUAL_MAX );

    // Start out DEAD.  Then spawn.
    m_Health       = 0.0f;
    m_bDead        = TRUE;
    m_bWantToSpawn = TRUE;

    #ifndef X_EDITOR
    m_Net.LifeSeq = -1;
    #endif

    // Setup pointer to loco for base class to use.
    m_pLoco = &m_Loco;
    m_pLoco->SetGhostMode( TRUE );  // Player controls movement, not animations
}

//==============================================================================

ghost::~ghost( void )
{
}

//==============================================================================

vector3 ghost::GetPositionWithOffset( eOffsetPos offset )
{
    switch( offset ) 
    {
    case OFFSET_NONE:
        return GetPosition();
        break;
    case OFFSET_CENTER:
        return GetBBox().GetCenter();
        break;
    case OFFSET_AIM_AT:
        return GetBBox().GetCenter();
        break;
    case OFFSET_EYES:
        return GetEyesPosition();
        break;
    case OFFSET_TOP_OF_BBOX:
        return GetPosition() + vector3( 0.0f, GetBBox().Max.GetY(), 0.0f );
        break;
    default:
        return GetPosition();
        break;
    }
}

//==============================================================================

vector3 ghost::GetEyesPosition( void ) const
{
    return m_EyesPosition;
}

//==============================================================================

void ghost::GetEyesPitchYaw( radian& Pitch, radian& Yaw ) const
{
    Pitch   = m_EyesPitch;
    Yaw     = m_EyesYaw;
}

//==============================================================================

radian3 ghost::GetProjectileTrajectory( void )
{
    return radian3( m_Pitch, m_Yaw, 0.0f );
}

//==============================================================================
/*
void ghost::OnReset( void )
{
    m_Health = m_MaxHealth;
}
*/

//==============================================================================
xbool ghost::AddMutagen( const f32& nDeltaMutagen )
{        
    // do not allow Mutagen to go above max.
    if( m_Mutagen == m_MaxMutagen && nDeltaMutagen > 0.0f )
    {
        return FALSE;
    }
    else if( (m_Mutagen + nDeltaMutagen) < 0.0f )  // does what we are using take us below 0?
    {
        // don't have enough
        return false;
    }
    else
    {
        // add/subtract mutagen
        m_Mutagen = fMin( m_Mutagen + nDeltaMutagen , m_MaxMutagen );
        m_Mutagen = fMax( m_Mutagen , 0.0f );

        return TRUE;
    }

    return FALSE;
}

//==============================================================================

void ghost::OnKill( void )
{
}

//==============================================================================

const matrix4& ghost::GetL2W( void ) const
{
    matrix4& L2W = *(matrix4*)(&actor::GetL2W()); // de-constification
    const vector3 Pos( GetPosition() );
    L2W.Identity();
    L2W.RotateY( m_Yaw );
    L2W.Translate( Pos );
    return L2W;
}

//==============================================================================

const char* ghost::GetVirtualWeaponName( player_virtual_weapon VirtualWeapon )
{
    switch( VirtualWeapon )
    {
        case MACHINE_GUN:   return "SMP";           break;
        case SHOTGUN:       return "Shotgun";       break;
        case GAUSS_RIFLE:   return "Gauss Rifle";   break;
        case SNIPER_RIFLE:  return "Sniper Rifle";  break;
        case DESERT_EAGLE:  return "Desert Eagle";  break;
        case MHG:           return "MHG";           break;
        case MSN:           return "MSN";           break;
        case DUAL_SMP:      return "Dual SMP";      break;
        case MUTATION:      return "Mutation";      break;
        case MISC_01:       return "Misc";          break;
        default:            return "Unknown";       break;
    }
}
    
//==============================================================================

const char* ghost::GetWeaponObjName( player_weapon_obj WeaponObj )
{
    switch( WeaponObj )
    {
        case MACHINE_GUN_OBJ:   return "SMP";           break;
        case SHOTGUN_OBJ:       return "Shotgun";       break;
        case GAUSS_RIFLE_OBJ:   return "Gauss Rifle";   break;
        case SNIPER_RIFLE_OBJ:  return "Sniper Rifle";  break;
        case DESERT_EAGLE_OBJ:  return "Desert Eagle";  break;
        case MHG_OBJ:           return "MHG";           break;
        case MSN_OBJ:           return "MSN";           break;
        case MUTATION_OBJ:      return "Mutation";      break;
        default:                return "Unknown";       break;
    }
}

//==============================================================================

void ghost::LogWeapons( void )
{
#if defined(X_LOGGING)
    s32 i;
    LOG_MESSAGE( "ghost::LogWeapons", "*** Virtual Weapons" );
    for( i=0 ; i<WEAPON_VIRTUAL_MAX ; i++ )
    {
        const char* pWeaponName = GetVirtualWeaponName( (player_virtual_weapon)i );
        const char* pHas        = "No ";
        if( m_bWeaponInInventory[ i ] )
            pHas = "Yes";
        LOG_MESSAGE( "ghost::LogWeapons", "  %02d: %s [%s]", i, pHas, pWeaponName ); 
    }

    LOG_MESSAGE( "ghost::LogWeapons", "*** Weapon Object GUIDS" );
    for( i=0 ; i<WEAPON_MAX_OBJ ; i++ )
    {
        const char* pHas           = "No ";
        const char* pWeaponName    = GetWeaponObjName( (player_weapon_obj)i );
        s32         lo             = m_GuidWeaponArray[ i ].GetLow();
        s32         hi             = m_GuidWeaponArray[ i ].GetHigh();
        s32         primary_ammo   = 0;
        s32         secondary_ammo = 0;

        if( m_GuidWeaponArray[ i ] )
        {
            pHas = "Yes";
            object* pObj = g_ObjMgr.GetObjectByGuid( m_GuidWeaponArray[ i ] );
            ASSERT( pObj );
            if( !pObj )
            {
                continue;
            }
            
            new_weapon& WeaponItem = new_weapon::GetSafeType( *pObj );
            primary_ammo   = WeaponItem.m_WeaponAmmo[0].m_AmmoAmount;
            secondary_ammo = WeaponItem.m_WeaponAmmo[1].m_AmmoAmount;
        }

        LOG_MESSAGE( "ghost::LogWeapons", 
                     "  %02d: %s PrimaryAmmo: %03d, SecondaryAmmo: %03d, GUID: %08x:%08x [%s]", 
                     i, 
                     pHas, 
                     primary_ammo, 
                     secondary_ammo, 
                     (s32)hi, 
                     (s32)lo, 
                     pWeaponName ); 
    }

    LOG_MESSAGE( "ghost::LogWeapons", "*** Weapon Inventory" );
    m_WeaponInventory.LogInventory();
#endif
}

//==============================================================================
// OLD PLAYER_ALL_STRAINS STUFF
//==============================================================================

static xbool s_DumpWeapons = FALSE;

void ghost::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "ghost::OnAdvanceLogic" );
    LOG_STAT(k_stats_Player);

    m_DeltaTime = DeltaTime;

    if( s_DumpWeapons )
    {
        LogWeapons();
        s_DumpWeapons = 0;
    }

    // Store a pointer to the current weapon.  This pointer is only valid 
    // this frame and can only be used in my methods, not engine overloads.  
    // Must be very careful with this.
    m_pCurrentWeapon = GetCurrentWeaponPtr();

	// Check to see if our current weapon is still in our inventory
	if ( m_pCurrentWeapon 
		&& (m_WeaponInventory.GetItemTypeCount( m_pCurrentWeapon->GetInvType()) < 1 ) )
	{
		// we no longer have the weapon, drop it

		player_virtual_weapon WeaponType = GetWeaponStateFromType( m_pCurrentWeapon->GetType() );
		m_bWeaponInInventory[WeaponType] = FALSE;
	    m_GuidWeaponArray[GetWeaponObjFromVirtual( WeaponType )]   = NULL;
		m_pCurrentWeapon = NULL;
	}

    // SB - Added to keep the weapon in sync with the player hands
    //      (player::OnAdvanceLogic now updates the weapon)
    if (( m_pCurrentWeapon ) && ( m_pCurrentWeapon->GetRenderState() != new_weapon::RENDER_STATE_PLAYER ))
    {
        m_pCurrentWeapon->OnAdvanceLogic( DeltaTime );
    }


    actor::OnAdvanceLogic( DeltaTime );    

    //invalidate the pointer to the current weapon.
    m_pCurrentWeapon = NULL;

    if( !m_bDead )
    {
        //
        // Update loco
        //
        radian3 Rot;
        Rot.Roll  = R_0;
        Rot.Pitch = m_Pitch;
        Rot.Yaw   = m_Yaw;
        vector3 P = GetPosition();
        vector3 Velocity( m_Physics[m_CurrentStrain].GetVelocity() );
        P.GetY() += 180.0f;
        m_pLoco->SetLookAt( P + (vector3( Rot ) * 100.0f) );
        m_pLoco->SetMoveAt( P + (Velocity * 100.0f) );

        /* Attempt to play the animation at a speed which looks "good".
        {   
            loco_motion_controller LMC = m_pLoco->GetMotionController();
            f32 Speed  = LMC.GetMovementSpeed();
            f32 Vel    = Velocity.Length();
            f32 Factor = 1.0f;

            if( !IN_RANGE( -0.001, Vel, 0.001f ) )
                Factor = Speed / Vel;

            LMC.SetRate( Factor );
        }
        */

        const f32 SpeedSquared = Velocity.LengthSquared();
        if( SpeedSquared <= 0.0f )
        {
            m_pLoco->SetState( loco::STATE_IDLE );
        }
        else
        {
            m_pLoco->SetState( loco::STATE_MOVE );
        }

        if( m_bIsCrouching && m_pLoco->GetMoveStyle() != loco::MOVE_STYLE_CROUCH )
        {
            m_pLoco->SetMoveStyle( loco::MOVE_STYLE_CROUCH );
        }
        else if( !m_bIsCrouching && m_pLoco->GetMoveStyle() != loco::MOVE_STYLE_RUNAIM )
        {
            m_pLoco->SetMoveStyle( loco::MOVE_STYLE_RUNAIM );
        }
    }


    if( m_Physics[m_CurrentStrain].GetFallMode() )
    {
        m_bFalling = TRUE; // used for pain when we land
    }
    else if( m_bFalling )
    {
        // We've just landed
        m_bJustLanded = TRUE;
        // Hurt?
        TakeFallPain();
    }

    // Handled by player
    WakeUpDoors();
}

//==============================================================================
/*
void ghost::OnDeath( void )
{
    m_Health = m_MaxHealth;

    #ifndef X_EDITOR
    m_NetDirtyBits |= HEALTH_BIT;
    #endif // X_EDITOR
}
*/
//==============================================================================

ghost::player_weapon_obj ghost::GetWeaponObjFromVirtual( player_virtual_weapon VirtualWeapon )
{
    switch( VirtualWeapon )
    {
        case MACHINE_GUN:   return MACHINE_GUN_OBJ;

// KSS -- TO ADD NEW WEAPON
        case SHOTGUN:       return SHOTGUN_OBJ;

        case GAUSS_RIFLE:   return GAUSS_RIFLE_OBJ;
        case SNIPER_RIFLE:  return SNIPER_RIFLE_OBJ;
        case DESERT_EAGLE:  return DESERT_EAGLE_OBJ;
        case MHG:           return MHG_OBJ;     
        case MSN:           return MSN_OBJ;
        case DUAL_SMP:      return MACHINE_GUN_OBJ;
        case MUTATION:      return MUTATION_OBJ;

        default:            return WEAPON_UNDEFINED_OBJ;
    }
}

//==============================================================================

s32 ghost::GetRenderIndexFromPlayer( void )
{
    if( m_CurrentStrain == STRAIN_HUMAN )
    {
        if( ( m_CurrentVirtualWeapon == DUAL_SMP ) )
            return WEAPON_STRAIN_HUMAN_DUAL;
        else
            return WEAPON_STRAIN_HUMAN;
    }
    else
    {
        if( ( m_CurrentVirtualWeapon == DUAL_SMP ) )
            return WEAPON_STRAIN_MUTANT_DUAL;
        else
            return WEAPON_STRAIN_MUTANT;
    }
}

//==============================================================================

inventory_item::inv_type ghost::GetInventoryType( player_virtual_weapon VirtualWeapon )
{
    switch( VirtualWeapon )
    {
        case MACHINE_GUN:   return inventory_item::INV_WPN_SMP;
        
// KSS -- TO ADD NEW WEAPON
        case SHOTGUN:       return inventory_item::INV_WPN_SHOTGUN;

        case GAUSS_RIFLE:   return inventory_item::INV_WPN_GAUSS;
        case SNIPER_RIFLE:  return inventory_item::INV_WPN_SNIPER;
        case DESERT_EAGLE:  return inventory_item::INV_WPN_DESERT_EAGLE;
        case MHG:           return inventory_item::INV_WPN_MHG;     
        case MSN:           return inventory_item::INV_WPN_MSN;     
        case DUAL_SMP:      return inventory_item::INV_WPN_SMP;
        case MUTATION:      return inventory_item::INV_WPN_MUTATION;

        default:            return inventory_item::INV_INVALID;
    }
}

//==============================================================================

void ghost::OnAliveLogic( f32 DeltaTime )
{   
    // Let physics keep track of riding on platform
    m_Physics[m_CurrentStrain].CatchUpWithRidingPlatform( DeltaTime );
    m_Physics[m_CurrentStrain].WatchForRidingPlatform();

    UpdateMovement( DeltaTime );

    // Call base class
    actor::OnAliveLogic( DeltaTime );
}

//==============================================================================

void ghost::OnDeathLogic( f32 DeltaTime )
{
    // Keep physics going for death while falling.
    m_Physics[m_CurrentStrain].Advance( m_Physics[m_CurrentStrain].GetPosition(), DeltaTime );

    OnMove( m_Physics[m_CurrentStrain].GetPosition() );

    if( m_pLoco && m_pLoco->IsPlayAnimComplete() )
    {
        CreateDeadBody();
    }

    // Call base class
    actor::OnDeathLogic( DeltaTime );
}

//==============================================================================

void ghost::OnMove( const vector3& rNewPos )
{
    CONTEXT( "ghost::OnMove" );

    if( GetAttrBits() & object::ATTR_DESTROY )
        return;

    m_DeltaPos = rNewPos - GetPosition();

    // HACKOMOTRON - 
    //
    // Problem: When the ghost spawns or is created, he is created essentially 
    // at the origin and then moved to his starting point.  The above 
    // computation for m_DeltaPos results in a large vector which is, in turn,
    // used by the physics the first time the ghost runs his logic.  The poor
    // ghost then proceeds to collide with several walls.
    //
    // Proper solution:
    // (1) The ghost should not run physics or collision.
    // (2) Upon creation or spawning, the ghost should not attempt to travel
    //     long distances in the first frame following.
    //
    // HACK: If m_DeltaPos is too large, set it to 0.

    if( m_DeltaPos.LengthSquared() > (500*500) )  // 5 meters squared
        m_DeltaPos.Zero();

    actor::OnMove( rNewPos );

    m_Physics[m_CurrentStrain].SetPosition( rNewPos );
}

//==============================================================================

void ghost::OnTransform( const matrix4& L2W )
{
    actor::OnTransform(L2W);
    
    //update physics
    m_Physics[m_CurrentStrain].SetPosition( L2W.GetTranslation() );
}

//=============================================================================
//==============================================================================

void ghost::UpdateMovement( f32 DeltaTime )
{
    m_Physics[m_CurrentStrain].Advance( 
            m_Physics[m_CurrentStrain].GetPosition() + m_DeltaPos, 
            DeltaTime );
}

//==============================================================================

void ghost::OnColCheck( void )
{
    m_Physics[m_CurrentStrain].ApplyCollision();
}

//==============================================================================

#ifndef X_RETAIL
void ghost::OnColRender( xbool bRenderHigh )
{    
    (void)bRenderHigh;
    m_Physics[m_CurrentStrain].RenderCollision();
} 
#endif // X_RETAIL

//==============================================================================

ghost::player_virtual_weapon ghost::GetWeaponStateFromType( object::type Type )
{
    switch ( Type )
    {
        case TYPE_WEAPON_DESERT_EAGLE:
        {
            return DESERT_EAGLE;
        }        
        case TYPE_WEAPON_SMP:
        {
            if ( m_WeaponInventory.GetItemTypeCount( inventory_item::INV_WPN_SMP ) > 1 )
            {
                return DUAL_SMP;
            }
            else
            {
                return MACHINE_GUN;
            }
        }

// KSS -- TO ADD NEW WEAPON
        case TYPE_WEAPON_SHOTGUN:
        {
            return SHOTGUN;            
        }
        case TYPE_WEAPON_SNIPER:
        {
            return SNIPER_RIFLE;
        }
        case TYPE_WEAPON_GAUSS:
        {
            return GAUSS_RIFLE;
        }
        case TYPE_WEAPON_MHG:
        {
            return MHG;
        }
        case TYPE_WEAPON_MSN:
        {
            return MSN;
        }
        case TYPE_WEAPON_MUTATION:
        {
            return MUTATION;
        }

        default:
            return WEAPON_UNDEFINED;
            break;
    }
}

//==============================================================================

void ghost::AddNewWeapon( guid WeaponGuid )
{
    xtimer AddNewWeaponTimer;
    AddNewWeaponTimer.Start();

    object_ptr<new_weapon> WeaponObj( WeaponGuid );
    ASSERT( WeaponObj.IsValid() );

    player_virtual_weapon WeaponType = GetWeaponStateFromType( WeaponObj.m_pObject->GetType() );

    // Coudn't find the weapon?
    if (WeaponType == WEAPON_UNDEFINED)
    {
        x_DebugMsg( "ghost::AddNewWeapon %f\n", AddNewWeaponTimer.StopSec() );
        return;
    }

    // Already have the weapon?
    if ( m_bWeaponInInventory[WeaponType] )
    {
        x_DebugMsg( "ghost::AddNewWeapon %f\n", AddNewWeaponTimer.StopSec() );
        return;
    }
    
    player_weapon_obj PlayerWeaponObj     = GetWeaponObjFromVirtual( WeaponType );
    m_GuidWeaponArray[PlayerWeaponObj]    = WeaponGuid;
    m_bWeaponInInventory[WeaponType]      = TRUE;
    
    if( m_CurrentVirtualWeapon == WEAPON_UNDEFINED )
    {
        m_CurrentVirtualWeapon = WeaponType;
        m_CurrentWeaponObj = PlayerWeaponObj;

        // Set the render index.
        object_ptr<new_weapon> WeaponObj( m_GuidWeaponArray[m_CurrentWeaponObj] );
        
        if ( WeaponObj.IsValid() )
        {
            WeaponObj.m_pObject->SetCurrentRenderIndex( GetRenderIndexFromPlayer() );

            // Set the current weapon pointer.
            m_pCurrentWeapon = WeaponObj.m_pObject;
        }
    }

    // We are trying to switch to the dual instance of the same weapon.
    if( PlayerWeaponObj == m_CurrentWeaponObj )
    {
        
        m_NextVirtualWeapon = WeaponType;

        object_ptr<new_weapon> WeaponObj(m_GuidWeaponArray[m_CurrentWeaponObj]);
        if ( WeaponObj.IsValid() )
        {
            // Set the current weapon pointer.
            m_pCurrentWeapon = WeaponObj.m_pObject;
        }
    }

    x_DebugMsg( "ghost::AddNewWeapon %f\n", AddNewWeaponTimer.StopSec() );
}

//==============================================================================

void ghost::SwitchWeapon( new_weapon* pWeapon )
{
    if( GetWeaponStateFromType( pWeapon->GetType() ) != m_CurrentVirtualWeapon )
    {
        m_NextVirtualWeapon = GetWeaponStateFromType( pWeapon->GetType() );
    }
}

f32 ghost::GetMovementNoiseLevel()
{
    const xbool bIsMoving = (m_Physics[m_CurrentStrain].GetVelocity()).LengthSquared() > 0.0f;
    if( bIsMoving && m_pLoco )
    {
        switch( m_pLoco->GetMoveStyle() )
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

//==============================================================================

radian ghost::GetSightYaw( void ) const
{
    return m_EyesYaw;
}

//==============================================================================

void ghost::UpdateFellFromAltitude( void )
{
    // Use the current position if we have moved upwards or are not falling
    f32 Altitude = GetPosition().GetY();
    if ( (Altitude > m_FellFromAltitude)
        || !m_bFalling )
    {
        m_FellFromAltitude = Altitude;
    }
}

//==============================================================================

void ghost::TakeFallPain( void )
{
    m_bFalling = FALSE;

    const f32 CurrentAltitude = GetPosition().GetY();
    const f32 FallAltitude = m_FellFromAltitude - CurrentAltitude;

    if ( FallAltitude > m_SafeFallAltitude )
    {
        
        // y=mx+b --> x = (y-b) / m   x is damage, and y is altitude :)
        f32 m = (m_DeathFallAltitude - m_SafeFallAltitude) / m_MaxHealth ; 
        f32 Damage = (FallAltitude - m_SafeFallAltitude) / m;

        pain PainEvent;

        PainEvent.Type      = pain::TYPE_GENERIC;
        PainEvent.Center    = GetPosition();
        PainEvent.Origin    = GetGuid();
        PainEvent.PtOfImpact= GetPosition();
        PainEvent.Direction.Set( 0.0f, 1.0f, 0.0f );
        PainEvent.DamageR0  = Damage;
        PainEvent.DamageR1  = 0.0f;
        PainEvent.ForceR0   = 0.0f;
        PainEvent.ForceR1   = 0.0f;
        PainEvent.RadiusR0  = 0.0f;
        PainEvent.RadiusR1  = 200.0f;

        OnPain( PainEvent );

        m_FellFromAltitude = GetPosition().GetY();
    }
}

//==============================================================================

f32 ghost::GetCollisionHeight( void )
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

f32 ghost::GetCollisionRadius( void )
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

//========================================================================

void ghost::OnEvent( const event& Event )
{
    (void)Event;
}
//==============================================================================

s32 ghost::GetWeaponRenderState( void )
{
    return new_weapon::RENDER_STATE_NPC;
}

//==============================================================================
