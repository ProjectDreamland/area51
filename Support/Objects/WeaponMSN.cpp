//=========================================================================
// WEAPON MESON CANNON
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileEnergy.hpp"
#include "ProjectileMesonSeeker.hpp"
#include "WeaponMSN.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\Projector.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "render\LightMgr.hpp"
#include "Player.hpp"
#include "GravChargeProjectile.hpp"
#include "Characters\character.hpp"

#if !defined(X_EDITOR)
#include "NetworkMgr/NetworkMgr.hpp"
#endif
#include "Gamelib/DebugCheats.hpp"

//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================
f32 CHARGEUP_START_SCALE        = 1.5f;
f32 CHARGEUP_SCALE_PER_SECOND   = 1.2f;

f32 s_MSNLockonBeepDelay            = 1.0f;

tweak_handle GainSecondsTweak ( "MESON_Secondary_GainSeconds" );
tweak_handle BurnSecondsTweak ( "MESON_Secondary_BurnSeconds" );

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_msn_desc : public object_desc
{

    weapon_msn_desc( void ) : object_desc( object::TYPE_WEAPON_MSN, 
                                        "Meson Cannon",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
										object::ATTR_NEEDS_LOGIC_TIME		|
                                        object::ATTR_SOUND_SOURCE			|
                                        object::ATTR_RENDERABLE				|
 										object::ATTR_COLLISION_PERMEABLE    ,
                                        FLAGS_IS_DYNAMIC
                                        //| FLAGS_GENERIC_EDITOR_CREATE                                                                       
										)
    {

    }

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_msn;
    }

} s_Weapon_Msn_Desc;

//=========================================================================

const object_desc&  weapon_msn::GetTypeDesc     ( void ) const
{
    return s_Weapon_Msn_Desc;
}

//=========================================================================

const object_desc&  weapon_msn::GetObjectType   ( void )
{
    return s_Weapon_Msn_Desc;
}

//=========================================================================

weapon_msn::weapon_msn( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_MSN;
#ifndef X_EDITOR
    // in multiplayer, the meson cannon can only ever have 1 round.
    if( GameMgr.IsGameMultiplayer() )
    {
	    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 1;
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = 1;
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 1;
    }
    else
#endif
    {
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 3;
	    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = 1;
	    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 1;
    }

	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
	
    m_WeaponAmmo[ AMMO_SECONDARY ].m_ProjectileType = BULLET_MSN_SECONDARY;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

	//Both primary and secondary fires use same ammo, so secondary ammo is only initialized
	//in the constructor of new_weapon and set to undefined.
    
    //set aim degradation
    m_AimDegradePrimary     = 0.2f;
    m_AimDegradeSecondary   = 0.25f;
    m_AimRecoverSpeed       = 0.9f;

    m_ReloadBoneIndex       = -1;
    m_CurrentWaitTime       = 0.0f;
    m_ReloadWaitTime        = 5.0f;
    m_bStartReloadFx        = FALSE; 

    m_ChargeUpFxGuid        = NULL_GUID;
    m_ChargeUpGlobeFxGuid   = NULL_GUID;

    m_ReloadFxGuid          = NULL_GUID;

    m_LoopVoiceId           = 0;
    m_ChargeupVoiceID        = 0;

    m_SecondaryFireBaseDamage   = 20.0f;
    m_SecondaryFireMaxDamage    = 35.0f;
    m_SecondaryFireSpeed        = 3000.0f;
    m_SecondaryFireBaseForce    = 10.0f;
    m_SecondaryFireMaxForce     = 20.0f;

    m_LastLockonTime            = s_MSNLockonBeepDelay;

    m_Item = INVEN_WEAPON_MESON_CANNON;

    m_hMuzzleFXPrimary.SetName( PRELOAD_FILE("mhg_muzzleflash_000.fxo") );
    m_hMuzzleFXSecondary.SetName( PRELOAD_FILE("mhg_muzzleflash_000.fxo") );

    // initialize time
    m_LastUpdateTime = (f32)x_GetTimeSec();

    m_bIsAltFiring = FALSE;
    m_AltChargeUpTime = 0.0f;
    m_LastAmmoBurnTime = m_LastUpdateTime;
    m_AmmoBurned = 0;

    m_FiringStage = FS_NONE;

    m_AutoSwitchRating = 8;
}

//=========================================================================

weapon_msn::~weapon_msn()
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }

    if( m_ChargeupVoiceID )
    {
        g_AudioMgr.Release( m_ChargeupVoiceID, 0.0f );
        m_ChargeupVoiceID = 0;
    }

    // get rid of reload particle if it exists
    DestroyReloadFx();

    // destroy chargeup effect if any
    DestroyChargeUpFx();
}

//===========================================================================

void weapon_msn::OnEnumProp( prop_enum& PropEnumList )
{
    new_weapon::OnEnumProp( PropEnumList );    

    PropEnumList.PropEnumHeader  ( "Meson Cannon", "Meson Cannon Weapon", 0 );
    PropEnumList.PropEnumExternal( "Meson Cannon\\Bullet Audio Package",   "Resource\0audiopkg", "The audio package associated with this weapon's bullets.", 0 );

    PropEnumList.PropEnumExternal( "Meson Cannon\\Reload Effect",          "Resource\0fxo",      "The particle effect for the reload", 0 );

    PropEnumList.PropEnumExternal( "Meson Cannon\\ChargeUp Effect",        "Resource\0fxo",      "The particle effect for the chargeup", 0 );
    PropEnumList.PropEnumExternal( "Meson Cannon\\ChargeUp Globe Effect",  "Resource\0fxo",      "The particle effect for the chargeup scalable globe", 0 );

    PropEnumList.PropEnumExternal( "Meson Cannon\\Primary Fire Effect",    "Resource\0fxo",      "The particle effect for the primary fire", 0 );
    PropEnumList.PropEnumExternal( "Meson Cannon\\Secondary Fire Effect",  "Resource\0fxo",      "The particle effect for the secondary fire", 0 );    
    PropEnumList.PropEnumExternal( "Meson Cannon\\Wall Bounce Efect",      "Resource\0fxo",      "The particle effect for when primary projectile hits a wall", 0 );

    PropEnumList.PropEnumFloat   ( "Meson Cannon\\Reload Wait Time",           "How long to wait before the MSN starts reloading.", 0 );
    PropEnumList.PropEnumFloat   ( "Meson Cannon\\Secondary Fire Base Damage", "The minimum amount of damage the secondary fire will cause.", 0 );
    PropEnumList.PropEnumFloat   ( "Meson Cannon\\Secondary Fire Max Damage",  "The maximum amount of damage the secondary fire will cause.", 0 );
    PropEnumList.PropEnumFloat   ( "Meson Cannon\\Secondary Fire Speed",       "How fast is the secondary fire going to travel.", 0 );
    PropEnumList.PropEnumFloat   ( "Meson Cannon\\Secondary Fire Base Force",  "The minimum force the secondary fire going to emmit when it hits something.", 0 );
    PropEnumList.PropEnumFloat   ( "Meson Cannon\\Secondary Fire Max Force",   "The maximum force is the secondary fire going to emmit when it hits something.", 0 );
}

//===========================================================================

xbool weapon_msn::OnProperty( prop_query& PropQuery )
{
    if( new_weapon::OnProperty( PropQuery ) )
    {
#ifndef X_EDITOR
        // in multiplayer, the meson cannon can only ever have 1 round.
        if( GameMgr.IsGameMultiplayer() )
        {
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 1;
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = 1;
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 1;
        }
#endif

        return TRUE;
    }

    if(  new_weapon::OnProperty( PropQuery ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Cannon\\Reload Wait Time",            m_ReloadWaitTime ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Cannon\\Secondary Fire Base Damage",  m_SecondaryFireBaseDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Cannon\\Secondary Fire Max Damage",   m_SecondaryFireMaxDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Cannon\\Secondary Fire Speed",        m_SecondaryFireSpeed ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Cannon\\Secondary Fire Base Force",   m_SecondaryFireBaseForce ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Cannon\\Secondary Fire Max Force",    m_SecondaryFireMaxForce ) )
        return TRUE;

    // External
    if( PropQuery.IsVar( "Meson Cannon\\Reload Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ReloadFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ReloadFx.SetName( pString );                

                // Load the audio package.
                if( m_ReloadFx.IsLoaded() == FALSE )
                    m_ReloadFx.GetPointer();
            }
        }
        return( TRUE );
    }

    if( PropQuery.IsVar( "Meson Cannon\\ChargeUp Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ChargeUpFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ChargeUpFx.SetName( pString );                

                // Load the audio package.
                if( m_ChargeUpFx.IsLoaded() == FALSE )
                    m_ChargeUpFx.GetPointer();
            }
        }
        return( TRUE );
    }

    if( PropQuery.IsVar( "Meson Cannon\\ChargeUp Globe Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ChargeUpGlobeFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ChargeUpGlobeFx.SetName( pString );                

                // Load the audio package.
                if( m_ChargeUpGlobeFx.IsLoaded() == FALSE )
                    m_ChargeUpGlobeFx.GetPointer();
            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "Meson Cannon\\Bullet Audio Package" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_hBulletAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_hBulletAudioPackage.SetName( pString );                

                // Load the audio package.
                if( m_hBulletAudioPackage.IsLoaded() == FALSE )
                    m_hBulletAudioPackage.GetPointer();

            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "Meson Cannon\\Primary Fire Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_PrimaryProjectileFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_PrimaryProjectileFx.SetName( pString );                

                // Load the audio package.
                if( m_PrimaryProjectileFx.IsLoaded() == FALSE )
                    m_PrimaryProjectileFx.GetPointer();
            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "Meson Cannon\\Secondary Fire Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_SecondaryProjectileFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_SecondaryProjectileFx.SetName( pString );                

                // Load the audio package.
                if( m_SecondaryProjectileFx.IsLoaded() == FALSE )
                    m_SecondaryProjectileFx.GetPointer();
            }
        }
        return( TRUE );
    }
    
    // External
    if( PropQuery.IsVar( "Meson Cannon\\Wall Bounce Efect") )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_WallBounceFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_WallBounceFx.SetName( pString );                

                // Load the audio package.
                if( m_WallBounceFx.IsLoaded() == FALSE )
                    m_WallBounceFx.GetPointer();
            }
        }
        return( TRUE );
    }


    return FALSE;
}

//==============================================================================

void weapon_msn::InitWeapon			(   
                                                 const char* pSkinFileName , 
                                                 const char* pAnimFileName , 
                                                 const vector3& rInitPos , 
                                                 const render_state& rRenderState,
                                                 const guid& rParentGuid )
{
	new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos ,rRenderState, rParentGuid);
	
    m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];

    if( m_AnimGroup[rRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[rRenderState].GetBoneIndex( "firepoint" );
        
        ASSERT( m_ReloadBoneIndex != -1 );
    }
}

//===========================================================================

void weapon_msn::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

	m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];

    if( m_AnimGroup[rRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[rRenderState].GetBoneIndex( "firepoint" );

        ASSERT( m_ReloadBoneIndex != -1 );
    }
}

//==============================================================================

void weapon_msn::ResetWeapon( void )
{
    new_weapon::ResetWeapon();

    // reset initializer
    m_bStartReloadFx = TRUE;

    // get rid of reload particle if it exists
    DestroyReloadFx();

    // destroy chargeup effect if any
    DestroyChargeUpFx();

    m_bIsAltFiring = FALSE;
    m_AltChargeUpTime = 0.0f;
    m_SecondaryFireProjectileGuid = NULL_GUID;
}

//=========================================================================
f32 s_MSN_Pitch_Factor = 0.5f;
void weapon_msn::OnAdvanceLogic( f32 DeltaTime )
{
    m_LastUpdateTime = (f32)x_GetTimeSec();

    new_weapon::OnAdvanceLogic( DeltaTime );
}

//==============================================================================
void weapon_msn::UpdateReticle( f32 DeltaTime )
{
    if( !ShouldUpdateReticle() )
    {
        return;
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;

        if( pPlayer->IsCinemaRunning() )
        {
            return;
        }

        // reticle is on
        if( CheckReticleLocked() )
        {
            m_LastLockonTime += DeltaTime;

            if( m_LastLockonTime >= s_MSNLockonBeepDelay )
            {
                g_AudioMgr.Play( "Reticule_Shift_Red" );
                m_LastLockonTime = 0.0f;
            }
        }
        else
        {
            // set delay so as soon as we lock on again, sound will beep.
            m_LastLockonTime = s_MSNLockonBeepDelay;
        }
    }
    else
    {
        // set delay so as soon as we lock on again, sound will beep.
        m_LastLockonTime = s_MSNLockonBeepDelay;
    }
}
//==============================================================================

void weapon_msn::SetupRenderInformation( )
{    
    new_weapon::SetupRenderInformation( );

    if( m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint" );
        
        ASSERT( m_ReloadBoneIndex != -1 );
    }
}

//==============================================================================
xbool weapon_msn::GetFiringStartPosition( vector3 &Pos )
{
    // get the "firepoint" position for this gun
    Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] );
    return TRUE;
}

//=========================================================================

xbool weapon_msn::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;
    ( void )InitPos;    // we don't need InitPos because this weapon fires from its FIRE_POINT

 	ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        vector3 StartPosition;
        GetFiringStartPosition( StartPosition );

        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iFirePoint ] );
        radian3 Rot = L2W.GetRotation();
        (void)Rot;

//        base_projectile* pBaseProjectile;
//        ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );
        
        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;
        pain_handle PainHandle( xfs("%s_%s_PRIMARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

        // Create the Meson Projectile
        guid ProjectileID = CREATE_NET_OBJECT( grav_charge_projectile::GetObjectType(), netobj::TYPE_MESON_1ST );
        grav_charge_projectile* pProjectile = (grav_charge_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
        ASSERT( pProjectile );

        // Compute velocity
        tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
        vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
        Velocity.Rotate( InitRot );
        Velocity += BaseVelocity;

        pProjectile->Setup( Owner,
                            pOwnerObject->net_GetSlot(),
                            StartPosition,
                            radian3(0.0f,0.0f,0.0f), //InitRot
                            Velocity,
                            GetZone1(),
                            GetZone2(),
                            0.0f,
                            PainHandle );

/*
            new_weapon::CreateBullet

            // Lookup speed
            tweak_handle SpeedTweak( xfs("%s_SPEED",pWeaponLogicalName) );
            pBullet->Initialize( InitPos, InitRot, InheritedVelocity, SpeedTweak.GetF32(), OwnerGuid, PainHandle, bHitLiving );

            // Lookup pain degradation
            tweak_handle PainDropDistTweak ( xfs("%s_PainDropDist",pWeaponLogicalName) );
            tweak_handle PainDropScaleTweak( xfs("%s_PainDropScale",pWeaponLogicalName) );
            pBullet->SetPainDegradation( PainDropDistTweak.GetF32(), PainDropScaleTweak.GetF32() );
*/
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );
        
        // Decrement count of bullets in current clip
        DecrementAmmo();

		return TRUE;
	}
}

//=========================================================================

xbool weapon_msn::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
	( void )Power;
    ( void )iFirePoint;
    ( void )InitPos;    // we don't need InitPos because this weapon fires from its FIRE_POINT

    if( m_FiringStage < FS_ONE )
    {
        KillFire();
        return FALSE;
    }

 	ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );

	// create a new bullet projectile, init it's position, and send it on it's way.
    
    vector3 StartPosition;
    GetFiringStartPosition( StartPosition );

    matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iFirePoint ] );
    radian3 Rot = L2W.GetRotation();
    (void)Rot;

    ASSERT( m_WeaponAmmo[ AMMO_SECONDARY ].m_ProjectileTemplateID >= 0 );
    
    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return FALSE;
    pain_handle PainHandle( xfs("%s_%s_SECONDARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

    // Create the Meson Projectile
    guid ProjectileID = CREATE_NET_OBJECT( mesonseeker_projectile::GetObjectType(), netobj::TYPE_MESON_2ND );
    mesonseeker_projectile* pProjectile = (mesonseeker_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
    ASSERT( pProjectile );

    // Compute velocity
    tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
    vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
    Velocity.Rotate( InitRot );
    Velocity += BaseVelocity;

    // cost tweaks
    //tweak_handle SecondaryCostTweak( xfs("%s_SECONDARY_FIRE_COST", GetLogicalName()) );
    //s32 SecondaryCost = SecondaryCostTweak.GetS32();

    pProjectile->Setup( Owner,
                        pOwnerObject->net_GetSlot(),
                        StartPosition,
                        radian3(0.0f,0.0f,0.0f), //InitRot
                        Velocity,
                        GetZone1(),
                        GetZone2(),
                        PainHandle,
                        m_FiringStage);

    pProjectile->SetTarget( m_TargetGuid );

    //add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    if( 1 )  // just a tap fire
    {
        //DecrementAmmo(AMMO_PRIMARY, SecondaryCost);

        voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Fire_Tap", 
                                                GetPosition(), GetZone1(), TRUE );
        g_AudioManager.NewAudioAlert( VoiceID, 
                                      audio_manager::EXPLOSION, 
                                      GetPosition(), 
                                      GetZone1(), 
                                      GetGuid() );
    }
    else
    {
        // charged up
        //g_AudioManager.Play( "MSN_Alt_Fire_Charge", audio_manager::EXPLOSION, GetPosition(), GetZone1(), GetGuid() );
    }

    m_bIsAltFiring = FALSE;

    m_AltChargeUpTime = 0.0f;

    DestroyChargeUpFx();
    m_SecondaryFireProjectileGuid = NULL_GUID;

	return TRUE;
}

//=========================================================================

xbool weapon_msn::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{   
    (void)isHit;
    (void) fDegradeMultiplier;
    vector3 StartPosition;

    //if there weapon is not ready, do nothing.
    if ( ! IsWeaponReady( AMMO_PRIMARY ) )
    {
        return FALSE;
    }
    //otherwise, create a new bullet projectile, init it's position, and send it on it's way.
    else
    {
        GetFiringStartPosition( StartPosition );

        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] );
        radian3 Rot = L2W.GetRotation();
        (void)Rot;

        vector3 toTarget = Target - StartPosition;
        radian3 InitRot(toTarget.GetPitch(),toTarget.GetYaw(),0.0f);

        ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );
        
        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;
        
        pain_handle PainHandle( xfs("%s_%s_PRIMARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

//        base_projectile* pBaseProjectile;
//        pBaseProjectile = CreateBullet( GetLogicalName(), StartPosition, InitRot, BaseVelocity, Owner, PainHandle, AMMO_PRIMARY, isHit );
//        ((grav_charge_projectile*)pBaseProjectile)->LoadEffect( m_PrimaryProjectileFx.GetName(), StartPosition, Rot );

        // Create the Meson Projectile
        guid ProjectileID = CREATE_NET_OBJECT( grav_charge_projectile::GetObjectType(), netobj::TYPE_MESON_1ST );
        grav_charge_projectile* pProjectile = (grav_charge_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
        ASSERT( pProjectile );

        // Compute velocity
        tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
        vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
        Velocity.Rotate( InitRot );
        Velocity += BaseVelocity;

        pProjectile->Setup( Owner,
            pOwnerObject->net_GetSlot(),
            StartPosition,
            radian3(0.0f,0.0f,0.0f), //InitRot
            Velocity,
            GetZone1(),
            GetZone2(),
            0.0f,
            PainHandle );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // Play the sound associated with this actor's faction.
        object* pObj = g_ObjMgr.GetObjectByGuid( Owner );
        actor& OwnerActor = actor::GetSafeType( *pObj );

        factions Faction = OwnerActor.GetFaction();
        s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

        if( m_FactionFireSfx[ BitIndex ] != -1 )
        {
            m_LoopVoiceId = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ), 
                                             GetPosition(), GetZone1(), TRUE );
            g_AudioManager.NewAudioAlert( m_LoopVoiceId, 
                                          audio_manager::GUN_SHOT, 
                                          GetPosition(), 
                                          GetZone1(), 
                                          GetGuid() );
        }

        return TRUE;
    }
}

//=========================================================================

xbool weapon_msn::FireNPCSecondaryProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{
    ( void ) BaseVelocity;
    ( void ) Target;
    ( void ) Owner;
    ( void ) fDegradeMultiplier;
    ( void ) isHit;

    return FALSE;
}

//=========================================================================

xbool weapon_msn::FireGhostPrimary( s32 iFirePoint, xbool, vector3& )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    // Don't create anything, the projectile is networked and will create itself on the other machines,

    // Start the muzzle FX
    InitMuzzleFx( TRUE, iFirePoint );

    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

    vector3 InitPos     = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iFirePoint ] );

    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID < 0 )
    {
        return FALSE;
    }

    // add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    // Play the sound associated with this actor's faction.
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    actor& OwnerActor = actor::GetSafeType( *pObj );

    factions Faction = OwnerActor.GetFaction();
    s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

#if !defined(X_EDITOR)
    if( g_NetworkMgr.IsOnline() )
    {
        BitIndex = 0;
    }
#endif
    if( m_FactionFireSfx[ BitIndex ] != -1 )
    {
        voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Fire_Tap", 
                                            GetPosition(), GetZone1(), TRUE );
        g_AudioManager.NewAudioAlert( VoiceID, 
                                      audio_manager::GUN_SHOT, 
                                      GetPosition(), 
                                      GetZone1(), 
                                      GetGuid() );
    }

    return TRUE;
}

//=========================================================================

xbool weapon_msn::FireGhostSecondary( s32 iFirePoint )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    // Don't create anything, the projectile is networked and will create itself on the other machines,

    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

    vector3 InitPos     = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iFirePoint ] );

    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID < 0 )
    {
        return FALSE;
    }

    // add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    // Play the sound associated with this actor's faction.
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    actor& OwnerActor = actor::GetSafeType( *pObj );

    factions Faction = OwnerActor.GetFaction();
    s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

#if !defined(X_EDITOR)
    if( g_NetworkMgr.IsOnline() )
    {
        BitIndex = 0;
    }
#endif
    voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Fire_Charge", 
                                            GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, 
                                  audio_manager::EXPLOSION, 
                                  GetPosition(), 
                                  GetZone1(), 
                                  GetGuid() );

    return TRUE;
}

//==============================================================================

void weapon_msn::ProcessSfx( void )
{

}

//==============================================================================

void weapon_msn::BeginAltFire( void )
{

}

//==============================================================================

void weapon_msn::EndAltFire( void )
{
}

//==============================================================================
void weapon_msn::BeginAltRampUp( void )
{
    m_AmmoBurned = 0;
    m_bIsAltFiring = TRUE;
    m_LastAmmoBurnTime = (f32)x_GetTimeSec();
    m_AltChargeUpTime = 0.0f;
    m_FiringStage = FS_NONE;
}

//==============================================================================
void weapon_msn::EndAltRampUp( xbool bGoingIntoHold, xbool bSwitchingWeapon )
{
    (void)bGoingIntoHold;

    if( bSwitchingWeapon )
    {
        // switching weapons, just destroy the projectile
        if( m_SecondaryFireProjectileGuid )
        {
            g_ObjMgr.DestroyObjectEx( m_SecondaryFireProjectileGuid, TRUE );
            m_SecondaryFireProjectileGuid = NULL_GUID;
            return;
        }
    }
}
//==============================================================================

void weapon_msn::EndAltHold( xbool bSwitchingWeapon )
{
    if( bSwitchingWeapon )
    {
        // switching weapons, just destroy the projectile
        if( m_SecondaryFireProjectileGuid )
        {
            g_ObjMgr.DestroyObjectEx( m_SecondaryFireProjectileGuid, TRUE );
            m_SecondaryFireProjectileGuid = NULL_GUID;
        }
    }
}
//==============================================================================

void weapon_msn::KillFire( void )
{
    DestroyChargeUpFx();
    DecrementAmmo();
    m_bIsAltFiring = FALSE;

    voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Projectile_Fizzle", GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, 
        audio_manager::GUN_SHOT, 
        GetPosition(), 
        GetZone1(), 
        GetGuid() );
}

//==============================================================================

void weapon_msn::MoveMuzzleFx( void )
{
    new_weapon::MoveMuzzleFx();

    // KSS -- FIXME -- correct bones need to be set up for player and NPC
    if( m_CurrentRenderState == RENDER_STATE_NPC )
        return;

    // update reload fx position
    UpdateReloadFx();

    // update chargeup fx position
    UpdateChargeUpFx();
    
    /*
    // update projectile's position and time( for scale ).
    mesonseeker_projectile* pProj = (mesonseeker_projectile*)g_ObjMgr.GetObjectByGuid( m_SecondaryFireProjectileGuid );
    if( pProj )
    {
        //vector3 Pos;
        s32 iBone = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];
        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( iBone );
        L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( iBone ) );
        //radian3 Rot = L2W.GetRotation();
        //GetFiringStartPosition(Pos);

        // update position, rotation and time
        pProj->UpdateProjectile(L2W, m_AltChargeUpTime);
    }
    */
}

//==============================================================================

xbool weapon_msn::CanReload( const ammo_priority& Priority )
{
    (void)Priority;
    //return FALSE;
    return TRUE;
}

//==============================================================================

xbool weapon_msn::CanFire( xbool bIsAltFire )
{
    s32 AmmoCount = 0;

    // REMEMBER: alt and primary fire use the same ammo
    ammo_priority AmmoPriority = GetPrimaryAmmoPriority();
    AmmoCount = GetAmmoCount( AmmoPriority );

    if( bIsAltFire )
    {
        return FALSE;
    }

    // normal fire will fire 1 round
    return( AmmoCount >= 1 );
}

//==============================================================================

void weapon_msn::BeginPrimaryFire ( void )
{   
}

//==============================================================================

void weapon_msn::EndPrimaryFire ( void )
{
}

//==============================================================================

void weapon_msn::ReleaseAudio( void )
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }

    if( m_ChargeupVoiceID )
    {
        g_AudioMgr.Release( m_ChargeupVoiceID, 0.0f );
        m_ChargeupVoiceID = 0;
    }
}

//==============================================================================

void weapon_msn::InitReloadFx( void )
{
    if( (x_strlen( m_ReloadFx.GetName() ) > 0) && (m_bStartReloadFx == FALSE) && (m_ReloadBoneIndex != -1) )
    {
        m_ReloadFxGuid = particle_emitter::CreateGenericParticle( m_ReloadFx.GetName(), 0 );
    
        // Update the effects position.
        if( m_ReloadFxGuid != NULL_GUID )
        {
            matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
            L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );
        
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ReloadFxGuid );
            pObj->OnTransform( L2W );
        }

        m_bStartReloadFx = TRUE;
    }
}

//==============================================================================

void weapon_msn::UpdateReloadFx( void )
{
    // Update the reload FX's position.
    if( m_ReloadFxGuid != NULL_GUID )
    {
        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
        L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );

        object* pObj = g_ObjMgr.GetObjectByGuid( m_ReloadFxGuid );
        if( pObj )
        {
            // object still good, just move it
            pObj->OnTransform( L2W );
        }
        else
        {
            // make sure we are cleared out from previous destroy since the object is no longer valid, then, reinitialize.
            m_ReloadFxGuid = NULL_GUID;            
        }
    }
}

//==============================================================================

void weapon_msn::DestroyReloadFx( void )
{
    // Destroy the particle.
    if( m_ReloadFxGuid != NULL_GUID )
    {
        g_ObjMgr.DestroyObjectEx( m_ReloadFxGuid, FALSE );
        m_ReloadFxGuid = NULL_GUID;
    }
}

//==============================================================================

void weapon_msn::SuspendReloadFX( void )
{
    // just kill the effect but don't set the FxGuid to NULL so that we can update it.
    if( m_ReloadFxGuid != NULL_GUID )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ReloadFxGuid  );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
    }
}

//==============================================================================

void weapon_msn::InitChargeUpFx( void )
{
    if( !m_ChargeUpFxGuid && (m_ReloadBoneIndex != -1) )
    {
        m_ChargeUpFxGuid = particle_emitter::CreateGenericParticle( m_ChargeUpFx.GetName(), 0 );

        // Update the effects position.
        if( m_ChargeUpFxGuid != NULL_GUID )
        {
            matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
            L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );

            object* pObj = g_ObjMgr.GetObjectByGuid( m_ChargeUpFxGuid );
            pObj->OnTransform( L2W );
        }

        if( (m_ChargeupVoiceID == 0) && (m_CurrentRenderState == RENDER_STATE_PLAYER) )
        {
            // do chargup sound
            m_ChargeupVoiceID = g_AudioMgr.Play( "MSN_Alt_Charge_Loop", GetPosition(), GetZone1(), TRUE );
            g_AudioManager.NewAudioAlert( m_ChargeupVoiceID, audio_manager::GUN_SHOT, GetPosition(), GetZone1(), GetGuid() );
        }
    }
    
    if( !m_ChargeUpGlobeFxGuid && (m_ReloadBoneIndex != -1) )
    {
        m_ChargeUpGlobeFxGuid = particle_emitter::CreateGenericParticle( m_ChargeUpGlobeFx.GetName(), 0 );

        // Update the effects position.
        if( m_ChargeUpGlobeFxGuid != NULL_GUID )
        {
            matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
            L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );

            object* pObj = g_ObjMgr.GetObjectByGuid( m_ChargeUpGlobeFxGuid );
            pObj->OnTransform( L2W );
        }
    }
}

//==============================================================================

void weapon_msn::UpdateChargeUpFx( void )
{
    // Update the ChargeUp FX's position.
    if( m_ChargeUpFxGuid != NULL_GUID )
    {
        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
        L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );

        object* pObj = g_ObjMgr.GetObjectByGuid( m_ChargeUpFxGuid );
        if( pObj )
        {
            // object still good, just move it
            pObj->OnTransform( L2W );
        }
        else
        {
            // make sure we are cleared out from previous destroy since the object is no longer valid, then, reinitialize.
            m_ChargeUpFxGuid = NULL_GUID;            
        }
    }

    // Update the ChargeUp globe FX's position.
    if( m_ChargeUpGlobeFxGuid != NULL_GUID )
    {
        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
        L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );

        object* pObj = g_ObjMgr.GetObjectByGuid( m_ChargeUpGlobeFxGuid );
        if( pObj )
        {
            if( pObj && pObj->IsKindOf(particle_emitter::GetRTTI()) )
            {
                f32 scale = CHARGEUP_START_SCALE;
                scale += (m_AltChargeUpTime * CHARGEUP_SCALE_PER_SECOND);
                particle_emitter &pEmitter = particle_emitter::GetSafeType( *pObj );
                pEmitter.SetScale( scale );
            }

            // object still good, just move it
            pObj->OnTransform( L2W );
        }
        else
        {
            // make sure we are cleared out from previous destroy since the object is no longer valid, then, reinitialize.
            m_ChargeUpGlobeFxGuid = NULL_GUID;            
        }
    }
}

//==============================================================================

void weapon_msn::SuspendChargeUpFX( void )
{
    // just kill the effect but don't set the FxGuid to NULL so that we can update it.
    if( m_ChargeUpFxGuid != NULL_GUID )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ChargeUpFxGuid  );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
    }

    // just kill the effect but don't set the FxGuid to NULL so that we can update it.
    if( m_ChargeUpGlobeFxGuid != NULL_GUID )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ChargeUpGlobeFxGuid  );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
    }
}

//==============================================================================

void weapon_msn::DestroyChargeUpFx( void )
{
    // Destroy the particle.
    if( m_ChargeUpFxGuid != NULL_GUID )
    {
        g_ObjMgr.DestroyObjectEx( m_ChargeUpFxGuid, TRUE );
        m_ChargeUpFxGuid = NULL_GUID;
    }

    // kill looping chargeup sound
    if( m_ChargeupVoiceID )
    {
        g_AudioMgr.Release( m_ChargeupVoiceID, 0.0f );
        m_ChargeupVoiceID = 0;
    }

    // Destroy the particle.
    if( m_ChargeUpGlobeFxGuid != NULL_GUID )
    {
        g_ObjMgr.DestroyObjectEx( m_ChargeUpGlobeFxGuid, TRUE );
        m_ChargeUpGlobeFxGuid = NULL_GUID;
    }
}

//==============================================================================
xbool weapon_msn::CanSwitchIdleAnim( void )
{
    if( (m_CurrentWaitTime < m_ReloadWaitTime) &&
        (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) )
    {
        return FALSE;
    }
    else
    {
        if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount == m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax )
            return TRUE;
        else
            return FALSE;
    }
}

//==============================================================================
void weapon_msn::BeginSwitchTo( void )
{
    // reset initializer
    m_bStartReloadFx = FALSE;
}

//==============================================================================

void weapon_msn::BeginSwitchFrom( void )
{
    // reset initializer
    m_bStartReloadFx = TRUE;

    DestroyReloadFx();

    DestroyChargeUpFx();

    m_bIsAltFiring = FALSE;

    m_AltChargeUpTime = 0.0f;

    m_SecondaryFireProjectileGuid = NULL_GUID;
}

//==============================================================================

void weapon_msn::BeginIdle( void )
{
    m_bIdleMode = TRUE;
    m_CurrentWaitTime = 0.0f;
}

//==============================================================================

void weapon_msn::EndIdle( void )
{
    m_bIdleMode         = FALSE;
    m_bStartReloadFx    = FALSE;

    SuspendReloadFX();
}

//==============================================================================

void weapon_msn::ClearAmmo( const ammo_priority& rAmmoPriority  )
{
    if( DEBUG_INFINITE_AMMO == FALSE )
    {
        // clear count of bullets in current clip
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip = 0;
 
        // clear all bullets    
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount = 0;
    }
}

//===========================================================================

void weapon_msn::DecrementAmmo( const ammo_priority& rAmmoPriority, const s32& nAmt)
{
    object *pObj = g_ObjMgr.GetObjectByGuid(m_ParentGuid);

    if( DEBUG_INFINITE_AMMO == FALSE && !pObj->IsKindOf( character::GetRTTI() ) )
    {
        // decrement count of bullets in current clip
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip -= nAmt;

        // also, take away from total if we aren't unlimited ammo
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount -= nAmt;

#ifndef X_EDITOR
        object* pObject = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );

        if( pObject ) 
        {
            if( pObject->IsKindOf( actor::GetRTTI() ) )
            {
                ((actor*)pObject)->DirtyAmmo();
            }
        }
#endif
    }
}

//===========================================================================

void weapon_msn::FillAmmo( void )
{
    // make sure ammo is constantly full - Meson Cannon works a bit differently
    if( DEBUG_INFINITE_AMMO )
    {
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    }
}

//===========================================================================
