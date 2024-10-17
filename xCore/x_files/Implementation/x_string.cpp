//==============================================================================
//
//  x_string.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_STRING_HPP
#include "../x_string.hpp"
#endif

#ifndef X_STDIO_HPP
#include "../x_stdio.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "../x_memory.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

#ifndef X_FILES_PRIVATE_HPP
#include "x_files_private.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================
//
//  These defines are rigged to work with both xstring and xwstring.  See the 
//  implementation notes for each class below.
//
#define BUFFER_SIZE    (*(((s32*)m_pData) - 2))
#define STRING_LENGTH  (*(((s32*)m_pData) - 1))

//==============================================================================
//  SHARED FUNCTIONS FOR CLASSES xfs AND xvfs
//==============================================================================

static
char* FormatIntoStringBuffer( const char* pFormatString, x_va_list Args )
{
    s32*              pNChars;
    char*             pResult;
    x_thread_globals* pGlobals;

    pGlobals = x_GetThreadGlobals();
    pNChars  = (s32*)(pGlobals->StringBuffer + pGlobals->NextOffset);
    pResult  = pGlobals->StringBuffer + pGlobals->NextOffset + 4;
    *pNChars = x_vsprintf( pResult, pFormatString, Args );

    pGlobals->NextOffset = ALIGN_4( pGlobals->NextOffset + *pNChars + 5 );

    // Make sure we did not overflow the string buffer.
    ASSERT( pGlobals->NextOffset < pGlobals->BufferSize );

    return( pResult );
}

//==============================================================================

static
void ReleaseFromStringBuffer( const char* pString )
{
    x_thread_globals* pGlobals;
    s32               NewOffset;

    pGlobals  = x_GetThreadGlobals();
    NewOffset = pString - pGlobals->StringBuffer - 4;

    // Make sure we are always "releasing memory" in the buffer.  That is, when
    // an xfs destructs, its pointer should be "behind" the current global 
    // offset.
    ASSERT( NewOffset <= pGlobals->NextOffset );

    pGlobals->NextOffset = NewOffset;
    pGlobals->StringBuffer[ NewOffset+0 ] = '\0';
    pGlobals->StringBuffer[ NewOffset+1 ] = '\0';
    pGlobals->StringBuffer[ NewOffset+2 ] = '\0';
    pGlobals->StringBuffer[ NewOffset+3 ] = '\0';
    pGlobals->StringBuffer[ NewOffset+4 ] = '\0';
}

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xvfs'
//==============================================================================

xvfs::xvfs( const char* pFormatString, x_va_list Args )
{
    ASSERT( pFormatString );
    m_pString = FormatIntoStringBuffer( pFormatString, Args );
}

//==============================================================================

xvfs::operator const char* ( void )
{
    return( m_pString );
}

//==============================================================================

xvfs::~xvfs( void )
{
    ReleaseFromStringBuffer( m_pString );
}

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xfs'
//==============================================================================

xfs::xfs( const char* pFormatString, ... )
{
    ASSERT( pFormatString );

    x_va_list   Args;
    x_va_start( Args, pFormatString );

    m_pString = FormatIntoStringBuffer( pFormatString, Args );
}

//==============================================================================

xfs::operator const char* ( void )
{
    return( m_pString );
}

//==============================================================================

xfs::~xfs( void )
{
    ReleaseFromStringBuffer( m_pString );
}

//==============================================================================
//  IMPLEMENTATION NOTES NOTES FOR CLASS 'xstring'
//==============================================================================
//
//  In order to get an xstring to be as compatible as possible with C strings,
//  the only class member field is the data pointer.  The other information 
//  needed to make the class work, specifically the "buffer size" and the 
//  "string length" values, have been moved down into the dynamic buffer.
//  
//      xstring
//      +-----------+
//      | m_pData---=-----------------------------+
//      | (char*)   |                             |
//      +-----------+                             |
//                                                V
//                  +--------------------------------------------------------+
//                  | BufferSize | StringLength | string character data...   |
//                  | (s32)      | (s32)        | (char[])                   |
//                  +--------------------------------------------------------+
//  
//  The macros BUFFER_SIZE and STRING_LENGTH fetch the appropriate values from
//  the m_pData pointer.
//  
//  Note that there is ALWAYS a terminating NULL at the end of the string.
//  
//==============================================================================

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xstring'
//==============================================================================

void xstring::EnsureCapacity( s32 NewCapacity )
{
    s32  NewBufferSize;
    s32  OldBufferSize;

    // Since there is always a terminating NULL and we store the buffer size and
    // string length in the buffer, it must be at least 9 bytes larger than the 
    // size of the requested capacity.  Furthermore, in order to reduce heap 
    // activity, the allocation is rounded up to a multiple of 16.

    NewBufferSize = ALIGN_16( NewCapacity + 9 );
    NewCapacity   = NewBufferSize - 9;
    OldBufferSize = BUFFER_SIZE;

    if( NewBufferSize > BUFFER_SIZE )
    {
        if ( (m_pData - 8) == m_LocalData)
        {
            m_pData = (char*)x_malloc(NewBufferSize) + 8;
            x_memcpy(m_pData - 8,m_LocalData,OldBufferSize);
        }
        else
        {
            // Careful!  We must back the pointer off by 8, and then advance by 8.
            m_pData = (char*)x_realloc( m_pData - 8, NewBufferSize ) + 8;
            ASSERT( m_pData-8 );
        }
        BUFFER_SIZE = NewBufferSize;
    }
}

//==============================================================================

xstring::xstring( void )
{
    m_pData = m_LocalData+8;

    BUFFER_SIZE   = sizeof(m_LocalData);
    STRING_LENGTH =  0;
    m_pData[0]    = '\0';
}

//==============================================================================

xstring::xstring( s32 Reserve )
{
    // See comments in EnsureCapacity for why we added 9 and aligned to 16.
    s32 BufferSize = ALIGN_16( Reserve + 9 );

    if (BufferSize <= (s32)sizeof(m_LocalData))
    {
        BufferSize = sizeof(m_LocalData);
        m_pData = m_LocalData + 8;
    }
    else
    {
        m_pData = (char*)x_malloc( BufferSize ) + 8;
        ASSERT( m_pData-8 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = 0;
    m_pData[0]    = '\0';
}

//==============================================================================

xstring::xstring( char Character )
{
    m_pData = m_LocalData+8;
    BUFFER_SIZE   = sizeof(m_LocalData);
    STRING_LENGTH =  1;
    m_pData[0]    = Character;
    m_pData[1]    = '\0';
}

//==============================================================================

xstring::xstring( const char* pString )
{
    ASSERT( pString );

    // See comments in EnsureCapacity for why we added 9 and aligned to 16.
    s32 Len        = x_strlen( pString );
    s32 BufferSize = ALIGN_16( Len + 9 );

    if (BufferSize <= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData+8;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (char*)x_malloc( BufferSize ) + 8;
        ASSERT( m_pData-8 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;

    x_strcpy( m_pData, pString );
}

//==============================================================================

xstring::xstring( const char* pString, s32 Length )
{
    ASSERT( pString );

    // See comments in EnsureCapacity for why we added 9 and aligned to 16.
    s32 Len        = Length;
    s32 BufferSize = ALIGN_16( Len + 9 );

    if (BufferSize <= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData+8;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (char*)x_malloc( BufferSize ) + 8;
        ASSERT( m_pData-8 );

    }
    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;
    x_memcpy( m_pData, pString, Length );
    m_pData[Length] = '\0';
}

//==============================================================================

xstring::xstring( const xstring& String )
{
    // See comments in EnsureCapacity for why we added 9 and aligned to 16.
    s32 Len        = String.GetLength();
    s32 BufferSize = ALIGN_16( Len + 9 );

    if (BufferSize <= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData + 8;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (char*)x_malloc( BufferSize ) + 8;
        ASSERT( m_pData-8 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;
    x_memcpy( m_pData, String.m_pData, Len+1 );
}

//==============================================================================

xstring::xstring( const xwstring& String )
{
    // See comments in EnsureCapacity for why we added 9 and aligned to 16.
    s32 i;
    s32 Len        = String.GetLength();
    s32 BufferSize = ALIGN_16( Len + 9 );

    if (BufferSize <= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData + 8;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (char*)x_malloc( BufferSize ) + 8;
        ASSERT( m_pData-8 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;
    for( i=0 ; i<Len ; i++ )
    {
        m_pData[i] = (char)String[i];
    }
    m_pData[Len] = 0;
}

//==============================================================================

xstring::~xstring( void )
{
    // Careful!  We must back the pointer off by 8.
    if( m_pData && ((m_pData - 8) != m_LocalData))   x_free( m_pData - 8 );
}
//==============================================================================

void xstring::SetLength( s32 Length )
{
    EnsureCapacity( Length );
    STRING_LENGTH = Length;
}

//==============================================================================

void xstring::Clear( void )
{
    STRING_LENGTH = 0;
    m_pData[0]    = '\0';
}

//==============================================================================

void xstring::FreeExtra( void )
{
    s32 Size = ALIGN_16( STRING_LENGTH+9 );

    if ((m_pData - 8) == m_LocalData)
        return;

    if( Size <= (s32)sizeof(m_LocalData) )
    {
        x_memcpy( m_LocalData, m_pData-8, Size );
        x_free( m_pData-8 );
        m_pData = m_LocalData+8;
        BUFFER_SIZE = sizeof(m_LocalData);
    }
    else if( Size < BUFFER_SIZE )
    {
        // Careful!  We must back the pointer off by 8, and then advance by 8.
        m_pData = (char*)x_realloc( m_pData - 8, Size ) + 8;
        ASSERT( m_pData-8 );
        BUFFER_SIZE = Size;
    }
}

//==============================================================================

void xstring::IndexToRowCol( s32 Index, s32& Row, s32& Col ) const
{
    ASSERT( Index >= 0 );

    s32 Scan = 0;
    
    if( Index > STRING_LENGTH )
    {
        Row = -1;
        Col = -1;
        return;
    }

    Row = 1;
    Col = 1;

    while( Scan < Index )
    {
        if( m_pData[Scan] == '\n' )
        {
            Row++;
            Col = 1;
        }
        else
        {
            Col++;
        }
        Scan++;
    }
}

//==============================================================================

xstring xstring::Mid( s32 Index, s32 Count ) const
{
    ASSERT( Count >= 0 );
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( (Index+Count) <= STRING_LENGTH );

    xstring Result( Count );

    x_memcpy( Result.m_pData, &m_pData[Index], Count );
    Result.m_pData[Count] = '\0';

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Count;    

    return( Result );
}

//==============================================================================

xstring xstring::Left( s32 Count ) const
{
    ASSERT( Count >= 0 );
    ASSERT( Count <= STRING_LENGTH );

    xstring Result( Count );

    x_memcpy( Result.m_pData, m_pData, Count );
    Result.m_pData[Count] = '\0';

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Count;    

    return( Result );
}

//==============================================================================

xstring xstring::Right( s32 Count ) const
{
    ASSERT( Count >= 0 );
    ASSERT( Count <= STRING_LENGTH );

    xstring Result( Count );

    x_memcpy( Result.m_pData, &m_pData[(STRING_LENGTH-Count)], Count );
    Result.m_pData[Count] = '\0';

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Count;    

    return( Result );
}

//==============================================================================

void xstring::MakeUpper( void )
{
    s32 i;
    s32 Len = STRING_LENGTH;

    for( i = 0; i < Len; i++ )
    {
        char c = m_pData[i];
        if( (c >= 'a') && (c <= 'z') )
            c += ('A'-'a');
        m_pData[i] = c;
    }
}

//==============================================================================

void xstring::MakeLower( void )
{
    s32 i;
    s32 Len = STRING_LENGTH;

    for( i = 0; i < Len; i++ )
    {
        char c = m_pData[i];
        if( (c >= 'A') && (c <= 'Z') )
            c += ('a'-'A');
        m_pData[i] = c;
    }
}

//==============================================================================

void xstring::Insert( s32 Index, char Character )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the string and the new character.
    EnsureCapacity( STRING_LENGTH + 1 );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + 1], 
               &m_pData[ Index ], 
               (STRING_LENGTH - Index + 1) );

    // Increase length appropriately.
    STRING_LENGTH += 1;

    // Insert the Character.
    m_pData[Index] = Character;
}

//==============================================================================

void xstring::Insert( s32 Index, const char* pString )
{
    s32 Len;

    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( pString );

    Len = x_strlen( pString );

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the string and the new string.
    EnsureCapacity( STRING_LENGTH + Len );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + Len ], 
               &m_pData[ Index ], 
               (STRING_LENGTH - Index + 1) );

    // Increase length appropriately.
    STRING_LENGTH += Len;

    // Insert the string.
    x_memcpy( &m_pData[Index], pString, Len );
}

//==============================================================================

void xstring::Insert( s32 Index, const xstring& String )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( this  != &String );     // Cannot insert an xstring into itself!

    s32 Len2 = String.GetLength();

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the string and the new string.
    EnsureCapacity( STRING_LENGTH + Len2 );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + Len2 ], 
               &m_pData[ Index ], 
               (STRING_LENGTH - Index + 1) );

    // Increase length appropriately.
    STRING_LENGTH += Len2;

    // Insert the string.
    x_memcpy( m_pData+Index, String.m_pData, Len2 );
}

//==============================================================================

void xstring::Delete( s32 Index, s32 Count )
{
    ASSERT( Count >= 0 );
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( (Index+Count) <= STRING_LENGTH );

    // Move the "end" of the string "over" the stuff to be deleted.
    x_memmove( &m_pData[ Index ], 
               &m_pData[ Index + Count ], 
               (STRING_LENGTH - (Index + Count) + 1) );

    // Adjust length.
    STRING_LENGTH -= Count;
}

//==============================================================================

s32 xstring::Format( const char* pFormat, ... )
{
    ASSERT( pFormat );

    x_va_list Args;
    x_va_start( Args, pFormat );

    xvfs  Temp( pFormat, Args );
    s32   Len = x_strlen( Temp );

    EnsureCapacity( Len );
    x_strcpy( m_pData, Temp );
    STRING_LENGTH = Len;

    // Return the new length.
    return( Len );
}

//==============================================================================

s32 xstring::FormatV( const char* pFormat, x_va_list Args )
{
    ASSERT( pFormat );

    xvfs  Temp( pFormat, Args );
    s32   Len = x_strlen( Temp );

    EnsureCapacity( Len );
    x_strcpy( m_pData, Temp );
    STRING_LENGTH = Len;

    // Return the new length.
    return( Len );
}

//==============================================================================

s32 xstring::AddFormat( const char* pFormat, ... )
{
    ASSERT( pFormat );

    x_va_list Args;
    x_va_start( Args, pFormat );

    xvfs  Temp( pFormat, Args );
    s32   Len = x_strlen( Temp );

    EnsureCapacity( STRING_LENGTH + Len );
    x_strcpy( &m_pData[STRING_LENGTH], Temp );
    STRING_LENGTH += Len;

    // Return the new length.
    return( Len );
}

//==============================================================================

s32 xstring::AddFormatV( const char* pFormat, x_va_list Args )
{
    ASSERT( pFormat );

    xvfs  Temp( pFormat, Args );
    s32   Len = x_strlen( Temp );

    EnsureCapacity( STRING_LENGTH + Len );
    x_strcpy( &m_pData[STRING_LENGTH], Temp );
    STRING_LENGTH += Len;

    // Return the new length.
    return( Len );
}

//==============================================================================

s32 xstring::Find( char Character, s32 StartIndex ) const
{
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STRING_LENGTH );

    char* pScan = m_pData + StartIndex;
    char* pEnd  = m_pData + STRING_LENGTH;

    while( pScan != pEnd )
    {
        if( *pScan == Character )
            return( pScan - m_pData );
        pScan++;
    }

    return( -1 );
}

//==============================================================================

s32 xstring::Find( const char* pSubString, s32 StartIndex ) const
{
    ASSERT( pSubString );
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STRING_LENGTH );

    s32   Len   = x_strlen( pSubString );

    // If the sub string is larger than the string to be searched, then there is
    // no chance to succeed.
    if( Len > (STRING_LENGTH - StartIndex) )
        return( -1 );

    char* pScan = m_pData + StartIndex;
    char* pEnd  = m_pData + STRING_LENGTH - Len + 1;

    while( pScan < pEnd )
    {
        // We must compare the sub string against the current scan point.

        char*       p1 = pScan;
        const char* p2 = pSubString;

        for( ; ; )
        {
            if( *p2 == '\0' )
            {
                // We have worked all the way through the sub string with no 
                // mismatches.  We have a winner!

                return( pScan - m_pData );
            }

            if( *p1 != *p2 )
                break;

            p1++;
            p2++;
        }

        pScan++;
    }

    return( -1 );
}

//==============================================================================

s32 xstring::Find( const xstring& SubString, s32 StartIndex ) const
{
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STRING_LENGTH );

    // If the sub string is larger than the string to be searched, then there is
    // no chance to succeed.
    s32 SubStrLen = SubString.GetLength();

    if( SubStrLen > (STRING_LENGTH - StartIndex) )
        return( -1 );

    char* pScan = m_pData + StartIndex;
    char* pEnd  = m_pData + STRING_LENGTH - SubStrLen + 1;

    while( pScan != pEnd )
    {
        // We must compare the sub string against the current scan point.

        s32   i;
        char* p1 = pScan;
        char* p2 = SubString.m_pData;

        for( i = 0; i < SubStrLen; i++ )
        {
            if( *p1 != *p2 )
                break;

            p1++;
            p2++;
        }

        // See how far we got on that last comparison run.  If we made it the 
        // full length of the sub string, then we have a winner!

        if( i == SubStrLen )
        {
            return( pScan - m_pData );
        }

        pScan++;
    }

    return( -1 );
}

//==============================================================================

const xstring& xstring::operator = ( char Character )
{
    EnsureCapacity( 1 );
    m_pData[0]    = Character;
    m_pData[1]    = '\0';
    STRING_LENGTH = 1;

    return( *this );
}

//==============================================================================

const xstring& xstring::operator = ( const char* pString )
{
    s32 Len = x_strlen( pString );
    EnsureCapacity( Len );
    x_strcpy( m_pData, pString );
    STRING_LENGTH = Len;

    return( *this );
}

//==============================================================================

const xstring& xstring::operator = ( const xstring& String )
{
    if( this != &String )
    {
        s32 Len = String.GetLength();
        EnsureCapacity( Len );
        x_memcpy( m_pData, String.m_pData, Len + 1 );
        STRING_LENGTH = Len;
    }

    return( *this );
}

//==============================================================================

const xstring& xstring::operator = ( const xwstring& String )
{
    s32 i;
    s32 Len = String.GetLength();
    EnsureCapacity( Len );
    for( i=0 ; i<Len ; i++ )
    {
        m_pData[i] = (char)String[i];
    }
    m_pData[i] = 0;
    STRING_LENGTH = Len;

    return( *this );
}

//==============================================================================

xstring operator + ( const xstring& String, char Character )
{
    s32 Len = String.GetLength();

    xstring Result( Len + 1 );

    x_memcpy( Result.m_pData, String.m_pData, Len );

    Result.m_pData[Len  ] = Character;
    Result.m_pData[Len+1] = '\0';

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Len + 1;      

    return( Result );
}

//==============================================================================

xstring operator + ( char Character, const xstring& String )
{
    s32 Len = String.GetLength();

    xstring Result( Len + 1 );

    Result.m_pData[0] = Character;
    x_memcpy( Result.m_pData+1, String.m_pData, Len + 1 );
    
    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Len + 1;

    return( Result );
}

//==============================================================================

xstring operator + ( const xstring& String, const char* pString )
{
    s32 Len1 = String.GetLength();
    s32 Len2 = x_strlen( pString );

    xstring Result( Len1 + Len2 );

    x_memcpy( Result.m_pData,        String.m_pData, Len1     );
    x_memcpy( Result.m_pData + Len1, pString,        Len2 + 1 );

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Len1 + Len2;  
    

    return( Result );
}

//==============================================================================

xstring operator + ( const char* pString, const xstring& String )
{
    s32 Len1 = x_strlen( pString );
    s32 Len2 = String.GetLength();

    xstring Result( Len1 + Len2 );

    x_memcpy( Result.m_pData,        pString,        Len1     );
    x_memcpy( Result.m_pData + Len1, String.m_pData, Len2 + 1 );

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Len1 + Len2;  

    return( Result );
}

//==============================================================================

xstring operator + ( const xstring& String1, const xstring& String2 )
{
    s32 Len1 = String1.GetLength();
    s32 Len2 = String2.GetLength();

    xstring Result( Len1 + Len2 );

    x_memcpy( Result.m_pData,        String1.m_pData, Len1     );
    x_memcpy( Result.m_pData + Len1, String2.m_pData, Len2 + 1 );

    // Set length for Result.
    *((s32*)(Result.m_pData-4)) = Len1 + Len2;  

    return( Result );
}

//==============================================================================

const xstring& xstring::operator += ( char Character )
{
    EnsureCapacity( STRING_LENGTH + 1 );
    m_pData[ STRING_LENGTH   ] = Character;
    m_pData[ STRING_LENGTH+1 ] = '\0';
    STRING_LENGTH++;

    return( *this );
}

//==============================================================================

const xstring& xstring::operator += ( const char* pString )
{
    s32 Len = x_strlen( pString );
    EnsureCapacity( STRING_LENGTH + Len );
    x_memcpy( m_pData + STRING_LENGTH, pString, Len+1 );
    STRING_LENGTH += Len;

    return( *this );
}

//==============================================================================

const xstring& xstring::operator += ( const xstring& String )
{
    // Use x_memmove in this function in case of S += S.
    // For example (let '~' represent the NULL character):
    // 
    // Before: ABC~     After: ABCABC~
    //         ^^^^\_____________/^^^^
    // 
    // The NULL originally in position [3] is overwritten.  So, the source and 
    // destinations overlap.  Thus, you can't use x_memcpy.

    s32 Len = String.GetLength();
    EnsureCapacity( STRING_LENGTH + Len );
    x_memmove( m_pData + STRING_LENGTH, String.m_pData, Len+1 );
    STRING_LENGTH += Len;

    return( *this );
}

//==============================================================================

xbool operator == ( const xstring& S1, const char* S2 )
{
    ASSERT( S2 );
    return( (S1.GetLength() == x_strlen( S2 )) &&
            (x_strcmp( S1, S2 ) == 0) );
}

//==============================================================================

xbool operator == ( const char* S1, const xstring& S2 )
{
    ASSERT( S1 );
    return( (S2.GetLength() == x_strlen( S1 )) &&
            (x_strcmp( S1, S2 ) == 0) );
}

//==============================================================================

xbool operator == ( const xstring& S1, const xstring& S2 )
{
    // Use x_memcmp rather than x_strcmp since there may be embedded NULLs.
    return( (S1.GetLength() == S2.GetLength()) &&
            (x_memcmp( S1.m_pData, S2.m_pData, S1.GetLength() ) == 0) );
}

//==============================================================================

xbool operator != ( const xstring& S1, const char* S2 )
{
    ASSERT( S2 );
    return( (S1.GetLength() != x_strlen( S2 )) ||
            (x_strcmp( S1, S2 ) != 0) );
}

//==============================================================================

xbool operator != ( const char* S1, const xstring& S2 )
{
    ASSERT( S1 );
    return( (S2.GetLength() != x_strlen( S1 )) ||
            (x_strcmp( S1, S2 ) != 0) );
}

//==============================================================================

xbool operator != ( const xstring& S1, const xstring& S2 )
{
    // Use x_memcmp rather than x_strcmp since there may be embedded NULLs.
    return( (S1.GetLength() != S2.GetLength()) ||
            (x_memcmp( S1.m_pData, S2.m_pData, S1.GetLength() ) != 0) );
}

//==============================================================================
//  
//  Comparisons with xstrings cen be a little tricky since embedded NULLs are
//  allowed.  Consider the following strings.  Let a '~' represent a NULL ('\0')
//  character.  Remember that ALL xstrings have a terminating NULL (which is
//  shown in the data below).
//  
//    (A) xstring -- "XYZ~" -- length=3
//    (B) xstring -- "X~Z~" -- length=3
//    (C) xstring -- "X~Z~" -- length=1    (The "Z~" is old, garbage data.)
//    (D) xstring -- "X~"   -- length=1
//
//    (E) char*   -- "XYZ~" -- length=3
//    (F) char*   -- "X~Z~" -- length=1    (The "Z~" is old, garbage data.)
//    (G) char*   -- "X~"   -- length=1    
//  
//  String (C) is particularly dangerous (and perfectly legal).  String (F) can
//  be trouble, too.
//  
//  The functions which follow will note cases from the above sample strings.
//
//==============================================================================

xbool operator < ( const xstring& S1, const char* S2 )
{
    // Cases to note:
    //  (B) < (G) -- FALSE
    //  (C) < (G) -- FALSE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 < S2 only if it is shorter.
        return( S1.GetLength() < s32(x_strlen(S2)) );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator < ( const char* S1, const xstring& S2 )
{
    // Cases to note:
    //  (G) < (B) -- TRUE
    //  (G) < (C) -- FALSE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 < S2 only if it is shorter.
        return( s32(x_strlen(S1)) < S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator < ( const xstring& S1, const xstring& S2 )
{
    // Cases to note:
    //  (B) < (C) -- FALSE
    //  (C) < (B) -- TRUE
    //  (D) < (C) -- FALSE
    //  (A) < (A) -- FALSE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           MIN( S1.GetLength(), S2.GetLength() ) );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 < S2 only if it is shorter.
        return( S1.GetLength() < S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator > ( const xstring& S1, const char* S2 )
{
    // Cases to note:
    //  (B) > (G) -- TRUE
    //  (C) > (G) -- FALSE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 > S2 only if it is longer.
        return( S1.GetLength() > s32(x_strlen(S2)) );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator > ( const char* S1, const xstring& S2 )
{
    // Cases to note:
    //  (G) > (B) -- FALSE
    //  (G) > (C) -- FALSE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 > S2 only if it is longer.
        return( s32(x_strlen(S1)) > S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator > ( const xstring& S1, const xstring& S2 )
{
    // Cases to note:
    //  (B) > (C) -- TRUE
    //  (C) > (B) -- FALSE
    //  (D) > (C) -- FALSE
    //  (A) > (A) -- FALSE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           MIN( S1.GetLength(), S2.GetLength() ) );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 > S2 only if it is longer.
        return( S1.GetLength() > S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator <= ( const xstring& S1, const char* S2 )
{
    // Cases to note:
    //  (B) <= (G) -- FALSE
    //  (C) <= (G) -- TRUE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 <= S2 only if it is the same length or shorter.
        return( S1.GetLength() <= s32(x_strlen(S2)) );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator <= ( const char* S1, const xstring& S2 )
{
    // Cases to note:
    //  (G) <= (B) -- TRUE
    //  (G) <= (C) -- TRUE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 <= S2 only if it is the same length or shorter.
        return( s32(x_strlen(S1)) <= S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator <= ( const xstring& S1, const xstring& S2 )
{
    // Cases to note:
    //  (B) <= (C) -- FALSE
    //  (C) <= (B) -- TRUE
    //  (D) <= (C) -- TRUE
    //  (A) <= (A) -- TRUE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           MIN( S1.GetLength(), S2.GetLength() ) );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 <= S2 only if it is the same length or shorter.
        return( S1.GetLength() <= S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator >= ( const xstring& S1, const char* S2 )
{
    // Cases to note:
    //  (B) >= (G) -- TRUE
    //  (C) >= (G) -- TRUE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 >= S2 only if it is the same length or longer.
        return( S1.GetLength() >= s32(x_strlen(S2)) );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator >= ( const char* S1, const xstring& S2 )
{
    // Cases to note:
    //  (G) >= (B) -- FALSE
    //  (G) >= (C) -- TRUE    

    s32 Relate = x_strcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 >= S2 only if it is the same length or longer.
        return( s32(x_strlen(S1)) >= S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator >= ( const xstring& S1, const xstring& S2 )
{
    // Cases to note:
    //  (B) >= (C) -- TRUE
    //  (C) >= (B) -- FALSE
    //  (D) >= (C) -- TRUE
    //  (A) >= (A) -- TRUE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           MIN( S1.GetLength(), S2.GetLength() ) );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 >= S2 only if it is the same length or longer.
        return( S1.GetLength() >= S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

void xstring::Dump( xbool LineFeed ) const
{
    s32 i;
    s32 Len = STRING_LENGTH;

    for( i = 0; i < Len; i++ )
        x_printf( "%c", m_pData[i] >= 32 ? m_pData[i] : '.' );

    if( LineFeed )
        x_printf( "\n" );
}

//==============================================================================

void xstring::DumpHex( void ) const
{
    //            1         2         3
    //  01234567890123456789012345678901234
    // "XX XX XX XX XX XX XX XX - XXXXXXXX"

    char Text[35];
    char Hex[] = "0123456789ABCDEF";
    s32  i;
    s32  j   = 0;
    s32  Len = STRING_LENGTH;

    x_memset( Text, ' ', 34 );
    Text[34] = '\0';
    Text[24] = '-';

    for( i = 0; i < Len; i++ )
    {
        j = i & 7;
        Text[j*3+0] = Hex[ (m_pData[i] >> 4) & 15 ];
        Text[j*3+1] = Hex[ (m_pData[i] >> 0) & 15 ];

        if( m_pData[i] >= 32 )
            Text[j+26] = m_pData[i];
        else
            Text[j+26] = '.';

        if( j == 7 )
        {
            x_printf( "%s\n", Text );
            x_memset( Text, ' ', 34 );
            Text[24] = '-';
        }
    }

    if( j != 7 )
        x_printf( "%s\n", Text );
}

//==============================================================================

xbool xstring::LoadFile( const char* pFileName )
{
    s32     Size;
    s32     Read;
    X_FILE* pFile = x_fopen( pFileName, "rb" );

    if( !pFile )
        return( FALSE );

    Size = x_flength( pFile );
    EnsureCapacity( Size );
    Read = x_fread( m_pData, 1, Size, pFile );
    STRING_LENGTH = Read;
    m_pData[Read] = '\0';
    x_fclose( pFile );

    return( Read == Size );
}

//==============================================================================

xbool xstring::SaveFile( const char* pFileName, xbool Append ) const
{
    X_FILE* pFile = NULL;
    s32     NChars;
    s32     Written = 0;

#if defined(TARGET_PC)
    
    if( Append )
        pFile = x_fopen( pFileName, "at" );
    else
        pFile = x_fopen( pFileName, "wt" );

    if( !pFile )
        return( FALSE );

    NChars = GetLength();
    if( NChars > 0 )
    {
        Written = x_fwrite( (const byte*)(const char*)m_pData, 1, NChars, pFile );
    }

    x_fclose( pFile );
#else

    if( Append )
        pFile = x_fopen( pFileName, "ab" );
    else
        pFile = x_fopen( pFileName, "wb" );

    if( !pFile )
        return( FALSE );

    // Replace \n with \r\n
    xstring TempStr( (s32)(GetLength()*2) );
    TempStr = *this;
    s32 Index=0;
    while( (Index < TempStr.GetLength() ) && 
           ((Index = TempStr.Find( '\n', Index )) != -1) )
    {
        TempStr.Insert( Index, "\r" );
        Index+=2;
    }

    NChars = TempStr.GetLength();
    if( NChars > 0 )
    {
        Written = x_fwrite( (const byte*)(const char*)TempStr, 1, NChars, pFile );
    }

    x_fclose( pFile );
#endif

    return( Written == NChars );
}

//==============================================================================
//  IMPLEMENTATION NOTES NOTES FOR CLASS 'xwstring'
//==============================================================================
//
//  Class xwstring is based on the xstring implementation.
//
//  Careful!  In xstring, since we were working with chars, we had the luxory of
//  coding with the assuming that sizeof(char) = sizeof(byte).  In xwstring, we
//  are dealing with wide characters.  Even the NULL is 2 bytes wide!  When we
//  need to "back the pointer off" in order to access the BufferSize and 
//  StringLength, we back off by 4 since the pointer is to xwchar.
//
//  Make sure to use wide versions of functions on C strings (x_wstrcpy) and to
//  double number of bytes when using memory functions as needed, for example:
//
//      x_memcpy( a, b, Count << 1 );
//
//
//      xstring
//      +-----------+
//      | m_pData---=-----------------------------+
//      | (xwchar*) |                             |
//      +-----------+                             |
//                                                V
//                  +--------------------------------------------------------+
//                  | BufferSize | StringLength | string character data...   |
//                  | (s32)      | (s32)        | (xwchar[])                 |
//                  +--------------------------------------------------------+
//  
//  Macros BUFFER_SIZE and STRING_LENGTH from xstring work in xwstring, too.
//  
//==============================================================================

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xwstring'
//==============================================================================

void xwstring::EnsureCapacity( s32 NewCapacity )
{
    s32  NewBufferSize;
    s32     OldBufferSize;

    // Since there is always a terminating NULL and we store the buffer size and
    // string length in the buffer, it must be at least 10 bytes larger than the 
    // size of the requested capacity.  (That's 8 bytes for the to values, and 
    // more for the terminating NULL.)  Furthermore, in order to reduce heap 
    // activity, the allocation is rounded up to a multiple of 16.

    OldBufferSize = BUFFER_SIZE;
    NewBufferSize = ALIGN_16( (NewCapacity << 1) + 10 );
    NewCapacity   = (NewBufferSize - 10) >> 1;

    if( NewBufferSize > BUFFER_SIZE )
    {
        if ( (m_pData - 4) == m_LocalData)
        {
            m_pData = (xwchar*)x_malloc( NewBufferSize ) + 4;
            ASSERT(m_pData);
            x_memcpy(m_pData-4,m_LocalData,OldBufferSize);
        }
        else
        {
            // Careful!  We must back the pointer off by 4, and then advance by 4.
            m_pData = (xwchar*)x_realloc( m_pData - 4, NewBufferSize ) + 4;
            ASSERT( m_pData-4 );
        }
        BUFFER_SIZE = NewBufferSize;
    }
}

//==============================================================================

xwstring::xwstring( void )
{
    m_pData          = m_LocalData+4;
    BUFFER_SIZE   = sizeof(m_LocalData);
    STRING_LENGTH =  0;
    m_pData[0]    = NULL;
}

//==============================================================================

xwstring::xwstring( s32 Reserve )
{
    // See comments in EnsureCapacity for why we added 10 and aligned to 16.
    s32 BufferSize = ALIGN_16( (Reserve << 1) + 10 );

    if (BufferSize <= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData+4;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (xwchar*)x_malloc( BufferSize ) + 4;
        ASSERT( m_pData-4 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = 0;
    m_pData[0]    = NULL;
}

//==============================================================================

xwstring::xwstring( xwchar WideChar )
{
    m_pData          = m_LocalData+4;
    BUFFER_SIZE   = sizeof(m_LocalData);
    STRING_LENGTH =  1;
    m_pData[0]    = WideChar;
    m_pData[1]    = NULL;
}

//==============================================================================

xwstring::xwstring( const xwchar* pWideString )
{
    ASSERT( pWideString );

    // See comments in EnsureCapacity for why we added 10 and aligned to 16.
    s32 Len        = x_wstrlen( pWideString );
    s32 BufferSize = ALIGN_16( (Len << 1) + 10 );

    if (BufferSize<= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData+4;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (xwchar*)x_malloc( BufferSize ) + 4;
        ASSERT( m_pData-4 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;
    x_wstrcpy( m_pData, pWideString );
}

//==============================================================================

xwstring::xwstring( const char* pString )
{
    ASSERT( pString );

    // See comments in EnsureCapacity for why we added 10 and aligned to 16.
    s32 Len        = pString ? x_strlen( pString ) : 0;
    s32 BufferSize = ALIGN_16( (Len << 1) + 10 );

    if (BufferSize<= (s32)sizeof(m_LocalData))
    {
        m_pData = m_LocalData+4;
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (xwchar*)x_malloc( BufferSize ) + 4;
        ASSERT( m_pData-4 );
    }

    s32 i=0;
    for( ; i<Len ; i++ )
    {
        m_pData[i] = (u8)pString[i];
    }
    m_pData[i] = 0;

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;
}

//==============================================================================

xwstring::xwstring( const xwstring& WideString )
{
    // See comments in EnsureCapacity for why we added 10 and aligned to 16.
    s32 Len        = WideString.GetLength();
    s32 BufferSize = ALIGN_16( (Len << 1) + 10 );

    if (BufferSize<= (s32)sizeof(m_LocalData))
    {
        m_pData = &m_LocalData[4];
        BufferSize = sizeof(m_LocalData);
    }
    else
    {
        m_pData = (xwchar*)x_malloc( BufferSize ) + 4;
        ASSERT( m_pData-4 );
    }

    BUFFER_SIZE   = BufferSize;
    STRING_LENGTH = Len;
    x_memcpy( m_pData, WideString.m_pData, (Len+1) << 1 );
}

//==============================================================================

xwstring::~xwstring( void )
{
    // Careful!  We must back the pointer off by 4.
    if( m_pData && ( (m_pData-4) != m_LocalData) )   x_free( m_pData - 4 );
}

//==============================================================================

void xwstring::Clear( void )
{
    STRING_LENGTH = 0;
    m_pData[0]    = NULL;
}

//==============================================================================

void xwstring::FreeExtra( void )
{
    // See comments in EnsureCapacity for why we added 10 and aligned to 16.
    s32 Size = ALIGN_16( (STRING_LENGTH << 1) + 10 );

    if ( (m_pData - 4 ) == m_LocalData)
        return;

    if( Size <= (s32)sizeof(m_LocalData) )
    {
        x_memcpy( m_LocalData, m_pData-4, Size );
        x_free( m_pData-4 );
        m_pData = m_LocalData+4;
        BUFFER_SIZE = sizeof(m_LocalData);
    }
    else if( Size < BUFFER_SIZE )
    {
        // Careful!  We must back the pointer off by 4, and then advance by 4.
        m_pData = (xwchar*)x_realloc( m_pData - 4, Size ) + 4;
        ASSERT( m_pData-4 );
        BUFFER_SIZE = Size;
    }
}

//==============================================================================

s32 xwstring::GetHashKey( s32 Index, s32 Count ) const
{
    ASSERT( Count >  0 );
    ASSERT( Count >= 0 );
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( (Index+Count) <= STRING_LENGTH );

    s32     HashKey = 0;

    while( Count-- > 0 )
    {
        HashKey = (HashKey << 4) ^ (HashKey >> 28) + m_pData[Index++];
    }

    return HashKey;
}

//==============================================================================

xwstring xwstring::Mid( s32 Index, s32 Count ) const
{
    ASSERT( Count >= 0 );
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( (Index+Count) <= STRING_LENGTH );

    xwstring Result( Count );

    x_memcpy( Result.m_pData, &m_pData[Index], Count << 1 );
    Result.m_pData[Count] = NULL;

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Count;    

    return( Result );
}

//==============================================================================

xwstring xwstring::Left( s32 Count ) const
{
    ASSERT( Count >= 0 );
    ASSERT( Count <= STRING_LENGTH );

    xwstring Result( Count );

    x_memcpy( Result.m_pData, m_pData, Count << 1 );
    Result.m_pData[Count] = NULL;

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Count;    

    return( Result );
}

//==============================================================================

xwstring xwstring::Right( s32 Count ) const
{
    ASSERT( Count >= 0 );
    ASSERT( Count <= STRING_LENGTH );

    xwstring Result( Count );

    x_memcpy( Result.m_pData, &m_pData[(STRING_LENGTH-Count)], Count * sizeof(xwchar) );
    Result.m_pData[Count] = NULL;

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Count;    

    return( Result );
}

//==============================================================================

void xwstring::Insert( s32 Index, xwchar WideChar )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the string and the new character.
    EnsureCapacity( STRING_LENGTH + 1 );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + 1 ], 
               &m_pData[ Index ], 
               (STRING_LENGTH - Index + 1) << 1 );

    // Increase length appropriately.
    STRING_LENGTH += 1;

    // Insert the WideChar.
    m_pData[Index] = WideChar;
}

//==============================================================================

void xwstring::Insert( s32 Index, const xwchar* pWideString )
{
    s32 Len;

    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( pWideString );

    Len = x_wstrlen( pWideString );

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the string and the new string.
    EnsureCapacity( STRING_LENGTH + Len );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + Len ], 
               &m_pData[ Index ], 
               (STRING_LENGTH - Index + 1) << 1 );

    // Increase length appropriately.
    STRING_LENGTH += Len;

    // Insert the string.
    x_memcpy( &m_pData[Index], pWideString, Len << 1 );
}

//==============================================================================

void xwstring::Insert( s32 Index, const xwstring& WideString )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( this  != &WideString );     // Cannot insert an xwstring into self!

    s32 Len = WideString.GetLength();

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the string and the new string.
    EnsureCapacity( STRING_LENGTH + Len );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + Len ], 
               &m_pData[ Index ], 
               (STRING_LENGTH - Index + 1) << 1 );

    // Increase length appropriately.
    STRING_LENGTH += Len;

    // Insert the string.
    x_memcpy( m_pData+Index, WideString.m_pData, Len << 1 );
}

//==============================================================================

void xwstring::Delete( s32 Index, s32 Count )
{
    ASSERT( Count >= 0 );
    ASSERT( Index >= 0 );
    ASSERT( Index <= STRING_LENGTH );
    ASSERT( (Index+Count) <= STRING_LENGTH );

    // Move the "end" of the string "over" the stuff to be deleted.
    x_memmove( &m_pData[ Index ], 
               &m_pData[ Index + Count ], 
               (STRING_LENGTH - (Index + Count) + 1) << 1 );

    // Adjust length.
    STRING_LENGTH -= Count;
}

//==============================================================================

s32 xwstring::Find( xwchar WideChar, s32 StartIndex ) const
{
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STRING_LENGTH );

    xwchar* pScan = m_pData + StartIndex;
    xwchar* pEnd  = m_pData + STRING_LENGTH;

    while( pScan != pEnd )
    {
        if( *pScan == WideChar )
            return( pScan - m_pData );
        pScan++;
    }

    return( -1 );
}

//==============================================================================

s32 xwstring::Find( const xwchar* pWideSubString, s32 StartIndex ) const
{
    ASSERT( pWideSubString );
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STRING_LENGTH );

    s32     Len   = x_wstrlen( pWideSubString );

    //If the length of the sub string is longer than our remaining length, then obviously the 
    //sub string cannot be in our string.
    if (Len > (STRING_LENGTH - StartIndex) )
        return -1;

    xwchar* pScan = m_pData + StartIndex;
    xwchar* pEnd  = m_pData + STRING_LENGTH - Len + 1;

    while( pScan != pEnd )
    {
        // We must compare the sub string against the current scan point.

        xwchar*       p1 = pScan;
        const xwchar* p2 = pWideSubString;

        for( ; ; )
        {
            if( *p2 == NULL )
            {
                // We have worked all the way through the sub string with no 
                // mismatches.  We have a winner!

                return( pScan - m_pData );
            }

            if( *p1 != *p2 )
                break;

            p1++;
            p2++;
        }

        pScan++;
    }

    return( -1 );
}

//==============================================================================

s32 xwstring::Find( const xwstring& WideSubString, s32 StartIndex ) const
{
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STRING_LENGTH );

    // If the sub string is larger than the string to be searched, then there is
    // no chance to succeed.

    s32 SubStrLen = WideSubString.GetLength();

    if( SubStrLen > (STRING_LENGTH - StartIndex) )
        return( -1 );

    xwchar* pScan = m_pData + StartIndex;
    xwchar* pEnd  = m_pData + STRING_LENGTH - SubStrLen + 1;

    while( pScan != pEnd )
    {
        // We must compare the sub string against the current scan point.

        s32   i;
        xwchar* p1 = pScan;
        xwchar* p2 = WideSubString.m_pData;

        for( i = 0; i < SubStrLen; i++ )
        {
            if( *p1 != *p2 )
                break;

            p1++;
            p2++;
        }

        // See how far we got on that last comparison run.  If we made it the 
        // full length of the sub string, then we have a winner!

        if( i == SubStrLen )
        {
            return( pScan - m_pData );
        }

        pScan++;
    }

    return( -1 );
}

//==============================================================================

xbool xwstring::LoadFile( const char* pFileName )
{
    xbool   Success = FALSE;
    s32     Size;
    s32     Read;
    s32     StringLen;
    u8      Data;
    X_FILE* pFile = x_fopen( pFileName, "rb" );

    if( !pFile )
        return( FALSE );

    Size      = x_flength( pFile );
    StringLen = (Size-2)/2;

    Read = x_fread( &Data, 1, 1, pFile );
    if( (Read == 1) && (Data == 0xFF) )
    {
        Read = x_fread( &Data, 1, 1, pFile );
        if( (Read == 1) && (Data == 0xFE) )
        {
            EnsureCapacity( StringLen );
            Read = x_fread( m_pData, 2, StringLen, pFile );
            STRING_LENGTH = Read;
            m_pData[Read] = '\0';
            Success = (Read == StringLen );
        }
    }

    x_fclose( pFile );

    // Failed to read as a UNICODE text file, try standard ascii and convert
    if( !Success )
    {
        xstring Temp;
        if( Temp.LoadFile( pFileName ) )
        {
            *this = Temp;
            Success = TRUE;
        }
    }

    return Success;
}

//==============================================================================

xbool xwstring::SaveFile( const char* pFileName ) const
{
    xbool       Success = FALSE;
    s32         Written;
    u8          Data;
    xwstring    TempStr( *this );

    X_FILE* pFile = x_fopen( pFileName, "wb" );

    if( !pFile )
        return( FALSE );

    s32 NChars = TempStr.GetLength();
    
    // Write Header
    Data = 0xFF; Written = x_fwrite( &Data, 1, 1, pFile );
    if( Written == 1 )
    {
        Data = 0xFE; Written = x_fwrite( &Data, 1, 1, pFile );
        if( Written == 1 )
        {
            Written = x_fwrite( (const byte*)(const xwchar*)TempStr, 2, NChars, pFile );
            Success = (Written == NChars);
        }
    }

    x_fclose( pFile );

    return Success;
}

//==============================================================================

const xwstring& xwstring::operator = ( xwchar WideChar )
{
    EnsureCapacity( 1 );
    m_pData[0]    = WideChar;
    m_pData[1]    = NULL;
    STRING_LENGTH = 1;

    return( *this );
}

//==============================================================================

const xwstring& xwstring::operator = ( const xwchar* pWideString )
{
    ASSERT( pWideString );
    s32 Len = x_wstrlen( pWideString );
    EnsureCapacity( Len );
    x_wstrcpy( m_pData, pWideString );
    STRING_LENGTH = Len;

    return( *this );
}

//==============================================================================

const xwstring& xwstring::operator = ( const xwstring& WideString )
{
    if( this != &WideString )
    {
        s32 Len = WideString.GetLength();
        EnsureCapacity( Len );
        x_memcpy( m_pData, WideString.m_pData, (Len + 1) << 1 );
        STRING_LENGTH = Len;
    }

    return( *this );
}

//==============================================================================

const xwstring& xwstring::operator = ( const xstring& String )
{
    s32 i;
    s32 Len = String.GetLength();
    EnsureCapacity( Len );
    for( i=0 ; i<String.GetLength() ; i++ )
        m_pData[i] = (u8)String[i];
    m_pData[i] = 0;
    STRING_LENGTH = Len;

    return( *this );
}

//==============================================================================

xwstring operator + ( const xwstring& WideString, xwchar WideChar )
{
    s32 Len = WideString.GetLength();

    xwstring Result( Len + 1 );

    x_memcpy( Result.m_pData, WideString.m_pData, Len << 1 );

    Result.m_pData[Len  ] = WideChar;
    Result.m_pData[Len+1] = NULL;

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Len + 1;      

    return( Result );
}

//==============================================================================

xwstring operator + ( xwchar WideChar, const xwstring& WideString )
{
    s32 Len = WideString.GetLength();

    xwstring Result( Len + 1 );

    Result.m_pData[0] = WideChar;
    x_memcpy( Result.m_pData+1, WideString.m_pData, (Len + 1) << 1 );
    
    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Len + 1;

    return( Result );
}

//==============================================================================

xwstring operator + ( const xwstring& WideString, const xwchar* pWideString )
{
    ASSERT( pWideString );
    s32 Len1 = WideString.GetLength();
    s32 Len2 = x_wstrlen( pWideString );

    xwstring Result( Len1 + Len2 );

    x_memcpy( Result.m_pData,        WideString.m_pData, (Len1    ) << 1 );
    x_memcpy( Result.m_pData + Len1, pWideString,        (Len2 + 1) << 1 );

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Len1 + Len2;  
    

    return( Result );
}

//==============================================================================

xwstring operator + ( const xwchar* pWideString, const xwstring& WideString )
{
    ASSERT( pWideString );

    s32 Len1 = x_wstrlen( pWideString );
    s32 Len2 = WideString.GetLength();

    xwstring Result( Len1 + Len2 );

    x_memcpy( Result.m_pData,        pWideString,        (Len1    ) << 1 );
    x_memcpy( Result.m_pData + Len1, WideString.m_pData, (Len2 + 1) << 1 );

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Len1 + Len2;  

    return( Result );
}

//==============================================================================

xwstring operator + ( const xwstring& String1, const xwstring& String2 )
{
    s32 Len1 = String1.GetLength();
    s32 Len2 = String2.GetLength();

    xwstring Result( Len1 + Len2 );

    x_memcpy( Result.m_pData,        String1.m_pData, (Len1    ) << 1 );
    x_memcpy( Result.m_pData + Len1, String2.m_pData, (Len2 + 1) << 1 );

    // Set length for Result.
    *(((s32*)(Result.m_pData)) - 1) = Len1 + Len2;  

    return( Result );
}

//==============================================================================

const xwstring& xwstring::operator += ( xwchar WideChar )
{
    EnsureCapacity( STRING_LENGTH + 1 );
    m_pData[ STRING_LENGTH   ] = WideChar;
    m_pData[ STRING_LENGTH+1 ] = NULL;
    STRING_LENGTH++;

    return( *this );
}

//==============================================================================

const xwstring& xwstring::operator += ( const xwchar* pWideString )
{
    ASSERT( pWideString );

    s32 Len = x_wstrlen( pWideString );
    EnsureCapacity( STRING_LENGTH + Len );
    x_memcpy( m_pData + STRING_LENGTH, pWideString, (Len+1) << 1 );
    STRING_LENGTH += Len;

    return( *this );
}

//==============================================================================

const xwstring& xwstring::operator += ( const xwstring& WideString )
{
    // Use x_memmove in this function in case of S += S.
    // For example (let '~' represent the NULL character):
    // 
    // Before: ABC~     After: ABCABC~
    //         ^^^^\_____________/^^^^
    // 
    // The NULL originally in position [3] is overwritten.  So, the source and 
    // destinations overlap.  Thus, you can't use x_memcpy.

    s32 Len = WideString.GetLength();
    EnsureCapacity( STRING_LENGTH + Len );
    x_memmove( m_pData + STRING_LENGTH, WideString.m_pData, (Len+1) << 1 );
    STRING_LENGTH += Len;

    return( *this );
}

//==============================================================================

xbool operator == ( const xwstring& S1, const xwchar* S2 )
{
    ASSERT( S2 );
    return( (S1.GetLength() == x_wstrlen( S2 )) &&
            (x_wstrcmp( S1, S2 ) == 0) );
}

//==============================================================================

xbool operator == ( const xwchar* S1, const xwstring& S2 )
{
    ASSERT( S1 );
    return( (S2.GetLength() == x_wstrlen( S1 )) &&
            (x_wstrcmp( S1, S2 ) == 0) );
}

//==============================================================================

xbool operator == ( const xwstring& S1, const xwstring& S2 )
{
    // Use x_memcmp rather than x_strcmp since there may be embedded NULLs.
    return( (S1.GetLength() == S2.GetLength()) &&
            (x_memcmp( S1.m_pData, S2.m_pData, (S1.GetLength()) << 1 ) == 0) );
}

//==============================================================================

xbool operator != ( const xwstring& S1, const xwchar* S2 )
{
    ASSERT( S2 );
    return( (S1.GetLength() != x_wstrlen( S2 )) ||
            (x_wstrcmp( S1, S2 ) != 0) );
}

//==============================================================================

xbool operator != ( const xwchar* S1, const xwstring& S2 )
{
    ASSERT( S1 );
    return( (S2.GetLength() != x_wstrlen( S1 )) ||
            (x_wstrcmp( S1, S2 ) != 0) );
}

//==============================================================================

xbool operator != ( const xwstring& S1, const xwstring& S2 )
{
    // Use x_memcmp rather than x_strcmp since there may be embedded NULLs.
    return( (S1.GetLength() != S2.GetLength()) ||
            (x_memcmp( S1.m_pData, S2.m_pData, (S1.GetLength()) << 1 ) != 0) );
}

//==============================================================================
//  
//  It may seem as though these functions are jumping through some unusual 
//  hoops.  Please read the comment block above the comparision functions for
//  class xstring.  The same logic is used for xwstring.
//
//==============================================================================

xbool operator < ( const xwstring& S1, const xwchar* S2 )
{
    // Cases to note:
    //  (B) < (G) -- FALSE
    //  (C) < (G) -- FALSE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 < S2 only if it is shorter.
        return( S1.GetLength() < x_wstrlen(S2) );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator < ( const xwchar* S1, const xwstring& S2 )
{
    // Cases to note:
    //  (G) < (B) -- TRUE
    //  (G) < (C) -- FALSE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 < S2 only if it is shorter.
        return( x_wstrlen(S1) < S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator < ( const xwstring& S1, const xwstring& S2 )
{
    // Cases to note:
    //  (B) < (C) -- FALSE
    //  (C) < (B) -- TRUE
    //  (D) < (C) -- FALSE
    //  (A) < (A) -- FALSE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           (MIN( S1.GetLength(), S2.GetLength() )) << 1 );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 < S2 only if it is shorter.
        return( S1.GetLength() < S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator > ( const xwstring& S1, const xwchar* S2 )
{
    // Cases to note:
    //  (B) > (G) -- TRUE
    //  (C) > (G) -- FALSE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 > S2 only if it is longer.
        return( S1.GetLength() > x_wstrlen(S2) );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator > ( const xwchar* S1, const xwstring& S2 )
{
    // Cases to note:
    //  (G) > (B) -- FALSE
    //  (G) > (C) -- FALSE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 > S2 only if it is longer.
        return( x_wstrlen(S1) > S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator > ( const xwstring& S1, const xwstring& S2 )
{
    // Cases to note:
    //  (B) > (C) -- TRUE
    //  (C) > (B) -- FALSE
    //  (D) > (C) -- FALSE
    //  (A) > (A) -- FALSE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           (MIN( S1.GetLength(), S2.GetLength() )) << 1 );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 > S2 only if it is longer.
        return( S1.GetLength() > S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator <= ( const xwstring& S1, const xwchar* S2 )
{
    // Cases to note:
    //  (B) <= (G) -- FALSE
    //  (C) <= (G) -- TRUE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 <= S2 only if it is the same length or shorter.
        return( S1.GetLength() <= x_wstrlen(S2) );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator <= ( const xwchar* S1, const xwstring& S2 )
{
    // Cases to note:
    //  (G) <= (B) -- TRUE
    //  (G) <= (C) -- TRUE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 <= S2 only if it is the same length or shorter.
        return( x_wstrlen(S1) <= S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator <= ( const xwstring& S1, const xwstring& S2 )
{
    // Cases to note:
    //  (B) <= (C) -- FALSE
    //  (C) <= (B) -- TRUE
    //  (D) <= (C) -- TRUE
    //  (A) <= (A) -- TRUE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           (MIN( S1.GetLength(), S2.GetLength() )) << 1 );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 <= S2 only if it is the same length or shorter.
        return( S1.GetLength() <= S2.GetLength() );
    }
    else
    {
        return( Relate < 0 );
    }
}

//==============================================================================

xbool operator >= ( const xwstring& S1, const xwchar* S2 )
{
    // Cases to note:
    //  (B) >= (G) -- TRUE
    //  (C) >= (G) -- TRUE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S2.
        // S1 >= S2 only if it is the same length or longer.
        return( S1.GetLength() >= x_wstrlen(S2) );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator >= ( const xwchar* S1, const xwstring& S2 )
{
    // Cases to note:
    //  (G) >= (B) -- FALSE
    //  (G) >= (C) -- TRUE    

    s32 Relate = x_wstrcmp( S1, S2 );

    if( Relate == 0 )
    {
        // Strings are the same up to length of S1.
        // S1 >= S2 only if it is the same length or longer.
        return( x_wstrlen(S1) >= S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

xbool operator >= ( const xwstring& S1, const xwstring& S2 )
{
    // Cases to note:
    //  (B) >= (C) -- TRUE
    //  (C) >= (B) -- FALSE
    //  (D) >= (C) -- TRUE
    //  (A) >= (A) -- TRUE

    s32 Relate = x_memcmp( S1.m_pData, 
                           S2.m_pData, 
                           (MIN( S1.GetLength(), S2.GetLength() )) << 1 );
    if( Relate == 0 )
    {
        // Strings are the same up to the SHORTER length.
        // S1 >= S2 only if it is the same length or longer.
        return( S1.GetLength() >= S2.GetLength() );
    }
    else
    {
        return( Relate > 0 );
    }
}

//==============================================================================

void xwstring::DumpHex( void ) const
{
    //            1         2         3         4
    //  01234567890123456789012345678901234567890
    // "XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX"

    xwchar Text[40];
    char   Hex[] = "0123456789ABCDEF";
    s32  i;
    s32  j   = 0;
    s32  Len = STRING_LENGTH;

    x_memset( Text, ' ', 39 );
    Text[39] = NULL;

    for( i = 0; i < Len; i++ )
    {
        j = i & 7;
        Text[j*5+0] = Hex[ (m_pData[i] >> 12) & 15 ];
        Text[j*5+1] = Hex[ (m_pData[i] >>  8) & 15 ];
        Text[j*5+2] = Hex[ (m_pData[i] >>  4) & 15 ];
        Text[j*5+3] = Hex[ (m_pData[i] >>  0) & 15 ];

        if( j == 7 )
        {
            x_printf( "%s\n", Text );
            x_memset( Text, ' ', 39 );
            Text[39] = NULL;
        }
    }

    if( j != 7 )
        x_printf( "%s\n", Text );
}

//==============================================================================
