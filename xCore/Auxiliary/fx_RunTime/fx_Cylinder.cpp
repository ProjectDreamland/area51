//==============================================================================
//
//  fx_Cylinder.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Cylinder.hpp"
#include "x_context.hpp"
#include "e_Draw.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_Cylinder;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void CylinderVertAndIndexCount( const fx_edef_cylinder& CylinderDef,
                                s32& Verts, s32& Indexes )
{
    if( (CylinderDef.SizeTop == 0.0f) || (CylinderDef.SizeBottom == 0.0f) )
    {
        if( CylinderDef.PlanarMap )
        {
            Indexes = (CylinderDef.NSegments  ) * 3;  // 3 per tri
            Verts   = (CylinderDef.NSegments+2);      // 1/Seg + peak + 1 "wrap"
        }
        else
        {
            Indexes = (CylinderDef.NSegments  ) * 3;  // 3 per tri
            Verts   = (CylinderDef.NSegments+1) * 2;  // 2/Seg + 2 "wrap"
        }                                               
    }                                                   
    else                                                
    {                                                   
        Indexes = (CylinderDef.NSegments+1) * 2;
        Verts   = (CylinderDef.NSegments+1) * 2;
    }
}

//==============================================================================

s32 CylinderMemoryFn( const fx_element_def& ElementDef )
{
    const fx_edef_cylinder& CylinderDef = (fx_edef_cylinder&)(ElementDef);

    s32 Indexes;
    s32 Verts;
    s32 PerVert;
    s32 Result;

    //                  XYZ                 UV                 RGBA
    PerVert = sizeof( vector3 ) + sizeof( vector2 ) + sizeof( xcolor );

    CylinderVertAndIndexCount( CylinderDef, Verts, Indexes );

    Result = (sizeof(fx_cylinder)) + 
             (Indexes * sizeof(s16)) +
             (Verts * PerVert);

    Result += 3;
    Result &= 0xFFFFFFFC;

    return( Result );
}

//==============================================================================

void fx_cylinder::Initialize( const fx_element_def* pElementDef, 
                                    f32*            pInput )
{
    // Let the base class take care of the basics.
    fx_element::Initialize( pElementDef, pInput );

    //
    // Now, on to the cylinder specific details.
    //

    s32                     i;
    const fx_edef_cylinder& CylinderDef = (fx_edef_cylinder&)(*m_pElementDef);

    // Is this cylinder actually a cone?
    if     ( CylinderDef.SizeTop    == 0.0f )    m_Cone =  1;
    else if( CylinderDef.SizeBottom == 0.0f )    m_Cone = -1;
    else                                         m_Cone =  0;

    // No UV offsets yet.
    m_OffsetU = 0.0f;
    m_OffsetV = 0.0f;

    // Determine the number of verts and indexes.
    CylinderVertAndIndexCount( CylinderDef, m_NVerts, m_NIndexes );

    //
    // Set the vertex, UV, color, and index pointers.
    //

    m_pVertex = (vector3*)(this      + 1);
    m_pUV     = (vector2*)(m_pVertex + m_NVerts);
    m_pColor  = (xcolor* )(m_pUV     + m_NVerts);
    m_pIndex  = (s16*    )(m_pColor  + m_NVerts);

    //
    // Initialize the lists.
    //

    if( m_Cone && CylinderDef.PlanarMap )
    {
        // Planar mapped cone.  We can completely share the "peak" vertex.

        // Establish some helper values.

        f32 Y0, Y1;    // 0 = pinched end -- 1 = full end
        f32 V0, V1;
        f32 R;

        if( m_Cone == 1 )
        {
            Y0 =  0.5f;  
            Y1 = -0.5f;
            V0 =  0.0f;
            V1 = CylinderDef.TileV;
            R  = CylinderDef.SizeBottom * 0.5f;
        }
        else
        {
            Y0 = -0.5f;
            Y1 =  0.5f;  
            V0 = CylinderDef.TileV;
            V1 =  0.0f;
            R  = CylinderDef.SizeTop * 0.5f;
        }

        // Set up vert positions for the "full" end.  Color, too.
        for( i = 0; i < CylinderDef.NSegments; i++ )
        {
            radian Angle = (R_360 * i) / CylinderDef.NSegments;
            m_pVertex[i+1]( 0, Y1, R );
            m_pVertex[i+1].RotateY( Angle );
        }
        m_pVertex[CylinderDef.NSegments+1] = m_pVertex[1];

        // Set up the "peak" vert.
        m_pVertex[0]( 0, Y0, 0 );

        // Set up the index list.  Be careful with the facing direction!
        if( m_Cone == 1 )
        {
            for( i = 0; i < CylinderDef.NSegments; i++ )
            {
                m_pIndex[(i*3)+0] = 0;
                m_pIndex[(i*3)+1] = i+1;
                m_pIndex[(i*3)+2] = i+2;
            }
        }
        else
        {
            for( i = 0; i < CylinderDef.NSegments; i++ )
            {
                m_pIndex[(i*3)+0] = 0;
                m_pIndex[(i*3)+1] = i+2;
                m_pIndex[(i*3)+2] = i+1;
            }
        }
    }
    else
    {
        // Normal cylinder -or- non-planar mapped cone.

        // Set up the verts.

        f32 Y[2] = { 0.5f, -0.5f };
        f32 Z[2] = { 0.5f * CylinderDef.SizeTop,  
                     0.5f * CylinderDef.SizeBottom };

        if( m_Cone ==  1 )  Z[0] = 0;
        if( m_Cone == -1 )  Z[1] = 0;

        for( i = 0; i < CylinderDef.NSegments * 2; i++ )
        {
            radian Angle = (R_360 * i) / (CylinderDef.NSegments * 2);
            m_pVertex[i]( 0, Y[i&1], Z[i&1] );
            m_pVertex[i].RotateY( Angle );
        }

        m_pVertex[ CylinderDef.NSegments * 2     ] = m_pVertex[0];
        m_pVertex[ CylinderDef.NSegments * 2 + 1 ] = m_pVertex[1];

        // Set up the index list.
        if( m_Cone == 0 )
        {
            // Normal cylinder.  This is really easy.  One long strip.
            for( i = 0; i < m_NIndexes; i++ )
                m_pIndex[i] = i;
        }
        else if( m_Cone == 1 )
        {
            // Cone with peak at the top.
            for( i = 0; i < CylinderDef.NSegments; i++ )
            {
                m_pIndex[(i*3)+0] = (i*2)+3;
                m_pIndex[(i*3)+1] = (i*2)+2;
                m_pIndex[(i*3)+2] = (i*2)+1;
                // 0 : 3 2 1
                // 1 : 5 4 3
                // 2 : 7 6 5
            }
        }
        else
        {
            // Cone with peak at the bottom.
            for( i = 0; i < CylinderDef.NSegments; i++ )
            {
                m_pIndex[(i*3)+0] = (i*2)+0;
                m_pIndex[(i*3)+1] = (i*2)+1;
                m_pIndex[(i*3)+2] = (i*2)+2;
                // 0 : 0 1 2
                // 1 : 2 3 4
                // 2 : 4 5 6
            }
        }
    }

    //
    // Set up UVs.
    //

    if( CylinderDef.PlanarMap )
    {
        // The UVs are just offset from the XZs.
        for( i = 0; i < m_NVerts; i++ )
        {
            m_pUV[i].X = (m_pVertex[i].GetX() + 0.5f) * CylinderDef.TileU;
            m_pUV[i].Y = (m_pVertex[i].GetZ() + 0.5f) * CylinderDef.TileV;
        }
    }
    else
    {
        f32 V[2] = { 0.0f, CylinderDef.TileV };

        for( i = 0; i < (CylinderDef.NSegments+1) * 2; i++ )
        {
            f32 U = (CylinderDef.TileU * i) / (CylinderDef.NSegments * 2);
            m_pUV[i]( U, V[i&1] );
        }
    }
}

//==============================================================================

void fx_cylinder::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_cylinder::AdvanceLogic" );

    //
    // Update the bounding box.
    //

    const matrix4& L2W = GetL2W( pEffect );

    m_BBox.Set( vector3( -0.5f, -0.5f, -0.5f ), 
                vector3(  0.5f,  0.5f,  0.5f ) );
    m_BBox.Transform( L2W );

    //
    // Update the UVs.
    //

    const fx_edef_cylinder& CylinderDef = (fx_edef_cylinder&)(*m_pElementDef);

    if( (CylinderDef.ScrollU == 0.0f) && (CylinderDef.ScrollV == 0.0f) )
        return;

    f32 DeltaU = CylinderDef.ScrollU * DeltaTime;
    f32 DeltaV = CylinderDef.ScrollV * DeltaTime;
    
    m_OffsetU += DeltaU;
    m_OffsetV += DeltaV;

    if( m_OffsetU >  1.0f )     { m_OffsetU -= 1.0f; DeltaU -= 1.0f; }
    if( m_OffsetU < -1.0f )     { m_OffsetU += 1.0f; DeltaU += 1.0f; }
    if( m_OffsetV >  1.0f )     { m_OffsetV -= 1.0f; DeltaV -= 1.0f; }
    if( m_OffsetV < -1.0f )     { m_OffsetV += 1.0f; DeltaV += 1.0f; }

    if( DeltaU != 0.0f )
    {
        s32 i;
        for( i = 0; i < m_NVerts; i++ )
            m_pUV[i].X += DeltaU;
    }

    if( DeltaV != 0.0f )
    {
        s32 i;
        for( i = 0; i < m_NVerts; i++ )
            m_pUV[i].Y += DeltaV;
    }
}

//==============================================================================

#ifdef TARGET_GCN
void SPEmitterGCNSetup( const xbitmap* pAlpha, xbool Sub );
#endif

//==============================================================================

void fx_cylinder::Render( const fx_effect_base* pEffect ) const
{
    CONTEXT( "fx_cylinder::Render" );

    if( pEffect->IsSuspended() )
        return;

    const fx_edef_cylinder& CylinderDef = (fx_edef_cylinder&)(*m_pElementDef);
  
    const matrix4& L2W = GetL2W( pEffect );

    const xbitmap* pDiffuse  = NULL;
    const xbitmap* pAlpha    = NULL;

    pEffect->GetBitmaps( CylinderDef.BitmapIndex, pDiffuse, pAlpha );

    s32 DrawMode  = DRAW_TRIANGLE_STRIPS;
    u32 DrawFlags = DRAW_TEXTURED | DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_USE_ALPHA;

    if( CylinderDef.CombineMode > 0 )
        DrawFlags |= DRAW_BLEND_ADD;

    if( CylinderDef.CombineMode < 0 )
        DrawFlags |= DRAW_BLEND_SUB;

    if( !CylinderDef.ReadZ )
        DrawFlags |= DRAW_NO_ZBUFFER;

    if( m_Cone )
        DrawMode = DRAW_TRIANGLES;

    draw_SetL2W( L2W );

    draw_Begin( (draw_primitive)DrawMode, DrawFlags );
    draw_SetTexture( *pDiffuse );

    #ifdef TARGET_GCN
    SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
    #endif

    draw_UVs    ( m_pUV,     m_NVerts );
    draw_Colors ( m_pColor,  m_NVerts );
    draw_Verts  ( m_pVertex, m_NVerts );
    draw_Execute( m_pIndex,  m_NIndexes );

    draw_End();

    if( 0 )
    {
        draw_Begin(( draw_primitive )DrawMode, DRAW_WIRE_FRAME | DRAW_CULL_NONE );

        draw_SetL2W( L2W );

        draw_UVs    ( m_pUV,     m_NVerts );
        draw_Colors ( m_pColor,  m_NVerts );
        draw_Verts  ( m_pVertex, m_NVerts );
        draw_Execute( m_pIndex,  m_NIndexes );

        draw_End();
    }

#ifndef X_RETAIL
    fx_element::Render( pEffect );
#endif // X_RETAIL
}

//==============================================================================

void fx_cylinder::ApplyColor( void )
{
    vector4 FloatColor[2];
    xcolor  Color[2];
    s32     i;

    const fx_edef_cylinder& CylinderDef = (fx_edef_cylinder&)(*m_pElementDef);

    // Enforce the float color range.  
    // (Smooth controlled color can get out of range.)
    FloatColor[0].Set( MINMAX( 0.0f, m_FloatColor[0], 1.0f ),
                       MINMAX( 0.0f, m_FloatColor[1], 1.0f ),
                       MINMAX( 0.0f, m_FloatColor[2], 1.0f ),
                       MINMAX( 0.0f, m_FloatColor[3], 1.0f ) );

    FloatColor[1] = FloatColor[0];

    // Create the two combined colors.
    FloatColor[0] = FloatColor[0] * vector4( (f32)(m_BaseColor.R * CylinderDef.ColorTop.R),
                                             (f32)(m_BaseColor.G * CylinderDef.ColorTop.G),
                                             (f32)(m_BaseColor.B * CylinderDef.ColorTop.B),
                                             (f32)(m_BaseColor.A * CylinderDef.ColorTop.A) );

    FloatColor[1] = FloatColor[1] * vector4( (f32)(m_BaseColor.R * CylinderDef.ColorBottom.R),
                                             (f32)(m_BaseColor.G * CylinderDef.ColorBottom.G),
                                             (f32)(m_BaseColor.B * CylinderDef.ColorBottom.B),
                                             (f32)(m_BaseColor.A * CylinderDef.ColorBottom.A) );

    FloatColor[0] *= 1.0f/255.0f;
    FloatColor[1] *= 1.0f/255.0f;

    Color[0].R = (u8)(FloatColor[0].GetX());
    Color[0].G = (u8)(FloatColor[0].GetY());
    Color[0].B = (u8)(FloatColor[0].GetZ());
    Color[0].A = (u8)(FloatColor[0].GetW());

    Color[1].R = (u8)(FloatColor[1].GetX());
    Color[1].G = (u8)(FloatColor[1].GetY());
    Color[1].B = (u8)(FloatColor[1].GetZ());
    Color[1].A = (u8)(FloatColor[1].GetW());

    //
    // Install the colors.
    //
    
    if( m_Cone && CylinderDef.PlanarMap )
    {
        if( m_Cone == 1 )
        {
            m_pColor[0] = Color[0];
            for( i = 1; i < m_NVerts; i++ )
                m_pColor[i] = Color[1];
        }
        else
        {
            m_pColor[0] = Color[1];
            for( i = 1; i < m_NVerts; i++ )
                m_pColor[i] = Color[0];
        }
    }
    else
    {
        for( i = 0; i < m_NVerts; i++ )
            m_pColor[i] = Color[i&1];
    }
}

//==============================================================================

#undef new
REGISTER_FX_ELEMENT_CLASS( fx_cylinder, "CYLINDER", CylinderMemoryFn );

//==============================================================================
