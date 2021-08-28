//==============================================================================
//
//  xsc_parse_while_statement
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
//  ParseWhileStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseWhileStatement( void )
{
    // Expect start of compound statement
    Expect( T_WHILE, err_syntax, L"Expecting 'while'" );

    // Create node for while
    xsc_ast_node* pWhileNode = m_AST.NewNode( ast_while_statement );

    // Parse
    Expect( T_LPAREN, err_syntax, L"Expecting '('" );
    xsc_ast_node* pExpressionNode = ParseExpression( FALSE );
    if( pExpressionNode )
    {
        Expect( T_RPAREN, err_syntax, L"Expecting ')'" );

        // Valid Expression
        pWhileNode->Children.Append() = pExpressionNode;

        // Parse the statement
        xsc_ast_node* pStatementNode = ParseStatement();
        if( pStatementNode )
        {
            // Valid statement
            pWhileNode->Children.Append() = pStatementNode;
        }
        else
        {
            // TODO: Error condition, how to recover?
        }
    }
    else
    {
        // TODO: Error condition, how to recover?
    }

    // Return while node
    return pWhileNode;
}

//==============================================================================
