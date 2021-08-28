//==============================================================================
//
//  xsc_codegen_compound_statement
//
//==============================================================================

#include "xsc_codegen.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "../ScriptVM/xsc_vm_fileformat.hpp"
#include "../ScriptVM/xsc_vm_instructions.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  EmitCompoundStatement
//==============================================================================

void xsc_codegen::EmitCompoundStatement( xsc_ast_node* pStatementNode )
{
    s32     i;

    ASSERT( pStatementNode->NodeType == ast_compound_statement );

    // TODO: Emit constructors for all variables in scope

    // Emit the statements
    for( i=0 ; i<pStatementNode->Children.GetCount() ; i++ )
    {
        EmitStatement( pStatementNode->Children[i] );
    }

    // TODO: Emit destructors for all variables in scope
}

//==============================================================================
//  EmitStatement
//==============================================================================

void xsc_codegen::EmitStatement( xsc_ast_node* pNode )
{
    switch( pNode->NodeType )
    {
    case ast_compound_statement:
        EmitCompoundStatement( pNode );
        break;
    case ast_if_statement:
        EmitIfStatement( pNode );
        break;
    case ast_while_statement:
        EmitWhileStatement( pNode );
        break;
    case ast_for_statement:
        EmitForStatement( pNode );
        break;
    case ast_continue_statement:
        EmitContinueStatement( pNode );
        break;
    case ast_return_statement:
        EmitReturnStatement( pNode );
        break;
    case ast_expression:
        EmitExpression( pNode );
        break;
    case ast_var_defs:
        {
            for( s32 i=0 ; i<pNode->Children.GetCount() ; i++ )
            {
                xsc_ast_node* pChild = pNode->Children[i];
                ASSERT( pChild->NodeType == ast_var_def );
                EmitStatement( pChild );
            }
        }
        break;
    case ast_var_def:
        // TODO: Emit variable declaration code (should it construct here?)
        {
            if( pNode->Children.GetCount() == 1 )
                EmitStatement( pNode->Children[0] );
        }
        break;
    case ast_empty_statement:
        // TODO: Do anything here?
        break;

    default:
        // Error, malformed AST
        ASSERT( 0 );
    }
}

//==============================================================================
