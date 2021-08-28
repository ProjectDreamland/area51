//==============================================================================
//
//  LevelSettings.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "LevelSettings.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
static struct level_settings_desc : public object_desc
{
    level_settings_desc( void ) : object_desc( 
        object::TYPE_LEVEL_SETTINGS, 
        "Level Settings", 
        "SYSTEM",

        object::ATTR_NULL,

        FLAGS_GENERIC_EDITOR_CREATE ) {}         

    //---------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new level_settings;
    }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const
    { 
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_LEVEL_SETTINGS; 
    }

#endif // X_EDITOR

} s_LevelSettings_Desc;

//=========================================================================

const object_desc& level_settings::GetTypeDesc( void ) const
{
    return s_LevelSettings_Desc;
}

//=========================================================================

const object_desc& level_settings::GetObjectType( void )
{
    return s_LevelSettings_Desc;
}


//==============================================================================
// level_settings
//==============================================================================

void level_settings::slideshow_slide::Reset( void )
{
    TextureName[0]  = 0;
    SlideColor      = XCOLOR_WHITE;
    StartFadeIn     = 0;
    EndFadeIn       = 0;
    StartFadeOut    = 0;
    EndFadeOut      = 0;
}

//==============================================================================

level_settings::level_settings ( void )
{
}

//==============================================================================

level_settings::~level_settings( void )
{
}

//==============================================================================
                                                    
void level_settings::OnInit( void )
{
    m_hFogPalette.SetName("");
    m_NearPlane              = 10.0f;
    m_FarPlane               = 8000.0f;
    m_StartUpTrigger         = NULL_GUID;

    #ifdef X_EDITOR
    m_SlideShowDescriptor[0] = 0;
    m_nSlideShowSlides       = 0;
    m_StartTextAnim          = 0;

    s32 i;
    for( i = 0; i < MAX_SLIDES; i++ )
    {
        m_SlideShowSlides[i].Reset();
    }
    #endif // X_EDITOR
}

//==============================================================================

void level_settings::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "LevelSettings",                "Level Settings", 0 );
    List.PropEnumFloat   ( "LevelSettings\\NearPlane",     "Camera distance where near objects clip out", 0 );
    List.PropEnumFloat   ( "LevelSettings\\FarPlane",      "Camera distance where far objects clip out", 0 );
    List.PropEnumExternal( "LevelSettings\\FogPalette",    "Resource\0xbmp", "Resource File", 0 );
    List.PropEnumGuid    ( "LevelSettings\\StartUpTrigger","Trigger to execute prior to level start", 0 );

    #ifdef X_EDITOR
    List.PropEnumString  ( "LevelSettings\\SlideShowAudio", "Slideshow audio descriptor", 0 );
    List.PropEnumInt     ( "LevelSettings\\StartTextAnim",  "Frame to start the text animation", 0 );
    List.PropEnumInt     ( "LevelSettings\\NumSlides",      "Number of slides in the loading slideshow", PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    List.PropEnumButton  ( "LevelSettings\\AddSlide",       "Add a slide to the slide show", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE );
    for( s32 i = 0; i < m_nSlideShowSlides; i++ )
    {
        List.PropEnumHeader ( xfs("LevelSettings\\Slide[%d]",i),                    "Properties for the slideshow", 0 );
        List.PropEnumString ( xfs("LevelSettings\\Slide[%d]\\ImageName",i),         "Slideshow bitmap (max 5 images in slideshow)", 0 );
        List.PropEnumColor  ( xfs("LevelSettings\\Slide[%d]\\SlideColor",i),        "Slide color", 0 );
        List.PropEnumInt    ( xfs("LevelSettings\\Slide[%d]\\StartFadeIn",i),       "Frame where the image or color starts fading in (at 30 fps)", 0 );
        List.PropEnumInt    ( xfs("LevelSettings\\Slide[%d]\\EndFadeIn",i),         "Frame where the image or color finishes fading in (at 30 fps)", 0 );
        List.PropEnumInt    ( xfs("LevelSettings\\Slide[%d]\\StartFadeOut",i),      "Frame where the image or color starts fading out (at 30 fps)", 0 );
        List.PropEnumInt    ( xfs("LevelSettings\\Slide[%d]\\EndFadeOut",i),        "Frame where the image or color finishes fading out (at 30 fps)", 0 );
        List.PropEnumButton ( xfs("LevelSettings\\Slide[%d]\\RemoveSlide",i),       "Remove this slide", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE );
    }
    #endif  // X_EDITOR
}

//==============================================================================

xbool level_settings::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
    }
    else if( I.VarFloat( "LevelSettings\\NearPlane", m_NearPlane, 1.0f, 100000.0f ) )
    {
    }
    else if( I.VarFloat( "LevelSettings\\FarPlane", m_FarPlane, 1.0f, 100000.0f ) )
    {
    }
    else if( I.IsVar( "LevelSettings\\FogPalette" ) )
    {
        // External
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hFogPalette.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            xstring String = I.GetVarExternal();
            if ( !String.IsEmpty() )
            {
                m_hFogPalette.SetName( String );
            }
        }
    }
    else if( I.IsVar( "LevelSettings\\StartUpTrigger" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarGUID( m_StartUpTrigger );
        }
        else
        {
            m_StartUpTrigger = I.GetVarGUID();
        }
    }
    
    // slide show information
    #ifdef X_EDITOR
    else if( I.IsVar( "LevelSettings\\SlideShowAudio" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( m_SlideShowDescriptor, 64 );
        }
        else
        {
            x_strncpy( m_SlideShowDescriptor, I.GetVarString(), 64 );
        }
    }
    else if( I.VarInt( "LevelSettings\\StartTextAnim", m_StartTextAnim, 0, S32_MAX ) )
    {
    }
    else if( I.VarInt( "LevelSettings\\NumSlides", m_nSlideShowSlides, 0, MAX_SLIDES ) )
    {
    }
    else if( I.IsVar( "LevelSettings\\AddSlide" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Slide (16 max)" );
        }
        else
        {
            if( m_nSlideShowSlides < MAX_SLIDES )
                m_nSlideShowSlides++;
        }
    }
    else if( I.IsBasePath( "LevelSettings\\Slide[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.VarString( "LevelSettings\\Slide[]\\ImageName", m_SlideShowSlides[i].TextureName, 64 ) )
        {
        }
        else if( I.VarColor( "LevelSettings\\Slide[]\\SlideColor", m_SlideShowSlides[i].SlideColor ) )
        {
        }
        else if( I.VarInt( "LevelSettings\\Slide[]\\StartFadeIn", m_SlideShowSlides[i].StartFadeIn, 0, S32_MAX ) )
        {
        }
        else if( I.VarInt( "LevelSettings\\Slide[]\\EndFadeIn", m_SlideShowSlides[i].EndFadeIn, 0, S32_MAX ) )
        {
        }
        else if( I.VarInt( "LevelSettings\\Slide[]\\StartFadeOut", m_SlideShowSlides[i].StartFadeOut, 0, S32_MAX ) )
        {
        }
        else if( I.VarInt( "LevelSettings\\Slide[]\\EndFadeOut", m_SlideShowSlides[i].EndFadeOut, 0, S32_MAX ) )
        {
        }
        else if( I.IsVar( "LevelSettings\\Slide[]\\RemoveSlide" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "Remove slide" );
            }
            else
            {
                ASSERT( i >= 0 );
                x_memmove( &m_SlideShowSlides[i], &m_SlideShowSlides[i+1], (m_nSlideShowSlides-i-1)*sizeof(slideshow_slide) );
                m_nSlideShowSlides--;
            }
        }
        else
        {
            return FALSE;
        }
    }
    #endif // X_EDITOR

    else
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

#ifdef X_EDITOR
s32 level_settings::OnValidateProperties( xstring& ErrorMsg )
{
    s32 errors = object::OnValidateProperties( ErrorMsg );

    // make sure we're not using more than five textures in the slide show
    s32 i;
    s32 TextureCount = 0;
    for( i = 0; i < m_nSlideShowSlides; i++ )
    {
        if( m_SlideShowSlides[i].TextureName[0] != '\0' )
        {
            TextureCount++;
        }
    }

    if( TextureCount > 5 )
    {
        ErrorMsg += "\nSlide show can only have 5 images\n";
        errors   += 1;
    }

    return errors;
}
#endif
