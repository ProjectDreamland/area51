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
    
    enum Type
    {
        SMALL,
        LARGE,
    };

    enum Flags
    {
        EMBEDDED_COLOR = 0x1000,
    };

    void        Init            ( void );
    void        Kill            ( void );
    void        Render          ( void );

    void        RenderText      ( const irect& R, u32 Flags, const xcolor& Color, const   char* pString, s8 FontSize = SMALL );
    void        RenderText      ( const irect& R, u32 Flags, s32 Alpha, const xwchar* pString, s8 FontSize = SMALL );

protected:
    
    // Use these structures to store the strings, depending on the size of the string we will use either the Short,
    // Med and Long string structure.
    struct ShortString
    {
        irect   m_Rect;
        u32     m_Flags;
        xcolor  m_Color;
        s8      m_FontSize;
        xwchar  m_String[SHORT_STRING];
    };

    struct MedString
    {
        irect   m_Rect;
        u32     m_Flags;
        xcolor  m_Color;
        s8      m_FontSize;
        xwchar  m_String[MED_STRING];
    };

    struct LongString
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
    ShortString m_ShortString[SHORT_STRING_COUNT];
    MedString   m_MedString[MED_STRING_COUNT];
    LongString  m_LongString[LONG_STRING_COUNT];

    s16         m_ShortStringCount;
    s16         m_MedStringCount;
    s16         m_LongStringCount;
};

#endif