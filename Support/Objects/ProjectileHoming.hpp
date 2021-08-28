//=========================================================================
// GRAVITON CHARGE PROJECTILE
//=========================================================================

#ifndef HOMING_PROJECTILE_HPP
#define HOMING_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"

//=========================================================================

class homing_projectile : public base_projectile
{
public:
	CREATE_RTTI( homing_projectile , base_projectile , object )

	homing_projectile();
	virtual ~homing_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void    Initialize          ( const vector3&    InitPos,
                                          const radian3&    InitRot,
                                          const vector3&    InheritedVelocity,
                                                f32         Speed,
                                                guid        OwnerGuid,
                                                pain_handle PainID,
                                                xbool       bHitLiving = TRUE );

//=========================================================================
//
// GetMaterial		-   Legacy.  Needed for now.
// OnAdvanceLogic	-   Updates every frame
// OnMove			-   Handles the motion of the spike
// Initialize		-	Sets the initial position of the object.  Needs an initial
//						velocity, an initial position, and either a radian3 or matrix4 for
//						rotation information.
//
//=========================================================================

    virtual	void	OnEnumProp		    ( prop_enum& List );
	virtual	xbool	OnProperty		    ( prop_query& PropQuery );

//=========================================================================
	virtual s32		GetMaterial         ( void) const { return 0; }
	virtual	bbox	GetLocalBBox		( void ) const;
    virtual void    OnAdvanceLogic      ( f32 DeltaTime );
    virtual void    UpdatePhysics       ( const f32& DeltaTime );
	virtual	void	OnMove				( const vector3& rNewPos );
    virtual void    OnRender            ( void );

    virtual void    SetTarget           ( guid target )                     { m_Target = target; }
    virtual void    SetTargetPosition   ( const vector3& TargetPosition )   { m_NoTargetPosition = TargetPosition; }

public:
    static f32                          s_SeekerGrenade_Alert_Time ;

protected:
    virtual void    UpdateVelocity      ( f32 DeltaTime );
    virtual vector3 GetAimAtPosition    ( void );
    virtual vector3 GetTargetPoint      ( object* pTarget );
    guid                                m_Target;
    vector3                             m_NoTargetPosition;     // Where we go if we have no target
    f32                                 m_YawRate;
    radian                              m_MaxYawRate;
    radian                              m_YawIncreaseRate;      // How fast yaw rate is increased until
                                                                // max yaw rate is achieved

    radian                              m_LaunchConeSize;       // How large is the launching cone?     
                                                                // This controls the amount of random
                                                                // Pitch/yaw the projectiles velocity 
                                                                // gains when it is initially fired.

    f32                                 m_AccelerationPercent;  // 0-1, this is a pct of max speed/second
    f32                                 m_SpeedPercent;         // 0-1, this is what percent of max speed we're going now
    f32                                 m_AliveTime;
    f32                                 m_TimeSinceLastBroadcast;
	f32			                        m_MaxAliveTime;			// How long does this object remain in the world.
    vector3                             m_InitialVelocity;
    vector3                             m_InitialPosition;
    f32                                 m_Traction;             // How far along we predict pursuit (0..1)
    vector3                             m_DebugGoToPosition;    // Debug parameter to see where we are going

    u8                                  m_bBroadcastAlert       :1,
                                        m_bFirstUpdate          :1,
                                        m_bDestroyAfterLogic    :1;
};

//=========================================================================
// END
//=========================================================================
#endif //HOMING_PROJECTILE_HPP