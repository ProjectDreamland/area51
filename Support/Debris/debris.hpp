//=============================================================================
//  DEBRIS
//=============================================================================

#ifndef DEBRIS_HPP
#define DEBRIS_HPP

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "..\objects\Render\RigidInst.hpp"
#include "..\PainMgr\Pain.hpp"
#include "Dictionary\Global_Dictionary.hpp"

//=============================================================================
class debris : public object
{
public:
    CREATE_RTTI( debris, object, object )


//=============================================================================
						debris              ( void );
	virtual				~debris             ( void );

    virtual void        OnInit              ( void );

    virtual void        Create              ( const char*            rigidFileName,
                                              const vector3&         startingPosition,
                                              const vector3&         startingVelocity,
                                              f32                    lifeSpan,
                                              xbool                  bBounces = FALSE );

    virtual void        Create              ( rigid_inst&            Inst,
                                              const vector3&         startingPosition,
                                              const vector3&         startingVelocity,
                                              f32                    lifeSpan,
                                              xbool                  bBounces = FALSE );

    virtual bbox        GetLocalBBox        ( void ) const;
    virtual s32         GetMaterial         ( void ) const { return MAT_TYPE_CONCRETE;}

    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        UpdatePhysics       ( f32 DeltaTime );
    virtual void        OnBounce            ( void );     

    virtual void        OnMove				( const vector3& rNewPos );
    virtual void        OnRender            ( void );

            vector3     OnProcessCollision  ( f32 DeltaTime );
    
            void        IgnoreLiving        ( void );

            void        SetSpin             ( const radian3& NewSpin );
            void        SetInitialRotation  ( const radian3& NewInitialRotation );
            void        SetBounceSoundID      ( s32    BounceSoundID );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=============================================================================
protected:
    xbool                   m_KillMe;
    vector3                 m_Velocity;
    radian3                 m_Spin;
    radian3                 m_TotalSpin;
    f32                     m_Elasticity;
    f32                     m_LifeSpan;
    xbool                   m_IgnoreLiving;
    xbool                   m_Inactive;
    s32                     m_BounceSoundID;
    s32                     m_BounceCount;
    xbool                   m_IgnoreCollision;
};

//=============================================================================

inline void debris::IgnoreLiving( void )
{
    m_IgnoreLiving = TRUE;
}

//=============================================================================

inline void debris::SetSpin( const radian3& NewSpin )
{
    m_Spin = NewSpin;
}

//=============================================================================

inline void debris::SetInitialRotation( const radian3& NewInitialRotation )
{
    m_TotalSpin = NewInitialRotation;
}

//=============================================================================

inline void debris::SetBounceSoundID( s32 BounceSoundID )
{
    m_BounceSoundID = BounceSoundID;
}

//=============================================================================

//=============================================================================
// END
//=============================================================================
#endif//DEBRIS_HPP
