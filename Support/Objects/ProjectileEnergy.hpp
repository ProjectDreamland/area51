//=========================================================================
// ENERGY PROJECTILE
//=========================================================================
#ifndef _ENERGYPROJECTILE_HPP
#define _ENERGYPROJECTILE_HPP

//=========================================================================
//INCLUDES
//=========================================================================

#include "objects\ParticleEmiter.hpp"
#include "NetProjectile.hpp"
#include "Decals/DecalPackage.hpp"

//=============================================================================================
// DEFINES
//=============================================================================================
#define MAX_ENERGY_PROJ_IMPACTS         2   // how many times can we rebound before exploding

//=========================================================================

class energy_projectile : public net_proj
{
public:
	CREATE_RTTI( energy_projectile , net_proj , object )

    	            energy_projectile               ( void );
	virtual         ~energy_projectile              ( void );

    virtual const   object_desc&  GetTypeDesc       ( void ) const;
    static  const   object_desc&  GetObjectType     ( void );

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

//=========================================================================

    void            Setup          ( guid           OriginGuid,
                                     s32            OriginNetSlot,
                                     const vector3& Position,
                                     const radian3& Orientation,
                                     const vector3& Velocity,
                                     s32            Zone1,
                                     s32            Zone2,
                                     f32            Gravity,
                                     pain_handle    PainHandle );

virtual         void            SetStart            ( const vector3& Position,
                                                      const radian3& Orientation,
                                                      const vector3& Velocity,
                                                            s32      Zone1,
                                                            s32      Zone2,
                                                            f32      Gravity = 0.0f );


virtual	        bbox	        GetLocalBBox		( void ) const;
	
                void            DestroyParticles    ( void );
                void            UpdateParticles     ( const vector3& NewPosition );
virtual         void            OnAdvanceLogic      ( f32 DeltaTime );

virtual         void            OnImpact            ( collision_mgr::collision& Coll, object* pTarget );
                void            DoImpactPain        ( guid HitGuid, collision_mgr::collision& Coll );

virtual	        void	        OnMove				( const vector3& NewPosition );
virtual	        void	        OnRender			( void );

                xbool	        LoadInstance		( const char* pFileName );
                xbool           LoadDecalPackage    ( const char* pFileName );
                xbool	        LoadEffect		    ( const char* pFileName, const vector3& InitPos, const radian3& InitRot );

virtual         void            DoImpactEffects     ( collision_mgr::collision& Coll );
virtual         void            OnExplode           ( void );
virtual         void            CausePain           ( guid HitGuid = 0 );

virtual         void            OnAttachToObject    ( collision_mgr::collision& Coll, object* pTarget );

#ifndef X_EDITOR
virtual         void            net_Deactivate      ( void );
#endif

protected:
	f32			                        m_MaxAliveTime;			// How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;

    rigid_inst		                    m_RigidInst;		// Instance for rendering object.
    rhandle<char>                       m_ProjectileFx;     // The energy projectile particle effect.
    rhandle<char>                       m_ExplosionFx;      // The energy projectile particle explosion effect.
    rhandle<char>                       m_WallBounceFx;     // The energy projectile particle wall bounce effect.
    rhandle<decal_package>              m_hDecalPackage;
    guid                                m_FxGuid;           // Guid of the effect object.
    f32                                 m_ExplodeTimer;     // how long 'til we explode?
    guid                                m_ExplosionFxGuid;  // Guid of the wall bounce effect object.
    guid                                m_WallBounceFxGuid; // Guid of the explosion effect object.
};

//=========================================================================
// END
//=========================================================================
#endif