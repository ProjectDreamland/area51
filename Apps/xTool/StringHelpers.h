/////////////////////////////////////////////////////////////////////////////
//
//  CResString - Can be inline constructed to access resourced strings
//               CResString( IDS_STRINGID )
//
//  CFmtString - Can be inline constructed with a format applied to it
//               CFmtString( "%d", i )
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Functions

void PrettyInt      ( CString& String, int Number );
void PrettyFloat    ( CString& String, double Number, int nFractional );
void PrettySeconds  ( CString& String, double Seconds, int nFractional );

/////////////////////////////////////////////////////////////////////////////
// CResString

class CResString : public CString
{
public:
    CResString() : CString()
    {
    }

    CResString( UINT nResID )
    {
        //Init();
        VERIFY( LoadString(nResID) != 0 );
        //_ASSERTE( m_pchData );
    }

    void operator () ( UINT nResID )
    {
        //Init();
        VERIFY( LoadString(nResID) != 0 );
        //_ASSERTE( m_pchData );
    }
};

/////////////////////////////////////////////////////////////////////////////
// CFmtString

class CFmtString : public CString
{
public:
    CFmtString() : CString()
    {
    }

    CFmtString( char *szFormat, ... )
    {
        //Init();
        va_list argList;
        va_start( argList, szFormat );
        FormatV( szFormat, argList );
    }

    CFmtString( UINT uResID, ... )
    {
        //Init();
        va_list argList;
        CResString sFormat( uResID );
        va_start( argList, sFormat );
        FormatV( sFormat, argList );
    }

    void operator () ( char *szFormat, ... )
    {
        //Init();
        va_list argList;
        va_start( argList, szFormat );
        FormatV( szFormat, argList );
    }

    void operator () ( UINT uResID, ... )
    {
        //Init();
        va_list argList;
        CResString sFormat( uResID );
        va_start( argList, sFormat );
        FormatV( sFormat, argList );
    }
};

/////////////////////////////////////////////////////////////////////////////
