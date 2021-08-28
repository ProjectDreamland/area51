//=========================================================================
// GRAVITON CHARGE PROJECTILE
//=========================================================================

#ifndef _MESONSEEKER_PROJECTILE_HPP
#define _MESONSEEKER_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Objects\NetProjectile.hpp"
#include "Objects\WeaponMSN.hpp"

//=========================================================================

class mesonseeker_projectile : public net_proj
{
public:
	CREATE_RTTI( mesonseeker_projectile , net_proj , object )

	mesonseeker_projectile();
	virtual ~mesonseeker_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================

                void            Setup               (   guid              OriginGuid,
                                                        s32               OriginNetSlot,
                                                        const vector3&    Position,
                                                        const radian3&    Orientation,
                                                        const vector3&    Velocity,
                                                        s32               Zone1,
                                                        s32               Zone2,
                                                        pain_handle       PainHandle,
                                                        firing_stage      FiringStage);

virtual         void            SetStart            ( const vector3& Position,
                                                      const radian3& Orientation,
                                                      const vector3& Velocity,
                                                            s32      Zone1,
                                                            s32      Zone2,
                                                            f32      Gravity = 0.0f );



                xbool           LoadEffect          ( const char* pFileName, const vector3& InitPos, const radian3& InitRot );
                void            DestroyParticles    ( void );
                void            UpdateParticles     ( const vector3& Position );
virtual	        bbox	        GetLocalBBox		( void ) const;

virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
virtual         void            OnImpact            ( collision_mgr::collision& Coll, object* pTarget );

virtual	        void	        OnMove				( const vector3& NewPosition );
virtual         void            OnRender            ( void );

virtual         void            OnExplode           ( void );
virtual         void            SetTarget           ( guid target )             { m_SeekerTarget = target; }
                xbool	        LoadInstance		( const char* pFileName );
                void            ReleaseProjectile   ( xbool bHoming, f32 scalar );
                void            UpdateProjectile    ( const matrix4& L2W, f32 time );

#ifndef X_EDITOR
virtual         void            net_Deactivate      ( void );
#endif

public:
    static f32                          s_SeekerGrenade_Alert_Time ;

protected:
    guid                                m_SeekerTarget;
    f32                                 m_YawRate;

    f32                                 m_TimeSinceLastBroadcast;
    radian3                             m_Spin;
    radian3                             m_TotalSpin;
    rigid_inst		                    m_RigidInst;		// Instance for rendering object.

	f32			                        m_MaxAliveTime;			// How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;
    guid                                m_EffectTrail;          // Guid of the effect trail
    
    xbool                               m_bBroadcastAlert;

    rhandle<char>                       m_ProjectileFx;     // The energy projectile particle effect.    
    rhandle<decal_package>              m_hDecalPackage;
    f32                                 m_Scale;            // our size
    f32                                 m_Scalar;           // damage, size and explosion radius modifier
};

//=========================================================================
// END
//=========================================================================
#endif