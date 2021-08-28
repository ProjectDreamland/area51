//=========================================================================
// BASE PROJECTILE
//=========================================================================

#ifndef BASE_PROJECTILE_HPP
#define BASE_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Object.hpp"
#include "Decals\DecalPackage.hpp"
#include "Render\RigidInst.hpp"
#include "PainMgr\Pain.hpp"

struct pain_tweaks
{
    xbool m_bUseTweaks;
    f32 m_DEagleDamage;
    f32 m_ShotgunDamage;
    f32 m_ShotgunDistIncrement;
    f32 m_ShotgunLossPerIncrement;
    f32 m_ShotgunMinDamage;
    f32 m_SniperDamage;
    f32 m_SMPDamage;
    f32 m_MesonDamage;
    f32 m_MutantParasiteDamage;
    f32 m_MeleeDamage;
    f32 m_GrenadeDamage;
    f32 m_GravGrenadeDamage;
};

extern pain_tweaks g_MultiplayerPainTweaks;

xbool IsGameMultiplayer( void );

//=========================================================================
// BASE PROJECTILE
//=========================================================================

class base_projectile : public object
{
public:
    CREATE_RTTI( base_projectile , object , object )

                    base_projectile     ( void );
    virtual         ~base_projectile    ( void );

//=========================================================================
//
// GetLocalBBox         -   returns the local BBox
// LoadInstance         -   Loads the rigid inst of this object
// OnRender             -   Called during the render loop
// OnProcessCollision   -   Collision handling for the projectiles.
//
// SetTarget            -   Unused by most projectiles, but homing projectiles
//                          can be informed of their target using this.
//=========================================================================

    virtual void    OnEnumProp          ( prop_enum& List );
    virtual xbool   OnProperty          ( prop_query& PropQuery );

    virtual bbox    GetLocalBBox        ( void ) const;
            
            

    virtual void    OnRender            ( void );
    virtual xbool   OnProcessCollision  ( const f32& DeltaTime );

    virtual void    Initialize          ( const vector3&    InitPos,
                                          const radian3&    InitRot,
                                          const vector3&    InheritedVelocity,
                                                f32         Speed,
                                                guid        OwnerGuid,
                                                pain_handle PainID,
                                                xbool       bHitLiving = TRUE,
                                                s32         iFirePoint = 0 );

    virtual void    StartFlyby          ( void );
    inline const vector3& GetInitialPos ( void ) { return m_InitialPosition; }
    inline const vector3& GetCurrentPos ( void ) { return m_CurrentPosition; }

    inline  guid    GetOwnerID          ( void ) ;

    inline const vector3& GetVelocity   ( void ) { return m_Velocity ; }
    inline  void    SetThroughGlass     ( xbool bThroughGlass );
    inline  void    SetThroughActors    ( xbool bThroughActors );
            void    SetPainDegradation  ( f32 PainDropDist, f32 PainDropScale );
    const   char*   GetNearFlyByDescriptor      ( void );
    const   char*   GetMediumFlyByDescriptor    ( void );
    const   char*   GetFarFlyByDescriptor       ( void );
    
    virtual void    SetTarget           ( guid TargetGuid );

protected:
    
            xbool    OnProcessCollisionRigid        ( const f32& DeltaTime );
            xbool    OnProcessCollisionPermable     ( const f32& DeltaTime  );
    virtual void     OnBulletHit                    ( collision_mgr::collision& rColl, const vector3& HitPos );
            void     CreateDecal                    ( collision_mgr::collision& rColl );
            f32      CalcDamageDegradation          ( vector3& DamagePos );

            
    rhandle<decal_package>  m_hDecalPackage;    // decals for this bullet
    rhandle<char>           m_hAudioPackage;    // Audio package for the weapon.
    f32                     m_Speed;            // Scalar speed value in cm. / sec.
    vector3                 m_Velocity;         // Direction of projectile's motion
    vector3                 m_CurrentPosition;  // Current position of the object.
    vector3                 m_InitialPosition;  // Initial position of the object.
    guid                    m_OwnerGuid;        // Who fired the projectile
    pain_handle             m_PainHandle;       // What pain event type does this projectile have
    f32                     m_TracerFadeTime;
    f32                     m_PainDropDist;     
    f32                     m_PainDropScale;
    xcolor                  m_TracerColor;
    s32                     m_iFirePoint;       // Bone point from gun (default, left, or right)
    u32                     m_bThroughGlass:1,
                            m_bThroughActors:1,
                            m_bHitLiving:1,
                            m_bSplitScreenInitialPosReady:1;

    // Sound information
    s32                     m_BulletSoundID;
    s32                     m_FlyBySoundID;
    s32                     m_BulletNearFlyBySfxID;     // What sound to play when the bullet flies by.
    s32                     m_BulletMedFlyBySfxID;      // What sound to play when the bullet flies by.
    s32                     m_BulletFarFlyBySfxID;      // What sound to play when the bullet flies by.

    xbool                   m_bIsLargeProjectile; 
};

//===================================================================================================

inline
guid base_projectile::GetOwnerID( void )
{
    return m_OwnerGuid ;
}

//=========================================================================

inline
void    base_projectile::SetThroughGlass( xbool bThroughGlass )
{
    m_bThroughGlass = bThroughGlass;
}

//=========================================================================

inline
void    base_projectile::SetThroughActors( xbool bThroughActors )
{
    m_bThroughActors = bThroughActors;
}

//=========================================================================
#endif // #ifndef BASE_PROJECTILE_HPP
//=========================================================================
