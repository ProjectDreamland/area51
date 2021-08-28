//==============================================================================
//
//  fx_ShockWave.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_ShockWave.hpp"
#include "x_context.hpp"
#include "e_Draw.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_ShockWave;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void ShockWaveVertAndIndexCount( const fx_edef_shockwave& ShockWaveDef,
                                 s32& Verts, s32& Indexes )
{
    if( ShockWaveDef.IsFlat )
    {
        Verts   = (ShockWaveDef.NSegments+1) * 2;
        Indexes = (ShockWaveDef.NSegments+1) * 2;
    }
    else
    {
        Verts   = (ShockWaveDef.NSegments+1) * 4;
        Indexes = (ShockWaveDef.NSegments+1) * 8 + 4;
    }
}

//==============================================================================

s32 ShockWaveMemoryFn( const fx_element_def& ElementDef )
{
    const fx_edef_shockwave& ShockWaveDef = (fx_edef_shockwave&)(ElementDef);

    s32 Verts;
    s32 Indexes;
    s32 PerVert;
    s32 Result;

    ShockWaveVertAndIndexCount( ShockWaveDef, Verts, Indexes );

    PerVert = sizeof( vector3 ) +   // XYZ
              sizeof( vector2 ) +   // UV
              sizeof( xcolor  );    // RGBA

    Result = (Verts * PerVert) + (Indexes * sizeof(s16)) + sizeof(fx_shockwave);

    Result += 3;
    Result &= 0xFFFFFFFC;

    return( Result );
}

//==============================================================================

void fx_shockwave::Initialize( const fx_element_def* pElementDef, 
                                     f32*            pInput )
{
    // Let the base class take care of the basics.
    fx_element::Initialize( pElementDef, pInput );

    //
    // Now, on to the shock wave specific details.
    //

    s32                      i;
    const fx_edef_shockwave& ShockWaveDef = (fx_edef_shockwave&)(*m_pElementDef);

    // No UV offsets yet.
    m_OffsetU = 0.0f;
    m_OffsetV = 0.0f;

    // Determine the number of verts and indexes.
    ShockWaveVertAndIndexCount( ShockWaveDef, m_NVerts, m_NIndexes );

    if( ShockWaveDef.IsFlat )
    {
        s32 D    = ShockWaveDef.NSegments * 2;
        f32 R[2] = { 0.5f, 0.5f - (ShockWaveDef.Width / 2.0f) };

        //
        // Set the vertex, UV, color, and index pointers.
        //

        m_pVertex = (vector3*)(this      + 1);
        m_pUV     = (vector2*)(m_pVertex + m_NVerts);
        m_pColor  = (xcolor* )(m_pUV     + m_NVerts);
        m_pIndex  = (s16*    )(m_pColor  + m_NVerts);

        // XYZ

        for( i = 0; i < D; i++ )
        {
            radian Angle = (R_360 * i) / D;

            m_pVertex[i]( 0, 0, R[i&1] );
            m_pVertex[i].RotateY( Angle );
        }

        m_pVertex[D  ] = m_pVertex[0];
        m_pVertex[D+1] = m_pVertex[1];

        // UVs if NOT planar mapped.

        if( !ShockWaveDef.PlanarMap )
        {
            f32 V[2] = { 0.0f, ShockWaveDef.TileV };

            for( i = 0; i < D; i++ )
            {
                f32 U = -(ShockWaveDef.TileU * i) / D;
                m_pUV[i]( U, V[i&1] );
            }

            m_pUV[D  ]( m_pUV[0].X - ShockWaveDef.TileU, V[0] );
            m_pUV[D+1]( m_pUV[1].X - ShockWaveDef.TileU, V[1] );
        }

        // Build the index list.  (Easy!  One strip!)
        for( i = 0; i < m_NVerts; i++ )
            m_pIndex[i] = i;
    }
    else
    {   
        // We need some mid ridge values.
        f32 MR     = 0.5f - (ShockWaveDef.Center / 2.0f);
        f32 MV     = ShockWaveDef.CenterV * ShockWaveDef.TileV;
        f32 Factor = 1.0f / (ShockWaveDef.NSegments * 2.0f);

        //
        // Set the vertex, UV, color, and index pointers.
        //

        m_pVertex = (vector3*)(this      + 1);
        m_pUV     = (vector2*)(m_pVertex + m_NVerts);
        m_pColor  = (xcolor* )(m_pUV     + m_NVerts);
        m_pIndex  = (s16*    )(m_pColor  + m_NVerts);

        //
        // Intialize the lists.
        //
       
        s32 S = ShockWaveDef.NSegments+1;         // "Skip"

        // XYZ

        // VERY CAREFULLY STRUCTURED LOOP!
        for( i = 0; i < ShockWaveDef.NSegments; i++ )
        {                     
            radian A0 = (R_360 * (2*i  )) * Factor;
            radian A1 = (R_360 * (2*i+1)) * Factor;

            m_pVertex[ i + S*0 ]( 0, 0, 0.5f );
            m_pVertex[ i + S*0 ].RotateY( A0 );

            m_pVertex[ i + S*1 ]( 0, 0.5f, MR );
            m_pVertex[ i + S*1 ].RotateY( A1 );
                           
            m_pVertex[ i + S*2 ]( 0, 0, 0.5f - (ShockWaveDef.Width/2.0f) );
            m_pVertex[ i + S*2 ].RotateY( A0 );

            m_pVertex[ i + S*3 ]( 0, -0.5f, MR );
            m_pVertex[ i + S*3 ].RotateY( A1 );
        }

        // VERY CAREFULLY STRUCTURED LOOP!
        for( i = 0; i < m_NVerts; i += (ShockWaveDef.NSegments+1) ) 
        {
            m_pVertex[i+ShockWaveDef.NSegments] = m_pVertex[i];
        }

        // UVs if NOT planar mapped.

        if( !ShockWaveDef.PlanarMap )
        {
            // VERY CAREFULLY STRUCTURED LOOP!
            for( i = 0; i < ShockWaveDef.NSegments; i++ )
            {
                f32 U0 = -(ShockWaveDef.TileU * (2*i  )) * Factor;
                f32 U1 = -(ShockWaveDef.TileU * (2*i+1)) * Factor;

                m_pUV[ i + S*0 ]( U0, 0.0f );
                m_pUV[ i + S*1 ]( U1, MV );
                m_pUV[ i + S*2 ]( U0, ShockWaveDef.TileV );
                m_pUV[ i + S*3 ]( U1, MV );
            }

            // VERY CAREFULLY STRUCTURED LOOP!
            for( i = 0; i < m_NVerts; i += (ShockWaveDef.NSegments+1) ) 
            {
                m_pUV[i+ShockWaveDef.NSegments]    = m_pUV    [i];  
                m_pUV[i+ShockWaveDef.NSegments].X -= ShockWaveDef.TileU;
            }
        }

        //
        // Build the index list.  Four strips.
        //

        // If segments is 4...
        // (1st) 00 05 01 06 02 07 03 08 04 09 -1     00 01 02 03 04 05 06 07 08 09 10
        // (2nd) 10 05 11 06 12 07 13 08 14 09 -1     11 12 13 14 15 16 17 18 19 20 21
        // (3rd) 10 15 11 16 12 17 13 18 14 19 -1     22 23 24 25 26 27 28 29 30 31 32
        // (4th) 00 15 01 16 02 17 03 18 04 19 -1     33 34 35 35 37 38 39 40 41 42 43
        //       ^^ ^^                                ^^ ^^

        s32 I = ((ShockWaveDef.NSegments+1) * 2) + 1;

        for( i = 0; i < ShockWaveDef.NSegments+1; i++ )
        {
            m_pIndex[ (i*2)             ] = i + (S*2);
            m_pIndex[ (i*2)         + 1 ] = i + (S*3);
            m_pIndex[ (i*2) + (I*1)     ] = i;        
            m_pIndex[ (i*2) + (I*1) + 1 ] = i + (S*3);

            m_pIndex[ (i*2) + (I*2)     ] = i;         
            m_pIndex[ (i*2) + (I*2) + 1 ] = i + (S*1); 
            m_pIndex[ (i*2) + (I*3)     ] = i + (S*2); 
            m_pIndex[ (i*2) + (I*3) + 1 ] = i + (S*1); 
        }

        m_pIndex[ I * 1 - 1 ] = -1;
        m_pIndex[ I * 2 - 1 ] = -1;
        m_pIndex[ I * 3 - 1 ] = -1;
        m_pIndex[ I * 4 - 1 ] = -1;
    }

    // UVs if planar mapped.

    if( ShockWaveDef.PlanarMap )
    {
        for( i = 0; i < m_NVerts; i++ )
        {
            m_pUV[i].X = (m_pVertex[i].GetX() + 0.5f) * ShockWaveDef.TileU;
            m_pUV[i].Y = (m_pVertex[i].GetZ() + 0.5f) * ShockWaveDef.TileV;
        }
    }
}

//==============================================================================

void fx_shockwave::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_shockwave::AdvanceLogic" );

    //
    // Update the bounding box.
    //

    matrix4 L2W;

    L2W.Setup( m_Scale, m_Rotate, m_Translate );
    L2W = pEffect->GetL2W() * L2W;

    m_BBox.Set( vector3( -0.5f, -0.5f, -0.5f ), 
                vector3(  0.5f,  0.5f,  0.5f ) );
    m_BBox.Transform( L2W );

    //
    // Update the UVs.
    //

    const fx_edef_shockwave& ShockWaveDef = (fx_edef_shockwave&)(*m_pElementDef);

    if( (ShockWaveDef.ScrollU == 0.0f) && (ShockWaveDef.ScrollV == 0.0f) )
        return;

    f32 DeltaU = ShockWaveDef.ScrollU * DeltaTime;
    f32 DeltaV = ShockWaveDef.ScrollV * DeltaTime;
    
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

void fx_shockwave::Render( const fx_effect_base* pEffect ) const
{
    CONTEXT( "fx_shockwave::Render" );

    if( pEffect->IsSuspended() )
        return;

    const fx_edef_shockwave& ShockWaveDef = (fx_edef_shockwave&)(*m_pElementDef);
  
    matrix4 L2W;

    L2W.Setup( m_Scale, m_Rotate, m_Translate );
    L2W = pEffect->GetL2W() * L2W;

    const xbitmap* pDiffuse  = NULL;
    const xbitmap* pAlpha    = NULL;

    pEffect->GetBitmaps( ShockWaveDef.BitmapIndex, pDiffuse, pAlpha );

    u32 DrawFlags = DRAW_TEXTURED | DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_USE_ALPHA;

    if( ShockWaveDef.CombineMode > 0 )
        DrawFlags |= DRAW_BLEND_ADD;

    if( ShockWaveDef.CombineMode < 0 )
        DrawFlags |= DRAW_BLEND_SUB;

    if( !ShockWaveDef.ReadZ )
        DrawFlags |= DRAW_NO_ZBUFFER;

    if( ShockWaveDef.IsFlat )
    {
        draw_SetL2W( L2W );

        draw_Begin( DRAW_TRIANGLE_STRIPS, DrawFlags );
        draw_SetTexture( *pDiffuse );

        #ifdef TARGET_GCN
        SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
        #endif

        draw_UVs    ( m_pUV,     m_NVerts );
        draw_Colors ( m_pColor,  m_NVerts );
        draw_Verts  ( m_pVertex, m_NVerts );
        draw_Execute( m_pIndex,  m_NIndexes );

        draw_End();

#ifdef DEBUG_FX
        if( FXDebug.ElementWire )
        {              
            draw_SetL2W ( L2W );
            draw_Begin  ( DRAW_TRIANGLE_STRIPS, DRAW_WIRE_FRAME | DRAW_CULL_NONE );
            draw_Color  ( XCOLOR_BLUE );
            draw_Verts  ( m_pVertex, m_NVerts );
            draw_Execute( m_pIndex,  m_NIndexes );
            draw_End();
        }
#endif // DEBUG_FX
    }
    else
    {
        draw_SetL2W( L2W );
        draw_Begin( DRAW_TRIANGLE_STRIPS, DrawFlags );
        draw_SetTexture( *pDiffuse );

        #ifdef TARGET_GCN
        SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
        #endif

        draw_UVs    ( m_pUV,     m_NVerts );
        draw_Colors ( m_pColor,  m_NVerts );
        draw_Verts  ( m_pVertex, m_NVerts );
        draw_Execute( m_pIndex,  m_NIndexes );

        draw_End();

#ifdef DEBUG_FX
        if( FXDebug.ElementWire )
        {
            draw_SetL2W ( L2W );
            draw_Begin  ( DRAW_TRIANGLE_STRIPS, DRAW_WIRE_FRAME | DRAW_CULL_NONE );
            draw_Color  ( XCOLOR_BLUE );
            draw_Verts  ( m_pVertex, m_NVerts );
            draw_Execute( m_pIndex,  m_NIndexes );
            draw_End();
        }
#endif // DEBUG_FX
    }

#ifndef X_RETAIL
    fx_element::Render( pEffect );
#endif // X_RETAIL
}

//==============================================================================

void fx_shockwave::ApplyColor( void )
{
    const fx_edef_shockwave& ShockWaveDef = (fx_edef_shockwave&)(*m_pElementDef);

    if( ShockWaveDef.IsFlat )
    {
        vector4 FloatColor[2];
        xcolor  Color[2];
        s32     i;

        FloatColor[0].Set( MINMAX( 0.0f, m_FloatColor[0], 1.0f ),
                           MINMAX( 0.0f, m_FloatColor[1], 1.0f ),
                           MINMAX( 0.0f, m_FloatColor[2], 1.0f ),
                           MINMAX( 0.0f, m_FloatColor[3], 1.0f ) );

        FloatColor[1] = FloatColor[0];

        // Create the two combined colors.
        
        FloatColor[0] = FloatColor[0] * vector4( (f32)(m_BaseColor.R * m_Color.R * ShockWaveDef.OuterColor.R),
                                                 (f32)(m_BaseColor.G * m_Color.G * ShockWaveDef.OuterColor.G),
                                                 (f32)(m_BaseColor.B * m_Color.B * ShockWaveDef.OuterColor.B),
                                                 (f32)(m_BaseColor.A * m_Color.A * ShockWaveDef.OuterColor.A) );

        FloatColor[1] = FloatColor[1] * vector4( (f32)(m_BaseColor.R * m_Color.R * ShockWaveDef.InnerColor.R),
                                                 (f32)(m_BaseColor.G * m_Color.G * ShockWaveDef.InnerColor.G),
                                                 (f32)(m_BaseColor.B * m_Color.B * ShockWaveDef.InnerColor.B),
                                                 (f32)(m_BaseColor.A * m_Color.A * ShockWaveDef.InnerColor.A) );

        FloatColor[0] *= 1.0f/(255.0f*255.0f);
        FloatColor[1] *= 1.0f/(255.0f*255.0f);

        Color[0].R = (u8)(FloatColor[0].GetX());
        Color[0].G = (u8)(FloatColor[0].GetY());
        Color[0].B = (u8)(FloatColor[0].GetZ());
        Color[0].A = (u8)(FloatColor[0].GetW());

        Color[1].R = (u8)(FloatColor[1].GetX());
        Color[1].G = (u8)(FloatColor[1].GetY());
        Color[1].B = (u8)(FloatColor[1].GetZ());
        Color[1].A = (u8)(FloatColor[1].GetW());

        for( i = 0; i < m_NVerts; i++ )
            m_pColor[i] = Color[i&1];
    }
    else
    {
        vector4 FloatColor[3];
        xcolor  Color[3];
        s32     i;

        FloatColor[0].Set( MINMAX( 0.0f, m_FloatColor[0], 1.0f ),
                           MINMAX( 0.0f, m_FloatColor[1], 1.0f ),
                           MINMAX( 0.0f, m_FloatColor[2], 1.0f ),
                           MINMAX( 0.0f, m_FloatColor[3], 1.0f ) );

        FloatColor[1] = FloatColor[0];
        FloatColor[2] = FloatColor[0];

        // Create the three combined colors.

        FloatColor[0] = FloatColor[0] * vector4( (f32)(m_BaseColor.R * ShockWaveDef.OuterColor.R),
                                                 (f32)(m_BaseColor.G * ShockWaveDef.OuterColor.G),
                                                 (f32)(m_BaseColor.B * ShockWaveDef.OuterColor.B),
                                                 (f32)(m_BaseColor.A * ShockWaveDef.OuterColor.A) );

        FloatColor[1] = FloatColor[1] * vector4( (f32)(m_BaseColor.R * ShockWaveDef.InnerColor.R),
                                                 (f32)(m_BaseColor.G * ShockWaveDef.InnerColor.G),
                                                 (f32)(m_BaseColor.B * ShockWaveDef.InnerColor.B),
                                                 (f32)(m_BaseColor.A * ShockWaveDef.InnerColor.A) );

        FloatColor[2] = (FloatColor[0] * (1 - ShockWaveDef.CenterV)) + 
                        (FloatColor[1] * (    ShockWaveDef.CenterV));

        FloatColor[0] *= (1.0f/255.0f);
        FloatColor[1] *= (1.0f/255.0f);
        FloatColor[2] *= (1.0f/255.0f);

        Color[0].R = (u8)(FloatColor[0].GetX());
        Color[0].G = (u8)(FloatColor[0].GetY());
        Color[0].B = (u8)(FloatColor[0].GetZ());
        Color[0].A = (u8)(FloatColor[0].GetW());

        Color[1].R = (u8)(FloatColor[1].GetX());
        Color[1].G = (u8)(FloatColor[1].GetY());
        Color[1].B = (u8)(FloatColor[1].GetZ());
        Color[1].A = (u8)(FloatColor[1].GetW());

        Color[2].R = (u8)(FloatColor[2].GetX());
        Color[2].G = (u8)(FloatColor[2].GetY());
        Color[2].B = (u8)(FloatColor[2].GetZ());
        Color[2].A = (u8)(FloatColor[2].GetW());

        s32 S = ShockWaveDef.NSegments + 1;

        for( i = 0; i < S; i++ )
        {
            m_pColor[ i + S*0 ] = Color[0];
            m_pColor[ i + S*1 ] = Color[2];
            m_pColor[ i + S*2 ] = Color[1];
            m_pColor[ i + S*3 ] = Color[2];            
        }
    }
}

//==============================================================================

#undef new
REGISTER_FX_ELEMENT_CLASS( fx_shockwave, "SHOCKWAVE", ShockWaveMemoryFn );

//==============================================================================

