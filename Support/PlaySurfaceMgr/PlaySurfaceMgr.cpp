#include "PlaySurfaceMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\ProxyPlaySurface.hpp"
#include "Gamelib\RigidGeomCollision.hpp"
#include "Render\LightMgr.hpp"

#ifdef TARGET_XBOX
#include "Entropy/XBox/xbox_private.hpp"
#endif

#ifdef TARGET_PS2
#include "Entropy/PS2/ps2_spad.hpp"
#endif

//=========================================================================
// typedefs and structures
//=========================================================================

struct file_header
{
    s32 Version;
    s32 NZones;
    s32 NPortals;
    s32 NGeoms;
};

//=========================================================================
// globals
//=========================================================================

playsurface_mgr g_PlaySurfaceMgr;

#ifdef TARGET_PS2
static matrix4 s_GBToClip;
#endif

//=========================================================================
// zone_info implementation
//=========================================================================

playsurface_mgr::zone_info::zone_info( void ) :
    Resolved            (FALSE),
    FileOffset          (0),
    NSurfaces           (0),
    pSurfaces           (NULL),
    NColors             (0),
    pColorData          (NULL),
    hColorData          (NULL)
{
    // surface MUST be 16-byte aligned.
    ASSERT( (sizeof(surface) & 0xf) == 0 );
}

//=========================================================================

playsurface_mgr::zone_info::~zone_info( void )
{
    Unload();
}

//=========================================================================

void playsurface_mgr::zone_info::Unload( void )
{
#ifdef TARGET_XBOX
    if( hColorData )
    {
        delete( vert_factory::handle )hColorData;
        hColorData = NULL;
        pColorData = NULL;
    }
#else
    if ( pColorData )   x_free( pColorData );
#endif
    if ( pSurfaces  )   x_free( pSurfaces  );
    pSurfaces  = NULL;
    pColorData = NULL;
}

//=========================================================================
// playsurface_mgr implementation
//=========================================================================

playsurface_mgr::playsurface_mgr( void ) :
    m_Version           (VERSION),
    m_Zones             (),
    m_Portals           (),
    m_Geoms             (),
    m_File              (NULL),
    m_Loading           (FALSE),
    m_SpatialDBase      (),
    m_QueryNumber       (0),
    m_ProxyPlaySurface  (0)
{
};

//=========================================================================

playsurface_mgr::~playsurface_mgr( void )
{
    Reset();
}

//=========================================================================

void playsurface_mgr::Reset( void )
{
    s32 i;

    m_QueryNumber = 0;
    m_SpatialDBase.m_QueryNumber = 0;

    // unload all zones and portals
    for ( i = 0; i < m_Zones.GetCount(); i++ )
        UnloadZone(i);
    for ( i = 0; i < m_Portals.GetCount(); i++ )
        UnloadZone( m_Portals[i] );

    // clear out the data
    m_Zones.Clear();
    m_Portals.Clear();
    m_Geoms.Clear();

    m_SpatialDBase.Reset();

    // if there are no more playsurfaces, then there is no more need
    // for the proxy playsurface
    if( m_ProxyPlaySurface )
    {
        g_ObjMgr.DestroyObjectEx( m_ProxyPlaySurface, TRUE );
        g_ObjMgr.SetProxyPlaySurface( NULL );
        m_ProxyPlaySurface  = 0;
    }
}

//=========================================================================

void playsurface_mgr::Init( void )
{
    Reset();
}

//=========================================================================

void playsurface_mgr::Kill( void )
{
    Reset();
}

//=========================================================================

#ifdef X_DEBUG
void playsurface_mgr::SanityCheck( void )
{
    ASSERT( m_Version == VERSION );
    m_SpatialDBase.SanityCheck();
}
#endif

//=========================================================================

s32 playsurface_mgr::FindPortalIndex( u16 Zone1, u16 Zone2 )
{
    if ( Zone1 && Zone2 )
    {
        // this guy belongs in a portal, but which one?
        for ( s32 iPortal = 0; iPortal < g_ZoneMgr.GetPortalCount(); iPortal++ )
        {
            zone_mgr::portal& Portal = g_ZoneMgr.GetPortal(iPortal);

            if ( ((Portal.iZone[0] == Zone1) && (Portal.iZone[1] == Zone2)) ||
                 ((Portal.iZone[0] == Zone2) && (Portal.iZone[1] == Zone1)) )
            {
                return iPortal;
            }
        }
    }

    return -1;
}

//=========================================================================

s32 playsurface_mgr::FindZoneIndex( u16 Zone1, u16 Zone2 )
{
    // place it in the first zone?
    if ( Zone1 && (Zone1 < g_ZoneMgr.GetZoneCount()) )
    {
        return (s32)Zone1;
    }

    // place it in the second zone?
    if ( Zone2 && (Zone2 < g_ZoneMgr.GetZoneCount()) )
    {
        return (s32)Zone2;
    }

    // if all else fails, put it in the default/global zone
    return 0;
}

//=========================================================================

s32 playsurface_mgr::GetNameIndex( const char* GeomName )
{
    // is this name already in the list?
    for ( s32 i = 0; i < m_Geoms.GetCount(); i++ )
    {
        if ( !x_strcmp( m_Geoms[i].Name, GeomName ) )
        {
            return i;
        }
    }

    // if not, add it!
    geom_name& Geom = m_Geoms.Append();
    x_strsavecpy( Geom.Name, GeomName, RESOURCE_NAME_SIZE );
    return (m_Geoms.GetCount() - 1);
}

//=========================================================================

s32 SurfaceCompareFn( const void* p1, const void* p2 )
{
    playsurface_mgr::surface* pSurf1 = (playsurface_mgr::surface*)p1;
    playsurface_mgr::surface* pSurf2 = (playsurface_mgr::surface*)p2;

    if ( pSurf1->GeomNameIndex > pSurf2->GeomNameIndex )    return  1;
    if ( pSurf1->GeomNameIndex < pSurf2->GeomNameIndex )    return -1;

    return 0;
}

//=========================================================================

void playsurface_mgr::SortSurfaces( playsurface_mgr::zone_info& Zone )
{
    x_qsort( Zone.pSurfaces, Zone.NSurfaces, sizeof(surface), SurfaceCompareFn );
}

//=========================================================================

void playsurface_mgr::RebuildList( const xarray<guid>& lstGuidsToExport,platform PlatformType )
{
    s32     i;
    s32     iGuid;

    // this routine is kinda nasty, but it is only done at export time, so it
    // shouldn't be that big of a problem
    Reset();

    // make the arrays big enough for our zones and portals
    s32 ZoneCount   = g_ZoneMgr.GetZoneCount();
    s32 PortalCount = g_ZoneMgr.GetPortalCount();
    ZoneCount = MAX( 1, ZoneCount );    // must have the default zone at least
    m_Zones.SetCapacity ( ZoneCount   );
    m_Portals.SetCapacity( PortalCount );

    // initialize the zones and portals
    for ( i = 0; i < ZoneCount; i++ )
    {
        zone_info& ZoneInfo       = m_Zones.Append();
        ZoneInfo.Resolved         = FALSE;
        ZoneInfo.FileOffset       = 0;
        ZoneInfo.NSurfaces        = 0;
        ZoneInfo.pSurfaces        = NULL;
        ZoneInfo.NColors          = 0;
        ZoneInfo.pColorData       = NULL;
    }

    for ( i = 0; i < PortalCount; i++ )
    {
        zone_info& ZoneInfo       = m_Portals.Append();
        ZoneInfo.Resolved         = FALSE;
        ZoneInfo.FileOffset       = 0;
        ZoneInfo.NSurfaces        = 0;
        ZoneInfo.pSurfaces        = NULL;
        ZoneInfo.NColors          = 0;
        ZoneInfo.pColorData       = NULL;
    }

    // how many playsurfaces will we need to store? And how much color data?
    for ( iGuid = 0; iGuid < lstGuidsToExport.GetCount(); iGuid++ )
    {
        // grab the relevant pointers
        object*       pObject      = g_ObjMgr.GetObjectByGuid( lstGuidsToExport[iGuid] );
        play_surface* pPlaySurface = &play_surface::GetSafeType( *pObject );
        rigid_inst&   RigidInst    = pPlaySurface->GetRigidInst();

        // which zone or portal does this belong to?
        u16 Zone1 = pObject->GetZone1();
        u16 Zone2 = pObject->GetZone2();
        ASSERT( Zone1 ? TRUE : (Zone2==0) );

        // does this guy belong in a portal?        
        s32 PortalIndex = FindPortalIndex( Zone1, Zone2 );
        if ( PortalIndex != -1 )
        {
            ASSERT( (PortalIndex >= 0) && (PortalIndex < PortalCount) );
            m_Portals[PortalIndex].NSurfaces++;
            m_Portals[PortalIndex].NColors += RigidInst.GetNumColors();
        }
        else
        {
            // it must belong in a zone
            s32 ZoneIndex = FindZoneIndex( Zone1, Zone2 );
            ASSERT( (ZoneIndex >= 0) && (ZoneIndex < ZoneCount) );
            m_Zones[ZoneIndex].NSurfaces++;
            m_Zones[ZoneIndex].NColors += RigidInst.GetNumColors();
        }
    }

    // now that we have our counts, allocate space for the surfaces and
    // color data (zones and portals), and as long as we're looping, clear out
    // the NSurfaces and NColors members (don't worry, they'll get added back in,
    // it's just a handy way to add the actual surface data).
    for ( i = 0; i < ZoneCount; i++ )
    {
        zone_info& ZoneInfo = m_Zones[i];
        if( ZoneInfo.NSurfaces )
            ZoneInfo.pSurfaces  = (surface*)x_malloc(sizeof(surface)*ZoneInfo.NSurfaces);
        if( ZoneInfo.NColors )
        {
            u32 Size;
            switch( PlatformType )
            {
                case PLATFORM_XBOX:
                    Size = sizeof(u32)*ZoneInfo.NColors;
                    break;
                case PLATFORM_PS2:
				    Size = sizeof(u16)*ZoneInfo.NColors;
                    break;
                case PLATFORM_PC:
                    Size = sizeof(u32)*ZoneInfo.NColors;
                    break;

                default:
                    Size = 0xFFFFFFFF;
                    ASSERT(0);
                    break;
            }
            ZoneInfo.pColorData = x_malloc( Size );
        }

        ZoneInfo.NSurfaces = 0;
        ZoneInfo.NColors   = 0;
    }

    for ( i = 0; i < PortalCount; i++ )
    {
        zone_info& ZoneInfo = m_Portals[i];
        if( ZoneInfo.NSurfaces )
            ZoneInfo.pSurfaces  = (surface*)x_malloc(sizeof(surface)*ZoneInfo.NSurfaces);
        if( ZoneInfo.NColors )
        {
            u32 Size;
            switch( PlatformType )
            {
                case PLATFORM_XBOX:
                    Size = sizeof(u32)*ZoneInfo.NColors;
                    break;
                case PLATFORM_PS2:
				    Size = sizeof(u16)*ZoneInfo.NColors;
                    break;
                case PLATFORM_PC:
                    Size = sizeof(u32)*ZoneInfo.NColors;
                    break;

                default:
                    ASSERT(0);
                    Size = 0xFFFFFFFF;
                    break;
            }
            ZoneInfo.pColorData = x_malloc( Size );
        }

        ZoneInfo.NSurfaces = 0;
        ZoneInfo.NColors   = 0;
    }

    // create the surface and color data
    for ( iGuid = 0; iGuid < lstGuidsToExport.GetCount(); iGuid++ )
    {
        // grab the relevant pointers
        object*             pObject      = g_ObjMgr.GetObjectByGuid( lstGuidsToExport[iGuid] );
        play_surface*       pPlaySurface = &play_surface::GetSafeType( *pObject );
        rigid_inst&         RigidInst    = pPlaySurface->GetRigidInst();

        // which zone or portal does this belong to?
        u16 Zone1 = pObject->GetZone1();
        u16 Zone2 = pObject->GetZone2();
        ASSERT( Zone1 ? TRUE : (Zone2==0) );

        // does this guy belong in a portal?        
        s32 PortalIndex = FindPortalIndex( Zone1, Zone2 );
        zone_info* pZoneInfo;
        if ( PortalIndex != -1 )
        {
            ASSERT( (PortalIndex >= 0) && (PortalIndex < PortalCount) );
            pZoneInfo = &m_Portals[PortalIndex];
        }
        else
        {
            // it must belong in a zone
            s32 ZoneIndex = FindZoneIndex( Zone1, Zone2 );
            ASSERT( (ZoneIndex >= 0) && (ZoneIndex < ZoneCount) );
            pZoneInfo = &m_Zones[ZoneIndex];
        }

        static const u32 AttrBits = object::ATTR_DISABLE_PROJ_SHADOWS     |
                                    object::ATTR_RECEIVE_SHADOWS          |
                                    object::ATTR_COLLIDABLE               |
                                    object::ATTR_BLOCKS_PLAYER            |
                                    object::ATTR_BLOCKS_CHARACTER         |
                                    object::ATTR_BLOCKS_RAGDOLL           |
                                    object::ATTR_BLOCKS_SMALL_DEBRIS      |
                                    object::ATTR_BLOCKS_SMALL_PROJECTILES |
                                    object::ATTR_BLOCKS_LARGE_PROJECTILES |
                                    object::ATTR_BLOCKS_CHARACTER_LOS     |
                                    object::ATTR_BLOCKS_PLAYER_LOS        |
                                    object::ATTR_BLOCKS_PAIN_LOS;

        // add in the surface data
        surface* pSurface = &pZoneInfo->pSurfaces[pZoneInfo->NSurfaces];
        pSurface->L2W       = pObject->GetL2W();
        pSurface->WorldBBox = pObject->GetBBox();
        pSurface->AttrBits  = pObject->GetAttrBits() & AttrBits;
        switch( PlatformType )
        {
            case PLATFORM_XBOX:
                pSurface->ColorOffset   = pZoneInfo->NColors*sizeof(u32);
                break;
            case PLATFORM_PS2:
			    pSurface->ColorOffset   = pZoneInfo->NColors*sizeof(u16);
                break;
            case PLATFORM_PC:
                pSurface->ColorOffset   = pZoneInfo->NColors*sizeof(u32);
                break;

            default:
                ASSERT(0);
                break;
        }
        pSurface->GeomNameIndex = GetNameIndex(RigidInst.GetRigidGeomName());
        pSurface->DBaseQuery    = 0;
        pSurface->ZoneInfo      = (Zone1&0xff) | ((Zone2&0xff)<<8);
        pSurface->RenderFlags   = 0;
        if( pSurface->AttrBits & object::ATTR_DISABLE_PROJ_SHADOWS )
        {
            pSurface->RenderFlags |= render::DISABLE_PROJ_SHADOWS;
        }

        // add in the color data
        const void* pInstColors = RigidInst.GetColorTable( PlatformType );
        if ( RigidInst.GetNumColors() && pInstColors )
        {
            void* pColorData = ((byte*)pZoneInfo->pColorData)+pSurface->ColorOffset;
            u32 Size;
            switch( PlatformType )
            {
                case PLATFORM_XBOX:
                    Size = RigidInst.GetNumColors()*sizeof(u32);
                    break;
                case PLATFORM_PS2:
				    Size = RigidInst.GetNumColors()*sizeof(u16);
                    break;
                case PLATFORM_PC:
                    Size = RigidInst.GetNumColors()*sizeof(u32);
                    break;

                default:
                    ASSERT(0);
                    Size = 0xFFFFFFFF;
                    break;
            }
            x_memcpy( pColorData,pInstColors,Size );
        }

        // keep our counts current
        pZoneInfo->NSurfaces++;
        pZoneInfo->NColors   += RigidInst.GetNumColors();
    }

    // now we have enough information to build up the spatial database
    static const s32 kSpatialCellSize = 800; // 8 meters
    m_SpatialDBase.StartNewDBase( kSpatialCellSize );
    for ( i = 0; i < ZoneCount; i++ )
    {
        zone_info& ZoneInfo = m_Zones[i];
        for ( s32 iSurface = 0; iSurface < ZoneInfo.NSurfaces; iSurface++ )
            m_SpatialDBase.AddToNewDBase( ZoneInfo.pSurfaces[iSurface] );
    }
    for ( i = 0; i < PortalCount; i++ )
    {
        zone_info& ZoneInfo = m_Portals[i];
        for ( s32 iSurface = 0; iSurface < ZoneInfo.NSurfaces; iSurface++ )
            m_SpatialDBase.AddToNewDBase( ZoneInfo.pSurfaces[iSurface] );
    }
    m_SpatialDBase.EndNewDBase();

    // done! that was easy, right?
}

//=========================================================================

void playsurface_mgr::OpenFile( const char* Filename, xbool DoLoad )
{
    if ( DoLoad )
    {
        m_Loading = TRUE;
        m_File    = x_fopen( Filename, "rb" );
        if ( !m_File )
        {
            ASSERT( FALSE );
            return;
        }

        // load the info shared by everyone
        LoadBasicInfo();
    }
    else
    {
        m_Loading = FALSE;
        m_File    = x_fopen( Filename, "wb" );
        if ( !m_File )
        {
            ASSERT( FALSE );
            return;
        }
    }
}

//=========================================================================

void playsurface_mgr::UnloadZone( zone_info& Zone )
{
    // unregister the playsurfaces and remove them from the spatial dbase
    if ( Zone.Resolved && Zone.pSurfaces )
    {
        for ( s32 i = 0; i < Zone.NSurfaces; i++ )
        {
            if( Zone.pSurfaces[i].RenderInst.IsNonNull() )
            {
                // unregister the render instance
                render::UnregisterRigidInstance( Zone.pSurfaces[i].RenderInst );
            }

            // and remove it from the spatial database
            m_SpatialDBase.RemoveSurface( Zone.pSurfaces[i] );
        }

        Zone.Resolved = FALSE;
    }

    // delete the associated data
    Zone.Unload();
}

//=========================================================================

void playsurface_mgr::LoadZone( zone_info& Zone )
{
    MEMORY_OWNER( "LOAD ZONE" );

    // make sure we're unloaded so we don't cause memory leaks
    UnloadZone(Zone);

    // empty zone?
    if ( Zone.NSurfaces == 0 )
        return;

    // seek to the surface and color data
    x_fseek( m_File, Zone.FileOffset, X_SEEK_SET );
    
    // load the surfaces
    {
        MEMORY_OWNER( "PLAYSURFACES" );
        Zone.pSurfaces  = (surface*)x_malloc(sizeof(surface)*Zone.NSurfaces);
        ASSERT( Zone.pSurfaces );
        x_fread( Zone.pSurfaces, sizeof(surface), Zone.NSurfaces, m_File );
    }

    // load the color data
    if ( Zone.NColors )
    {
        MEMORY_OWNER( "COLOR DATA" );
	#if defined(TARGET_XBOX)
        vert_factory::handle hColorData = g_VertFactory.Create( "Zone colour data",Zone.NColors*sizeof( u32 ),0 );
        x_fread( hColorData->m_Ptr,sizeof(u32),Zone.NColors,m_File );
        Zone.pColorData = hColorData->m_Ptr;
        Zone.hColorData = hColorData;
    #elif defined(TARGET_PS2)
        Zone.pColorData = (byte*)x_malloc(Zone.NColors*sizeof(u16));
        ASSERT ( Zone.pColorData );
        x_fread( Zone.pColorData, 1, Zone.NColors*sizeof(u16), m_File );
    #elif defined(TARGET_PC)
        Zone.pColorData = (byte*)x_malloc(Zone.NColors*sizeof(u32));
        ASSERT ( Zone.pColorData );
        x_fread( Zone.pColorData, 1, Zone.NColors*sizeof(u32), m_File );
    #endif
    }

    // resolve pointers and geometry handles
    ResolveSurfaceData( Zone );
}

//=========================================================================

void playsurface_mgr::UnloadZone( zone_mgr::zone_id Zone )
{
    ASSERT( Zone<m_Zones.GetCount() );
    zone_info& ZoneInfo = m_Zones[(s32)Zone];
    UnloadZone( ZoneInfo );
}

//=========================================================================

void playsurface_mgr::LoadZone( zone_mgr::zone_id Zone )
{
    ASSERT( m_File && m_Loading );
    ASSERT( Zone<m_Zones.GetCount() );
    zone_info& ZoneToLoad = m_Zones[(s32)Zone];
    LoadZone( ZoneToLoad );
}

//=========================================================================

void playsurface_mgr::LoadAllZones( void )
{
    // Start at 1 since the default zone is always loaded.
    for( s32 iZone = 1; iZone < m_Zones.GetCount(); iZone++ )
    {
        LoadZone( iZone );
    }

    #ifdef X_DEBUG
    m_SpatialDBase.SanityCheck();
    #endif
}

//=========================================================================

void playsurface_mgr::SaveFile( platform PlatformType )
{
    ASSERT( m_File && !m_Loading );

    // save out the header
    file_header Hdr;
    Hdr.Version  = VERSION;
    Hdr.NZones   = m_Zones.GetCount();
    Hdr.NPortals = m_Portals.GetCount();
    Hdr.NGeoms   = m_Geoms.GetCount();
    x_fwrite( &Hdr, sizeof(file_header), 1, m_File );

    // save out the spatial database
    m_SpatialDBase.Save( m_File );

    // save out the pieces of unique geometry
    x_fwrite( m_Geoms.GetPtr(), sizeof(geom_name), m_Geoms.GetCount(), m_File );

    // where would playsurfaces and colors begin?
    s32 StartOffset = x_ftell( m_File );
    StartOffset    += m_Zones.GetCount() * sizeof(zone_info);
    StartOffset    += m_Portals.GetCount() * sizeof(zone_info);

    // save out zones with the appropriate pointers cleared out, and the
    // file offsets figured out
    s32 i;
    for ( i = 0; i < m_Zones.GetCount(); i++ )
    {
        zone_info ZoneToSave   = m_Zones[i];
        ZoneToSave.pColorData  = NULL;
        ZoneToSave.pSurfaces   = NULL;
        ZoneToSave.Resolved    = FALSE;
        ZoneToSave.FileOffset  = StartOffset;
        StartOffset           += m_Zones[i].NSurfaces*sizeof(surface);
        switch( PlatformType )
        {
            case PLATFORM_XBOX:
                StartOffset   += m_Zones[i].NColors*sizeof(u32);
                break;
            case PLATFORM_PS2:
			    StartOffset   += m_Zones[i].NColors*sizeof(u16);
                break;
            case PLATFORM_PC:
                StartOffset   += m_Zones[i].NColors*sizeof(u32);
                break;

            default:
                ASSERT(0);
                break;
        }
        x_fwrite( &ZoneToSave, sizeof(zone_info), 1, m_File );
    }

    // save out portals with the appropriate pointers cleared out, and the
    // file offsets figured out
    for ( i = 0; i < m_Portals.GetCount(); i++ )
    {
        zone_info ZoneToSave   = m_Portals[i];
        ZoneToSave.pColorData  = NULL;
        ZoneToSave.pSurfaces   = NULL;
        ZoneToSave.Resolved    = FALSE;
        ZoneToSave.FileOffset  = StartOffset;
        StartOffset           += m_Portals[i].NSurfaces*sizeof(surface);
        switch( PlatformType )
        {
            case PLATFORM_XBOX:
                StartOffset   += m_Portals[i].NColors*sizeof(u32);
                break;
            case PLATFORM_PS2:
			    StartOffset   += m_Portals[i].NColors*sizeof(u16);
                break;
            case PLATFORM_PC:
                StartOffset   += m_Portals[i].NColors*sizeof(u32);
                break;

            default:
                ASSERT(0);
                break;
        }
        x_fwrite( &ZoneToSave, sizeof(zone_info), 1, m_File );
    }

    // save out the playsurfaces and colorsfor zones and portals
    for ( i = 0; i < m_Zones.GetCount(); i++ )
    {
        zone_info& ZoneToSave = m_Zones[i];
        if ( ZoneToSave.NSurfaces )
            x_fwrite( ZoneToSave.pSurfaces, sizeof(surface), ZoneToSave.NSurfaces, m_File );
        if ( ZoneToSave.NColors )
        {
            switch( PlatformType )
            {
                case PLATFORM_XBOX:
                    x_fwrite( ZoneToSave.pColorData, sizeof(u32), ZoneToSave.NColors, m_File );
                    break;
                case PLATFORM_PS2:
				    x_fwrite( ZoneToSave.pColorData, sizeof(u16), ZoneToSave.NColors, m_File );
                    break;
                case PLATFORM_PC:
                    x_fwrite( ZoneToSave.pColorData, sizeof(u32), ZoneToSave.NColors, m_File );
                    break;

                default:
                    ASSERT(0);
                    break;
            }
        }
    }

    for ( i = 0; i < m_Portals.GetCount(); i++ )
    {
        zone_info& ZoneToSave = m_Portals[i];
        if ( ZoneToSave.NSurfaces )
            x_fwrite( ZoneToSave.pSurfaces, sizeof(surface), ZoneToSave.NSurfaces, m_File );
        if ( ZoneToSave.NColors )
        {
            switch( PlatformType )
            {
                case PLATFORM_XBOX:
                    x_fwrite( ZoneToSave.pColorData, sizeof(u32), ZoneToSave.NColors, m_File );
                    break;
                case PLATFORM_PS2:
				    x_fwrite( ZoneToSave.pColorData, sizeof(u16), ZoneToSave.NColors, m_File );
                    break;
                case PLATFORM_PC:
                    x_fwrite( ZoneToSave.pColorData, sizeof(u32), ZoneToSave.NColors, m_File );
                    break;

                default:
                    ASSERT(0);
                    break;
            }
        }
    }

    // sanity check
    ASSERT( StartOffset == x_ftell(m_File) );
}

//=========================================================================

void playsurface_mgr::CloseFile( void )
{
    ASSERT( m_File );
    x_fclose( m_File );
    m_Loading = FALSE;
    m_File    = NULL;
}

//=========================================================================

void playsurface_mgr::CreateProxyPlaySurfaceObject( void )
{
    // If we're about to start loading playsurfaces, then we need to provide
    // and interface for the game to talk to them.
    m_ProxyPlaySurface  = g_ObjMgr.CreateObject( proxy_playsurface::GetObjectType() );
    object* pProxy      = g_ObjMgr.GetObjectByGuid(m_ProxyPlaySurface);
    ASSERT( pProxy );
    g_ObjMgr.SetProxyPlaySurface( pProxy );
}

//=========================================================================

void playsurface_mgr::LoadBasicInfo()
{
    MEMORY_OWNER("BASIC INFO");
    ASSERT( m_File && m_Loading );
    x_fseek( m_File, 0, X_SEEK_SET );

    // make sure we start with a clean slate
    Reset();

    // Create the proxy play surface.
    CreateProxyPlaySurfaceObject();

    // load the header info
    file_header Hdr;
    x_fread( &Hdr, 1, sizeof(file_header), m_File );
    if ( Hdr.Version != VERSION )
        x_throw( "Playsurface data is wrong version." );

    // load the spatial database information
    {
        MEMORY_OWNER("SPATIAL DBASE");
        m_SpatialDBase.Load( m_File );
    }

    // load the different geometries this level uses
    {
        MEMORY_OWNER("GEOMETRY NAMES");
        m_Geoms.SetCapacity( Hdr.NGeoms );
        m_Geoms.SetCount( Hdr.NGeoms );
        x_fread( m_Geoms.GetPtr(), sizeof(geom_name), Hdr.NGeoms, m_File );
    }

    // load the basic zone information
    {
        MEMORY_OWNER("ZONE INFO");
        m_Zones.SetCapacity( Hdr.NZones );
        m_Zones.SetCount( Hdr.NZones );
        x_fread( m_Zones.GetPtr(), sizeof(zone_info), Hdr.NZones, m_File );
    }
    
    // load the basic portal information
    {
        MEMORY_OWNER("PORTAL INFO");
        m_Portals.SetCapacity( Hdr.NPortals );
        m_Portals.SetCount( Hdr.NPortals );
        x_fread( m_Portals.GetPtr(), sizeof(zone_info), Hdr.NPortals, m_File );
    }

    // load the globals zone
    if ( m_Zones.GetCount() > 0 )
        LoadZone( 0 );

    // load all of the portals
    for ( s32 i = 0; i < m_Portals.GetCount(); i++ )
    {
        LoadZone( m_Portals[i] );
    }

    // sanity check time
    if ( g_ZoneMgr.GetZoneCount() && (m_Zones.GetCount() != g_ZoneMgr.GetZoneCount()) )
        x_throw( "playsurface data and zones don't match" );
    if ( g_ZoneMgr.GetPortalCount() && (m_Portals.GetCount() != g_ZoneMgr.GetPortalCount()) )
        x_throw( "playsurface data and portals don't match" );
}

//=========================================================================

void playsurface_mgr::ResolveSurfaceData( zone_info& Zone )
{
    ASSERT( Zone.Resolved == FALSE );
    Zone.Resolved = TRUE;

    s32 iSurface;
    for ( iSurface = 0; iSurface < Zone.NSurfaces; iSurface++ )
    {
        surface* pSurface = &Zone.pSurfaces[iSurface];

        // fix up the color pointer
        pSurface->pColor = ((byte*)Zone.pColorData)+pSurface->ColorOffset;

        // fix up the geometry
        s32 iGeomName = pSurface->GeomNameIndex;
        ASSERT( (iGeomName>=0) && (iGeomName < m_Geoms.GetCount()) );
        rhandle<rigid_geom> RigidGeom;
        RigidGeom.SetName( m_Geoms[iGeomName].Name );
        rigid_geom* pGeom = RigidGeom.GetPointer();
        //ASSERT( pGeom );
        if ( pGeom )
        {
            pSurface->RenderInst = render::RegisterRigidInstance( *pGeom );
        }
        else
        {
            pSurface->RenderInst = HNULL;
        }

        // make sure our database queries will work out the first time through
        ASSERT( pSurface->DBaseQuery == 0 );
    }

    // sort the surfaces by their geometry indices. *hopefully* this will
    // improve the data cache slightly for rendering...
    SortSurfaces( Zone );

    // add the surfaces to our spatial database
    for ( iSurface = 0; iSurface < Zone.NSurfaces; iSurface++ )
        m_SpatialDBase.AddSurface( Zone.pSurfaces[iSurface] );

    #ifdef TARGET_PS2
    // we dma surface data to scratchpad, so make sure we've flushed the
    // data cache
    FlushCache(0);
    #endif
}

//=========================================================================

void playsurface_mgr::PrepVisCheck( void )
{
#ifdef TARGET_PS2
    static const s32 kGuardbandClip = 1800;

    const view* pActiveView = eng_GetView();
    pActiveView->GetW2C( kGuardbandClip, s_GBToClip );
#endif
}

//=========================================================================

s32 playsurface_mgr::VisCheck( const bbox& BBox, u32 CheckPlaneMask )
{
    #if !defined(TARGET_PS2)
    {
        return g_ObjMgr.IsBoxInView( BBox, CheckPlaneMask );
    }
    #else
    {
        // QUESTION: Could this work be done in vu1 micro mode, and how much
        // would that give us? Is it worth exploring?
        (void)CheckPlaneMask;
        register s32        Result;
        register u_long128  Temp1;
        register u_long128  Temp2;

        asm
        ("
            .set noreorder

            lqc2    vf24, 0x00(%5)  # load the guardband clip matrix
            lqc2    vf25, 0x10(%5)  # load the guardband clip matrix
            lqc2    vf26, 0x20(%5)  # load the guardband clip matrix
            lqc2    vf27, 0x30(%5)  # load the guardband clip matrix
            lqc2    vf28, 0x00(%4)  # load the screen clip matrix
            lqc2    vf29, 0x10(%4)  # load the screen clip matrix
            lqc2    vf30, 0x20(%4)  # load the screen clip matrix
            lqc2    vf31, 0x30(%4)  # load the screen clip matrix

            #// load the bounding box into a vf registers
            lqc2    vf01, 0(%3)
            lqc2    vf08, 16(%3)

            #// build the other corners of our bounding box
            vadd.xy         vf02,   vf00,   vf01
            vadd.xz         vf03,   vf00,   vf01
            vadd.x          vf04,   vf00,   vf01
            vadd.yz         vf05,   vf00,   vf01
            vadd.y          vf06,   vf00,   vf01
            vadd.z          vf07,   vf00,   vf01
            vadd.z          vf02,   vf00,   vf08
            vadd.y          vf03,   vf00,   vf08
            vadd.yz         vf04,   vf00,   vf08
            vadd.x          vf05,   vf00,   vf08
            vadd.xz         vf06,   vf00,   vf08
            vadd.xy         vf07,   vf00,   vf08

            #//////////////////////////////////////////////////////////////////////
            #// do the clipping--note this code is kinda nasty, because the clip
            #// tests and transforms are all interleaved, but we're going for
            #// performance, not readability, right?
            #//////////////////////////////////////////////////////////////////////
            li              %0,     0               # accumulated or's
            li              %2,     0x3f            # accumulated and's
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 0 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf01z   # xform point 0 to screen clip
            vmadday.xyzw    acc,    vf29,   vf01y   # xform point 0 to screen clip
            vmaddx.xyzw     vf09,   vf28,   vf01x   # xform point 0 to screen clip
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 1 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf02z   # xform point 1 to screen clip
            vmadday.xyzw    acc,    vf29,   vf02y   # xform point 1 to screen clip
            vmaddx.xyzw     vf10,   vf28,   vf02x   # xform point 1 to screen clip
            vclipw.xyz      vf09,   vf09            #      clip test 0
            vnop
            vnop
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 2 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf03z   # xform point 2 to screen clip
            vmadday.xyzw    acc,    vf29,   vf03y   # xform point 2 to screen clip
            vmaddx.xyzw     vf11,   vf28,   vf03x   # xform point 2 to screen clip
            cfc2            %1,     $vi18           # read clip test 0
            vclipw.xyz      vf10,   vf10            #      clip test 1
            or              %0,     %0,     %1      # or   clip test 0
            and             %2,     %2,     %1      # and  clip test 0
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 3 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf04z   # xform point 3 to screen clip
            vmadday.xyzw    acc,    vf29,   vf04y   # xform point 3 to screen clip
            vmaddx.xyzw     vf12,   vf28,   vf04x   # xform point 3 to screen clip
            cfc2            %1,     $vi18           # read clip test 1
            vclipw.xyz      vf11,   vf11            #      clip test 2
            or              %0,     %0,     %1      # or   clip test 1
            and             %2,     %2,     %1      # and  clip test 1
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 4 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf05z   # xform point 4 to screen clip
            vmadday.xyzw    acc,    vf29,   vf05y   # xform point 4 to screen clip
            vmaddx.xyzw     vf13,   vf28,   vf05x   # xform point 4 to screen clip
            cfc2            %1,     $vi18           # read clip test 2
            vclipw.xyz      vf12,   vf12            #      clip test 3
            or              %0,     %0,     %1      # or   clip test 2
            and             %2,     %2,     %1      # and  clip test 2
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 5 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf06z   # xform point 5 to screen clip
            vmadday.xyzw    acc,    vf29,   vf06y   # xform point 5 to screen clip
            vmaddx.xyzw     vf14,   vf28,   vf06x   # xform point 5 to screen clip
            cfc2            %1,     $vi18           # read clip test 3
            vclipw.xyz      vf13,   vf13            #      clip test 4
            or              %0,     %0,     %1      # or   clip test 3
            and             %2,     %2,     %1      # and  clip test 3
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 6 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf07z   # xform point 6 to screen clip
            vmadday.xyzw    acc,    vf29,   vf07y   # xform point 6 to screen clip
            vmaddx.xyzw     vf15,   vf28,   vf07x   # xform point 6 to screen clip
            cfc2            %1,     $vi18           # read clip test 4
            vclipw.xyz      vf14,   vf14            #      clip test 5
            or              %0,     %0,     %1      # or   clip test 4
            and             %2,     %2,     %1      # and  clip test 4
            vmulaw.xyzw     acc,    vf31,   vf00w   # xform point 7 to screen clip
            vmaddaz.xyzw    acc,    vf30,   vf08z   # xform point 7 to screen clip
            vmadday.xyzw    acc,    vf29,   vf08y   # xform point 7 to screen clip
            vmaddx.xyzw     vf16,   vf28,   vf08x   # xform point 7 to screen clip
            cfc2            %1,     $vi18           # read clip test 5
            vclipw.xyz      vf15,   vf15            #      clip test 6
            or              %0,     %0,     %1      # or   clip test 5
            and             %2,     %2,     %1      # and  clip test 5
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 0 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf01z   # xform point 0 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf01y   # xform point 0 to guardband clip
            vmaddx.xyzw     vf01,   vf24,   vf01x   # xform point 0 to guardband clip
            cfc2            %1,     $vi18           # read clip test 6
            vclipw.xyz      vf16,   vf16            #      clip test 7
            or              %0,     %0,     %1      # or   clip test 6
            and             %2,     %2,     %1      # and  clip test 6
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 1 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf02z   # xform point 1 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf02y   # xform point 1 to guardband clip
            vmaddx.xyzw     vf02,   vf24,   vf02x   # xform point 1 to guardband clip
            cfc2            %1,     $vi18           # read clip test 7
            vclipw.xyz      vf01,   vf01            #      clip test 0 (guardband)
            or              %0,     %0,     %1      # or   clip test 7
            and             %2,     %2,     %1      # and  clip test 7

            #//////////////////////////////////////////////////////////////////////
            #// at this point, we are completely transformed to clip space, and can
            #// perform trivial rejection or acceptance against the smaller frustum
            #//////////////////////////////////////////////////////////////////////
            andi            %0,     %0,     0x3f    # lop off the top bits that aren't useful
            bgtz            %2,     0f              # completely outside of a plane?
            nop
            beq             %0,     $0,     1f      # completely inside?
            nop
            andi            %1,     %0,     0x10    # throw away far z
            bgtz            %1,     0f              # reject anything that touches the far plane

            #//////////////////////////////////////////////////////////////////////
            #// now attempt trivial acceptance within the guardband, otherwise
            #// we need to clip (rejection testing was already done earlier)
            #//////////////////////////////////////////////////////////////////////
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 2 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf03z   # xform point 2 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf03y   # xform point 2 to guardband clip
            vmaddx.xyzw     vf03,   vf24,   vf03x   # xform point 2 to guardband clip
            cfc2            %1,     $vi18           # read clip test 0 (guardband)
            vclipw.xyz      vf02,   vf02            #      clip test 1 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 0 (guardband)
            bgtz            %1,     1f              # bit set   test 0 (guardband)
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 3 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf04z   # xform point 3 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf04y   # xform point 3 to guardband clip
            vmaddx.xyzw     vf04,   vf24,   vf04x   # xform point 3 to guardband clip
            cfc2            %1,     $vi18           # read clip test 1 (guardband)
            vclipw.xyz      vf03,   vf03            #      clip test 2 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 1 (guardband)
            bgtz            %1,     1f              # bit set   test 1 (guardband)
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 4 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf05z   # xform point 4 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf05y   # xform point 4 to guardband clip
            vmaddx.xyzw     vf05,   vf24,   vf05x   # xform point 4 to guardband clip
            cfc2            %1,     $vi18           # read clip test 2 (guardband)
            vclipw.xyz      vf04,   vf04            #      clip test 3 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 2 (guardband)
            bgtz            %1,     1f              # bit set   test 2 (guardband)
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 5 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf06z   # xform point 5 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf06y   # xform point 5 to guardband clip
            vmaddx.xyzw     vf06,   vf24,   vf06x   # xform point 5 to guardband clip
            cfc2            %1,     $vi18           # read clip test 3 (guardband)
            vclipw.xyz      vf05,   vf05            #      clip test 4 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 3 (guardband)
            bgtz            %1,     1f              # bit set   test 3 (guardband)
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 6 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf07z   # xform point 6 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf07y   # xform point 6 to guardband clip
            vmaddx.xyzw     vf07,   vf24,   vf07x   # xform point 6 to guardband clip
            cfc2            %1,     $vi18           # read clip test 4 (guardband)
            vclipw.xyz      vf06,   vf06            #      clip test 5 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 4 (guardband)
            bgtz            %1,     1f              # bit set   test 4 (guardband)
            vmulaw.xyzw     acc,    vf27,   vf00w   # xform point 7 to guardband clip
            vmaddaz.xyzw    acc,    vf26,   vf08z   # xform point 7 to guardband clip
            vmadday.xyzw    acc,    vf25,   vf08y   # xform point 7 to guardband clip
            vmaddx.xyzw     vf08,   vf24,   vf08x   # xform point 7 to guardband clip
            cfc2            %1,     $vi18           # read clip test 5 (guardband)
            vclipw.xyz      vf07,   vf07            #      clip test 6 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 5 (guardband)
            bgtz            %1,     1f              # bit set   test 5 (guardband)
            vnop
            vnop
            vnop
            vnop
            cfc2            %1,     $vi18           # read clip test 6 (guardband)
            vclipw.xyz      vf08,   vf08            #      clip test 7 (guardband)
            andi            %1,     %1,     0x3f    # and  clip test 6 (guardband)
            bgtz            %1,     1f              # bit set   test 6 (guardband)
            vnop
            vnop
            vnop
            vnop
            cfc2            %1,     $vi18           # read clip test 7 (guardband)
            vnop
            andi            %1,     %1,     0x3f    # and  clip test 7 (guardband)
            bgtz            %1,     1f              # bit set   test 7 (guardband)
            nop
            
            b               1f
            addi            %0,     $0,     0x00    # acceptance inside guardband

        0:
            addi    %0, $0, -1

        1:
            .set reorder

                " : "=r" (Result) , "=r" (Temp1) , "=r" (Temp2) : "r" (&BBox), "r" (&eng_GetView()->GetW2C()), "r" (&s_GBToClip) :
                "vf1",  "vf2",  "vf3",  "vf4",  "vf5",  "vf6",  "vf7",  "vf8",
                "vf9",  "vf10", "vf11", "vf12", "vf13", "vf14", "vf16",
                "vf24", "vf25", "vf26", "vf27", "vf28", "vf29", "vf30", "vf31" );
        return Result;
    }
    #endif
}

//=========================================================================

void playsurface_mgr::RenderZone( zone_info& ZoneInfo, zone_mgr::zone_id Zone1, zone_mgr::zone_id Zone2 )
{
    s32 i;

    // if this zone isn't loaded fully, we can't render it
    if ( ZoneInfo.Resolved == FALSE )
        return;

    if ( ZoneInfo.NSurfaces == 0 )
        return;

    // use the starting zone to determine whether we should do a zone vis check
    s32   StartingZone = g_ZoneMgr.GetStartingZone();
    xbool bNotInStartingZone = !((Zone1==StartingZone) || (Zone2==StartingZone));

    // limit ourselves to 4k of playsurfaces in spad (using 8k total double-buffered)
#if defined(TARGET_PS2)
    ASSERT( SPAD.GetUsableSize() >= 8192 );
    SPAD.Lock();
#endif
    const s32 kMaxSurfacesToProcess = 4096 / sizeof(surface);
    s32 NSurfacesProcessed = 0;

    // dma the first batch of surfaces to scratchpad
    s32 SpadOffsets[2] = { 0, 4096 };
    s32 Buffer = 0;
    s32 nSurfacesToDma = MIN(kMaxSurfacesToProcess, ZoneInfo.NSurfaces-NSurfacesProcessed);
    surface* pDmaSource = ZoneInfo.pSurfaces;
#if defined(TARGET_PS2)
    SPAD.DmaTo( SpadOffsets[Buffer], pDmaSource, nSurfacesToDma*sizeof(surface) );
#endif
    pDmaSource += nSurfacesToDma;
    Buffer = !Buffer;

    // what are the default render flags?
    s32 DefaultFlags = 0;

#ifndef X_RETAIL
    if ( eng_ScreenShotActive() )
        DefaultFlags |= render::CLIPPED;
#endif

    // the surface processing loop
    surface* pSurface = ZoneInfo.pSurfaces;
    while ( NSurfacesProcessed < ZoneInfo.NSurfaces )
    {
        // how many surfaces will this loop handle?
        s32 NSurfacesToProcess = nSurfacesToDma;

        // make sure the surfaces have finished dma'ing
        #ifdef TARGET_PS2
        SPAD.DmaSyncTo();
        #endif

        // start dma'ing the next batch
        nSurfacesToDma = MIN(kMaxSurfacesToProcess, ZoneInfo.NSurfaces-NSurfacesProcessed-NSurfacesToProcess);
        if ( nSurfacesToDma )
        {
            #ifdef TARGET_PS2
            SPAD.DmaTo( SpadOffsets[Buffer], pDmaSource, nSurfacesToDma*sizeof(surface) );
            #endif
            pDmaSource += nSurfacesToDma;
        }
        Buffer = !Buffer;

        // handle the current batch of surfaces
        #ifdef TARGET_PS2
        surface* pSpadSurface = (surface*)((byte*)SPAD.GetUsableStartAddr()+SpadOffsets[Buffer]);
        #else
        surface* pSpadSurface = pSurface;
        #endif
        for ( i = 0; i < NSurfacesToProcess; i++, pSurface++, pSpadSurface++ )
        {
            // check if this surface is in the zone
            if ( bNotInStartingZone && !g_ZoneMgr.IsBBoxVisible(pSpadSurface->WorldBBox, Zone1, Zone2 ) )
            {
                continue;
            }

            // check for clipping against the view frustum
            s32 Vis = VisCheck(pSpadSurface->WorldBBox, XBIN(111111));
            if ( Vis == -1 )
            {
                continue;
            }

            // check if we have a valid instance (bad data could cause this to get hit)
            if ( pSpadSurface->RenderInst.IsNull() )
            {
                continue;
            }

            // accumulate any other flags (Note that we don't use pSpadSurface because
            // we have no guarantee that pSurface was flushed from the cache before
            // our dma. That's okay becauses everything we access besides the render
            // flags is constant.)
            s32 Flags = pSurface->RenderFlags | DefaultFlags;

            // to clip or not to clip?
            if ( Vis )
                Flags |= render::CLIPPED;

            // render it
            #if defined(TARGET_XBOX) || defined(TARGET_PC)
            render::AddRigidInstanceSimple( pSpadSurface->RenderInst,
                                            (const u32*)pSpadSurface->pColor,
                                            &pSurface->L2W,
                                            pSpadSurface->WorldBBox,
                                            Flags );
            #else
            render::AddRigidInstanceSimple( pSpadSurface->RenderInst,
                                            (const u16*)pSpadSurface->pColor,
                                            &pSurface->L2W,
                                            pSpadSurface->WorldBBox,
                                            Flags );
            #endif

            // clear any accumulated flags for the next frame
            pSurface->RenderFlags &= ~(render::CLIPPED | render::SHADOW_PASS | render::DO_SIMPLE_LIGHTING);
        }

        // move to the next batch
        NSurfacesProcessed += NSurfacesToProcess;
    }

    ASSERT( pDmaSource == ZoneInfo.pSurfaces + ZoneInfo.NSurfaces );

#if defined(TARGET_PS2)
    SPAD.Unlock();
#endif
}

//=========================================================================

#ifndef X_RETAIL
void playsurface_mgr::RenderZoneCollision( zone_info& ZoneInfo, xbool bRenderHigh )
{
    s32 i;

    // if this zone isn't loaded fully, we can't render it
    if ( ZoneInfo.Resolved == FALSE )
        return;

    if ( ZoneInfo.NSurfaces == 0 )
        return;

    // Render all of the playsurfaces
    for ( i = 0; i < ZoneInfo.NSurfaces; i++ )
    {
        surface* pSurface = &ZoneInfo.pSurfaces[i];

        if ( pSurface->RenderInst.IsNull() )
            continue;

        matrix4* pL2W = NULL;

        pL2W = &pSurface->L2W;

        // grab the useful pointers out
        rigid_geom* pGeom = (rigid_geom*)render::GetGeom( pSurface->RenderInst );


        RigidGeom_RenderCollision( pL2W, pGeom, bRenderHigh, 0xFFFFFFFF );
    }
}
#endif // X_RETAIL

//=========================================================================

void playsurface_mgr::MarkLitPlaySurfaces( void )
{
    s32 i;
    s32 nLights = g_LightMgr.GetNNonCharLights();

    // Loop through all of the dynamic lights, and mark playsurfaces that
    // may be hit by them.
    for( i = 0; i < nLights; i++ )
    {
        vector3 Pos;
        f32     Radius;
        xcolor  Color;
        g_LightMgr.GetLight( i, Pos, Radius, Color );
        CollectSurfaces( bbox(Pos, Radius), object::ATTR_ALL, 0 );
        
        surface* pSurface = GetNextSurface();
        while( pSurface != NULL )
        {
            pSurface->RenderFlags |= render::DO_SIMPLE_LIGHTING;
            pSurface = GetNextSurface();
        }
    }
}

//=========================================================================

void playsurface_mgr::RenderPlaySurfaces( void )
{
#ifndef X_EDITOR
    s32 i;

    if ( m_Zones.GetCount() == 0 )
        return;

    // mark playsurfaces that need to have lighting calculations done (going
    // through the lights using the spatial dbase should be quicker than
    // doing additional checks for all playsurfaces)
    MarkLitPlaySurfaces();

    // render the zones
    PrepVisCheck();
    RenderZone( m_Zones[0], 0, 0 );   // default zone is always visible
    for ( i = 1; i < m_Zones.GetCount(); i++ )
    {
        if ( g_ZoneMgr.IsZoneVisible((u8)i) )
        {
            RenderZone( m_Zones[i], i, 0 );
        }
    }

    // render the portals
    for ( i = 0; i < m_Portals.GetCount(); i++ )
    {
        zone_mgr::portal& Portal = g_ZoneMgr.GetPortal(i);
        if ( g_ZoneMgr.IsZoneVisible(Portal.iZone[0]) ||
             g_ZoneMgr.IsZoneVisible(Portal.iZone[1]) )
        {
            RenderZone( m_Portals[i], Portal.iZone[0], Portal.iZone[1] );
        }
    }
#endif // X_EDITOR
}

//=========================================================================

#ifndef X_RETAIL
void playsurface_mgr::RenderPlaySurfacesCollision( xbool bRenderHi )
{
#ifndef X_EDITOR
    s32 i;

    if ( m_Zones.GetCount() == 0 )
        return;

    // render the zones
    PrepVisCheck();
    RenderZone( m_Zones[0], 0, 0 );   // default zone is always visible
    for ( i = 1; i < m_Zones.GetCount(); i++ )
    {
        if ( g_ZoneMgr.IsZoneVisible((u8)i) )
        {
            RenderZoneCollision( m_Zones[i], bRenderHi );
        }
    }

    // render the portals
    for ( i = 0; i < m_Portals.GetCount(); i++ )
    {
        zone_mgr::portal& Portal = g_ZoneMgr.GetPortal(i);
        if ( g_ZoneMgr.IsZoneVisible(Portal.iZone[0]) ||
             g_ZoneMgr.IsZoneVisible(Portal.iZone[1]) )
        {
            RenderZoneCollision( m_Portals[i], bRenderHi );
        }
    }
#endif // X_EDITOR
}
#endif // X_RETAIL

//=========================================================================

void playsurface_mgr::ClearDBaseQueries( void )
{
    s32 i, j;
    for ( i = 0; i < m_Zones.GetCount(); i++ )
    {
        zone_info& ZoneInfo = m_Zones[i];
        for ( j = 0; j < ZoneInfo.NSurfaces; j++ )
            ZoneInfo.pSurfaces[j].DBaseQuery = 0;
    }

    for ( i = 0; i < m_Portals.GetCount(); i++ )
    {
        zone_info& ZoneInfo = m_Portals[i];
        for ( j = 0; j < ZoneInfo.NSurfaces; j++ )
            ZoneInfo.pSurfaces[j].DBaseQuery = 0;
    }
}

//=========================================================================

void playsurface_mgr::CollectSurfaces( const bbox&  BBox,
                                       u32          Attributes,
                                       u32          NotTheseAttributes )
{
    if ( m_Zones.GetCount() == 0 )
    {
        m_QueryNumber = 0;
        return;
    }

    m_QueryNumber++;
    if ( m_QueryNumber == 0 )
    {
        // zero is considered a special query, and will force us to reset
        // all of the id's so that we are 100% correct when this number
        // wraps around
        ClearDBaseQueries();
        m_QueryNumber++;
    }

    m_SpatialDBase.CollectSurfaces( BBox, m_QueryNumber, Attributes, NotTheseAttributes );
}

//=========================================================================

void playsurface_mgr::CollectSurfaces( const vector3& RayStart,
                                       const vector3& RayEnd,
                                       u32            Attributes,
                                       u32            NotTheseAttributes )
{
    if ( m_Zones.GetCount() == 0 )
    {
        m_QueryNumber = 0;
        return;
    }

    m_QueryNumber++;
    if ( m_QueryNumber == 0 )
    {
        // zero is considered a special query, and will force us to reset
        // all of the id's so that we are 100% correct when this number
        // wraps around
        ClearDBaseQueries();
        m_QueryNumber++;
    }

    m_SpatialDBase.CollectSurfaces( RayStart, RayEnd, m_QueryNumber, Attributes, NotTheseAttributes );
}

//=========================================================================

playsurface_mgr::surface* playsurface_mgr::GetNextSurface( void )
{
    return m_SpatialDBase.GetNextSurface();
}

//==============================================================================
