/*
#ifndef __LOCO_PHYSICS_HPP__
#define __LOCO_PHYSICS_HPP__

//=========================================================================
// INLCLUDES
//=========================================================================

#include "Auxiliary\MiscUtils\Property.hpp"
#include "Obj_mgr\obj_mgr.hpp"


#define NUM_STILL_FRAMES 15
//=========================================================================
// CHARACTER PHYSICS
//=========================================================================
class loco_physics : public prop_interface
{
//=========================================================================
public:

                        loco_physics        ( void );
    void                Init                ( guid Guid );
    void                ApplyCollision      ( void );
    void                RenderCollision     ( void );
    void                SetPosition         ( const vector3& Position );
    void                Advance             ( const vector3& MoveTo, f32 DeltaTime );
    vector3             GetPosition         ( void );
    vector3             GetVelocity         ( void );
    void                AddVelocity         ( const vector3& Delta ) ;
    void                ZeroVelocity        ( void ) ;
    void                SetVelocity         ( const vector3& Velocity ) ;
    virtual void        OnEnumProp          ( prop_enum&   List        );
    virtual xbool       OnProperty          ( prop_query&  I           );
    f32                 GetColHeight        ( void ) const { return m_CollisionCurrentHeight; } 
    void                SetColHeight        ( f32 Height ) ;
    f32                 GetColRadius        ( void ) const { return m_CollisionRadius; }
    void                SetColRadius        ( f32 Radius ) ;
    bbox                GetBBox             ( void ) const;
    xbool               SetCrouchParametric ( f32 NormalizePercent );
    void                Jump                ( f32 YVel );
	void				SetGravityAccel		( const f32& GravityAccel ); 
	f32					GetGravityAccel		( void );
	xbool				GetFallMode			( void );
	xbool				GetJumpMode			( void );
	void				SetAirControl		( const f32& AirControl ) { m_AirControl = AirControl; }
    f32                 GetActorCollisionRadius( void )                 { return m_ActorCollisionRadius; }
    f32                 GetActorCollisionHeight( void )                 { return m_CollisionHeight; }
	xbool				GetNavCollided		( void ) { return m_bNavCollided; }
    guid                GetGuid             ( void ) { return m_Guid ; }

    void                SetDeltaPos         ( vector3 &DeltaPos );
    vector3             GetRecentDeltaPos   ( void ) ;
    void                ResetDeltaPos       ( void ) ;
    
    void                SetLocoGravityOn   ( xbool bValue ) ;
    void                SetLocoCollisionOn ( xbool bValue ) ;

    void                SolveActorCollisions( void );
    void                SetSolveActorCollisions( xbool doSolve )    { m_SolveActorCollisions = doSolve; }    
    
//=========================================================================
protected:

    void    HandleMove( vector3&        NewPos, 
                        const vector3&  Velocity, 
                        f32             SlideFriction,
                        f32             SteepestSlide );

    xbool   FindGround          ( vector3& Ground, const vector3& FromHere, f32 DeltaTime );
    void    UpdatePhysics       ( f32 DeltaTime );
    void    CollectPermeable    ( object* pObject, vector3& NewPos );

//=========================================================================
protected:

    guid        m_Guid;
    f32         m_CollisionHeight;
    f32         m_CollisionRadius;
    f32         m_CollisionCrouchOffset;
    f32         m_CollisionCurrentHeight;
    f32         m_AirControl;
    s32         m_MaxCollisions;
    xbool       m_bHandlePermeable;
    xbool       m_bNavCollided;
    xbool       m_bFallMode;
    xbool       m_bJumpMode;
    xbool       m_bUseGravity;
    xbool       m_bLocoGravityOn;
    xbool       m_bLocoCollisionOn;
    f32         m_CollisionSnuggleDistance;
    vector3     m_Velocity;
    vector3     m_Position;
    f32         m_GroundTolerance;
    f32         m_FallTolerance;
    f32         m_VelocityForFallMode;
    f32         m_GravityAcceleration;
    f32         m_MaxDistanceToGround;
    f32         m_SteepestSlide;
    f32         m_CrouchPercent;

    vector3     m_LstDeltaPos[NUM_STILL_FRAMES];
    s32         m_DeltaPosIndex;
    
    xbool       m_SolveActorCollisions;
    f32         m_ActorCollisionRadius;
};

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================

inline vector3 loco_physics::GetPosition( void )
{
    return m_Position;
}

//=========================================================================

inline vector3 loco_physics::GetVelocity( void )
{
    return m_Velocity;
}

//=========================================================================

inline void loco_physics::AddVelocity( const vector3& Delta )
{
    m_Velocity += Delta ;
    if (m_Velocity.Y > 0)
        m_bJumpMode = TRUE;
}

//=========================================================================

inline void loco_physics::ZeroVelocity( void )
{
    m_Velocity.Zero() ;
}

//=========================================================================

inline void loco_physics::SetVelocity( const vector3& Velocity )
{
    m_Velocity = Velocity ;
}

//=========================================================================

inline void loco_physics::SetColHeight( f32 Height )
{
    m_CollisionHeight       = Height ;
    m_CollisionCurrentHeight = Height ;
}

//=========================================================================

inline void loco_physics::SetColRadius( f32 Radius )
{
    m_CollisionRadius = Radius ;
}

//=========================================================================

inline bbox loco_physics::GetBBox( void ) const
{
    bbox BBox;
    BBox.Max.Z = BBox.Max.X = GetColRadius();
    BBox.Min.Z = BBox.Min.X = -BBox.Max.X;
    BBox.Max.Y = GetColHeight();
    BBox.Min.Y = 0;
    return BBox;
}

//=========================================================================

inline void loco_physics::SetGravityAccel( const f32& fGravityAccel )
{
	m_GravityAcceleration = fGravityAccel;
}

//=========================================================================

inline f32	loco_physics::GetGravityAccel( void )
{
	return m_GravityAcceleration;
}

//=========================================================================
inline xbool loco_physics::GetFallMode( void )
{
	return m_bFallMode;
}

//=========================================================================
inline xbool loco_physics::GetJumpMode( void )
{
	return m_bJumpMode;
}

//=========================================================================
// END
//=========================================================================
#endif
*/