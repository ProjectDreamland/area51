//==============================================================================
//  
//  FontManager.h
//  
//==============================================================================

#ifndef _FONTMANAGER_H_
#define _FONTMANAGER_H_

//==============================================================================
// INCLUDES
//==============================================================================

#include "Font.hpp"

//==============================================================================
// DEFINES
//==============================================================================

#define SHORT_STRING    8
#define MED_STRING      32
#define LONG_STRING     128

#define SHORT_STRING_COUNT    16
#define MED_STRING_COUNT      8
#define LONG_STRING_COUNT     4

class FontManager
{
public:
    
    enum type
    {
        SMALL,
        LARGE,
    };

    enum flags
    {
        H_LEFT             = 0x0001,
        H_CENTER           = 0x0002,
        H_RIGHT            = 0X0004,
        V_TOP              = 0X0008,
        V_CENTER           = 0X0010,
        V_BOTTOM           = 0X0020,
        CLIP_CHARACTER     = 0X0040,
        CLIP_L_JUSTIFY     = 0X0080,
        CLIP_R_JUSTIFY     = 0X0100,
        USE_EMBEDDED_COLOR = 0x1000,
    };

                FontManager     ( void );
                ~FontManager    ( void );
    void        Init            ( void );
    void        Kill            ( void );
    void        Render          ( void );

    void        RenderText      ( const irect& R, u32 Flags, const xcolor& Color, const char* pString, s8 FontSize = SMALL );
    void        RenderText      ( const irect& R, u32 Flags, s32 Alpha, const char* pString, s8 FontSize = SMALL );

    void        RenderText      ( const irect& R, u32 Flags, const xcolor& Color, const xwchar* pString, s8 FontSize = SMALL );
    void        RenderText      ( const irect& R, u32 Flags, s32 Alpha, const xwchar* pString, s8 FontSize = SMALL );

//protected:
    
    // Use these structures to store the strings, depending on the size of the string we will use either the Short,
    // Med and Long string structure.
    struct short_string
    {
        irect   m_Rect;
        u32     m_Flags;
        xcolor  m_Color;
        s8      m_FontSize;
        xwchar  m_String[SHORT_STRING];
    };

    struct med_string
    {
        irect   m_Rect;
        u32     m_Flags;
        xcolor  m_Color;
        s8      m_FontSize;
        xwchar  m_String[MED_STRING];
    };

    struct long_string
    {
        irect   m_Rect;
        u32     m_Flags;
        xcolor  m_Color;
        s8      m_FontSize;
        xwchar  m_String[LONG_STRING];
    };

    font        m_SmallFont;
    font        m_LargeFont;
    
    // If we ever run out of the string space and the font manager will assert out.
    short_string m_ShortString[SHORT_STRING_COUNT];
    med_string   m_MedString[MED_STRING_COUNT];
    long_string  m_LongString[LONG_STRING_COUNT];

    s32         m_ShortStringCount;
    s32         m_MedStringCount;
    s32         m_LongStringCount;
};

#endif