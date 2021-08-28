// StringHelpers.cpp : implementation file
//

#include "stdafx.h"
#include "StringHelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

void PrettyInt( CString& String, int Number )
{
    ASSERT( Number < (U32_MAX/10) );

    String.Empty();

    // Determine Order of number
    int Order    = 0;
    int Divisor  = 1;
    while( Divisor <= Number )
    {
        Order++;
        Divisor *= 10;
    }
    Divisor /= 10;
    Order--;

    // Generate digits
    while( Order >= 0 )
    {
        int Value = Number / Divisor;
        String += (char)('0'+Value);
        if( Order && ((Order % 3) == 0) )
            String += ',';
        Number -= Value * Divisor;
        Divisor /= 10;
        Order--;
    }

    // Zero if nothing in string
    if( String.GetLength() == 0 )
        String = _T("0");
}

/////////////////////////////////////////////////////////////////////////////

void PrettyFloat( CString& String, double Number, int nFractional )
{
    // Format number into string
    String.Format( _T("%.*f"), nFractional, Number );

    // Scan string and insert thousand seperators
    int i = String.Find( '.' );
    int nDigits = 1;
    while( --i > 0 )
    {
        if( (nDigits++ % 3) == 0)
        {
            String.Insert( i, ',' );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void PrettySeconds( CString& String, double Seconds, int nFractional )
{
    // Format number into string
    String.Format( _T("%.*f"), nFractional, Seconds );

    // Scan string and insert thousand seperators
    int i = String.Find( '.' );
    int nDigits = 1;
    while( --i > 0 )
    {
        if( (nDigits++ % 3) == 0)
        {
            String.Insert( i, ',' );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
