//==============================================================================
//
//  xsc_codegen_utility
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
//  CanGetlvalue
//==============================================================================

xsc_codegen::CanGetlvalue( const xsc_ast_node* pNode ) const
{
    xbool   CanGet = TRUE;

    // Check for node type
    switch( pNode->NodeType )
    {
    case ast_expression:
    case ast_var_ref:
    case ast_member_op:
        break;
    default:
        CanGet = FALSE;
    }

    // Loop through children
    for( s32 i=0 ; (i<pNode->Children.GetCount()) && CanGet ; i++ )
    {
        CanGet &= CanGetlvalue( pNode->Children[i] );
    }

    // Return code
    return CanGet;
}

//==============================================================================

s32 xsc_codegen::GetArgumentsByteSize( const xsc_symbol* pSymbol ) const
{
    s32 ByteSize = 0;

    // Does the symbol have a child scope
    if( pSymbol->pChildScope )
    {
        for( s32 i=0 ; i<pSymbol->pChildScope->GetNumSymbols() ; i++ )
        {
            xsc_symbol* pChildSymbol = pSymbol->pChildScope->GetSymbol(i);
            ASSERT( pChildSymbol );
            ByteSize += pChildSymbol->Type.GetByteSize();
        }
    }

    // Return the bytesize
    return ByteSize;
}

//==============================================================================
