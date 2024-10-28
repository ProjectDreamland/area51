//=========================================================================
// Weapon Mutation
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "WeaponMutation.hpp"
#include "ProjectileBullett.hpp"
#include "render\LightMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "objects\Render\PostEffectMgr.hpp"
#include "objects\ProjectileHoming.hpp"
#include "objects\ProjectileMutantContagion.hpp"
#include "Objects\ProjectileMutantTendril.hpp"
#include "player.hpp"
#include "Characters\character.hpp"
#include "Characters\GenericNPC\GenericNPC.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Objects\Corpse.hpp"
#if !defined(X_EDITOR)
#include "NetworkMgr/NetworkMgr.hpp"
#endif
#include "Gamelib/DebugCheats.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"
#include "objects\ProjectileMutantParasite2.hpp"
#include "MiscUtils\SimpleUtils.hpp"

//=========================================================================
// DEFINES and CONSTS
//=========================================================================

f32 s_HitShakeTime     = 0.8f;
f32 s_HitShakeAmount   = 1.0f;
f32 s_HitShakeSpeed    = 1.5f;
xbool MELEE_SHAKE_CAM   = TRUE;

f32 COLLISION_CHECK_SIZE                =     30.0f;

// Damage/Force
f32 MUTATION_MELEE_DAMAGE               =    500.0f;
f32 MUTATION_MELEE_FORCE                =    225.0f;
f32 PAIN_RADIUS                         =    150.0f; // 1.5 meters
f32 SECONDARY_PAIN_RADIUS               =    50.0f;  // .5 meters

xbool SCALE_PAIN                        =     FALSE;

// FOV stuff
radian ZOOM_FOV                         =      R_28;
xbool USE_FOV                           =     FALSE;


xbool USE_PLAYER_POSITION               =      FALSE;
xbool USE_UPWARD_FORCE                  =      FALSE;
radian HIT_ANGLE                        =      R_35;

// input feedback (stick shake)
xbool DO_FEEDBACK                       =      TRUE;
xbool RAMP_FEEDBACK                     =      TRUE;

xbool USE_MUTATION_ATTACK_COSTS         =      TRUE;  // do we charge for using contagions/parasites?

xbool g_RenderTendrilConstraints        = FALSE;
xbool g_RenderTendrilConstraintsTRUE    = FALSE;
xbool g_RenderMeleeSphere               = FALSE;
f32   g_ShockFactor                     = 30.0f;
f32   g_CorpseLiftHeight                = 40.0f;
f32   g_TendrilKeepUpElasticity         = 0.75f;
f32   g_TendrilKeepUpDistance           = 5.0f;
f32   g_TendrilConstraintSize           = 20.0f;

tweak_handle PrimaryFireCostTweak      ("MUTATION_PrimaryFireCost");    // Parasite or mutation primary fire mutagen cost.
tweak_handle SecondaryFireCostTweak    ("MUTATION_SecondaryFireCost");  // Contagion or mutation secondary fire mutagen cost.

tweak_handle TendrilLiftHeightTweak    ("MUTATION_TendrilLiftHeight");  // Height in cm that the body is lifted off the ground by the melee tendrils.
tweak_handle TendrilThrowForceTweak    ("MUTATION_TendrilThrowForce");  // How hard do we throw the corpse when our melee tendrils are done with it.
tweak_handle TendrilReachDistanceTweak ("PLAYER_TendrilReachDistance");


//=========================================================================
// EXTERNALS
//=========================================================================

extern s32 g_Difficulty;
extern const char* DifficultyText[];

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_mutation_desc : public object_desc
{

    weapon_mutation_desc( void ) : object_desc( object::TYPE_WEAPON_MUTATION, 
                                        "Mutation ",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
                                        object::ATTR_NEEDS_LOGIC_TIME       |
                                        object::ATTR_RENDERABLE				|
                                        object::ATTR_SOUND_SOURCE,
                                        //FLAGS_GENERIC_EDITOR_CREATE         |
                                        FLAGS_IS_DYNAMIC    
                                        )
    {

    }

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_mutation;
    }

} s_Weapon_Mutation_Desc;


//=========================================================================

const object_desc&  weapon_mutation::GetTypeDesc     ( void ) const
{
    return s_Weapon_Mutation_Desc;
}

//=========================================================================

const object_desc&  weapon_mutation::GetObjectType   ( void )
{
    return s_Weapon_Mutation_Desc;
}


//=========================================================================

weapon_mutation::weapon_mutation( void ) :
    m_ViewChangeRate            ( 1.0f      ),
    m_CurrentViewX              ( R_0       ),    
    m_bMeleeComplete            ( FALSE     ),    
    m_HostPlayerGuid            ( 0         ),
    m_nPrimaryParasitesPerShot  ( 3         ),
    m_nSecondaryParasitesPerShot( 1 		),    
    m_MutagenTemplateID         ( -1        ),
    m_MeleeState                ( player::ANIM_STATE_MELEE ),
    m_LiftHeight                ( 0.0f )
{
    //initialize the ammo structures.
    m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_MUTATION;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 64;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 6;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;

    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

    m_NPCMuzzleSoundFx  = "_Primary_Fire";

    //set aim degradation
    m_AimDegradePrimary = 0.5f;
    m_AimRecoverSpeed = 0.5f;

    m_Item           = INVEN_WEAPON_MUTATION;
    m_MutationZoomSound   = 0;

    m_nZoomSteps = 0;
    m_ContagionGuid = 0;

    for( s32 i = 0; i < MAX_TENDRIL_CONSTRAINTS; i++ )
    {
        m_iConstraint[i] = -1;
        m_iBone[i]       = -1;
        m_ConstraintPosition[i].Zero();
        m_ConstraintOffset[i].Zero();
    }

    m_AutoSwitchRating = 255;
}

//=========================================================================

weapon_mutation::~weapon_mutation()
{
}

//===========================================================================

void weapon_mutation::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

    if ( rRenderState == RENDER_STATE_PLAYER )
    {
        // SetState( NO_ZOOM_WEAPON_STATE );
    }
}

//==============================================================================
void weapon_mutation::OnRenderTransparent(void)
{
#ifndef X_RETAIL
    if( g_RenderTendrilConstraints )
    {
        if( m_CorpseGuid )
        {
            if( g_RenderTendrilConstraintsTRUE )
            {
                corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);
                if( pCorpse )
                {
                    pCorpse->OnDebugRender();
                }
            }
            else
            {
                // render all OUR constraint positions (i.e. where we think they are)
                for( s32 i = 0; i < MAX_TENDRIL_CONSTRAINTS; i++ )
                {
                    if( i == CORPSE_PIN )
                    {
                        draw_Sphere( m_ConstraintPosition[i], g_TendrilConstraintSize, XCOLOR_YELLOW );
                    }
                    else
                    {
                        // this is where WE think we are
                        draw_Sphere( m_ConstraintPosition[i], g_TendrilConstraintSize, XCOLOR_BLUE );
                    }
                }
            }
        }        
    }

    if( g_RenderMeleeSphere )
    {
        object *playerObject = g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid );

        // if we have a NULL pointer, get out
        if( !playerObject )
        {
            return;
        }

        // get player pointer
        player* pPlayer = (player*)playerObject;

        vector3 MyPos ( pPlayer->GetEyesPosition() );
        radian3 RadRot;
        pPlayer->GetEyesPitchYaw( RadRot.Pitch, RadRot.Yaw );
        vector3 theRotation( RadRot );
        theRotation.NormalizeAndScale( PAIN_RADIUS );
        vector3 TargetPos = MyPos + theRotation;

        draw_Line(MyPos, TargetPos);
        draw_Sphere(TargetPos, PAIN_RADIUS);

        // now look for anything damageable because we found nothing living
        g_CollisionMgr.SphereSetup( pPlayer->GetGuid(), MyPos, TargetPos, PAIN_RADIUS );
        g_CollisionMgr.AddToIgnoreList( m_HostPlayerGuid );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_DAMAGEABLE, object::ATTR_COLLISION_PERMEABLE);

        draw_Sphere(g_CollisionMgr.m_Collisions[0].Point, 20.0f, XCOLOR_RED);
    }
#endif
}

//==============================================================================

xbool weapon_mutation::GetFiringStartPosition( vector3 &Pos )
{
    if( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] == -1 )
    {
        actor* pActor = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );
        ASSERT( pActor );
        Pos = pActor->GetPositionWithOffset( actor::OFFSET_EYES );
        return TRUE;
    }

    // get the "firepoint" position for this gun
    Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] );
    return TRUE;
}

//==============================================================================

xbool weapon_mutation::GetAltFiringStartPosition( vector3 &Pos )
{
    if( m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] == -1 )
    {
        actor* pActor = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );
        ASSERT( pActor );
        Pos = pActor->GetPositionWithOffset( actor::OFFSET_EYES );
        return TRUE;
    }

    // get the "firepoint" position for this gun
    Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_AltFiringPointBoneIndex[ FIRE_POINT_DEFAULT ] );
    return TRUE;
}

//===========================================================================

xbool weapon_mutation::FireTendril( const vector3& InitPos , const vector3& BaseVelocity, const radian3& InitRot, const guid& Owner, xbool bLeft )
{
    (void)BaseVelocity;
    (void)InitRot;
    (void)InitPos;
    (void)bLeft;

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );

    if( !pOwnerObject )
        return FALSE;

    // Create the tendril Projectile
    guid ProjectileID = CREATE_NET_OBJECT( mutant_tendril_projectile::GetObjectType(), netobj::TYPE_TENDRIL );
    mutant_tendril_projectile *pProjectile = (mutant_tendril_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
    ASSERT( pProjectile );
    
    if( pProjectile )
    {
        // Compute velocity
        //tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
        //vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
        vector3 Velocity( 0.0f , 0.0f , 1200.0f*2 );
        Velocity.Rotate( InitRot );
        Velocity += BaseVelocity;

// KSS -- fixup for flymode.  Since we never get the event to retract, tendrils stay around
#ifndef CONFIG_RETAIL
        mutant_tendril_projectile *pLeftTendril  = (mutant_tendril_projectile *)g_ObjMgr.GetObjectByGuid(m_LeftTendril);
        mutant_tendril_projectile *pRightTendril = (mutant_tendril_projectile *)g_ObjMgr.GetObjectByGuid(m_RightTendril);

        if( pLeftTendril && pRightTendril )
        {
            pLeftTendril->OnExplode();
            pRightTendril->OnExplode();

            ClearCorpseConstraints();
        }
#endif

        // store off tendril guids for retraction later
        if( bLeft )
        {
            m_LeftTendril = pProjectile->GetGuid();
        }
        else
        {
            m_RightTendril = pProjectile->GetGuid();
        }

        if( m_TargetGuid ) 
        {
            pProjectile->SetTarget( m_TargetGuid );            
        }

        pain_handle PainHandle( xfs("%s_ExtremeMelee", pOwnerObject->GetLogicalName(), GetLogicalName()) );

        pain Pain;
        Pain.Setup( PainHandle, m_OwnerGuid, pOwnerObject->GetColBBox().GetCenter() );

        pProjectile->Setup( Owner,
            pOwnerObject->net_GetSlot(),
            InitPos,
            radian3(0.0f,0.0f,0.0f), //InitRot
            Velocity,
            GetZone1(),
            GetZone2(),
            PainHandle,
            bLeft);
    }

    return TRUE;
}

//===========================================================================
xbool weapon_mutation::RetractTendril( xbool bLeft )
{
    mutant_tendril_projectile *pTendril = NULL;
    xbool bValid = FALSE;

    // retract particular tendril
    if( bLeft )
    {
        pTendril = (mutant_tendril_projectile*)g_ObjMgr.GetObjectByGuid(m_LeftTendril);        
    }
    else
    {
        pTendril = (mutant_tendril_projectile*)g_ObjMgr.GetObjectByGuid(m_RightTendril);
    }

    ASSERT(pTendril);
    
    // go ahead and retract it if it exists
    if( pTendril )
    {
        pTendril->RetractTendril(m_OwnerGuid, bLeft);
        bValid = TRUE;
    }

    return bValid;
}

//===========================================================================

xbool weapon_mutation::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    CONTEXT( "mutation::FireWeaponProtected" );

    ( void )InitPos;
    ( void )Power;
    ( void )iFirePoint;

    // If we have a contagion guid then try to detonate it!
    if( TriggerContagion() )
        return FALSE;
        
//    ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );
    object* pOwner = g_ObjMgr.GetObjectByGuid( Owner );
    player* pPlayer = NULL;
    if ( pOwner->GetType() == TYPE_PLAYER )
    {
        pPlayer = (player *)pOwner;
    }

    // can we fire this attack?
    if( !ImposeMutagenCosts( pOwner ) )
    {
        return FALSE;
    }
        
    //otherwise, create new mutation missiles, init their positions, and send them off.
    vector3 StartPosition;
    GetFiringStartPosition( StartPosition );

    guid TargetGuid;
    if ( m_TargetGuid )
    {
        TargetGuid = m_TargetGuid;
    }
    else
    {
        TargetGuid = FindDumbTarget( InitPos, InitRot, Owner );
    }

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return FALSE;

    s32 Difficulty = 0; // Easy

    // xstring to build string for mutagen return difficulty tweak (if available)
    xstring StringPrefix = (const char*)xfs( "%s_%s_1",pOwnerObject->GetLogicalName(),GetLogicalName() );

#ifndef X_EDITOR
    // get real difficulty
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
#endif
    {
        // difficulty only applies in Campaign.
        Difficulty = g_Difficulty;

        // append difficulty.  Online has its own tweaks and Editor should just be Easy.
        StringPrefix += (const char*)xfs( "_%s", DifficultyText[Difficulty] );
    }

    // get a const pointer as an alias so we can pass it in to pain_handle constructor.
    const char* pString = (const char*)StringPrefix;

    pain_handle PainHandle( pString );

    s32 nParasitesPerShot = 3;

#ifndef X_EDITOR
    // Only 2 total parasites in MP
    if (GameMgr.IsGameMultiplayer())
    {
        nParasitesPerShot = 1;
    }
#endif

    s32 i;
    for ( i = 0; i < nParasitesPerShot; ++i )
    {
        // Create the bullet.
        ASSERT(  m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

        guid BulletID = CREATE_NET_OBJECT( mutant_parasite_projectile::GetObjectType(), netobj::TYPE_PARASITE );

        if (BulletID)
        {
            object_ptr<mutant_parasite_projectile>pProj( BulletID );
            ASSERT( pProj.IsValid() );

            pProj->SetZones( GetZones() );

            tweak_handle SpeedTweak( xfs("%s_PRIMARY_SPEED", GetLogicalName()) );
            f32 Speed = 1333.0f;
            if (SpeedTweak.Exists())
                Speed = SpeedTweak.GetF32();
                        
            pProj->Initialize( StartPosition , InitRot, BaseVelocity, Speed, Owner, PainHandle, pOwnerObject->net_GetSlot(), FALSE );            

            pProj->SetTarget( TargetGuid );
        }
        
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );
    }

    return TRUE;
}

//==============================================================================

// MP HACK MP HACK
// MP HACK MP HACK
// MP HACK MP HACK
f32 TweakCostParasite  = 16.0f;
f32 TweakCostContagion = 50.0f;
// MP HACK MP HACK
// MP HACK MP HACK
// MP HACK MP HACK

xbool weapon_mutation::ImposeMutagenCosts( object *pOwner, xbool bIsPrimaryFire )
{
    if( DEBUG_INFINITE_AMMO )
    {
        return TRUE;
    }

    // if this is a player, check mutagen
    if( pOwner->GetType() == TYPE_PLAYER )
    {
        if( USE_MUTATION_ATTACK_COSTS )
        {
            f32 Cost = bIsPrimaryFire ? PrimaryFireCostTweak.GetF32() : SecondaryFireCostTweak.GetF32();

#ifndef X_EDITOR
// MP HACK MP HACK
// MP HACK MP HACK
// MP HACK MP HACK
            if( GameMgr.GetGameType() != GAME_CAMPAIGN )
            {
                Cost = bIsPrimaryFire ? TweakCostParasite : TweakCostContagion;
            }
// MP HACK MP HACK
// MP HACK MP HACK
// MP HACK MP HACK
#endif

            // decrement mutagen if we can
            if( ! ((player*)pOwner)->AddMutagen(-Cost) )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

//==============================================================================
xbool weapon_mutation::CanFire( xbool bIsAltFire )
{
    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
    player* pPlayer = NULL;

    // something's wrong, bail.
    if( !pOwner )
    {
        ASSERT(0);
        return FALSE;
    }

    if( pOwner->GetType() == TYPE_PLAYER )
    {
        pPlayer = (player *)pOwner;
    }
    else
    {
        // not a player, let 'em fire
        return TRUE;
    }

    // Be sure the player can trigger the contagion when it's out there in the world, regardless of mutagen
    mutant_contagion_projectile* pContagion = (mutant_contagion_projectile*) g_ObjMgr.GetObjectByGuid( m_ContagionGuid );
    if( pContagion )
        return TRUE;

    f32 Cost = bIsAltFire ? SecondaryFireCostTweak.GetF32() : PrimaryFireCostTweak.GetF32();
    if( (Cost > pPlayer->GetMutagen()) )
    {
        // don't have enough mutagen
        return FALSE;
    }

    // good to go
    return TRUE;
}

//==============================================================================

xbool weapon_mutation::TriggerContagion( void )
{
    if( m_ContagionGuid==0 )
        return FALSE;

    mutant_contagion_projectile* pContagion = (mutant_contagion_projectile*) g_ObjMgr.GetObjectByGuid( m_ContagionGuid );
    if( pContagion==NULL )
        return FALSE;

    pContagion->TriggerFromWeapon();

    return TRUE;
}

//==============================================================================

xbool weapon_mutation::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    CONTEXT( "mutation::FireSecondaryProtected" );
	( void )Power;
    ( void )iFirePoint;
    ( void )InitPos;    // we don't need InitPos because this weapon fires from its FIRE_POINT

    // If we have a contagion guid then try to detonate it!
    if( TriggerContagion() )
        return FALSE;

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return FALSE;

    if( !ImposeMutagenCosts(pOwnerObject, FALSE) )
    {
        return FALSE;
    }

    // create a new bullet projectile, init it's position, and send it on it's way.    
    vector3 StartPosition;
    GetAltFiringStartPosition( StartPosition );

    //matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iFirePoint ] );
    matrix4 L2W;
    if ( pOwnerObject->GetType() == TYPE_PLAYER )
    {
        // we need to override the given rotation and just use the player's view
        view View( ((player*)pOwnerObject)->GetView() );
        L2W = View.GetV2W();
    }
    else
    {
        L2W = pOwnerObject->GetL2W();
    }

    radian3 Rot = L2W.GetRotation();
    (void)Rot;

    ASSERT( m_WeaponAmmo[ AMMO_SECONDARY ].m_ProjectileTemplateID >= 0 );
    
    //pain_handle PainHandle( xfs("%s_%s_SECONDARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );
    pain_handle PainHandle( xfs("%s_%s_2",pOwnerObject->GetLogicalName(),GetLogicalName()) );
/*
    // Create the contagion Projectile
    guid ProjectileID = CREATE_NET_OBJECT( mutant_contagion_projectile::GetObjectType(), netobj::TYPE_CONTAGION );
    mutant_contagion_projectile* pProjectile = (mutant_contagion_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
    ASSERT( pProjectile );

    // Remember guid for attack 
    m_ContagionGuid = ProjectileID;

    // Compute velocity
    //tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
    //vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
    vector3 Velocity( 0.0f , 0.0f , 1200.0f*2 );
    Velocity.Rotate( InitRot );
    Velocity += BaseVelocity;

    pProjectile->Setup( Owner,
                        pOwnerObject->net_GetSlot(),
                        StartPosition,
                        radian3(0.0f,0.0f,0.0f), //InitRot
                        Velocity,
                        GetZone1(),
                        GetZone2(),
                        PainHandle);
*/

    // Create the bullet.    

    guid BulletID = CREATE_NET_OBJECT( mutant_parasite_projectile::GetObjectType(), netobj::TYPE_PARASITE );

    if (BulletID)
    {
        object_ptr<mutant_parasite_projectile>pProj( BulletID );
        ASSERT( pProj.IsValid() );

        pProj->SetZones( GetZones() );

        tweak_handle SpeedTweak( xfs("%s_PRIMARY_SPEED", GetLogicalName()) );
        f32 Speed = 1333.0f;
        if (SpeedTweak.Exists())
            Speed = SpeedTweak.GetF32();

        pProj->Initialize( StartPosition , InitRot, BaseVelocity, Speed, Owner, PainHandle, pOwnerObject->net_GetSlot(), TRUE);            

        pProj->SetTarget( m_TargetGuid );
    }


    //add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );


    s32 VoiceId = g_AudioMgr.Play( "MSN_Alt_Fire_Tap", GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceId, audio_manager::GUN_SHOT, GetPosition(), GetZone1(), GetGuid() );

	return TRUE;
}

//=========================================================================

xbool weapon_mutation::FireGhostPrimary( s32 iFirePoint, xbool , vector3& )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    //actor* pActor = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );
    //ASSERT( pActor );

    // Pass through to regular fire code for now
    vector3 InitPos;
    GetFiringStartPosition( InitPos );

    // THIS IS BAD, THANK YOU MAX EXPORTER.
//    radian3 InitRot( pActor->GetPitch(), pActor->GetYaw(), 0 );

    //FireWeapon( InitPos, vector3(0,0,0), 0, InitRot, GetParentGuid(), -1 );

    // Don't do anything, the projectile is networked and will create itself on the other machines,

    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

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

//=========================================================================

xbool weapon_mutation::FireGhostSecondary( s32 iFirePoint )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    actor* pActor = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );
    ASSERT( pActor );

    // Pass through to regular fire code for now
    vector3 InitPos;
    GetFiringStartPosition( InitPos );

    // THIS IS BAD, THANK YOU MAX EXPORTER.
    radian3 InitRot( pActor->GetPitch(), pActor->GetYaw(), 0 );

    FireSecondary( InitPos, vector3(0,0,0), 0, InitRot, GetParentGuid(), iFirePoint );

    // Don't do anything, the projectile is networked and will create itself on the other machines,

    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

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

//==============================================================================

void weapon_mutation::OnEnumProp( prop_enum& PropEnumList )
{
    new_weapon::OnEnumProp      ( PropEnumList );

    PropEnumList.PropEnumHeader  ( "Mutation Weapon", "Mutation Weapon-specific properties", PROP_TYPE_HEADER );
    PropEnumList.PropEnumInt     ( "Mutation Weapon\\Primary Parasites Per Shot", "Number of primary parasite projectiles fired with each shot.", 0 );
    PropEnumList.PropEnumInt     ( "Mutation Weapon\\Secondary Parasites Per Shot", "Number of secondary parasite projectiles fired with each shot.", 0 );	
    

    SMP_UTIL_EnumHiddenManualResource( PropEnumList, "Mutation Weapon\\ParasiteHitInert", SMP_FXO );
    SMP_UTIL_EnumHiddenManualResource( PropEnumList, "Mutation Weapon\\ParasiteHitLive", SMP_FXO );
    SMP_UTIL_EnumHiddenManualResource( PropEnumList, "Mutation Weapon\\ParasiteFly", SMP_FXO );
    SMP_UTIL_EnumHiddenManualResource( PropEnumList, "Mutation Weapon\\ContagFly", SMP_FXO );



    PropEnumList.PropEnumHeader(   "Melee", "Any mutation melee related items", PROP_TYPE_HEADER );

    // Particle systems
    PropEnumList.PropEnumExternal( "Melee\\Melee Hit Particle",
                                    "Resource\0fxo\0",
                                    "Particle Resource for this item",
                                    PROP_TYPE_MUST_ENUM );

    PropEnumList.PropEnumFileName( "Melee\\Mutagen Blueprint Path",
                                    "Area51 blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
                                    "Resource for this item",
                                    PROP_TYPE_MUST_ENUM );

}

xbool weapon_mutation::OnProperty( prop_query& rPropQuery )
{
    // Base classes
    if( new_weapon::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if (SMP_UTIL_IsHiddenManualResource( rPropQuery, "Mutation Weapon\\ParasiteHitInert", "Parasite_Impact_000.fxo" ))
        return TRUE;
    if (SMP_UTIL_IsHiddenManualResource( rPropQuery, "Mutation Weapon\\ParasiteHitLive", "MucousBurst_000.fxo" ))
        return TRUE;
    if (SMP_UTIL_IsHiddenManualResource( rPropQuery, "Mutation Weapon\\ParasiteFly", "Parasite_000.fxo" ))
        return TRUE;
    if (SMP_UTIL_IsHiddenManualResource( rPropQuery, "Mutation Weapon\\ContagFly", "Contagion_missle.fxo" ))
        return TRUE;

    if ( rPropQuery.VarInt( "Mutation Weapon\\Primary Parasites Per Shot", m_nPrimaryParasitesPerShot ) )
    { 
        return TRUE;
    }
    
    if ( rPropQuery.VarInt( "Mutation Weapon\\Secondary Parasites Per Shot", m_nSecondaryParasitesPerShot ))
    { 
        return TRUE;
    }
    
    if( rPropQuery.IsVar( "Melee\\Melee Hit Particle" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hMeleeParticle_Hit.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_hMeleeParticle_Hit.SetName( pString );

                // Load the particle effect.
                if( m_hMeleeParticle_Hit.IsLoaded() == FALSE )
                    m_hMeleeParticle_Hit.GetPointer();
            }
        }
        return( TRUE );
    }

    if( rPropQuery.IsVar( "Melee\\Mutagen Blueprint Path" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_MutagenTemplateID < 0 )
            {
                rPropQuery.SetVarFileName("",256);
            }
            else
            {
                rPropQuery.SetVarFileName( g_TemplateStringMgr.GetString( m_MutagenTemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarFileName() ) > 0 )
            {
                m_MutagenTemplateID = g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
            }

        }
        return TRUE;
    }

    return FALSE;
}

//==============================================================================

void weapon_mutation::Setup( const guid& PlayerGuid, const player::animation_state& AnimState )
{
    // initialize    
    SetMeleeComplete(FALSE);
    m_HostPlayerGuid        = PlayerGuid;    
    m_CurrentViewX          = ZOOM_FOV;
    m_MeleeState            = AnimState;
}

//==============================================================================

void weapon_mutation::OnAdvanceLogic( f32 DeltaTime )
{    
    new_weapon::OnAdvanceLogic( DeltaTime );

    object *playerObject = g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid );

    if( !playerObject )
    {
        return;
    }

    // get player pointer
    player* pPlayer = (player*)playerObject;

    // we're done, don't need to do anything
    if( m_bMeleeComplete )
    {
        return;
    }
    
    // do tendril logic here.  If player is dead, tendrils will be destroyed if they exist.
    TendrilLogic(pPlayer, DeltaTime);

    // if the player is dead or we have a NULL pointer, get out
    if( (pPlayer->IsDead()) )
    {
        return;
    }
}

//==============================================================================

void weapon_mutation::TendrilLogic( player *pPlayer, f32 DeltaTime )
{
    (void)DeltaTime;
    
    // check tendrils
    mutant_tendril_projectile *pLeftTendril  = (mutant_tendril_projectile *)g_ObjMgr.GetObjectByGuid(m_LeftTendril);
    mutant_tendril_projectile *pRightTendril = (mutant_tendril_projectile *)g_ObjMgr.GetObjectByGuid(m_RightTendril);

    xbool bPlayerDead = (pPlayer->IsDead());

    // if the player is dead or we have a NULL pointer, get out
    if( bPlayerDead )
    {
        // clean up our tendrils if we have any
        if( pLeftTendril )
        {
            pLeftTendril->OnExplode();
        }

        if( pRightTendril )
        {
            pRightTendril->OnExplode();
        }
    }

    // Removing all the constraints so the corpse falls to the ground:
    // only remove all if all tendrils are gone or the player died
    if( bPlayerDead || (!pLeftTendril && !pRightTendril) )
    {
        // delete all the corpses constraints, we are done with it
        ClearCorpseConstraints();

        return;
    }

    // update corpse pin constraint
    UpdateCorpsePin();

    if( pLeftTendril && pRightTendril )
    {
        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
        if( !pOwnerObject )
        {
            return;
        }

        if( pLeftTendril->GetBeginRetract() || pRightTendril->GetBeginRetract() )
        {
            ClearCorpseConstraints();
            return;
        }

        // make sure both tendrils have hit an actor and that we haven't already created the corpse and such
        if( pLeftTendril->DidHitBody() && pRightTendril->DidHitBody() && (pLeftTendril->GetCorpseGuid() == NULL_GUID) )
        {
            ASSERT( pLeftTendril->GetTarget() == pRightTendril->GetTarget() );

            object *pTarget = g_ObjMgr.GetObjectByGuid(m_TargetGuid);
            ASSERT( pTarget && pTarget->IsKindOf(actor::GetRTTI()) );

            actor *pActor = (actor*)pTarget;
            if( !pActor )
            {
                ASSERT(0);
                return;
            }

            // if it's a tank, we don't kill it, tendrils will do the damage
            if( pActor->IsKindOf(mutant_tank::GetRTTI()) )
            {
                return;
            }

            // create the corpse object and pin it
            CreateCorpse( pActor );

            // give player a percentage of mutagen
            GiveMutagenFromTendrils();
        }
    }
}

//==============================================================================
#define CORPSE_PARTICLE_THROW        "BloodBag_000.fxo"
xbool g_UseOwnerFacingToThrow = FALSE;
void weapon_mutation::ClearCorpseConstraints( void )
{
    if( m_CorpseGuid )
    {
        corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);

        ASSERT(pCorpse);

        if( pCorpse )
        {
            // Lookup physics instance and geometry
            physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();

            // Remove all body->world constraints
            PhysicsInst.DeleteAllBodyWorldConstraints();

            actor* pOwner = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );

            ASSERT( pOwner );

            // the vector from the owner to the corpse
            vector3 Direction = (m_ConstraintPosition[CORPSE_PIN] - pOwner->GetPosition());

            vector3 StartPos, EndPos;
            
            // get the direction the owner is facing and get a vector to the corpse to apply force
            if( g_UseOwnerFacingToThrow )
            {
                // get position out in space where you want the body to end up
                GetCorpseDirection( StartPos, EndPos );

                // now get the direction vector
                Direction = EndPos - StartPos;
            }

            // pull force out of tweak table
            f32 Force = TendrilThrowForceTweak.GetF32();

            // apply some force
            PhysicsInst.ApplyVectorForce( Direction, Force );

            rhandle<char> FxResource;
            FxResource.SetName( PRELOAD_FILE( CORPSE_PARTICLE_THROW ) );

            for( s32 i = 0; i < MAX_TENDRIL_CONSTRAINTS; i++ )
            {
                // don't spew blood from corpse pin
                if( i == CORPSE_PIN )
                    continue;

                // kick off particle
                particle_emitter::CreatePresetParticleAndOrient( FxResource.GetName(), Direction, m_ConstraintPosition[i], GetZone1() );
            }
        }

        // clear corpse guid
        m_CorpseGuid = NULL_GUID;
    }

    // clear pin constraints since tendrils are finished
    for( s32 i = 0; i < MAX_TENDRIL_CONSTRAINTS; i++ )
    {
        m_iConstraint[i] = -1;
    }
}


//==============================================================================
void weapon_mutation::GetCorpseDirection( vector3 &StartPos, vector3 &EndPos )
{
    // clear vectors just in case
    StartPos.Zero();
    EndPos.Zero();

    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );

    // get position out in space where you want the body to end up
    if( pOwner->IsKindOf( player::GetRTTI() ) )
    {
        player *pPlayer = (player*)pOwner;
        StartPos = pPlayer->GetEyesPosition();
        EndPos   = StartPos + (pPlayer->GetView().GetViewZ() * TendrilReachDistanceTweak.GetF32());
    }
    else
    {
        actor *pActor = (actor*)pOwner;

        ASSERT(pActor);

        if( !pActor )
        {
            return;
        }
        // this isn't a player, so we need to get their "aim" yaw
        StartPos = pOwner->GetPosition();
        radian LookatYaw(pActor->GetLocoPointer()->GetAimerYaw());
        vector3 RotateOffset = vector3(0.0f, 0.0f, TendrilReachDistanceTweak.GetF32());
        RotateOffset.RotateY( LookatYaw );
        EndPos   = StartPos + RotateOffset;
    }
}

//==============================================================================
void weapon_mutation::UpdateCorpsePin( void )
{
    // update pin constraint
    if( m_iConstraint[CORPSE_PIN] != -1 )
    {
        if( m_CorpseGuid )
        {
            corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);
            if( pCorpse )
            {
                // Lookup physics instance and geometry
                physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();

                // Updating the constraint position - the corpse rigid bodies will follow:

                // Keep all the corpse rigid bodies awake
                PhysicsInst.Activate();

                actor* pOwner = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );
                
                ASSERT( pOwner );

                if( !pOwner )
                {
                    return;
                }

                vector3 StartPos;
                vector3 EndPos;

                // get start and end positions for direction
                GetCorpseDirection(StartPos, EndPos);

                // add in lift that we saved off
                EndPos += vector3(0.0f, m_LiftHeight, 0.0f);

                // try to keep up with owner's "offset"
                vector3 A,B,Delta;
                f32 Dist = 0.0f;
                f32 d = g_TendrilKeepUpDistance;

                for( s32 i = 0; i < MAX_TENDRIL_CONSTRAINTS; i++ )
                {
                    // move all constraints according to position of corpse pin
                    if( i == CORPSE_PIN )
                    {
                        A = EndPos;                     // leading
                        B = m_ConstraintPosition[i];    // following

                        // update segment positions
                        Delta = A - B;
                        Dist = Delta.Length();

                        // need to update constraint position?
                        if( Dist > d )
                        {
                            Delta.Normalize();

                            // add in elasticity factor i.e. 0.75f
                            Delta *= ( (Dist - d) * g_TendrilKeepUpElasticity );
                            B += Delta;

                            // update constraint position
                            m_ConstraintPosition[i] = B;
                        }
                    }
                    else
                    {
                        // put in offset after moving corpse pin
                        m_ConstraintPosition[i] = m_ConstraintPosition[CORPSE_PIN] + m_ConstraintOffset[i];
                    }
                   
                    // "shock" effect direction (pulling/pushing to and from owner)
                    vector3 Direction = (m_ConstraintPosition[i] - pOwner->GetPosition());
                    Direction.Normalize();
                    
                    // add in shock factor to constraint position
                    m_ConstraintPosition[i] += ( Direction * x_frand(-g_ShockFactor, g_ShockFactor) );

                    // Update constraint world position
                    PhysicsInst.SetBodyWorldConstraintWorldPos( m_iConstraint[i], m_ConstraintPosition[i] );
                }
            }
        }
    }
}

//==============================================================================
void weapon_mutation::GiveMutagenFromTendrils( void )
{
    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
    if( pOwner->IsKindOf( player::GetRTTI() ) )
    {
        player *pPlayer = ((player*)pOwner);
        f32 maxMutagen  = pPlayer->GetMaxMutagen();

        // this is disabled for now anyway... no tendrils
        f32 mutagenPct  = 5.0f;//MeleeMutagenReturnPctTweak.GetF32();
        mutagenPct      = (mutagenPct/100.0f); // make this a percentage
        
        // round it up
        mutagenPct = x_round((maxMutagen*mutagenPct), 0.1f);

        // give the player mutagen back
        pPlayer->AddMutagen( mutagenPct );
    }
}

//==============================================================================
void weapon_mutation::CreateCorpse( actor *pActor )
{
    // if this is a dust mite or something, just kill it, don't create a corpse
    if( !pActor->IsKindOf(genericNPC::GetRTTI()) )
    {
        // get the bone in the center of the chest
        m_iBone[CORPSE_PIN]   = pActor->GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Spine02" );

        // left shoulder
        m_iBone[1]            = pActor->GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Arm_L_UpperArm" );

        // right shoulder
        m_iBone[2]            = pActor->GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Arm_R_UpperArm" );

        actor* pOwner = (actor*)g_ObjMgr.GetObjectByGuid( GetParentGuid() );

        if( pActor->IsKindOf(character::GetRTTI()) )
        {
            character *pCharacter = (character*)pActor;
            // Play death audio        
            g_AudioMgr.Play( xfs( "%s_VOX_DEATH01", pCharacter->GetDialogPrefix() ), 
                pCharacter->GetPosition(),
                pCharacter->GetZone1(),
                TRUE, 
                TRUE );
        }

        for( s32 i = 0; i < MAX_TENDRIL_CONSTRAINTS; i++ )
        {
            ASSERT( m_iBone[i] >= 0 );

            // set constraint position
            m_ConstraintPosition[i] = pActor->GetBonePos(m_iBone[i]);

            vector3 PosDiff = pActor->GetPosition() - pOwner->GetPosition(); 
            f32 CorpseLiftHeight = g_CorpseLiftHeight; //TendrilLiftHeightTweak.GetF32();
            f32 YDiff            = PosDiff.GetY();       
            f32 LiftHeight       = CorpseLiftHeight - YDiff;;
                    
            // make sure the difference in height is within boundaries.
            if( LiftHeight < 0.0f )
            {
                // victim was WAY below us, bring it up to us
                LiftHeight = -(YDiff);
            }

            // keep it within tolerences
            m_LiftHeight = x_clamp(LiftHeight, 0.0f, CorpseLiftHeight);

            corpse *pCorpse = NULL;
            if( i == CORPSE_PIN )
            {
                pCorpse = pActor->CreateCorpseObject(TRUE);
                m_CorpseGuid = pCorpse->GetGuid();
            }
            else
            {
                // have already created corpse, just get pointer
                pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);                
            }

            ASSERT(pCorpse);

            if( !pCorpse )
            {
                return;
            }

            CreateCorpsePin( pCorpse, i );

            // this has to be here after the constraint is created!
            m_ConstraintPosition[i] += vector3(0.0f, LiftHeight, 0.0f);

            if( i == CORPSE_PIN )
            {
                m_ConstraintOffset[i].Zero();
            }
            else
            {
                m_ConstraintOffset[i] = m_ConstraintPosition[i] - m_ConstraintPosition[CORPSE_PIN];
            }
           
        }

        // make some alias from the tendril guids
        mutant_tendril_projectile *pLeftTendril  = (mutant_tendril_projectile *)g_ObjMgr.GetObjectByGuid(m_LeftTendril);
        mutant_tendril_projectile *pRightTendril = (mutant_tendril_projectile *)g_ObjMgr.GetObjectByGuid(m_RightTendril);

        // set corpse guid
        pLeftTendril->SetCorpseGuid(m_CorpseGuid);
        pRightTendril->SetCorpseGuid(m_CorpseGuid);
    }

    pActor->OnKill();
}

//==============================================================================
void weapon_mutation::CreateCorpsePin( corpse *pCorpse, s32 Index )
{
    // create pin constraint
    {
        // Lookup physics instance and geometry
        physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
        geom* pGeom = PhysicsInst.GetSkinInst().GetGeom();
        ASSERT( pGeom );

        // Get rigid body that bone is attached to        
        ASSERT( m_iBone[Index] < pGeom->m_nBones );
        s32 iRigidBody = pGeom->m_pBone[ m_iBone[Index] ].iRigidBody;
        ASSERT( iRigidBody != -1 );

        // Add constraint and store the index for later
        m_iConstraint[Index] = PhysicsInst.AddBodyWorldConstraint( iRigidBody, // Your rigid body
                                                m_ConstraintPosition[Index],   // Where in the world you want the constraint
                                                0.0f );                  // Set max dist to zero to keep it pinned exactly
        ASSERT( m_iConstraint[Index] != -1 );
    }
}


//==============================================================================

void weapon_mutation::ChangeFOV( f32 DeltaTime )
{
    if( !USE_FOV )
        return;

    // Get the player.
    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid ) );
    const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();

    if( m_bMeleeComplete )
    {
        // Zooming Out.
        m_CurrentViewX += m_ViewChangeRate * DeltaTime;
        m_CurrentViewX = fMin( m_CurrentViewX , OriginalViewInfo.XFOV );     
        if( m_CurrentViewX >= OriginalViewInfo.XFOV )
        {
            m_CurrentViewX = OriginalViewInfo.XFOV;    
            Player.ResetView();
        }
    }
    else
    {
        // Zooming in.
        m_CurrentViewX -= m_ViewChangeRate * DeltaTime;
        m_CurrentViewX = fMax( m_CurrentViewX , ZOOM_FOV );
    }
}

//==============================================================================
radian weapon_mutation::GetXFOV( void )
{
    /*
    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid ) );
    const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();

    if( m_bMeleeComplete )
    {
        // Zooming Out.        
        m_CurrentViewX = OriginalViewInfo.XFOV;    
        Player.ResetView();        
    }
    else
    {
        // Zooming in.        
        m_CurrentViewX = ZOOM_FOV;
    }
    */

    if( !USE_FOV )
    {
        player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid ) );
        const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();

        m_CurrentViewX = OriginalViewInfo.XFOV;
    }

    return m_CurrentViewX;
}

//==============================================================================
void weapon_mutation::CausePain( player *pPlayer, object *pObject ) const
{    
    if( MELEE_SHAKE_CAM )
    {
        pPlayer->ShakeView( s_HitShakeTime, s_HitShakeAmount, s_HitShakeSpeed );
    }

    if( DO_FEEDBACK )
    {
        pPlayer->DoFeedback((s_HitShakeTime/2.0f), s_HitShakeAmount);
    }

    // here's what we hit    
    if( (pObject->GetAttrBits() & object::ATTR_LIVING) )
    {
        // hit a living something
        g_AudioMgr.Play( "Mut_Melee_Flesh" );
    }
    else
    {
        // hit a wall or something
        g_AudioMgr.Play( "Mut_Melee_Surface" );
    }

    // where is the pain coming from?
    vector3 PainDirection;
    vector3 StartPosition( pPlayer->GetPosition() );

    if( USE_PLAYER_POSITION )
    {
        StartPosition = pPlayer->GetPosition();
    }
    else
    // if we don't want to use an upward force on right/left attacks, make the position start from the player's BBox center
    if( !USE_UPWARD_FORCE )
    {
        StartPosition.GetY() = pPlayer->GetPosition().GetY();
    }

    if( USE_PLAYER_POSITION )
    {
        PainDirection = (pObject->GetBBox().GetCenter() - StartPosition);
    }
    else
    {        
        switch( m_MeleeState )
        {
        case player::ANIM_STATE_MELEE_FROM_RIGHT:
            {
                ///////////
                // make this come from the right
                PainDirection = (pObject->GetBBox().GetCenter() - StartPosition);
                PainDirection.RotateY(HIT_ANGLE);
            }
            break;
        case player::ANIM_STATE_MELEE_FROM_LEFT:
            {
                ///////////
                // make this come from the left
                PainDirection = (pObject->GetBBox().GetCenter() - StartPosition);
                PainDirection.RotateY(-HIT_ANGLE);
            }
            break;        
        case player::ANIM_STATE_MELEE_FROM_UP:
        case player::ANIM_STATE_MELEE_FROM_CENTER:
            {
                ///////////
                // make this come from the front (no upward force)
                PainDirection = (pObject->GetBBox().GetCenter() - pPlayer->GetBBox().GetCenter());                
            }
            break;
        
        case player::ANIM_STATE_MELEE_FROM_DOWN:
        default:
            {
                ///////////
                // make this come from the front/low (knock them in the air)
                PainDirection = (pObject->GetBBox().GetCenter() - pPlayer->GetPosition());
            }
            break;
        }
    }

    vector3 PainCenter( (pObject->GetBBox().GetCenter() - PainDirection) );
    
    vector3 ClosestPoint;
                
    // Get the closest point projection to the target.
    g_EventMgr.ClosestPointToAABox( PainCenter, pObject->GetBBox(), ClosestPoint );

    object* pHostPlayer = g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid );
    if( !pHostPlayer )
        return;

    pain_handle PainHandle(xfs("%s_ExtremeMelee",pHostPlayer->GetLogicalName()));
    pain Pain;
    Pain.Setup( PainHandle, m_HostPlayerGuid, PainCenter );
    Pain.SetDirection( PainDirection );
    Pain.SetDirectHitGuid( pObject->GetGuid() );
    Pain.SetCollisionInfo( g_CollisionMgr.m_Collisions[0] );
    Pain.ApplyToObject(pObject);
/*
    // set up a pain event
    pain PainEvent;
    PainEvent.Type      = pain::TYPE_MUTATION;
    PainEvent.Center    = PainCenter;
    PainEvent.Origin    = m_HostPlayerGuid ;
    PainEvent.PtOfImpact= ClosestPoint;//pObject->GetBBox().GetCenter();
    PainEvent.Direction = PainDirection;
    PainEvent.ForceR0   = MUTATION_MELEE_FORCE;
    PainEvent.ForceR1   = 0;

    // scale pain over PAIN_RADIUS
    if( SCALE_PAIN )
    {
        PainEvent.DamageR0  = 0;
        PainEvent.RadiusR0  = 0;
    }
    else
    {
        // pain is evenly distributed across PAIN_RADIUS
        PainEvent.DamageR0  = MUTATION_MELEE_DAMAGE;
        PainEvent.RadiusR0  = PAIN_RADIUS;
    }

    PainEvent.DamageR1  = MUTATION_MELEE_DAMAGE;    // R0 and R1 the same = direct damage, not scaled
    PainEvent.RadiusR1  = PAIN_RADIUS;

    PainEvent.Direction.Normalize();

    pObject->OnPain( PainEvent );
  
    if( (pObject->GetAttrBits() & object::ATTR_LIVING) )
    {
        SpawnMutagen(pObject);
    }
    */
}

//==============================================================================
#define nSPAWNING_MUTAGEN
void weapon_mutation::SpawnMutagen( object *pObject ) const
{
    if( ! pObject->IsKindOf( character::GetRTTI() ) )
    {
        return;
    }

    character *pCharacter = (character*)pObject;

    // didn't kill target
    if( (pCharacter->GetHealth() > F32_MIN) )
    {
        return;
    }

#if defined( SPAWNING_MUTAGEN )
    if( m_MutagenTemplateID == -1 )
        return;
#endif

    object* pHostPlayer = g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid );
    if( !pHostPlayer )
        return;

    // get player pointer
    player* pPlayer = (player*)pHostPlayer;

    // mreed: don't spawn a pickup, just give it to the player

#ifndef SPAWNING_MUTAGEN
    pPlayer->AddMutagen( pPlayer->GetInventory2().GetMaxAmount( INVEN_MUTAGEN ) );
#else
    random r( x_rand() );

    s32 iBone = pCharacter->GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Spine02" );
    vector3 BonePos = pCharacter->GetLocoPointer()->m_Player.GetBonePosition( iBone );

    pickup::CreatePickup( pPlayer->GetGuid(), pPlayer->net_GetSlot(), INVEN_MUTAGEN, 
                          pPlayer->GetInventory2().GetMaxAmount( INVEN_MUTAGEN ),  5.0f, 
                          BonePos + vector3(0.0f, 100.0f, 0.0f), 
                          radian3(0.0f,r.frand(R_0,R_360),0.0f), vector3(0.0f, 0.0f, 0.0f), 
                          GetZone1(), GetZone2() );
#endif // SPAWNING_MUTAGEN
}



//=========================================================================
// KSS -- This is the new melee where you only hit one object at a time.
void weapon_mutation::DoExtremeMelee( void )
{
    object *playerObject = g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid );

    // if the player is dead or we have a NULL pointer, get out
    if( !playerObject )
    {
        return;
    }

    // get player pointer
    player* pPlayer = (player*)playerObject;

    if( pPlayer->IsDead() )
    {
        return;
    }

    //vector3 MyPos ( pPlayer->GetPositionWithOffset( actor::OFFSET_AIM_AT ) );
    vector3 MyPos ( pPlayer->GetEyesPosition() );
    radian3 RadRot;
    pPlayer->GetEyesPitchYaw( RadRot.Pitch, RadRot.Yaw );
    vector3 theRotation( RadRot );
    theRotation.NormalizeAndScale( PAIN_RADIUS );
    vector3 TargetPos = MyPos + theRotation;

    // set up collision tests
    //g_CollisionMgr.RaySetup( pPlayer->GetGuid(), MyPos, TargetPos );
    g_CollisionMgr.SphereSetup( pPlayer->GetGuid(), MyPos, TargetPos, PAIN_RADIUS );
    g_CollisionMgr.AddToIgnoreList( m_HostPlayerGuid );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_LIVING, object::ATTR_COLLISION_PERMEABLE );

    xbool bFound = FALSE;

    s32 NumCollisions = g_CollisionMgr.m_nCollisions;

    // look for living things first!
    if( (NumCollisions > 0) )
    {
        s32 count = 0;

        s32 totalSize = NumCollisions * sizeof(collision_mgr::collision);

        // cache collisions out so we can do other collision checks on target object
        collision_mgr::collision *CachedCollision = (collision_mgr::collision*)x_malloc( totalSize );

        // clear memory
        x_memset( CachedCollision, 0, totalSize );

        // copy over data to cache
        x_memcpy( CachedCollision, g_CollisionMgr.m_Collisions, totalSize );

        // KSS -- Make sure this is a living thing... some weirdness with collision manager
        while( count < NumCollisions )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( CachedCollision[count].ObjectHitGuid );

            // See if you can see what you're swinging at
            g_CollisionMgr.RaySetup( pPlayer->GetGuid(), MyPos, pObject->GetBBox().GetCenter() );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_ALL_PROJECTILES, 
                                            object::ATTR_LIVING | object::ATTR_COLLISION_PERMEABLE );

            // make sure object is valid and that there is nothing in the way
            if( pObject && g_CollisionMgr.m_nCollisions == 0 )
            {
                // KSS -- Make sure this is a living thing... some weirdness with collision manager
                if( (pObject->GetGuid() != m_HostPlayerGuid) && (pObject->GetAttrBits() & object::ATTR_LIVING) )
                {
                    // found one
                    bFound = TRUE;
                    CausePain( pPlayer, pObject );            
                    
                    s32 Difficulty = 0; // Easy

                    // xstring to build string for mutagen return difficulty tweak (if available)
                    xstring StringPrefix = (const char*)"MUTATION_MeleeMutagenPct";
#ifndef X_EDITOR
                    // get real difficulty
                    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
#endif
                    {
                        // difficulty only applies in Campaign.
                        Difficulty = g_Difficulty;

                        // append difficulty.  Online has its own tweaks and Editor should just be Easy.
                        StringPrefix += (const char*)xfs( "_%s", DifficultyText[Difficulty] );
                    }

                    // get a const pointer as an alias so we can pass it in to tweak_handle constructor.
                    const char* pString = (const char*)StringPrefix;
                    
                    // Add mutagen back to player                    
                    tweak_handle MeleeMutagenReturnPctTweak( pString );    // What percent of mutagen is returned on a melee attack?
                    f32 mutagenPct        = MeleeMutagenReturnPctTweak.GetF32();

                    mutagenPct            = (mutagenPct / 100.0f); // make this a percentage
                    const f32 maxMutagen  = pPlayer->GetMaxMutagen();

                    // round it up
                    mutagenPct = x_round( (maxMutagen*mutagenPct), 0.1f );

                    // give the player mutagen back
                    pPlayer->AddMutagen( mutagenPct );

                    break;
                } 
            }
            count++;
        }

        // free cached collision info
        x_free( CachedCollision );
        CachedCollision = NULL;
    }

    // now look for anything damageable because we found nothing living
    g_CollisionMgr.SphereSetup( pPlayer->GetGuid(), MyPos, TargetPos, SECONDARY_PAIN_RADIUS );
    g_CollisionMgr.AddToIgnoreList( m_HostPlayerGuid );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

    s32 i = 0;
    
    for( i = 0; i < g_CollisionMgr.m_nCollisions; i++ )
    {
        if( g_CollisionMgr.m_Collisions[i].ObjectHitGuid != NULL_GUID )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( g_CollisionMgr.m_Collisions[i].ObjectHitGuid );
            if( (pObject) && (pObject->GetGuid() != m_HostPlayerGuid) )
            {
                CausePain( pPlayer, pObject );            
            } 
        }
    }
}
/*
//==============================================================================
void weapon_mutation::DoExtremeMelee( void )
{
    // Collect all damagable objects in range (MAX 32)
    const s32 kMAX_CONTACTS = 128;
    slot_id idList[kMAX_CONTACTS];
    s32 contactCount = 1;

    object *playerObject = g_ObjMgr.GetObjectByGuid( m_HostPlayerGuid );

    // if the player is dead or we have a NULL pointer, get out
    if( !playerObject )
    {
        return;
    }

    // get player pointer
    player* pPlayer = (player*)playerObject;

    if( pPlayer->IsDead() )
    {
        return;
    }

    //bbox Pain( pPlayer->GetPosition(), PAIN_RADIUS );
    bbox Pain( pPlayer->GetPosition(), PAIN_RADIUS );
    g_ObjMgr.SelectBBox( object::ATTR_DAMAGEABLE, Pain, object::TYPE_ALL_TYPES );    
    slot_id aID = g_ObjMgr.StartLoop();
    while( aID != SLOT_NULL )
    {
        if ( contactCount >= kMAX_CONTACTS )
            break ;

        idList[contactCount]= aID;
        ++contactCount;

        aID = g_ObjMgr.GetNextResult( aID );
    }
    g_ObjMgr.EndLoop();

    // Apply damage to all the collected objects
    while( --contactCount)
    {
        object* pObject = g_ObjMgr.GetObjectBySlot(idList[contactCount] );
        if( (pObject) && (pObject->GetGuid() != m_HostPlayerGuid) )
        {
             CausePain( pPlayer, pObject );            
        }
    }
}
*/

//=============================================================================
void weapon_mutation::SetMeleeComplete( xbool bComplete )
{ 
    m_bMeleeComplete = bComplete; 
}
//=============================================================================

guid weapon_mutation::FindDumbTarget( const vector3& InitPos, const radian3& InitRot, guid Owner )
{
    //
    // Loop through active actors, discard those outside a minimum angle,
    // and keep the one with the smallest angle. If one is found within a minimum distance,
    // bail out and return NULL_GUID
    //
    guid TargetGuid = NULL_GUID;

    // all bets are off if we aren't owned by an actor
    object* pObject = g_ObjMgr.GetObjectByGuid( Owner );
    actor* pOwner = NULL;
    if ( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
    {
        pOwner = (actor*)pObject;
    }


    static f32 MIN_AUTO_LOCK_DISTANCE = 300.0f;
    static radian MIN_AUTO_LOCK_ANGLE_DIFF = R_8;
    radian MinAngleDiff = MIN_AUTO_LOCK_ANGLE_DIFF;

    actor* pActor = actor::m_pFirstActive;
    while( pActor && pOwner )
    {
        if( pActor->GetGuid() != Owner )
        {
            if( !pOwner->IsFriendlyFaction( SMP_UTIL_GetFactionForGuid( pActor->GetGuid() ) ) )
            {
                const vector3 ToTarget( pActor->GetPosition() - InitPos );
                const f32 DistanceSq = ToTarget.LengthSquared();
                if ( DistanceSq > x_sqr( MIN_AUTO_LOCK_DISTANCE ) )
                {
                    // We have an actor that's too close to us, bail out
                    break;
                }

                vector3 TargetDir( ToTarget );
                TargetDir.Normalize();
                const vector3 AimDir( InitRot );

                radian AngleDiff = x_abs( x_acos( AimDir.Dot( TargetDir ) ) ); 

                if ( AngleDiff < MinAngleDiff )
                {
                    // ok this could be a valid target, find out if we can see him
                    g_CollisionMgr.RaySetup( GetGuid(), InitPos, pActor->GetPosition() );   
                    g_CollisionMgr.AddToIgnoreList( pActor->GetGuid() );
                    g_CollisionMgr.AddToIgnoreList( Owner );
                    g_CollisionMgr.AddToIgnoreList( GetGuid() );
                    g_CollisionMgr.StopOnFirstCollisionFound();
                    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_PROJECTILES, object::ATTR_COLLISION_PERMEABLE);

                    if ( (g_CollisionMgr.m_nCollisions == 0)
                        || g_CollisionMgr.m_Collisions[0].ObjectHitGuid == pActor->GetGuid() )
                    {
                        MinAngleDiff = AngleDiff;
                        TargetGuid = pActor->GetGuid();
                    }
                }
            }
        }
        pActor = pActor->m_pNextActive;
    }

    return TargetGuid;
}

//=============================================================================

void weapon_mutation::BeginSwitchFrom( void )
{
    TriggerContagion();
    TriggerContagion();
}

//=============================================================================

