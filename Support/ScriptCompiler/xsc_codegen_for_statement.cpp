//==============================================================================
//
//  xsc_codegen_for_statement
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
//  EmitForStatement
//==============================================================================

void xsc_codegen::EmitForStatement( xsc_ast_node* pStatementNode )
{
    ASSERT( pStatementNode->NodeType == ast_for_statement );
    ASSERT( pStatementNode->Children.GetCount() == 4 );

    // Emit Child0 for setup
    EmitExpression( pStatementNode->Children[0] );

    // Save code location for top of loop
    s32 LoopAddress = m_Methods.GetLength();

    // Emit Child1 for test
    EmitExpression( pStatementNode->Children[1] );
    
    // Save code location to backpatch the branch
    s32 BranchAddress = m_Methods.GetLength();
    EmitOpcode ( vm_bf );
    EmitOperand( 0 );

    // Emit Statement
    EmitStatement( pStatementNode->Children[3] );

    // Emit Child2 for incrementer
    EmitExpression( pStatementNode->Children[2] );

    // Emit Branch back to top of loop
    EmitOpcode ( vm_ba );
    EmitOperand( LoopAddress - (m_Methods.GetLength()+2) );

    // Backpatch branch
    EmitOperandAt( m_Methods.GetLength() - (BranchAddress+3), BranchAddress+1 );
}

//==============================================================================
