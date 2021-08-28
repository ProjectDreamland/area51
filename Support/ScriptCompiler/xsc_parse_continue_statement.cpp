//==============================================================================
//
//  xsc_parse_continue_statement
//
//==============================================================================

#include "xsc_parser.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  ParseContinueStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseContinueStatement( void )
{
    // Expect start of compound statement
    Expect( T_CONTINUE, err_syntax, L"Expecting 'continue'" );

    // Create node for continue
    xsc_ast_node* pContinueNode = m_AST.NewNode( ast_continue_statement );

    // Parse
    Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );

    // Return continue node
    return pContinueNode;
}

//==============================================================================
