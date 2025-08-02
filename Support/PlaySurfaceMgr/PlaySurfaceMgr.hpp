#ifndef PLAYSURFACEMGR_HPP
#define PLAYSURFACEMGR_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "ZoneMgr\ZoneMgr.hpp"
#include "Render\Render.hpp"
#include "Objects\Object.hpp"
#include "Objects\Render\RenderInst.hpp"

class proxy_playsurface;

//=========================================================================
// This is the main playsurface manager
//=========================================================================

class playsurface_mgr
{
public:
    #ifdef TARGET_PC
    enum { VERSION = 16 };
    #else
    enum { VERSION = 5 };
    #endif

    struct surface
    {
        // must keep this structure 16-byte aligned!!!!
        matrix4             L2W     PS2_ALIGNMENT(16);
        bbox                WorldBBox;
        u32                 AttrBits;       // copy of specific attr bits
        s32                 ColorOffset;
        render::hgeom_inst  RenderInst;     // only valid if resolved
        union
        {
            u32           * pColor32;
            u16           * pColor16;
            void          * pColor;
        };
        u32                 DBaseQuery;     // for internal use by the dbase only
        s32                 NextSurface;    // for internal use by the dbase only
        u16                 ZoneInfo;
        s16                 GeomNameIndex;
        s32                 RenderFlags;
    };

    //---------------------------------------------------------------------
    // Initialization and Kill routines
    //---------------------------------------------------------------------

         playsurface_mgr    ( void );
       ~ playsurface_mgr    ( void );
    void Reset              ( void );
    void Init               ( void );
    void Kill               ( void );

    //---------------------------------------------------------------------
    // Debugging routines
    //---------------------------------------------------------------------
    #ifdef X_DEBUG
    void SanityCheck        ( void );
    #endif

    //---------------------------------------------------------------------
    // Rebuild routines
    //---------------------------------------------------------------------

    void RebuildList        ( const xarray<guid>& lstGuidsToExport,platform PlatformType );

    //---------------------------------------------------------------------
    // File routines
    //---------------------------------------------------------------------

    void OpenFile           ( const char* Filename, xbool DoLoad );
    void UnloadZone         ( zone_mgr::zone_id Zone );
    void LoadZone           ( zone_mgr::zone_id Zone );
    void SaveFile           ( platform  PlatformType );
    void LoadAllZones       ( void );
    void CloseFile          ( void );

    //---------------------------------------------------------------------
    // Collection routines for looking up surfaces (most likely for
    // collision purposes)
    //---------------------------------------------------------------------
    void        CollectSurfaces     ( const bbox&       BBox,
                                      u32               Attributes,
                                      u32               NotTheseAttributes );
    void        CollectSurfaces     ( const vector3&    RayStart,
                                      const vector3&    RayEnd,
                                      u32               Attributes,
                                      u32               NotTheseAttributes );
    surface*    GetNextSurface      ( void );

    //---------------------------------------------------------------------
    // Rendering routines
    //---------------------------------------------------------------------
    void RenderPlaySurfaces ( void );
    void RenderPlaySurfacesCollision ( xbool bRenderHi );

    //---------------------------------------------------------------------
    // Helpers to fake out the collision system
    //---------------------------------------------------------------------
    guid    GetPlaySurfaceGuid  ( void )    { return m_SpatialDBase.GetCurrentGuid(); }

    //---------------------------------------------------------------------
    // the proxy_playsurface is a friend so that it can access collision
    // information without jumping through all kinds of hoops
    //---------------------------------------------------------------------
    void    CreateProxyPlaySurfaceObject( void );
    friend proxy_playsurface;

protected:

    //---------------------------------------------------------------------
    // Protected data types used
    //---------------------------------------------------------------------

    class dbase
    {
    public:
         dbase  ( void );
        ~dbase  ( void );

        // load/save functions...returns the number of bytes written or read
        s32 Load    ( X_FILE* fh );
        s32 Save    ( X_FILE* fh );

        void    Reset   ( void );

        // debugging
        #ifdef X_DEBUG
        void    SanityCheck ( void );
        #endif

        // These functions are for editor use only, and are used to build the
        // initial spatial database.
        void    StartNewDBase   ( s32 CellSize );
        void    AddToNewDBase   ( surface& Surface );
        void    EndNewDBase     ( void );

        // These functions are for loading and unloading zones, and since
        // the spatial database is not meant to be dynamic, do not attempt
        // adding or removing surfaces that haven't been accounted for by
        // the above editor functions.
        void    AddSurface      ( surface& Surface );
        void    RemoveSurface   ( surface& Surface );

        // Collection routines for looking up surfaces (most likely for
        // collision purposes)
        void        CollectSurfaces     ( const bbox&    BBox,
                                          u32            QueryNumber,
                                          u32            Attributes,
                                          u32            NotTheseAttributes );
        void        CollectSurfaces     ( const vector3& RayStart,
                                          const vector3& RayEnd,
                                          u32            QueryNumber,
                                          u32            Attributes,
                                          u32            NotTheseAttributes );
        surface*    GetNextSurface      ( void );
        surface*    GetCurrentSurface   ( void );
        guid        GetCurrentGuid      ( void ) const;
        surface*    GetSurfaceByGuid    ( guid Guid );
        friend      proxy_playsurface;

        u32    m_QueryNumber;

    protected:

        // protected structures and types
        enum { HASH_SIZE = 1021 };  // keep this a prime number for good hashing

        struct cell
        {
            s32     X, Y, Z;
            s32     iSurfaces;
            s32     nSurfaces;
            s32     MaxSurfaces;
        };

        struct hash_entry
        {
            s32 iCells;
            s32 nCells;
        };

        //---------------------------------------------------------------------

        // internal routines for building the database
        void    AddCell         ( xarray<s32>& NextLinks,
                                  s32 XCell, s32 YCell, s32 ZCell );
        void    ReorderCells    ( xarray<s32>& NextLinks );
        
        //---------------------------------------------------------------------

        // internal routines for querying the database
        cell*   GetCell         ( s32 XCell, s32 YCell, s32 ZCell );
        void    GetCellRange    ( const bbox& BBox,
                                  s32& XMin,  s32& XMax,
                                  s32& YMin,  s32& YMax,
                                  s32& ZMin,  s32& ZMax );
        bbox    GetCellBounds   ( s32  XCell, s32  YCell, s32  ZCell );
        void    GetCell         ( f32  XPos,  f32  YPos,  f32  ZPos,
                                  s32& XCell, s32& YCell, s32& ZCell );
        u32     DBaseHashFn     ( s32  XCell, s32  YCell, s32  ZCell );

        //---------------------------------------------------------------------

        // internal data
        xbool               m_BuildingNewDBase;
        s32                 m_CellSize;
        hash_entry          m_HashTable[HASH_SIZE];
        xarray<cell>        m_Cells;
        xarray<surface*>    m_Surfaces;

        // collection data
        s32                 m_CurrentSurface;
        s32                 m_NextSurface;
    };

    //---------------------------------------------------------------------

    struct zone_info
    {
        zone_info           ( void );
       ~zone_info           ( void );

        void Unload         ( void );

        xbool               Resolved;
        s32                 FileOffset;
        s32                 NSurfaces;
        surface*            pSurfaces;
        s32                 NColors;
        void*               pColorData;
        void*               hColorData;
    };

    //---------------------------------------------------------------------

    struct geom_name
    {
        char Name[RESOURCE_NAME_SIZE];
    };

    //---------------------------------------------------------------------
    // Internal functions
    //---------------------------------------------------------------------
    
    void    SortSurfaces        ( zone_info& Zone );
    s32     FindPortalIndex     ( u16 Zone1, u16 Zone2 );
    s32     FindZoneIndex       ( u16 Zone1, u16 Zone2 );
    s32     GetNameIndex        ( const char* GeomName );
    void    UnloadZone          ( zone_info& Zone );
    void    LoadZone            ( zone_info& Zone );
    void    LoadBasicInfo       ( void );
    void    ResolveSurfaceData  ( zone_info& Zone );
    void    PrepVisCheck        ( void );
    s32     VisCheck            ( const bbox& BBox, u32 CheckPlaneMask );
    void    RenderZone          ( zone_info& Zone,
                                  zone_mgr::zone_id Zone1,
                                  zone_mgr::zone_id Zone2 );
    void    ClearDBaseQueries   ( void );
    void    RenderZoneCollision ( zone_info& ZoneInfo, xbool bRenderHigh );
    void    MarkLitPlaySurfaces ( void );

    friend s32  SurfaceCompareFn( const void* p1, const void* p2 );


    //---------------------------------------------------------------------
    // The data
    //---------------------------------------------------------------------

    s32                 m_Version;
    xarray<zone_info>   m_Zones;
    xarray<zone_info>   m_Portals;
    xarray<geom_name>   m_Geoms;
    X_FILE*             m_File;
    xbool               m_Loading;
    dbase               m_SpatialDBase;
    u32                 m_QueryNumber;
    guid                m_ProxyPlaySurface;
};

//=========================================================================

extern playsurface_mgr  g_PlaySurfaceMgr;

//=========================================================================

#endif // PLAYSURFACEMGR_HPP
