#include "ProxyPlaySurface.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "GameLib\RigidGeomCollision.hpp"

//=========================================================================
// proxy_playsurface implementation
//=========================================================================

static struct proxy_playsurface_desc : public object_desc
{
    proxy_playsurface_desc( void ) : object_desc( 
            object::TYPE_PLAY_SURFACE_PROXY,
            "Proxy Play Surface",
            "SYSTEM",
            object::ATTR_COLLIDABLE,
            0 ) {}

    virtual object* Create( void ) { return new proxy_playsurface; }

} s_ProxyPlaySurface_Desc;

//=========================================================================

const object_desc& proxy_playsurface::GetTypeDesc( void ) const
{
    return s_ProxyPlaySurface_Desc;
}

//=========================================================================

const object_desc& proxy_playsurface::GetObjectType( void )
{
    return s_ProxyPlaySurface_Desc;
}

//=========================================================================

proxy_playsurface::proxy_playsurface( void ) :
    object          (),
    m_CurrentGuid   (0)
{
}

//=========================================================================

proxy_playsurface::~proxy_playsurface( void )
{
}

//=========================================================================

bbox proxy_playsurface::GetLocalBBox( void ) const
{
    playsurface_mgr::surface* pSurface = g_PlaySurfaceMgr.m_SpatialDBase.GetSurfaceByGuid(m_CurrentGuid);
    if ( !pSurface )
        return bbox( vector3(0.0f,0.0f,0.0f), 10.0f );

    vector3 Translation = pSurface->L2W.GetTranslation();
    return ( bbox( pSurface->WorldBBox.Min-Translation, pSurface->WorldBBox.Max-Translation ) );
}

//=========================================================================

xbool proxy_playsurface::GetColDetails( s32 Key, object::detail_tri& Tri )
{
    if( Key == -1 )
        return( FALSE );

    playsurface_mgr::surface* pSurface = g_PlaySurfaceMgr.m_SpatialDBase.GetSurfaceByGuid(m_CurrentGuid);
    if ( !pSurface )
        return( FALSE );

    rigid_geom* pRigidGeom = (rigid_geom*)render::GetGeom( pSurface->RenderInst );
    if( !pRigidGeom )
        return( FALSE );

    if( !pRigidGeom->m_Collision.nHighClusters )
        return( FALSE );

    return RigidGeom_GetColDetails( pRigidGeom, &pSurface->L2W, pSurface->pColor, Key, Tri );
}

//=========================================================================

void proxy_playsurface::SetSurface( guid Guid )
{
    m_CurrentGuid = Guid;

    playsurface_mgr::surface* pSurface = g_PlaySurfaceMgr.m_SpatialDBase.GetSurfaceByGuid(m_CurrentGuid);
    SetZones( pSurface->ZoneInfo );
    SetTransform( pSurface->L2W );
    SetAttrBits( pSurface->AttrBits );
}

//=========================================================================

void proxy_playsurface::OnRenderShadowReceive( u64 ProjMask )
{
    playsurface_mgr::surface* pSurface = g_PlaySurfaceMgr.m_SpatialDBase.GetSurfaceByGuid(m_CurrentGuid);

    if ( pSurface->RenderInst.IsNull() )
        return;

    // TODO: This method of vis testing is nasty...should obj_mgr be handling this for playsurfaces?!?!?
    s32 InView = g_ObjMgr.IsBoxInView( pSurface->WorldBBox, XBIN(111111) );

#ifndef X_RETAIL
    // turn on clipping if the screen shot is active
    if ( eng_ScreenShotActive() )
    {
        InView = 1;
    }
#endif

    if ( InView == -1 )
        return;

    s32 Flags  = 0;
    if ( InView != 0 )
        Flags = render::CLIPPED;

    render::AddRigidReceiverSimple( pSurface->RenderInst,
                                    &pSurface->L2W,
                                    Flags,
                                    ProjMask );

    pSurface->RenderFlags |= render::SHADOW_PASS;
}