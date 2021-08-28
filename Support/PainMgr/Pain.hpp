//==============================================================================
//
// Pain.hpp
//
//==============================================================================

#ifndef PAIN_HPP
#define PAIN_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_stdio.hpp"
#include "x_math.hpp"
#include "x_array.hpp"
#include "CollisionMgr/CollisionMgr.hpp"
#include "PainMgr/PainMgr.hpp"
#include "MiscUtils\RTTI.hpp"

//==============================================================================
//==============================================================================
//==============================================================================
// PAIN
//==============================================================================
//==============================================================================
//==============================================================================

class pain
{

public:

    pain(void);
    ~pain(void);

    //==========================================================================
    // PAIN INITIALIZATION
    //==========================================================================

    // This routine clears the pain class and initializes it with the basics
    void Setup                  ( pain_handle PainID,   guid OriginGuid, const vector3& Position );
    void Setup                  ( const char* PainDesc, guid OriginGuid, const vector3& Position );

    // Status routines
    xbool SetupCalled                   ( void ) const;
    xbool ComputeDamageAndForceCalled   ( void ) const;
    
    // These routines provide additional information (if available) to later inquiries
    void SetDirection           ( const vector3& Direction ); 
    void SetDirectHitGuid       ( guid DirectHitGuid );
    void SetCustomScalar        ( f32 CustomScalar );
    void SetCollisionInfo       ( const collision_mgr::collision& CollisionInfo );
    void SetAnimEventID         ( s32 AnimEventID );

    // Distribute pain to victims.  Returns TRUE if any object's OnPain() was called.
    xbool ApplyToWorld          ( xbool bIgnoreOrigin=FALSE );
    xbool ApplyToObject         ( guid VictimGuid );
    xbool ApplyToObject         ( object* pVictimObject );

    // Queries available after Setup()
    pain_handle         GetPainHandle       ( void ) const; 
    pain_health_handle  GetPainHealthHandle ( void ) const;
    guid                GetOriginGuid       ( void ) const; 
    const rtti&         GetOriginRTTI       ( void ) const;
    const vector3&      GetPosition         ( void ) const; 
    const vector3&      GetDirection        ( void ) const; 
    f32                 GetCustomScalar     ( void ) const; 
    guid                GetDirectHitGuid    ( void ) const; 
    s32                 GetAnimEventID      ( void ) const; 
    const vector3&      GetImpactPoint      ( void ) const;
    const vector3&      GetImpactNormal     ( void ) const;
    xbool               IsDamageComputed    ( void ) const;

    // The collision info provides a number of useful values
    // - ObjectHit guid
    // - PrimitiveKey for triangle index or character bone index
    // - Flags for triangle material type
    // - collision point and plane.  These area mirrored in ImpactPoint/Normal

    xbool                           HasCollision    ( void ) const; // Defaults to FALSE
    const collision_mgr::collision& GetCollision    ( void ) const; // Asserts if not available

    //==========================================================================
    // PAIN RESOLUTION
    //==========================================================================

    // Returns TRUE if any damage or force should be applied
    xbool ComputeDamageAndForce( const char*   HealthDesc, guid VictimGuid, const vector3& VictimPainPosition ) const;
    xbool ComputeDamageAndForce( health_handle HealthID,   guid VictimGuid, const vector3& VictimPainPosition ) const;

    // Queries available after ComputeDamageAndForce().  
    f32             GetDamage               ( void ) const; // Health to subtract from object
    f32             GetForce                ( void ) const; // Change in speed after pain is applied (cm/sec) 
    const vector3&  GetForceDirection       ( void ) const; // Normalized direction
    vector3         GetForceVelocity        ( void ) const; // ForceDirection * Force
    xbool           IsDirectHit             ( void ) const; 
    guid            GetVictimGuid           ( void ) const; 
    const rtti&     GetVictimRTTI           ( void ) const;
    health_handle   GetHealthHandle         ( void ) const; 
    const vector3&  GetVictimPainPosition   ( void ) const; 
    s32             GetHitType              ( void ) const; // Custom hit-type as listed in HitTypeTable
    xbool           IsFriendlyFire          ( void ) const; // Were the Origin & Victim friends?
    f32             GetLOSCoverage          ( void ) const; // 1.0-FullCovered, 0.0-ClearLOS

private:

    void    Clear               ( void );
    void    SetupDefaults       ( void );
    void    ComputeLOSCoverage  ( void ) const;
    xbool   HandleFriendlyFire  ( object* pKiller, object* pVictim, f32& DamageModifier, f32& ForceModifier ) const;

private:

    // Values set by Setup()
    pain_handle                 m_PainHandle;
    vector3                     m_Position;
    bbox                        m_PainBBox;
    guid                        m_OriginGuid;
    vector3                     m_Direction;
    guid                        m_DirectHitGuid;
    f32                         m_CustomScalar;
    s32                         m_AnimEventID;
    vector3                     m_ImpactPoint;
    vector3                     m_ImpactNormal;
    const rtti*                 m_pOriginRTTI;
    collision_mgr::collision    m_Collision;

    // Values set by ComputeDamageAndForce()
    mutable f32                 m_Force;
    mutable f32                 m_Damage;
    mutable vector3             m_ForceDirection;
    mutable health_handle       m_HealthHandle;
    mutable pain_health_handle  m_PainHealthHandle;
    mutable guid                m_VictimGuid;
    mutable vector3             m_VictimPainPosition;
    mutable s32                 m_HitType;
    mutable const rtti*         m_pVictimRTTI;
    mutable f32                 m_LOSCoverage;

    // General flags
    mutable u32                 m_bSetupCalled:1,
                                m_bComputeDamageAndForceCalled:1,
                                m_bCollisionAvailable:1,
                                m_bIsFriendlyFire:1,                                                    
                                m_bDirectHit:1;
};

//==============================================================================
// END 
//==============================================================================
#endif // PAIN_HPP

