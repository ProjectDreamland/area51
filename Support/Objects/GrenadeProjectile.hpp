//=========================================================================
// GRENADE PROJECTILE
//=========================================================================

#ifndef _GRENADEPROJECTILE_HPP
#define _GRENADEPROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "NetProjectile.hpp"
#include "objects\ParticleEmiter.hpp"

//=========================================================================

class grenade_projectile : public net_proj
{
public:
    CREATE_RTTI( grenade_projectile , net_proj , netobj )

                                grenade_projectile  ( void );
virtual                        ~grenade_projectile  ( void );

virtual const   object_desc&    GetTypeDesc         ( void ) const;
static  const   object_desc&    GetObjectType       ( void );

//=========================================================================

                 void           Setup               ( guid           OriginGuid,
                                                      s32            OriginNetSlot,
                                                      const vector3& Position,
                                                      const radian3& Orientation,
                                                      const vector3& Velocity,
                                                      s32            Zone1,
                                                      s32            Zone2,
                                                      pain_handle    PainHandle );

//=========================================================================

virtual         xbool           GetDoCollisions     ( void );
virtual         bbox            GetLocalBBox        ( void ) const;
virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
virtual         void            OnRender            ( void );
virtual         void            OnImpact            ( collision_mgr::collision& Coll, object* pTarget );
virtual         void            OnExplode           ( void );
                xbool           LoadInstance        ( const char* pFileName );
                void            SetFromNPC          ( xbool fromNPC )       { m_bFromNPC = fromNPC; }

virtual         void            OnMove              ( const vector3& NewPos );

public:
    static f32                          s_Grenade_Alert_Time;

protected:

                void            DestroyTrail        ( void );

    radian3                             m_Spin;
    radian3                             m_TotalSpin;
    
    f32                                 m_MaxAliveTime;         // How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;
    guid                                m_EffectTrail;          // Guid of the effect trail
    
    f32                                 m_SinceLastBroadcast;
    xbool                               m_bFromNPC;
//  matrix4                             m_OldMovingPlatformL2W;

    rigid_inst                          m_RigidInst;        // Instance for rendering object.
//  rhandle<char>                       m_ProjectileFx;     // The energy projectile particle effect.
//  guid                                m_FxGuid;           // Guid of the effect object.

    f32                                 m_GrenadeSoundTime;
};

//=========================================================================
#endif
//=========================================================================
