//==============================================================================
//
//  xsc_parse_for_statement
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
//  ParseForStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseForStatement( void )
{
    // Expect start of compound statement
    Expect( T_FOR, err_syntax, L"Expecting 'for'" );

    // Create node for for
    xsc_ast_node* pForNode = m_AST.NewNode( ast_for_statement );

    Expect( T_LPAREN, err_syntax, L"Expecting '('" );

    // Parse Expression1
    xsc_ast_node* pExpressionNode = ParseExpression( TRUE );

    Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );

    // Valid Expression
    pForNode->Children.Append() = pExpressionNode;

    // Parse Expression2
    pExpressionNode = ParseExpression( FALSE );

    Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );

    // Valid Expression
    pForNode->Children.Append() = pExpressionNode;

    // Parse Expression3
    pExpressionNode = ParseExpression( TRUE );

    Expect( T_RPAREN, err_syntax, L"Expecting ')'" );

    // Valid Expression
    pForNode->Children.Append() = pExpressionNode;

    // Parse the statement
    xsc_ast_node* pStatementNode = ParseStatement();
    if( pStatementNode )
    {
        // Valid statement
        pForNode->Children.Append() = pStatementNode;
    }
    else
    {
        // TODO: Error condition, how to recover?
    }

    // Return for node
    return pForNode;
}

//==============================================================================
