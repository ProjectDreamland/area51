//=========================================================================
// BULLET PROJECTILE
//=========================================================================
#ifndef _BULLETTPROJECTILE_HPP
#define _BULLETTPROJECTILE_HPP

//=========================================================================
//INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"

//=========================================================================

class bullet_projectile : public base_projectile
{
public:
	CREATE_RTTI( bullet_projectile , base_projectile , object )

	bullet_projectile();
	virtual ~bullet_projectile();

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
//
//=========================================================================

    virtual s32		GetMaterial         ( void ) const { return 0; }
	virtual	bbox	GetLocalBBox		( void ) const;
	
    virtual void    OnAdvanceLogic      ( f32 DeltaTime );      
	virtual	void	OnMove				( const vector3& rNewPos );

    virtual void    StartFlyby          ( void );


protected:

	f32			        m_MaxAliveTime;			// How long does this object remain in the world.
    
};

#endif
