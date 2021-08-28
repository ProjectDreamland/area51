//==============================================================================
//
//  xsc_parse_return_statement
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
//  ParseReturnStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseReturnStatement( void )
{
    // Expect start of compound statement
    const xsc_token* pReturnToken = m_t;
    Expect( T_RETURN, err_syntax, L"Expecting 'return'" );

    // Create node for return
    xsc_ast_node* pReturnNode = m_AST.NewNode( ast_return_statement );

    // Parse expression
    xsc_ast_node* pExpressionNode = ParseExpression( FALSE );
    if( pExpressionNode )
    {
        pReturnNode->Children.Append() = pExpressionNode;
        pReturnNode->Type              = pExpressionNode->Type;

        // Check type of return expression
        if( pExpressionNode->Type != m_pMethodSymbol->Type )
        {
            m_Errors.Error( err_syntax, pReturnToken, "Return type does not match method type" );
        }
    }

    Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );

    // Return return node
    return pReturnNode;
}

//==============================================================================
