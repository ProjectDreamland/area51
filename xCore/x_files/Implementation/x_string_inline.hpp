//==============================================================================
//
//  x_string_inline.hpp
//
//==============================================================================

#ifndef X_STRING_INLINE_HPP
#define X_STRING_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================
//
//  These defines are rigged to work with both xstring and xwstring.  See the 
//  implementation notes for each class in x_string.cpp.
//
#define BUFFER_SIZE    (*(((s32*)m_pData) - 2))
#define STRING_LENGTH  (*(((s32*)m_pData) - 1))

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
//  Functions for class xstring.
//==============================================================================

inline s32 xstring::GetLength( void ) const
{
    return( STRING_LENGTH );
}

//==============================================================================

inline xbool xstring::IsEmpty( void ) const
{
    return( STRING_LENGTH == 0 );
}

//==============================================================================

inline char xstring::GetAt( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );

    return( m_pData[Index] );
}

//==============================================================================

inline void xstring::SetAt( s32 Index, char Character )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  STRING_LENGTH );

    m_pData[Index] = Character;
}

//==============================================================================

inline xstring::operator const char* ( void ) const
{
    return( (const char*)m_pData );
}

//==============================================================================

inline char& xstring::operator [] ( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );

    return( m_pData[Index] );
}

//==============================================================================
//  Functions for class xwstring.
//==============================================================================

inline s32 xwstring::GetLength( void ) const
{
    return( STRING_LENGTH );
}

//==============================================================================

inline xbool xwstring::IsEmpty( void ) const
{
    return( STRING_LENGTH == 0 );
}

//==============================================================================

inline xwchar xwstring::GetAt( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );

    return( m_pData[Index] );
}

//==============================================================================

inline void xwstring::SetAt( s32 Index, xwchar Character )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  STRING_LENGTH );

    m_pData[Index] = Character;
}

//==============================================================================

inline xwstring::operator const xwchar* ( void ) const
{
    return( (const xwchar*)m_pData );
}

//==============================================================================

inline xwchar& xwstring::operator [] ( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );

    return( m_pData[Index] );
}

//==============================================================================
//  CLEAR DEFINES
//==============================================================================

#undef BUFFER_SIZE
#undef STRING_LENGTH

//==============================================================================
