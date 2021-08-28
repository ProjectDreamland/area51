//=========================================================================
// GRAVITON CHARGE PROJECTILE
//=========================================================================

#ifndef _SEEKER_PROJECTILE_HPP
#define _SEEKER_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"
#include "objects\ParticleEmiter.hpp"

//=========================================================================

class seeker_projectile : public base_projectile
{
public:
	CREATE_RTTI( seeker_projectile , base_projectile , object )

	seeker_projectile();
	virtual ~seeker_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
//
// GetMaterial		-   Legacy.  Needed for now.
// OnAdvanceLogic	-   Updates every frame
// OnMove			-   Handles the motion of the spike
// Initialize		-	Sets the initial position of the object.  Needs an initial
//						velocity, an initial position, and either a radian3 or matrix4 for
//						rotation information.
// LoadColorTable	-	Loads the color tables for this object.
//
//=========================================================================

    virtual void    Initialize          ( const vector3&    InitPos,
                                          const radian3&    InitRot,
                                          const vector3&    InheritedVelocity,
                                                f32         Speed,
                                                guid        OwnerGuid,
                                                pain_handle PainID,
                                                xbool       bHitLiving = TRUE );

//=========================================================================
	virtual s32		GetMaterial         ( void) const { return 0; }
	virtual	bbox	GetLocalBBox		( void ) const;
    virtual void    OnAdvanceLogic      ( f32 DeltaTime );
	virtual	xbool	OnProcessCollision  ( const f32& DeltaTime );
    virtual void    UpdatePhysics       ( const f32& DeltaTime );
	virtual	void	OnMove				( const vector3& rNewPos );
    virtual void    OnRender            ( void );
    virtual void    OnPain              ( const pain& Pain );
    virtual void    OnColCheck          ( void );

    virtual void    OnExplode           ( void );
    virtual void    SetTarget           ( guid target )             { m_SeekerTarget = target; }
            xbool	LoadInstance		( const char* pFileName );

public:
    static f32                          s_SeekerGrenade_Alert_Time ;

protected:
            void    UpdateVelocity      ( f32 DeltaTime );
    guid                                m_SeekerTarget;
    f32                                 m_YawRate;

    f32                                 m_AliveTime;
    f32                                 m_GravityAcceleration;  // Acceleration due to gravity.
    f32                                 m_TimeSinceLastBroadcast;
    radian3                             m_Spin;
    radian3                             m_TotalSpin;
    matrix4                             m_RenderL2W;
    rigid_inst		                    m_RigidInst;		// Instance for rendering object.

    vector3                             m_NormalCollision;
    vector3                             m_CollisionPoint;
    
	f32			                        m_MaxAliveTime;			// How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;
    guid                                m_EffectTrail;          // Guid of the effect trail
    
    xbool                               m_bBroadcastAlert;
};

//=========================================================================
// END
//=========================================================================
#endif