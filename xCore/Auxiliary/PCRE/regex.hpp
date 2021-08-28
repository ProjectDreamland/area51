#include "pcre-4.4-src/src/pcre-4.4/pcre.h"
#include "x_string.hpp"

class regex
{
public:
                regex               ( const char* pPattern );
               ~regex               ( );

    bool        Match               ( const char* pSubject, int Options = 0 );

    int         GetNumSubstrings    ( void );
    const char* GetSubstringPtr     ( int Index );
    int         GetSubstringLen     ( int Index );
    xstring     GetSubstring        ( int Index );

private:
                regex               ( );

    pcre*       m_pRE;
    int         m_nVectors;
    int*        m_pVectors;

    const char* m_pSubject;
    int         m_nSubstrings;
};
