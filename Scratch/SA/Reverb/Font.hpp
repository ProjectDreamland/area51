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
        H_LEFT          = 0X0001,
        H_CENTER        = 0X0002,
        H_RIGHT         = 0X0004,
        V_TOP           = 0X0008,
        V_CENTER        = 0X0010,
        V_BOTTOM        = 0X0020,
        CLIP_CHARACTER  = 0X0040,
        CLIP_L_JUSTIFY  = 0X0080,
        CLIP_R_JUSTIFY  = 0X0100,
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
