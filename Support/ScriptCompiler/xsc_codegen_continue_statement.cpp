//==============================================================================
//
//  xsc_codegen_continue_statement
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
//  EmitContinueStatement
//==============================================================================

void xsc_codegen::EmitContinueStatement( xsc_ast_node* pStatementNode )
{
    ASSERT( pStatementNode->NodeType == ast_continue_statement );

    // TODO: Emit continue statement
    ASSERT( 0 );
}

//==============================================================================
