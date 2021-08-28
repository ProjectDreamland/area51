#include "controller.hpp"
#include "element_sprite.hpp"
#include "effect.hpp"

namespace fx_core
{

#define SPRITE_VER      2

#define ELEMENT_EXPORT_TYPE         ( 'S' | ('P'<<8) | ('R'<<16) | ('T'<<24) )

//============================================================================
// element REGISTRATION
//============================================================================
element*         s_SpriteFactory    ( void )  { return (element*)new element_sprite; }
static elem_reg  s_SpriteKeyReg               ( "sprite", s_SpriteFactory );


//============================================================================

void SwapColorEndian( xcolor& Color );

//============================================================================
// Constructor

element_sprite::element_sprite( )
: element()
{
    m_pType         = "Sprite";
}

element_sprite::~element_sprite()
{
}

//============================================================================

void element_sprite::Create( const char* pElementID, effect& Effect )
{
    // Assert that the ID not be NULL
    ASSERT( pElementID );

    m_ID        = pElementID;
    m_pEffect   = &Effect;

    m_BitmapName    = "";
    m_CombineMode   = COMBINEMODE_ALPHA;
    m_ZBias         = 0.0f;

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

element* element_sprite::Duplicate( void )
{
    element_sprite* pNew = new element_sprite( *this );

    pNew->m_BitmapName      = m_BitmapName;
    pNew->m_CombineMode     = m_CombineMode;
    pNew->m_ZBias           = m_ZBias;

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

void element_sprite::PostCopyPtrFix( void )
{
}

//============================================================================

xbool element_sprite::GetWorldBBoxAtTime( f32 T, bbox& BBox ) const
{
    // Sprite BBox is defined by:
    // - Position:  L2W position
    // - Rotation:  NO rotation
    // - Scale:     Average of X & Y scale

    if( ExistsAtTime( T ) )
    {
        matrix4 L2W;
        matrix4 BBoxL2W;
        vector3 Scale;

        VERIFY( GetLocalBBoxAtTime( T, BBox  ) );
        VERIFY( GetL2WAtTime      ( T, L2W   ) );
        VERIFY( GetScaleAtTime    ( T, Scale ) );

        f32 BBoxScale = ( Scale.GetX() + Scale.GetY() ) * 0.5f;

        BBoxL2W.Identity();
        BBoxL2W.SetScale( BBoxScale );
        BBoxL2W.Translate( L2W.GetTranslation() );

        BBox.Transform( BBoxL2W );

        return TRUE;
    }

    return FALSE;
}

//============================================================================
// Render

void element_sprite::Render( f32 T )
{
    // Only render if we exist
    if( ExistsAtTime( T ) && (!m_Hide) )
    {
        vector3 Scale;
        vector3 Rot;
        vector3 Pos;
        xcolor  Color;

        GetScaleAtTime   ( T, Scale );
        GetRotationAtTime( T, Rot   );
        GetPositionAtTime( T, Pos   );
        GetColorAtTime   ( T, Color );

        // Clear L2W
        draw_ClearL2W();

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

         // draw flags
        u32 DrawFlags   = DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_UV_CLAMP | DrawBlendMode;
        if( !m_ZRead )  { DrawFlags |= DRAW_NO_ZBUFFER; }

       // Start drawing
        draw_Begin( DRAW_SPRITES, DrawFlags );

        // Setup bitmap
        if ( m_BitmapName.IsEmpty() )
            m_BitmapName = "fx_default.xbmp";

        g_pTextureMgr->ActivateBitmap( m_BitmapName );

        // Draw the sprite
        draw_SpriteUV( Pos,
                       vector2(Scale[0], Scale[1]),
                       vector2(0,0),
                       vector2(1,1),
                       Color,
                       Rot[2] );

        // End drawing
        draw_End();

        // Render element bbox
        if ( m_pEffect->RenderBBoxesEnabled() )
        {
            bbox B;
            GetWorldBBoxAtTime( T, B ) ;
            draw_BBox( B, IsSelected() ? XCOLOR_WHITE : XCOLOR_GREY );
        }
    }

    // Render the translation path of the object
    RenderTrajectory();
}

//============================================================================
// Show properties

xbool element_sprite::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    f32     Pos  [3];
    f32     Scale[3];
    f32     Rot  [3];
    f32     Color[3];
    f32     Alpha;

    m_pScale->GetValue         ( (f32)T, Scale  );
    m_pRotation->GetValue      ( (f32)T, Rot    );
    m_pTranslation->GetValue   ( (f32)T, Pos    );
    m_pColor->GetValue         ( (f32)T, Color  );
    m_pAlpha->GetValue         ( (f32)T, &Alpha );

    xcolor  HeaderColor ( 119, 128, 144 );
    xcolor  ItemColor   ( 176, 176, 176 );

    // Let's over-ride the base class implementation....easier to just handle it all here
    switch( Idx )
    {
        case 0:
            //============================================================================
            Name.Format( "Object" );
            Value.Format( "" );
            Type        = PROP_HEADER;
            UIColor     = HeaderColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 1:
            //============================================================================
            Name.Format( "Object\\Name" );
            Value.Format( "%s", (const char*)m_ID );
            Type        = PROP_STRING;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 2:
            //============================================================================
            Name.Format( "Object\\Custom Type" );
            Value.Format( "%s", (const char*)m_CustomType );
            Type        = PROP_STRING;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 3:
            //============================================================================
            Name.Format( "Object\\Immortal" );
            Value.Format( "%s", m_IsImmortal == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 4:
            //============================================================================
            Name.Format( "Object\\Start Frame" );
            Value.Format( "%d", m_LifeStartFrame );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 5:
            //============================================================================
            Name.Format( "Object\\Stop Frame" );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;

            if( m_IsImmortal )
            {
                Value       = "";
                IsDisabled  = TRUE;
            }
            else
            {
                Value.Format( "%d", m_LifeStopFrame );
                IsDisabled  = FALSE;
            }

            return TRUE;
        case 6:
            //============================================================================
            Name.Format( "Object\\Show Trajectory" );
            Value.Format( "%s", m_ShowTrajectory == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 7:
            //============================================================================
            Name.Format( "Transform" );
            Value.Format( "" );
            Type        = PROP_HEADER;
            UIColor     = HeaderColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 8:
            //============================================================================
            Name.Format( "Transform\\Position" );
            if( ((ctrl_linear*)m_pTranslation)->IsSmooth() )    { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 9:
            //============================================================================
            Name.Format( "Transform\\Position\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pTranslation->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 10:
            //============================================================================
            Name.Format( "Transform\\Position\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pTranslation->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 11:
            //============================================================================
            Name.Format( "Transform\\Position\\XYZ" );
            Value.Format( "%g, %g, %g", Pos[0], Pos[1], Pos[2] );
            Type        = PROP_V3;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 12:
            //============================================================================
            Name.Format( "Transform\\Rotation" );
            if( ((ctrl_linear*)m_pRotation)->IsSmooth() )       { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 13:
            //============================================================================
            Name.Format( "Transform\\Rotation\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pRotation->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 14:
            //============================================================================
            Name.Format( "Transform\\Rotation\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pRotation->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 15:
            //============================================================================
            Name.Format( "Transform\\Rotation\\Z" );
            Value.Format( "%g", RAD_TO_DEG(Rot[2]) );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 16:
            //============================================================================
            Name.Format( "Transform\\Scale" );
            if( ((ctrl_linear*)m_pScale)->IsSmooth() )          { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 17:
            //============================================================================
            Name.Format( "Transform\\Scale\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pScale->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 18:
            //============================================================================
            Name.Format( "Transform\\Scale\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pScale->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 19:
            //============================================================================
            Name.Format( "Transform\\Scale\\X" );
            Value.Format( "%g", Scale[0] );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 20:
            //============================================================================
            Name.Format( "Transform\\Scale\\Y" );
            Value.Format( "%g", Scale[1] );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 21:
            //============================================================================
            Name.Format( "Material" );
            Value.Format( "" );
            Type        = PROP_HEADER;
            UIColor     = HeaderColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 22:
            //============================================================================
            Name.Format( "Material\\Color" );
            if( ((ctrl_linear*)m_pColor)->IsSmooth() )          { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 23:
            //============================================================================
            Name.Format( "Material\\Color\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pColor->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 24:
            //============================================================================
            Name.Format( "Material\\Color\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pColor->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 25:
            //============================================================================
            Name.Format( "Material\\Color\\RGBA" );
            Value.Format( "%d, %d, %d, %d", (u8)Color[0], (u8)Color[1], (u8)Color[2], (u8)Alpha );
            Type        = PROP_COLOR;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 26:
            //============================================================================
            Name.Format( "Material\\Z Read" );
            Value.Format( "%s", m_ZRead == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 27:
            //============================================================================
            Name.Format( "Material\\Z Bias" );
            Value.Format( "%g", m_ZBias );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 28:
            //============================================================================
            Name.Format( "Material\\Draw Mode" );
            Value = CombineMode_ToString( m_CombineMode );
            Type        = PROP_COMBINEMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 29:
            //============================================================================
            Name.Format( "Material\\Bitmap" );
            Value = m_BitmapName;
            Type        = PROP_FILENAME;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;

        default:
            return FALSE;
    }
   
    return FALSE;
}


//============================================================================
// Update data fed back from the property bar


xbool element_sprite::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
{
    xbool Dirty = FALSE;

    // Strip out the header information from the Field, so we can check the property strings
    s32     PropLength  = Field.GetLength() - ( 17 + m_ID.GetLength() );
    xstring Property    = Field.Right( PropLength );

    //-------------------------------------------------------------------------
    // Check for properites we want to over-ride first 

    // Non-Animatable Properties
    if( x_strcmp( Property, "Material\\Z Bias" ) == 0 )
    {
        m_ZBias = x_atof( Value );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Draw Mode" ) == 0 )
    {
        m_CombineMode = CombineMode_FromString( Value );
        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Bitmap" ) == 0 )
    {
        m_BitmapName = Value;
        Dirty = TRUE;
    }

    // Animatable Properties
    if ( m_pEffect->IsAnimateModeOn() == TRUE )
    {
        if( x_strcmp( Property, "Transform\\Rotation\\Z" ) == 0 )
        {
            float Rot[3];

            m_pRotation->GetValue( (f32)T, Rot );

            // plug in the new value
            Rot[2] = DEG_TO_RAD( (f32)atof( Value ) );
        
            // set the key
            key* pKey = new key(3);
            pKey->SetKey( T, Rot );
            m_pRotation->SetValue( T, pKey );

            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Scale\\X" ) == 0 )
        {
            float Scale[3];
            m_pScale->GetValue( (f32)T, Scale );

            // plug in the new value
            Scale[0] = (f32)atof( Value );

            // set the key
            key* pKey = new key(3);
            pKey->SetKey( T, Scale );
            m_pScale->SetValue( T, pKey );

            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Scale\\Y" ) == 0 )
        {
            float Scale[3];
            m_pScale->GetValue( (f32)T, Scale );

            // plug in the new value
            Scale[1] = (f32)atof( Value );

            // set the key
            key* pKey = new key(3);
            pKey->SetKey( T, Scale );
            m_pScale->SetValue( T, pKey );

            Dirty = TRUE;
        }
    }
    else
    {
        if( x_strcmp( Property, "Transform\\Rotation\\Z" ) == 0 )
        {
            float Rot[3];
            m_pRotation->GetValue( (f32)T, Rot );

            radian3 Delta;
            Delta.Pitch = 0;
            Delta.Yaw   = 0;
            Delta.Roll  = DEG_TO_RAD( (f32)atof( Value ) ) - Rot[2];

            Rotate( Delta );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Scale\\X" ) == 0 )
        {
            float Scale[3];
            m_pScale->GetValue( (f32)T, Scale );

            vector3 Delta;
            Delta.GetX()     = (f32)atof( Value ) - Scale[0];
            Delta.GetY()     = 0;
            Delta.GetZ()     = 0;

            this->Scale( Delta );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Scale\\Y" ) == 0 )
        {
            float Scale[3];
            m_pScale->GetValue( (f32)T, Scale );

            vector3 Delta;
            Delta.GetX()     = 0;
            Delta.GetY()     = (f32)atof( Value ) - Scale[1];
            Delta.GetZ()     = 0;

            this->Scale( Delta );
            Dirty = TRUE;
        }
    }

    //-------------------------------------------------------------------------
    // Let the base class handle any properties we didn't over-ride

    if( Dirty == FALSE)
    {
        Dirty = element::OnPropertyChanged( T, Field, Value );
    }

    // m_pEffect->GetDocument()->UpdateAllViews( NULL );

    return Dirty;
}


//============================================================================
// Save data from an element

void element_sprite::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "SPRITE", "Type of element" );

        // add the base stuff
        element::Save( Igf );

        // add our specific stuff
        Igf.AddS32      ( "SPRITE_VER",     SPRITE_VER,     "Sprite Element Version Number" );
        Igf.AddString   ( "Bitmap",         m_BitmapName  );
        Igf.AddS32      ( "CombineMode",    m_CombineMode );
        Igf.AddF32      ( "ZBias",          m_ZBias       );
    }
    
    Igf.ExitGroup();
}

//============================================================================
void element_sprite::Load( igfmgr& Igf )
{
    s32 SpriteVer = 0;

    // base stuff first
    element::Load( Igf );

    // get our specific stuff
    m_BitmapName  = Igf.GetString( "Bitmap" );

    // This is for backwards compatibility
    if ( Igf.Find( "SPRITE_VER" ) )
        SpriteVer = Igf.GetS32();

    m_CombineMode   = ( SpriteVer > 0 ) ? Igf.GetS32( "CombineMode" ) : 0;      // Version 1+
    m_ZBias         = ( SpriteVer > 1 ) ? Igf.GetF32( "ZBias"       ) : 0.0f;   // Version 2+
}

//============================================================================
// Export data
void element_sprite::ExportData( export::fx_elementhdr& ElemHdr, 
                                 xstring& Type,
                                 xbytestream& Stream, 
                                 s32 ExportTarget )
{
    
    Type.Format( "SPRITE" );

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
    // Texture Index
    TempStream.Insert( sizeof(export::fx_elementhdr), (const u8*)&BitmapIndex, sizeof(s32) );
    // Z Bias
    TempStream.Insert( sizeof(export::fx_elementhdr)+sizeof(s32), (const u8*)&m_ZBias, sizeof(f32) );

    // Adjust the size
    ElemHdr.TotalSize = TempStream.GetLength() / sizeof(s32);
    TempStream.Replace( 0, (const u8*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // Append the new data onto our stream
    Stream.Append( TempStream );
}

//============================================================================
// Flag the texture as being used
void element_sprite::FlagExportTextures( void )
{
    g_pTextureMgr->MarkAsUsed(m_BitmapName);
}

//============================================================================
// Activate the texture
void element_sprite::ActivateTextures( void )
{
    g_pTextureMgr->ActivateBitmap(m_BitmapName);
}

} // namespace fx_core
