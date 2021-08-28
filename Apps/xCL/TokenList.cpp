//==============================================================================
//
//  TokenList.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "TokenList.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

token_list::token_list( void )
{
    m_Count = 0;
    m_Entry = NULL;
}

//==============================================================================

token_list::~token_list()
{
    int i;

    for( i = 0; i < m_Count; i++ )
    {
        free( m_Entry[i] );
    }

    if( m_Entry != NULL )
        free( m_Entry );

    m_Count = 0;
    m_Entry = NULL;
}

//==============================================================================

void token_list::Add( char* pToken )
{
    int Len;

    assert( pToken );
    Len = strlen( pToken );

    // Remove surrounding double quotes.
    if( (pToken[0] == '"') && (pToken[Len-1] == '"') )
    {
        pToken[Len-1] = '\0';
        pToken += 1;
        Len    -= 2;
    }

    // Grow the list of char**'s.
    m_Entry = (char**)realloc( m_Entry, sizeof(char*) * (m_Count+1) );
    assert( m_Entry );

    // Allocate space for the token.
    m_Entry[ m_Count ] = (char*)malloc( Len+1 );
    assert( m_Entry[m_Count] );

    // Copy the token into the new storage.
    strcpy( m_Entry[m_Count], pToken );
    m_Count += 1;
}

//==============================================================================
 
int token_list::GetCount( void )
{
    return( m_Count );
}

//==============================================================================

char* token_list::operator [] ( int Index )
{
    assert( Index < m_Count );
    return( m_Entry[Index] );
}

//==============================================================================

void token_list::Dump( char* pLabel )
{
    int i;

    printf( "Contents of list: %s\n", pLabel );

    for( i = 0; i < m_Count; i++ )
        printf( "    [%03d]  %s\n", i, m_Entry[i] );
}

//==============================================================================
