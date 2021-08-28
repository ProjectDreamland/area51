//==============================================================================
//  DecalMgr.hpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class handles pasting decals all around the level (both static and
//  dynamic). This could include blood stains, bullet holes, dirt, mold, etc.
//==============================================================================

#ifndef DECALMGR_HPP
#define DECALMGR_HPP

//==============================================================================
//  Includes
//==============================================================================

#include "DecalDefinition.hpp"
#include "Render/Texture.hpp"
#include "Auxiliary\MiscUtils\Fileio.hpp"

//==============================================================================
// forward references
//==============================================================================

struct rigid_geom;

//==============================================================================
// decal_mgr class
//==============================================================================

class decal_mgr
{
public:
    enum { MAX_VERTS_PER_DECAL = 32 }; // the maximum number of verts a decal may have (if it wraps around complex geometry)

    //==========================================================================
    // This structure contains everything a decal will need to render. At the
    // bottom level we'll use structures that are better suited to the hardware,
    // but this is good for the high-level interface.
    //==========================================================================
    struct decal_vert
    {
        enum { FLAG_DRAW_WIREFRAME = 0x0001,
               FLAG_DECAL_START    = 0x0002,
               FLAG_SKIP_TRIANGLE  = 0x8000 };
        
        vector3 Pos;
        s32     Flags;
        vector2 UV;
    };

    //==========================================================================
    // Construction/destruction, initialization
    //==========================================================================
            decal_mgr   ( void );
            ~decal_mgr  ( void );
    void    Init        ( void );
    void    Kill        ( void );

    //==========================================================================
    // load/unload functions
    //==========================================================================
    void    ClearDynamicQueue   ( void );
    void    ResetDynamicDecals  ( void );
    void    LoadStaticDecals    ( const char* FileName );
    void    UnloadStaticDecals  ( void );

    //==========================================================================
    // Methods for rendering decals
    //==========================================================================

    // This function is for rendering static decals in the editor. In the game,
    // static decals will be pre-compiled and this function won't be necessary.
    void    RenderStaticDecal       ( const decal_definition& Def,
                                      const decal_vert*       pVerts,
                                      s32                     nVerts,
                                      const matrix4&          L2W,
                                      xbool                   Wireframe );

    void    OnRender                ( void );
    void    OnUpdate                ( f32 DeltaTime );

    //==========================================================================
    // Methods for plastering decals all over the world
    //==========================================================================
    
    // This function is meant to be used by the editor and will cast a ray to
    // determine if a decal can be stuck in the world, and if so, it will fill
    // in the transform matrix that will place that decal. The function returns
    // the # of verts in the created decal, or 0 if a decal couldn't be made.
    s32     CreateStaticDecalData   ( const decal_definition& Def,
                                      const vector3&          Start,
                                      const vector3&          End,
                                      const vector2&          Size,
                                      radian                  Roll,
                                      decal_vert              Verts[MAX_VERTS_PER_DECAL],
                                      matrix4&                L2W );

    // This function will cast a ray, and stick a decal where any collisions
    // have occured on playsurfaces. An example of this would be casting a ray
    // from the back of a guy's head after being shot, and making a blood splatter
    // three feet away.
    void    CreateDecalFromRayCast  ( const decal_definition& Def,
                                      const vector3&          Start,
                                      const vector3&          End,
                                      const vector2&          Size,
                                      radian                  Roll );
    void    CreateDecalFromRayCast  ( const decal_definition& Def,
                                      const vector3&          Start,
                                      const vector3&          End );

    // This function will take a point that was pre-computed (probably from a
    // collision), and create a decal there. An example of this would be when
    // a bullet impact occurs, you already know the point of collision on the
    // wall, so you don't need to re-cast that ray.
    void    CreateDecalAtPoint      ( const decal_definition& Def,
                                      const vector3&          Point,
                                      const vector3&          Normal,
                                      const vector2&          Size,
                                      radian                  Roll );
    void    CreateDecalAtPoint      ( const decal_definition& Def,
                                      const vector3&          Point,
                                      const vector3&          Normal );

    //
    // This is a fast path for bullet holes which take advantage of possible
    // shortcuts
    //
    void    CreateBulletHole        ( const decal_definition& Def,
                                      const vector3&          Point,
                                      const plane&            Plane,
                                      const vector3*          pTriPos );

    //==========================================================================
    // Methods for registering decal definitions
    //==========================================================================
    xhandle RegisterDefinition      ( decal_definition& Def );
    void    UnregisterDefinition    ( decal_definition& Def );

    //==========================================================================
    // Methods for the editor to export static decals
    //==========================================================================
    void    BeginStaticDecalExport  ( const char*         FileName );
    void    AddStaticDecalToExport  ( const char*         PackageName,
                                      s32                 iGroup,
                                      s32                 iDecalDef,
                                      s32                 nVerts,
                                      const decal_vert    Verts[MAX_VERTS_PER_DECAL],
                                      const matrix4&      L2W,
                                      u16                 ZoneInfo );
    void    EndStaticDecalExport    ( platform            PlatformType );

protected:

    //==========================================================================
    // Structures used for simplifying decal geometry
    //==========================================================================
    struct decal_edge
    {
        s16   P0;
        s16   P1;
        xbool Added;
    };

    struct triangulate_link
    {
        s16     iPrev;
        s16     iNext;
        s16     iPrevConcave;
        s16     iNextConcave;
        xbool   bConcave;
    };

    struct working_data
    {
        enum { MAX_WORKING_VERTS = 512 };
        enum
        {
            FLAG_POLY_ADDED  = 0x0001,
            FLAG_POLY_START  = 0x8000
        };

        // Raw data version
        s32         nVerts;
        u16         VertFlags[MAX_WORKING_VERTS];
        vector3     ClippedVerts[MAX_WORKING_VERTS];
        
        // Indexed version
        s32         nIndexedVerts;
        s16         RemapIndices[MAX_WORKING_VERTS];
        vector3     IndexedVertPool[MAX_WORKING_VERTS];

        // edge data for poly reduction and retriangulation
        s32         nFinalVerts;
        s32         nEdges;
        decal_edge  EdgeList[MAX_WORKING_VERTS];
        s32         nCoplanarPolyVerts;
        s16         CoplanarPolyVerts[MAX_WORKING_VERTS];
        u16         FinalVertFlags[MAX_VERTS_PER_DECAL];
    };

    //==========================================================================
    // The following functions are for creating the different decal types and
    // are very high-level (although still considered protected)
    //==========================================================================
    s32     CalcNoClipDecal         ( s32               Flags,
                                      const vector3&    Point,
                                      const vector3&    Normal,
                                      const vector2&    Size,
                                      radian            Roll,
                                      decal_vert        Verts[MAX_VERTS_PER_DECAL],
                                      matrix4&          L2W );
    s32     CalcProjectedDecal      ( const vector3&    Point,
                                      const vector3&    SurfaceNormal,
                                      const vector3&    NegIncomingRay,
                                      const vector2&    Size,
                                      radian            Roll,
                                      decal_vert        Verts[MAX_VERTS_PER_DECAL],
                                      matrix4&          L2W );

    //==========================================================================
    // These functions are used to add the various geometry triangles to our
    // decal. The geometry polygons will get clipped against the projection
    //==========================================================================
    void    AddGeometryToDecal      ( const rigid_geom* pRigidGeom,
                                      const matrix4&    GeomL2W,
                                      const matrix4&    OrthoProjection,
                                      const vector3&    ProjectionRay,
                                      working_data&     WorkingData     );
    void    CalcDecalVertsFromVolume( const bbox&       WorldBBox,
                                      const matrix4&    OrthoProjection,
                                      const vector3&    ProjectionRay,
                                      working_data&     WorkingData     );

    //==========================================================================
    // These functions are all used to reduce the number of polygons we have by
    // combing coplanar polygons and removing degenerate tris.
    //==========================================================================
    void    CreateIndexedVertPool   ( working_data&     WorkingData     );
    void    RemoveDegenerateTris    ( working_data&     WorkingData     );
    xbool   CollectCoplanarEdges    ( working_data&     WorkingData,
                                      plane&            PolyPlane       );
    void    RemoveDuplicateEdges    ( working_data&     WorkingData     );
    xbool   GetCoplanarPolyVerts    ( working_data&     WorkingData     );
    xbool   IsCollinear             ( const vector3&    StartPoint,
                                      const vector3&    MidPoint,
                                      const vector3&    EndPoint        );
    void    RedoWinding             ( working_data&     WorkingData,
                                      const plane&      PolyPlane       );
    void    BuildConvexStrip        ( working_data&     WorkingData     );
    xbool   TriContainsVert         ( working_data&     WorkingData,
                                      s32               P0,
                                      s32               P1,
                                      s32               P2,
                                      s32               PointToTest     );
    xbool   IsAnEar                 ( working_data&     WorkingData,
                                      triangulate_link  VertPool[MAX_VERTS_PER_DECAL],
                                      s32               FirstConcave,
                                      s32               VertToTest      );
    void    RemoveFromPool          ( triangulate_link  VertPool[MAX_VERTS_PER_DECAL],
                                      s32&              FirstVert,
                                      s32               VertToRemove    );
    void    RemoveFromConcave       ( triangulate_link  VertPool[MAX_VERTS_PER_DECAL],
                                      s32&              FirstConcave,
                                      s32               VertToRemove    );
    xbool   IsConvex                ( working_data&     WorkingData,
                                      triangulate_link  VertPool[MAX_VERTS_PER_DECAL],
                                      s32               VertToTest,
                                      const plane&      PolyPlane       );
    void    Triangulate             ( working_data&     WorkingData,
                                      const plane&      PolyPlane       );
    void    CombineCoplanarPolys    ( working_data&     WorkingData     );

    //==========================================================================
    // Friends for sorting
    //==========================================================================
    friend s32 DecalEdgeSortFn( const void* pA, const void* pB );

    //==========================================================================
    // The hardware-friendly data structures
    //==========================================================================
    struct position_data
    {
        void    FileIO( fileio& File );

        vector3p Pos;
        u32      Flags;
    };

    struct uv_data
    {
        void FileIO( fileio& File );

        s16     U;
        s16     V;
    };

    //==========================================================================
    // A file format that contains all static decals
    //==========================================================================
    struct static_data
    {
        enum { STATIC_DECAL_VERSION = 3 };

        struct package
        {
            void FileIO( fileio& File );

            char    PackageName[256];
            s32     iDefinition;
            s32     nDefinitions;
        };

        struct definition
        {
            void FileIO( fileio& File );

            s32     iGroup;
            s32     iDecalDef;
            s32     iZoneInfo;
            s32     nZones;
        };

        struct zone_info
        {
            void FileIO( fileio& File );

            s32     iVert;
            s32     nVerts;
            u32     Zone;
        };

                    static_data ( void );
                    static_data ( fileio& File );
        void        FileIO      ( fileio& File );

        s32             Version;
        s32             nPackages;
        package*        pPackage;
        s32             nDefinitions;
        definition*     pDefinition;
        s32             nZones;
        zone_info*      pZone;
        s32             nVerts;
        position_data*  pPos;
        uv_data*        pUV;
        u32*            pColor;
    };

    //==========================================================================
    // The runtime information for both dynamic and static decals.
    //==========================================================================
    struct registration_info
    {
        registration_info   ( void );
        ~registration_info  ( void );
        void Kill( void );

        void AllocVertList          ( s32 nVerts );
        void GrowStaticVertListBy   ( s32 nVerts );
        s32  GetAllocSize           ( s32 nVerts );
    
        s32                 m_nVertsAllocated;
        s32                 m_Start;
        s32                 m_End;
        s32                 m_Blank;
        position_data*      m_pPositions;
        uv_data*            m_pUVs;
        u32*                m_pColors;
        f32*                m_pElapsedTimes;
        rhandle<texture>    m_Bitmap;
        u16                 m_BlendMode;
        s32                 m_Flags;
        f32                 m_FadeoutTime;
        xcolor              m_Color;

        #ifdef TARGET_PC
        s32                 m_nStaticVertsAlloced;
        s32                 m_nStaticVerts;
        position_data*      m_pStaticPositions;
        uv_data*            m_pStaticUVs;
        u32*                m_pStaticColors;
        #endif

        s32                 m_StaticDataOffset;
    };

    enum { MAX_DECAL_RESOURCES = 128 };

    //==========================================================================
    // A queue for adding dynamic decals. Since this is an expensive operation,
    // we'll only let one get added per frame
    //==========================================================================
    struct queue_element
    {
        xbool   Valid;  // whether or not this is a valid queue entry
        xhandle Handle; // handle to the registered definition;
        vector2 Size;   // size of the decal to be added
        radian  Roll;   // roll of the decal to be added
        xcolor  Color;  // color of the decal to be added
        s32     Flags;  // flags of the decal to be added
        vector3 Point;  // point of collision
        vector3 Normal; // surface normal
        vector3 NegRay; // Negative incoming ray for projection
    };

    enum { DYNAMIC_QUEUE_SIZE = 8 };

    //==========================================================================
    // Internal functions for creating dynamic decals
    //==========================================================================
    s32         CalcDecalVerts      ( s32                     Flags,
                                      const vector3&          Point,
                                      const vector3&          SurfaceNormal,
                                      const vector3&          NegIncomingRay,
                                      const vector2&          Size,
                                      radian                  Roll,
                                      decal_vert              Verts[MAX_VERTS_PER_DECAL],
                                      matrix4&                L2W );
    s32         GetDecalStart       ( xhandle                 RegInfoHandle,
                                      s32                     nVerts );
    void        AddDecal            ( xhandle                 RegInfoHandle,
                                      s32                     nVerts,
                                      decal_vert              DecalVerts[MAX_VERTS_PER_DECAL],
                                      const matrix4&          L2W );
    void        AddClippedToQueue   ( const decal_definition& DecalDef,
                                      const vector3&          Point,
                                      const vector3&          Normal,
                                      const vector3&          NegIncomingRay,
                                      const vector2&          Size,
                                      radian                  Roll );

    //==========================================================================
    // Internal functions for exporting static decals
    //==========================================================================
    void        SetupExportVertBuffers  ( platform      PlatformType );

    //==========================================================================
    // Rendering functions
    //==========================================================================
    void        RenderVerts         ( s32                nVerts,
                                      position_data*     pPos,
                                      uv_data*           pUV,
                                      u32*               pColor );
    void        RenderEditorStatics ( registration_info& RegInfo,
                                      u32                DrawFlags );
    void        RenderStaticDecals  ( registration_info& RegInfo );
    void        RenderDynamicDecals ( registration_info& RegInfo );

    //==========================================================================
    // Update functions
    //==========================================================================
    void        UpdateAlphaFade     ( f32                DeltaTime,
                                      f32                FadeTime,
                                      s32                nVerts,
                                      u32*               pColor,
                                      f32*               pTimeElapsed );

    //==========================================================================
    // Finally, some data!
    //==========================================================================

    s32                         m_DynamicQueueAddPos;
    s32                         m_DynamicQueueReadPos;
    queue_element               m_DynamicQueue[DYNAMIC_QUEUE_SIZE];
    xharray<registration_info>  m_RegisteredDefs;
    static_data*                m_pStaticData;

    uv_data                     m_TriangleTemplateUV[3];
};

//==============================================================================

inline void decal_mgr::CreateDecalFromRayCast( const decal_definition& Def,
                                               const vector3& Start,
                                               const vector3& End )
{
    CreateDecalFromRayCast( Def, Start, End, Def.RandomSize(), Def.RandomRoll() );
}

//==============================================================================

inline void decal_mgr::CreateDecalAtPoint( const decal_definition& Def,
                                           const vector3&          Point,
                                           const vector3&          Normal )
{
    CreateDecalAtPoint( Def, Point, Normal, Def.RandomSize(), Def.RandomRoll() );
}

//==============================================================================

extern decal_mgr g_DecalMgr;

#endif // DECALMGR_HPP
