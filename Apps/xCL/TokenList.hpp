//==============================================================================
//
//  TokenList.hpp
//
//==============================================================================

#ifndef TOKENLIST_HPP
#define TOKENLIST_HPP

//==============================================================================

class token_list
{
public:
            token_list( void );
           ~token_list();

    void    Add         ( char* pToken );
    int     GetCount    ( void );
    char*   operator [] ( int   Index  );
    void    Dump        ( char* pLabel );

private:
    int     m_Count;
    char**  m_Entry;
};

//==============================================================================
#endif // TOKEN_LIST_HPP
//==============================================================================
