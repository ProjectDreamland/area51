//==============================================================================
//
//  xsc_parse_typedef
//
//==============================================================================

#include "xsc_parser.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "xsc_compiler.hpp"
#include "xsc_vm_module.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  ParseTypedef
//==============================================================================

void xsc_parser::ParseTypedef( void )
{
    const xsc_token* pTypedefToken = m_t;

    ReadToken();

    // Check for type
    if( m_t->Code == T_IDENTIFIER )
    {
        const xsc_token*  pTypeToken = m_t;
        xsc_symbol* pTypeSymbol = GetTypeFromToken( pTypeToken );
        if( pTypeSymbol == NULL )
        {
            m_Errors.Error( err_syntax, pTypeToken, xfs("Undefined type '%ls'", pTypeToken->ValStr) );
            pTypeSymbol = g_pTs32;
        }
        ReadToken();
        if( m_t->Code == T_IDENTIFIER )
        {
            const xsc_token* pTypedefToken = m_t;
            xsc_symbol* pSymbol = m_SymbolTable.AddSymbol( pTypedefToken->ValStr, pTypedefToken, symtype_typedef );
            pSymbol->Type = typeref( pTypeSymbol );
            pSymbol->StorageSize = pTypeSymbol->StorageSize;
            ReadToken();
        }
        else
        {
            m_Errors.Error( err_syntax, pTypedefToken, xfs("Expecting typedef identifier") );
        }
    }
    else
    {
        m_Errors.Error( err_syntax, pTypedefToken, xfs("Expecting type identifier '%ls'", m_t->ValStr ) );
    }

    // Expect a terminating semicolon
    Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );
}

//==============================================================================
