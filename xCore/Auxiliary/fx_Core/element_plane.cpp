#include "controller.hpp"
#include "element_plane.hpp"
#include "effect.hpp"

namespace fx_core
{

#define PLANE_VER      1

#define ELEMENT_EXPORT_TYPE     ( ('P') | ('L'<<8) | ('A'<<16) | ('N'<<24) )


//============================================================================
// element REGISTRATION
//============================================================================
element*         s_PlaneFactory     ( void )  { return (element*)new element_plane; }
static elem_reg  s_PlaneKeyReg                  ( "plane", s_PlaneFactory );


//============================================================================

void SwapColorEndian( xcolor& Color );

//============================================================================
// Constructor

element_plane::element_plane( )
: element()
{
    m_pType         = "Plane";
}

element_plane::~element_plane()
{
}

//============================================================================

void element_plane::Create( const char* pElementID, effect& Effect )
{
    // Assert that the ID not be NULL
    ASSERT( pElementID );

    m_ID        = pElementID;
    m_pEffect   = &Effect;

    m_BitmapName        = "";
    m_CombineMode       = COMBINEMODE_ALPHA;
    m_MappingTileU      = 1.0f;
    m_MappingTileV      = 1.0f;
    m_MappingScrollU    = 0.0f;
    m_MappingScrollV    = 0.0f;

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

element* element_plane::Duplicate( void )
{
    element_plane* pNew = new element_plane( *this );

    pNew->m_BitmapName      = m_BitmapName;
    pNew->m_CombineMode     = m_CombineMode;
    pNew->m_MappingTileU    = m_MappingTileU;
    pNew->m_MappingTileV    = m_MappingTileV;
    pNew->m_MappingScrollU  = m_MappingScrollU;
    pNew->m_MappingScrollV  = m_MappingScrollV;

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

void element_plane::PostCopyPtrFix( void )
{
}

//============================================================================

xbool element_plane::GetLocalBBoxAtTime( f32 T, bbox& BBox ) const
{
    if( ExistsAtTime( T ) )
    {
        BBox.Set( vector3(0.0f,0.0f,0.0f), 0.5f );
        return TRUE;
    }

    return FALSE;
}

//============================================================================
// Render

void element_plane::Render( f32 T )
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

        if ( (m_MappingTileU == 1.0f) && (m_MappingScrollU == 0.0f) )   { DrawFlags |= DRAW_U_CLAMP;    }
        if ( (m_MappingTileV == 1.0f) && (m_MappingScrollV == 0.0f) )   { DrawFlags |= DRAW_V_CLAMP;    }
        if ( !m_ZRead )                                                 { DrawFlags |= DRAW_NO_ZBUFFER; }

        // Start drawing
        draw_Begin( DRAW_TRIANGLES, DrawFlags );

        // Setup bitmap
        if ( m_BitmapName.IsEmpty() )
            m_BitmapName = "fx_default.xbmp";

        g_pTextureMgr->ActivateBitmap( m_BitmapName );

        // UV scroll starts at LifeStart...we multiply by 0.033333333 for 30fps
        f32 MapScrollTime       = ( T - (f32)m_LifeStartFrame ) * 0.03333333f;
        f32 MapScrollU          = m_MappingScrollU * MapScrollTime;
        f32 MapScrollV          = m_MappingScrollV * MapScrollTime;

        f32 u1 = ( m_MappingScrollU * MapScrollTime );
        f32 u2 = ( m_MappingScrollU * MapScrollTime ) + m_MappingTileU;

        f32 v1 = ( m_MappingScrollV * MapScrollTime );
        f32 v2 = ( m_MappingScrollV * MapScrollTime ) + m_MappingTileV;

        // Draw the plane
        draw_Color( Color );

        draw_UV( u2, v2 );      draw_Vertex( vector3(  0.5f, -0.5f, 0.0f ) );
        draw_UV( u1, v2 );      draw_Vertex( vector3( -0.5f, -0.5f, 0.0f ) );
        draw_UV( u1, v1 );      draw_Vertex( vector3( -0.5f,  0.5f, 0.0f ) );

        draw_UV( u2, v2 );      draw_Vertex( vector3(  0.5f, -0.5f, 0.0f ) );
        draw_UV( u1, v1 );      draw_Vertex( vector3( -0.5f,  0.5f, 0.0f ) );
        draw_UV( u2, v1 );      draw_Vertex( vector3(  0.5f,  0.5f, 0.0f ) );

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

xbool element_plane::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
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
                Name.Format( "Material\\Tile U" );
                Value.Format( "%g", m_MappingTileU );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 29:
                //============================================================================
                Name.Format( "Material\\Tile V" );
                Value.Format( "%g", m_MappingTileV );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 30:
                //============================================================================
                Name.Format( "Material\\Scroll U" );
                Value.Format( "%g", m_MappingScrollU );
                Type        = PROP_FLOAT;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 31:
                //============================================================================
                Name.Format( "Material\\Scroll V" );
                Value.Format( "%g", m_MappingScrollV );
                Type        = PROP_FLOAT;
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


xbool element_plane::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
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

void element_plane::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "PLANE", "Type of element" );

        // add the base stuff
        element::Save( Igf );

        // add our specific stuff
        Igf.AddS32      ( "PLANE_VER",      PLANE_VER,      "Plane Element Version Number" );
        Igf.AddString   ( "Bitmap",         m_BitmapName     );
        Igf.AddS32      ( "CombineMode",    m_CombineMode    );
        Igf.AddF32      ( "MappingTileU",   m_MappingTileU   );
        Igf.AddF32      ( "MappingTileV",   m_MappingTileV   );
        Igf.AddF32      ( "MappingScrollU", m_MappingScrollU );
        Igf.AddF32      ( "MappingScrollV", m_MappingScrollV );
    }
    
    Igf.ExitGroup();
}

//============================================================================
void element_plane::Load( igfmgr& Igf )
{
    s32 PlaneVer = 0;

    // base stuff first
    element::Load( Igf );

    // This is for backwards compatibility
    if ( Igf.Find( "PLANE_VER" ) )
        PlaneVer = Igf.GetS32();

    // get our specific stuff
    m_BitmapName        = Igf.GetString ( "Bitmap"         );
    m_CombineMode       = Igf.GetS32    ( "CombineMode"    );
    m_MappingTileU      = Igf.GetF32    ( "MappingTileU"   );
    m_MappingTileV      = Igf.GetF32    ( "MappingTileV"   );
    m_MappingScrollU    = Igf.GetF32    ( "MappingScrollU" );
    m_MappingScrollV    = Igf.GetF32    ( "MappingScrollV" );
}

//============================================================================
// Export data
void element_plane::ExportData( export::fx_elementhdr& ElemHdr, 
                                 xstring& Type,
                                 xbytestream& Stream, 
                                 s32 ExportTarget )
{
    
    Type.Format( "PLANE" );

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

    // plane-specific stuff
    element_plane_export    ExportData;

    ExportData.m_MappingScrollU =   m_MappingScrollU;
    ExportData.m_MappingScrollV =   m_MappingScrollV;
    ExportData.m_MappingTileU =     m_MappingTileU;
    ExportData.m_MappingTileV =     m_MappingTileV;

    TempStream.Append( (byte*)&ExportData, sizeof(element_plane_export) );

    // Adjust the size
    ElemHdr.TotalSize = TempStream.GetLength() / sizeof(s32);
    TempStream.Replace( 0, (const u8*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // Append the new data onto our stream
    Stream.Append( TempStream );
}

//============================================================================
// Flag the texture as being used
void element_plane::FlagExportTextures( void )
{
    g_pTextureMgr->MarkAsUsed(m_BitmapName);
}

//============================================================================
// Activate the texture
void element_plane::ActivateTextures( void )
{
    g_pTextureMgr->ActivateBitmap(m_BitmapName);
}

} // namespace fx_core
