//==============================================================================
//
//  xsc_codegen_if_statement
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
//  EmitIfStatement
//==============================================================================

void xsc_codegen::EmitIfStatement( xsc_ast_node* pStatementNode )
{
    ASSERT( pStatementNode->NodeType == ast_if_statement );
    ASSERT( pStatementNode->Children.GetCount() >= 2 );

    // Emit test
    EmitExpression( pStatementNode->Children[0] );

    // Save code location to backpatch the branch
    s32 BranchAddress = m_Methods.GetLength();
    EmitOpcode ( vm_bf );
    EmitOperand( 0 );

    // Emit True case
    EmitStatement( pStatementNode->Children[1] );

    // If an else clause then patch previous and set new backpatch
    if( pStatementNode->Children.GetCount() == 3 )
    {
        // Patch bf from condition
        EmitOperandAt( (m_Methods.GetLength()+3) - (BranchAddress+3), BranchAddress+1 );

        // Emit new branch to pass else clause
        BranchAddress = m_Methods.GetLength();
        EmitOpcode ( vm_ba );
        EmitOperand( 0 );

        // Emit False case
        EmitStatement( pStatementNode->Children[2] );
    }

    // Patch last branch
    EmitOperandAt( m_Methods.GetLength() - (BranchAddress+3), BranchAddress+1 );
}

//==============================================================================
