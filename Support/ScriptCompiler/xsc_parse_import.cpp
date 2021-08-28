//==============================================================================
//
//  xsc_parse_import
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
//  ParseImport
//==============================================================================

void xsc_parser::ParseImport( void )
{
    const xsc_token*    pImportToken = m_t;

    ReadToken();

    // Check for module name identifier
    if( m_t->Code == T_IDENTIFIER )
    {
        // Import the appropriate symbol table
        xstring ModuleName( m_t->ValStr );
        xstring SymbolName = ModuleName + ".sym";
        xsc_symbol_table    ImportTable;
        xbool Success = ImportTable.Load( SymbolName );

        if( Success )
        {
            m_SymbolTable.Import( ImportTable );
        }
        else
        {
            m_Errors.Error( err_syntax, m_t, xfs("Failed to import symbols for module '%s'", SymbolName) );
        }

        ReadToken();
    }
    else
    {
        // Expecting module name
        m_Errors.Error( err_syntax, m_t, "Expecting a module name" );
        ReadToken();
    }
}

//==============================================================================
