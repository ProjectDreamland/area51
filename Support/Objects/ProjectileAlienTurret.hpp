//=========================================================================
// GRAVITON CHARGE PROJECTILE
//=========================================================================

#ifndef _ALIEN_TURRET_PROJECTILE_HPP
#define _ALIEN_TURRET_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"
#include "objects\ParticleEmiter.hpp"

//=========================================================================

class alien_turret_projectile : public base_projectile
{
public:
	CREATE_RTTI( alien_turret_projectile , base_projectile , object )

	alien_turret_projectile();
	virtual ~alien_turret_projectile();

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

    virtual void    ReportProjectileCreation( void );
    virtual	void	OnEnumProp		    ( prop_enum& List );
	virtual	xbool	OnProperty		    ( prop_query& PropQuery );

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
    virtual void    SetTarget           ( guid target )             { m_Target = target; }
            xbool	LoadInstance		( const char* pFileName );

            void 	AvoidObstacles      ( xbool bAvoid );

public:
    static f32                          s_SeekerGrenade_Alert_Time ;

protected:
            void    UpdateVelocity      ( f32 DeltaTime );
    guid                                m_Target;
    f32                                 m_YawRate;
    radian                              m_MaxYawRate;
    radian                              m_YawIncreaseRate;      // How fast yaw rate is increased until
                                                                // max yaw rate is achieved
    radian                              m_LaunchConeSize;       // How large is the launching cone?     
                                                                // This controls the amount of random
                                                                // Pitch/yaw the projectiles velocity 
                                                                // gains when it is initially fired.
                                                                

    f32                                 m_AliveTime;
    f32                                 m_GravityAcceleration;  // Acceleration due to gravity.
    f32                                 m_TimeSinceLastBroadcast;
    
    vector3                             m_NormalCollision;
    vector3                             m_CollisionPoint;
    
	f32			                        m_MaxAliveTime;			// How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;
    guid                                m_Effect;               // Guid of the projectile effect
    guid                                m_EffectTrail;          // Guid of the effect trail
    
    xbool                               m_bBroadcastAlert;

    u8                                  m_bFirstUpdate:1,
                                        m_bAvoidObstacles:1;

    vector3 m_TargetPos;    
    xbool m_bDeflecting;
    vector3 m_LastValidTargetPos;
};

//=========================================================================
// END
//=========================================================================
#endif