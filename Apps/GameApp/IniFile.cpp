//==============================================================================
//
//  IniFile.cpp 
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "IniFile.hpp"
#include "x_plus.hpp"

#ifndef CONFIG_RETAIL

//==============================================================================
//  DEFINES
//==============================================================================

#define TAB                  9
#define SPACE               32

//==============================================================================
//  FUNCTIONS
//==============================================================================

ini_file::ini_file( void )
{
} 

//==============================================================================

ini_file::~ini_file( void )
{
}

//==============================================================================

xbool ini_file::Load( const char* pFileName )
{    
    m_Data.Clear();
    if( !m_Data.LoadFile( pFileName ) )
    {
        return( FALSE );
    }

    //
    // Clobber all comments.
    // Clobber all '\n'.
    // Remove trailing whitespace from all data values.
    //
    {
        s32   i;
        s32   End = m_Data.GetLength();
        xbool InComment = FALSE;

        for( i = 0; i < End; i++ )
        {
            // Start a comment?
            if( !InComment && (m_Data[i] == ';') )
            {
                InComment = TRUE;
                m_Data[i] = '\0';

                // Nuke white space backwards up to data.
                s32 j = i-1;
                while( (j >= 0) && (x_isspace(m_Data[j])) )
                {
                    m_Data[j] = '\0';
                    j--;
                }

                continue;
            }

            // End of line?
            if( (m_Data[i] == 0x0A) || (m_Data[i] == 0x0D) )
            {
                // No longer in a comment.
                InComment = FALSE;
                m_Data[i] = '\0';

                // Nuke white space backwards up to data.
                s32 j = i-1;
                while( (j >= 0) && (x_isspace(m_Data[j])) )
                {
                    m_Data[j] = '\0';
                    j--;
                }

                continue;
            }

            if( InComment )
            {
                m_Data[i] = '\0';
            }
        }
    }

    return TRUE;
}

//==============================================================================

xbool ini_file::GetString( const char* pKey, char* pBuffer )
{
    ASSERT( pKey );
    ASSERT( pBuffer );

    const char* pValue = FindValue( pKey );

    if( !pValue )
        return( FALSE );

    if( *pValue == '"' )
    {
        // Skip openning '"'.
        x_strcpy( pBuffer, pValue+1 );
        // Nuke trailing '"'.
        s32 i = x_strlen( pBuffer ) - 1;
        if( pBuffer[i] == '"' )
            pBuffer[i]  = '\0';
    }
    else
    {
        x_strcpy( pBuffer, pValue );
    }

    return( TRUE );
}

//==============================================================================

xbool ini_file::GetS32( const char* pKey, s32& Value )
{
    ASSERT( pKey );

    const char* pValue = FindValue( pKey );

    if( !pValue )
        return( FALSE );

    Value = x_atoi( pValue );
    return( TRUE );
}

//==============================================================================

xbool ini_file::GetF32( const char* pKey, f32& Value )
{
    ASSERT( pKey );

    const char* pValue = FindValue( pKey );

    if( !pValue )
        return( FALSE );

    Value = x_atof( pValue );
    return( TRUE );
}

//==============================================================================

xbool ini_file::GetBool( const char* pKey, xbool& Value )
{
    ASSERT( pKey );

    char* True [] = { "TRUE",  "YES", "ON",  "1", NULL };
    char* False[] = { "FALSE", "NO",  "OFF", "0", NULL };

    const char* pValue = FindValue( pKey );

    if( !pValue )
        return( FALSE );

    s32 i;

    // Look for a match with on of the True strings.
    for( i = 0; True[i]; i++ )
    {
        if( x_stricmp( True[i], pValue ) == 0 )
        {
            Value = TRUE;
            return( TRUE );
        }
    }

    // Look for a match with on of the False strings.
    for( i = 0; False[i]; i++ )
    {
        if( x_stricmp( False[i], pValue ) == 0 )
        {
            Value = FALSE;
            return( TRUE );
        }
    }

    // Um.  No match so far.  What to do?  Key is here, but value is suspect.
    Value = FALSE;
    return( FALSE );
}

//==============================================================================

const char* ini_file::FindValue( const char* pKey )
{
    // This is the "quik and dirty" version.  This version will fail if there
    // is data which matches the desired key appearing in the file before the
    // key.  To be proper about it, the key found must meet two conditions:
    //   (1) The key must be preceded by a NULL (or beginning of file).
    //   (2) There must be whitespace and then a '=' following the key.
    // But I'm not going to mess with that tonight.

    s32 i = m_Data.Find( pKey );

    if( i == -1 )
        return( NULL );

    // We have the key!  Search forward for the data.

    s32 Limit = m_Data.GetLength();

    // Skip over the key itself.
    while( (i < Limit) && (x_isalpha(m_Data[i]) || x_isdigit(m_Data[i])) )
        i++;

    // Skip whitespace before the '='.
    while( (i < Limit) && (x_isspace(m_Data[i])) )
        i++;

    // Confirm required '='.
    if( (i >= Limit) || (m_Data[i] != '=') )
        return( NULL );

    // Skip required '='.
    i++;

    // Skip whitespace before the data.
    while( (i < Limit) && (x_isspace(m_Data[i])) )
        i++;

    // Make sure we didn't hit the end of the data.
    if( i >= Limit )
        return( NULL );

    // And that's a wrap.
    return( &m_Data[i] );
}

//==============================================================================

#endif // CONFIG_RETAIL
