#include "controller.hpp"
#include "element_cylinder.hpp"
#include "effect.hpp"

namespace fx_core
{

#define CYLINDER_VER      1

#define ELEMENT_EXPORT_TYPE     ( 'C' | ('Y'<<8) | ('L'<<16) | ('N'<<24) )

//============================================================================
// element REGISTRATION
//============================================================================
element*         s_CylinderFactory  ( void )    { return (element*)new element_cylinder; }
static elem_reg  s_CylinderKeyReg               ( "cylinder", s_CylinderFactory );


//============================================================================

void SwapColorEndian( xcolor& Color );

//============================================================================
// Constructor

element_cylinder::element_cylinder( )
: element()
{
    m_pType         = "Cylinder";
}

element_cylinder::~element_cylinder()
{
}

//============================================================================

void element_cylinder::Create( const char* pElementID, effect& Effect )
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
    m_nSegments         = 8;
    m_SizeTop           = 1.0f;
    m_SizeBottom        = 1.0f;
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

element* element_cylinder::Duplicate( void )
{
    element_cylinder* pNew = new element_cylinder( *this );

    pNew->m_BitmapName      = m_BitmapName;
    pNew->m_CombineMode     = m_CombineMode;
    pNew->m_MapPlanar       = m_MapPlanar;
    pNew->m_MappingTileU    = m_MappingTileU;
    pNew->m_MappingTileV    = m_MappingTileV;
    pNew->m_MappingScrollU  = m_MappingScrollU;
    pNew->m_MappingScrollV  = m_MappingScrollV;
    pNew->m_nSegments       = m_nSegments;
    pNew->m_SizeTop         = m_SizeTop;
    pNew->m_SizeBottom      = m_SizeBottom;

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

void element_cylinder::PostCopyPtrFix( void )
{
}


//============================================================================
// Render

void element_cylinder::Render( f32 T )
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

        if ( (m_MappingTileV == 1.0f) && (m_MappingScrollV == 0.0f) ) { DrawFlags |= DRAW_V_CLAMP; }
        //if ( DrawBlendMode != 0 )                                     { DrawFlags |= DRAW_NO_ZWRITE; }
        if ( !m_ZRead )                                               { DrawFlags |= DRAW_NO_ZBUFFER; }

        // Start drawing
        draw_Begin( DRAW_TRIANGLES, DrawFlags );

        // Setup bitmap
        if ( m_BitmapName.IsEmpty() )
            m_BitmapName = "fx_default.xbmp";

        g_pTextureMgr->ActivateBitmap( m_BitmapName );

        // Setup for drawing
        f32 RadiansPerSegmentU  = DEG_TO_RAD( 360.f / (f32)m_nSegments );
        f32 RadianOffset        = RadiansPerSegmentU / 2.0f;

        f32 MappingPerSegmentU  = m_MappingTileU / (f32)m_nSegments;

        // UV scroll starts at LifeStart...we multiply by 0.033333333 for 30fps
        f32 MapScrollTime       = ( T - (f32)m_LifeStartFrame ) * 0.03333333f;
        f32 MapScrollU          = m_MappingScrollU * MapScrollTime;
        f32 MapScrollV          = m_MappingScrollV * MapScrollTime;

        f32 Radius  = 0.5f;
        f32 Radius1 = 0.5f * m_SizeTop;
        f32 Radius2 = 0.5f * m_SizeBottom;

        f32 x[4];
        f32 z[4];

        f32 u1, u2, u3, u4;
        f32 v1, v2, v3, v4;

        f32 uAngle1, uAngle2;

        // UV Mapping - Pre-compute V if we're not doing planar mapping
        if( !m_MapPlanar )
        {
            v1 = MapScrollV;
            v2 = v1;
            v3 = MapScrollV + m_MappingTileV;
            v4 = v3;
        }

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

        xcolor NewTopColor;
        xcolor NewBottomColor;

        NewTopColor.SetfRGBA    ( r1*r2, g1*g2, b1*b2, a1*a2 );
        NewBottomColor.SetfRGBA ( r1*r3, g1*g3, b1*b3, a1*a3 );

        // U Segments
        for( s32 i = 0; i < m_nSegments; i++ )
        {
            uAngle1 = RadiansPerSegmentU * i;
            uAngle2 = RadiansPerSegmentU * ((i+1)%m_nSegments); //(i+1);

            x[0] = x_sin( uAngle1 ) * Radius1;
            x[1] = x_sin( uAngle2 ) * Radius1;
            x[2] = x_sin( uAngle1 - RadianOffset ) * Radius2;
            x[3] = x_sin( uAngle2 - RadianOffset ) * Radius2;

            z[0] = x_cos( uAngle1 ) * Radius1;
            z[1] = x_cos( uAngle2 ) * Radius1;
            z[2] = x_cos( uAngle1 - RadianOffset ) * Radius2;
            z[3] = x_cos( uAngle2 - RadianOffset ) * Radius2;

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
            else
            {
                u1 = MapScrollU + ( MappingPerSegmentU *   i   );
                u2 = MapScrollU + ( MappingPerSegmentU * (i+1) );

                u3 = u1 - (MappingPerSegmentU * 0.5f);
                u4 = u2 - (MappingPerSegmentU * 0.5f);
            }

            // Draw UV Quad
            draw_UV( u1, v1 );    draw_Color( NewTopColor    );     draw_Vertex( x[0],  0.5f, z[0] );
            draw_UV( u3, v3 );    draw_Color( NewBottomColor );     draw_Vertex( x[2], -0.5f, z[2] );
            draw_UV( u4, v4 );    draw_Color( NewBottomColor );     draw_Vertex( x[3], -0.5f, z[3] );

            draw_UV( u1, v1 );    draw_Color( NewTopColor    );     draw_Vertex( x[0],  0.5f, z[0] );
            draw_UV( u4, v4 );    draw_Color( NewBottomColor );     draw_Vertex( x[3], -0.5f, z[3] );
            draw_UV( u2, v2 );    draw_Color( NewTopColor    );     draw_Vertex( x[1],  0.5f, z[1] );
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

xbool element_cylinder::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
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
                Name.Format( "Cylinder" );
                Value.Format( "" );
                Type        = PROP_HEADER;
                UIColor     = HeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 34:
                //============================================================================
                Name.Format( "Cylinder\\Segments" );
                Value.Format( "%d", m_nSegments );
                Type        = PROP_INT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 35:
                //============================================================================
                Name.Format( "Cylinder\\Top Size" );
                Value.Format( "%g", m_SizeTop );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 36:
                //============================================================================
                Name.Format( "Cylinder\\Bottom Size" );
                Value.Format( "%g", m_SizeBottom );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 37:
                //============================================================================
                Name.Format( "Cylinder\\Top Color" );
                Value.Format( "%d, %d, %d, %d", m_ColorTop.R, m_ColorTop.G, m_ColorTop.B, m_ColorTop.A );
                Type        = PROP_COLOR;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 38:
                //============================================================================
                Name.Format( "Cylinder\\Bottom Color" );
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


xbool element_cylinder::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
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
    else if( x_strcmp( Property, "Cylinder\\Segments" ) == 0 )
    {
        m_nSegments = x_atoi(Value);
        m_nSegments = MIN( m_nSegments, 200 );
        m_nSegments = MAX( m_nSegments, 3 );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Cylinder\\Top Size" ) == 0 )
    {
        m_SizeTop = (f32)x_atof(Value);
        m_SizeTop = MIN( m_SizeTop, 1.0f );
        m_SizeTop = MAX( m_SizeTop, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Cylinder\\Bottom Size" ) == 0 )
    {
        m_SizeBottom = (f32)x_atof(Value);
        m_SizeBottom = MIN( m_SizeBottom, 1.0f );
        m_SizeBottom = MAX( m_SizeBottom, 0.0f );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Cylinder\\Top Color" ) == 0 )
    {
        String2Color( Value, m_ColorTop );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Cylinder\\Bottom Color" ) == 0 )
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

void element_cylinder::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "CYLINDER", "Type of element" );

        // add the base stuff
        element::Save( Igf );

        // add our specific stuff
        Igf.AddS32      ( "CYLINDER_VER",   CYLINDER_VER,     "Cylinder Element Version Number" );
        Igf.AddString   ( "Bitmap",         m_BitmapName     );
        Igf.AddS32      ( "CombineMode",    m_CombineMode    );
        Igf.AddBool     ( "MapPlanar",      m_MapPlanar      );
        Igf.AddF32      ( "MappingTileU",   m_MappingTileU   );
        Igf.AddF32      ( "MappingTileV",   m_MappingTileV   );
        Igf.AddF32      ( "MappingScrollU", m_MappingScrollU );
        Igf.AddF32      ( "MappingScrollV", m_MappingScrollV );
        Igf.AddS32      ( "nSegments",      m_nSegments      );
        Igf.AddF32      ( "SizeTop",        m_SizeTop        );
        Igf.AddF32      ( "SizeBottom",     m_SizeBottom     );
        Igf.AddColor    ( "ColorTop",       m_ColorTop       );
        Igf.AddColor    ( "ColorBottom",    m_ColorBottom    );
    }
    
    Igf.ExitGroup();
}

//============================================================================
void element_cylinder::Load( igfmgr& Igf )
{
    s32 ShockWaveVer = 0;

    // base stuff first
    element::Load( Igf );

    // This is for backwards compatibility
    if ( Igf.Find( "SHOCKWAVE_VER" ) )
        ShockWaveVer = Igf.GetS32();

    // get our specific stuff
    m_BitmapName        = Igf.GetString ( "Bitmap"         );
    m_CombineMode       = Igf.GetS32    ( "CombineMode"    );
    m_MapPlanar         = Igf.GetBool   ( "MapPlanar"      );
    m_MappingTileU      = Igf.GetF32    ( "MappingTileU"   );
    m_MappingTileV      = Igf.GetF32    ( "MappingTileV"   );
    m_MappingScrollU    = Igf.GetF32    ( "MappingScrollU" );
    m_MappingScrollV    = Igf.GetF32    ( "MappingScrollV" );
    m_nSegments         = Igf.GetS32    ( "nSegments"      );
    m_SizeTop           = Igf.GetF32    ( "SizeTop"        );
    m_SizeBottom        = Igf.GetF32    ( "SizeBottom"     );
    m_ColorTop          = Igf.GetColor  ( "ColorTop"       );
    m_ColorBottom       = Igf.GetColor  ( "ColorBottom"    );
}

//============================================================================
// Export data
void element_cylinder::ExportData( export::fx_elementhdr& ElemHdr, 
                                   xstring& Type,
                                   xbytestream& Stream, 
                                   s32 ExportTarget )
{
    
    Type.Format( "CYLINDER" );

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

    element_cylinder_export ExportData;

    ExportData.m_ColorBottom    = m_ColorBottom;
    ExportData.m_ColorTop       = m_ColorTop;
    ExportData.m_MappingScrollU = m_MappingScrollU;
    ExportData.m_MappingScrollV = m_MappingScrollV;
    ExportData.m_MappingTileU   = m_MappingTileU;
    ExportData.m_MappingTileV   = m_MappingTileV;
    ExportData.m_MapPlanar      = m_MapPlanar;
    ExportData.m_nSegments      = m_nSegments;
    ExportData.m_SizeBottom     = m_SizeBottom;
    ExportData.m_SizeTop        = m_SizeTop;

    if( ExportTarget == EXPORT_TARGET_GCN )
    {
        // Must reverse the Endian of the colors.
        SwapColorEndian( ExportData.m_ColorBottom );
        SwapColorEndian( ExportData.m_ColorTop );
    }    

    TempStream.Append( (byte*)&ExportData, sizeof(element_cylinder_export) );

    // Adjust the size
    ElemHdr.TotalSize = TempStream.GetLength() / sizeof(s32);
    TempStream.Replace( 0, (const u8*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // Append the new data onto our stream
    Stream.Append( TempStream );
}

//============================================================================
// Flag the texture as being used
void element_cylinder::FlagExportTextures( void )
{
    g_pTextureMgr->MarkAsUsed(m_BitmapName);
}

//============================================================================
// Activate the texture
void element_cylinder::ActivateTextures( void )
{
    g_pTextureMgr->ActivateBitmap(m_BitmapName);
}

} // namespace fx_core
