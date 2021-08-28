#include "controller.hpp"
#include "element_shockwave.hpp"
#include "effect.hpp"

namespace fx_core
{

#define SHOCKWAVE_VER      1

#define ELEMENT_EXPORT_TYPE     ( ('S') | ('H'<<8) | ('O'<<16) | ('K'<<24) )


//============================================================================
// element REGISTRATION
//============================================================================
element*         s_ShockWaveFactory   ( void )  { return (element*)new element_shockwave; }
static elem_reg  s_ShockWaveKeyReg              ( "shockwave", s_ShockWaveFactory );


//============================================================================

void SwapColorEndian( xcolor& Color );

//============================================================================
// Constructor

element_shockwave::element_shockwave( )
: element()
{
    m_pType         = "ShockWave";
}

element_shockwave::~element_shockwave()
{
}

//============================================================================

void element_shockwave::Create( const char* pElementID, effect& Effect )
{
    // Assert that the ID not be NULL
    ASSERT( pElementID );

    m_ID        = pElementID;
    m_pEffect   = &Effect;

    m_BitmapName        = "";
    m_CombineMode       = COMBINEMODE_ALPHA;
    m_MapPlanar         = false;
    m_MappingTileU      = 16.0f;
    m_MappingScrollU    = 0.0f;
    m_MappingTileV      = 1.0f;
    m_MappingScrollV    = 0.0f;
    m_MappingCenterV    = 0.2f;
    m_nSegments         = 16;
    m_Width             = 0.5f;
    m_Center            = 0.2f;
    m_InnerColor        = XCOLOR_WHITE;
    m_OuterColor        = XCOLOR_WHITE;

    //m_NoiseStrengthIn   = 0.0f;
    //m_NoiseStrengthOut  = 0.0f;
    //m_NoiseFrequencyIn  = 0.333f;
    //m_NoiseFrequencyOut = 1.000f;
    //m_NoiseSpeedIn      = 0.5f;
    //m_NoiseSpeedOut     = 0.25f;
    m_IsFlat            = TRUE;

    m_pScale =       new ctrl_linear;
    m_pRotation =    new ctrl_linear;
    m_pTranslation = new ctrl_linear;
    m_pColor =       new ctrl_linear;
    m_pAlpha =       new ctrl_linear;

    // fix the color scalar
    m_pColor->SetScalar( 1.0f/255.0f );
    m_pAlpha->SetScalar( 1.0f/255.0f );

    m_pScale->SetNumFloats      (3);
    m_pRotation->SetNumFloats   (3);
    m_pTranslation->SetNumFloats(3);
    m_pColor->SetNumFloats      (3);
    m_pAlpha->SetNumFloats      (1);

    Effect.AddController( m_pScale       );
    Effect.AddController( m_pRotation    );
    Effect.AddController( m_pTranslation );
    Effect.AddController( m_pColor       );
    Effect.AddController( m_pAlpha       );

    // add myself to the element list
    Effect.AddElement( this );
}

//============================================================================

element* element_shockwave::Duplicate( void )
{
    element_shockwave* pNew = new element_shockwave( *this );

    pNew->m_BitmapName          = m_BitmapName;
    pNew->m_CombineMode         = m_CombineMode;
    pNew->m_MapPlanar           = m_MapPlanar;
    pNew->m_MappingTileU        = m_MappingTileU;
    pNew->m_MappingScrollU      = m_MappingScrollU;
    pNew->m_MappingTileV        = m_MappingTileV;
    pNew->m_MappingScrollV      = m_MappingScrollV;
    pNew->m_MappingCenterV      = m_MappingCenterV;
    pNew->m_Width               = m_Width;
    pNew->m_Center              = m_Center;
    pNew->m_InnerColor          = m_InnerColor;
    pNew->m_OuterColor          = m_OuterColor;
    
    //pNew->m_NoiseStrengthIn     = m_NoiseStrengthIn;
    //pNew->m_NoiseStrengthOut    = m_NoiseStrengthOut;
    //pNew->m_NoiseFrequencyIn    = m_NoiseFrequencyIn;
    //pNew->m_NoiseFrequencyOut   = m_NoiseFrequencyOut;
    //pNew->m_NoiseSpeedIn        = m_NoiseSpeedIn;
    //pNew->m_NoiseSpeedOut       = m_NoiseSpeedOut;
    pNew->m_IsFlat              = m_IsFlat;

    pNew->m_pScale              = (ctrl_linear*)m_pScale->CopyOf();
    pNew->m_pRotation           = (ctrl_linear*)m_pRotation->CopyOf();
    pNew->m_pTranslation        = (ctrl_linear*)m_pTranslation->CopyOf();
    pNew->m_pColor              = (ctrl_linear*)m_pColor->CopyOf();
    pNew->m_pAlpha              = (ctrl_linear*)m_pAlpha->CopyOf();

    pNew->MakeDuplicateName( m_ID );

    m_pEffect->AddController( pNew->m_pScale       );
    m_pEffect->AddController( pNew->m_pRotation    );
    m_pEffect->AddController( pNew->m_pTranslation );
    m_pEffect->AddController( pNew->m_pColor       );
    m_pEffect->AddController( pNew->m_pAlpha       );

    return (element*)pNew;
}

//============================================================================

void element_shockwave::PostCopyPtrFix( void )
{
}


//============================================================================
// Render

void element_shockwave::Render( f32 T )
{
    // Only render if we exist
    if( ExistsAtTime( T ) && (!m_Hide) )
    {
        matrix4 L2W;
        xcolor  Color;

        GetL2WAtTime  ( T, L2W   );
        GetColorAtTime( T, Color );

        // Determine blend mode
        s32 DrawBlendMode = 0;
        switch( m_CombineMode )
        {
            case COMBINEMODE_ADDITIVE:
                DrawBlendMode = DRAW_BLEND_ADD;
                break;
            case COMBINEMODE_SUBTRACTIVE:
                DrawBlendMode = DRAW_BLEND_SUB;
                break;
        }

        // Set L2W
        draw_SetL2W( L2W );

        // draw flags
        u32 DrawFlags = DRAW_TEXTURED | DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_USE_ALPHA | DrawBlendMode;

        if ( (m_MappingTileV == 1.0f) && (m_MappingScrollV == 0.0f) )   { DrawFlags |= DRAW_V_CLAMP;   }
        //if ( DrawBlendMode != 0 )                                       { DrawFlags |= DRAW_NO_ZWRITE; }
        if ( !m_ZRead )                                                 { DrawFlags |= DRAW_NO_ZBUFFER; }

        // Start drawing
        draw_Begin( DRAW_TRIANGLES, DrawFlags );

        // Setup bitmap
        if ( m_BitmapName.IsEmpty() )
            m_BitmapName = "fx_default.xbmp";

        g_pTextureMgr->ActivateBitmap( m_BitmapName );

        // Draw the shockwave
        draw_Color( Color );

        // Setup for drawing
        f32 RadiusOut           = 0.5f;
        f32 RadiusIn            = 0.5f * ( 1.0f - m_Width );
        f32 RadiusMid           = RadiusIn + ( (RadiusOut - RadiusIn) * ( 1.0f - m_Center ) );

        f32 RadiansPerSegment   = DEG_TO_RAD( 360.f / (f32)m_nSegments );

        f32 MappingPerSegmentU  = -m_MappingTileU / (f32)m_nSegments;
        f32 MappingPerSegmentV  = m_MappingTileV ;

        // UV scroll starts at LifeStart...we multiply by 0.033333333 for 30fps
        f32 MapScrollTime       = ( T - (f32)m_LifeStartFrame ) * 0.03333333f;
        f32 MapScrollU          = m_MappingScrollU * MapScrollTime;
        f32 MapScrollV          = m_MappingScrollV * MapScrollTime;

        /*****************************************************************************
        // Calculate Noise values
        f32 PhaseIn         = PI * T * 0.03333333f * m_NoiseSpeedIn;
        f32 PhaseOut        = PI * T * 0.03333333f * m_NoiseSpeedOut;

        f32 OffsetIn        = (RadiusOut - RadiusIn) * m_NoiseStrengthIn;
        f32 OffsetOut       = (RadiusOut - RadiusIn) * m_NoiseStrengthOut;

        f32 FrequencyIn     = m_nSegments * m_NoiseFrequencyIn;
        f32 FrequencyOut    = m_nSegments * m_NoiseFrequencyOut;
        ******************************************************************************/

        //----------------------------------------------------------------------------
        //    Vertex/UV Order
        //
        //     Flat       Curved
        //                                                                             
        //   0 +---+ 1          0 +---+ 1
        //      \  |\              \ / \ 
        //       \ | \            2 +---+ 3
        //        \|  \            / \ / 
        //       2 +---+ 3      4 +---+ 5
        //----------------------------------------------------------------------------

        f32 x[6];
        f32 z[6];

        f32 u[6];
        f32 v[6];

        // UV Mapping - Pre-compute V if we're not doing planar mapping
        if( !m_MapPlanar )
        {
            if( m_IsFlat )
            {
                v[0] = MapScrollV;
                v[1] = MapScrollV;
                v[2] = v[0] + MappingPerSegmentV;
                v[3] = v[0] + MappingPerSegmentV;
            }
            else
            {
                v[0]    = MapScrollV;
                v[1]    = MapScrollV;
                v[2]    = MapScrollV + ( MappingPerSegmentV * m_MappingCenterV );
                v[3]    = MapScrollV + ( MappingPerSegmentV * m_MappingCenterV );
                v[4]    = MapScrollV + MappingPerSegmentV;
                v[5]    = MapScrollV + MappingPerSegmentV;
            }
        }

        // Calculate final Outer/Inner colors (blend keyed color with Outer/Inner)
        f32 r1 = Color.R / 255.0f;
        f32 g1 = Color.G / 255.0f;
        f32 b1 = Color.B / 255.0f;
        f32 a1 = Color.A / 255.0f;
        
        f32 r2 = m_OuterColor.R / 255.0f;
        f32 g2 = m_OuterColor.G / 255.0f;
        f32 b2 = m_OuterColor.B / 255.0f;
        f32 a2 = m_OuterColor.A / 255.0f;

        f32 r3 = m_InnerColor.R / 255.0f;
        f32 g3 = m_InnerColor.G / 255.0f;
        f32 b3 = m_InnerColor.B / 255.0f;
        f32 a3 = m_InnerColor.A / 255.0f;

        xcolor OuterColor;
        xcolor InnerColor;
        xcolor MiddleColor;

        OuterColor.SetfRGBA     ( r1*r2, g1*g2, b1*b2, a1*a2 );
        InnerColor.SetfRGBA     ( r1*r3, g1*g3, b1*b3, a1*a3 );
        MiddleColor.SetfRGBA    ( ((r2+r3)/2.0f)*r1, ((g2+g3)/2.0f)*g1, ((b2+b3)/2.0f)*b1, ((a2+a3)/2.0f)*a1 );

        // Loop over all segments
        for( s32 i = 0; i < m_nSegments; i++ )   //< 1; i++ )
        {
            f32 Angle1 = RadiansPerSegment * (i - 0.5f);    // Left  Out
            f32 Angle2 = RadiansPerSegment * (i + 0.5f);    // Right Out
            f32 Angle3 = RadiansPerSegment * (i + 0.0f);    // Left  In
            f32 Angle4 = RadiansPerSegment * (i + 1.0f);    // Right In

            // Get cos & sin for radial edges of segment
            f32 Angle1_cos = x_cos( Angle1 );
            f32 Angle1_sin = x_sin( Angle1 );

            f32 Angle2_cos = x_cos( Angle2 );
            f32 Angle2_sin = x_sin( Angle2 );

            f32 Angle3_cos = x_cos( Angle3 );
            f32 Angle3_sin = x_sin( Angle3 );

            f32 Angle4_cos = x_cos( Angle4 );
            f32 Angle4_sin = x_sin( Angle4 );

            /*****************************************************************************************************
            // Calculate noise for the segment - Trailing
            f32 InRadius1   = RadiusIn + ( OffsetIn * (0.5f + x_sin( FrequencyIn * (PhaseIn + Angle1) ) * 0.5f ) );
            f32 InRadius2   = RadiusIn + ( OffsetIn * (0.5f + x_sin( FrequencyIn * (PhaseIn + Angle2) ) * 0.5f ) );

            // Calculate noise for the segment - Leading
            f32 OutRadius1  = RadiusOut - ( OffsetOut * (0.5f + x_sin( FrequencyOut * (PhaseOut + Angle1) ) * 0.5f ) );
            f32 OutRadius2  = RadiusOut - ( OffsetOut * (0.5f + x_sin( FrequencyOut * (PhaseOut + Angle2) ) * 0.5f ) );

            // Calculate noise for the segment - Center
            f32 CenterRadius1  = InRadius1 + (OutRadius1 - InRadius1) * ( 1.0f - m_Center );
            f32 CenterRadius2  = InRadius2 + (OutRadius2 - InRadius2) * ( 1.0f - m_Center );
            *****************************************************************************************************/

            if( m_IsFlat )
            {
                // Get x&z position for segment verts
                x[0] = Angle1_sin * RadiusOut;  // Left  Out
                x[1] = Angle2_sin * RadiusOut;  // Right Out
                x[2] = Angle3_sin * RadiusIn;   // Left  In
                x[3] = Angle4_sin * RadiusIn;   // Right In

                z[0] = Angle1_cos * RadiusOut;  // Left  Out
                z[1] = Angle2_cos * RadiusOut;  // Right Out
                z[2] = Angle3_cos * RadiusIn;   // Left  In
                z[3] = Angle4_cos * RadiusIn;   // Right In

                // Calculate UV Mapping
                if( m_MapPlanar )
                {
                    u[0] = MapScrollU + ( m_MappingTileU * ( x[0] + 0.5f ) );   // Left  Out
                    u[1] = MapScrollU + ( m_MappingTileU * ( x[1] + 0.5f ) );   // Right Out
                    u[2] = MapScrollU + ( m_MappingTileU * ( x[2] + 0.5f ) );   // Left  In
                    u[3] = MapScrollU + ( m_MappingTileU * ( x[3] + 0.5f ) );   // Right In

                    v[0] = MapScrollV + ( m_MappingTileV * ( z[0] + 0.5f ) );   // Left  Out
                    v[1] = MapScrollV + ( m_MappingTileV * ( z[1] + 0.5f ) );   // Right Out
                    v[2] = MapScrollV + ( m_MappingTileV * ( z[2] + 0.5f ) );   // Left  In
                    v[3] = MapScrollV + ( m_MappingTileV * ( z[3] + 0.5f ) );   // Right In
                }
                else
                {
                    u[0] = MapScrollU + ( MappingPerSegmentU * (i - 0.5f) );    // Left  Out
                    u[1] = MapScrollU + ( MappingPerSegmentU * (i + 0.5f) );    // Right Out
                    u[2] = MapScrollU + ( MappingPerSegmentU * (i + 0.0f) );    // Left  In
                    u[3] = MapScrollU + ( MappingPerSegmentU * (i + 1.0f) );    // Right In
                }

                // Draw 2 triangles for segment
                draw_UV( u[0], v[0] );   draw_Color( OuterColor );   draw_Vertex( x[0], 0.0f, z[0] ); // Left  Out
                draw_UV( u[1], v[1] );   draw_Color( OuterColor );   draw_Vertex( x[1], 0.0f, z[1] ); // Right Out
                draw_UV( u[2], v[2] );   draw_Color( InnerColor );   draw_Vertex( x[2], 0.0f, z[2] ); // Left  In

                draw_UV( u[1], v[1] );   draw_Color( OuterColor );   draw_Vertex( x[1], 0.0f, z[1] ); // Right Out
                draw_UV( u[2], v[2] );   draw_Color( InnerColor );   draw_Vertex( x[2], 0.0f, z[2] ); // Left  In
                draw_UV( u[3], v[3] );   draw_Color( InnerColor );   draw_Vertex( x[3], 0.0f, z[3] ); // Right In
            }
            else
            {
                // Get x&z position for segment verts
                x[0] = Angle1_sin * RadiusOut;  // Left  Out
                x[1] = Angle2_sin * RadiusOut;  // Right Out
                x[2] = Angle3_sin * RadiusMid;  // Left  Mid
                x[3] = Angle4_sin * RadiusMid;  // Right Mid
                x[4] = Angle1_sin * RadiusIn;   // Left  In
                x[5] = Angle2_sin * RadiusIn;   // Right In

                z[0] = Angle1_cos * RadiusOut;  // Left  Out
                z[1] = Angle2_cos * RadiusOut;  // Right Out
                z[2] = Angle3_cos * RadiusMid;  // Left  Mid
                z[3] = Angle4_cos * RadiusMid;  // Right Mid
                z[4] = Angle1_cos * RadiusIn;   // Left  In
                z[5] = Angle2_cos * RadiusIn;   // Right In

                // Calculate UV Mapping
                if( m_MapPlanar )
                {
                    u[0] = MapScrollU + ( m_MappingTileU * ( x[0] + 0.5f ) );   // Left  Out
                    u[1] = MapScrollU + ( m_MappingTileU * ( x[1] + 0.5f ) );   // Right Out
                    u[2] = MapScrollU + ( m_MappingTileU * ( x[2] + 0.5f ) );   // Left  Mid
                    u[3] = MapScrollU + ( m_MappingTileU * ( x[3] + 0.5f ) );   // Right Mid
                    u[4] = MapScrollU + ( m_MappingTileU * ( x[4] + 0.5f ) );   // Left  In
                    u[5] = MapScrollU + ( m_MappingTileU * ( x[5] + 0.5f ) );   // Right In

                    v[0] = MapScrollV + ( m_MappingTileV * ( z[0] + 0.5f ) );   // Left  Out
                    v[1] = MapScrollV + ( m_MappingTileV * ( z[1] + 0.5f ) );   // Right Out
                    v[2] = MapScrollV + ( m_MappingTileV * ( z[2] + 0.5f ) );   // Left  Mid
                    v[3] = MapScrollV + ( m_MappingTileV * ( z[3] + 0.5f ) );   // Right Mid
                    v[4] = MapScrollV + ( m_MappingTileV * ( z[4] + 0.5f ) );   // Left  In
                    v[5] = MapScrollV + ( m_MappingTileV * ( z[5] + 0.5f ) );   // Right In
                }
                else
                {
                    u[0] = MapScrollU + ( MappingPerSegmentU * (i - 0.5f) );    // Left  Out
                    u[1] = MapScrollU + ( MappingPerSegmentU * (i + 0.5f) );    // Right Out
                    u[2] = MapScrollU + ( MappingPerSegmentU * (i + 0.0f) );    // Left  Mid
                    u[3] = MapScrollU + ( MappingPerSegmentU * (i + 1.0f) );    // Right Mid
                    u[4] = MapScrollU + ( MappingPerSegmentU * (i - 0.5f) );    // Left  In
                    u[5] = MapScrollU + ( MappingPerSegmentU * (i + 0.5f) );    // Right In
                }

                // Draw top segment
                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2],  0.5f, z[2] ); // Left  Mid
                draw_UV( u[1], v[1] );    draw_Color( OuterColor  );    draw_Vertex( x[1],  0.0f, z[1] ); // Right Out
                draw_UV( u[0], v[0] );    draw_Color( OuterColor  );    draw_Vertex( x[0],  0.0f, z[0] ); // Left  Out

                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2],  0.5f, z[2] ); // Left  Mid
                draw_UV( u[3], v[3] );    draw_Color( MiddleColor );    draw_Vertex( x[3],  0.5f, z[3] ); // Right Mid
                draw_UV( u[1], v[1] );    draw_Color( OuterColor  );    draw_Vertex( x[1],  0.0f, z[1] ); // Right Out

                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2],  0.5f, z[2] ); // Left  Mid
                draw_UV( u[4], v[4] );    draw_Color( InnerColor  );    draw_Vertex( x[4],  0.0f, z[4] ); // Left  In
                draw_UV( u[5], v[5] );    draw_Color( InnerColor  );    draw_Vertex( x[5],  0.0f, z[5] ); // Right In

                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2],  0.5f, z[2] ); // Left  Mid
                draw_UV( u[5], v[5] );    draw_Color( InnerColor  );    draw_Vertex( x[5],  0.0f, z[5] ); // Right In
                draw_UV( u[3], v[3] );    draw_Color( MiddleColor );    draw_Vertex( x[3],  0.5f, z[3] ); // Right Mid

                // Draw bottom segment
                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2], -0.5f, z[2] ); // Left  Mid
                draw_UV( u[0], v[0] );    draw_Color( OuterColor  );    draw_Vertex( x[0],  0.0f, z[0] ); // Left  Out
                draw_UV( u[1], v[1] );    draw_Color( OuterColor  );    draw_Vertex( x[1],  0.0f, z[1] ); // Right Out

                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2], -0.5f, z[2] ); // Left  Mid
                draw_UV( u[1], v[1] );    draw_Color( OuterColor  );    draw_Vertex( x[1],  0.0f, z[1] ); // Right Out
                draw_UV( u[3], v[3] );    draw_Color( MiddleColor );    draw_Vertex( x[3], -0.5f, z[3] ); // Right Mid

                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2], -0.5f, z[2] ); // Left  Mid
                draw_UV( u[5], v[5] );    draw_Color( InnerColor  );    draw_Vertex( x[5],  0.0f, z[5] ); // Right In
                draw_UV( u[4], v[4] );    draw_Color( InnerColor  );    draw_Vertex( x[4],  0.0f, z[4] ); // Left  In

                draw_UV( u[2], v[2] );    draw_Color( MiddleColor );    draw_Vertex( x[2], -0.5f, z[2] ); // Left  Mid
                draw_UV( u[3], v[3] );    draw_Color( MiddleColor );    draw_Vertex( x[3], -0.5f, z[3] ); // Right Mid
                draw_UV( u[5], v[5] );    draw_Color( InnerColor  );    draw_Vertex( x[5],  0.0f, z[5] ); // Right In
            }
        }

        // End drawing
        draw_End();

        // Render element bbox
        RenderBBox( T );

        // Reset L2W
        draw_ClearL2W();
    }

    // Render the translation path of the object
    RenderTrajectory();
}

//============================================================================
// Show properties

xbool element_shockwave::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    xcolor  HeaderColor         ( 119, 128, 144 );
    //xcolor  NoiseHeaderColor    ( 151, 160, 176 );
    xcolor  ItemColor           ( 176, 176, 176 );

    // give the base class a chance to deal with the basics
    if ( element::GetProperty( Idx, T, UIColor, Name, Value, IsDisabled, Type ) == FALSE )
    {
        // return our specific data here
        switch( Idx )
        {
            case 26:
                //============================================================================
                Name.Format( "Material\\Draw Mode" );
                Value = CombineMode_ToString( m_CombineMode );
                Type        = PROP_COMBINEMODE;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 27:
                //============================================================================
                Name.Format( "Material\\Bitmap" );
                Value = m_BitmapName;
                Type        = PROP_FILENAME;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 28:
                //============================================================================
                Name.Format( "Material\\Planar Map" );
                Value.Format( m_MapPlanar ? "true" : "false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 29:
                //============================================================================
                Name.Format( "Material\\Tile U" );
                Value.Format( "%g", m_MappingTileU );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 30:
                //============================================================================
                Name.Format( "Material\\Tile V" );
                Value.Format( "%g", m_MappingTileV );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 31:
                //============================================================================
                Name.Format( "Material\\Scroll U" );
                Value.Format( "%g", m_MappingScrollU );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 32:
                //============================================================================
                Name.Format( "Material\\Scroll V" );
                Value.Format( "%g", m_MappingScrollV );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 33:
                //============================================================================
                Name.Format( "Material\\Center V" );
                Value.Format( "%g", m_MappingCenterV );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 34:
                //============================================================================
                Name.Format( "Wave" );
                Value.Format( "" );
                Type        = PROP_HEADER;
                UIColor     = HeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 35:
                //============================================================================
                Name.Format( "Wave\\Is Flat" );
                Value.Format( m_IsFlat ? "true" : "false" );
                Type        = PROP_BOOL;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 36:
                //============================================================================
                Name.Format( "Wave\\Segments" );
                Value.Format( "%d", m_nSegments );
                Type        = PROP_INT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 37:
                //============================================================================
                Name.Format( "Wave\\Width" );
                Value.Format( "%g", m_Width );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 38:
                //============================================================================
                Name.Format( "Wave\\Center" );
                Value.Format( "%g", m_Center );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 39:
                //============================================================================
                Name.Format( "Wave\\Inner Color" );
                Value.Format( "%d, %d, %d, %d", m_InnerColor.R, m_InnerColor.G, m_InnerColor.B, m_InnerColor.A );
                Type        = PROP_COLOR;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 40:
                //============================================================================
                Name.Format( "Wave\\Outer Color" );
                Value.Format( "%d, %d, %d, %d", m_OuterColor.R, m_OuterColor.G, m_OuterColor.B, m_OuterColor.A );
                Type        = PROP_COLOR;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;

                /*
            case 41:
                //============================================================================
                Name.Format( "Wave\\Inner Noise" );
                Value.Clear();
                Type        = PROP_HEADER;
                UIColor     = NoiseHeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 42:
                //============================================================================
                Name.Format( "Wave\\Inner Noise\\Strength" );
                Value.Format( "%g", m_NoiseStrengthIn );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 43:
                //============================================================================
                Name.Format( "Wave\\Inner Noise\\Frequency" );
                Value.Format( "%g", m_NoiseFrequencyIn );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 44:
                //============================================================================
                Name.Format( "Wave\\Inner Noise\\Speed" );
                Value.Format( "%g", m_NoiseSpeedIn );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 45:
                //============================================================================
                Name.Format( "Wave\\Outer Noise" );
                Value.Clear();
                Type        = PROP_HEADER;
                UIColor     = NoiseHeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 46:
                //============================================================================
                Name.Format( "Wave\\Outer Noise\\Strength" );
                Value.Format( "%g", m_NoiseStrengthOut );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 47:
                //============================================================================
                Name.Format( "Wave\\Outer Noise\\Frequency" );
                Value.Format( "%g", m_NoiseFrequencyOut );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 48:
                //============================================================================
                Name.Format( "Wave\\Outer Noise\\Speed" );
                Value.Format( "%g", m_NoiseSpeedOut );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
                */
            default:
                return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
   
    return FALSE;
}


//============================================================================
// Update data fed back from the property bar


xbool element_shockwave::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
{
    xbool Dirty = FALSE;

    // Strip out the header information from the Field, so we can check the property strings
    s32     PropLength  = Field.GetLength() - ( 17 + m_ID.GetLength() );
    xstring Property    = Field.Right( PropLength );

    //-------------------------------------------------------------------------
    // Over-ride the properties we want to
    //-------------------------------------------------------------------------

    if( x_strcmp( Property, "Material\\Draw Mode" ) == 0 )
    {
        m_CombineMode = CombineMode_FromString( Value );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Bitmap" ) == 0 )
    {
        g_pTextureMgr->DeActivateBitmap( m_BitmapName );
        Value.IsEmpty() ? m_BitmapName = "fx_default.xbmp" : m_BitmapName = Value;
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Planar Map" ) == 0 )
    {
        m_MapPlanar = (x_strcmp( Value, "true" ) == 0) ? TRUE : FALSE;
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Tile U" ) == 0 )
    {
        m_MappingTileU = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Scroll U" ) == 0 )
    {
        m_MappingScrollU = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Tile V" ) == 0 )
    {
        m_MappingTileV = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Scroll V" ) == 0 )
    {
        m_MappingScrollV = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Center V" ) == 0 )
    {
        m_MappingCenterV = (f32)x_atof(Value);
        m_MappingCenterV = MIN( m_MappingCenterV , 1.0f );
        m_MappingCenterV = MAX( m_MappingCenterV , 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Is Flat" ) == 0 )
    {
        m_IsFlat = (x_strcmp( Value, "true" ) == 0) ? TRUE : FALSE;
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Segments" ) == 0 )
    {
        m_nSegments = x_atoi(Value);
        m_nSegments = MIN( m_nSegments, 200 );
        m_nSegments = MAX( m_nSegments, 3 );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Width" ) == 0 )
    {
        m_Width = (f32)x_atof(Value);
        m_Width = MIN( m_Width, 1.0f );
        m_Width = MAX( m_Width, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Center" ) == 0 )
    {
        m_Center = (f32)x_atof(Value);
        m_Center = MIN( m_Center, 1.0f );
        m_Center = MAX( m_Center, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Inner Color" ) == 0 )
    {
        String2Color( Value, m_InnerColor );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Outer Color" ) == 0 )
    {
        String2Color( Value, m_OuterColor );
        Dirty = TRUE;
    }

    /*
    else if( x_strcmp( Property, "Wave\\Inner Noise\\Strength" ) == 0 )
    {
        m_NoiseStrengthIn = (f32)x_atof(Value);
        m_NoiseStrengthIn = MIN( m_NoiseStrengthIn, 1.0f );
        m_NoiseStrengthIn = MAX( m_NoiseStrengthIn, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Outer Noise\\Strength" ) == 0 )
    {
        m_NoiseStrengthOut = (f32)x_atof(Value);
        m_NoiseStrengthOut = MIN( m_NoiseStrengthOut, 1.0f );
        m_NoiseStrengthOut = MAX( m_NoiseStrengthOut, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Inner Noise\\Frequency" ) == 0 )
    {
        m_NoiseFrequencyIn = (f32)x_atof(Value);
        m_NoiseFrequencyIn = MIN( m_NoiseFrequencyIn, 1.0f );
        m_NoiseFrequencyIn = MAX( m_NoiseFrequencyIn, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Outer Noise\\Frequency" ) == 0 )
    {
        m_NoiseFrequencyOut = (f32)x_atof(Value);
        m_NoiseFrequencyOut = MIN( m_NoiseFrequencyOut, 1.0f );
        m_NoiseFrequencyOut = MAX( m_NoiseFrequencyOut, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Inner Noise\\Speed" ) == 0 )
    {
        m_NoiseSpeedIn = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Wave\\Outer Noise\\Speed" ) == 0 )
    {
        m_NoiseSpeedOut = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    */

    //-------------------------------------------------------------------------
    // Let the base class handle any properties we didn't over-ride
    //-------------------------------------------------------------------------

    if( Dirty == FALSE )
    {
        Dirty = element::OnPropertyChanged( T, Field, Value );
    }

    return Dirty;
}


//============================================================================
// Save data from an element

void element_shockwave::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "SHOCKWAVE", "Type of element" );

        // add the base stuff
        element::Save( Igf );

        // add our specific stuff
        Igf.AddS32      ( "SHOCKWAVE_VER",      SHOCKWAVE_VER,  "ShockWave Element Version Number" );
        Igf.AddString   ( "Bitmap",             m_BitmapName        );
        Igf.AddS32      ( "CombineMode",        m_CombineMode       );
        Igf.AddBool     ( "MapPlanar",          m_MapPlanar         );
        Igf.AddF32      ( "MappingTileU",       m_MappingTileU      );
        Igf.AddF32      ( "MappingScrollU",     m_MappingScrollU    );
        Igf.AddF32      ( "MappingTileV",       m_MappingTileV      );
        Igf.AddF32      ( "MappingScrollV",     m_MappingScrollV    );
        Igf.AddF32      ( "MappingCenterV",     m_MappingCenterV    );
        Igf.AddBool     ( "IsFlat",             m_IsFlat            );
        Igf.AddS32      ( "nSegments",          m_nSegments         );
        Igf.AddF32      ( "Width",              m_Width             );
        Igf.AddF32      ( "Center",             m_Center            );
        Igf.AddColor    ( "InnerColor",         m_InnerColor        );
        Igf.AddColor    ( "OuterColor",         m_OuterColor        );

        //Igf.AddF32      ( "NoiseStrengthIn",    m_NoiseStrengthIn   );
        //Igf.AddF32      ( "NoiseStrengthOut",   m_NoiseStrengthOut  );
        //Igf.AddF32      ( "NoiseFrequencyIn",   m_NoiseFrequencyIn  );
        //Igf.AddF32      ( "NoiseFrequencyOut",  m_NoiseFrequencyOut );
        //Igf.AddF32      ( "NoiseSpeedIn",       m_NoiseSpeedIn      );
        //Igf.AddF32      ( "NoiseSpeedOut",      m_NoiseSpeedOut     );
    }
    
    Igf.ExitGroup();
}

//============================================================================
void element_shockwave::Load( igfmgr& Igf )
{
    s32 ShockWaveVer = 0;

    // base stuff first
    element::Load( Igf );

    // This is for backwards compatibility
    if ( Igf.Find( "SHOCKWAVE_VER" ) )
        ShockWaveVer = Igf.GetS32();

    // get our specific stuff
    m_BitmapName        = Igf.GetString ( "Bitmap"            );
    m_CombineMode       = Igf.GetS32    ( "CombineMode"       );
    m_MapPlanar         = Igf.GetBool   ( "MapPlanar"         );
    m_MappingTileU      = Igf.GetF32    ( "MappingTileU"      );
    m_MappingScrollU    = Igf.GetF32    ( "MappingScrollU"    );
    m_MappingTileV      = Igf.GetF32    ( "MappingTileV"      );
    m_MappingScrollV    = Igf.GetF32    ( "MappingScrollV"    );
    m_MappingCenterV    = Igf.GetF32    ( "MappingCenterV"    );
    m_IsFlat            = Igf.GetBool   ( "IsFlat"            );
    m_nSegments         = Igf.GetS32    ( "nSegments"         );
    m_Width             = Igf.GetF32    ( "Width"             );
    m_Center            = Igf.GetF32    ( "Center"            );
    m_InnerColor        = Igf.GetColor  ( "InnerColor"        );
    m_OuterColor        = Igf.GetColor  ( "OuterColor"        );
    
    //m_NoiseStrengthIn   = Igf.GetF32    ( "NoiseStrengthIn"   );
    //m_NoiseStrengthOut  = Igf.GetF32    ( "NoiseStrengthOut"  );
    //m_NoiseFrequencyIn  = Igf.GetF32    ( "NoiseFrequencyIn"  );
    //m_NoiseFrequencyOut = Igf.GetF32    ( "NoiseFrequencyOut" );
    //m_NoiseSpeedIn      = Igf.GetF32    ( "NoiseSpeedIn"      );
    //m_NoiseSpeedOut     = Igf.GetF32    ( "NoiseSpeedOut"     );
}

//============================================================================
// Export data
void element_shockwave::ExportData( export::fx_elementhdr& ElemHdr, 
                                 xstring& Type,
                                 xbytestream& Stream, 
                                 s32 ExportTarget )
{
    
    Type.Format( "SHOCKWAVE" );

    // Call base
    xbytestream     TempStream;
    TempStream.Clear();
    element::ExportData( ElemHdr, Type, TempStream, ExportTarget );
    ElemHdr.TypeIndex = ELEMENT_EXPORT_TYPE;

    switch( m_CombineMode )
    {
        case COMBINEMODE_ADDITIVE:      ElemHdr.CombineMode =  1;    break;
        case COMBINEMODE_SUBTRACTIVE:   ElemHdr.CombineMode = -1;    break;
        case COMBINEMODE_ALPHA:         ElemHdr.CombineMode =  0;    break;
        default:                        ElemHdr.CombineMode =  0;    break;
    }
    
    // Insert the extra info
    s32 BitmapIndex = g_pTextureMgr->GetTextureIndex( m_BitmapName );
    TempStream.Insert( sizeof(export::fx_elementhdr), (const u8*)&BitmapIndex, sizeof(s32) );

    element_shockwave_export ExportData;

    ExportData.m_Center         = m_Center;
    ExportData.m_InnerColor     = m_InnerColor;
    ExportData.m_IsFlat         = m_IsFlat;
    ExportData.m_MappingCenterV = m_MappingCenterV;
    ExportData.m_MappingScrollU = m_MappingScrollU;
    ExportData.m_MappingScrollV = m_MappingScrollV;
    ExportData.m_MappingTileU   = m_MappingTileU;
    ExportData.m_MappingTileV   = m_MappingTileV;
    ExportData.m_MapPlanar      = m_MapPlanar;
    ExportData.m_nSegments      = m_nSegments;
    ExportData.m_OuterColor     = m_OuterColor;
    ExportData.m_Width          = m_Width;

    if( ExportTarget == EXPORT_TARGET_GCN )
    {
        // Must reverse the Endian of the colors.
        SwapColorEndian( ExportData.m_InnerColor );
        SwapColorEndian( ExportData.m_OuterColor );
    }

    TempStream.Append( (byte*)&ExportData, sizeof(element_shockwave_export) );

    // Adjust the size
    ElemHdr.TotalSize = TempStream.GetLength() / sizeof(s32);
    TempStream.Replace( 0, (const u8*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // Append the new data onto our stream
    Stream.Append( TempStream );
}

//============================================================================
// Flag the texture as being used
void element_shockwave::FlagExportTextures( void )
{
    g_pTextureMgr->MarkAsUsed(m_BitmapName);
}

//============================================================================
// Activate the texture
void element_shockwave::ActivateTextures( void )
{
    g_pTextureMgr->ActivateBitmap(m_BitmapName);
}

} // namespace fx_core
