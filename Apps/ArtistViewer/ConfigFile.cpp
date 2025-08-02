//==============================================================================
//
//  File:           Config.cpp
//
//  Description:    Config class
//
//  Author:         Stephen Broumley
//
//  Date:           Started July29th, 2002 (C) Inevitable Entertainment Inc.
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================
#include "ConfigFile.hpp"

//==============================================================================

config_file::config_file()
{
    m_bFound = FALSE;
}

//==============================================================================

config_file::~config_file()
{
    // Make sure file is closed
    CloseFile();
}

//==============================================================================

void config_file::Read( void )
{
    token_stream::Read();
    m_bFound = FALSE;
}

//==============================================================================

xbool config_file::Is ( const char* pVar )
{
    // Already found a variable?
    if( m_bFound )
        return FALSE;

    return (x_stricmp( String(), pVar ) == 0 );
}

//==============================================================================

// Integer or boolean
xbool config_file::Read( const char* pVar, s32& I )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        // Could be a number, symbol or string
        Read();

        // Number or string?
        if( Type() == TOKEN_NUMBER )
        {
            I = Int();
        }            
        else if( ( Type() == TOKEN_SYMBOL ) || ( Type() == TOKEN_STRING ) )
        {
            if( x_stricmp( String(), "true" ) == 0 )
                I = 1;
            else if( x_stricmp( String(), "false" ) == 0 )
                I = 0;
            else
                I = 0;
        }
        
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// Float
xbool config_file::Read( const char* pVar, f32& F )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        F = ReadFloat();
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// Color
xbool config_file::Read( const char* pVar, xcolor& C )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        C.R = ReadInt();
        C.G = ReadInt();
        C.B = ReadInt();
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// String
xbool config_file::Read( const char* pVar, char* S )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        x_strcpy( S, ReadString() );
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// Double string
xbool config_file::Read( const char* pVar, char* S0, char* S1 )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        x_strcpy( S0, ReadString() );
        x_strcpy( S1, ReadString() );
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// Tripple string
xbool config_file::Read( const char* pVar, char* S0, char* S1, char* S2 )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        x_strcpy( S0, ReadString() );
        x_strcpy( S1, ReadString() );
        x_strcpy( S2, ReadString() );
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// String, float
xbool config_file::Read( const char* pVar, char* S, f32& F )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        x_strcpy( S, ReadString() );
        F = ReadFloat();
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// Path
xbool config_file::Read( const char* pVar, config_options::path& P )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        x_strcpy( P.m_Name, ReadString() );
        Util_FixPath( P.m_Name );
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================

// Vector3
xbool config_file::Read( const char* pVar, vector3& V )
{
    if( ( m_bFound == FALSE ) && ( x_stricmp( String(), pVar ) == 0 ) )
    {
        V.GetX() = ReadFloat();
        V.GetY() = ReadFloat();
        V.GetZ() = ReadFloat();
        m_bFound = TRUE;
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
