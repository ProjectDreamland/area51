#include "controller.hpp"
#include "element_sphere.hpp"
#include "effect.hpp"

namespace fx_core
{

#define SPHERE_VER      1
#define ELEMENT_EXPORT_TYPE     ( 'S' | ('P'<<8) | ('H'<<16) | ('R'<<24) )


//============================================================================
// element REGISTRATION
//============================================================================
element*         s_SphereFactory     ( void )   { return (element*)new element_sphere; }
static elem_reg  s_SphereKeyReg                 ( "sphere", s_SphereFactory );


//============================================================================

void SwapColorEndian( xcolor& Color );

//============================================================================
// Constructor

element_sphere::element_sphere( )
: element()
{
    m_pType         = "Sphere";
}

element_sphere::~element_sphere()
{
}

//============================================================================

void element_sphere::Create( const char* pElementID, effect& Effect )
{
    // Assert that the ID not be NULL
    ASSERT( pElementID );

    m_ID        = pElementID;
    m_pEffect   = &Effect;

    m_BitmapName        = "";
    m_CombineMode       = COMBINEMODE_ALPHA;
    m_MapPlanar         = false;
    m_MappingTileU      = 1.0f;
    m_MappingTileV      = 1.0f;
    m_MappingScrollU    = 0.0f;
    m_MappingScrollV    = 0.0f;
    m_nSegmentsU        = 8;
    m_nSegmentsV        = 8;
    m_TopPos            = 1.0f;
    m_BottomPos         = 0.0f;
    m_ColorTop          = XCOLOR_WHITE;
    m_ColorBottom       = XCOLOR_WHITE;

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

element* element_sphere::Duplicate( void )
{
    element_sphere* pNew = new element_sphere( *this );

    pNew->m_BitmapName      = m_BitmapName;
    pNew->m_CombineMode     = m_CombineMode;
    pNew->m_MapPlanar       = m_MapPlanar;
    pNew->m_MappingTileU    = m_MappingTileU;
    pNew->m_MappingTileV    = m_MappingTileV;
    pNew->m_MappingScrollU  = m_MappingScrollU;
    pNew->m_MappingScrollV  = m_MappingScrollV;
    pNew->m_nSegmentsU      = m_nSegmentsU;
    pNew->m_nSegmentsV      = m_nSegmentsV;
    pNew->m_TopPos          = m_TopPos;
    pNew->m_BottomPos       = m_BottomPos;
    pNew->m_ColorTop        = m_ColorTop;
    pNew->m_ColorBottom     = m_ColorBottom;

    pNew->m_pScale =        (ctrl_linear*)m_pScale->CopyOf();
    pNew->m_pRotation =     (ctrl_linear*)m_pRotation->CopyOf();
    pNew->m_pTranslation =  (ctrl_linear*)m_pTranslation->CopyOf();
    pNew->m_pColor =        (ctrl_linear*)m_pColor->CopyOf();
    pNew->m_pAlpha =        (ctrl_linear*)m_pAlpha->CopyOf();

    pNew->MakeDuplicateName( m_ID );

    m_pEffect->AddController( pNew->m_pScale       );
    m_pEffect->AddController( pNew->m_pRotation    );
    m_pEffect->AddController( pNew->m_pTranslation );
    m_pEffect->AddController( pNew->m_pColor       );
    m_pEffect->AddController( pNew->m_pAlpha       );


    return (element*)pNew;
}

//============================================================================

void element_sphere::PostCopyPtrFix( void )
{
}


//============================================================================
// Render

void element_sphere::Render( f32 T )
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
        u32 DrawFlags = DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_CULL_NONE | DRAW_NO_ZWRITE | DrawBlendMode;

        if ( (m_MappingTileV == 1.0f) && (m_MappingScrollV == 0.0f) )   { DrawFlags |= DRAW_V_CLAMP; }
        //if ( DrawBlendMode != 0 )                                       { DrawFlags |= DRAW_NO_ZWRITE; }
        if ( !m_ZRead )                                                 { DrawFlags |= DRAW_NO_ZBUFFER; }

        // Start drawing
        draw_Begin( DRAW_TRIANGLES, DrawFlags );

        // Setup bitmap
        if ( m_BitmapName.IsEmpty() )
            m_BitmapName = "fx_default.xbmp";

        g_pTextureMgr->ActivateBitmap( m_BitmapName );

        // Draw the sphere
        draw_Color( Color );

        // Setup for drawing
        f32 RadiansPerSegmentU  = DEG_TO_RAD( 360.f / (f32)m_nSegmentsU );
        f32 RadiansPerSegmentV  = DEG_TO_RAD( 180.f / (f32)m_nSegmentsV ) * ( m_TopPos - m_BottomPos );
        f32 StartAngle = ( 1.0f - m_TopPos ) * DEG_TO_RAD(180);
        f32 Radius = 0.5f;

        f32 MappingPerSegmentU  = m_MappingTileU / (f32)m_nSegmentsU;
        f32 MappingPerSegmentV  = m_MappingTileV / (f32)m_nSegmentsV;

        // UV scroll starts at LifeStart...we multiply by 0.033333333 for 30fps
        f32 MapScrollTime       = ( T - (f32)m_LifeStartFrame ) * 0.03333333f;
        f32 MapScrollU          = m_MappingScrollU * MapScrollTime;
        f32 MapScrollV          = m_MappingScrollV * MapScrollTime;

        f32 x[4];
        f32 y[2];
        f32 z[4];

        f32 u1, u2, u3, u4;
        f32 v1, v2, v3, v4;

        f32 uAngle1, uAngle2, uAngle3, uAngle4;
        f32 vAngle1, vAngle2;

        f32 xTerm1, xTerm2;
        f32 yTerm1, yTerm2;
        f32 zTerm1, zTerm2;

        f32 xTerm3, xTerm4;
        f32 zTerm3, zTerm4;

        // calculate final top/bottom colors (blend keyed color with top/bottom)
        f32 r1, g1, b1, a1;
        f32 r2, g2, b2, a2;
        f32 r3, g3, b3, a3;

        r1 = Color.R / 255.0f;
        g1 = Color.G / 255.0f;
        b1 = Color.B / 255.0f;
        a1 = Color.A / 255.0f;
        
        r2 = m_ColorTop.R / 255.0f;
        g2 = m_ColorTop.G / 255.0f;
        b2 = m_ColorTop.B / 255.0f;
        a2 = m_ColorTop.A / 255.0f;

        r3 = m_ColorBottom.R / 255.0f;
        g3 = m_ColorBottom.G / 255.0f;
        b3 = m_ColorBottom.B / 255.0f;
        a3 = m_ColorBottom.A / 255.0f;

        vector4 Dif;

        vector4 fColorTop;
        vector4 fColorBottom;

        vector4 fInterpTop;
        vector4 fInterpBottom;

        f32     ColorScale1;
        f32     ColorScale2;

        xcolor  VBlend1;
        xcolor  VBlend2;

        xcolor  NewTopColor;
        xcolor  NewBottomColor;

        NewTopColor.SetfRGBA    ( r1*r2, g1*g2, b1*b2, a1*a2 );
        NewBottomColor.SetfRGBA ( r1*r3, g1*g3, b1*b3, a1*a3 );

        // U Segments
        for( s32 i = 0; i < m_nSegmentsU; i++ )
        {
            // Get Mapping U
            u1 = MapScrollU + ( MappingPerSegmentU * ( i + 0.0f ) );
            u2 = MapScrollU + ( MappingPerSegmentU * ( i + 1.0f ) );
            u3 = MapScrollU + ( MappingPerSegmentU * ( i - 0.5f ) );
            u4 = MapScrollU + ( MappingPerSegmentU * ( i + 0.5f ) );

            uAngle1 = RadiansPerSegmentU * ( i + 0.0f );
            uAngle2 = RadiansPerSegmentU * ( i + 1.0f );
            uAngle3 = RadiansPerSegmentU * ( i - 0.5f );
            uAngle4 = RadiansPerSegmentU * ( i + 0.5f );

            xTerm1  = x_sin( uAngle1 ) * Radius;
            xTerm2  = x_sin( uAngle2 ) * Radius;
            xTerm3  = x_sin( uAngle3 ) * Radius;
            xTerm4  = x_sin( uAngle4 ) * Radius;

            zTerm1  = x_cos( uAngle1 ) * Radius;
            zTerm2  = x_cos( uAngle2 ) * Radius;
            zTerm3  = x_cos( uAngle3 ) * Radius;
            zTerm4  = x_cos( uAngle4 ) * Radius;

            // V Segments
            for( s32 j = 0; j < m_nSegmentsV; j++ )
            {
                fColorTop.Set( NewTopColor.R / 255.0f,
                               NewTopColor.G / 255.0f,
                               NewTopColor.B / 255.0f,
                               NewTopColor.A / 255.0f );

                fColorBottom.Set( NewBottomColor.R / 255.0f,
                                  NewBottomColor.G / 255.0f,
                                  NewBottomColor.B / 255.0f,
                                  NewBottomColor.A / 255.0f );

                ColorScale1 = (f32)j / m_nSegmentsV;
                ColorScale2 = (f32)(j+1) / m_nSegmentsV;

                Dif             = fColorBottom - fColorTop;

                fInterpTop      = Dif * ColorScale1;
                fInterpBottom   = Dif * ColorScale2;

                fInterpTop      += fColorTop;
                fInterpBottom   += fColorTop;

                // finally, stuff converted values into colors
                VBlend1.SetfRGBA( fInterpTop.GetX(),    fInterpTop.GetY(),    fInterpTop.GetZ(),    fInterpTop.GetW()    );
                VBlend2.SetfRGBA( fInterpBottom.GetX(), fInterpBottom.GetY(), fInterpBottom.GetZ(), fInterpBottom.GetW() );
                
                // Get Mapping V
                v1 = MapScrollV + ( MappingPerSegmentV *   j   );
                v2 = MapScrollV + ( MappingPerSegmentV *   j   );
                v3 = MapScrollV + ( MappingPerSegmentV * (j+1) );
                v4 = MapScrollV + ( MappingPerSegmentV * (j+1) );

                vAngle1 = StartAngle + RadiansPerSegmentV *  j;
                vAngle2 = StartAngle + RadiansPerSegmentV * (j+1);

                yTerm1  = x_sin( vAngle1 );
                yTerm2  = x_sin( vAngle2 );

                y[0]    = x_cos( vAngle1 ) * Radius;
                y[1]    = x_cos( vAngle2 ) * Radius;

                if ( j % 2 )
                {
                    x[0] = xTerm3 * yTerm1; // Top    Left
                    x[1] = xTerm4 * yTerm1; // Top    Right
                    x[2] = xTerm1 * yTerm2; // Bottom Left
                    x[3] = xTerm2 * yTerm2; // Bottom Right

                    z[0] = zTerm3 * yTerm1; // Top    Left
                    z[1] = zTerm4 * yTerm1; // Top    Right
                    z[2] = zTerm1 * yTerm2; // Bottom Left
                    z[3] = zTerm2 * yTerm2; // Bottom Right

                    // Calculate UV Mapping
                    if( m_MapPlanar )
                    {
                        u1  = MapScrollU + ( m_MappingTileU * ( x[2] + 0.5f ) );
                        u2  = MapScrollU + ( m_MappingTileU * ( x[3] + 0.5f ) );
                        u3  = MapScrollU + ( m_MappingTileU * ( x[0] + 0.5f ) );
                        u4  = MapScrollU + ( m_MappingTileU * ( x[1] + 0.5f ) );

                        v1  = MapScrollV + ( m_MappingTileV * ( z[0] + 0.5f ) );
                        v2  = MapScrollV + ( m_MappingTileV * ( z[1] + 0.5f ) );
                        v3  = MapScrollV + ( m_MappingTileV * ( z[2] + 0.5f ) );
                        v4  = MapScrollV + ( m_MappingTileV * ( z[3] + 0.5f ) );
                    }

                    // Draw UV Quad
                    draw_UV( u3, v1 );  draw_Color( VBlend1 );  draw_Vertex( x[0], y[0], z[0] ); // Top    Left
                    draw_UV( u1, v3 );  draw_Color( VBlend2 );  draw_Vertex( x[2], y[1], z[2] ); // Bottom Left
                    draw_UV( u4, v2 );  draw_Color( VBlend1 );  draw_Vertex( x[1], y[0], z[1] ); // Top    Right

                    draw_UV( u4, v2 );  draw_Color( VBlend1 );  draw_Vertex( x[1], y[0], z[1] ); // Top    Right
                    draw_UV( u1, v3 );  draw_Color( VBlend2 );  draw_Vertex( x[2], y[1], z[2] ); // Bottom Left
                    draw_UV( u2, v4 );  draw_Color( VBlend2 );  draw_Vertex( x[3], y[1], z[3] ); // Bottom Right

                }
                else
                {
                    x[0] = xTerm1 * yTerm1; // Top    Left
                    x[1] = xTerm2 * yTerm1; // Top    Right
                    x[2] = xTerm3 * yTerm2; // Bottom Left
                    x[3] = xTerm4 * yTerm2; // Bottom Right
                    
                    z[0] = zTerm1 * yTerm1; // Top    Left
                    z[1] = zTerm2 * yTerm1; // Top    Right
                    z[2] = zTerm3 * yTerm2; // Bottom Left
                    z[3] = zTerm4 * yTerm2; // Bottom Right

                    // Calculate UV Mapping
                    if( m_MapPlanar )
                    {
                        u1  = MapScrollU + ( m_MappingTileU * ( x[0] + 0.5f ) );
                        u2  = MapScrollU + ( m_MappingTileU * ( x[1] + 0.5f ) );
                        u3  = MapScrollU + ( m_MappingTileU * ( x[2] + 0.5f ) );
                        u4  = MapScrollU + ( m_MappingTileU * ( x[3] + 0.5f ) );

                        v1  = MapScrollV + ( m_MappingTileV * ( z[0] + 0.5f ) );
                        v2  = MapScrollV + ( m_MappingTileV * ( z[1] + 0.5f ) );
                        v3  = MapScrollV + ( m_MappingTileV * ( z[2] + 0.5f ) );
                        v4  = MapScrollV + ( m_MappingTileV * ( z[3] + 0.5f ) );
                    }

                    // Draw UV Quad
                    draw_UV( u1, v1 );  draw_Color( VBlend1 );  draw_Vertex( x[0], y[0], z[0] ); // Top    Left
                    draw_UV( u3, v3 );  draw_Color( VBlend2 );  draw_Vertex( x[2], y[1], z[2] ); // Bottom Left
                    draw_UV( u4, v4 );  draw_Color( VBlend2 );  draw_Vertex( x[3], y[1], z[3] ); // Bottom Right
                    
                    draw_UV( u1, v1 );  draw_Color( VBlend1 );  draw_Vertex( x[0], y[0], z[0] ); // Top    Left
                    draw_UV( u4, v4 );  draw_Color( VBlend2 );  draw_Vertex( x[3], y[1], z[3] ); // Bottom Right
                    draw_UV( u2, v2 );  draw_Color( VBlend1 );  draw_Vertex( x[1], y[0], z[1] ); // Top    Right
                }
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

xbool element_sphere::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    xcolor  HeaderColor ( 119, 128, 144 );
    xcolor  ItemColor   ( 176, 176, 176 );

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
                Name.Format( "Sphere" );
                Value.Format( "" );
                Type        = PROP_HEADER;
                UIColor     = HeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 34:
                //============================================================================
                Name.Format( "Sphere\\Segments U" );
                Value.Format( "%d", m_nSegmentsU );
                Type        = PROP_INT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 35:
                //============================================================================
                Name.Format( "Sphere\\Segments V" );
                Value.Format( "%d", m_nSegmentsV );
                Type        = PROP_INT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 36:
                //============================================================================
                Name.Format( "Sphere\\Top Pos" );
                Value.Format( "%g", m_TopPos );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 37:
                //============================================================================
                Name.Format( "Sphere\\Bottom Pos" );
                Value.Format( "%g", m_BottomPos );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 38:
                //============================================================================
                Name.Format( "Sphere\\Top Color" );
                Value.Format( "%d, %d, %d, %d", m_ColorTop.R, m_ColorTop.G, m_ColorTop.B, m_ColorTop.A );
                Type        = PROP_COLOR;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 39:
                //============================================================================
                Name.Format( "Sphere\\Bottom Color" );
                Value.Format( "%d, %d, %d, %d", m_ColorBottom.R, m_ColorBottom.G, m_ColorBottom.B, m_ColorBottom.A );
                Type        = PROP_COLOR;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
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


xbool element_sphere::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
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
    else if( x_strcmp( Property, "Material\\Tile V" ) == 0 )
    {
        m_MappingTileV = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Scroll U" ) == 0 )
    {
        m_MappingScrollU = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Scroll V" ) == 0 )
    {
        m_MappingScrollV = (f32)x_atof(Value);
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Sphere\\Segments U" ) == 0 )
    {
        m_nSegmentsU = x_atoi(Value);
        m_nSegmentsU = MIN( m_nSegmentsU, 200 );
        m_nSegmentsU = MAX( m_nSegmentsU, 3 );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Sphere\\Segments V" ) == 0 )
    {
        m_nSegmentsV = x_atoi(Value);
        m_nSegmentsV = MIN( m_nSegmentsV, 200 );
        m_nSegmentsV = MAX( m_nSegmentsV, 3 );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Sphere\\Top Pos" ) == 0 )
    {
        m_TopPos = x_atof(Value);
        m_TopPos = MIN( m_TopPos, 1.0f );
        m_TopPos = MAX( m_TopPos, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Sphere\\Bottom Pos" ) == 0 )
    {
        m_BottomPos = x_atof(Value);
        m_BottomPos = MIN( m_BottomPos, 1.0f );
        m_BottomPos = MAX( m_BottomPos, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Sphere\\Top Color" ) == 0 )
    {
        String2Color( Value, m_ColorTop );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Sphere\\Bottom Color" ) == 0 )
    {
        String2Color( Value, m_ColorBottom );
        Dirty = TRUE;
    }

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

void element_sphere::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "SPHERE", "Type of element" );

        // add the base stuff
        element::Save( Igf );

        // add our specific stuff
        Igf.AddS32      ( "SPHERE_VER",     SPHERE_VER,     "Sphere Element Version Number" );
        Igf.AddString   ( "Bitmap",         m_BitmapName     );
        Igf.AddS32      ( "CombineMode",    m_CombineMode    );
        Igf.AddBool     ( "MapPlanar",      m_MapPlanar      );
        Igf.AddF32      ( "MappingTileU",   m_MappingTileU   );
        Igf.AddF32      ( "MappingTileV",   m_MappingTileV   );
        Igf.AddF32      ( "MappingScrollU", m_MappingScrollU );
        Igf.AddF32      ( "MappingScrollV", m_MappingScrollV );
        Igf.AddS32      ( "nSegmentsU",     m_nSegmentsU     );
        Igf.AddS32      ( "nSegmentsV",     m_nSegmentsV     );
        Igf.AddF32      ( "TopPos",         m_TopPos         );
        Igf.AddF32      ( "BottomPos",      m_BottomPos      );
        Igf.AddColor    ( "ColorTop",       m_ColorTop       );
        Igf.AddColor    ( "ColorBottom",    m_ColorBottom    );
    }
    
    Igf.ExitGroup();
}

//============================================================================
void element_sphere::Load( igfmgr& Igf )
{
    s32 SphereVer = 0;

    // base stuff first
    element::Load( Igf );

    // This is for backwards compatibility
    if ( Igf.Find( "SPHERE_VER" ) )
        SphereVer = Igf.GetS32();

    // get our specific stuff
    m_BitmapName        = Igf.GetString ( "Bitmap"         );
    m_CombineMode       = Igf.GetS32    ( "CombineMode"    );
    m_MapPlanar         = Igf.GetBool   ( "MapPlanar"      );
    m_MappingTileU      = Igf.GetF32    ( "MappingTileU"   );
    m_MappingTileV      = Igf.GetF32    ( "MappingTileV"   );
    m_MappingScrollU    = Igf.GetF32    ( "MappingScrollU" );
    m_MappingScrollV    = Igf.GetF32    ( "MappingScrollV" );
    m_nSegmentsU        = Igf.GetS32    ( "nSegmentsU"     );
    m_nSegmentsV        = Igf.GetS32    ( "nSegmentsV"     );
    m_TopPos            = Igf.GetF32    ( "TopPos"         );
    m_BottomPos         = Igf.GetF32    ( "BottomPos"      );
    m_ColorTop          = Igf.GetColor  ( "ColorTop"       );
    m_ColorBottom       = Igf.GetColor  ( "ColorBottom"    );
}

//============================================================================
// Export data
void element_sphere::ExportData( export::fx_elementhdr& ElemHdr, 
                                 xstring& Type,
                                 xbytestream& Stream, 
                                 s32 ExportTarget )
{
    
    Type.Format( "SPHERE" );

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

    element_sphere_export ExportData;

    ExportData.m_BottomPos      = m_BottomPos;
    ExportData.m_ColorBottom    = m_ColorBottom;
    ExportData.m_ColorTop       = m_ColorTop;
    ExportData.m_MappingScrollU = m_MappingScrollU;
    ExportData.m_MappingScrollV = m_MappingScrollV;
    ExportData.m_MappingTileU   = m_MappingTileU;
    ExportData.m_MappingTileV   = m_MappingTileV;
    ExportData.m_MapPlanar      = m_MapPlanar;
    ExportData.m_nSegmentsU     = m_nSegmentsU;
    ExportData.m_nSegmentsV     = m_nSegmentsV;
    ExportData.m_TopPos         = m_TopPos;

    if( ExportTarget == EXPORT_TARGET_GCN )
    {
        // Must reverse the Endian of the colors.
        SwapColorEndian( ExportData.m_ColorBottom );
        SwapColorEndian( ExportData.m_ColorTop );
    }

    TempStream.Append( (byte*)&ExportData, sizeof(element_sphere_export) );

    // Adjust the size
    ElemHdr.TotalSize = TempStream.GetLength() / sizeof(s32);
    TempStream.Replace( 0, (const u8*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // Append the new data onto our stream
    Stream.Append( TempStream );
}

//============================================================================
// Flag the texture as being used
void element_sphere::FlagExportTextures( void )
{
    g_pTextureMgr->MarkAsUsed(m_BitmapName);
}

//============================================================================
// Activate the texture
void element_sphere::ActivateTextures( void )
{
    g_pTextureMgr->ActivateBitmap(m_BitmapName);
}

} // namespace fx_core
