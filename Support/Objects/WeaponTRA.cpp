//=========================================================================
// WEAPON TRA (Bouncy Ball Gun) or NAW (New Alien Weapon)
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileEnergy.hpp"
#include "WeaponTRA.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\Projector.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "render\LightMgr.hpp"
#include "Player.hpp"
#if !defined(X_EDITOR)
#include "NetworkMgr/NetworkMgr.hpp"
#endif
#include "Gamelib/DebugCheats.hpp"

//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================
f32     g_TRAReleaseTime    = 0.050f;

tweak_handle TRA_GainSecondsTweak ( "TRA_GainSeconds" );

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_tra_desc : public object_desc
{

    weapon_tra_desc( void ) : object_desc( object::TYPE_WEAPON_TRA, 
                                        "TRA",
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
        return new weapon_tra;
    }

} s_Weapon_tra_Desc;

//=========================================================================

const object_desc&  weapon_tra::GetTypeDesc     ( void ) const
{
    return s_Weapon_tra_Desc;
}

//=========================================================================

const object_desc&  weapon_tra::GetObjectType   ( void )
{
    return s_Weapon_tra_Desc;
}

//=========================================================================

weapon_tra::weapon_tra( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_MSN;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 50;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 50;
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
    
    m_CurrentWaitTime       = 0.0f;
    m_ReloadWaitTime        = 5.0f;
    
    m_LoopVoiceId           = 0;
    m_SecondaryFireBaseDamage   = 20.0f;
    m_SecondaryFireMaxDamage    = 35.0f;
    m_SecondaryFireSpeed        = 3000.0f;
    m_SecondaryFireBaseForce    = 10.0f;
    m_SecondaryFireMaxForce     = 20.0f;

    m_Item = INVEN_WEAPON_TRA;

    m_hMuzzleFXPrimary.SetName( PRELOAD_FILE("mhg_muzzleflash_000.fxo") );
    m_hMuzzleFXSecondary.SetName( PRELOAD_FILE("mhg_muzzleflash_000.fxo") );

    // initialize time
    m_LastUpdateTime = (f32)x_GetTimeSec();

    m_bIsAltFiring = FALSE;    
    m_LastAmmoBurnTime = m_LastUpdateTime;
    m_AmmoBurned = 0;
    m_AutoSwitchRating = 255;
}

//=========================================================================

weapon_tra::~weapon_tra()
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }    
}

//===========================================================================

void weapon_tra::OnEnumProp( prop_enum& PropEnumList )
{
    new_weapon::OnEnumProp( PropEnumList );    

    PropEnumList.PropEnumHeader  ( "TRA", "Bouncy Ball Gun Weapon", 0 );
    PropEnumList.PropEnumExternal( "TRA\\Bullet Audio Package",   "Resource\0audiopkg", "The audio package associated with this weapon's bullets.", 0 );

    PropEnumList.PropEnumExternal( "TRA\\Primary Fire Effect",    "Resource\0fxo",      "The particle effect for the primary fire", 0 );
    PropEnumList.PropEnumExternal( "TRA\\Secondary Fire Effect",  "Resource\0fxo",      "The particle effect for the secondary fire", 0 );    
    PropEnumList.PropEnumExternal( "TRA\\Wall Bounce Efect",      "Resource\0fxo",      "The particle effect for when primary projectile hits a wall", 0 );

    PropEnumList.PropEnumFloat   ( "TRA\\Reload Wait Time",           "How long to wait before the TRA starts reloading.", 0 );
    PropEnumList.PropEnumFloat   ( "TRA\\Secondary Fire Base Damage", "The minimum amount of damage the secondary fire will cause.", 0 );
    PropEnumList.PropEnumFloat   ( "TRA\\Secondary Fire Max Damage",  "The maximum amount of damage the secondary fire will cause.", 0 );
    PropEnumList.PropEnumFloat   ( "TRA\\Secondary Fire Speed",       "How fast is the secondary fire going to travel.", 0 );
    PropEnumList.PropEnumFloat   ( "TRA\\Secondary Fire Base Force",  "The minimum force the secondary fire going to emmit when it hits something.", 0 );
    PropEnumList.PropEnumFloat   ( "TRA\\Secondary Fire Max Force",   "The maximum force is the secondary fire going to emmit when it hits something.", 0 );
}

//===========================================================================

xbool weapon_tra::OnProperty( prop_query& PropQuery )
{
    if ( new_weapon::OnProperty( PropQuery ) )
    {
        return TRUE;
    }

    if(  new_weapon::OnProperty( PropQuery ) )
        return TRUE;

    if(  PropQuery.VarFloat( "TRA\\Reload Wait Time",            m_ReloadWaitTime ) )
        return TRUE;

    if(  PropQuery.VarFloat( "TRA\\Secondary Fire Base Damage",  m_SecondaryFireBaseDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "TRA\\Secondary Fire Max Damage",   m_SecondaryFireMaxDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "TRA\\Secondary Fire Speed",        m_SecondaryFireSpeed ) )
        return TRUE;

    if(  PropQuery.VarFloat( "TRA\\Secondary Fire Base Force",   m_SecondaryFireBaseForce ) )
        return TRUE;

    if(  PropQuery.VarFloat( "TRA\\Secondary Fire Max Force",    m_SecondaryFireMaxForce ) )
        return TRUE;
    
    // External
    if( PropQuery.IsVar( "TRA\\Bullet Audio Package" ) )
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
    if( PropQuery.IsVar( "TRA\\Primary Fire Effect" ) )
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
    if( PropQuery.IsVar( "TRA\\Secondary Fire Effect" ) )
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
    if( PropQuery.IsVar( "TRA\\Wall Bounce Efect") )
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

void weapon_tra::InitWeapon			(   
                                                 const char* pSkinFileName , 
                                                 const char* pAnimFileName , 
                                                 const vector3& rInitPos , 
                                                 const render_state& rRenderState,
                                                 const guid& rParentGuid )
{
	new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos ,rRenderState, rParentGuid);
	
    m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];
}

//===========================================================================

void weapon_tra::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

	m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];    
}

//=========================================================================

void weapon_tra::OnAdvanceLogic( f32 DeltaTime )
{
    f32 currentTime = (f32)x_GetTimeSec();

    if( m_bIdleMode )
    {   
        m_CurrentWaitTime += ((currentTime - m_LastUpdateTime));
        
        if( (m_CurrentWaitTime >= m_ReloadWaitTime) && 
            (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) )
        {
            s32 Ammo = (s32)((m_CurrentWaitTime-m_ReloadWaitTime)/TRA_GainSecondsTweak.GetF32());

            // clamp ammo
            Ammo = MIN( Ammo,   ((m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) - (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount)) );

            // Did the ammo count change.
            if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount != (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount+Ammo) )
            {
                // Update the ammo and set the time back.
                m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount += Ammo;
                m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip    = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
                m_CurrentWaitTime = m_ReloadWaitTime;
            }
        }        
    }

    m_LastUpdateTime = (f32)x_GetTimeSec();

    new_weapon::OnAdvanceLogic( DeltaTime );
}

//==============================================================================

void weapon_tra::SetupRenderInformation( )
{    
    new_weapon::SetupRenderInformation( );
}

//=========================================================================

xbool weapon_tra::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;

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
        pain_handle PainHandle( xfs("%s_%s_PRIMARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

        BP = CreateBullet( GetLogicalName(), InitPos, InitRot, BaseVelocity, Owner, PainHandle );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // Decrement count of bullets in current clip
        DecrementAmmo();

        return TRUE;
    }
}

//=========================================================================

xbool weapon_tra::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;

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
        ASSERT( m_WeaponAmmo[ AMMO_SECONDARY ].m_ProjectileTemplateID >= 0 );

        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;
        pain_handle PainHandle( xfs("%s_%s_SECONDARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

        BP = CreateBullet( GetLogicalName(), InitPos, InitRot, BaseVelocity, Owner, PainHandle, AMMO_SECONDARY );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // Decrement count of bullets in current clip
        DecrementAmmo( AMMO_SECONDARY );

        return TRUE;
    }
}

//=========================================================================

xbool weapon_tra::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
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
//        ((energy_projectile*)pBaseProjectile)->LoadEffect( m_PrimaryProjectileFx.GetName(), StartPosition, Rot );

        // Create the Meson Projectile
//        guid ProjectileID = CREATE_NET_OBJECT( energy_projectile::GetObjectType(), netobj::TYPE_TRA_1ST );
        energy_projectile* pProjectile = (energy_projectile*) g_ObjMgr.GetObjectByGuid( NULL_GUID );
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

        // Decrement count of bullets in current clip
        DecrementAmmo();

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
                audio_manager::GUN_SHOT, GetPosition(), 
                GetZone1(), GetGuid() );
        }

        return TRUE;
    }
}

//=========================================================================

xbool weapon_tra::FireNPCSecondaryProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{
    ( void ) BaseVelocity;
    ( void ) Target;
    ( void ) Owner;
    ( void ) fDegradeMultiplier;
    ( void ) isHit;

    return FALSE;
}

//=========================================================================

xbool weapon_tra::FireGhostPrimary( s32 iFirePoint, xbool , vector3& )
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
    if( m_FactionFireSfx[ BitIndex ] != -1 )
    {
        voice_id VoiceID = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ), GetPosition(), GetZone1(), TRUE );
        g_AudioManager.NewAudioAlert( VoiceID, audio_manager::GUN_SHOT, GetPosition(), GetZone1(), GetGuid() );
    }

    return TRUE;
}

//=========================================================================

xbool weapon_tra::FireGhostSecondary( s32 iFirePoint )
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
    voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Fire_Charge",GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, GetPosition(), GetZone1(), GetGuid() );

    return TRUE;
}

//==============================================================================

void weapon_tra::ProcessSfx( void )
{

}

//==============================================================================

void weapon_tra::BeginAltFire( void )
{
    if( (m_LoopVoiceId == 0) && (m_CurrentRenderState == RENDER_STATE_PLAYER) )
    {
        m_LoopVoiceId = g_AudioMgr.Play( "SMP_Fire" );
        g_AudioManager.NewAudioAlert( m_LoopVoiceId, 
            audio_manager::GUN_SHOT, 
            GetPosition(), 
            GetZone1(), 
            GetGuid() );
    }
}

//==============================================================================

void weapon_tra::EndAltFire( void )
{
    if( m_LoopVoiceId )
    {
        // Stop looping sfx
        g_AudioMgr.Release( m_LoopVoiceId, g_TRAReleaseTime );
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

void weapon_tra::BeginAltRampUp( void )
{
    m_AmmoBurned = 0;
    m_bIsAltFiring = TRUE;
    m_LastAmmoBurnTime = (f32)x_GetTimeSec();    
}

//==============================================================================
void weapon_tra::EndAltRampUp( xbool bGoingIntoHold, xbool bSwitchingWeapon )
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

void weapon_tra::EndAltHold( xbool bSwitchingWeapon )
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

void weapon_tra::MoveMuzzleFx( void )
{
    new_weapon::MoveMuzzleFx();

    // KSS -- FIXME -- correct bones need to be set up for player and NPC
    if( m_CurrentRenderState == RENDER_STATE_NPC )
        return;
    
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

xbool weapon_tra::CanReload( const ammo_priority& Priority )
{
    (void)Priority;
    return FALSE;
}

//==============================================================================

xbool weapon_tra::CanFire( xbool bIsAltFire )
{
    (void) bIsAltFire;
    s32 AmmoCount = 0;

    // REMEMBER: alt and primary fire use the same ammo
    ammo_priority AmmoPriority = GetPrimaryAmmoPriority();
    AmmoCount = GetAmmoCount( AmmoPriority );

    // normal fire will fire 1 round
    return( AmmoCount >= 1 );
}

//==============================================================================

void weapon_tra::BeginPrimaryFire ( void )
{   
}

//==============================================================================

void weapon_tra::EndPrimaryFire ( void )
{
}

//==============================================================================

void weapon_tra::ReleaseAudio( void )
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }
}

//==============================================================================
xbool weapon_tra::CanSwitchIdleAnim( void )
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
void weapon_tra::BeginSwitchTo( void )
{    
}

//==============================================================================

void weapon_tra::BeginSwitchFrom( void )
{    
}

//==============================================================================

void weapon_tra::BeginIdle( void )
{
    m_bIdleMode = TRUE;
    m_CurrentWaitTime = 0.0f;
}

//==============================================================================

void weapon_tra::EndIdle( void )
{
    m_bIdleMode         = FALSE;
}

//==============================================================================

void weapon_tra::ClearAmmo( const ammo_priority& rAmmoPriority  )
{
    // KSS -- as per design, manned turrets do not run out of ammo
    (void)rAmmoPriority;
}

//===========================================================================

void weapon_tra::DecrementAmmo( const ammo_priority& rAmmoPriority, const s32& nAmt)
{
    // KSS -- as per design, manned turrets do not run out of ammo
    (void)rAmmoPriority;
    (void)nAmt;
}

//===========================================================================

void weapon_tra::FillAmmo( void )
{
    // make sure ammo is constantly full - TRA works a bit differently
    if( DEBUG_INFINITE_AMMO )
    {
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    }
}

//===========================================================================
