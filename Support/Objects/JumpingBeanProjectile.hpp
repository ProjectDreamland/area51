//=========================================================================
// JUMPING BEAN PROJECTILE
//=========================================================================

#ifndef _JUMPINGBEANPROJECTILE_HPP
#define _JUMPINGBEANPROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "NetProjectile.hpp"
#include "objects\ParticleEmiter.hpp"

enum grenade_mode
{
    GM_NORMAL,
    GM_EXPERT,
};

enum grenade_stage
{
    GS_STAGE_LIFTOFF,
    GS_STAGE_FALL,
    GS_STAGE_TAKEOFF,
    GS_STAGE_MAX
};

//=========================================================================

class jumping_bean_projectile : public net_proj
{
public:
    CREATE_RTTI( jumping_bean_projectile , net_proj , netobj )

                                jumping_bean_projectile  ( void );
virtual                        ~jumping_bean_projectile  ( void );

virtual const   object_desc&    GetTypeDesc         ( void ) const;
static  const   object_desc&    GetObjectType       ( void );

//=========================================================================

virtual         void            SetStart            ( const vector3& Position,
                                                     const radian3& Orientation,
                                                     const vector3& Velocity,
                                                     s32      Zone1,
                                                     s32      Zone2,
                                                     f32      Gravity = 0.0f );

virtual         bbox            GetLocalBBox        ( void ) const;
virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
virtual         void            OnRender            ( void );

virtual         void            OnExplode           ( void );
                xbool           LoadInstance        ( const char* pFileName );

virtual         vector3         GetExpertTargetPoint( void );

virtual         void            OnImpact            ( collision_mgr::collision& Coll, object* pTarget );
virtual         void            OnMove              ( const vector3& NewPos );
virtual         void            UpdateParticles     ( const vector3& NewPosition );
virtual         void            DestroyParticles    ( void );
virtual         void            CausePain           ( guid HitGuid = 0 );
virtual         void            StageLogic          ( f32 DeltaTime );

                grenade_mode    GetGrenadeMode      ( void ) { return m_GrenadeMode; }                

//------------------------------------------------------------------------------
#ifndef X_EDITOR
//------------------------------------------------------------------------------

virtual         void            net_AcceptActivate  ( const bitstream& BitStream );
virtual         void            net_ProvideActivate (       bitstream& BitStream );

virtual         void            net_AcceptStart     ( const bitstream& BitStream );
virtual         void            net_ProvideStart    (       bitstream& BitStream );

//------------------------------------------------------------------------------
#endif // X_EDITOR
//------------------------------------------------------------------------------

public:
    static f32                          s_JumpingBean_Alert_Time;

protected:
    s32                                 m_RandomSeed;
    
    f32                                 m_MaxAliveTime;         // How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;
    guid                                m_EffectTrail;          // Guid of the effect trail
    guid                                m_ExpertEffectTrail;    // what the smoke trail looks like in expert mode.
    guid                                m_ExpertEffectTrailFlame; // the flames for expert mode
    guid                                m_FragmentParticle;     // in expert mode, this will be ignition particles
    
    xbool                               m_bBroadcastAlert;

    s32                                 m_MaxBounces;

    grenade_mode                        m_GrenadeMode;          // normal or expert
    grenade_stage                       m_GrenadeStage;         // for expert mode, what state are we in?
    xbool                               m_bStageInit;           // has the grenade taken off yet (expert mode only)

    vector3                             m_TakeoffTargetPoint;   // what was the speed when we take off on third stage?    

//  matrix4                             m_OldMovingPlatformL2W;

    rigid_inst                          m_RigidInst;        // Instance for rendering object.
//  rhandle<char>                       m_ProjectileFx;     // The energy projectile particle effect.
//  guid                                m_FxGuid;           // Guid of the effect object.
};

//=========================================================================
#endif
//=========================================================================
