//==============================================================================
//
//  xsc_codegen_return_statement
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
//  EmitReturnStatement
//==============================================================================

void xsc_codegen::EmitReturnStatement( xsc_ast_node* pStatementNode )
{
    ASSERT( pStatementNode->NodeType == ast_return_statement );

    // TODO: Emit code to jump to method prolog
    //CJG
    if( pStatementNode->Children.GetCount() > 0 )
    {
        EmitExpression( pStatementNode->Children[0] );
    }
}

//==============================================================================
