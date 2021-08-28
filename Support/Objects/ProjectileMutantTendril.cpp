//=============================================================================================
// MUTANT TENDRIL PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileMutantTendril.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "Objects\actor\actor.hpp"
#include "Decals\DecalMgr.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "player.hpp"
#include "Characters\character.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Objects\DeadBody.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "Characters\GenericNPC\GenericNPC.hpp"
#include "Objects\Corpse.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "Objects\WeaponMutation.hpp"

//=============================================================================================
// STATICS & CONSTANTS
//=============================================================================================

#define TENDRIL_AUDIO_LAUNCH            "Contagion_Launch"
#define TENDRIL_AUDIO_FLY_LOOP          "Contagion_Fly_Loop"

#define TENDRIL_AUDIO_IMPACT            "BulletImpactFlesh"

#define TENDRIL_PARTICLE_HIT            "BloodBag_000.fxo"
#define TENDRIL_PARTICLE_LAUNCH         "MucousBurst_000.fxo"
#define TENDRIL_PARTICLE_FLYING         "Parasite_000.fxo"
#define TENDRIL_MAIN_PARTICLE_FLYING    "Parasite_000.fxo"

f32 Tendril_FactorRun             = 1.0f;
f32 Tendril_FactorRise            = -1.0f;
f32 g_TendrilMaxLength = 200.0f;

struct tendril_tweaks
{
    tendril_tweaks( void );
    
    xbool m_bDrawLineToTarget;

    f32   m_BoneDist;                    // what is the distance between segments (This is for rendering)
    f32   m_SegmentDistance;             // This is the max distance between segments (Used in the elasticity code).
    
    f32   m_SegmentElasticity;           // elasticity between segments - NORMAL
    f32   m_EnterBodySegmentElasticity;  // elasticity between segments - When entering body
    f32   m_PullbackElasticity;          // elasticity when we're reeling in to take up slack
    f32   m_SegmentSize;                 // how big is each segment
    xbool m_RenderSegments;              // do we render the segments? (debug)
    xbool m_bOnlyRenderHead;             // do we render the segments? (debug)
    f32   m_SegmentCollapseTime;         // how long does it take to collapse?
    f32   m_TendrilSpeed;                // how fast?
    xbool m_bDoCameraShake;              // do we do feedback on hit?
    f32   m_TendrilForce;                // how hard do we hit?
    f32   m_TendrilForceRadius;          // how big is the force's radius    
    xbool m_bDoHitEffects;               // do we do pain effects?
    xbool m_bDrawBBox;                   // for showing what the BBox looks like (debug)
    f32   m_TendrilOffsetY;              // put the tendrils down into the body a bit since the bones are right on the shoulders.

    f32 m_HitShakeTime;                  // feedback shake time
    f32 m_HitShakeAmount;                // feedback shake amount
    f32 m_HitShakeSpeed;                 // feedback shake speed
};

tendril_tweaks::tendril_tweaks( void )
{
    m_bDrawLineToTarget             = FALSE;
    m_BoneDist                      = 2.5f;
    m_SegmentDistance               = 31.25f;
    m_SegmentElasticity             = 0.2f;
    m_EnterBodySegmentElasticity    = 0.5f;
    m_PullbackElasticity            = 0.75f;
    m_SegmentSize                   = 10.0f;
    m_RenderSegments                = FALSE;
    m_bOnlyRenderHead               = FALSE;
    m_SegmentCollapseTime           = 0.175f;
    m_TendrilSpeed                  = 3500.0f;
    m_bDoCameraShake                = TRUE;
    m_TendrilForce                  = 1000.0f;
    m_TendrilForceRadius            = 100.0f;    
    m_bDoHitEffects                 = TRUE;
    m_bDrawBBox                     = FALSE;
    m_TendrilOffsetY                = 10.0f;

    m_HitShakeTime                  = 0.8f;
    m_HitShakeAmount                = 0.6f;
    m_HitShakeSpeed                 = 1.5f;
}

tendril_tweaks g_TendrilTweaks;

xbool g_RenderTendrilGeom        = TRUE;
f32 g_HackTendrilTime            = 3.0f;
xbool g_bHackTendrilRetract      = FALSE;

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct mutant_tendril_projectile_desc : public object_desc
{
    mutant_tendril_projectile_desc( void ) : object_desc( 
        object::TYPE_MUTANT_TENDRIL_PROJECTILE, 
        "Mutant Tendril Projectile", 
        "WEAPON",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_NO_RUNTIME_SAVE    |
        object::ATTR_RENDERABLE,
        FLAGS_IS_DYNAMIC /*| FLAGS_GENERIC_EDITOR_CREATE*/ )
    {}

    virtual object* Create          ( void )
    {
        return new mutant_tendril_projectile;
    }

} s_Mutant_Tendril_Projectile_Desc;

//=============================================================================================

const object_desc&  mutant_tendril_projectile::GetTypeDesc     ( void ) const
{
    return s_Mutant_Tendril_Projectile_Desc;
}


//=============================================================================================
const object_desc&  mutant_tendril_projectile::GetObjectType   ( void )
{
    return s_Mutant_Tendril_Projectile_Desc;
}

//=============================================================================================

mutant_tendril_projectile::mutant_tendril_projectile() :    
    m_FlyFXGuid                 ( NULL_GUID     ),
    m_LaunchFXGuid              ( NULL_GUID     ),
    m_AimAtBone                 ( -1            ),
    m_FlyVoiceID                ( 0             ),
    m_CollapseTime              ( 0.0f          ),
    m_bEnterBody                ( FALSE         ),
    m_bRetractTendrils          ( FALSE         ),
    m_AttachBone                ( -1            ),
    m_iBone                     ( -1            ),
    m_Target                    ( NULL_GUID     ),
    m_CorpseGuid                ( NULL_GUID     ),    
    m_bLeft                     ( FALSE ),    
    m_BonePosition              ( 0.0f, 0.0f, 0.0f  ),
    m_bHitBody                  ( FALSE         ),
    m_HitEffect                 ( NULL_GUID     ),
    m_LastDistance              ( 0.0f ),
    m_SplineT                   ( 0.0f ),
    m_SplineRate                ( 0.0f ),
    m_SplineTime                ( 0.0f )
{   
    m_SkinInst.SetUpSkinGeom( PRELOAD_MP_FILE( "WPN_MUT_Spear.skingeom" ) );

    m_BounceFactorRise  = Tendril_FactorRise;
    m_BounceFactorRun   = Tendril_FactorRun;

    m_HackTendrilTime    = 0.0f;
    m_HackTendrilRetract = FALSE;

    // set segment positions to initialized position, currently 0,0,0
    for( u32 i = 0; i < MAX_SEGMENTS; i++ )
    {        
        m_SegmentPositions[i] = m_BonePosition;
    }

    m_SplineRate = 1.0f/x_frand(0.5f, 0.75f);

    // get a resource pointer for the hit effect    
    m_hParticleHit.SetName( PRELOAD_FILE( TENDRIL_PARTICLE_HIT ) );

    // Load the particle effect.
    if( m_hParticleHit.IsLoaded() == FALSE )
        m_hParticleHit.GetPointer();
}

//=============================================================================================

mutant_tendril_projectile::~mutant_tendril_projectile()
{
    DestroyParticles();
    g_AudioMgr.Release( m_FlyVoiceID, 0.10f );
}

//=============================================================================
void mutant_tendril_projectile::DestroyParticles( void )
{
    // Destroy the particle effects.
    if( m_HitEffect )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_HitEffect );
        if( pObj )
        {
            // let it die slowly
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        
        m_HitEffect = NULL_GUID;
    }

    if ( m_LaunchFXGuid )
    {
        g_ObjMgr.DestroyObject( m_LaunchFXGuid );
    }

    if ( m_FlyFXGuid )
    {
        g_ObjMgr.DestroyObject( m_FlyFXGuid );
    }
}

//=============================================================================
void mutant_tendril_projectile::Setup( guid               OriginGuid,
                                        s32               OriginNetSlot,
                                        const vector3&    Position,
                                        const radian3&    Orientation,
                                        const vector3&    Velocity,
                                        s32               Zone1,
                                        s32               Zone2,
                                        pain_handle       PainHandle,
                                        xbool             bLeft)
{
    (void)Position;
    (void)Velocity;
    (void)Orientation;

    SetOrigin    ( OriginGuid, OriginNetSlot );    
    SetPainHandle( PainHandle );

    m_CollapseTime  = g_TendrilTweaks.m_SegmentCollapseTime;
    m_bLeft         = bLeft;

    // Start up audio
    g_AudioMgr.Play( TENDRIL_AUDIO_LAUNCH, Position, GetZone1(), TRUE );    
    
    m_FlyVoiceID = g_AudioMgr.Play( TENDRIL_AUDIO_FLY_LOOP, Position, GetZone1(), TRUE );

    object *pFromObject = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
    actor *pFromActor = (actor*)pFromObject;

    if( !pFromObject || !pFromActor || !pFromObject->IsKindOf( actor::GetRTTI() ) )
    {
        ASSERT(0);
        return;
    }
    
    // get start position
    {
        matrix4 L2W;
        vector3 BonePos;            

        // get spawn location which should be a bone position
        GetTendrilAttachLocation( pFromActor, L2W, BonePos, m_AttachBone, bLeft);
        
        // set segment positions to initialized position
        for( u32 i = 0; i < MAX_SEGMENTS; i++ )
        {        
            m_SegmentPositions[i] = BonePos;
        }

        radian Pitch, Yaw;

        // get to target vector
        vector3 ToTarget( GetAimAtPosition() - BonePos );
        ToTarget.NormalizeAndScale(g_TendrilTweaks.m_TendrilSpeed);
        ToTarget.GetPitchYaw(Pitch,Yaw);
        
        // rotate our projectile in the proper orientation
        radian3 rot(Pitch,Yaw,R_0);

        SetStart( BonePos, rot, ToTarget, Zone1, Zone2, 0.0f );
    }

    //m_LaunchFXGuid = particle_emitter::CreatePresetParticleAndOrient( TENDRIL_PARTICLE_LAUNCH, m_Velocity, m_BonePosition, GetZone1() );
}

//==============================================================================

void mutant_tendril_projectile::SetStart( const vector3& Position, 
                                           const radian3& Orientation,
                                           const vector3& Velocity,
                                           s32            Zone1, 
                                           s32            Zone2,
                                           f32            Gravity )
{
    net_proj::SetStart( Position, Orientation, Velocity, Zone1, Zone2, Gravity );
    m_FXHandle.SetTranslation( Position );
}

//=============================================================================
void mutant_tendril_projectile::RetractTendril( const guid& Owner, xbool bLeft )
{
    (void)Owner;
    (void)bLeft;

    ASSERT(m_bLeft == bLeft);

    if( g_bHackTendrilRetract )
    {
        m_HackTendrilTime    = g_HackTendrilTime;
        m_HackTendrilRetract = TRUE;
    }
    else
    {   
        // going back into hands now
        m_bRetractTendrils = TRUE;
    }
}

#ifndef X_EDITOR
//=============================================================================
void mutant_tendril_projectile::net_Deactivate( void )
{
    if( (GameMgr.GameInProgress()) && 
        (m_OwningClient != g_NetworkMgr.GetClientIndex()) )
    {
        OnExplode();
    }
}

#endif

//=============================================================================
void mutant_tendril_projectile::OnRender( void )
{
#ifndef X_RETAIL
    if( g_TendrilTweaks.m_bDrawLineToTarget )
    {
        draw_Line( GetPosition(), GetAimAtPosition(), XCOLOR_RED );
    }

    if( g_TendrilTweaks.m_bDrawBBox )
    {
        draw_BBox(GetBBox());
    }

    xcolor tColor[MAX_SEGMENTS] = { XCOLOR_GREEN,   XCOLOR_RED,     XCOLOR_BLUE, 
                                    XCOLOR_YELLOW,  XCOLOR_PURPLE,  XCOLOR_AQUA, 
                                    XCOLOR_RED,     XCOLOR_PURPLE,  XCOLOR_YELLOW};

    // draw temp head
    if( g_TendrilTweaks.m_RenderSegments )
    {        
        for( u32 i = 0; i < MAX_SEGMENTS; i++ )
        {
            draw_Sphere( m_SegmentPositions[i], g_TendrilTweaks.m_SegmentSize, tColor[i] );

            // draw connectors
            if( i < (MAX_SEGMENTS-1) )
            {
                if( m_bLeft )
                {
                    draw_Line( m_SegmentPositions[i], m_SegmentPositions[i+1], XCOLOR_RED );
                }
                else
                {
                    draw_Line( m_SegmentPositions[i], m_SegmentPositions[i+1], XCOLOR_YELLOW );
                }
            }
        }
    }
    else
    if( g_TendrilTweaks.m_bOnlyRenderHead )
    {
        draw_Sphere( m_SegmentPositions[0], g_TendrilTweaks.m_SegmentSize, tColor[0] );
    }

#endif

    // render skin geometry
    if( g_RenderTendrilGeom )// don't render until we are moving out of the body
    {
        // Geometry present?
        skin_geom* pSkinGeom = m_SkinInst.GetSkinGeom();

        if (!pSkinGeom)
            return ;

        // get matrix
        matrix4* mat = m_SMEM_Mat_Cache.GetMatrices(pSkinGeom->m_nBones);
        vector3 LookAt;
        f32 Pitch, Yaw;

        ASSERT( pSkinGeom->m_nBones == MAX_SEGMENTS );

        // set up head segment
        {
            s32 Head = 0;
            LookAt = m_Velocity;

            LookAt.GetPitchYaw(Pitch, Yaw);

            // setup matrix
            mat[Head].Setup(vector3(1.0f, 1.0f, 1.0f), radian3(Pitch, Yaw, R_0), m_SegmentPositions[Head]);

            // pre-translate info
            mat[Head].PreTranslate( vector3(0.0f, 0.0f, -(g_TendrilTweaks.m_BoneDist*Head)) );
        }

        // set up trailing segments
        for( s32 i = 1; i < MAX_SEGMENTS; i++ )
        {
            // vector to leading segment
            LookAt = m_SegmentPositions[i-1] - m_SegmentPositions[i];

            LookAt.GetPitchYaw(Pitch, Yaw);

            // setup matrix
            mat[i].Setup(vector3(1.0f, 1.0f, 1.0f), radian3(Pitch, Yaw, R_0), m_SegmentPositions[i]);

            // pre-translate info
            mat[i].PreTranslate( vector3(0.0f, 0.0f, -(g_TendrilTweaks.m_BoneDist*i)) );
        }

        // Compute LOD mask
        u64 LODMask = m_SkinInst.GetLODMask(GetL2W());

        if (LODMask == 0)
            return;

        // Setup render flags
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0 ;

        // Render that puppy!
        m_SkinInst.Render( &GetL2W(), 
            mat,
            pSkinGeom->m_nBones, 
            Flags | GetRenderMode(),
            LODMask, 
            xcolor(128, 128, 128, 255) ) ;
    }
}

//==============================================================================
void mutant_tendril_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    if( m_bHitBody )
        return;

    m_ImpactNormal = Coll.Plane.Normal;

    net_proj::OnImpact( Coll, pTarget );

    g_AudioMgr.Play( TENDRIL_AUDIO_IMPACT, Coll.Point, GetZone1(), TRUE );

    // Move to new position
    OnMove( Coll.Point );

    // Shake camera and controller
    if( g_TendrilTweaks.m_bDoCameraShake )
    {
        object* pParent = (object*)g_ObjMgr.GetObjectByGuid( m_OriginGuid );
        if( pParent->IsKindOf( player::GetRTTI() ) )
        {
            player* pPlayer = (player*)pParent;            
            pPlayer->ShakeView( g_TendrilTweaks.m_HitShakeTime, g_TendrilTweaks.m_HitShakeAmount, g_TendrilTweaks.m_HitShakeSpeed );
            pPlayer->DoFeedback((g_TendrilTweaks.m_HitShakeTime/2.0f), g_TendrilTweaks.m_HitShakeAmount);
        }
    }

    // if this is an actor (should be) spray some funk.
    if( pTarget->IsKindOf( actor::GetRTTI() ) )
    {
        m_bHitBody = TRUE;
        FakePain(pTarget);
    }
}

//=============================================================================================
void mutant_tendril_projectile::OnMove( const vector3& NewPos )
{
    net_proj::OnMove(NewPos);

    UpdateParticles(m_SegmentPositions[0]);
}

//=============================================================================================
void mutant_tendril_projectile::UpdateParticles( const vector3& NewPosition )
{
    // tendril hitting body effect
    if( m_HitEffect )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_HitEffect );
        if( pObj != NULL )
        {
            matrix4 L2W;
            L2W.Identity();
            L2W.SetTranslation( NewPosition );

            // set proper trail rotation
            radian3 ParticleRotation( m_Velocity.GetPitch(), m_Velocity.GetYaw(), R_0 );

            L2W.SetRotation( ParticleRotation );

            pObj->OnTransform( L2W );
            pObj->OnMove( NewPosition );
        }
    }
}

/*
//=============================================================================
void mutant_tendril_projectile::SetupConstraints(vector3 &CorpsePinPosition, f32 LiftHeight )
{
    (void)CorpsePinPosition;

    // create a constraint on the corpse:
    // we have a corpse, set up constraint
    if( m_CorpseGuid )
    {
        corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);

        ASSERT(pCorpse);

        if( pCorpse )
        {
            // Lookup physics instance and geometry
            physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
            geom* pGeom = PhysicsInst.GetSkinInst().GetGeom();
            ASSERT( pGeom );

            PhysicsInst.ApplyBlast( m_ConstraintPosition, g_TendrilTweaks.m_TendrilForceRadius, g_TendrilTweaks.m_TendrilForce/2.0f );
            PhysicsInst.ApplyBlast( m_ConstraintPosition, vector3(0,1,0), g_TendrilTweaks.m_TendrilForceRadius, g_TendrilTweaks.m_TendrilForce ) ;

            ASSERT( m_ConstraintBone >= 0 );
            ASSERT( m_ConstraintBone < pGeom->m_nBones );
            
            // Get rigid body that bone is attached to
            s32 iRigidBody = pGeom->m_pBone[ m_ConstraintBone ].iRigidBody;
            
            ASSERT( iRigidBody != -1 );

            // Add constraint and store the index for later
            m_iConstraint = PhysicsInst.AddBodyWorldConstraint( iRigidBody, 			    // Your rigid body
                                                m_ConstraintPosition,                       // Where in the world you want the constraint
                                                0.0f, g_TendrilTweaks.m_ConstraintMax );    // Set min + max dist to zero to keep it pinned exactly

            ASSERT( m_iConstraint != -1 );

            // pick it up a bit.
            LiftBody( LiftHeight );
        }
    }
}
*/

//=============================================================================

void mutant_tendril_projectile::OnExplode( void )
{
    net_proj::OnExplode();

#ifndef X_EDITOR
    if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
    {
        DESTROY_NET_OBJECT( this );
    }
}


//=============================================================================
void mutant_tendril_projectile::FakePain( object* pObject )
{
    // Apply fake pain
    pain_handle PainHandle("MUTANT_CONT_FAKE");

    pain Pain;
    Pain.Setup( PainHandle, m_OriginGuid, m_NewPos );
    Pain.SetDirection( m_Velocity );
    Pain.SetDirectHitGuid( pObject->GetGuid() );
    Pain.ApplyToObject( pObject );
    
    // Create the hit effect.
    if( g_TendrilTweaks.m_bDoHitEffects && m_HitEffect == NULL_GUID )
    {
        m_HitEffect = particle_emitter::CreatePresetParticleAndOrient( TENDRIL_PARTICLE_HIT, m_Velocity, m_SegmentPositions[0], GetZone1() );
    }
}

//=============================================================================
xbool mutant_tendril_projectile::GetTendrilAttachLocation( actor *pFromActor, matrix4 &L2W, vector3 &BonePos, s32 &iBone, xbool bLeft )
{
    iBone = -1;

    if( pFromActor->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pFromActor;
        anim_group::handle* hAnimGroup = pPlayer->GetAnimGroupHandlePtr();
        const anim_group *pAnimGroup   = hAnimGroup->GetPointer();
        ASSERT(pAnimGroup);
        if( pAnimGroup )
        {
            if( bLeft )
            {
                iBone = pAnimGroup->GetBoneIndex( "B_L_Hand" );
            }
            else
            {
                iBone = pAnimGroup->GetBoneIndex( "B_R_Hand" );
            }

            BonePos = pPlayer->GetBonePos( iBone );
        }
    }
    else
    {
        ASSERT(0);
    }

    if( iBone == -1 )
    {
        ASSERT(0);

        // safely recover if we have a bad bone
        BonePos = pFromActor->GetPosition();
        m_AttachBone = 0;
    }

    L2W = pFromActor->GetL2W();
    L2W.SetTranslation( BonePos );

    return TRUE;
}

//=============================================================================
// Returns spline position
static vector3 GetSplinePos( const vector3& P0,
                             const vector3& V0,
                             const vector3& P1,
                             const vector3& V1,
                             f32 T )
{
    // Compute time powers
    f32 T2   = T*T ;
    f32 T3   = T2*T ;
    f32 T3x2 = T3*2 ;
    f32 T2x3 = T2*3 ;

    // Compute coefficients
    f32 w0 =  T3x2 - T2x3 + 1.0f ;
    f32 w1 = -T3x2 + T2x3 ;    
    f32 w2 =  T3  - (2.0f*T2) + T ; 
    f32 w3 =  T3  -  T2 ;   

    // Compute final spline position
    return ( (w0*P0) + (w1*P1) + (w2*V0) + (w3*V1) ) ;
}

//=============================================================================
f32 g_SegmentDivisor = 0.15f;
f32 g_TendrilAmp = 50.0f;
f32 g_TendrilPhase = 1.0f;
f32 g_TendrilFlailAngle = R_90;
xbool g_TendrilUseYaw = TRUE;

void mutant_tendril_projectile::UpdateSegments( object *pOwner, f32 DeltaTime )
{
    (void)DeltaTime;

    vector3 Delta = vector3(0.0f, 0.0f, 0.0f);
    vector3 A, B;
    f32 d = g_TendrilTweaks.m_SegmentDistance;

    f32 Dist = 0.0f;
    f32 Elasticity = g_TendrilTweaks.m_SegmentElasticity;
    
    // fixup if for some reason we couldn't find the bone
    if( m_AttachBone == -1 || m_AttachBone == 0 )
    {
        ASSERT(0);
        
        vector3 BonePos = pOwner->GetPosition();
        m_SegmentPositions[MAX_SEGMENTS-1] = BonePos;
    }
    else
    {
        m_SegmentPositions[MAX_SEGMENTS-1] = ((actor*)pOwner)->GetBonePos(m_AttachBone);
    }

    // re-Enter body stage?
    if( m_bRetractTendrils )
    {
        // force them to collapse on each other            
        d = 0.0f;

        // if we are entering body, make sure we use the different elasticity
        Elasticity = g_TendrilTweaks.m_EnterBodySegmentElasticity;       

        // loop through all, tail is updated above
        for( s32 i = MAX_SEGMENTS-2; i >= 0; i-- )
        {
            if( !m_bRetractTendrils )
            {
                // don't move head unless we are retracting
                if( i == 0 )
                {
                    continue;
                }
            }

            A = m_SegmentPositions[i+1]; // leading
            B = m_SegmentPositions[i];   // following

            // update segment positions
            Delta = A - B;
            Dist = Delta.Length();
            
            if( Dist > d )
            {
                Delta.Normalize();
                Delta *= ( (Dist - d) * Elasticity ); // add in elasticity factor i.e. 0.75f
                B += Delta;
                m_SegmentPositions[i] = B;
            }
        }
    }
    else
    {   
        // this is the head
        m_SegmentPositions[0] = m_NewPos;

        // offset the tendril a bit so it's in the body.
        m_SegmentPositions[0].GetY() -= g_TendrilTweaks.m_TendrilOffsetY;

        // force them to collapse on each other            
        Delta = m_SegmentPositions[0] - m_SegmentPositions[MAX_SEGMENTS-1];

        Dist = Delta.Length();

        d = Delta.Length() / (MAX_SEGMENTS-1);
        d = d/g_SegmentDivisor;

        m_SplineTime += DeltaTime;

        // get location of bone in hand
        vector3 StartPos = m_SegmentPositions[MAX_SEGMENTS-1];

        radian Pitch, Yaw;

        vector3 Dir = m_NewPos - StartPos;
        vector3 StartVelocity = Dir;

        Pitch = g_TendrilFlailAngle * x_sin( ((m_SplineTime * g_TendrilAmp) + x_frand(0.0f, g_TendrilPhase)) );
        
        if( g_TendrilUseYaw )
            Yaw = g_TendrilFlailAngle * x_sin( ((m_SplineTime * g_TendrilAmp) + x_frand(0.0f, g_TendrilPhase)) );
        else
            Yaw =   R_0;

        StartVelocity.Set(Pitch, Yaw);
        StartVelocity.NormalizeAndScale(d);
        
        vector3 EndVelocity = Dir;
        Pitch = g_TendrilFlailAngle * x_sin( ((m_SplineTime * g_TendrilAmp) + x_frand(0.0f, g_TendrilPhase)) );
        
        if( g_TendrilUseYaw )
            Yaw = g_TendrilFlailAngle * x_sin( ((m_SplineTime * g_TendrilAmp) + x_frand(0.0f, g_TendrilPhase)) );
        else
            Yaw =   R_0;
        
        EndVelocity.Set(Pitch, Yaw);        
        EndVelocity.NormalizeAndScale(d);

        // spline position
        for( s32 i = 0; i < MAX_SEGMENTS; i++ )
        {
            f32 SplineT = ((f32)i)/((f32)(MAX_SEGMENTS-1));

            SplineT = 1.0f - SplineT;

            if( SplineT > 1.0f )
                SplineT = 1.0f;

            m_SegmentPositions[i] = GetSplinePos( StartPos, StartVelocity, m_NewPos, EndVelocity, SplineT );
        }
    }
}

//=============================================================================
void mutant_tendril_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OriginGuid );

    if( g_bHackTendrilRetract )
    {
        if( m_HackTendrilRetract )
        {
            m_HackTendrilTime -= DeltaTime;

            if( m_HackTendrilTime <= F32_MIN )
            {
                m_HackTendrilRetract = FALSE;
                m_bRetractTendrils = TRUE;
            }
        }
    }

    // tendrils are "reeling in"
    if( m_bRetractTendrils )
    {
        m_CollapseTime -= DeltaTime;

        if( m_CollapseTime <= F32_MIN )
        {
            OnExplode();
            return;
        }
    }

    // ok, we've killed something, update bone position as needed
    if( m_CorpseGuid )
    {
        OnTransform(GetL2W());

        corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);

        ASSERT(pCorpse);

        // Updating the bone position - the tendrils will follow.
        if( pCorpse )
        {
            // We don't have a bone, but we have a corpse.
            if( m_iBone == -1 )
            {
                return;
            }

            // Lookup physics instance
            physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();

            // get the bone's world position
            m_BonePosition = PhysicsInst.GetBoneWorldPosition( m_iBone );

            // just make sure tendril ends up in the right place
            m_StartPos = m_NewPos = m_BonePosition;

            // update tendril segments
            UpdateSegments( pOwner, DeltaTime );
        }

        // get out
        return;
    }

    // hit a body, make sure our tendril head and our projectile are in the right spot
    if( m_bHitBody )
    {
        // just make sure tendril ends up in the right place
        m_StartPos = m_NewPos = m_BonePosition;
    }

    // update tendril segments
    UpdateSegments( pOwner, DeltaTime );
    
    // update bone position just in case
    GetAimAtPosition();

    // make sure we don't go too far
    // if we're retracting, don't do normal net projectile behavior
    if( !m_bRetractTendrils )
    {
        net_proj::OnAdvanceLogic( DeltaTime );
    }

    OnTransform(GetL2W());
}

//=============================================================================
bbox mutant_tendril_projectile::GetLocalBBox( void ) const 
{
    bbox lBox;
    lBox.Clear();
    vector3 Verts[MAX_SEGMENTS];

    matrix4 W2L = GetL2W();
    W2L.InvertRT();

    // put verts in local space
    for( s32 i = 0; i < MAX_SEGMENTS; i++ )
    {
        Verts[i] = (W2L * m_SegmentPositions[i]);
    }
    
    // add verts and build bbox
    lBox.AddVerts(Verts, MAX_SEGMENTS);
    lBox.Inflate(g_TendrilTweaks.m_SegmentSize, g_TendrilTweaks.m_SegmentSize, g_TendrilTweaks.m_SegmentSize);    

    return lBox;
}

//=============================================================================
void mutant_tendril_projectile::CausePain( collision_mgr::collision& Coll, object* pObject )
{
    // Apply pain
    pain Pain;
    Pain.Setup( m_PainHandle, m_OriginGuid, Coll.Point );
    Pain.SetDirection( m_Velocity );
    Pain.SetDirectHitGuid( pObject->GetGuid() );
    Pain.ApplyToObject( pObject );

    // this is a destructable but, it's not a living thing... get out
    if( !(pObject->GetAttrBits() & ATTR_LIVING) )
        return;

    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
    if( pOwner->IsKindOf( player::GetRTTI() ) )
    {
        player *pPlayer = ((player*)pOwner);
        f32 maxMutagen  = pPlayer->GetMaxMutagen();

        // make this a percentage        
        f32 mutagenPct  = (0.1f);
        
        mutagenPct = x_round((maxMutagen*mutagenPct), 0.1f);

        // give the player mutagen back
        pPlayer->AddMutagen( mutagenPct );
    }
}

//=============================================================================

void mutant_tendril_projectile::OnEnumProp( prop_enum& PropEnumList )
{
    net_proj::OnEnumProp( PropEnumList );

    PropEnumList.PropEnumHeader( "Render", "Render information.", 0 );

    //--------------------------------------------------------------------------
    // Render instance for melee tendrils.
    //--------------------------------------------------------------------------
    PropEnumList.PropEnumHeader  ( "Render\\High res Inst", "Mutation melee attack tendril render instance", 0 );
    s32 ID = PropEnumList.PushPath( "Render\\High res Inst\\" );
    m_SkinInst.OnEnumProp( PropEnumList );
    PropEnumList.PopPath( ID );
    //--------------------------------------------------------------------------
}

//=============================================================================

xbool mutant_tendril_projectile::OnProperty( prop_query& rPropQuery )
{
    if( net_proj::OnProperty( rPropQuery ) )
    {
        // Nothing to do
        return TRUE;
    }

    s32 ID = rPropQuery.PushPath( "Render\\High res Inst\\" );

    if( m_SkinInst.OnProperty( rPropQuery ) )
    {
        if( !rPropQuery.IsRead() && rPropQuery.IsVar("RenderInst\\File") )
        {
            // initialization stuff here?
        }

        rPropQuery.PopPath( ID );

        return TRUE;
    }

    rPropQuery.PopPath( ID );

    return FALSE;
}

//=============================================================================

vector3 mutant_tendril_projectile::GetAimAtPosition( void )
{
    vector3 AimAt;

    // Get a position along our velocity.  
    // This shouldn't be needed as the player should check for target before firing tendrils.
    vector3 FarAway( m_Velocity * g_TendrilMaxLength );
    vector3 NoTargetPosition = GetPosition() + FarAway;

    if( m_Target )
    {
        object *pTargetObject = g_ObjMgr.GetObjectByGuid( m_Target );

        // valid target?
        if( pTargetObject )
        {
            AimAt = GetTargetPoint(pTargetObject);
        }
        else
        {           
            AimAt = NoTargetPosition;
        }
    }
    else
    {
        AimAt = NoTargetPosition;
    }

    return AimAt;
}

//=============================================================================
vector3 mutant_tendril_projectile::GetTargetPoint( object* pTarget )
{
    vector3 TargetPos = vector3(0.0f, 0.0f, 0.0f );

    if( pTarget && pTarget->IsKindOf(actor::GetRTTI()) )
    {
        actor &ActorSource = actor::GetSafeType( *pTarget );

        // generic NPC such as dust mites
        if( pTarget->IsKindOf(genericNPC::GetRTTI()) )
        {
            // no vector realignment for things like dust mites
            TargetPos = ActorSource.GetColBBox().GetCenter();
        }
        else
        {
            s32 iBone = -1;

            if( ActorSource.GetLocoPointer()->m_Player.HasAnimGroup() )
            {
                // KSS -- FIXME -- this needs to be random chest bones?
                //iBone = pFromActor->GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Spine02" );
                // remember left tendril should go to right arm and vice versa
                if( m_bLeft )
                {
                    iBone = ActorSource.GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Arm_R_UpperArm" );
                }
                else
                {
                    iBone = ActorSource.GetLocoPointer()->m_Player.GetBoneIndex( "B_01_Arm_L_UpperArm" );
                }

                // save off bone bone
                m_iBone = iBone;

                // this is our bone on the target we are hitting
                TargetPos = ActorSource.GetBonePos(iBone);

                // where was the "head bone" when we hit?
                m_BonePosition = TargetPos;
            }
            else
            {
                // no animgroup, just get the center
                TargetPos = ActorSource.GetColBBox().GetCenter();
            }
        }
    }
    else
    {
        // probably not an actor?  Should be impossible.
        if( pTarget )
        {
            TargetPos = pTarget->GetPosition();
        }
    }

    return TargetPos;
}

//=============================================================================

void mutant_tendril_projectile::OnColCheck( void )
{
    // do nothing
}

