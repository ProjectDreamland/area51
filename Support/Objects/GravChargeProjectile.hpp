//=========================================================================
// GRAVITON CHARGE PROJECTILE
//=========================================================================

#ifndef _GRAVCHARGE_PROJECTILE_HPP
#define _GRAVCHARGE_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "NetProjectile.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Decals\DecalPackage.hpp"
//=========================================================================

class grav_charge_projectile : public net_proj
{
public:
    CREATE_RTTI( grav_charge_projectile , net_proj , object )

    grav_charge_projectile();
    virtual ~grav_charge_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
//
// GetMaterial      -   Legacy.  Needed for now.
//
//=========================================================================
   
    void            Setup          (guid           OriginGuid,
                                    s32            OriginNetSlot,
                                    const vector3& Position,
                                    const radian3& Orientation,
                                    const vector3& Velocity,
                                    s32            Zone1,
                                    s32            Zone2,
                                    f32            Gravity,
                                    pain_handle    PainHandle );

    virtual void    SetStart        ( const vector3& Position,
                                      const radian3& Orientation,
                                      const vector3& Velocity,
                                      s32      Zone1,
                                      s32      Zone2,
                                      f32      Gravity = 0.0f );

    xbool           LoadEffect          ( const char* pFileName, const vector3& InitPos, const radian3& InitRot );                                
    void            DestroyParticles    ( void );
    void            UpdateParticles     ( const vector3& Position );

    //=========================================================================

    virtual s32     GetMaterial         ( void) const { return 0; }
    virtual bbox    GetLocalBBox        ( void ) const;
    virtual void    OnMove              ( const vector3& rNewPos );
    virtual void    OnRender            ( void );
    virtual void    OnAdvanceLogic      ( f32 DeltaTime );
    virtual void    OnImpact            ( collision_mgr::collision& Coll, object* pTarget );
    virtual void    OnExplode           ( void );
    xbool   LoadInstance        ( const char* pFileName );

#ifndef X_EDITOR
    virtual         void            net_Deactivate      ( void );
#endif

public:
    static f32                          s_GravGrenade_Alert_Time ;

protected:

    f32                                 m_AliveTime;
    radian3                             m_Spin;
    radian3                             m_TotalSpin;

    vector3                             m_NormalCollision;
    vector3                             m_CollisionPoint;
    
    f32                                 m_MaxAliveTime;         // How long does this object remain in the world.
    particle_emitter::particle_type     m_ParticleExplosion;
    f32                                 m_ExplosionRadius;
    guid                                m_EffectTrail;          // Guid of the effect trail
    xbool                               m_bDestroyed;
    
    xbool                               m_bBroadcastAlert;

    rigid_inst                          m_RigidInst;        // Instance for rendering object.
    rhandle<char>                       m_ProjectileFx;     // The energy projectile particle effect.
    guid                                m_FxGuid;           // Guid of the effect object.
    rhandle<decal_package>              m_hDecalPackage;

    voice_id                            m_FlyVoiceID;

};

//=========================================================================
// END
//=========================================================================
#endif