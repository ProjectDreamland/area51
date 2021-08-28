#include "regex.hpp"
#include "x_files.hpp"

regex::regex( const char* pPattern )
{
    const char* pError;
    int         ErrorOffset;

    // Compile the regex
    m_pRE = pcre_compile( pPattern,                 /* the pattern */
                          0,                        /* default options */
                          &pError,                  /* for error message */
                          &ErrorOffset,             /* for error offset */
                          NULL );                   /* use default character tables */

    ASSERT( m_pRE != NULL );

    // Allocate output vectors
    m_nVectors = 99;
    m_pVectors = (int*)malloc( m_nVectors * sizeof(int) );
    ASSERT( m_pVectors );

    // Clear number of substrings
    m_nSubstrings = 0;
}

regex::~regex()
{
    // Free pcre memory
    pcre_free( m_pRE );

    // Delete output vectors
    free( m_pVectors );
    m_nVectors = 0;
    m_pVectors = NULL;
}

bool regex::Match( const char* pSubject, int Options )
{
    // Save off subject pointer
    m_pSubject = pSubject;

    // Get the length of the subject
    int SubjectLen = (int)strlen( pSubject );

    // Execute the regex
    int rc = pcre_exec( m_pRE,                      /* the compiled pattern */
                        NULL,                       /* no extra data - we didn't study the pattern */
                        pSubject,                   /* the subject string */
                        SubjectLen,                 /* the length of the subject */
                        0,                          /* start at offset 0 in the subject */
                        Options,                    /* default options */
                        m_pVectors,                 /* output vector for substring information */
                        m_nVectors);                /* number of elements in the output vector */

    bool Matched = false;

    // Error or no match
    if( rc < 0 )
    {
        return false;
    }

    // Not enough output vectors, return what we can
    if( rc == 0 )
    {
        rc = m_nVectors/3;
    }

    // Success, found a match
    m_nSubstrings = rc;
    return true;
}

int regex::GetNumSubstrings( void )
{
    return m_nSubstrings;
}

const char* regex::GetSubstringPtr( int Index )
{
    ASSERT( (Index >= 0) && (Index < m_nSubstrings) );
    return m_pSubject + m_pVectors[2*Index];
}

int regex::GetSubstringLen( int Index )
{
    ASSERT( (Index >= 0) && (Index < m_nSubstrings) );
    return m_pVectors[2*Index+1] - m_pVectors[2*Index];
}

xstring regex::GetSubstring( int Index )
{
    ASSERT( (Index >= 0) && (Index < m_nSubstrings) );

    s32 Len = m_pVectors[2*Index+1] - m_pVectors[2*Index];
    xstring String( m_pSubject + m_pVectors[2*Index], Len );

    return String;
}
