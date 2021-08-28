//==============================================================================
//
//  CollisionMgr.hpp
//
//==============================================================================

#ifndef COLLISION_MGR_HPP
#define COLLISION_MGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_math.hpp"
#include "x_time.hpp"
#include "miscutils\Guid.hpp"
#include "objects\object.hpp"
#include "CollisionPrimatives.hpp"

//==============================================================================

#define MAX_NUM_SPHERES                  10
#define MAX_COLLISION_MGR_COLLISIONS     32
#define MAX_IGNORED_OBJECTS              32
#define MAX_PERMEABLE_OBJECTS            32

#ifndef X_RETAIL
#define ENABLE_COLLISION_STATS
#endif // X_RETAIL

//==============================================================================
//  TYPES
//==============================================================================

enum primitive 
{
    PRIMITIVE_START = 0,
    PRIMITIVE_INVALID = PRIMITIVE_START,
    
    // moving primitives
    PRIMITIVE_DYNAMIC_START,
    PRIMITIVE_DYNAMIC_CYLINDER = PRIMITIVE_DYNAMIC_START,
    PRIMITIVE_DYNAMIC_SPHERE,
    PRIMITIVE_DYNAMIC_RAY,
    PRIMITIVE_DYNAMIC_LOS,
    PRIMITIVE_DYNAMIC_END = PRIMITIVE_DYNAMIC_LOS,

    // stationary primitives
    PRIMITIVE_STATIC_START,
    PRIMITIVE_STATIC_SPHERE = PRIMITIVE_STATIC_START,
    PRIMITIVE_STATIC_TRIANGLE,
    PRIMITIVE_STATIC_AA_BBOX,
    PRIMITIVE_STATIC_END = PRIMITIVE_STATIC_AA_BBOX
};

//==============================================================================


//==============================================================================

class collision_mgr
{
//------------------------------------------------------------------------------
//  Public constants
public:
    
//------------------------------------------------------------------------------
//  Public Types

public:

    struct collision_context_info
    {
        s32                 Context;
        guid                Guid;
        matrix4             L2W;
        matrix4             W2L;
    };

    //
    // dynamic primitive structs
    //
    struct dynamic_cylinder
    {
        vector3 BotStart;
        vector3 BotEnd;
        vector3 TopStart;
        vector3 TopEnd;
        f32     Radius;
        f32     Height;

        vector3 StartSpherePositions[MAX_NUM_SPHERES];
        vector3 EndSpherePositions[MAX_NUM_SPHERES];
        s32     nStartSpheres;
        s32     nEndSpheres;
    };
    
    struct dynamic_sphere
    {
        vector3 Start;
        vector3 End;
        f32     Radius;
    };

    struct dynamic_ray
    {
        vector3 Start;
        vector3 End;
    };

#ifdef ENABLE_COLLISION_STATS
    // Statistic tracking
    struct stats
    {
        // 0 - Cylinder
        // 1 - Sphere
        // 2 - Ray
        // 3 - LOS
        f32     Time[4];
        s32     Count[4];

        // Functions
        void Reset( void )
        {
            Count[0] = Count[1] = Count[2] = Count[3] = 0;
            Time[0]  = Time[1]  = Time[2]  = Time[3]  = 0.0f;
        }
    } ;
#endif // ENABLE_COLLISION_STATS

    struct collision
    {
        f32                 T;              // collisoin intersection 0->1
        vector3             Point;          // Point of collision
        plane               Plane;          // Plane of collision
        plane               SlipPlane;      // Plane we slip along
        guid                ObjectHitGuid;  // Object that was hit
        s32                 PrimitiveKey;   // ID (can be used to identify
                                            //      sub-objects in object etc)
        primitive           StaticPrimitive;// Static primitive hit
        xbool               HitTriangleEdge;// Point is on the tri edge
        u32                 Flags;          // Flags indicating the properties of the collision object.

        collision(       void ) { Clear(); }

        void    Clear( void )
        {
            T               = 0;
            ObjectHitGuid   = 0;
            PrimitiveKey    = 0;
            StaticPrimitive = PRIMITIVE_INVALID;
            HitTriangleEdge = FALSE;
            Flags           = 0;

            Point.Set      (0,0,0);
            Plane.Setup    (0,1,0,0);
            SlipPlane.Setup(0,1,0,0);            
        }

        xbool   IsValid( void ) const
        {
            return !!(ObjectHitGuid != 0);
        }

        collision(
                  f32           FinalT,
            const vector3&      HitPoint,
            const plane&        HitPlane,
            const plane&        SlipPlane,
                  guid          HitGuid,
                  s32           HitKey,
                  primitive     HitStatic,
                  xbool         HitEdge,
                  f32           HitObjectHeight,
                  u32           aFlags )
            :
            T               (   FinalT      ),
            Point           (   HitPoint    ),
            Plane           (   HitPlane    ),
            SlipPlane       (   SlipPlane   ),
            ObjectHitGuid   (   HitGuid     ),
            PrimitiveKey    (   HitKey      ),
            StaticPrimitive (   HitStatic   ),
            HitTriangleEdge (   HitEdge     ),
            Flags           (   aFlags      )
        {
            (void)HitObjectHeight;
        }

    };


//------------------------------------------------------------------------------
//  Public Functions

public:

    //--------------------------------------------------
    // Initial Setup functions.  Each function sets up
    // the other options as listed below.
    //--------------------------------------------------

    //--------------------------------------------------
    // CylinderSetup
    //--------------------------------------------------
    // MaxCollisions                    = 1
    // UseLowPoly                       = TRUE
    // IgnoreGlass                      = FALSE
    // StopOnFirstCollisionFound        = FALSE
    // CollectPermeables                = FALSE
    // RemoveDuplicateGuids             = TRUE
    // Ignore list is emptied
    //--------------------------------------------------
    void    CylinderSetup       (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd,
                                        f32         Radius,
                                        f32         Height);
                                          
    //--------------------------------------------------
    // SphereSetup
    //--------------------------------------------------
    // MaxCollisions                    = 1
    // UseLowPoly                       = FALSE
    // IgnoreGlass                      = FALSE
    // StopOnFirstCollisionFound        = FALSE
    // CollectPermeables                = FALSE
    // RemoveDuplicateGuids             = TRUE
    // Ignore list is emptied
    //--------------------------------------------------
    void    SphereSetup         (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd,
                                        f32         Radius);

    //--------------------------------------------------
    // RaySetup and EditorSelectRaySetup
    //--------------------------------------------------
    // MaxCollisions                    = 1
    // UseLowPoly                       = FALSE
    // IgnoreGlass                      = FALSE
    // StopOnFirstCollisionFound        = FALSE
    // CollectPermeables                = FALSE
    // RemoveDuplicateGuids             = TRUE
    // Ignore list is emptied
    //--------------------------------------------------
    void    RaySetup            (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd);

    void    EditorSelectRaySetup( const vector3&    WorldStart,
                                  const vector3&    WorldEnd  );

    //--------------------------------------------------
    // LineOfSightSetup
    //--------------------------------------------------
    // MaxCollisions                    = 1
    // UseLowPoly                       = FALSE
    // IgnoreGlass                      = TRUE
    // StopOnFirstCollisionFound        = TRUE
    // CollectPermeables                = FALSE
    // RemoveDuplicateGuids             = TRUE
    // Ignore list is emptied
    //--------------------------------------------------
    void    LineOfSightSetup    (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd);

    //--------------------------------------------------
    // Here are additional options to be applied after one of 
    // the Setup functions is called
    //--------------------------------------------------

    // Set prefered maximum number of collisions to keep.
    // All defaults are 1 so only change this when you
    // intend on looping through the multiple collisions
    void    SetMaxCollisions    ( s32 nMaxCollisions );

    // Add entries into the ignore list.  Remember that the
    // MovingObjGuid passed into the setup function will
    // always be ignored so you don't need to add it into
    // the ignore list.
    void    AddToIgnoreList     ( guid  Guid );
    void    AddToIgnoreList     ( guid* pGuids, s32 nGuids ); 

    // Turn on collection of permeables
    void    CollectPermeables   ( void );

    // Turn on low-poly collision.  Low poly collision or
    // the objects bbox will be used instead of the default
    // high poly collision
    void    UseLowPoly       ( void );

    // Turn on ignore glass.  This will ignore rigid geometry
    // tagged as being glass.  Used by LOS.
    void    IgnoreGlass         ( void );

    // Stop searching after first collision.  This is usefull
    // in cases when you only need A collision, not the closest.
    void    StopOnFirstCollisionFound( void );

    void    DoNotRemoveDuplicateGuids( void );

    //--------------------------------------------------
    // Collision checking
    //--------------------------------------------------

    xbool   CheckCollisions     ( object::type        ThisType            = object::TYPE_ALL_TYPES, 
                                  u32                 TheseAttributes     = object::ATTR_COLLIDABLE, 
                                  u32                 NotTheseAttributes  = object::ATTR_NULL );
                                  
    //--------------------------------------------------
    // Post collision-check queries
    //--------------------------------------------------

    // Permeable list
    s32     GetNPermeables      ( void );
    guid    GetPermeableGuid    ( s32 Index );
    void    NotifyPermeables    ( void );

    //--------------------------------------------------
    // Apply functions
    //--------------------------------------------------
    
    void    StartApply          (       guid      Guid, 
                                  const matrix4&  L2W,
                                  const matrix4&  W2L   );

    void    StartApply          (       guid      Guid  );

    void    EndApply            (       void );


    void    ApplySphere         ( const vector3&    WorldPos,
                                        f32         Radius,
                                        u32         Flags           =  0,
                                        s32         PrimitiveKey    = -1 );
    
    void    ApplyTriangle       ( const vector3&    P0,
                                  const vector3&    P1,
                                  const vector3&    P2,
                                        u32         Flags           =  0,
                                        s32         PrimitiveKey    = -1 );

    void    ApplyQuad           ( const vector3&    P0,
                                  const vector3&    P1,
                                  const vector3&    P2,
                                  const vector3&    P3,
                                        u32         Flags           =  0,
                                        s32         PrimitiveKey    = -1 );

    void    ApplyAABBox         ( const bbox&       BBox,
                                        u32         Flags           =  0,
                                        s32         PrimitiveKey    = -1 );

    void    ApplyOOBBox         ( const bbox&       LocalBBox,
                                  const matrix4&    L2W,
                                        u32         Flags           =  0,
                                        s32         PrimitiveKey    = -1 );

    //--------------------------------------------------

    guid GetMovingObjGuid       ( void ) const;
                                
    void EditorSelectRay        ( const vector3& Start, const vector3& End, xbool bIncludeIcons );
                                
    xbool   IsUsingHighPoly     ( void ) { return !m_bUseLowPoly; }
    xbool   IsIgnoringGlass     ( void ) { return m_bIgnoreGlass; }
    xbool   IsEditorSelectRay   ( void ) { return m_bIsEditorSelectRay; }
    xbool   IsStopOnFirstCollision( void ) { return m_bStopOnFirstCollisionFound; }

    //--------------------------------------------------
    // Generic functions
    //--------------------------------------------------

    // construct/destruct
            collision_mgr       (       void );
           ~collision_mgr       (       void ) { };

    // init
    void    InitializeCollisionCheckDefaults( void );

    // Get moving object bounds
    primitive               GetDynamicPrimitive ( void ) const { return m_DynamicPrimitive; }
    const bbox&             GetDynamicBBox      ( void ) const;
    const dynamic_cylinder& GetDynamicCylinder  ( void ) const;
    const dynamic_ray&      GetDynamicRay       ( void ) const;
    
    static s32 GetCylinderSpherePositions(  const vector3&  Bottom,
                                            const vector3&  Top,
                                                  f32       Radius,
                                                  vector3*  SpherePositions,
                                                  s32       MaxNSpheres );


    void    RecordCollision     ( const collision_mgr::collision&  Collision );

#ifdef ENABLE_COLLISION_STATS
    void    Render( void );
#endif // ENABLE_COLLISION_STATS

//------------------------------------------------------------------------------
// Private Functions
private:
    // static sphere to dynamic cylinder
    void    ApplySphereToCylinder(const vector3&        WorldPos,
                                        f32             Radius,
                                        u32             Flags,
                                        s32             PrimitiveKey    = -1 );
                                                        
    // static sphere to dynamic sphere                  
    void    ApplySphereToSphere  (const vector3&        WorldPos,
                                        f32             Radius,
                                        u32             Flags,
                                        s32             PrimitiveKey    = -1 );

    // static sphere to dynamic ray
    void    ApplySphereToRay     (const vector3&        WorldPos,
                                        f32             Radius,
                                        u32             Flags,
                                        s32             PrimitiveKey    = -1 );

    // static triangle to dynamic cylinder
    void    ApplyTriangleToCylinder(const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    // static triangle to dynamic sphere
    void    ApplyTriangleToSphere  (const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    // static triangle to dynamic ray
    void    ApplyTriangleToRay     (const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    // static aabbox to dynamic cylinder
    void    ApplyAABBoxToCylinder(  const   bbox&       AABBox,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    // static aabbox to dynamic sphere
    void    ApplyAABBoxToSphere  (  const   bbox&       AABBox,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    // static aabbox to dynamic ray
    void    ApplyAABBoxToRay     (  const   bbox&       AABBox,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    void    ApplyTriangleToStretchedSphere(const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            u32         Flags,
                                            s32         PrimitiveKey    = -1 );

    void    ApplySphereToPolyCache      ( void );
    void    ApplyCylinderToPolyCache    ( void );
    void    ApplyRayToPolyCache         ( void );

//------------------------------------------------------------------------------

    void    ClearIgnoreList     ( void );
	xbool   IsInIgnoreList	    ( guid  Guid );

    void    SortCollisions      ( void );
    void    CleanPermeables     ( void );

    void    UsePolyCache        ( void );

//------------------------------------------------------------------------------
// Hidden public functions
public:

    xbool   fn_CheckCollisions     ( object::type        ThisType            = object::TYPE_ALL_TYPES, 
                                     u32                 TheseAttributes     = object::ATTR_COLLIDABLE, 
                                     u32                 NotTheseAttributes  = object::ATTR_NULL );

#ifdef ENABLE_COLLISION_STATS
    void    fn_LOG                 ( const char* pFileName, s32 LineNumber );

    void    fn_REPORT              ( void );
#endif // ENABLE_COLLISION_STATS

//------------------------------------------------------------------------------
// Public Storage
public:
    // collisions
    collision           m_Collisions[MAX_COLLISION_MGR_COLLISIONS];
    s32                 m_nCollisions;
    s32                 m_nMaxCollisions;
#ifdef ENABLE_COLLISION_STATS
    stats               m_Stats ;
    xbool               m_DisplayStats;
#endif // ENABLE_COLLISION_STATS

//------------------------------------------------------------------------------
//  Private Storage
private:

    //
    // moving primative stuff
    //
    primitive           m_DynamicPrimitive;
    bbox                m_DynamicBBoxes[2];
    guid                m_MovingObjGuid;
	
    //
    // Other internals
    //
    xbool                   m_bApplyStarted;
    dynamic_cylinder        m_CylinderInfo[2];
    dynamic_sphere          m_SphereInfo[2];
    dynamic_ray             m_RayInfo[2];
    collision_context_info  m_ContextInfo;

    //
    // Ignore list
    //
    xbool               m_bUseIgnoreList;
    guid				m_IgnoreList[ MAX_IGNORED_OBJECTS ];
	s32					m_nIgnoredObjects;

    //
    // Permeable list
    //
    xbool               m_bCollectPermeable;
    guid				m_Permeable[ MAX_PERMEABLE_OBJECTS ];
    f32                 m_PermeableT[ MAX_PERMEABLE_OBJECTS ];
	s32					m_nPermeables;
    xbool               m_bNotifyingPermeables;

    //
    // Other options
    //
    xbool               m_bUseLowPoly;
    xbool               m_bIgnoreGlass;
    xbool               m_bStopOnFirstCollisionFound;
    xbool               m_bIsRayCheck;
    xbool               m_bIsEditorSelectRay;
    xbool               m_bRemoveDuplicateGuids;
    xbool               m_bUsePolyCache;

#ifdef ENABLE_COLLISION_STATS
    //
    // For logging
    //
    xbool               m_bReportLog;
    const char*         m_pLogFileName;
    s32                 m_LogLineNumber;
    s32                 m_LogNLogicLoops;
    f32                 m_LogTime;
    xbool               m_bDontLogCYL;
    xbool               m_bDontLogSPH;
    xbool               m_bDontLogRAY;
    xbool               m_bDontLogLOS;
#endif // ENABLE_COLLISION_STATS

    //
    // Filters used during CheckCollisions
    //
    object::type        m_FilterThisType;
    u32                 m_FilterTheseAttributes;
    u32                 m_FilterNotTheseAttributes;

};

//==============================================================================

extern collision_mgr g_CollisionMgr;

#include "CollisionMgr_Private.hpp"

//==============================================================================
#endif // COLLISION_MGR_HPP
//==============================================================================

