//==============================================================================
// Weapon Meson Hand Gun.cpp
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileEnergy.hpp"
#include "WeaponMHG.hpp"
#include "Debris\debris_mgr.hpp"
#include "Objects\Projector.hpp"
#include "render\LightMgr.hpp"

//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================

static const s32 s_Num_Pellets = 4;
static f32 k_MHGPrimaryDamage = 15.0f;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_mhg_desc : public object_desc
{

    weapon_mhg_desc( void ) : object_desc( object::TYPE_WEAPON_MHG, 
                                        "MHG",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
										object::ATTR_NEEDS_LOGIC_TIME		|
                                        object::ATTR_SOUND_SOURCE			|
                                        object::ATTR_RENDERABLE				|
 										object::ATTR_COLLISION_PERMEABLE,
                                        FLAGS_GENERIC_EDITOR_CREATE |
                                        FLAGS_IS_DYNAMIC

										) {}

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_mhg;
    }

} s_weapon_mhg_Desc;

//=========================================================================

const object_desc&  weapon_mhg::GetTypeDesc     ( void ) const
{
    return s_weapon_mhg_Desc;
}

//=========================================================================

const object_desc&  weapon_mhg::GetObjectType   ( void )
{
    return s_weapon_mhg_Desc;
}

//=========================================================================

weapon_mhg::weapon_mhg( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_MHG;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 10;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 10;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
	

	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;
    
    m_fFiringDelay      = .35f;		//firing delay for the shotgun is .1
    m_NPCMuzzleSoundFx  = "MHG_Primary_Fire";

	//Both primary and secondary fires use same ammo, so secondary ammo is only initialized
	//in the constructor of new_weapon and set to undefined.
    
    //set aim degradation
    m_AimDegradePrimary     = 0.2f;
    m_AimDegradeSecondary   = 0.3f;
    m_AimRecoverSpeed       = 0.95f;
    m_AltReloadBoneIndex    = -1;
    m_ReloadBoneIndex       = -1;
    m_CurrentWaitTime       = 0.0f;
    m_ReloadWaitTime        = 5.0f;
    m_bStartReloadFx        = FALSE;
    m_AltReloadFxGuid       = NULL;
    m_ReloadFxGuid          = NULL;

    m_SecondaryFireBaseDamage   = 20.0f;
    m_SecondaryFireMaxDamage    = 35.0f;
    m_SecondaryFireSpeed        = 3000.0f;
    m_SecondaryFireBaseForce    = 10.0f;
    m_SecondaryFireMaxForce     = 20.0f;

    m_InvType = inventory_item::INV_WPN_MHG;

    m_hMuzzleFXPrimary.SetName( "mhg_muzzleflash_000.fxo" );
    m_hMuzzleFXSecondary.SetName( "mhg_muzzleflash_000.fxo" );

}

//=========================================================================

weapon_mhg::~weapon_mhg()
{
}

//=========================================================================

void weapon_mhg::InitWeapon( const char* pSkinFileName, 
                             const char* pAnimFileName, 
                             const vector3& rInitPos, 
                             const render_state& rRenderState,
                             const guid& rParentGuid )
{
    new_weapon::InitWeapon( pSkinFileName, pAnimFileName, rInitPos, rRenderState, rParentGuid );

    // Get the reload bone indices.
    if( m_AnimGroup[m_CurrentRenderIndex][m_CurrentRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[m_CurrentRenderIndex][m_CurrentRenderState].GetBoneIndex( "reload_right" );
        m_AltReloadBoneIndex = m_AnimPlayer[m_CurrentRenderIndex][m_CurrentRenderState].GetBoneIndex( "reload_left" );
    }
}

//===========================================================================

void weapon_mhg::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

    // Get the reload bone indices.
    if( m_AnimGroup[m_CurrentRenderIndex][m_CurrentRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[m_CurrentRenderIndex][m_CurrentRenderState].GetBoneIndex( "reload_right" );
        m_AltReloadBoneIndex = m_AnimPlayer[m_CurrentRenderIndex][m_CurrentRenderState].GetBoneIndex( "reload_left" );
    }
}

//=========================================================================

void weapon_mhg::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_bIdleMode )
    {
        m_CurrentWaitTime += DeltaTime;
        if( (m_CurrentWaitTime >= m_ReloadWaitTime) && 
            (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) )
        {
            s32 Ammo = (s32)((m_CurrentWaitTime-m_ReloadWaitTime)/2.0f);

            // Did the ammo count change.
            if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount != (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount+Ammo) )
            {
                // Update the ammo and set the time back.
                m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount += Ammo;
                m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip    = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
                m_CurrentWaitTime = m_ReloadWaitTime;
            }
            
            InitReloadFx();
                        
            // All maxed out.
            if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount == m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax )
            {
                DestoryReloadFx();
            }
        }

        UpdateReloadFx();
    }

    new_weapon::OnAdvanceLogic( DeltaTime );
}

//=========================================================================

void weapon_mhg::OnMove( const vector3& NewPos )
{
    new_weapon::OnMove( NewPos );
}

//=========================================================================

void weapon_mhg::OnTransform( const matrix4& L2W )
{
    new_weapon::OnTransform( L2W );
}

//=========================================================================

xbool weapon_mhg::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint )
{
    ( void )Power;

 	ASSERT( m_FiringPointBoneIndex[ iBonePoint ] != -1 );

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        //set the firing timer.
		m_fFiringTimer = m_fFiringDelay;
 
#ifdef FIRE_FROM_GUN
        vector3 InitPos = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iBonePoint ] );
#endif
        
        matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iBonePoint ] );
        radian3 Rot = L2W.GetRotation();
        (void)Rot;

        base_projectile* pBaseProjectile;
        if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 )
        {
            pBaseProjectile = CreateBullet( InitPos, InitRot, BaseVelocity, Owner );
        }
        else
        {
            pBaseProjectile = CreateBullet( 
                energy_projectile::GetObjectType(), 
                InitPos, 
                InitRot,  
                BaseVelocity , 
                15.f , 
                5.0f,
                5000.f , 
                Owner );
        }

        ((energy_projectile*)pBaseProjectile)->LoadEffect( m_PrimaryProjectileFx.GetName(), InitPos, Rot );
        
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );
        
        // Decrement count of bullets in current clip
        DecrementAmmo();

		return TRUE;
	}
}

//=========================================================================

xbool weapon_mhg::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint )
{
	
    ( void )Power;

 	ASSERT( m_AltFiringPointBoneIndex[ iBonePoint ] != -1 );

	//if there weapon is not ready, do nothing.
	if( ! IsWeaponReady( AMMO_PRIMARY ) && (GetAmmoCount() < (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax/2.0f) ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        //set the firing timer.
		m_fFiringTimer = m_fFiringDelay;
 
#ifdef FIRE_FROM_GUN
        vector3 InitPos = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBonePosition( m_AltFiringPointBoneIndex[ iBonePoint ] );
#endif
        
        f32 DeltaDamage = ((m_SecondaryFireMaxDamage-m_SecondaryFireBaseDamage)* ((GetAmmoCount()*0.5f) / (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax*0.5f)));
        f32 Damage      = m_SecondaryFireBaseDamage + DeltaDamage;

        f32 DeltaForce  = ((m_SecondaryFireMaxForce-m_SecondaryFireBaseForce)* ((GetAmmoCount()*0.5f) / (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax*0.5f)));
        f32 Force       = m_SecondaryFireBaseForce + DeltaForce;
 
        matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iBonePoint ] );
        radian3 Rot = L2W.GetRotation();
        (void)Rot;

        base_projectile* pBaseProjectile;
        pBaseProjectile = CreateBullet( energy_projectile::GetObjectType(), 
                                        InitPos, 
                                        InitRot,  
                                        BaseVelocity , 
                                        Damage, 
                                        Force,
                                        m_SecondaryFireSpeed, 
                                        Owner );


         ((energy_projectile*)pBaseProjectile)->LoadEffect( m_SecondayProjectileFx.GetName(), InitPos, Rot );
        
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // clear all ammo
        ClearAmmo();

		return TRUE;
	}
}

//==============================================================================

xbool weapon_mhg::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{   
    vector3 InitPos;
    
    if (!GetFiringBonePosition(InitPos))
        return FALSE;
    
    vector3 TargetVector   = Target - InitPos ;
    radian3 TargetRot(TargetVector.GetPitch(), TargetVector.GetYaw(), 0.0f );
    
    DegradeAim( TargetRot, R_1*fDegradeMultiplier, InitPos, Owner );

    matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ FIRE_POINT ] );
    radian3 Rot = L2W.GetRotation();
    (void)Rot;

    base_projectile* pBaseProjectile;
    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 )
    {
        pBaseProjectile = CreateBullet( InitPos, TargetRot, BaseVelocity, Owner, pain::TYPE_PROJECTILE_BULLET, AMMO_PRIMARY, isHit );
    }
    else
    {
        pBaseProjectile = CreateBullet( 
             energy_projectile::GetObjectType(), 
             InitPos, 
             TargetRot,  
             BaseVelocity , 
             k_MHGPrimaryDamage ,
             5.0f,
             50000.f , 
             Owner, 
             pain::TYPE_PROJECTILE_BULLET, 
             isHit );
    }

     ((energy_projectile*)pBaseProjectile)->LoadEffect( m_PrimaryProjectileFx.GetName(), InitPos, Rot );

    //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    g_AudioManager.Play( "MHG_Primary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                                            GetZone1(), GetGuid() );
     
    return TRUE;
}

//=========================================================================

xbool weapon_mhg::FireNPCSecondaryProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{
    return FireNPCWeaponProtected( BaseVelocity, Target, Owner, fDegradeMultiplier, isHit );
}

//==============================================================================

void  weapon_mhg::InitMuzzleFx ( void )
{
    switch ( m_CurrentRenderState )
    {
    case RENDER_STATE_PLAYER:
        {
            for( s32 iBone = 0; iBone < MAX_BONE_POINTS; iBone++ )
            {
                if( m_FiringPointBoneIndex[iBone] != -1 )
                {
                    if (m_MuzzleFXGuid[iBone]==NULL)
                    {
                        m_MuzzleFXGuid[iBone] = particle_emitter::CreatePersistantParticle( 
                            particle_emitter::PLAYER_MUZZLE_FLASH_MHG );
                    }
                }
                
                if( m_AltFiringPointBoneIndex[iBone] != -1 )
                {            
                    if (m_MuzzleSecondaryFXGuid[iBone]==NULL)
                    {
                        m_MuzzleSecondaryFXGuid[iBone] = particle_emitter::CreatePersistantParticle( 
                            particle_emitter::PLAYER_MUZZLE_FLASH_MHG );
                    }
                }
            }
        }
        
        break;
        
    case  RENDER_STATE_NPC:
        {
            if ( m_MuzzleFXGuid[ FIRE_POINT ] == NULL )
            {
                m_MuzzleFXGuid[ FIRE_POINT ] = particle_emitter::CreatePersistantParticle(  
                    particle_emitter::PLAYER_MUZZLE_FLASH_MHG );
            }
        }
        break;
        
    default:
        ASSERT(0);
        break;
    }
}

//==============================================================================
/*
xbool weapon_mhg::CanIntereptPrimaryFire( s32 nFireAnimIndex )
{
    (void)nFireAnimIndex;

    return TRUE;
}
*/
//==============================================================================

xbool weapon_mhg::CanReload( const ammo_priority& Priority )
{
    (void)Priority;
    return FALSE;
}

//==============================================================================

void weapon_mhg::SetCurrentRenderIndex( s32 nRenderIndex )
{
    // Get the reload bone indices.
    if( m_AnimGroup[m_CurrentRenderIndex][m_CurrentRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[m_CurrentRenderIndex][m_CurrentRenderState].GetBoneIndex( "reload_right" );
        m_AltReloadBoneIndex = m_AnimPlayer[m_CurrentRenderIndex][m_CurrentRenderState].GetBoneIndex( "reload_left" );
    }

    new_weapon::SetCurrentRenderIndex( nRenderIndex );
}

//==============================================================================

void weapon_mhg::OnEnumProp( prop_enum& List )
{
    new_weapon::OnEnumProp( List );

    List.AddHeader  ( "Meson Hand Gun", "Meson hand gun" );
    List.AddExternal( "Meson Hand Gun\\Reload Effect",          "Resource\0fxo",  "The particle effect for the reload" );
    List.AddExternal( "Meson Hand Gun\\Primary Fire Effect",    "Resource\0fxo",  "The particle effect for the primary fire" );
    List.AddExternal( "Meson Hand Gun\\Secondary Fire Effect",  "Resource\0fxo",  "The particle effect for the secondary fire" );

    List.AddFloat   ( "Meson Hand Gun\\Reload Wait Time",           "How long to wait before the MHG starts reloading." );
    List.AddFloat   ( "Meson Hand Gun\\Secondary Fire Base Damage", "The minimum amount of damage the secondary fire will cause." );
    List.AddFloat   ( "Meson Hand Gun\\Secondary Fire Max Damage",  "The maximum amount of damage the secondary fire will cause." );
    List.AddFloat   ( "Meson Hand Gun\\Secondary Fire Speed",       "How fast is the secondary fire going to travel." );
    List.AddFloat   ( "Meson Hand Gun\\Secondary Fire Base Force",  "The minimum force the secondary fire going to emmit when it hits something." );
    List.AddFloat   ( "Meson Hand Gun\\Secondary Fire Max Force",   "The maximum force is the secondary fire going to emmit when it hits something." );
}

//==============================================================================

xbool weapon_mhg::OnProperty( prop_query& PropQuery )
{
    if(  new_weapon::OnProperty( PropQuery ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Hand Gun\\Reload Wait Time",            m_ReloadWaitTime ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Hand Gun\\Secondary Fire Base Damage",  m_SecondaryFireBaseDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Hand Gun\\Secondary Fire Max Damage",   m_SecondaryFireMaxDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Hand Gun\\Secondary Fire Speed",        m_SecondaryFireSpeed ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Hand Gun\\Secondary Fire Base Force",   m_SecondaryFireBaseForce ) )
        return TRUE;

    if(  PropQuery.VarFloat( "Meson Hand Gun\\Secondary Fire Max Force",    m_SecondaryFireMaxForce ) )
        return TRUE;

    // External
    if( PropQuery.IsVar( "Meson Hand Gun\\Reload Effect" ) )
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

    // External
    if( PropQuery.IsVar( "Meson Hand Gun\\Primary Fire Effect" ) )
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
    if( PropQuery.IsVar( "Meson Hand Gun\\Secondary Fire Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_SecondayProjectileFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_SecondayProjectileFx.SetName( pString );                

                // Load the audio package.
                if( m_SecondayProjectileFx.IsLoaded() == FALSE )
                    m_SecondayProjectileFx.GetPointer();
            }
        }
        return( TRUE );
    }

    return FALSE;
}


//==============================================================================

void weapon_mhg::BeginIdle( void )
{
    m_bIdleMode = TRUE;
    m_CurrentWaitTime = 0.0f;
}

//==============================================================================

void weapon_mhg::EndIdle( void )
{
    m_bIdleMode         = FALSE;
    m_bStartReloadFx    = FALSE;

    DestoryReloadFx();
}

//==============================================================================

xbool weapon_mhg::CanSwitchIdleAnim( void )
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

void weapon_mhg::InitReloadFx( void )
{
    if( (x_strlen( m_ReloadFx.GetName() ) > 0) && (m_bStartReloadFx == FALSE) && (m_ReloadBoneIndex != -1) )
    {
        m_ReloadFxGuid = particle_emitter::CreateGenericParticle( m_ReloadFx.GetName(), 0 );
    
        // Update the effects position.
        if( m_ReloadFxGuid != NULL )
        {
            matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
            L2W.PreTranslate( m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );
        
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ReloadFxGuid );
            pObj->OnTransform( L2W );
        }

        m_bStartReloadFx = TRUE;
    }

    if( (x_strlen( m_ReloadFx.GetName() ) > 0) && (m_bStartReloadFx == FALSE) && (m_AltReloadBoneIndex != -1) )
    {
        m_AltReloadFxGuid = particle_emitter::CreateGenericParticle( m_ReloadFx.GetName(), 0 );
    
        // Update the effects position.
        if( m_AltReloadFxGuid != NULL )
        {
            matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_AltReloadBoneIndex );
            L2W.PreTranslate( m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBindPosition( m_AltReloadBoneIndex ) );
        
            object* pObj = g_ObjMgr.GetObjectByGuid( m_AltReloadFxGuid );
            pObj->OnTransform( L2W );
        }

        m_bStartReloadFx = TRUE;
    }
}

//==============================================================================

void weapon_mhg::UpdateReloadFx( void )
{
    // Update the reload FX's position.
    if( m_ReloadFxGuid != NULL )
    {
        matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
        L2W.PreTranslate( m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );
    
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ReloadFxGuid );
        pObj->OnTransform( L2W );
    }

    if( m_AltReloadFxGuid != NULL )
    {
        matrix4 L2W = m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBoneL2W( m_AltReloadBoneIndex );
        L2W.PreTranslate( m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBindPosition( m_AltReloadBoneIndex ) );
    
        object* pObj = g_ObjMgr.GetObjectByGuid( m_AltReloadFxGuid );
        pObj->OnTransform( L2W );
    }
}

//==============================================================================

void weapon_mhg::DestoryReloadFx( void )
{
    // Destroy the particle.
    if( m_ReloadFxGuid != NULL )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ReloadFxGuid  );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_ReloadFxGuid = NULL;
    }

    // Destroy the particle.
    if( m_AltReloadFxGuid != NULL )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_AltReloadFxGuid  );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_AltReloadFxGuid = NULL;
    }
}

//==============================================================================
