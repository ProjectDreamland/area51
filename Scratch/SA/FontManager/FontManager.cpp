//==============================================================================
//  
//  FontManager.cpp
//  
//==============================================================================

#include "FontManager.h"

//=========================================================================

void FontManager::Init ( void )
{
    VERIFY( m_SmallFont.Load( "Font_small.xbmp" ) );
    VERIFY( m_LargeFont.Load( "Font_large.xbmp" ) );

    m_ShortStringCount = 0;
    m_MedStringCount = 0;
    m_LongStringCount = 0;

}

//=========================================================================

void FontManager::Kill ( void )
{
    m_SmallFont.Kill();
    m_LargeFont.Kill();
}

//=========================================================================

void FontManager::Render ( void )
{
    // Render all the string that have been stored in the string strucutres.
    for ( s32 i = 0; i < m_ShortStringCount; i++ )
    {
        // Check the size of the font and render the correct one.
        if( m_ShortString[i].m_FontSize == SMALL )
        {
            if( m_ShortString[i].m_Flags & EMBEDDED_COLOR )
            {
                m_SmallFont.RenderText( m_ShortString[i].m_Rect, m_ShortString[i].m_Flags, m_ShortString[i].m_Color, m_ShortString[i].m_String, FALSE );
            }
            else
            {
                m_SmallFont.RenderText( m_ShortString[i].m_Rect, m_ShortString[i].m_Flags, m_ShortString[i].m_Color, m_ShortString[i].m_String, TRUE );
            }
        }
        else
        {
            if( m_ShortString[i].m_Flags & EMBEDDED_COLOR )
            {
                m_LargeFont.RenderText( m_ShortString[i].m_Rect, m_ShortString[i].m_Flags, m_ShortString[i].m_Color, m_ShortString[i].m_String, FALSE );
            }
            else
            {
                m_LargeFont.RenderText( m_ShortString[i].m_Rect, m_ShortString[i].m_Flags, m_ShortString[i].m_Color, m_ShortString[i].m_String, TRUE );
            }
        }     
    }
    m_ShortStringCount = 0;

    for ( s32 i = 0; i < m_MedStringCount; i++ )
    {
        // Check the size of the font and render the correct one.
        if( m_MedString[i].m_FontSize == SMALL )
        {
            if( m_MedString[i].m_Flags & EMBEDDED_COLOR )
            {
                m_SmallFont.RenderText( m_MedString[i].m_Rect, m_MedString[i].m_Flags, m_MedString[i].m_Color, m_MedString[i].m_String, FALSE );
            }
            else
            {
                m_SmallFont.RenderText( m_MedString[i].m_Rect, m_MedString[i].m_Flags, m_MedString[i].m_Color, m_MedString[i].m_String, TRUE );
            }
        }
        else
        {
            if( m_MedString[i].m_Flags & EMBEDDED_COLOR )
            {
                m_LargeFont.RenderText( m_MedString[i].m_Rect, m_MedString[i].m_Flags, m_MedString[i].m_Color, m_MedString[i].m_String, FALSE );
            }
            else
            {
                m_LargeFont.RenderText( m_MedString[i].m_Rect, m_MedString[i].m_Flags, m_MedString[i].m_Color, m_MedString[i].m_String, TRUE );
            }
        }     
    }
    m_MedStringCount = 0;

    for ( s32 i = 0; i < m_LongStringCount; i++ )
    {
        // Check the size of the font and render the correct one.
        if( m_LongString[i].m_FontSize == SMALL )
        {
            if( m_LongString[i].m_Flags & EMBEDDED_COLOR )
            {
                m_SmallFont.RenderText( m_LongString[i].m_Rect, m_LongString[i].m_Flags, m_LongString[i].m_Color, m_LongString[i].m_String, FALSE );
            }
            else
            {
                m_SmallFont.RenderText( m_LongString[i].m_Rect, m_LongString[i].m_Flags, m_LongString[i].m_Color, m_LongString[i].m_String, TRUE );
            }
        }
        else
        {
            if( m_LongString[i].m_Flags & EMBEDDED_COLOR )
            {
                m_LargeFont.RenderText( m_LongString[i].m_Rect, m_LongString[i].m_Flags, m_LongString[i].m_Color, m_LongString[i].m_String, FALSE );
            }
            else
            {
                m_LargeFont.RenderText( m_LongString[i].m_Rect, m_LongString[i].m_Flags, m_LongString[i].m_Color, m_LongString[i].m_String, TRUE );
            }
        }     
    }
    m_LongStringCount = 0;
}

//=========================================================================

void FontManager::RenderText ( const irect& R, u32 Flags, const xcolor& Color, const   char* pString, s8 FontSize)
{
    // Get the size of the string so we can put it in the correct string structure.
    s32 StrLen = x_strlen( pString );
    
    if( StrLen <= SHORT_STRING )
    {
        ASSERTS( m_ShortStringCount >= SHORT_STRING_COUNT, "Change the SHORT_STRING_COUNT define to a higer number, we need more string space" );
        
        x_strcpy( (char*)m_ShortString[ m_ShortStringCount ].m_String, pString );
        m_ShortString[ m_ShortStringCount ].m_Rect = R;
        m_ShortString[ m_ShortStringCount ].m_Flags = Flags;
        m_ShortString[ m_ShortStringCount ].m_Color = Color;
        m_ShortString[ m_ShortStringCount ].m_FontSize = FontSize;

        m_ShortStringCount++;
    }
    else if( StrLen <= MED_STRING )
    {
        ASSERTS( m_MedStringCount >= MED_STRING_COUNT, "Change the MED_STRING_COUNT define to a higer number, we need more string space" );
        
        x_strcpy( (char*)m_MedString[ m_MedStringCount ].m_String, pString );
        m_MedString[ m_MedStringCount ].m_Rect = R;
        m_MedString[ m_MedStringCount ].m_Flags = Flags;
        m_MedString[ m_MedStringCount ].m_Color = Color;
        m_MedString[ m_MedStringCount ].m_FontSize = FontSize;

        m_MedStringCount++;
    }
    else if( StrLen <= LONG_STRING )
    {
        ASSERTS( m_LongStringCount >= LONG_STRING_COUNT, "Change the LONG_STRING_COUNT define to a higer number, we need more string space" );
        
        x_strcpy( (char*)m_LongString[ m_LongStringCount ].m_String, pString );
        m_LongString[ m_LongStringCount ].m_Rect = R;
        m_LongString[ m_LongStringCount ].m_Flags = Flags;
        m_LongString[ m_LongStringCount ].m_Color = Color;
        m_LongString[ m_LongStringCount ].m_FontSize = FontSize;

        m_LongStringCount++;
    }
    else
    {
        ASSERTS( FALSE, "Change the LONG_STRING define to accept a string that is bigger than 128 characters in length" );
    }
}

//=========================================================================

void FontManager::RenderText ( const irect& R, u32 Flags, s32 Alpha, const xwchar* pString, s8 FontSize)
{
    // Get the size of the string so we can put it in the correct string structure.
    s32 StrLen = x_strlen( (char*)pString );
    xcolor Color( 255, 255, 255, 255 );

    if( StrLen <= SHORT_STRING )
    {
        ASSERTS( m_ShortStringCount >= SHORT_STRING_COUNT, "Change the SHORT_STRING_COUNT define to a higer number, we need more string space" );
        
        x_strcpy( (char*)m_ShortString[ m_ShortStringCount ].m_String, (char*)pString );
        m_ShortString[ m_ShortStringCount ].m_Rect = R;
        m_ShortString[ m_ShortStringCount ].m_Flags = Flags;
        m_ShortString[ m_ShortStringCount ].m_Color = Color;
        m_ShortString[ m_ShortStringCount ].m_Color.A = Alpha;
        m_ShortString[ m_ShortStringCount ].m_FontSize = FontSize;

        m_ShortStringCount++;
    }
    else if( StrLen <= MED_STRING )
    {
        ASSERTS( m_MedStringCount >= MED_STRING_COUNT, "Change the MED_STRING_COUNT define to a higer number, we need more string space" );
        
        x_strcpy( (char*)m_MedString[ m_MedStringCount ].m_String, (char*)pString );
        m_MedString[ m_MedStringCount ].m_Rect = R;
        m_MedString[ m_MedStringCount ].m_Flags = Flags;
        m_MedString[ m_MedStringCount ].m_Color = Color;
        m_MedString[ m_MedStringCount ].m_Color.A = Alpha;
        m_MedString[ m_MedStringCount ].m_FontSize = FontSize;

        m_MedStringCount++;
    }
    else if( StrLen <= LONG_STRING )
    {
        ASSERTS( m_LongStringCount >= LONG_STRING_COUNT, "Change the LONG_STRING_COUNT define to a higer number, we need more string space" );
        
        x_strcpy( (char*)m_LongString[ m_LongStringCount ].m_String, (char*)pString );
        m_LongString[ m_LongStringCount ].m_Rect = R;
        m_LongString[ m_LongStringCount ].m_Flags = Flags;
        m_LongString[ m_LongStringCount ].m_Color = Color;
        m_LongString[ m_LongStringCount ].m_Color.A = Alpha;
        m_LongString[ m_LongStringCount ].m_FontSize = FontSize;

        m_LongStringCount++;
    }
    else
    {
        ASSERTS( FALSE, "Change the LONG_STRING define to accept a string that is bigger than 128 characters in length" );
    }
}

//=========================================================================