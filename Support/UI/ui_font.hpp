//==============================================================================
//  
//  ui_font.hpp
//  
//==============================================================================

#ifndef UI_FONT_HPP
#define UI_FONT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#include "Obj_mgr\obj_mgr.hpp"

//==============================================================================
//  ui_font
//==============================================================================

typedef struct _CustomRenderStruct
{
    s32 m_State;
    f32 m_Value;
    s32 m_Frame;
}CustomRenderStruct;


class ui_font
{
public:
    enum Flags
    {
        h_left          = 0x0001,
        h_center        = 0x0002,
        h_right         = 0x0004,
        v_top           = 0x0008,
        v_center        = 0x0010,
        v_bottom        = 0x0020,
        clip_character  = 0x0040,
        clip_l_justify  = 0x0080,
        clip_r_justify  = 0x0100,
        clip_ellipsis   = 0x0200,
        is_help_text    = 0x0400,
        set_position    = 0x0800,
        blend_additive  = 0x1000,
    };

    enum CUSTOM_RENDER_STATES
    {
        s_render        = 0x0001,
        s_no_render     = 0x0002,
        s_fade_in       = 0x0004,
        s_fade_out      = 0x0008
    };

#if defined(TARGET_XBOX) || defined(TARGET_PC)
#pragma pack(2)
#endif
    struct Character
    {
        u16     X;
        u16     Y;
        u16     W;
    };

    struct charmap
    {
        u16     character;
        u16     bitmap;
    };
#if defined(TARGET_XBOX) || defined(TARGET_PC)
#pragma pack()
#endif


protected:
    rhandle<xbitmap>    m_Bitmap;
    s32                 m_BmWidth;
    s32                 m_BmHeight;
    s32                 m_AvgWidth;
    s32                 m_Height;
    Character*          m_Characters;
    charmap*            m_CMap;
    s32                 m_CMapSize; 
    s32                 m_NumChars;

public:
    xbool               Load                ( const char* pPathName );
    void                Kill                ( void );
    void                TextSize            ( irect& Rect, const   char* pString, s32 Count = -1 ) const;
    void                TextSize            ( irect& Rect, const xwchar* pString, s32 Count = -1 ) const;
    s32                 TextWidth           (              const xwchar* pString, s32 Count = -1 ) const;
    s32                 TextHeight          (              const xwchar* pString, s32 Count = -1 ) const;
    u32                 LookUpCharacter     ( u32 c ) const;
    const Character&    GetCharacter        ( s32 Index ) const;
    s32                 GetLineHeight       ( void ) const { return m_Height; };
    const xwchar*       ClipEllipsis        ( const xwchar* pString, const irect& Rect ) const;
    void                TextWrap            ( const xwchar* pString, const irect& Rect, xwstring& WrappedString );


    void                RenderText          ( const irect& R, u32 Flags, const xcolor& Color, const   char* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = 0.0f ) const;
    void                RenderText          ( const irect& R, u32 Flags, const xcolor& Color, const xwchar* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = 0.0f ) const;
    void                RenderText          ( const irect& R, u32 Flags,       s32     Alpha, const xwchar* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = 0.0f ) const;
    void                RenderHelpText      ( const irect& R, u32 Flags, const xcolor& Color, const xwchar* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = 0.0f ) const;

    void                RenderStateControlledText( const irect& Rect, u32 Flags, const xcolor& Color, const xwchar* pString, void* StateData) const;

//  void                DrawFormattedText   ( const irect& R, u32 Flags, const xcolor& Color, const   char* pString ) const;
//  void                DrawFormattedText   ( const irect& R, u32 Flags, const xcolor& Color, const xwchar* pString ) const;
};

//==============================================================================
#endif // UI_FONT_HPP
//==============================================================================