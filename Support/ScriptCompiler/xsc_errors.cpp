//==============================================================================
//
//  xsc_errors
//
//==============================================================================

#include "xsc_errors.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Data
//==============================================================================

struct error_string
{
    s32             ErrorCode;
    const xwchar*   pString;
};

error_string ErrorStrings[] =
{
    { err_syntax,                       L"Syntax error"                          },
    { warn_syntax,                      L"Syntax warning"                        },
};

//==============================================================================
//  GetErrorString
//==============================================================================

static const xwchar* GetErrorString( s32 ErrorCode )
{
    s32 i;

    for( i=0 ; i<(sizeof(ErrorStrings)/sizeof(error_string)) ; i++ )
    {
        if( ErrorStrings[i].ErrorCode == ErrorCode )
        {
            return ErrorStrings[i].pString;
        }
    }

    return L"";
}

//==============================================================================
//  xsc_errors
//==============================================================================

xsc_errors::xsc_errors( const xwstring& Source ) : m_Source( Source )
{
    m_nWarnings = 0;
    m_nErrors   = 0;
}

//==============================================================================
//  ~xsc_errors
//==============================================================================

xsc_errors::~xsc_errors( void )
{
}

//==============================================================================
//  SetSourceName
//==============================================================================

void xsc_errors::SetSourceName( const xstring& SourceName )
{
    m_SourceName = SourceName;
}

//==============================================================================
//  Error
//==============================================================================

void xsc_errors::Error( s32 Code, s32 Index )
{
    ASSERT( (Code > err_start) && (Code < err_end) );

    err& Err = m_Errors.Append();
    Err.Code    = Code;
    Err.pToken  = 0;
    Err.Index   = Index;
    m_nErrors++;
}

void xsc_errors::Error( s32 Code, s32 Index, const char* pString )
{
    ASSERT( (Code > err_start) && (Code < err_end) );

    err& Err = m_Errors.Append();
    Err.Code    = Code;
    Err.pToken  = 0;
    Err.Index   = Index;
    if( pString )
        Err.String = (const xwchar*)pString;
    m_nErrors++;
}

void xsc_errors::Error( s32 Code, const xsc_token* pToken, const xwchar* pString )
{
    ASSERT( (Code > err_start) && (Code < err_end) );

    err& Err = m_Errors.Append();
    Err.Code    = Code;
    Err.pToken  = pToken;
    if( pString )
        Err.String = pString;
    m_nErrors++;
}

void xsc_errors::Error( s32 Code, const xsc_token* pToken, const char* pString )
{
    xwstring String;
    String = xstring(pString);
    Error( Code, pToken, (const xwchar*)String );
}

//==============================================================================
//  Warning
//==============================================================================

void xsc_errors::Warning( s32 Code, s32 Index )
{
    ASSERT( (Code > warn_start) && (Code < warn_end) );

    err& Err = m_Errors.Append();
    Err.Code    = Code;
    Err.pToken  = 0;
    Err.Index   = Index;
    m_nWarnings++;
}

void xsc_errors::Warning( s32 Code, const xsc_token* pToken, const xwchar* pString )
{
    ASSERT( (Code > warn_start) && (Code < warn_end) );

    err& Err = m_Errors.Append();
    Err.Code    = Code;
    Err.pToken  = pToken;
    if( pString )
        Err.String = pString;
    m_nWarnings++;
}

void xsc_errors::Warning( s32 Code, const xsc_token* pToken, const char* pString )
{
    xwstring String;
    String = xstring(pString);
    Warning( Code, pToken, (const xwchar*)String );
}

//==============================================================================
//  GetNumErrors
//==============================================================================

s32 xsc_errors::GetNumErrors( void ) const
{
    return m_nErrors;
}

//==============================================================================
//  GetNumWarnings
//==============================================================================

s32 xsc_errors::GetNumWarnings( void ) const
{
    return m_nErrors;
}

//==============================================================================
//  Dump
//==============================================================================

xstring xsc_errors::Dump( void ) const
{
    s32     i;
    xstring Output;

    for( i=0 ; i<m_Errors.GetCount() ; i++ )
    {
        err&            Err     = m_Errors[i];
        xbool           IsError = Err.Code < warn_start;
        const xwchar*   ErrWarn = IsError ? L"error" : L"warning";
        s32             Line;

        if( Err.pToken == NULL )
        {
            // Get Line Number
            Line = IndexToLine( Err.Index );

            // Error with Index specified
            Output.AddFormat( "%s(%d) : %ls C%04d: %ls\n", (const char*)m_SourceName, Line, ErrWarn, Err.Code, GetErrorString( Err.Code ) );
        }
        else
        {
            // Get Line Number
            Line = IndexToLine( Err.pToken->StartIndex );

            // Error with Token specified
            if( Err.String.IsEmpty() )
            {
                // Error with no message specified
                Output.AddFormat( "%s(%d) : %ls C%04d: %ls\n", (const char*)m_SourceName, Line, ErrWarn, Err.Code, GetErrorString( Err.Code ) );
            }
            else
            {
                // Error with message specified
                Output.AddFormat( "%s(%d) : %ls C%04d: %ls\n", (const char*)m_SourceName, Line, ErrWarn, Err.Code, (const xwchar*)Err.String );
            }
        }
    }

    // Add summary
    Output.AddFormat( "\n" );
    Output.AddFormat( "%d error(s), %d warning(s)\n", m_nErrors, m_nWarnings );

    // Return dump
    return Output;
}

//==============================================================================
//  IndexToLine
//==============================================================================

s32 xsc_errors::IndexToLine( s32 Index ) const
{
    s32 Line = 1;
    s32 i;

    // Count linefeeds
    for( i=1 ; i<Index ; i++ )
    {
        if( m_Source[i-1] == 0x0a ) Line++;
    }

    // Return Line Number
    return Line;
}

//==============================================================================
