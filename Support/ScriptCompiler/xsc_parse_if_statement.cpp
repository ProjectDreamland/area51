//==============================================================================
//
//  xsc_parse_if_statement
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
//  ParseIfStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseIfStatement( void )
{
    // Expect start of compound statement
    Expect( T_IF, err_syntax, L"Expecting 'if'" );

    // Create node for if
    xsc_ast_node* pIfNode = m_AST.NewNode( ast_if_statement );

    // Parse
    Expect( T_LPAREN, err_syntax, L"Expecting '('" );
    xsc_ast_node* pExpressionNode = ParseExpression( FALSE );
    if( pExpressionNode )
    {
        Expect( T_RPAREN, err_syntax, L"Expecting ')'" );

        // Valid Expression
        pIfNode->Children.Append() = pExpressionNode;

        // Parse the statement
        xsc_ast_node* pStatementNode = ParseStatement();
        if( pStatementNode )
        {
            // Valid statement
            pIfNode->Children.Append() = pStatementNode;

            // Check for else clause
            if( m_t->Code == T_ELSE )
            {
                ReadToken();

                // Parse the else statement
                xsc_ast_node* pStatementNode = ParseStatement();
                if( pStatementNode )
                {
                    // Valid else statement
                    pIfNode->Children.Append() = pStatementNode;
                }
                else
                {
                    // TODO: Error condition, how to recover?
                }
            }
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

    // Return if node
    return pIfNode;
}

//==============================================================================
