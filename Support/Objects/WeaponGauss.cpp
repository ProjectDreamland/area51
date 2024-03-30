#ifndef DISABLE_LEGACY_CODE
#include "Obj_mgr\obj_mgr.hpp"
#include "WeaponGauss.hpp"
#include "ProjectileBullett.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\GrenadeProjectile.hpp"
#include "objects\Projector.hpp"
#include "render\LightMgr.hpp"


//=========================================================================
// CONSTS
//=========================================================================

static f32  k_force_of_grenade_throw = 5000.0f;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_gauss_desc : public object_desc
{

    weapon_gauss_desc( void ) : object_desc( object::TYPE_WEAPON_GAUSS, 
                                        "Gauss Rifle",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
										object::ATTR_NEEDS_LOGIC_TIME		|
                                        object::ATTR_SOUND_SOURCE			|
                                        object::ATTR_RENDERABLE				|
 										object::ATTR_COLLISION_PERMEABLE,
                                        FLAGS_GENERIC_EDITOR_CREATE |
                                        FLAGS_IS_DYNAMIC    
										)
    {

    }

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_gauss;
    }

} s_Weapon_Gauss_Desc;

//=========================================================================

const object_desc&  weapon_gauss::GetTypeDesc( void ) const
{
    return s_Weapon_Gauss_Desc;
}

//=========================================================================

const object_desc&  weapon_gauss::GetObjectType( void )
{
    return s_Weapon_Gauss_Desc;
}

//=========================================================================

weapon_gauss::weapon_gauss(void)
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_GAUSS_RIFLE;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 1000;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 250;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;

	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_SECONDARY ].m_ProjectileType = GRENADE_FRAG;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 16;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 4;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

    m_HasSecondaryAmmo      = TRUE;
    m_fFiringDelay          = .05f;		//short firing delay for the Gauss rifle

    //set aim degradation
    m_AimDegradePrimary     = 0.2f;
    m_AimDegradeSecondary   = 0.4f;
    m_AimRecoverSpeed       = 0.8f;

    m_LoopVoiceId           = 0;

    m_InvType = inventory_item::INV_WPN_GAUSS;    

    m_hMuzzleFXPrimary.SetName( "Muzzleflash_1stperson_Gauss_001.fxo" );
}

//=========================================================================

weapon_gauss::~weapon_gauss()
{
}

//=========================================================================

void weapon_gauss::InitWeapon			(   
                                                 const char* pSkinFileName , 
                                                 const char* pAnimFileName , 
                                                 const vector3& rInitPos , 
                                                 const render_state& rRenderState,
                                                 const guid& rParentGuid)
{
    new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos , rRenderState, rParentGuid );

    SynchGunToCount() ;
}

//===========================================================================

void weapon_gauss::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

    // figure out which mesh and material corresponds to the numbers
    geom* pGeom = m_Skin[0]->GetGeom();
    if ( pGeom )
    {
        m_LEDInfo[0].Mesh     = pGeom->GetMeshIndex( "WPN_GAS_LED01_E" );
        m_LEDInfo[0].Material = 2;
        m_LEDInfo[1].Mesh     = pGeom->GetMeshIndex( "WPN_GAS_LED02_E" );
        m_LEDInfo[1].Material = 2;
        m_LEDInfo[2].Mesh     = pGeom->GetMeshIndex( "WPN_GAS_LED03_E" );
        m_LEDInfo[2].Material = 2;

        ASSERT( (m_LEDInfo[0].Mesh>=0) &&
                (m_LEDInfo[1].Mesh>=0) &&
                (m_LEDInfo[2].Mesh>=0) );
    }

    SynchGunToCount() ;
}

//===========================================================================   

xbool weapon_gauss::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint )
{
	( void ) InitPos;
    ( void ) Power;
    ( void )iBonePoint;

    ASSERT( m_FiringPointBoneIndex[ iBonePoint ] != -1 );

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        // If we have a registered template, use it.
        if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 )
        {
            // This creates the bullet using a template.
            CreateBullet(InitPos, InitRot, BaseVelocity, Owner );
        }
        else
        {
            // This creates a bullet the old way.
            CreateBullet(   bullet_projectile::GetObjectType(), 
                            InitPos, 
                            InitRot,  
                            BaseVelocity , 
                            20.f , 
                            10.f,
                            50000.f , 
                            Owner );
        }

        DecrementAmmo();

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor(0,124,120), 400.0f, 1.0f, 0.1f );
        
        SynchGunToCount() ;
		return TRUE;
	}
}

//=========================================================================

void weapon_gauss::SynchGunToCount( void )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        geom* pGeom = m_Skin[ m_CurrentRenderIndex ][RENDER_STATE_PLAYER].GetGeom() ;

        if ( pGeom == NULL )
            return ;

        for ( s32 i = 0; i < 3; i++ )
        {
            ASSERT( m_Skin[ m_CurrentRenderIndex ][RENDER_STATE_PLAYER].GetGeom()->HasBitmapAnim(m_LEDInfo[i].Material) );
            render::tex_anim TexAnim;
            render::htexanim_inst hTexAnims = render::GetTexAnimData( m_Skin[ m_CurrentRenderIndex ][RENDER_STATE_PLAYER].GetInst() );
            render::GetBitmapAnim( hTexAnims,
                                   m_LEDInfo[i].Mesh,
                                   m_LEDInfo[i].Material,
                                   TexAnim );
            s32 Image = 0;
            switch ( i )
            {
                case 0: // left LED
                    Image = m_WeaponAmmo[AMMO_PRIMARY].m_AmmoInCurrentClip / 100;
                    break;
                case 1: // center LED
                    Image = (m_WeaponAmmo[AMMO_PRIMARY].m_AmmoInCurrentClip / 10) % 10;
                    break;
                case 2: // right LED
                    Image = m_WeaponAmmo[AMMO_PRIMARY].m_AmmoInCurrentClip % 10;
                    break;
            }

            TexAnim.iStart = Image;
            render::SetBitmapAnim( hTexAnims,
                                   m_LEDInfo[i].Mesh,
                                   m_LEDInfo[i].Material,
                                   TexAnim );
        }
    }
}

//=======================================================================================================

xbool weapon_gauss::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iBonePoint )
{
	( void ) InitPos;
    ( void ) BaseVelocity;
    ( void ) Power;

    //Grenade projectile is not ready yet.  Just return False here.

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_SECONDARY ) )
	{
		return FALSE;
	}

	//otherwise, create a grenade projectile, init it's position, and send it on it's way.
	else
    {
        //create grenade
        guid GrenadeID = g_ObjMgr.CreateObject( grenade_projectile::GetObjectType() );
        grenade_projectile* pGrenade = ( grenade_projectile* ) g_ObjMgr.GetObjectByGuid( GrenadeID );
        
        //set the firing timer.
        m_fFiringTimer = m_fFiringDelay;
        
        //make sure the grenade was created.
        ASSERT( pGrenade );
        
        pGrenade->LoadInstance("WPN_FRG.rigidgeom");
        
        f32 ForceOfThrow = k_force_of_grenade_throw ;

        pGrenade->Initialize( m_AnimPlayer[m_CurrentRenderIndex][ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iBonePoint ] ), 
                              InitRot, 
                              BaseVelocity, 
                              200.f, 
                              100.f, 
                              ForceOfThrow, 
                              Owner );

        DecrementAmmo(AMMO_SECONDARY);

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        return TRUE;
	}
}

//==============================================================================

void weapon_gauss::ProcessSfx( void )
{
    /*
    // Check all events in current animation
    for (s32 i = 0; i < m_AnimPlayer[m_CurrentRenderState].GetNEvents(); i++)
    {
        // Is this event active?
        if (m_AnimPlayer[m_CurrentRenderState].IsEventActive(i))
        {
            // Lookup event and world position
            const anim_event& Event  = m_AnimPlayer[m_CurrentRenderState].GetEvent(i) ;
            vector3           Pos    = m_AnimPlayer[m_CurrentRenderState].GetEventPosition(i) ;

            // Old event?
            if (x_stricmp(Event.GetType(), "Old Event") == 0)
            {
                switch(Event.GetInt( anim_event::INT_IDX_OLD_TYPE ))
                {
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_LR:
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_LL:
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_UR:
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_UL:
                    case ANIM_EVENT_PRIMARY_FIRE:
                        
//                        g_AudioManager.Play( "Railgun_Primary_Fire", Pos );
                        
                        // Need to get looping working before we can enable this.
                        if( m_CurrentRenderState != RENDER_STATE_PLAYER )
                        {                        
                            g_AudioManager.Play( "Railgun_Primary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                                GetZone1(), GetGuid() );
                        }
                        else
                        {
                            if( m_LoopVoiceId )
                                g_AudioManager.SetPosition( m_LoopVoiceId, Pos );
                        }

                    break;
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_LR:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_LL:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_UR:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_UL:
	                case ANIM_EVENT_SECONDARY_FIRE:
                        g_AudioManager.Play( "Railgun_Secondary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                            GetZone1(), GetGuid() );
                    break;
                    case ANIM_EVENT_SFX_WEAPONLOOP:
                        //g_AudioManager.Play( "Shotgun_Primary_Fire", Pos );
                    break;
                    case ANIM_EVENT_SFX_SWITCH_TO:
                        g_AudioManager.Play( "Railgun_Ready", Pos );
                    break;
                    case ANIM_EVENT_SFX_SWITCH_FROM:
                        g_AudioManager.Play( "Railgun_Unready", Pos );
                    break;
                    case ANIM_EVENT_SFX_RELOAD:
                        g_AudioManager.Play( "Railgun_Reload", Pos );
                    break;
                    case ANIM_EVENT_SFX_ALT_RELOAD:
                        g_AudioManager.Play( "Railgun_Reload", Pos );
                    break;
                    default:
                        x_DebugMsg( "Anim Event: Guass Event[%d]\n", Event.GetInt( anim_event::INT_IDX_OLD_TYPE ) ) ;
                    break;
                }
            }
        }
    }
    */
}

//==============================================================================

void weapon_gauss::Reload( const ammo_priority& Priority )
{
    new_weapon::Reload( Priority ) ;

    SynchGunToCount() ;
}
//==============================================================================

void weapon_gauss::BeginPrimaryFire ( void )
{
    if( (m_LoopVoiceId == 0) && (m_CurrentRenderState == RENDER_STATE_PLAYER) )
    {
        // Use the weapons position as the place where the sound start since the next time the event for
        // the secondary fire comes it will move it to the muzzel.
        m_LoopVoiceId = g_AudioManager.Play( "Railgun_Primary_Fire_Loop", audio_manager::GUN_SHOT, GetPosition(), 
                                GetZone1(), GetGuid() );

    }
}

//==============================================================================

void weapon_gauss::EndPrimaryFire ( void )
{
    if( m_LoopVoiceId )
    {
        vector3 Pos( 0.0f, 0.0f, 0.0f);
        g_AudioManager.GetPosition( m_LoopVoiceId, Pos );
        g_AudioManager.Play( "Railgun_Primary_Fire", Pos );
        
        g_AudioManager.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }
}

//==============================================================================

void weapon_gauss::OnMove( const vector3& NewPos )
{   
    if( m_LoopVoiceId )
    {
        g_AudioManager.SetPosition( m_LoopVoiceId, NewPos, GetZone1() );
    }

    new_weapon::OnMove( NewPos );
}

//==============================================================================

void weapon_gauss::OnTransform( const matrix4& L2W )
{
    if( m_LoopVoiceId )
    {
        g_AudioManager.SetPosition( m_LoopVoiceId, L2W.GetTranslation(), GetZone1() );
    }

    new_weapon::OnTransform( L2W );
}

//==============================================================================

void  weapon_gauss::InitMuzzleFx ( void )
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
                            particle_emitter::PLAYER_MUZZLE_FLASH_GAUSS );
                    }
                }
                
                if( m_AltFiringPointBoneIndex[iBone] != -1 )
                {    
                    if (m_MuzzleSecondaryFXGuid[iBone]==NULL)
                    {
                        m_MuzzleSecondaryFXGuid[iBone] = particle_emitter::CreatePersistantParticle(  
                            particle_emitter::PLAYER_MUZZLE_FLASH_GAUSS_2ND );
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

xbool weapon_gauss::CanIntereptPrimaryFire( s32 nFireAnimIndex )
{
    (void)nFireAnimIndex;
    return TRUE;
}

//==============================================================================

xbool weapon_gauss::CanIntereptSecondaryFire( s32 nFireAnimIndex )
{
    (void)nFireAnimIndex;
    return TRUE;
}

//==============================================================================
#endif
