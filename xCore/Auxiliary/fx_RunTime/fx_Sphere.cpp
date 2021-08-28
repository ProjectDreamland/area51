//==============================================================================
//
//  fx_Sphere.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Sphere.hpp"
#include "x_context.hpp"
#include "e_Draw.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_Sphere;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void SphereVertAndIndexCount( const fx_edef_sphere& SphereDef,
                              s32& Verts, s32& StripIndexes, s32& TriIndexes )
{
    Verts = (SphereDef.NSegmentsU+1) * (SphereDef.NSegmentsV+1);

    StripIndexes = (((SphereDef.NSegmentsU+1)*2)+1) * (SphereDef.NSegmentsV);
    TriIndexes   = 0;

    if( SphereDef.Top == 1.0f )
    {
        // We have a top cap on the sphere.
        StripIndexes -= ((SphereDef.NSegmentsU+1)*2)+1;
        TriIndexes   += SphereDef.NSegmentsU * 3;
    }

    if( SphereDef.Bottom == 0.0f )
    {
        // We have a top cap on the sphere.
        StripIndexes -= ((SphereDef.NSegmentsU+1)*2)+1;
        TriIndexes   += SphereDef.NSegmentsU * 3;
    }     
}

//==============================================================================

s32 SphereMemoryFn( const fx_element_def& ElementDef )
{
    const fx_edef_sphere& SphereDef = (fx_edef_sphere&)(ElementDef);

    s32 Verts;
    s32 SIndexes;
    s32 TIndexes;
    s32 PerVert;
    s32 Result;

    SphereVertAndIndexCount( SphereDef, Verts, SIndexes, TIndexes );

    PerVert = sizeof( vector3 ) +   // XYZ
              sizeof( vector2 ) +   // UV
              sizeof( xcolor  );    // RGBA

    Result = (Verts * PerVert) + 
             (SIndexes * sizeof(s16)) + 
             (TIndexes * sizeof(s16)) + 
             ((SphereDef.NSegmentsV+1) * sizeof(f32)) + 
             sizeof(fx_sphere);

    Result += 3;
    Result &= 0xFFFFFFFC;

    return( Result );
}

//==============================================================================

void fx_sphere::Initialize( const fx_element_def* pElementDef, 
                                  f32*            pInput )
{
    // Let the base class take care of the basics.
    fx_element::Initialize( pElementDef, pInput );

    //
    // Now, on to the sphere specific details.
    //

    const fx_edef_sphere& SphereDef = (fx_edef_sphere&)(*m_pElementDef);

    // No UV offsets yet.
    m_OffsetU = 0.0f;
    m_OffsetV = 0.0f;

    // Determine the number of verts and indexes.
    SphereVertAndIndexCount( SphereDef, m_NVerts, m_NSIndexes, m_NTIndexes );

    // Caps?
    m_CapTop    = (SphereDef.Top    == 1.0f);
    m_CapBottom = (SphereDef.Bottom == 0.0f);

    //
    // Set the color mix, vertex, UV, color, and two index pointers.
    //

    m_pVertex   = (vector3*)(this      + 1);
    m_pUV       = (vector2*)(m_pVertex + m_NVerts);
    m_pColor    = (xcolor* )(m_pUV     + m_NVerts);
    m_pSIndex   = (s16*    )(m_pColor  + m_NVerts);
    m_pTIndex   = (s16*    )(m_pSIndex + m_NSIndexes);

    //
    // XYZ
    //
    {   
        radian TopAngle = ((1.0f - SphereDef.Top)    * R_180) - R_90;
        radian BotAngle = ((1.0f - SphereDef.Bottom) * R_180) - R_90;

        if( TopAngle > BotAngle )
        {
            f32 T    = TopAngle;
            TopAngle = BotAngle;
            BotAngle = T;
        }

        if( TopAngle < -R_90 )   TopAngle = -R_90;
        if( BotAngle >  R_90 )   BotAngle =  R_90;

        radian RX, RY;
        s32    i,  j;
        s32    I = 0;

        for( i = 0; i < SphereDef.NSegmentsV+1; i++ )
        {
            RX = (((BotAngle - TopAngle) * i) / (SphereDef.NSegmentsV)) + TopAngle;

            for( j = 0; j < SphereDef.NSegmentsU; j++ )
            {
                I   = (i * (SphereDef.NSegmentsU+1)) + j;
                RY  = (R_360 * j) /  SphereDef.NSegmentsU;
                RY -= (R_360 * i) / (SphereDef.NSegmentsU * 2);

                m_pVertex[I]( 0, 0, 0.5f );
                m_pVertex[I].RotateX( RX );
                m_pVertex[I].RotateY( RY );
            }

            m_pVertex[I+1] = m_pVertex[I-j+1];
        }

        if( m_CapTop )
        {
            for( i = 0; i < SphereDef.NSegmentsU + 1; i++ )
            {
                m_pVertex[i]( 0.0f, 0.5f, 0.0f );
            }
        }

        if( m_CapBottom )
        {
            for( i = m_NVerts - (SphereDef.NSegmentsU+1); i < m_NVerts; i++ )
            {
                m_pVertex[i]( 0.0f, -0.5f, 0.0f );
            }
        }
    }

    //
    // UVs
    //
    {
        s32 i, j;
        s32 I = 0;

        if( SphereDef.PlanarMap )
        {
            for( i = 0; i < m_NVerts; i++ )
            {
                m_pUV[i].X = (m_pVertex[i].GetX() + 0.5f) * SphereDef.TileU;
                m_pUV[i].Y = (m_pVertex[i].GetZ() + 0.5f) * SphereDef.TileV;
            }
        }
        else
        {
            f32 U, V;

            for( i = 0; i < SphereDef.NSegmentsV+1; i++ )
            {
                V = (SphereDef.TileV * i) / SphereDef.NSegmentsV;

                for( j = 0; j < SphereDef.NSegmentsU; j++ )
                {
                    I  = (i * (SphereDef.NSegmentsU+1)) + j;
                    U  = (SphereDef.TileU * j) /  SphereDef.NSegmentsU;
                    U -= (SphereDef.TileU * i) / (SphereDef.NSegmentsU * 2);
                    
                    m_pUV[I]( U, V );
                }

                m_pUV[I+1].X = m_pUV[I-j+1].X + SphereDef.TileU;
                m_pUV[I+1].Y = V;
            }
        }
    }

    //
    // Indexes
    //
    {
        s32 i, j;

        s32 Start = 0;
        s32 Stop  = SphereDef.NSegmentsV;

        if( m_CapTop    )  Start += 1;
        if( m_CapBottom )  Stop  -= 1;

        s16* pWrite = m_pSIndex;

        for( i = Start; i < Stop; i++ )
        {
            for( j = 0; j < SphereDef.NSegmentsU+1; j++ )
            {
                *pWrite++ = ((i+1) * (SphereDef.NSegmentsU+1)) + j;
                *pWrite++ = ((i  ) * (SphereDef.NSegmentsU+1)) + j;
            }
            *pWrite++ = -1;
        }

        pWrite = m_pTIndex;

        if( m_CapTop )
        {
            for( i = 0; i < SphereDef.NSegmentsU; i++ )
            {
                *pWrite++ = i;
                *pWrite++ = i + SphereDef.NSegmentsU + 1;
                *pWrite++ = i + SphereDef.NSegmentsU + 2;
            }
        }

        if( m_CapBottom )
        {
            for( i = 0; i < SphereDef.NSegmentsU; i++ )
            {
                *pWrite++ = m_NVerts - ((SphereDef.NSegmentsU+1)    ) + i + 1;
                *pWrite++ = m_NVerts - ((SphereDef.NSegmentsU+1) * 2) + i + 1;
                *pWrite++ = m_NVerts - ((SphereDef.NSegmentsU+1) * 2) + i;
            }
        }
    }
}

//==============================================================================

void fx_sphere::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_sphere::AdvanceLogic" );

    //
    // Update the bounding box.
    //

    const fx_edef_sphere& SphereDef = (fx_edef_sphere&)(*m_pElementDef);

    matrix4 L2W;

    L2W.Setup( m_Scale, m_Rotate, m_Translate );
    L2W = pEffect->GetL2W() * L2W;

    m_BBox.Set( vector3( -0.5f, SphereDef.Bottom - 0.5f, -0.5f ), 
                vector3(  0.5f, SphereDef.Top    - 0.5f,  0.5f ) );
    m_BBox.Transform( L2W );

    //
    // Update the UVs.
    //

    if( (SphereDef.ScrollU == 0.0f) && (SphereDef.ScrollV == 0.0f) )
        return;

    f32 DeltaU = SphereDef.ScrollU * DeltaTime;
    f32 DeltaV = SphereDef.ScrollV * DeltaTime;
    
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
void SPEmitterGCNSetup( const xbitmap* pAlpha, xbool SubMode );
#endif

//==============================================================================

void fx_sphere::Render( const fx_effect_base* pEffect ) const
{
    CONTEXT( "fx_sphere::Render" );

    if( pEffect->IsSuspended() )
        return;

    const fx_edef_sphere& SphereDef = (fx_edef_sphere&)(*m_pElementDef);
  
    matrix4 L2W;

    L2W.Setup( m_Scale, m_Rotate, m_Translate );
    L2W = pEffect->GetL2W() * L2W;

    const xbitmap* pDiffuse  = NULL;
    const xbitmap* pAlpha    = NULL;

    pEffect->GetBitmaps( SphereDef.BitmapIndex, pDiffuse, pAlpha );

    u32 DrawFlags = DRAW_TEXTURED | DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_USE_ALPHA;

    if( SphereDef.CombineMode > 0 )
        DrawFlags |= DRAW_BLEND_ADD;

    if( SphereDef.CombineMode < 0 )
        DrawFlags |= DRAW_BLEND_SUB;

    if( !SphereDef.ReadZ )
        DrawFlags |= DRAW_NO_ZBUFFER;

    draw_SetL2W( L2W );

    draw_Begin( DRAW_TRIANGLE_STRIPS, DrawFlags );
    draw_SetTexture( *pDiffuse );

    #ifdef TARGET_GCN
    SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
    #endif

    draw_UVs    ( m_pUV,     m_NVerts );
    draw_Colors ( m_pColor,  m_NVerts );
    draw_Verts  ( m_pVertex, m_NVerts );
    draw_Execute( m_pSIndex, m_NSIndexes );

    draw_End();

    if( m_NTIndexes )
    {
        draw_SetL2W( L2W );

        draw_Begin( DRAW_TRIANGLES, DrawFlags );
        draw_SetTexture( *pDiffuse );

        #ifdef TARGET_GCN
        SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
        #endif

        draw_UVs    ( m_pUV,     m_NVerts );
        draw_Colors ( m_pColor,  m_NVerts );
        draw_Verts  ( m_pVertex, m_NVerts );
        draw_Execute( m_pTIndex, m_NTIndexes );

        draw_End();
    }

#ifdef DEBUG_FX
    if( FXDebug.ElementWire )
    {
        draw_SetL2W ( L2W );
        draw_Begin  ( DRAW_TRIANGLE_STRIPS, DRAW_WIRE_FRAME | DRAW_CULL_NONE );
        draw_Color  ( XCOLOR_BLUE );
        draw_Verts  ( m_pVertex, m_NVerts );
        draw_Execute( m_pSIndex, m_NSIndexes );
        draw_End();
    }

    fx_element::Render( pEffect );
#endif // DEBUG_FX
}

//==============================================================================

void fx_sphere::ApplyColor( void )
{
    const fx_edef_sphere& SphereDef = (fx_edef_sphere&)(*m_pElementDef);

    vector4 FloatColor[3];
    xcolor  Color[3];
    s32     i, j, I;

    FloatColor[0].Set( MINMAX( 0.0f, m_FloatColor[0], 1.0f ),
                       MINMAX( 0.0f, m_FloatColor[1], 1.0f ),
                       MINMAX( 0.0f, m_FloatColor[2], 1.0f ),
                       MINMAX( 0.0f, m_FloatColor[3], 1.0f ) );

    FloatColor[1] = FloatColor[0];

    // Create the two combined colors.

    FloatColor[0] = FloatColor[0] * vector4( (f32)(m_BaseColor.R * SphereDef.ColorTop.R),
                                             (f32)(m_BaseColor.G * SphereDef.ColorTop.G),
                                             (f32)(m_BaseColor.B * SphereDef.ColorTop.B),
                                             (f32)(m_BaseColor.A * SphereDef.ColorTop.A) );

    FloatColor[1] = FloatColor[1] * vector4( (f32)(m_BaseColor.R * SphereDef.ColorBottom.R),
                                             (f32)(m_BaseColor.G * SphereDef.ColorBottom.G),
                                             (f32)(m_BaseColor.B * SphereDef.ColorBottom.B),
                                             (f32)(m_BaseColor.A * SphereDef.ColorBottom.A) );

    FloatColor[0] *= 1.0f/255.0f;
    FloatColor[1] *= 1.0f/255.0f;

    I = 0;

    for( i = 0; i < SphereDef.NSegmentsV+1; i++ )
    {
        f32 Factor = (f32)i / SphereDef.NSegmentsV;

        FloatColor[2] = (FloatColor[0] * (1.0f - Factor)) + 
                        (FloatColor[1] * (       Factor));

        Color[2].R = (u8)(FloatColor[2].GetX());
        Color[2].G = (u8)(FloatColor[2].GetY());
        Color[2].B = (u8)(FloatColor[2].GetZ());
        Color[2].A = (u8)(FloatColor[2].GetW());

        for( j = 0; j < SphereDef.NSegmentsU+1; j++ )
        {
            m_pColor[I] = Color[2];
            I++;
        }
    }
}

//==============================================================================

#undef new
REGISTER_FX_ELEMENT_CLASS( fx_sphere, "SPHERE", SphereMemoryFn );

//==============================================================================

