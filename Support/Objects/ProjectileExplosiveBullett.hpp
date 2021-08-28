//=========================================================================
// EXPLOSIVE BULLET PROJECTILE
//=========================================================================
#ifndef _EXPLOSIVE_BULLET_PROJECTILE_HPP
#define _EXPLOSIVE_BULLET_PROJECTILE_HPP

//=========================================================================
//INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"

//=========================================================================

class explosive_bullet_projectile : public base_projectile
{
public:
	CREATE_RTTI( explosive_bullet_projectile , base_projectile , object )

	                            explosive_bullet_projectile     ( void );
	virtual                     ~explosive_bullet_projectile    ( void );

    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );

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

    virtual s32		            GetMaterial         ( void ) const { return 0; }
	virtual	bbox	            GetLocalBBox		( void ) const;
	
    virtual void                OnAdvanceLogic      ( f32 DeltaTime );      
	virtual	void	            OnMove				( const vector3& rNewPos );
/*
    virtual void    Initialize          ( const vector3&    InitPos,
                                          const radian3&    InitRot,
                                          const vector3&    InheritedVelocity,
                                                f32         Speed,
                                                guid        OwnerGuid,
                                                pain_handle PainID,
                                                xbool       bHitLiving = TRUE );
*/
    virtual	void	            OnEnumProp		    ( prop_enum& List );
	virtual	xbool	            OnProperty		    ( prop_query& PropQuery );

            void                SetExplosionSize    ( f32 ExplosionSize );
            void                BroadcastPain       ( void );

#ifndef X_RETAIL
            void                OnDebugRender       ( void );
#endif // X_RETAIL

protected:
    
    virtual void                OnBulletHit          ( collision_mgr::collision& rColl, const vector3& HitPos );



	f32			                m_MaxAliveTime;			// How long does this object remain in the world.
    f32                         m_ExplosionSize;        // How big is the explosion going to be.
    rhandle<char>               m_ExplosionFx;          // What type of explosion to play on collision.
    f32                         m_PainRadius;
    f32                         m_PainAmount;
    
};
//-------------------------------------------------------------------------
inline
void explosive_bullet_projectile::SetExplosionSize( f32 ExplosionSize )
{ 
    m_ExplosionSize = ExplosionSize; 
}

#endif