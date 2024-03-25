
#include "PlaySurface.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "Debris\Debris_mgr.hpp"
#include "MiscUtils\SimpleUtils.hpp"

xbool ShowCollision = FALSE;

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

static struct play_surface_desc : public object_desc
{
    //-------------------------------------------------------------------------

    play_surface_desc( void ) : object_desc( 
        object::TYPE_PLAY_SURFACE, 
        "Play Surface",
        "PLAY SURFACE",
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS      | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_PAIN_LOS        | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             |
            object::ATTR_RECEIVE_SHADOWS        |
            object::ATTR_EDITOR_TEMP_OBJECT     |
            object::ATTR_SPACIAL_ENTRY,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new play_surface ;
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_PlaySurface_Desc;


//=============================================================================

const object_desc&  play_surface::GetTypeDesc( void ) const
{
    return s_PlaySurface_Desc;
}

//=============================================================================

const object_desc&  play_surface::GetObjectType( void )
{
    return s_PlaySurface_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

play_surface::play_surface( void )  
{
}

//=============================================================================

play_surface::~play_surface( void )
{
}

//=============================================================================

void play_surface::OnImport( text_in& TextIn )
{
    (void)TextIn;
}

//=============================================================================

bbox play_surface::GetLocalBBox( void ) const 
{ 
    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom == NULL ) 
    {
        bbox BBox;
        BBox.Set( vector3( -200, -200, -200 ), vector3( 200, 200, 200) );
        return BBox;
    }

    // The geometry bbox should incorporate both the collision and render
    // geometry (which doesn't always match up). Unfortunately, since we use
    // the same bbox for both operations, we have to go with the bigger one.
    return pRigidGeom->m_BBox; 
}

//=============================================================================

void play_surface::OnRender( void )
{
    CONTEXT( "play_surface::OnRender" );

#ifndef X_RETAIL
    if( ShowCollision )
    {
        OnColRender( TRUE );
        return;
    }
#endif // X_RETAIL

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        if( GetAttrBits() & object::ATTR_DISABLE_PROJ_SHADOWS )
            Flags |= render::DISABLE_PROJ_SHADOWS;
        
        if ( pRigidGeom->m_nBones > 1 )
        {
#ifdef X_EDITOR
            // only display the error message once
            static xbool Once = TRUE;
            if ( Once )
            {
                Once = FALSE;
                x_try;
                x_throw( xfs( "Play surface can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
                x_catch_display;
            }
#else
            ASSERTS( 0, xfs( "Play surface can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
#endif
        }
        else
        {
#ifdef X_EDITOR
            if( GetType() == object::TYPE_PLAY_SURFACE )
            {
                // playsurfaces don't get to choose which meshes render on the
                // consoles (performance and data optimization), so make sure the
                // editor behaves the same way.
                m_Inst.Render( &GetL2W(), Flags | GetRenderMode(), (u64)0xffffffffffffffffL, (u8)255 );
            }
            else
#endif
            {
                m_Inst.Render( &GetL2W(), Flags | GetRenderMode() );
            }
        }
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}

//=============================================================================

void play_surface::DoColCheck ( const matrix4* pBone )
{
    CONTEXT("play_surface::DoColCheck");    
    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();

    RigidGeom_ApplyCollision( GetGuid(), 
                              GetBBox(),
                              m_Inst.GetLODMask( U16_MAX ),
                              pBone, 
                              pRigidGeom );
}

//=============================================================================

void play_surface::OnColCheck( void )
{
    DoColCheck( &GetL2W() );
}

//=============================================================================

void play_surface::OnPolyCacheGather( void )
{
    RigidGeom_GatherToPolyCache( GetGuid(), 
                                 GetBBox(), 
                                 m_Inst.GetLODMask( U16_MAX ), 
                                 &GetL2W(), 
                                 m_Inst.GetRigidGeom() );
}

//=============================================================================

#ifndef X_RETAIL
void play_surface::DoColRender( const matrix4* pBone, xbool bRenderHigh )
{
    RigidGeom_RenderCollision( pBone,
                               m_Inst.GetRigidGeom(),
                               bRenderHigh,
                               m_Inst.GetLODMask( U16_MAX ) );
}
#endif // X_RETAIL

//=============================================================================

#ifndef X_RETAIL
void play_surface::OnColRender( xbool bRenderHigh )
{
    DoColRender( &GetL2W(), bRenderHigh );
}
#endif // X_RETAIL

//=============================================================================

xbool play_surface::GetColDetails( s32 Key, detail_tri& Tri )
{
    if( Key == -1 )
        return( FALSE );

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( !pRigidGeom )
        return( FALSE );

    if( !pRigidGeom->m_Collision.nHighClusters )
        return( FALSE );

    return RigidGeom_GetColDetails( pRigidGeom,
                                  & GetL2W(),
                                    m_Inst.GetColorTable(),
                                    Key,
                                    Tri );
}

//=============================================================================

const matrix4* play_surface::GetBoneL2Ws( void )
{
    // Just 1 bone in a play surface
    return &GetL2W() ;
}

//=============================================================================

void play_surface::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );
    m_Inst.OnEnumProp ( List );
}

//=============================================================================

xbool play_surface::OnProperty( prop_query& I )
{
    // HACK: fix this later!
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );
    
    if( object::OnProperty( I ) )
        return( TRUE );
    
    if( m_Inst.OnProperty( I ) )
        return( TRUE );
    
    return( FALSE );
}

//=============================================================================

void play_surface::OnKill( void )
{
    //
    // Be sure polycache knows this object just got nuked
    //
    g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );

    object::OnKill();
}

//==============================================================================

void play_surface::OnTransform( const matrix4& L2W )
{
#ifdef X_EDITOR
    matrix4 CleanM = L2W;
    CleanM.CleanRotation();
    object::OnTransform(CleanM);
#else
    object::OnTransform(L2W);
#endif
}

//=============================================================================

