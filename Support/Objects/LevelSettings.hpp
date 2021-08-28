//==============================================================================
//
//  LevelSettings.hpp
//
//==============================================================================

#ifndef LEVELSETTINGS_HPP
#define LEVELSETTINGS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_mgr\Obj_mgr.hpp"
#include "Render\Texture.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class level_settings : public object
{
public:

    struct slideshow_slide
    {
        void Reset( void );

        // This isn't necessarily the best way to store this data, but
        // it is the best way for artists to pull the information from
        // EDL files exported by Premiere. Better that than trying to
        // to parse EDL files ourselves...
        // When we create the slideshow script we can convert this into more
        // meaningful data.
        char    TextureName[64];
        xcolor  SlideColor;
        s32     StartFadeIn;
        s32     EndFadeIn;
        s32     StartFadeOut;
        s32     EndFadeOut;
    };

    enum { MAX_SLIDES = 16 };

    CREATE_RTTI( level_settings, object, object )

//==============================================================================

                        level_settings          ( void );
                        ~level_settings         ( void );
                                                                
    virtual void        OnInit			        ( void );

    virtual s32         GetMaterial             ( void ) const;
    virtual bbox        GetLocalBBox            ( void ) const;
    virtual void        OnEnumProp              ( prop_enum&     List     );
    virtual xbool       OnProperty              ( prop_query&    I        );
    
    #ifdef X_EDITOR
    virtual s32         OnValidateProperties    ( xstring& ErrorMsg );
    #endif

    virtual const   object_desc&  GetTypeDesc   ( void ) const;
    static  const   object_desc&  GetObjectType ( void );

    f32                     GetNearPlane            ( void ) const;
    f32                     GetFarPlane             ( void ) const;
    guid                    GetStartupGuid          ( void ) const;
    texture::handle         GetFogPalette           ( void ) const;

    #ifdef X_EDITOR
    const char*             GetSlideShowDescriptor  ( void ) const      { return (const char*)m_SlideShowDescriptor; }
    s32                     GetStartTextAnim        ( void ) const      { return m_StartTextAnim; }
    s32                     GetNumSlideShowSlides   ( void ) const      { return m_nSlideShowSlides; }
    const slideshow_slide*  GetSlideShowSlide       ( s32 Index ) const { return &m_SlideShowSlides[Index]; }
    #endif

protected:
   
    f32                 m_NearPlane;
    f32                 m_FarPlane;
    guid                m_StartUpTrigger;
    texture::handle     m_hFogPalette;

    #ifdef X_EDITOR
    char                m_SlideShowDescriptor[64];
    s32                 m_nSlideShowSlides;
    slideshow_slide     m_SlideShowSlides[MAX_SLIDES];
    s32                 m_StartTextAnim;
    #endif  // X_EDITOR
};

//==============================================================================
// INLINE FUNCTIONS
//==============================================================================

inline
guid level_settings::GetStartupGuid( void ) const
{
    return m_StartUpTrigger;
}

//==============================================================================
inline
s32 level_settings::GetMaterial( void ) const
{
    return MAT_TYPE_NULL;
}

//==============================================================================
inline
bbox level_settings::GetLocalBBox( void ) const
{
    return bbox( vector3(0.0f,0.0f,0.0f), 50.0f );
}

//==============================================================================

inline
f32 level_settings::GetNearPlane( void ) const
{
    return m_NearPlane;
}

//==============================================================================

inline
f32 level_settings::GetFarPlane( void ) const
{
    return m_FarPlane;
}

//==============================================================================

inline
texture::handle level_settings::GetFogPalette( void ) const
{
    return m_hFogPalette;
}

//==============================================================================
//==============================================================================
#endif // LEVELSETTINGS_HPP
//==============================================================================
