//===========================================================================
// INCLUDES
//===========================================================================
#include "Ladder_Field.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "entropy.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "CollisionMgr\PolyCache.hpp"


//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

static struct ladder_field_desc : public object_desc
{
    ladder_field_desc( void ) : object_desc( 
            object::TYPE_LADDER_FIELD, 
            "Ladder Field",
            "FIELDS",
            object::ATTR_COLLIDABLE          |
            object::ATTR_COLLISION_PERMEABLE |
            object::ATTR_BLOCKS_PLAYER       |
            object::ATTR_SPACIAL_ENTRY       |
            object::ATTR_COLLIDABLE,

            FLAGS_IS_DYNAMIC | 
            FLAGS_GENERIC_EDITOR_CREATE ) {}
    
    //-------------------------------------------------------------------------
    
    virtual object* Create( void ) 
    { 
        return new ladder_field; 
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        if( Object.IsKindOf( ladder_field::GetRTTI() ) )
        {
            ladder_field& Ladder = ladder_field::GetSafeType( Object );   
            Ladder.RenderLadderField();
        }
        return -1;
    }

#endif // X_EDITOR

} s_Ladder_Field_Desc;

//=========================================================================

const object_desc& ladder_field::GetTypeDesc( void ) const
{
    return s_Ladder_Field_Desc;
}

//=========================================================================

const object_desc&  ladder_field::GetObjectType   ( void )
{
    return s_Ladder_Field_Desc;
}

//===========================================================================

ladder_field::ladder_field() :
    m_Dimensions( 50.f, 400.f, 5.0f )
{
}

//===========================================================================

ladder_field::~ladder_field()
{
}

//===========================================================================

void ladder_field::OnInit( void )
{
}

//===========================================================================

void ladder_field::ComputeCollision( ladder_field::collision& Collision, 
                                     f32                      CylinderDiameter,
                                     f32                      CylinderHeight   )
{
    // Compute local bbox of collision plane
    const vector3& D = m_Dimensions ;
    vector3 Min( -D.GetX()*0.5f, 0,   0   ) ;
    vector3 Max(  D.GetX()*0.5f, D.GetY(), D.GetZ() ) ;

    // Compute collision quads
          vector3   LocalCorners[4] ;
    const matrix4&  L2W       = GetL2W() ;

    // Compute front quad
    LocalCorners[0].Set( Min.GetX(), Min.GetY(), 0.0f );
    LocalCorners[1].Set( Max.GetX(), Min.GetY(), 0.0f );
    LocalCorners[2].Set( Max.GetX(), Max.GetY(), 0.0f );
    LocalCorners[3].Set( Min.GetX(), Max.GetY(), 0.0f );

    L2W.Transform(Collision.m_FrontQuad.m_Pos, LocalCorners, 4) ;

    // Compute top quad by extruding 2 verts back horizontally only (keeps top flat)
    vector3 TopHorizOffset(L2W(2,0) * -30, 0 ,L2W(2,2) * -30) ;
    Collision.m_TopQuad.m_Pos[0] = Collision.m_FrontQuad.m_Pos[2] + TopHorizOffset;
    Collision.m_TopQuad.m_Pos[1] = Collision.m_FrontQuad.m_Pos[3] + TopHorizOffset;
    Collision.m_TopQuad.m_Pos[2] = Collision.m_FrontQuad.m_Pos[3] ;
    Collision.m_TopQuad.m_Pos[3] = Collision.m_FrontQuad.m_Pos[2] ;

    // Compute the back quad lip by offsetting by the cylinder diameter horizontally
    // (to stay in line with top quad), and up in the air. 
    // - This is so the player bangs his head into this plane and prevents falling off
    // - ledges where a ladder top is poking out
    vector3 DiamOffset(L2W(2,0) * CylinderDiameter, CylinderHeight, L2W(2,2) * CylinderDiameter) ;
    vector3 YOffset(L2W(1,0)    * -CylinderHeight, L2W(1,1) * -CylinderHeight, L2W(1,2) * -CylinderHeight) ;
    Collision.m_BackQuad.m_Pos[0] = Collision.m_FrontQuad.m_Pos[3] + DiamOffset ;
    Collision.m_BackQuad.m_Pos[1] = Collision.m_FrontQuad.m_Pos[2] + DiamOffset ;
    Collision.m_BackQuad.m_Pos[2] = Collision.m_FrontQuad.m_Pos[2] + DiamOffset + YOffset ;
    Collision.m_BackQuad.m_Pos[3] = Collision.m_FrontQuad.m_Pos[3] + DiamOffset + YOffset ;
}

//===========================================================================

void ladder_field::OnColCheck( void )
{
    ladder_field::collision Collision ;

    // Use precise object orientated collision
    g_CollisionMgr.StartApply( GetGuid() ) ;

    // Is this a character?
    if (g_CollisionMgr.GetDynamicPrimitive() == PRIMITIVE_DYNAMIC_CYLINDER)
    {
        // Lookup cylinder info
        const collision_mgr::dynamic_cylinder& Cylinder = g_CollisionMgr.GetDynamicCylinder() ;
        ComputeCollision(Collision, 
                         (Cylinder.Radius*2.0f) + 0.2f, 
                         Cylinder.TopStart.GetY() - Cylinder.BotEnd.GetY()) ;

        // Apply back of ladder collision if player is moving down only -
        // this stops the player from falling off ledges where the top of a ladder starts
        f32 YVel = Cylinder.BotEnd.GetY() - Cylinder.BotStart.GetY() ;
        if (YVel < 10.0f)
        {
            g_CollisionMgr.ApplyQuad(Collision.m_BackQuad.m_Pos[0],
                                     Collision.m_BackQuad.m_Pos[1],
                                     Collision.m_BackQuad.m_Pos[2],
                                     Collision.m_BackQuad.m_Pos[3],
                                     object::MAT_TYPE_METAL_GRATE,
                                     0) ;
        }
    }
    else
        ComputeCollision(Collision, 0, 180) ;

    // Apply front of ladder
    g_CollisionMgr.ApplyQuad(Collision.m_FrontQuad.m_Pos[0],
                             Collision.m_FrontQuad.m_Pos[1],
                             Collision.m_FrontQuad.m_Pos[2],
                             Collision.m_FrontQuad.m_Pos[3],
                             object::MAT_TYPE_METAL_GRATE,
                             0) ;
    
    // Apply top of ladder
    g_CollisionMgr.ApplyQuad(Collision.m_TopQuad.m_Pos[0],
                             Collision.m_TopQuad.m_Pos[1],
                             Collision.m_TopQuad.m_Pos[2],
                             Collision.m_TopQuad.m_Pos[3],
                             object::MAT_TYPE_METAL_GRATE,
                             0) ;

    g_CollisionMgr.EndApply();    
}

//===========================================================================

void ladder_field::OnColNotify( object& rObject )
{
    (void)rObject ;
}

//===========================================================================

#ifndef X_RETAIL
void ladder_field::OnColRender( xbool bRenderHigh )
{
    (void)bRenderHigh ;

    // Setup collision colors
    xcolor FrontCol = xcolor(0,255,0, 128) ;
    xcolor TopCol   = xcolor(0,255,0, 128) ;
    xcolor BackCol  = xcolor(0,255,0, 128) ;
    xcolor LineCol  = XCOLOR_WHITE ;

    // Render all collision
    ladder_field::collision Collision ;
    ComputeCollision(Collision, 30*2, 180) ;
    draw_NGon(Collision.m_FrontQuad.m_Pos, 4, FrontCol, FALSE ) ;
    draw_NGon(Collision.m_TopQuad.m_Pos,   4, TopCol,   FALSE ) ;
    draw_NGon(Collision.m_BackQuad.m_Pos,  4, BackCol,  FALSE ) ;
    draw_NGon(Collision.m_FrontQuad.m_Pos, 4, LineCol,  TRUE) ;
    draw_NGon(Collision.m_TopQuad.m_Pos,   4, LineCol,  TRUE) ;
    draw_NGon(Collision.m_BackQuad.m_Pos,  4, LineCol,  TRUE) ;

    // Render the world bbox
    bbox WorldBBox = GetLocalBBox() ;
    WorldBBox.Transform(GetL2W()) ;
    draw_BBox(WorldBBox, XCOLOR_GREEN) ;
}
#endif // X_RETAIL

//===========================================================================

#if !defined( CONFIG_RETAIL )

void ladder_field::RenderLadderField( void )
{
    // Setup normal colors
    xcolor FrontCol = xcolor(255,255,50,80) ;
    xcolor TopCol   = xcolor(150,150,25,80) ;
    xcolor LineCol  = xcolor(0,0,0,     128) ;

    // Select colors?
    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        FrontCol = xcolor(255,255,50,192) ;
        TopCol   = xcolor(150,150,25,192) ;
        LineCol  = XCOLOR_RED ;
    }
    
    // Just render top and front collision
    ladder_field::collision Collision ;
    ComputeCollision(Collision, 180.0f, 180) ;
    draw_NGon(Collision.m_FrontQuad.m_Pos, 4, FrontCol, FALSE ) ;
    draw_NGon(Collision.m_TopQuad.m_Pos,   4, TopCol,   FALSE ) ;
    draw_NGon(Collision.m_FrontQuad.m_Pos, 4, LineCol, TRUE) ;
    draw_NGon(Collision.m_TopQuad.m_Pos,   4, LineCol, TRUE) ;

    // Draw a bbox if selected
    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        // Compute world bbox
        bbox WorldBBox ;
        WorldBBox.Clear() ;
        WorldBBox.AddVerts(Collision.m_FrontQuad.m_Pos, 4) ;
        WorldBBox.AddVerts(Collision.m_TopQuad.m_Pos,   4) ;
        WorldBBox.Inflate(20,20,20) ;

        // Render
        draw_BBox(WorldBBox, XCOLOR_RED) ;
    }
}

#endif // !defined( CONFIG_RETAIL )

//===========================================================================

#ifndef X_RETAIL
void ladder_field::OnDebugRender( void )
{
    RenderLadderField();
}
#endif // X_RETAIL

//=========================================================================

void ladder_field::OnEnumProp( prop_enum& rList )
{
    object::OnEnumProp( rList );

    rList.PropEnumHeader  ( "Ladder Field", "Properties for this ladder field", 0 );
    rList.PropEnumFloat   ( "Ladder Field\\Width",    "The width of the ladder.", 0 );
    rList.PropEnumFloat   ( "Ladder Field\\Height",   "The height of the ladder.", 0 );
}

//===========================================================================

xbool ladder_field::OnProperty( prop_query& rPropQuery )
{
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );

    if ( object::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat   ( "Ladder Field\\Width", m_Dimensions[0] ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat   ( "Ladder Field\\Height", m_Dimensions[1] ) )
    {
        return TRUE;
    }
  
    return FALSE;
}

//===========================================================================

bbox ladder_field::GetLocalBBox( void ) const
{
    const vector3& D = m_Dimensions ;

    // Center on X, origin for Y,Z
    bbox LocalBBox;
    vector3 Min( -D.GetX()*0.5f, 0,      0      ) ;
    vector3 Max(  D.GetX()*0.5f, D.GetY() + 200, D.GetZ() ) ;  // Expand for back collision plane
    LocalBBox.Set( Min, Max );
    if ( LocalBBox.GetSize().LengthSquared() == 0 )
    {
        LocalBBox = bbox( vector3( 0.0f, 0.0f, 0.0f ), 100.f );
    }
    // Expand for top and back collision plane 
    LocalBBox.Inflate(1, 0, 70) ;

    return LocalBBox;
}

//===========================================================================

xbool ladder_field::DoesCylinderIntersect( const vector3& Pos, f32 Height, f32 Radius ) const
{
    vector3 WorldSpheres [12] ;
    vector3 LadderSpheres[12] ;
    s32     nSpheres ;
    s32     i ;
    
    // Compute cylinder info from physics
    vector3 Bottom = Pos ;
    vector3 Top    = Pos + vector3(0, Height, 0) ;

    // Compute spheres of character in world space
    nSpheres = collision_mgr::GetCylinderSpherePositions( Bottom, Top, Radius, WorldSpheres, 12 ) ;
    if (nSpheres == 0)
        return FALSE ;

    // Compute world -> local matrix
    matrix4 W2L = GetL2W() ;
    W2L.InvertRT() ;

    // Put spheres into local space of ladder
    W2L.Transform(LadderSpheres, WorldSpheres, nSpheres) ;

    // Compute local bbox of ladder for intersect test, and inflate by sphere radius
    const vector3& D = m_Dimensions ;
    bbox LocalBBox;
    vector3 Min( -D.GetX()*0.5f, 0,   0  ) ;
    vector3 Max(  D.GetX()*0.5f, D.GetY(), 10 ) ;
    LocalBBox.Set( Min, Max );
    LocalBBox.Inflate(Radius, Radius, Radius) ;

    // If any spheres centers are inside of expanded bbox then character is inside ladder
    for (i = 0 ; i < nSpheres ; i++)
    {
        // Is sphere in bbox?
        if (LocalBBox.Intersect(LadderSpheres[i]))
            return TRUE ;
    }

    return FALSE ;
}

//===========================================================================

const vector3 ladder_field::GetDimensions( void ) const
{
    return m_Dimensions ;
}

//===========================================================================

f32 ladder_field::GetTop( void ) const
{
    const matrix4& L2W = GetL2W() ;
    f32 Top = (L2W(1,1) * m_Dimensions.GetY()) + L2W(3,1) ;
    return Top ;
}

//===========================================================================

f32 ladder_field::GetBottom( void ) const
{
    f32 Bottom = GetPosition().GetY() ;
    return Bottom ;
}

//===========================================================================

void ladder_field::OnPolyCacheGather( void )
{
    return;
    //
    // Build Collision Data
    //
    collision_data CollisionData;
    CollisionData.nHighClusters     = 0;
    CollisionData.nHighIndices      = 0;
    CollisionData.pHighCluster      = NULL;
    CollisionData.pHighIndexToVert0 = 0;
    CollisionData.nLowClusters      = 1;
    CollisionData.nLowQuads         = 1;//3;
    CollisionData.nLowVectors       = 5;//15;

    collision_data::low_cluster     LowCluster;
    collision_data::low_quad        LowQuad[3];
    vector3                         LowVector[15];
    CollisionData.pLowCluster       = &LowCluster;
    CollisionData.pLowQuad          = LowQuad;
    CollisionData.pLowVector        = LowVector;

    collision Collision;
    static f32 LadderDiameter = 150.0f;
    ComputeCollision(Collision, 
                     LadderDiameter + 0.2f, 
                     200.0f ) ;


    LowCluster.BBox.Clear();

    // Set up quad positions


    // Back Quad
    s32 i;
    for ( i = 0; i < 4; i++ )
    {
        CollisionData.pLowVector[i]     = Collision.m_FrontQuad.m_Pos[i];
        LowCluster.BBox                += Collision.m_FrontQuad.m_Pos[i];
        CollisionData.pLowQuad[0].iP[i] = (byte)i;
    }

    // Front Quad
    for ( i = 0; i < 4; i++ )
    {
        CollisionData.pLowVector[4+i]   = Collision.m_FrontQuad.m_Pos[4+i];
        LowCluster.BBox                += Collision.m_FrontQuad.m_Pos[4+i];
        CollisionData.pLowQuad[1].iP[i] = (byte)(4+i);
    }

    // Top Quad
    for ( i = 0; i < 4; i++ )
    {
        CollisionData.pLowVector[4+i]   = Collision.m_TopQuad.m_Pos[8+i];
        LowCluster.BBox                += Collision.m_TopQuad.m_Pos[8+i];
        CollisionData.pLowQuad[1].iP[i] = (byte)(8+i);
    }

    // Set up quad normals

    // Back Quad
    plane Plane;
    Plane.Setup( Collision.m_BackQuad.m_Pos[0], Collision.m_BackQuad.m_Pos[1], Collision.m_BackQuad.m_Pos[2] );
    CollisionData.pLowVector[12] = Plane.Normal;

    // Front Quad
    Plane.Setup( Collision.m_FrontQuad.m_Pos[0], Collision.m_FrontQuad.m_Pos[1], Collision.m_FrontQuad.m_Pos[2] );
    CollisionData.pLowVector[13] = Plane.Normal;

    // Top Quad
    Plane.Setup( Collision.m_TopQuad.m_Pos[0], Collision.m_TopQuad.m_Pos[1], Collision.m_TopQuad.m_Pos[2] );
    CollisionData.pLowVector[14] = Plane.Normal;


    // Set up Low Cluster
    LowCluster.iVectorOffset    = 0;
    LowCluster.iBone            = 0;
    LowCluster.iMesh            = 0;
    LowCluster.iQuadOffset      = 0;
    LowCluster.nPoints          = 12;
    LowCluster.nNormals         = 3;
    LowCluster.nQuads           = 3;

    //
    // Pass it along
    //
    matrix4 L2W;
    L2W.Identity();
    g_PolyCache.GatherCluster( CollisionData, &L2W, ~((u64)0), GetGuid() );
}
