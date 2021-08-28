//==============================================================================
//
//  InvisWall.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================

//==========================================================================
// INCLUDE
//==========================================================================

#include "InvisWall.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "OccluderMgr\OccluderMgr.hpp"

//==========================================================================
// DEFINES
//==========================================================================

//==========================================================================
// GLOBAL
//==========================================================================

//==========================================================================
// FUNTIONS
//==========================================================================

static struct invisible_wall_desc : public object_desc
{
    invisible_wall_desc( void ) : object_desc( 
            object::TYPE_INVISIBLE_WALL_OBJ, 
            "Invisible Wall", 
            "SCRIPT",

            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_ACTORS      |         
            object::ATTR_TRANSPARENT            |
#ifdef X_EDITOR
            // Allows us to turn their display on or off within the Editor only
            object::ATTR_RENDERABLE             |
#endif           
            object::ATTR_SPACIAL_ENTRY,
            
            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC ) {}         

    //-------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return new invisible_wall_obj; 
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR
    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        Object.OnDebugRender();
        return EDITOR_ICON_PORTAL;
    }
#endif // X_EDITOR

} s_Invisible_Wall_Desc;

//=========================================================================

const object_desc& invisible_wall_obj::GetTypeDesc( void ) const
{
    return s_Invisible_Wall_Desc;
}

//=========================================================================

const object_desc& invisible_wall_obj::GetObjectType( void )
{
    return s_Invisible_Wall_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

invisible_wall_obj::invisible_wall_obj( void ) 
{
    m_Width             = 400.0f;
    m_Height            = 400.0f;
    m_Depth             = 5.0f;
    m_bIsOccluder       = FALSE;
}    

//=========================================================================
invisible_wall_obj::~invisible_wall_obj( void )
{

}
//=========================================================================

bbox invisible_wall_obj::GetLocalBBox( void ) const
{
    return bbox(vector3(m_Width/2,m_Height/2,m_Depth/2),vector3(-m_Width/2,-m_Height/2,-m_Depth/2));
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

void invisible_wall_obj::OnRender( void )
{
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

#if !defined( CONFIG_RETAIL )

void invisible_wall_obj::OnDebugRender( void )
{
    CONTEXT( "invisible_wall_obj::OnDebugRender" );

    matrix4 L2W = GetL2W();
    draw_SetL2W( L2W );
    bbox Wall(vector3(-m_Width/2,-m_Height/2,m_Depth/2), vector3(m_Width/2,m_Height/2,-m_Depth/2));
    draw_Volume( Wall, xcolor(255,0,0,128) );
    draw_ClearL2W();

    draw_BBox    ( GetBBox(), xcolor(255,255,255,255) );
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

#if !defined( CONFIG_RETAIL )

void invisible_wall_obj::OnRenderTransparent( void )
{
    matrix4 L2W = GetL2W();
    draw_SetL2W( L2W );
    bbox Wall(vector3(-m_Width/2,-m_Height/2,m_Depth/2), vector3(m_Width/2,m_Height/2,-m_Depth/2));
    draw_Volume( Wall, xcolor(0,255,0,64) );
    draw_ClearL2W();
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

void invisible_wall_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );
    List.PropEnumHeader  ( "InvisWall",                  "InvisWall Properties", 0          );
    List.PropEnumFloat   ( "InvisWall\\Width",           "InvisWall Width.", 0              );
    List.PropEnumFloat   ( "InvisWall\\Height",          "InvisWall Height.", 0             );
    List.PropEnumFloat   ( "InvisWall\\Depth",           "InvisWall Depth.", 0              );
    List.PropEnumBool    ( "InvisWall\\IsOccluder",      "Does wall occlude rendering", 0   );
}

//=============================================================================

xbool invisible_wall_obj::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
    }
    else if( I.VarFloat("InvisWall\\Width", m_Width ) )
    {
        if( !I.IsRead() )
        {
            OnMove( GetPosition() );
        }
    }
    else if( I.VarFloat("InvisWall\\Height", m_Height ) )
    {
        if( !I.IsRead() )
        {
            OnMove( GetPosition() );
        }
    }
    else if( I.VarFloat("InvisWall\\Depth", m_Depth ) )
    {
        if( !I.IsRead() )
        {
            OnMove( GetPosition() );
        }
    }
    else if( I.IsVar("InvisWall\\IsOccluder") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( m_bIsOccluder );
        }
        else
        {
            m_bIsOccluder = I.GetVarBool(); 
        }
    }
    else
    {   
        return FALSE ;        
    }
    return TRUE;
}

//==========================================================================

void invisible_wall_obj::OnColCheck( void )
{
    CONTEXT("invisible_wall_obj::OnColCheck");

    //
    // Compute corners
    //
    vector3 Corner[8];
    {
        vector3 Pos[8];
        //vector3 C[4];
        f32 W  = m_Width  * 0.5f;
        f32 H  = m_Height * 0.5f;
        f32 D  = m_Depth  * 0.5f;
        Pos[0] = vector3( -W, +H, D );
        Pos[1] = vector3( -W, -H, D );
        Pos[2] = vector3( +W, -H, D );
        Pos[3] = vector3( +W, +H, D );
        
        Pos[4] = vector3( -W, +H, -D ); 
        Pos[5] = vector3( -W, -H, -D ); 
        Pos[6] = vector3( +W, -H, -D ); 
        Pos[7] = vector3( +W, +H, -D ); 

        matrix4 L2W = GetL2W();
        L2W.Transform( Corner, Pos, 8 );
    }

    g_CollisionMgr.StartApply( GetGuid() );
    // Side 1
    g_CollisionMgr.ApplyTriangle( Corner[0],Corner[1],Corner[2], object::MAT_TYPE_NULL ); 
    g_CollisionMgr.ApplyTriangle( Corner[0],Corner[2],Corner[3], object::MAT_TYPE_NULL ); 
    // Side 2
    g_CollisionMgr.ApplyTriangle( Corner[4],Corner[0],Corner[1], object::MAT_TYPE_NULL ); 
    g_CollisionMgr.ApplyTriangle( Corner[4],Corner[1],Corner[5], object::MAT_TYPE_NULL ); 
    // Side 3
    g_CollisionMgr.ApplyTriangle( Corner[4],Corner[7],Corner[3], object::MAT_TYPE_NULL ); 
    g_CollisionMgr.ApplyTriangle( Corner[4],Corner[3],Corner[0], object::MAT_TYPE_NULL ); 
    // Side 4
    g_CollisionMgr.ApplyTriangle( Corner[3],Corner[7],Corner[6], object::MAT_TYPE_NULL ); 
    g_CollisionMgr.ApplyTriangle( Corner[3],Corner[6],Corner[2], object::MAT_TYPE_NULL ); 
    // Side 5
    g_CollisionMgr.ApplyTriangle( Corner[1],Corner[2],Corner[6], object::MAT_TYPE_NULL ); 
    g_CollisionMgr.ApplyTriangle( Corner[1],Corner[6],Corner[5], object::MAT_TYPE_NULL ); 
    // Side 6
    g_CollisionMgr.ApplyTriangle( Corner[7],Corner[4],Corner[5], object::MAT_TYPE_NULL ); 
    g_CollisionMgr.ApplyTriangle( Corner[7],Corner[5],Corner[6], object::MAT_TYPE_NULL ); 
    //
    g_CollisionMgr.EndApply();
}     

//==========================================================================

xbool invisible_wall_obj::GetOccluderPoints( vector3* pPoints ) const
{
    if( m_bIsOccluder == FALSE )
        return FALSE;

    f32 W      = m_Width  * 0.5f;
    f32 H      = m_Height * 0.5f;
    pPoints[0] = vector3( -W, +H, 0 );
    pPoints[1] = vector3( -W, -H, 0 );
    pPoints[2] = vector3( +W, -H, 0 );
    pPoints[3] = vector3( +W, +H, 0 );

    matrix4 L2W = GetL2W();
    L2W.Transform( pPoints, pPoints, 4 );

    return TRUE;
}

//==========================================================================
void invisible_wall_obj::OnMove(const vector3& NewPos )
{
    object::OnMove(NewPos);

    if( m_bIsOccluder )
        g_OccluderMgr.DirtyOccluders();
}

//==========================================================================
void invisible_wall_obj::OnTransform (const matrix4& L2W )
{
    object::OnTransform(L2W);

    if( m_bIsOccluder )
        g_OccluderMgr.DirtyOccluders();
}

//==========================================================================

void invisible_wall_obj::OnColNotify( object& Object )
{
    (void)Object;
} 

//==========================================================================

void invisible_wall_obj::OnPolyCacheGather( void )
{

    //
    // Compute corners
    //
    vector3 Corner[8];
    vector3 Normal[6];
    {
        vector3 Pos[8];
        f32 W  = m_Width  * 0.5f;
        f32 H  = m_Height * 0.5f;
        f32 D  = m_Depth  * 0.5f;

        Pos[0] = vector3( -W, +H, D );
        Pos[1] = vector3( -W, -H, D );
        Pos[2] = vector3( +W, -H, D );
        Pos[3] = vector3( +W, +H, D );

        Pos[4] = vector3( -W, +H, -D ); 
        Pos[5] = vector3( -W, -H, -D ); 
        Pos[6] = vector3( +W, -H, -D ); 
        Pos[7] = vector3( +W, +H, -D ); 

        matrix4 L2W = GetL2W();
        L2W.Transform( Corner, Pos, 8 );

        Normal[0] = v3_Cross( Corner[1]-Corner[0], Corner[2]-Corner[0] );
        Normal[0].Normalize();
        Normal[1] = v3_Cross( Corner[5]-Corner[4], Corner[1]-Corner[4] );
        Normal[1].Normalize();
        Normal[2] = v3_Cross( Corner[0]-Corner[4], Corner[3]-Corner[4] );
        Normal[2].Normalize();
        Normal[3] = v3_Cross( Corner[2]-Corner[3], Corner[6]-Corner[3] );
        Normal[3].Normalize();
        Normal[4] = v3_Cross( Corner[5]-Corner[1], Corner[6]-Corner[1] );
        Normal[4].Normalize();
        Normal[5] = v3_Cross( Corner[6]-Corner[7], Corner[5]-Corner[7] );
        Normal[5].Normalize();
    }

    //
    // Setup basic collision info arrays
    //
    collision_data                  CD;
    collision_data::low_cluster     LowCluster;
    collision_data::low_quad        LowQuad[6];
    vector3                         LowVector[24+6];

    CD.nHighClusters     = 0;
    CD.nHighIndices      = 0;
    CD.pHighCluster      = NULL;
    CD.pHighIndexToVert0 = 0;
    CD.nLowClusters      = 1;
    CD.nLowQuads         = 6;
    CD.nLowVectors       = 24+6;
    CD.pLowCluster       = &LowCluster;
    CD.pLowQuad          = LowQuad;
    CD.pLowVector        = LowVector;

    // Set up LowCluster
    LowCluster.iVectorOffset    = 0;
    LowCluster.iBone            = 0;
    LowCluster.iMesh            = 0;
    LowCluster.iQuadOffset      = 0;
    LowCluster.nPoints          = 24;
    LowCluster.nNormals         = 6;
    LowCluster.nQuads           = 6;
    LowCluster.BBox.Clear();
    LowCluster.BBox.AddVerts( Corner, 8 );

    // Setup vertex positions
    CD.pLowVector[0]  =  Corner[0];
    CD.pLowVector[1]  =  Corner[1];
    CD.pLowVector[2]  =  Corner[2];
    CD.pLowVector[3]  =  Corner[3];

    CD.pLowVector[4]  =  Corner[5]; 
    CD.pLowVector[5]  =  Corner[1]; 
    CD.pLowVector[6]  =  Corner[0]; 
    CD.pLowVector[7]  =  Corner[4]; 

    CD.pLowVector[8]  =  Corner[0]; 
    CD.pLowVector[9]  =  Corner[3]; 
    CD.pLowVector[10] =  Corner[7]; 
    CD.pLowVector[11] =  Corner[4]; 

    CD.pLowVector[12] =  Corner[2]; 
    CD.pLowVector[13] =  Corner[6]; 
    CD.pLowVector[14] =  Corner[7]; 
    CD.pLowVector[15] =  Corner[3]; 

    CD.pLowVector[16] =  Corner[5]; 
    CD.pLowVector[17] =  Corner[6]; 
    CD.pLowVector[18] =  Corner[2]; 
    CD.pLowVector[19] =  Corner[1]; 

    CD.pLowVector[20] =  Corner[6]; 
    CD.pLowVector[21] =  Corner[5]; 
    CD.pLowVector[22] =  Corner[4]; 
    CD.pLowVector[23] =  Corner[7]; 

    CD.pLowVector[24] =  Normal[0];
    CD.pLowVector[25] =  Normal[1];
    CD.pLowVector[26] =  Normal[2];
    CD.pLowVector[27] =  Normal[3];
    CD.pLowVector[28] =  Normal[4];
    CD.pLowVector[29] =  Normal[5];

    // Setup quad 1
    LowQuad[0].iP[0] = 0;
    LowQuad[0].iP[1] = 1;
    LowQuad[0].iP[2] = 2;
    LowQuad[0].iP[3] = 3;
    LowQuad[0].iN    = 0;
    LowQuad[0].Flags = 0;

    // Setup quad 2
    LowQuad[1].iP[0] = 4;
    LowQuad[1].iP[1] = 5;
    LowQuad[1].iP[2] = 6;
    LowQuad[1].iP[3] = 7;
    LowQuad[1].iN    = 1;
    LowQuad[1].Flags = 0;

    // Setup quad 3
    LowQuad[2].iP[0] = 8;
    LowQuad[2].iP[1] = 9;
    LowQuad[2].iP[2] = 10;
    LowQuad[2].iP[3] = 11;
    LowQuad[2].iN    = 2;
    LowQuad[2].Flags = 0;

    // Setup quad 4
    LowQuad[3].iP[0] = 12;
    LowQuad[3].iP[1] = 13;
    LowQuad[3].iP[2] = 14;
    LowQuad[3].iP[3] = 15;
    LowQuad[3].iN    = 3;
    LowQuad[3].Flags = 0;

    // Setup quad 5
    LowQuad[4].iP[0] = 16;
    LowQuad[4].iP[1] = 17;
    LowQuad[4].iP[2] = 18;
    LowQuad[4].iP[3] = 19;
    LowQuad[4].iN    = 4;
    LowQuad[4].Flags = 0;

    // Setup quad 6
    LowQuad[5].iP[0] = 20;
    LowQuad[5].iP[1] = 21;
    LowQuad[5].iP[2] = 22;
    LowQuad[5].iP[3] = 23;
    LowQuad[5].iN    = 5;
    LowQuad[5].Flags = 0;

    // Pass it to polycache
    matrix4 L2W;
    L2W.Identity();
    g_PolyCache.GatherCluster( CD, &L2W, ~((u64)0), GetGuid() );
}

//==========================================================================

void invisible_wall_obj::OnKill( void )
{ 
    // Update poly cache collision
    g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );

    // Dirty the Occluders to be sure to remove this.
    if( m_bIsOccluder )
        g_OccluderMgr.DirtyOccluders();

    // Call base class
    object::OnKill();
}
