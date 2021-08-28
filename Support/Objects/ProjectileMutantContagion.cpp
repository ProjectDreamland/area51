//=============================================================================================
// MESON SEEKER PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileMutantContagion.hpp"
#include "Entropy\e_Draw.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Render\LightMgr.hpp"
#include "Tracers\TracerMgr.hpp"
#include "Objects\actor\actor.hpp"
#include "Decals\DecalMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "Objects\Player.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "Objects\ParticleEmiter.hpp"

#define CONTAGION_AUDIO_BRANCH      "Contagion_Branch"
#define CONTAGION_AUDIO_ATTACK      "Contagion_Attack"
#define CONTAGION_AUDIO_LAUNCH      "Contagion_Launch"
#define CONTAGION_AUDIO_FLY_LOOP    "Contagion_Fly_Loop"

#define CONTAGION_PARTICLE_BRANCH           "MucousBurst_000.fxo"
#define CONTAGION_PARTICLE_EXPLOSION        "MucousBurst_000.fxo"
#define CONTAGION_PARTICLE_FLYING           "Theta_shield_Single.fxo"
#define CONTAGION_MAIN_PARTICLE_FLYING      "Theta_shield_Single.fxo"

#define CONTAGION_COLOR_ARMING              (xcolor(67,120,57))//(xcolor(150,255,150))
#define CONTAGION_COLOR_ARMED               (xcolor(135,122,71))//(xcolor(255,255,150))
#define CONTAGION_COLOR_ATTACKING           (xcolor(120,58,94))//(xcolor(255,150,150))

#define CONTAGION_FROM_WEAPON_ATTACK_TIME   ( 1.0f )

tweak_handle SecondaryHealthTweak       ("MUTATION_SecondaryHealthPct");    // How health does each contagion return when it hits?
tweak_handle SecondaryMutagenTweak      ("MUTATION_SecondaryMutagenPct");   // How mutagen does each contagion return when it hits?
tweak_handle ThetaContagionHealthTweak  ("THETA_ContagionHealthPercent");   // What percent of health does each contagion give the Theta when it hits the player?

const f32 k_TimeTillThetaLaunch = 1.5f;

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct mutant_contagion_projectile_desc : public object_desc
{
    mutant_contagion_projectile_desc( void ) : object_desc( 
            object::TYPE_MUTANT_CONTAGION_PROJECTILE, 
            "Mutant Contagion Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_NO_RUNTIME_SAVE    |
            object::ATTR_RENDERABLE         | 
            object::ATTR_TRANSPARENT,
            FLAGS_IS_DYNAMIC /*| FLAGS_GENERIC_EDITOR_CREATE*/ )
            {}

    virtual object* Create          ( void )
    {
        return new mutant_contagion_projectile;
    }

} s_Mutant_Contagion_Projectile_Desc;

//=============================================================================================

const object_desc&  mutant_contagion_projectile::GetTypeDesc     ( void ) const
{
    return s_Mutant_Contagion_Projectile_Desc;
}


//=============================================================================================
const object_desc&  mutant_contagion_projectile::GetObjectType   ( void )
{
    return s_Mutant_Contagion_Projectile_Desc;
}

//=============================================================================================

mutant_contagion_projectile::mutant_contagion_projectile()
{
    m_LocalBBox = bbox( vector3( 50.0f, 50.0f, 50.0f),
                        vector3(-50.0f,-50.0f,-50.0f) );
    m_FlyVoiceID = 0;

    s32 i;
    for( i=0; i<MAX_CONTAGION; i++ )
        m_Contagion[i].m_pContagionProjectile = this;

    // Setup effect
    rhandle<char> FxResource;
    FxResource.SetName( CONTAGION_MAIN_PARTICLE_FLYING );
    m_FXHandle.InitInstance( FxResource.GetPointer() );

    m_bFromTheta = FALSE;
    m_ThetaTarget = NULL_GUID;
    m_ThetaContagionAttackTime = 0.0f;

}

//=============================================================================================

mutant_contagion_projectile::~mutant_contagion_projectile()
{
    m_FXHandle.KillInstance();

    // Just to be safe
    g_AudioMgr.Release( m_FlyVoiceID, 0.10f );
}

//=============================================================================

void mutant_contagion_projectile::Setup( guid              OriginGuid,
                                    s32               OriginNetSlot,
                                    const vector3&    Position,
                                    const radian3&    Orientation,
                                    const vector3&    Velocity,
                                    s32               Zone1,
                                    s32               Zone2,
                                    pain_handle       PainHandle )
{
    SetOrigin    ( OriginGuid, OriginNetSlot );
    SetStart     ( Position, Orientation, Velocity, Zone1, Zone2, 0.0f );
    SetPainHandle( PainHandle );

    m_State = STATE_TRAVEL;
    m_TimeInState = 0.0f;

    // Start up audio
    g_AudioMgr.Play( CONTAGION_AUDIO_LAUNCH, GetPosition(), GetZone1(), TRUE );
    m_FlyVoiceID = g_AudioMgr.Play( CONTAGION_AUDIO_FLY_LOOP, GetPosition(), GetZone1(), TRUE );
}

//==============================================================================

void mutant_contagion_projectile::SetStart( const vector3& Position, 
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

bbox mutant_contagion_projectile::GetLocalBBox( void ) const 
{ 
    return m_LocalBBox;
}

//=============================================================================

void mutant_contagion_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_Exploded )
    {
        net_proj::OnAdvanceLogic( DeltaTime );
        return;
    }

    m_DeltaTime = DeltaTime;
    m_TimeInState += m_DeltaTime;

    switch( m_State )
    {
    case STATE_TRAVEL:      UpdateTravel(); break;
    case STATE_ARMING:      UpdateArming(); break;
    case STATE_ARMED_IDLE:  UpdateArmedIdle(); break;
    case STATE_ATTACK:      UpdateAttack(); break;
    }

    // theta specific stuff yo.
    if( m_bFromTheta &&
        m_TimeInState >= k_TimeTillThetaLaunch )
    {
        // Force attack immediately
        BeginAttack( m_ThetaContagionAttackTime, m_ThetaTarget );
        m_bFromTheta = FALSE;
    }

    // If we decided to be destroyed then just return.
    if( GetAttrBits() & ATTR_DESTROY )
        return;

    // Advance the contagion
    {
        s32 i;
        for( i=0; i<MAX_CONTAGION; i++ )
        {
            m_Contagion[i].AdvanceLogic( m_DeltaTime );
        }
    }

    // Update BBox for rendering
    if( m_State > STATE_TRAVEL )
    {
        bbox BBox;
        BBox.Clear();

        s32 i;
        for( i=0; i<MAX_CONTAGION; i++ )
        {
            BBox += m_Contagion[i].GetPosition();
        }

        BBox.Inflate( 50, 50, 50 );
        BBox.Translate( -GetPosition() );
        m_LocalBBox = BBox;

        // Move object to same location so bbox gets updated.
        OnMove( GetPosition() );
    }

    if( m_FlyVoiceID )
    {
        g_AudioMgr.SetPosition(m_FlyVoiceID, GetPosition(), GetZone1() );
    }
}

//==============================================================================

void mutant_contagion_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    m_ImpactNormal = Coll.Plane.Normal;

    net_proj::OnImpact( Coll, pTarget );

    // Move to new position and explode
    OnMove( m_NewPos );

    // If currently in the travel state then enter the arming state
    if( m_State == STATE_TRAVEL )
    {
        BeginArming();
        return;
    }
}

//=============================================================================

void mutant_contagion_projectile::OnExplode( void )
{
    if( m_Exploded )
        return;

    net_proj::OnExplode();
}

//=========================================================================

void mutant_contagion_projectile::OnMove( const vector3& NewPosition )
{
    net_proj::OnMove( NewPosition );
}

//=========================================================================

void mutant_contagion_projectile::OnRender( void )
{
}

//=========================================================================

void mutant_contagion_projectile::OnRenderTransparent( void )
{
    if( m_State == STATE_TRAVEL )
    {
        m_FXHandle.Render();
    }

    // Loop through contagion
    if( m_State > STATE_TRAVEL )
    {
        s32 i;
        for( i=0; i<MAX_CONTAGION; i++ )
        {
            m_Contagion[i].Render();
        }
    }
}

//=============================================================================================

#ifndef X_EDITOR

void mutant_contagion_projectile::net_Deactivate( void )
{
//    if( (GameMgr.GameInProgress()) && 
//        (m_OwningClient != g_NetworkMgr.GetClientIndex()) )
//    {
//        DESTROY_NET_OBJECT( this );
//    }
}

#endif

//=============================================================================================

void mutant_contagion_projectile::UpdateTravel( void )
{
    net_proj::OnAdvanceLogic( m_DeltaTime );
    g_AudioMgr.SetPosition( m_FlyVoiceID, GetPosition(), GetZone1() );

    // Update effect
    const vector3 Backwards( -m_Velocity );
    radian3 Rotation( Backwards.GetPitch(), Backwards.GetYaw(), 0.0f );
    m_FXHandle.SetRotation( Rotation );
    m_FXHandle.SetTranslation( GetPosition() );
    m_FXHandle.AdvanceLogic( m_DeltaTime );
}

//=============================================================================================

void mutant_contagion_projectile::BeginArming( void )
{
    m_State = STATE_ARMING;
    m_TimeInState = 0;
    m_Velocity.Zero();

    g_AudioMgr.Release( m_FlyVoiceID, 0.10f );

    s32 nInitialContagion=8;

    vector3 StartPos = GetPosition() + m_ImpactNormal;
    vector3 StartNormal = m_ImpactNormal;

    // Setup initial group of contagions
    {
        s32 i;
        for( i=0; i<nInitialContagion; i++ )
        {
            contagion& CG = m_Contagion[i];
            vector3 ArmPos = CG.ComputeArmPoint( StartPos, StartNormal );
            CG.BeginArming( StartPos, ArmPos, StartNormal );
        }
    }

    // Setup remaining group as branches
    {
        s32 nLookBack=8;
        s32 i;
        for( i=nInitialContagion; i<MAX_CONTAGION; i++ )
        {
            s32 iLookBack = i - nLookBack;
            if( iLookBack < 0 ) iLookBack = 0;
            s32 iBranch = x_irand( iLookBack, i-1 );
            f32 BranchT = x_frand( 0.4f, 0.9f );
            ASSERT( (iBranch>=0) && (iBranch<i) );

            //BranchT = 100000.0f;
            m_Contagion[i].BeginArming( &m_Contagion[iBranch], BranchT );
        }
    }
}

//=============================================================================================

void mutant_contagion_projectile::UpdateArming( void )
{
    if( m_TimeInState > 2.0f )
        BeginArmedIdle();
}

//=============================================================================================

void mutant_contagion_projectile::BeginArmedIdle( void )
{
    m_State = STATE_ARMED_IDLE;
    m_TimeInState = 0;
}

//=============================================================================================

void mutant_contagion_projectile::UpdateArmedIdle( void )
{
}

//=============================================================================================

void mutant_contagion_projectile::TriggerFromWeapon( void )
{
    if( m_State == STATE_TRAVEL )
    {
        m_ImpactNormal = m_Velocity;
        m_ImpactNormal.Normalize();
        BeginArming();
        m_Velocity.Zero();
    }
    else
    if( (m_State==STATE_ARMING) || (m_State == STATE_ARMED_IDLE) )
    {
        BeginAttack( CONTAGION_FROM_WEAPON_ATTACK_TIME );
    }
}

//=============================================================================================

void mutant_contagion_projectile::TriggerFromTheta( f32 ContagionAttackTime, guid TargetGuid )
{
    m_bFromTheta = TRUE;
    m_ThetaContagionAttackTime = ContagionAttackTime;
    m_ThetaTarget = TargetGuid;
    // Force arming of all contagion
    m_ImpactNormal = m_Velocity;
    m_ImpactNormal.Normalize();
    BeginArming();
    m_Velocity.Zero();
}

//=============================================================================================

xbool mutant_contagion_projectile::IsTargeted( guid OriginGuid, guid TargetGuid )
{
    // Loop through all contagion projectiles
    for( slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_MUTANT_CONTAGION_PROJECTILE ); Slot != SLOT_NULL; Slot = g_ObjMgr.GetNext( Slot ) )
    {
        // Lookup projectile
        mutant_contagion_projectile* pProjectile = (mutant_contagion_projectile*)g_ObjMgr.GetObjectBySlot( Slot );
        ASSERT( pProjectile );
    
        // Owner match?
        if( pProjectile->GetOriginGuid() == OriginGuid )
        {
            // Check all contagion
            for( s32 i = 0; i < MAX_CONTAGION; i++ )
            {
                // Lookup contagion
                contagion& Contagion = pProjectile->m_Contagion[ i ];

                // Targeted?
                if( ( Contagion.m_State != contagion::STATE_INACTIVE ) && ( Contagion.m_TargetGuid == TargetGuid ) )
                    return TRUE;
            }
        }
    }
        
    // Not yet targeted
    return FALSE;
}

//=============================================================================================

void mutant_contagion_projectile::BeginAttack( f32 ContagionAttackTime )
{
    m_State = STATE_ATTACK;
    m_TimeInState = 0;

    // Create list of targets
    const s32 MAX_TARGETS = 128;
    guid TargetGuid[MAX_TARGETS];
    vector3 TargetPos[MAX_TARGETS];
    s32  nTargets=0;
    {
        actor* pParent = (actor*)g_ObjMgr.GetObjectByGuid( m_OriginGuid );
        if( pParent )
        {
            // Collect living targets
            {
                actor* pNextActor = actor::m_pFirstActive;
                while( pNextActor )
                {
                    // Get ptr to actor and advance to next
                    actor* pActor = pNextActor;
                    pNextActor = pNextActor->m_pNextActive;

                    if( (pActor->GetGuid() != pParent->GetGuid()) && 
                        pActor->IsKindOf( actor::GetRTTI() ) &&
                        !pActor->IsDead() )
                    {
                        if( pParent->IsEnemyFaction( pActor->GetFaction() ) )
                        {
                            if( nTargets < MAX_TARGETS )
                            {
                                TargetGuid[nTargets] = pActor->GetGuid();
                                TargetPos[nTargets] = pActor->GetPosition();
                                nTargets++;
                            }
                        }
                    }
                }
            }

            // Collect turrets
            {
                slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_TURRET );
                while( Slot != SLOT_NULL )
                {
                    object* pObj = g_ObjMgr.GetObjectBySlot( Slot );
                    Slot = g_ObjMgr.GetNext( Slot );

                    if( nTargets < MAX_TARGETS )
                    {
                        TargetGuid[nTargets] = pObj->GetGuid();
                        TargetPos[nTargets] = pObj->GetPosition();
                        nTargets++;
                    }
                }
            }

            // Collect alien globs
            {
                slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_ALIEN_GLOB );
                while( Slot != SLOT_NULL )
                {
                    object* pObj = g_ObjMgr.GetObjectBySlot( Slot );
                    Slot = g_ObjMgr.GetNext( Slot );

                    if( nTargets < MAX_TARGETS )
                    {
                        TargetGuid[nTargets] = pObj->GetGuid();
                        TargetPos[nTargets] = pObj->GetPosition();
                        nTargets++;
                    }
                }
            }
        }        
    }

    // Keep only those targets that are within attack distance of ANY contagion
    {
        s32 i,j;
        s32 nNewTargets=0;
        for( j=0; j<nTargets; j++ )
        {
            xbool bFoundAttacker = FALSE;

            for( i=0; i<MAX_CONTAGION; i++ )
            if( m_Contagion[i].m_State != contagion::STATE_INACTIVE )
            {
                f32 Dist = (m_Contagion[i].m_CurrentPos - TargetPos[j]).LengthSquared();
                if( Dist < 600.0f*600.0f )
                {
                    bFoundAttacker = TRUE;
                    break;
                }
            }

            if( bFoundAttacker )
            {
                TargetGuid[nNewTargets] = TargetGuid[j];
                TargetPos[nNewTargets] = TargetPos[j];
                nNewTargets++;
            }
        }

        nTargets = nNewTargets;
    }

    // Choose targets for each contagion randomly
    {
        s32 i,j;
        s32 nActiveContagion=0;
        for( i=0; i<MAX_CONTAGION; i++ )
        if( m_Contagion[i].m_State != contagion::STATE_INACTIVE )
            nActiveContagion++;

        s32 nMatches = MIN(nTargets,nActiveContagion);

        // Be sure each target gets at least one contagion if possible
        j=0;
        for( i=0; i<nMatches; i++ )
        {
            while( m_Contagion[j].m_State == contagion::STATE_INACTIVE )
                j++;

            ASSERT( j < MAX_CONTAGION );
            m_Contagion[j].SetTarget( TargetGuid[i] );
            m_Contagion[j].BeginAttack( ContagionAttackTime );

            j++;
        }

        for( i=0; i<MAX_CONTAGION; i++ )
        if(( m_Contagion[i].m_State != contagion::STATE_INACTIVE ) && (m_Contagion[i].m_TargetGuid==0) )
        {
            if( nTargets>0 )
            {
                s32 iTarget = x_irand(0,nTargets-1);
                m_Contagion[i].SetTarget( TargetGuid[iTarget] );
            }
            else
            {
                s32 iTarget = x_irand(0,MAX_CONTAGION-1);
                m_Contagion[i].SetTarget( m_Contagion[iTarget].m_CurrentPos );
            }
            
            m_Contagion[i].BeginAttack( ContagionAttackTime );
        }
    }

    g_AudioMgr.Play( CONTAGION_AUDIO_ATTACK, GetPosition(), GetZone1(), TRUE );
}

//=============================================================================================

void mutant_contagion_projectile::BeginAttack( f32 ContagionAttackTime, guid TargetGuid )
{
    // Switch to attack state
    m_State = STATE_ATTACK;
    m_TimeInState = 0;

    // Setup all contagion
    for( s32 i = 0; i < MAX_CONTAGION; i++ )
    {
        // Lookup contagion
        contagion& Contagion = m_Contagion[ i ];
    
        // Setup if inactive
        if( ( Contagion.m_State != contagion::STATE_INACTIVE ) && ( Contagion.m_TargetGuid == 0 ) )
        {
            // Got for target
            Contagion.SetTarget( TargetGuid );
            Contagion.BeginAttack( ContagionAttackTime );
        }
    }
    
    // Trigger sfx
    g_AudioMgr.Play( CONTAGION_AUDIO_ATTACK, GetPosition(), GetZone1(), TRUE );
}

//=============================================================================================

void mutant_contagion_projectile::UpdateAttack( void )
{
    s32 i;
    for( i=0; i<MAX_CONTAGION; i++ )
    if( m_Contagion[i].m_State != contagion::STATE_INACTIVE )
        break;

    if( i==MAX_CONTAGION )
    {
        // Shake camera and controller
        {
            object* pParent = (object*)g_ObjMgr.GetObjectByGuid( m_OriginGuid );
            if( pParent->IsKindOf( player::GetRTTI() ) )
            {
                player* pPlayer = (player*)pParent;
                f32 s_HitShakeTime     = 0.8f;
                f32 s_HitShakeAmount   = 1.0f;
                f32 s_HitShakeSpeed    = 1.5f;
                pPlayer->ShakeView( s_HitShakeTime, s_HitShakeAmount, s_HitShakeSpeed );
                pPlayer->DoFeedback((s_HitShakeTime/2.0f), s_HitShakeAmount);
            }
        }

        #ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
        #endif
        {
            OnExplode();
        }
    }
}

//=============================================================================================
//=============================================================================================
// CONTAGION
//=============================================================================================
//=============================================================================================

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

//=============================================================================================

contagion::contagion( void )
{
    m_State         = STATE_INACTIVE;
    m_SplineRate    = 0.0f;
    m_pBranch       = NULL;
    m_BranchT       = 0;
    m_Color         = CONTAGION_COLOR_ARMING;

    // Setup effect
    rhandle<char> FxResource;
    FxResource.SetName( CONTAGION_PARTICLE_FLYING );
    m_FXHandle.InitInstance( FxResource.GetPointer() );
}

//=============================================================================================

contagion::~contagion( void )
{
    m_FXHandle.KillInstance();
}

//=============================================================================================

void    contagion::BeginArming( const vector3& SourcePos, const vector3& ArmedPos, const vector3& LaunchNormal )
{
    ASSERT( (m_State==STATE_INACTIVE) || (m_State==STATE_WAITING_TO_BRANCH) );
    m_State = STATE_ARMING;

    m_CurrentPos = SourcePos;
    m_PrevPos = m_CurrentPos;
    m_StartPos = SourcePos;
    m_EndPos   = ArmedPos;

    vector3 Dir = m_EndPos - m_StartPos;
    f32     Dist = Dir.Length();
    if( Dist > 0 )
        Dir /= Dist;

    radian Pitch,Yaw;
    LaunchNormal.GetPitchYaw(Pitch,Yaw);
    vector3 StartDir(0,0,1);
    StartDir.RotateX( x_frand(-R_60,R_60) );
    StartDir.RotateZ( x_frand( R_0, R_360) );
    StartDir.RotateX( Pitch );
    StartDir.RotateY( Yaw );
    m_StartVelocity = StartDir * (Dist*1.0f);

    m_EndVelocity = vector3(0,0,1);
    m_EndVelocity.RotateX( x_frand(R_0,R_360) );
    m_EndVelocity.RotateY( x_frand(R_0,R_360) );
    m_EndVelocity *= Dist * 1.0f;

    m_SplineRate = 1.0f / x_frand(0.5f,0.75f);
    m_SplineT = 0;
}

//=============================================================================================

void    contagion::BeginArming( contagion* pBranch, f32 BranchT )
{
    ASSERT( m_State == STATE_INACTIVE );
    m_State = STATE_WAITING_TO_BRANCH;

    m_pBranch       = pBranch;
    m_BranchT       = BranchT;
    m_SplineT       = 0;

    m_CurrentPos    = m_pBranch->m_CurrentPos;
    m_PrevPos       = m_CurrentPos;
    m_StartPos      = m_CurrentPos;
    m_EndPos        = m_CurrentPos;

}

//=============================================================================================

void    contagion::SetTarget( guid TargetGuid )
{
    m_TargetGuid = TargetGuid;
    m_TargetPos.Zero();
}

//=============================================================================================

void    contagion::SetTarget( const vector3& TargetPos )
{
    m_TargetGuid = 0;
    m_TargetPos = TargetPos;
}

//=============================================================================================

void    contagion::BeginAttack( f32 ContagionAttackTime )
{
    // If contagion has not begun arming at this point then it goes inactive
    if( m_State == STATE_WAITING_TO_BRANCH )
    {
        m_State = STATE_INACTIVE;
        return;
    }

    //ASSERT( (m_State==STATE_ARMING) || (m_State==STATE_ARMED_IDLE) );

    UpdateTargetPosition();
    m_State = STATE_ATTACKING;

    m_Color    = CONTAGION_COLOR_ATTACKING;
    m_StartPos = m_CurrentPos;
    m_EndPos   = m_TargetPos;
    m_SplineT  = 0;
    m_SplineRate = 1.0f / ContagionAttackTime;

    // If no target then vary rate
    if( m_TargetGuid==0 )
    {
        m_SplineRate = 1.0f / x_frand(ContagionAttackTime*0.75f,ContagionAttackTime*1.3f);
    }

    vector3 Dir = m_EndPos - m_StartPos;
    f32     Dist = Dir.Length();
    if( Dist > 0 )
    {
        radian Pitch,Yaw;
        Dir.GetPitchYaw(Pitch,Yaw);

        m_StartVelocity = vector3(0,0,1);
        m_StartVelocity.RotateX( x_frand(-R_20,R_20) );
        m_StartVelocity.RotateZ( x_frand( R_0, R_360) );
        m_StartVelocity.RotateX( -Pitch );
        m_StartVelocity.RotateY( -Yaw );
        m_StartVelocity *= 250.0f;

        m_EndVelocity = vector3(0,0,1);
        m_EndVelocity.RotateX( x_frand(-R_45,R_45) );
        m_EndVelocity.RotateZ( x_frand( R_0, R_360) );
        m_EndVelocity.RotateX( Pitch );
        m_EndVelocity.RotateY( Yaw );
        m_EndVelocity *= 4000.0f;
    }
    else
    {
        m_StartVelocity = vector3(0,0,1);
        m_StartVelocity.RotateX( x_frand( R_0, R_360) );
        m_StartVelocity.RotateY( x_frand( R_0, R_360) );
        m_StartVelocity *= 200.0f;

        m_EndVelocity = vector3(0,0,1);
        m_EndVelocity.RotateX( x_frand( R_0, R_360) );
        m_EndVelocity.RotateY( x_frand( R_0, R_360) );
        m_EndVelocity *= 200.0f;
    }
}

//=============================================================================================

void    contagion::AdvanceLogic( f32 DeltaTime )
{
    switch( m_State )
    {
    case STATE_INACTIVE: break;

    case STATE_WAITING_TO_BRANCH:
    case STATE_ARMING:              UpdateArming( DeltaTime ); break;
    case STATE_ARMED_IDLE:          UpdateArmedIdle( DeltaTime ); break;
    case STATE_ATTACKING:           UpdateAttack( DeltaTime ); break;
    }

    // Update effect
    if( (m_State != STATE_INACTIVE) && (m_pBranch==NULL) )
    {
        vector3 Dir = m_CurrentPos - m_PrevPos;
        Dir.SafeNormalize();
        const vector3 Backwards( -Dir );
        radian3 Rotation( Backwards.GetPitch(), Backwards.GetYaw(), 0.0f );
        m_FXHandle.SetRotation( Rotation );
        m_FXHandle.SetTranslation( m_CurrentPos );
        m_FXHandle.AdvanceLogic( DeltaTime );
    }
}

//=============================================================================================

void    contagion::BeginArmedIdle( void )
{
    ASSERT(m_State==STATE_ARMING);
    m_State=STATE_ARMED_IDLE;
    m_SplineRate = 1.0f / x_frand(0.75f,1.25f);
    m_Color = CONTAGION_COLOR_ARMED;
}

//=============================================================================================

void    contagion::UpdateArming( f32 DeltaTime )
{
    // Are we branch based?
    if( m_pBranch )
    {
        // Is it time to branch?
        if( (m_pBranch->m_pBranch==NULL) && (m_pBranch->m_SplineT >= m_BranchT)  )
        {
            // Evaluate position of CG at BranchT.
            vector3 StartPos = m_pBranch->ComputePositionAtT( m_BranchT );
            vector3 StartNormal = m_pBranch->GetBaseDirection();
            StartNormal.Normalize();
            vector3 ArmPos = ComputeArmPoint( StartPos, StartNormal );
            BeginArming( StartPos, ArmPos, StartNormal );

            // Turn off branching
            m_pBranch = NULL;

            // Play audio
            g_AudioMgr.Play( CONTAGION_AUDIO_BRANCH, m_CurrentPos, m_pContagionProjectile->GetZone1(), TRUE );

            // Play branch effect
            //particle_emitter::CreatePresetParticleAndOrient( CONTAGION_PARTICLE_BRANCH, StartNormal, StartPos, m_pContagionProjectile->GetZone1() );
        }
        else
        {
            return;
        }
    }

    // If we aren't branch based or branch is turned off...
    if( m_pBranch == NULL )
    {
        m_SplineT += DeltaTime * m_SplineRate;
        if( m_SplineT > 1.0f )
            m_SplineT = 1.0f;

        m_PrevPos = m_CurrentPos;
        m_CurrentPos = GetSplinePos( m_StartPos, m_StartVelocity, m_EndPos, m_EndVelocity, m_SplineT );    

        if( m_SplineT == 1.0f )
            BeginArmedIdle();
    }
}

//=============================================================================================

void    contagion::UpdateArmedIdle( f32 DeltaTime )
{
    if( m_SplineT == 1.0f )
    {
        // Build a new orbit spline
        m_SplineT = 0.0f;

        m_StartPos      = m_CurrentPos;
        m_EndPos        = m_CurrentPos;
        m_StartVelocity = m_EndVelocity;
        m_EndVelocity.GetX() = x_frand(-1,1);
        m_EndVelocity.GetY() = x_frand(-1,1);
        m_EndVelocity.GetZ() = x_frand(-1,1);
        m_EndVelocity.Normalize();
        m_EndVelocity   *= 50.0f;
    };

    // Update current spline path
    m_SplineT += DeltaTime * m_SplineRate;
    if( m_SplineT > 1.0f )
        m_SplineT = 1.0f;

    m_PrevPos = m_CurrentPos;
    m_CurrentPos = GetSplinePos( m_StartPos, m_StartVelocity, m_EndPos, m_EndVelocity, m_SplineT );    
}

//=============================================================================================

void    contagion::UpdateAttack( f32 DeltaTime )
{
    // Keep adjusting end of spline to catch up with target.
    UpdateTargetPosition();
    m_EndPos = m_TargetPos;

    m_SplineT += DeltaTime * m_SplineRate;
    if( m_SplineT > 1.0f )
        m_SplineT = 1.0f;

    m_PrevPos = m_CurrentPos;
    m_CurrentPos = GetSplinePos( m_StartPos, m_StartVelocity, m_EndPos, m_EndVelocity, m_SplineT );    

    if( m_SplineT == 1.0f )
    {
        // Play detonation and apply pain to target
        pain Pain;
        Pain.Setup( m_pContagionProjectile->GetPainHandle(), m_pContagionProjectile->GetOriginGuid(), m_CurrentPos );
        Pain.SetDirectHitGuid( m_TargetGuid );
        Pain.ApplyToObject( m_TargetGuid );

        // We are going to do a 'sweet' single audio since all the 
        // contagion hit their targets at the same time.
        // Play audio
        //g_AudioMgr.Play( CONTAGION_AUDIO_EXPLOSION, m_CurrentPos );

        // Play impact effect if we have no target upon dieing
        if( m_TargetGuid == 0 )
        {
            particle_emitter::CreatePresetParticleAndOrient( CONTAGION_PARTICLE_EXPLOSION, m_EndVelocity, m_CurrentPos, m_pContagionProjectile->GetZone1() );
        }

        // If has target then give player some health
        if( m_TargetGuid )
        {
            // Get target object
            object* pTarget = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
        
            // Give our owner some health?
            object* pOwner = g_ObjMgr.GetObjectByGuid( m_pContagionProjectile->GetOriginGuid() );
            if( pOwner && pOwner->IsKindOf( player::GetRTTI()) )
            {
                player *pPlayer = ((player*)pOwner);

                // calculate percentage of max health and max mutagen
                f32 healthReturned  = (SecondaryHealthTweak.GetF32()  * pPlayer->GetMaxHealth())/100.0f;
                f32 mutagenReturned = (SecondaryMutagenTweak.GetF32() * pPlayer->GetMaxMutagen())/100.0f;

                // give it to the player
                pPlayer->AddHealth( healthReturned );
                pPlayer->AddMutagen( mutagenReturned );
            }

            // Give Theta health back (only if target is the player)?
            if(     ( pTarget ) 
                &&  ( pTarget->IsKindOf( player::GetRTTI() ) )
                &&  ( pOwner )
                &&  ( pOwner->IsKindOf( mutant_tank::GetRTTI() ) ) )
            {
                mutant_tank* pTheta = (mutant_tank*)pOwner;

                // Calculate percentage of max health
                f32 HealthReturned  = ( ThetaContagionHealthTweak.GetF32()  * pTheta->GetMaxHealth() ) / 100.0f;

                // Give it to the Theta (divide by the # of contagion per projectile)
                pTheta->AddHealth( HealthReturned / (f32)MAX_CONTAGION );
                
                // Clear target
                m_TargetGuid = 0;
            }

            // Check for destroying the target?
            if( pTarget )
            {
                // Is target a projectile?
                if( pTarget->IsKindOf( net_proj::GetRTTI() ) )
                {
                    // Destroy it immediately
                    DESTROY_NET_OBJECT( pTarget );
                }
                else if( pTarget->IsKindOf( base_projectile::GetRTTI() ) )
                {
                    // Destroy it immediately
                    g_ObjMgr.DestroyObjectEx( m_TargetGuid, TRUE );
                }
                
                // Clear target
                m_TargetGuid = 0;
            }
        }
        
        m_State = STATE_INACTIVE;
    }
}

//=============================================================================================

void contagion::ComputeSplinePathFromNormal( const vector3& StartPos, const vector3& EndPos, const vector3& Normal )
{
    m_StartPos = StartPos;
    m_EndPos   = EndPos;

    vector3 Dir = EndPos - StartPos;
    f32     Dist = Dir.Length();
    Dir /= Dist;

    radian Pitch,Yaw;
    Normal.GetPitchYaw(Pitch,Yaw);
    vector3 StartDir(0,0,1);
    StartDir.RotateX( x_frand(-R_30,R_30) );
    StartDir.RotateZ( x_frand( R_0, R_360) );
    StartDir.RotateX( Pitch );
    StartDir.RotateY( Yaw );

    m_StartVelocity = StartDir * (Dist*2.0f);
    m_EndVelocity = Dir * (Dist*0.125f);
}

//=============================================================================================

void    contagion::UpdateTargetPosition( void )
{
    if( m_TargetGuid )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
        if( pObj )
        {
            m_TargetPos = pObj->GetPosition();
        }
        else
        {
            // Could choose another target since we lost this one
            m_TargetGuid = 0;
        }
    }
}

//=============================================================================================

vector3 contagion::ComputeArmPoint( const vector3& StartPos, const vector3& Normal )
{
    vector3 BestArmPos = StartPos;
    f32     BestArmDist = -F32_MAX;
    f32     DistToCheck = x_frand(250.0f,450.0f);
    
    radian Pitch,Yaw;
    Normal.GetPitchYaw( Pitch, Yaw );

    for( s32 Tries=0; Tries<8; Tries++ )
    {
        // Decide random location for contagion to arm to.
        vector3 Dir(0,0,1);
        Dir.RotateX( x_frand(-R_80,+R_80) );
        Dir.RotateZ( x_frand(R_0,R_360) );
        Dir.RotateX( Pitch );
        Dir.RotateY( Yaw );
        f32 Dist = DistToCheck;

        g_CollisionMgr.RaySetup( 0, StartPos, StartPos + Dir*Dist );
        //g_CollisionMgr.UseLowPoly();
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_PROJECTILES, object::ATTR_COLLISION_PERMEABLE);
        if( g_CollisionMgr.m_nCollisions > 0 )
        {
            Dist = Dist * g_CollisionMgr.m_Collisions[0].T;
            Dist -= 50.0f;
            if( Dist < 50 ) Dist = 0;
        }

        if( (Dist > 0) && (Dist > BestArmDist) )
        {
            BestArmDist = Dist;
            BestArmPos = StartPos + Dir*Dist;
            if( BestArmDist == DistToCheck )
                break;
        }
    }

    return BestArmPos;
}

//=============================================================================================

vector3 contagion::ComputePositionAtT( f32 T )
{
    return GetSplinePos( m_StartPos, m_StartVelocity, m_EndPos, m_EndVelocity, T );    
}

//=============================================================================================

vector3 contagion::GetBaseDirection( void )
{
    return m_EndPos - m_StartPos;
}

//=============================================================================================

void contagion::Render( void )
{
    /*
    xcolor C = XCOLOR_WHITE;
    switch( m_State )
    {
        case STATE_ARMING: C = XCOLOR_WHITE; break;
        case STATE_ARMED_IDLE: C = XCOLOR_YELLOW; break;
        case STATE_ATTACKING: C = XCOLOR_RED; break;
    }
    */
    /*
    if( C != XCOLOR_WHITE )
    {
        draw_Sphere( m_CurrentPos, 25.0f, C );
        vector3 Dir = m_CurrentPos - m_PrevPos;
        Dir.SafeNormalize();
        Dir *= 75.0f;
        draw_Line( m_CurrentPos, m_CurrentPos+Dir, XCOLOR_GREEN );
    }
    */

    if( (m_State != STATE_INACTIVE) && (m_pBranch==NULL) )
    {
//        m_FXHandle.SetColor( m_Color );
        m_FXHandle.Render();
    }
}
//=============================================================================================
