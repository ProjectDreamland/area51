#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileBullett.hpp"
#include "WeaponDualSMP.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\Projector.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "render\LightMgr.hpp"
#include "Objects\Player.hpp"

//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================

f32     g_DualSMPReleaseTime        = 0.050f;

tweak_handle DualSMP_SpreadTweak("DualSMP_Spread");

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_dual_smp_desc : public object_desc
{

    weapon_dual_smp_desc( void ) : object_desc( object::TYPE_WEAPON_DUAL_SMP, 
                                        "Dual SMP Rifles",
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
        return new weapon_dual_smp;
    }

} s_Weapon_Dual_SMP_Desc;

//=========================================================================

const object_desc&  weapon_dual_smp::GetTypeDesc     ( void ) const
{
    return s_Weapon_Dual_SMP_Desc;
}

//=========================================================================

const object_desc&  weapon_dual_smp::GetObjectType   ( void )
{
    return s_Weapon_Dual_SMP_Desc;
}


//=========================================================================
// WEAPON_DUAL_SMP
//=========================================================================

weapon_dual_smp::weapon_dual_smp( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_MACHINE_GUN;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 500;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 60;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
	
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
    m_ZoomMovementSpeed     = 0.9f;
    
    m_ViewChangeRate        = 1.0f;
    m_ZoomFOV               = R_35;

    m_LoopVoiceId           = 0;

    m_Item = INVEN_WEAPON_DUAL_SMP;

    m_hMuzzleFXPrimary.SetName( "Muzzleflash_1stperson_SMP_000.fxo" );

    m_nZoomSteps = 1;

    m_AutoSwitchRating = 6;
}

//=========================================================================

weapon_dual_smp::~weapon_dual_smp()
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }
}

//===========================================================================

void weapon_dual_smp::InitWeapon			(   
                                                 const char* pSkinFileName , 
                                                 const char* pAnimFileName , 
                                                 const vector3& rInitPos , 
                                                 const render_state& rRenderState,
                                                 const guid& rParentGuid )
{
	new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos ,rRenderState, rParentGuid);
	
	m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];

    // Setup NPC anim controller to position both weapons inside the same geometry
    InitNPCDualAnimController( &m_DualAnimController, "B_WPN_2MP_Left", "B_WPN_2MP_Right" );
}

//===========================================================================

void weapon_dual_smp::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

	m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];

    // Setup NPC anim controller to position both weapons inside the same geometry
    InitNPCDualAnimController( &m_DualAnimController, "B_WPN_2MP_Left", "B_WPN_2MP_Right" );
}

//===========================================================================

xbool weapon_dual_smp::CanReload( const ammo_priority& Priority )
{
    (void) Priority;

    // cannot reload Dual SMPs
    return FALSE;
}

//=========================================================================

xbool weapon_dual_smp::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;

 	ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        #ifdef FIRE_FROM_GUN
        vector3 InitPos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iFirePoint ] );
        #endif

        base_projectile* BP;
        ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;
        pain_handle PainHandle( xfs("%s_%s",pOwnerObject->GetLogicalName(),GetLogicalName()) );

        f32 DualSMP_Spread = DualSMP_SpreadTweak.GetF32();
        // pick a random point on a small box that is located 10 cm. away.        
        f32 rx = x_frand( -DualSMP_Spread, DualSMP_Spread );
        f32 ry = x_frand( -DualSMP_Spread, DualSMP_Spread );
        vector3 vDir = vector3( rx, ry , 10.f );

        //rotate that point into model space
        vDir.Rotate( InitRot );

        //make a radian3 out of this vector
        radian Pitch , Yaw;
        vDir.GetPitchYaw( Pitch,Yaw );
        radian3 vRot( Pitch , Yaw , 0.f );

        BP = CreateBullet( GetLogicalName(), 
                           InitPos, 
                           vRot, 
                           BaseVelocity, 
                           Owner, 
                           PainHandle, 
                           AMMO_PRIMARY,
                           TRUE,
                           iFirePoint );

//    if( IsGameMultiplayer() && g_MultiplayerPainTweaks.m_bUseTweaks )
//        BP->SetDamageAmount( g_MultiplayerPainTweaks.m_SMPDamage );
 
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );
        
        // Decrement count of bullets in current clip
        DecrementAmmo();

		return TRUE;
	}
}

//=========================================================================

xbool weapon_dual_smp::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool bHitLiving )
{
    vector3 InitPos;
    
    if (!GetFiringBonePosition(InitPos))
        return FALSE;
    
    vector3 TargetVector   = Target - InitPos ;
    radian3 TargetRot(TargetVector.GetPitch(), TargetVector.GetYaw(), 0.0f );
    
    DegradeAim( TargetRot, R_1*fDegradeMultiplier, InitPos, Owner );

    base_projectile* BP;
    ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return FALSE;

    pain_handle PainHandle( xfs("%s_%s",pOwnerObject->GetLogicalName(),GetLogicalName()) );
    tweak_handle SpeedTweak( xfs("%s_SPEED",GetLogicalName()) );

    BP = CreateBullet( GetLogicalName(), InitPos, TargetRot, BaseVelocity, Owner, PainHandle, AMMO_PRIMARY, bHitLiving, FIRE_POINT_DEFAULT );

    //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    if( g_AudioMgr.IsValidVoiceId( m_LoopVoiceId ) )
    {
        g_AudioMgr.Release( m_LoopVoiceId, g_DualSMPReleaseTime );
        m_LoopVoiceId = 0;
    }


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

//==============================================================================

void weapon_dual_smp::ProcessSfx( void )
{
}

//==============================================================================

void weapon_dual_smp::OnAdvanceLogic( f32 DeltaTime )
{
    new_weapon::OnAdvanceLogic( DeltaTime );

    // Are we in zoom mode.
    if( IsZoomEnabled() )
    {
        ASSERT( m_CurrentRenderState == RENDER_STATE_PLAYER );
           
        // Get the player.
        player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );
        const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();

        if( m_ZoomState == WEAPON_STATE_ZOOM_IN )
        {
            // Zooming in.
            m_CurrentViewX -= m_ViewChangeRate * DeltaTime;
            m_CurrentViewX = fMax( m_CurrentViewX , m_ZoomFOV );     
        }
        else
        {
            // Zooming Out.
            m_CurrentViewX += m_ViewChangeRate * DeltaTime;
            m_CurrentViewX = fMin( m_CurrentViewX , OriginalViewInfo.XFOV );     
            if( m_CurrentViewX >= OriginalViewInfo.XFOV )
                ClearZoom();
        }
    }
}

//==============================================================================

void weapon_dual_smp::BeginAltFire ( void )
{
    if( m_LoopVoiceId == 0 )
    {
        if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            // Play 2d ( ::OnTransform sound update will be ignored)
            m_LoopVoiceId = g_AudioMgr.Play( "SMP_Fire" );
        }                
        else          
        {      
            // Play 3d ( ::OnTransform will update the sound position )
            m_LoopVoiceId = g_AudioMgr.Play( "SMP_Fire", 
                GetL2W().GetTranslation(), 
                GetZone1(), 
                TRUE );
        }

        g_AudioManager.NewAudioAlert( m_LoopVoiceId, 
                                      audio_manager::GUN_SHOT, 
                                      GetPosition(), 
                                      GetZone1(), 
                                      GetGuid() );
    }
}

//==============================================================================

void weapon_dual_smp::EndAltFire ( void )
{
    if( m_LoopVoiceId )
    {
        // Stop looping sfx
        g_AudioMgr.Release( m_LoopVoiceId, g_DualSMPReleaseTime );
        m_LoopVoiceId = 0;
    
        // Play wind down sfx
        if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            // Play 2d
            g_AudioMgr.Play( "SMP_Fire_OS" );
        }
        else
        {
            // Play 3d
            g_AudioMgr.Play( "SMP_Fire_OS", 
                             GetL2W().GetTranslation(), 
                             GetZone1(), 
                             TRUE );
        }                
    }
}

//==============================================================================

void weapon_dual_smp::OnMove( const vector3& NewPos )
{   
    if( m_LoopVoiceId )
    {
        g_AudioMgr.SetPosition( m_LoopVoiceId, NewPos, GetZone1() );
    }

    new_weapon::OnMove( NewPos );
}

//==============================================================================

void weapon_dual_smp::OnTransform( const matrix4& L2W )
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.SetPosition( m_LoopVoiceId, L2W.GetTranslation(), GetZone1() );
    }

    new_weapon::OnTransform( L2W );
}

//==============================================================================

xbool weapon_dual_smp::CanIntereptPrimaryFire( s32 nFireAnimIndex )
{
    (void)nFireAnimIndex;
    return TRUE;
}

//==============================================================================

void weapon_dual_smp::BeginPrimaryFire ( void )
{
    if( m_LoopVoiceId == 0 )
    {
        if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            // Play 2d ( ::OnTransform sound update will be ignored)
            m_LoopVoiceId = g_AudioMgr.Play( "SMP_Fire" );
        }                
        else          
        {      
            // Play 3d ( ::OnTransform will update the sound position )
            m_LoopVoiceId = g_AudioMgr.Play( "SMP_Fire", 
                                                GetL2W().GetTranslation(), 
                                                GetZone1(), 
                                                TRUE );
        }
                        
        g_AudioManager.NewAudioAlert( m_LoopVoiceId, 
                                        audio_manager::GUN_SHOT, 
                                        GetPosition(), 
                                        GetZone1(), 
                                        GetGuid() );
    }
}

//==============================================================================

void weapon_dual_smp::EndPrimaryFire ( void )
{
    if( m_LoopVoiceId )
    {
        // Stop looping sfx
        g_AudioMgr.Release( m_LoopVoiceId, g_DualSMPReleaseTime );
        m_LoopVoiceId = 0;
    
        // Play wind down sfx
        if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            // Play 2d
            g_AudioMgr.Play( "SMP_Fire_OS" );
        }
        else
        {
            // Play 3d
            g_AudioMgr.Play( "SMP_Fire_OS", 
                             GetL2W().GetTranslation(), 
                             GetZone1(), 
                             TRUE );
        }                
    }
}

//==============================================================================

void weapon_dual_smp::ReleaseAudio( void )
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, g_DualSMPReleaseTime );
        m_LoopVoiceId = 0;
    }
}

//==============================================================================

void weapon_dual_smp::OnEnumProp( prop_enum& List )
{
    new_weapon::OnEnumProp( List );

    List.PropEnumHeader  ( "SMP", "", 0 );
    List.PropEnumAngle   ( "SMP\\Zoom FOV", "Whats the field of view when this weapon is zoomed in", 0 );
    List.PropEnumFloat   ( "SMP\\View Change Rate", "How fast is the view going to change.", 0 );
}

//==============================================================================

xbool weapon_dual_smp::OnProperty( prop_query& rPropQuery )
{
    if( new_weapon::OnProperty( rPropQuery ) )
        return TRUE;

    if( rPropQuery.IsVar( "SMP\\Zoom FOV" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarAngle( m_ZoomFOV );
        }
        else
        {
            m_ZoomFOV = rPropQuery.GetVarAngle();
        }    
        return TRUE;
    }

    if( rPropQuery.VarFloat( "SMP\\View Change Rate", m_ViewChangeRate ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

xbool weapon_dual_smp::GetFlashlightTransformInfo( matrix4& incMatrix, vector3& incVect )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        return new_weapon::GetFlashlightTransformInfo( incMatrix, incVect );
    }
    else
    {
        incMatrix = GetL2W();
        incVect.Set( 0.0f, 4.0f, -45.0f );
        return TRUE;
    }
}
