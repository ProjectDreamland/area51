#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileBullett.hpp"
#include "WeaponDualShotgun.hpp"
#include "Debris\debris_mgr.hpp"
#include "Objects\Projector.hpp"
#include "render\LightMgr.hpp"
#include "Objects\Player.hpp"
#include "Objects\ParticleEmiter.hpp"


//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================

static const s32 s_Num_Pellets = 4;
static const s32 s_MaxPotentialTargets = 16;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_dual_shotgun_desc : public object_desc
{

    weapon_dual_shotgun_desc( void ) : object_desc( object::TYPE_WEAPON_SHOTGUN, 
                                        "Dual Shotguns",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
										object::ATTR_NEEDS_LOGIC_TIME		|
                                        object::ATTR_SOUND_SOURCE			|
                                        object::ATTR_RENDERABLE				|
 										object::ATTR_COLLISION_PERMEABLE,
                                        //FLAGS_GENERIC_EDITOR_CREATE |
                                        FLAGS_IS_DYNAMIC

										) {}

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_dual_shotgun;
    }

} s_Weapon_Dual_Shotgun_Desc;

//=========================================================================

const object_desc&  weapon_dual_shotgun::GetTypeDesc     ( void ) const
{
    return s_Weapon_Dual_Shotgun_Desc;
}

//=========================================================================

const object_desc& weapon_dual_shotgun::GetObjectType   ( void )
{
    return s_Weapon_Dual_Shotgun_Desc;
}

//=========================================================================

weapon_dual_shotgun::weapon_dual_shotgun( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_SHOTGUN;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 64;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 2;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
	
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

    m_NPCMuzzleSoundFx  = "Shotgun_Primary_Fire";

	//Both primary and secondary fires use same ammo, so secondary ammo is only initialized
	//in the constructor of new_weapon and set to undefined.
    
    //set aim degradation
/*    m_AimDegradePrimary = 0.4f;
    m_AimDegradeSecondary = 0.6f;
    m_AimRecoverSpeed = 0.4f;*/

    m_Item = INVEN_WEAPON_DUAL_SHT;

    m_hMuzzleFXPrimary.SetName( PRELOAD_FILE("Muzzleflash_1stperson_Shotgun_000.fxo") );
    m_hMuzzleFXSecondary.SetName( PRELOAD_FILE("Muzzleflash_1stperson_Shotgun_002.fxo") );

    m_AutoSwitchRating = 7;
}

//=========================================================================

weapon_dual_shotgun::~weapon_dual_shotgun()
{
}

//=========================================================================

void weapon_dual_shotgun::InitWeapon			(   
                                                 const char* pSkinFileName , 
                                                 const char* pAnimFileName , 
                                                 const vector3& rInitPos , 
                                                 const render_state& rRenderState,
                                                 const guid& rParentGuid )
{
	new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos ,rRenderState, rParentGuid);
	
	m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];

    // Setup NPC anim controller to position both weapons inside the same geometry
    InitNPCDualAnimController( &m_DualAnimController, "B_WPN_2SH_Root_Left", "B_WPN_2SH_Root_Right" );
}

//=========================================================================

void weapon_dual_shotgun::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

	m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];
	
    // Setup NPC anim controller to position both weapons inside the same geometry
    InitNPCDualAnimController( &m_DualAnimController, "B_WPN_2SH_Root_Left", "B_WPN_2SH_Root_Right" );
}

//=========================================================================

xbool weapon_dual_shotgun::HasLOS( const vector3& A, const vector3& B )
{
    g_CollisionMgr.LineOfSightSetup( GetGuid(), A, B );
    g_CollisionMgr.AddToIgnoreList( GetGuid() );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                object::ATTR_BLOCKS_SMALL_PROJECTILES, 
                                object::ATTR_COLLISION_PERMEABLE );
    if( g_CollisionMgr.m_nCollisions != 0 )
    {
        return FALSE;
    }
    return TRUE;
}

//===========================================================================

xbool weapon_dual_shotgun::CanReload( const ammo_priority& Priority )
{
    (void) Priority;

    // cannot reload Dual SMPs
    return FALSE;
}

//=========================================================================

void weapon_dual_shotgun::HandleSweeperCone( const vector3& Pos,
                                        const vector3& AimDir,
                                        const radian3& Rot,
                                        const pain_handle& PainHandle,
                                        radian ConeAngle, 
                                        f32 Distance )
{
    (void)AimDir;
    
    Distance = 100 * 20;
    vector3 Dir(0,0,1);
    Dir.Rotate( Rot );
    Dir.Normalize();

    bbox Box;
    Box.Clear();
    Box.AddVerts( &Pos, 1 );

    vector3 Pt[4];
    s32     i;

    for (i=0;i<4;i++)
    {
        vector3 Arm(0,0,Distance);
        Arm.RotateY( ConeAngle );
        Arm.RotateZ( PI/2.0f*i );

        Arm.Rotate( Rot );

        Pt[i] = Arm + Pos;        
    }

    Box.AddVerts( Pt, 4 );

    g_ObjMgr.SelectBBox( object::ATTR_ALL, Box, object::TYPE_ALIEN_GLOB );

    slot_id iSlot = g_ObjMgr.StartLoop();
    object* pPotentialTarget[s_MaxPotentialTargets];
    s32     nPotentialTargets = 0;
    while ((SLOT_NULL != iSlot) && (nPotentialTargets < s_MaxPotentialTargets))
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( iSlot );
        ASSERT( pObj );
        vector3 ObjPos = pObj->GetL2W().GetTranslation();        
        vector3 Delta  = ObjPos - Pos;
        Delta.Normalize();

        f32 Dot = Delta.Dot(Dir);
        f32 Ang = x_acos( Dot );

        if (Ang <= ConeAngle)
        {
            pPotentialTarget[ nPotentialTargets++ ] = pObj;
        }

        iSlot = g_ObjMgr.GetNextResult( iSlot );
    }
    g_ObjMgr.EndLoop();

    s32 nHit = 0;
    for (i=0;i<nPotentialTargets;i++)
    {
        ASSERT( pPotentialTarget[i] );
        vector3 ObjPos = pPotentialTarget[i]->GetL2W().GetTranslation();   
        if (HasLOS( Pos,ObjPos ))
        {
            pain Pain;        
            Pain.Setup( PainHandle, GetGuid(), ObjPos );
            Pain.SetDirection( Dir );
            Pain.SetDirectHitGuid( pPotentialTarget[i]->GetGuid() );        
            Pain.SetCustomScalar( 1 );
            Pain.ApplyToObject( pPotentialTarget[i]  );
            nHit++;
        }
    }
#if defined(X_DEBUG) && defined(shird) && defined(X_EDITOR)
    LOG_MESSAGE("Shotgun_Sweeper", "%d potential globs, %d hit by sweeper cone", nPotentialTargets, nHit );
    LOG_FLUSH();
#endif

    g_ObjMgr.SelectBBox( object::ATTR_ALL, Box, object::TYPE_GENERIC_NPC );

    iSlot = g_ObjMgr.StartLoop();
    nPotentialTargets = 0;
    while ((SLOT_NULL != iSlot) && (nPotentialTargets < s_MaxPotentialTargets))
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( iSlot );
        ASSERT( pObj );
        vector3 ObjPos = pObj->GetL2W().GetTranslation();        
        vector3 Delta  = ObjPos - Pos;
        Delta.Normalize();

        f32 Dot = Delta.Dot(Dir);
        f32 Ang = x_acos( Dot );

        if (Ang <= ConeAngle)
        {
            pPotentialTarget[ nPotentialTargets++ ] = pObj;
        }

        iSlot = g_ObjMgr.GetNextResult( iSlot );
    }
    g_ObjMgr.EndLoop();

    nHit = 0;
    for (i=0;i<nPotentialTargets;i++)
    {
        ASSERT( pPotentialTarget[i] );
        vector3 ObjPos = pPotentialTarget[i]->GetL2W().GetTranslation();   
        if (HasLOS( Pos,ObjPos ))
        {
            pain Pain;        
            Pain.Setup( PainHandle, GetGuid(), ObjPos );
            Pain.SetDirection( Dir );
            Pain.SetDirectHitGuid( pPotentialTarget[i]->GetGuid() );        
            Pain.SetCustomScalar( 1 );
            Pain.ApplyToObject( pPotentialTarget[i]  );
            nHit++;
        }
    }
#if defined(X_DEBUG) && defined(shird) && defined(X_EDITOR)
    LOG_MESSAGE("Shotgun_Sweeper", "%d potential generic NPCs, %d hit by sweeper cone", nPotentialTargets, nHit );
    LOG_FLUSH();
#endif
}

//=========================================================================

xbool weapon_dual_shotgun::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    (void)Power;

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{   
		// Get the pain handle
        pain_handle PainHandle;
        GetPainHandle( PainHandle, Owner, TRUE );

        //calculate the direction of the shotgun's pellets.
		vector3 vDir[ s_Num_Pellets ];
		for ( s32 i = 0 ; i < s_Num_Pellets ; i ++ )
		{
            //pick a random point on a small box that is located 10 cm. away.
            f32 Spread = ((i == 0) ? 0.1f : 0.5f );
            f32 rx = x_frand( -Spread , Spread );
            f32 ry = x_frand( -Spread , Spread );
            vDir[i] = vector3( rx, ry , 10.f );

			//rotate that point into model space
			vDir[i].Rotate( InitRot );

			//make a radian3 out of this vector
			radian Pitch , Yaw;
		    vDir[i].GetPitchYaw( Pitch,Yaw );
			radian3 vRot( Pitch , Yaw , 0.f );
		
			//create Bullet
            FireBullet(  InitPos, vRot, BaseVelocity, Owner, TRUE, TRUE, PainHandle, iFirePoint );

		}

        // Sweep the area in front of the gun
        radian Angle = GetTweakRadian( "PLAYER_SHOTGUN_Primary_Sweep_Angle", R_15 );
        f32    Dist  = GetTweakF32   ( "PLAYER_SHOTGUN_Primary_Sweep_Dist", 700 );
        HandleSweeperCone( InitPos, BaseVelocity, InitRot, PainHandle, Angle, Dist );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        //decrement count of bullets in current clip
        DecrementAmmo();

		return TRUE;
	}
}

//=========================================================================

xbool weapon_dual_shotgun::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
	ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );
	ASSERT( m_AltFiringPointBoneIndex[ iFirePoint ] != -1 );

    (void)Power;
    
	//if there weapon is not ready, do nothing.  Shotgun uses primary ammo for both primary and secondary fire
	if( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        // Ammo count can become an odd number, let them fire as long as there is a shell.
        // Secondary fire will just call FireWeaponProtected if there are less than 4 shells.  
        // FireWeaponProtected handles everything else:
        // (i.e. if you fire and only have 1 shell left, it will only fire 1 shell)
        if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip < 4 )
        {
            // if we have an odd number of shells and not enough ammo, fire primary
            return FireWeaponProtected( InitPos, BaseVelocity, Power, InitRot, Owner, iFirePoint );
        }

		s32 PelletMultiplier = 2;

        if( (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip - 2) < 0 )
            PelletMultiplier = 1;

        // Get the pain handle
        pain_handle PainHandle;
        GetPainHandle( PainHandle, Owner, FALSE );

        //calculate the direction of the shotgun's pellets.
        ASSERT( PelletMultiplier <= 2 );
        vector3 pDir[ s_Num_Pellets * 2 ];
		for ( s32 i = 0 ; i < (s_Num_Pellets*PelletMultiplier) ; i ++ )
		{
            //pick a random point on a small box that is located 10 cm. away.
            f32 Spread = ((i == 0) ? 0.1f : 0.5f );
            f32 rx = x_frand( -Spread , Spread );
            f32 ry = x_frand( -Spread , Spread );
            pDir[i] = vector3( rx, ry , 10.f );

			//rotate that point into model space
			pDir[i].Rotate( InitRot );

			//make a radian3 out of this vector
			radian Pitch , Yaw;
		    pDir[i].GetPitchYaw( Pitch,Yaw );
			radian3 vRot( Pitch , Yaw , 0.f );
		
			//create Bullet
            FireBullet( InitPos, vRot, BaseVelocity, Owner, TRUE, FALSE, PainHandle, iFirePoint );
 		}

        // Sweep the area in front of the gun
        radian Angle = GetTweakRadian( "PLAYER_SHOTGUN_Secondary_Sweep_Angle", R_35 );
        f32    Dist  = GetTweakF32   ( "PLAYER_SHOTGUN_Secondary_Sweep_Dist", 400 );
        HandleSweeperCone( InitPos, BaseVelocity, InitRot, PainHandle, Angle, Dist );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // decrement count of bullets in current clip
        DecrementAmmo(AMMO_PRIMARY, PelletMultiplier);

		return TRUE;
	}
	
	return FALSE;
}

//==============================================================================

xbool weapon_dual_shotgun::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{   
	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}
       
    vector3 InitPos;
    
    if (!GetFiringBonePosition(InitPos))
        return FALSE;

    vector3 TargetVector   = Target - InitPos ; 
    radian3 TargetRot(TargetVector.GetPitch(), TargetVector.GetYaw(), 0.0f );

    DegradeAim( TargetRot, R_3*fDegradeMultiplier, InitPos, Owner );

    // Get the pain handle
    pain_handle PainHandle;
    GetPainHandle( PainHandle, Owner, TRUE );

    for (int i = 0; i < s_Num_Pellets; i++)
    { 	
        //pick a random point on a small box that is located 5 cm. away.
        f32 rx = x_frand(-.3f , .3f );
        f32 ry = x_frand(-.3f , .3f );
        vector3 RandomVector( rx, ry , 5.f );

        //rotate that point into model space
        RandomVector.Rotate( TargetRot );

        //make a radian3 out of this vector
        radian3 RandomRot( RandomVector.GetPitch() , RandomVector.GetYaw() , 0.f );

        FireBullet( InitPos, RandomRot, BaseVelocity, Owner, isHit, TRUE, PainHandle, FIRE_POINT_DEFAULT );
    }
    
    //add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    // Play the sound associated with this actor's faction.
    object* pObj = g_ObjMgr.GetObjectByGuid( Owner );
    actor& OwnerActor = actor::GetSafeType( *pObj );

    factions Faction = OwnerActor.GetFaction();
    s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

    if( m_FactionFireSfx[ BitIndex ] != -1 )
    {
        voice_id VoiceID = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ), 
                                            GetPosition(), GetZone1(), TRUE );

        g_AudioManager.NewAudioAlert( VoiceID, 
                                      audio_manager::GUN_SHOT, 
                                      GetPosition(), 
                                      GetZone1(), 
                                      GetGuid() );
    }
     
    return TRUE;
}

//===========================================================================

void weapon_dual_shotgun::GetPainHandle( pain_handle& OutHandle, guid Owner, xbool bIsPrimary )
{
    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return;

    xstring ModeString;
    if( bIsPrimary )
    {
        ModeString = xfs("%s_%s_PRIMARY", pOwnerObject->GetLogicalName(), GetLogicalName());
    }
    else
    {
        ModeString = xfs("%s_%s_SECONDARY", pOwnerObject->GetLogicalName(), GetLogicalName());
    }

    const char* pString = (const char*)ModeString;

    OutHandle.SetName( pString );
}

//===========================================================================

void weapon_dual_shotgun::FireBullet( const vector3& Pos, const radian3& Rot, const vector3& InheritedVelocity, guid Owner, const xbool bHitLiving, const xbool bIsPrimary, pain_handle& PainHandle, s32 iFirePoint )
{
    (void)bIsPrimary;

    base_projectile* BP;
    ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return;

    // There is a template there.  Use it
    BP = CreateBullet( GetLogicalName(), Pos, Rot, InheritedVelocity, Owner, PainHandle, AMMO_PRIMARY, bHitLiving, iFirePoint );
}

//=========================================================================

xbool weapon_dual_shotgun::FireNPCSecondaryProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{
    ( void ) BaseVelocity;
    ( void ) Target;
    ( void ) Owner;
    ( void ) fDegradeMultiplier;
    ( void ) isHit;

    return FALSE;
}

//==============================================================================

void weapon_dual_shotgun::ProcessSfx( void )
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
                        //g_AudioManager.Play( "Shotgun_Primary_Fire", Pos );
                        g_AudioManager.Play( "Shotgun_Primary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                                        GetZone1(), GetGuid() );

                    break;
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_LR:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_LL:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_UR:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_UL:
	                case ANIM_EVENT_SECONDARY_FIRE:
                        //g_AudioManager.Play( "Shotgun_Secondary_Fire", Pos );
                        g_AudioManager.Play( "Shotgun_Secondary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                                        GetZone1(), GetGuid() );
                    break;
                    case ANIM_EVENT_SFX_WEAPONLOOP:
                        g_AudioManager.Play( "Shotgun_Primary_Fire", Pos );
                    break;
                    case ANIM_EVENT_SFX_SWITCH_TO:
                        g_AudioManager.Play( "Shotgun_Ready", Pos );
                    break;
                    case ANIM_EVENT_SFX_SWITCH_FROM:
                        g_AudioManager.Play( "Shotgun_Unready", Pos );
                    break;
                    case ANIM_EVENT_SFX_RELOAD:
                        g_AudioManager.Play( "Shotgun_Reload", Pos );
                    break;
                    case ANIM_EVENT_SFX_ALT_RELOAD:
                        g_AudioManager.Play( "Shotgun_Reload", Pos );
                    break;
                    default:
                        x_DebugMsg( "Anim Event: Shotgun Event[%d]\n", Event.GetInt( anim_event::INT_IDX_OLD_TYPE ) );
                    break;
                }
            }
        }
    }
*/
}

//==============================================================================

void weapon_dual_shotgun::Reload( const ammo_priority& Priority )
{
    // For GameDay, we do not want Zach to run out of ammo.  Remove the next line after GameDay.
    //m_WeaponAmmo[Priority].m_AmmoAmount = m_WeaponAmmo[Priority].m_AmmoMax;
    s32 nNewAmmoCount   = iMin( m_WeaponAmmo[Priority].m_AmmoAmount, m_WeaponAmmo[Priority].m_AmmoPerClip );

    // KSS -- As per design, make shotgun reload ammo 2 shells per anim.
    if( m_CurrentRenderState == RENDER_STATE_NPC )
    {
        nNewAmmoCount       = iMin( nNewAmmoCount, m_WeaponAmmo[Priority].m_AmmoPerClip );
    }
    else
    {    
        nNewAmmoCount       = iMin( nNewAmmoCount, m_WeaponAmmo[Priority].m_AmmoInCurrentClip+2);
    }

    m_WeaponAmmo[Priority].m_AmmoInCurrentClip = nNewAmmoCount;
}

//==============================================================================

xbool weapon_dual_shotgun::ContinueReload( void )
{
    if( (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip) &&
        (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount) )
        return TRUE;
    else
        return FALSE;
}

//==============================================================================
xbool weapon_dual_shotgun::CanFire( xbool bIsAltFire )
{
    s32 AmmoCount = 0;

    ammo_priority AmmoPriority 
        = bIsAltFire 
        ? GetSecondaryAmmoPriority() 
        : GetPrimaryAmmoPriority();

    AmmoCount = GetAmmoCount( AmmoPriority );

    if( bIsAltFire )
    {
        // Ammo count can become an odd number, let them fire as long as there is a shell.
        // Secondary fire will just call FireWeaponProtected if there are less than 4 shells.  
        // FireWeaponProtected handles everything else:
        // (i.e. if you fire and only have 1 shell left, it will only fire 1 shell)
        return TRUE;
    }

    // primary takes 2 shells
    // Ammo count can become an odd number, let them fire primary as long as there is a shell.
    // FireWeaponProtected handles everything:
    // (i.e. if you fire and only have 1 shell left, it will only fire 1 shell)
    return( AmmoCount > 0 );  
}

//==============================================================================

xbool weapon_dual_shotgun::GetFlashlightTransformInfo( matrix4& incMatrix, vector3& incVect )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        return new_weapon::GetFlashlightTransformInfo( incMatrix, incVect );
    }
    else
    {
        incMatrix = GetL2W();
        incVect.Set( -6.0f, 5.5f, -79.0f );
        return TRUE;
    }
}

