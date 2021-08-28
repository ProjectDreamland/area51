#ifndef CHARACTER_PHYSICS_HPP
#define CHARACTER_PHYSICS_HPP

//=========================================================================
// INLCLUDES
//=========================================================================

#include "Auxiliary\MiscUtils\Property.hpp"
#include "Obj_mgr\obj_mgr.hpp"

#define NUM_STILL_FRAMES 15
//=========================================================================
// CHARACTER PHYSICS
//=========================================================================
class character_physics : public prop_interface
{
//=========================================================================
public:

                        character_physics   ( void );
    void                Init                ( guid Guid );
    void                ApplyCollision      ( void );
#ifndef X_RETAIL
    void                RenderCollision     ( void );
#endif
    void                SetPosition         ( const vector3& Position ) { m_Position = Position; }
    void                Advance             ( const vector3& MoveTo, f32 DeltaTime, xbool bIsDead = FALSE );
    void                AdvanceWithoutCollision( const vector3& MoveTo, f32 DeltaTime, xbool bIsDead = FALSE );
    vector3             GetPosition         ( void ) const ;
    vector3             GetVelocity         ( void ) const ;
    void                AddVelocity         ( const vector3& Delta ) ;
    void                SetVelocity         ( const vector3& Velocity ) ;
    void                ZeroVelocity        ( void ) ;
    virtual void        OnEnumProp          ( prop_enum&   List        );
    virtual xbool       OnProperty          ( prop_query&  I           );
    f32                 GetColHeight        ( void ) const { return m_NavCollisionCurentHeight; } 
    void                SetColHeight        ( f32 Height ) ;
    f32                 GetColRadius        ( void ) const { return m_NavCollisionRadius; }
    void                SetColRadius        ( f32 Radius ) ;
    void                SetColCrouchOffset  ( f32 Offset ) { m_NavCollisionCrouchOffset = Offset; }
    void                SetMaxCollsions     ( s32 Max ) { m_MaxCollisions = Max; }
    bbox                GetBBox             ( void ) const;
    xbool               SetCrouchParametric ( f32 NormalizePercent );
    void                Jump                ( f32 YVel );
    void                Fling               ( const vector3& Velocity, 
                                                    f32      DeltaTime, 
                                                    f32      AirControl, 
                                                    xbool    FlingOnly, 
                                                    xbool    ReflingOnly, 
                                                    xbool    Instantaneous, 
                                                    guid     LastFling );
    xbool               Flung               ( void ) { return m_bFlingMode; }
    void                SetUseGravity       ( xbool bEnable )   { m_bUseGravity = bEnable ; }
	void				SetGravityAccel		( const f32& GravityAccel ); 
	f32					GetGravityAccel		( void );
	xbool				GetFallMode			( void );
	xbool				GetJumpMode			( void );
	void				SetAirControl		( const f32& AirControl ) { m_AirControl = AirControl; }
    void                SetActorCollisionRadius( f32 newRadius)  { m_ActorCollisionRadius = newRadius; }
    f32                 GetActorCollisionRadius( void )  { return m_ActorCollisionRadius; }
	xbool				GetNavCollided		( void ) const { return m_bNavCollided ; }
    guid                GetGuid             ( void ) { return m_Guid ; }
    
    void                SetDeltaPos         ( vector3 &DeltaPos );
    vector3             GetRecentDeltaPos   ( void ) ;
    void                ResetDeltaPos       ( void ) ;

    void                SetLocoGravityOn   ( xbool bValue ) { m_bLocoGravityOn = bValue; }
    void                SetLocoCollisionOn ( xbool bValue ) { m_bLocoCollisionOn = bValue; }
    void                SetHandlePermeable ( xbool bValue ) { m_bHandlePermeable = bValue; }

    void                SetSolveActorCollisions( xbool doSolve )    { m_SolveActorCollisions = doSolve; }    

    void                CopyValues          ( character_physics& rPhysics );
    
    void                SolveActorAndPlatformCollisions( void );
    void                CatchUpWithRidingPlatform   ( f32 DeltaTime );
    guid                GetMovingPlatformGuid       ( void )            const { return m_MovingPlatformGuid; }
    void                WatchForRidingPlatform( void );
    void                ResetRidingPlatforms( void );
    void                Push( const vector3& PushVector );
    void                SetCollisionIgnoreGuid( guid ignoreGuid );

    xbool               IsAirborn ( void ) const;
    xbool               SetupPlayerCollisionCheck( const vector3& Start, const vector3&End );

    void                SetGroundTracking       ( xbool Track );
    void                InitialGroundCheck      ( const vector3& Position );

//=========================================================================
protected:

    void    HandleMove( vector3&        NewPos, 
                        vector3&         OriginalVelocity,
                        f32             DeltaTime,
                        f32             SlideFriction,
                        f32             SteepestSlide );

    xbool   UpdateGround        ( f32 DistBelow );
    void    UpdatePhysics       ( f32 DeltaTime );
    void    CollectPermeable    ( object* pObject, vector3& NewPos );
    void    CollectPermeable    ( object* pObject, const vector3& StartPos, const vector3& EndPos );
    void    SolveActorCollisions( void );
    void    SolveActorCollisions( const bbox& ActorBBox );
    void    SolvePlatformCollisions ( void );

    void    ResolvePenetrations ( void );

//=========================================================================
protected:

    guid        m_Guid;
    f32         m_NavCollisionHeight;
    f32         m_NavCollisionRadius;
    f32         m_NavCollisionCrouchOffset;
    f32         m_NavCollisionCurentHeight;
    f32         m_AirControl;
    f32         m_FlingAC;      // AirControl while in fling mode.
    s32         m_MaxCollisions;
    xbool       m_bHandlePermeable;
    xbool       m_bNavCollided;
    xbool       m_bFallMode;
    xbool       m_bJumpMode;
    xbool       m_bFlingMode;
    xbool       m_bTrackingGround;
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
    guid        m_IgnoreGuid;

    guid        m_MovingPlatformGuid;
    s32         m_MovingPlatformBone;
    matrix4     m_OldMovingPlatformL2W;
    vector3     m_OldMovingPlatformVelocity;
    guid        m_PlatformCollisionIgnoreList[8];
    s32         m_nPlatformsToIgnore;
    vector3     m_LastMove;
    guid        m_LastFling;

    vector3     m_LstDeltaPos[NUM_STILL_FRAMES];
    s32         m_DeltaPosIndex;
    
    xbool       m_SolveActorCollisions;
    f32         m_ActorCollisionRadius;

    guid        m_GroundGuid;
    plane       m_GroundPlane;
    vector3     m_GroundPos;
    s32         m_GroundBoneIndex;
    guid        m_LastSteepSurface;
    xbool       m_bIsAirborn;

    #define CHARACTER_PHYSICS_MAX_TRIGGERS 4
    guid        m_TriggerGuid[CHARACTER_PHYSICS_MAX_TRIGGERS];
    s32         m_nTriggerGuids;
};

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================

inline vector3 character_physics::GetPosition( void ) const
{
    return m_Position;
}

//=========================================================================

inline vector3 character_physics::GetVelocity( void ) const
{
    return m_Velocity;
}

//=========================================================================

inline void character_physics::AddVelocity( const vector3& Delta )
{
    m_Velocity += Delta ;
    if (m_Velocity.GetY() > 0)
        m_bJumpMode = TRUE;
}

//=========================================================================

inline void character_physics::SetVelocity( const vector3& Velocity )
{
    m_Velocity = Velocity ;
}

//=========================================================================

inline void character_physics::ZeroVelocity( void )
{
    m_Velocity.Zero() ;
}

//=========================================================================

inline void character_physics::SetColHeight( f32 Height )
{
    m_NavCollisionHeight       = Height ;
    m_NavCollisionCurentHeight = Height ;
}

//=========================================================================

inline void character_physics::SetColRadius( f32 Radius )
{
    m_NavCollisionRadius = Radius ;
}

//=========================================================================

inline bbox character_physics::GetBBox( void ) const
{
    bbox BBox;
    BBox.Max.GetZ() = BBox.Max.GetX() = GetColRadius();
    BBox.Min.GetZ() = BBox.Min.GetX() = -BBox.Max.GetX();
    BBox.Max.GetY() = GetColHeight();
    BBox.Min.GetY() = 0;
    return BBox;
}

//=========================================================================

inline void character_physics::SetGravityAccel( const f32& fGravityAccel )
{
	m_GravityAcceleration = fGravityAccel;
}

//=========================================================================

inline f32	character_physics::GetGravityAccel( void )
{
	return m_GravityAcceleration;
}

//=========================================================================
inline xbool character_physics::GetFallMode( void )
{
	return m_bFallMode;
}

//=========================================================================
inline xbool character_physics::GetJumpMode( void )
{
	return m_bJumpMode;
}

//=========================================================================

inline
xbool character_physics::IsAirborn ( void ) const
{
    return ( m_bFallMode || m_bJumpMode );
}

//=========================================================================
// END
//=========================================================================
#endif
