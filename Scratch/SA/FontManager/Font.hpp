//==============================================================================
//  
//  font.hpp
//  
//==============================================================================

#ifndef _FONT_HPP_
#define _FONT_HPP_

//==============================================================================
//  INCLUDES
//==============================================================================

//#ifndef X_TYPES_HPP
#include "Entropy.hpp"
//#include "x_types.hpp"
//#endif

//==============================================================================
//  font
//==============================================================================

class font
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
    };

    struct Character
    {
        s32     X;
        s32     Y;
        s32     W;
    };

protected:
    xbitmap     m_Bitmap;
    s32         m_BmWidth;
    s32         m_BmHeight;
    s32         m_RowHeight;
    s32         m_MaxWidth;
    s32         m_AvgWidth;
    s32         m_Height;
    Character   m_Characters[256];

public:
    xbool               Load                ( const char* pPathName );
    void                Kill                ( void );
    void                TextSize            ( irect& Rect, const   char* pString, s32 Count = -1 ) const;
    void                TextSize            ( irect& Rect, const xwchar* pString, s32 Count = -1 ) const;
    s32                 TextWidth           (              const xwchar* pString, s32 Count = -1 ) const;
    s32                 TextHeight          (              const xwchar* pString, s32 Count = -1 ) const;
    const Character&    GetCharacter        ( s32 Index ) const;
    s32                 GetLineHeight       ( void ) const { return m_Height; };

    void                RenderText          ( const irect& R, u32 Flags, const xcolor& Color, const   char* pString ) const;
    void                RenderText          ( const irect& R, u32 Flags, const xcolor& Color, const xwchar* pString, xbool IgnoreEmbeddedColor = TRUE ) const;
    void                RenderText          ( const irect& R, u32 Flags,       s32     Alpha, const xwchar* pString ) const;
//  void                DrawFormattedText   ( const irect& R, u32 Flags, const xcolor& Color, const   char* pString ) const;
//  void                DrawFormattedText   ( const irect& R, u32 Flags, const xcolor& Color, const xwchar* pString ) const;
};

//==============================================================================
#endif // FONT_HPP
//==============================================================================
