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

//==============================================================================
// DEFINES
//==============================================================================

#define MAX_NUM_SPHERES                 10
#define MAX_COLLISION_MGR_COLLISIONS    16
#define MAX_COLLISION_MGR_TRIANGLES     128

//==============================================================================
//  TYPES
//==============================================================================




enum primitive
{
    PRIMITIVE_START = 0,
    PRIMITIVE_INVALID = PRIMITIVE_START,
    
    // moving primitives
    PRIMITIVE_DYNAMIC_START,
    PRIMITIVE_DYNAMIC_CYLINDER = PRIMITIVE_DYNAMIC_START,   // type of query
    PRIMITIVE_DYNAMIC_SPHERE,
    PRIMITIVE_DYNAMIC_RAY,
    PRIMITIVE_DYNAMIC_RAY_SELECT,
    PRIMITIVE_DYNAMIC_LOS,                  // line of sight
    PRIMITIVE_DYNAMIC_END = PRIMITIVE_DYNAMIC_LOS,

    // stationary primitives
    PRIMITIVE_STATIC_START,
    PRIMITIVE_STATIC_SPHERE = PRIMITIVE_STATIC_START,   // possible answers
    PRIMITIVE_STATIC_TRIANGLE,
    PRIMITIVE_STATIC_AA_BBOX,
    PRIMITIVE_STATIC_END = PRIMITIVE_STATIC_AA_BBOX
};


enum primitive_type
{
    TRIANGLE,
    AA_BBOX,
    SPHERE,
};

enum query_type
{
    CYLINDER,
    SPHERE,
    LOS,        
    RAY,
    RAY_SELECT,
};


//--------------------------------------------------
struct collision_volume
{    
    vector3     P[3];           // tri
    bbox        AABBox;         // BBox
    s32         SearchSeq;      // Sequence number
};

//--------------------------------------------------
struct static_volume_set
{
    bbox                m_BBox;
    f32                 m_CellSize;
    s32                 m_nCells[3];
    s32                 m_nTotalCells;

    s32                 m_nVolumes;
    collision_volume*   m_pVolume;

    xarray<s32>         m_CellVolumeCount;
    xarray<s32>         m_CellVolumeOffset;
    xarray<s32>         m_VolumeIndex;
};

//--------------------------------------------------
struct collision
{
    f32                 T;                  // collisoin intersection 0->1
    vector3             Point;              // Point of collision
//    plane               Plane;             // Plane of collision
    plane               SlipPlane;          // Plane we slip along
    guid                ObjectHitGuid;      // Object that was hit
    s32                 PrimitiveKey;       // ID (can be used to identify
                                            //      sub-objects in object etc)
    primitive_type      PrimitiveType;      // Static primitive hit
    xbool               HitTriangleEdge;    // Point is on the tri edge

    collision(       void ) {}
    collision(       f32                FinalT,
               const vector3&           HitPoint,
               // const plane&             HitPlane,
               const plane&             SlipPlane,
                     guid               HitGuid,
                     s32                HitKey,
                     primitive_type     HitStatic,
                     xbool              HitEdge,
                     f32                HitObjectHeight );

};

//--------------------------------------------------
class collision_mgr
{
public:


    struct triangle
    {
        vector3 P[3];
    };

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


//------------------------------------------------------------------------------
//  Public Functions

public:

    //--------------------------------------------------
    // Setup functions
    //--------------------------------------------------
    void    CylinderSetup       (       guid        MovingObjGuid,
                                  const vector3&    WorldBotStart,
                                  const vector3&    WorldBotEnd,
                                  const vector3&    WorldTopStart,
                                  const vector3&    WorldTopEnd,
                                        f32         Radius,
                                        u32         StaticObjAttrs=0xFFFFFFFF);
                                          
    void    SphereSetup         (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd,
                                        f32         Radius,
                                        u32         StaticObjAttrs=0xFFFFFFFF);

    void    RaySetup            (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd,
                                        u32         StaticObjAttrs=0xFFFFFFFF);
                                      
    void    RaySetupSelect      ( const vector3&    WorldStart,
                                  const vector3&    WorldEnd,
                                        u32         StaticObjAttrs=0xFFFFFFFF);
                                      
    void    LineOfSightSetup    (       guid        MovingObjGuid,
                                  const vector3&    WorldStart,
                                  const vector3&    WorldEnd,
                                        u32         StaticObjAttrs=0xFFFFFFFF);

    // SetStaticVolumes will force the exclusive use of a list of primitives
    // for the collision check
    void    SetStaticVolumes    (       collision_volume*   StaticVolumes,
                                        s32                 nStaticVolumes );

    // This method creates a static volume set (*without* setting it in the
    // collision manager)
    void    CreateStaticVolumeSet(      static_volume_set&  StaticVolumeSet,
                                        collision_volume*   StaticVolumes,
                                        s32                 nStaticVolumes );
    
    void    SetStaticVolumeSet  (       static_volume_set*  pStaticVolumeSet );
    void    ConvertTrisToStaticVolumeSet( void );

    // This method does not free the memory of the static_volume_set used
    void    ClearStaticVolumeSet(       void );

    // SetSelSet will force the exclusive use of the objects in the selset
    // for the collision check
    void    SetSelSet           (       xhandle Set );

    void    SetMaxCollisions    (       s32 nMaxCollisions );

    //--------------------------------------------------
    // Collision checking
    //--------------------------------------------------
    xbool   CheckCollisions     (       void );
    void    StartApply          (       guid      Guid, 
                                  const matrix4&  L2W,
                                  const matrix4&  W2L   );
    void    StartApply          (       guid      Guid  );
    void    EndApply            (       void );
    s32     GetFirstCollision   (       void );

    //--------------------------------------------------
    // Triangle gathering
    //--------------------------------------------------
    void    GatherTriangles         ( const bbox& BBox, u32 ObjectAttributes );
    void    GatherObjectTriangles   ( const bbox& BBox, guid ObjectGuid );
    void    GatherTriangle          ( const vector3& P0,
                                      const vector3& P1,
                                      const vector3& P2 );
    void    GatherTrianglesToFile   ( const bbox& BBox );
    void    LoadTriangleFile        ( void );
    const bbox& GetGatherBBox       ( void );

    //--------------------------------------------------
    // Apply functions
    //--------------------------------------------------

    void    ApplySphere         ( const vector3&    WorldPos,
                                        f32         Radius,
                                        guid        ObjectHitGuid   = 0,
                                        s32         PrimitiveKey    = 0 );
    
    void    ApplyTriangle       ( const vector3&    P0,
                                  const vector3&    P1,
                                  const vector3&    P2,
                                        guid        ObjectHitGuid   = 0,
                                        s32         PrimitiveKey    = 0 );

    void    ApplyAABBox         ( const bbox&       BBox,
                                        guid        ObjectHitGuid   = 0,
                                        s32         PrimitiveKey    = 0 );

    //--------------------------------------------------
    // Generic functions
    //--------------------------------------------------

    // construct/destruct
            collision_mgr       (       void );
            ~collision_mgr      (       void ) { };

    // init
    void    Initialize          (       void );

    // Get moving object bounds
    primitive   GetDynamicPrimitive( void ) const { return m_DynamicPrimitive; }
    const bbox& GetDynamicBBox  ( void ) const;
    s32         GetSequence     ( void ) const;

    // ignore list
    void    AddToIgnoreList     (       guid        Guid );  // Add one guid
    void    AddToIgnoreList     (       guid*       Guids,   // Add guid list
                                        s32         nGuids ); 
    void    ClearIgnoreList     ( void );

    static s32 GetCylinderSpherePositions(  const vector3&  Bottom,
                                            const vector3&  Top,
                                                  f32       Radius,
                                                  vector3*  SpherePositions,
                                                  s32       MaxNSpheres );

    xbool CylinderSanityCheck( const vector3&  Bottom,
                               const vector3&  Top,
                               f32       Radius );

    // Get Surface Flags for a specified triangle
    byte    GetSurfaceFlags (const guid&       Guid, 
                                    s32         TriId );

#ifdef EDITOR
    void    EditorDisableCollisionRender ( const guid& Guid );
    void    EditorEnableCollisionRender ( const xarray<guid>& Guids, xbool DisableOthers = TRUE );
    void    EditorEnableCollisionRender ( const guid& Guid, xbool DisableOthers = TRUE );
#endif

//------------------------------------------------------------------------------
// Private Functions
private:
    // static sphere to dynamic cylinder
    void    ApplySphereToCylinder(const vector3&        WorldPos,
                                        f32             Radius,
                                        guid            ObjectHitGuid   = 0,
                                        s32             PrimitiveKey    = 0 );
                                                        
    // static sphere to dynamic sphere                  
    void    ApplySphereToSphere  (const vector3&        WorldPos,
                                        f32             Radius,
                                        guid            ObjectHitGuid   = 0,
                                        s32             PrimitiveKey    = 0 );

    // static sphere to dynamic ray
    void    ApplySphereToRay     (const vector3&        WorldPos,
                                        f32             Radius,
                                        guid            ObjectHitGuid   = 0,
                                        s32             PrimitiveKey    = 0 );

    // static triangle to dynamic cylinder
    void    ApplyTriangleToCylinder(const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            guid        ObjectHitGuid   = 0,
                                            s32         PrimitiveKey    = 0 );

    // static triangle to dynamic sphere
    void    ApplyTriangleToSphere  (const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            guid        ObjectHitGuid   = 0,
                                            s32         PrimitiveKey    = 0 );

    // static triangle to dynamic ray
    void    ApplyTriangleToRay     (const   vector3&    P0,
                                    const   vector3&    P1,
                                    const   vector3&    P2,
                                            guid        ObjectHitGuid   = 0,
                                            s32         PrimitiveKey    = 0 );

    // static aabbox to dynamic cylinder
    void    ApplyAABBoxToCylinder(  const   bbox&       AABBox,
                                            guid        ObjectHitGuid   = 0,
                                            s32         PrimitiveKey    = 0 );

    // static aabbox to dynamic sphere
    void    ApplyAABBoxToSphere  (  const   bbox&       AABBox,
                                            guid        ObjectHitGuid   = 0,
                                            s32         PrimitiveKey    = 0 );

    // static aabbox to dynamic ray
    void    ApplyAABBoxToRay     (  const   bbox&       AABBox,
                                            guid        ObjectHitGuid   = 0,
                                            s32         PrimitiveKey    = 0 );

    void    RecordCollision     ( collision_mgr::collision  Collision );
//------------------------------------------------------------------------------
// Public Storage
public:
    // collisions
    collision           m_Collisions[MAX_COLLISION_MGR_COLLISIONS];
    s32                 m_nCollisions;
    s32                 m_nMaxCollsions;
    xtimer              m_CheckCollisionTime;

    // Storage for static volumes
    static_volume_set   m_StaticVolumeSet;
    
    // triangles
    triangle            m_Triangle[MAX_COLLISION_MGR_TRIANGLES];
    s32                 m_nTriangles;
    bbox                m_GatherBBox;
    
    // Which collision is this?
    s32                 m_Sequence;

//------------------------------------------------------------------------------
//  Private Storage
private:

    // moving primative stuff
    primitive           m_DynamicPrimitive;
    bbox                m_DynamicBBoxes[2];
    guid                m_MovingObjGuid;
    u32                 m_StaticObjAttrs;
    dynamic_cylinder    m_CylinderInfo[2];
    dynamic_sphere      m_SphereInfo[2];
    dynamic_ray         m_RayInfo[2];
    
    //This volume set is for the dynamic case (but it's still a static_volume_set
    //...go figure)
    static_volume_set   *m_pDynamicVolumeSet;

    xbool               m_ApplyStarted;
    collision_context_info  m_ContextInfo;
};

extern collision_mgr g_CollisionMgr;

//==============================================================================
#endif // COLLISION_MGR_HPP
//==============================================================================

