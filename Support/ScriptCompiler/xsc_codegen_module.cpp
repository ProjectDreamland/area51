//==============================================================================
//
//  xsc_codegen_module
//
//==============================================================================

#include "xsc_codegen.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "../ScriptVM/xsc_vm_fileformat.hpp"
#include "../ScriptVM/xsc_vm_instructions.hpp"
#include "xsc_compiler.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  EmitModule
//==============================================================================

void xsc_codegen::EmitModule( xsc_ast_node* pModuleNode )
{
    s32     i;

    ASSERT( pModuleNode->NodeType == ast_module );

    // Set Header fields
    m_Header.Magic              = XSC_VM_MAGIC;
    m_Header.Version            = XSC_VM_VERSION;
    m_Header.ModuleNameOffset   = EmitConstStr( xwstring("Module") );
    m_Header.pModule            = NULL;

    // Emit the classes in the module
    for( i=0 ; i<pModuleNode->Children.GetCount() ; i++ )
    {
        xsc_ast_node* pNode = pModuleNode->Children[i];
        if( pNode->NodeType == ast_class )
        {
            // Emit Class Node
            EmitClass( pNode );
        }
        else
        {
            // Error, malformed AST
            ASSERT( 0 );
        }
    }
}

//==============================================================================
//  EmitClass
//==============================================================================

void xsc_codegen::EmitClass( xsc_ast_node* pClassNode )
{
    s32     i;

    ASSERT( pClassNode->NodeType == ast_class );

    // Emit Class definition
    s32 iClassDef = EmitClassDef( pClassNode->pSymbol->Name, pClassNode->pSymbol->pChildScope->GetStorageSize() );

    // Emit methods and field in the class
    for( i=0 ; i<pClassNode->Children.GetCount() ; i++ )
    {
        xsc_ast_node* pNode = pClassNode->Children[i];
        if( pNode->NodeType == ast_class_methoddef )
        {
            // Emit Method Node
            EmitMethod( pNode );
        }
        else if( pNode->NodeType == ast_class_fielddef )
        {
            // Emit Field Node
            EmitField( pNode );
        }
        else
        {
            // Error, malformed AST
            ASSERT( 0 );
        }
    }
}

//==============================================================================
//  EmitMethod
//==============================================================================

void xsc_codegen::EmitMethod( xsc_ast_node* pMethodNode )
{
    ASSERT( pMethodNode->NodeType == ast_class_methoddef );

    // Emit Method definition if defined
    // TODO: Get number of stack bytes and local bytes for method def
    xsc_symbol* pSymbol = pMethodNode->pSymbol;
    u32 Flags = 0;
    Flags |= pSymbol->IsConst()   ? XSC_VM_METHOD_CONST   : 0;
    Flags |= pSymbol->IsStatic()  ? XSC_VM_METHOD_STATIC  : 0;
    Flags |= pSymbol->IsVirtual() ? XSC_VM_METHOD_VIRTUAL : 0;
    Flags |= pSymbol->IsNative()  ? XSC_VM_METHOD_NATIVE  : 0;

    s32 iMethodDef = EmitMethodDef( pSymbol->pParentScope->GetOwningSymbol()->Name,
                                    pMethodNode->pSymbol->Name,
                                    pMethodNode->pSymbol->Signature,
                                    GetArgumentsByteSize( pMethodNode->pSymbol ),
                                    0,
                                    0,
                                    pMethodNode->pSymbol->StorageSize,
                                    Flags );

    // Emit code for the method
    if( pSymbol->IsDefined() )
    {
        EmitCompoundStatement( pMethodNode->Children[0] );

        // TODO: Backpatch all return's emitted for this method to point to
        //       the current code location

        // Emit exit code for the method
        if( pSymbol->Type.pType == g_pTvoid )
        {
            EmitOpcode( vm_vret );
        }
        else if( (pSymbol->Type.pType == g_pTxbool) ||
            (pSymbol->Type.pType == g_pTs32) )
        {
            EmitOpcode( vm_iret );
        }
        else if( pSymbol->Type.pType == g_pTf32 )
        {
            EmitOpcode( vm_fret );
        }
        else
        {
            // Return for class types
            EmitOpcode ( vm_cret );
            EmitOperand( EmitClassRef( pSymbol->Type.pType->Name ) );
        }

        // Set Code Length in MethodDef
        m_MethodDef[iMethodDef].MethodLength = m_Methods.GetLength() - m_MethodDef[iMethodDef].MethodOffset;
    }
    else
    {
        // Error if not a native method
        if( !pSymbol->IsNative() )
        {
            m_Errors.Error( err_syntax, pSymbol->pDefiningToken, "Method should have a body defined" );
        }

        // No code output
        m_MethodDef[iMethodDef].MethodOffset = 0;
        m_MethodDef[iMethodDef].MethodLength = 0;
    }
}

//==============================================================================
//  EmitField
//==============================================================================

void xsc_codegen::EmitField( xsc_ast_node* pFieldNode )
{
    ASSERT( pFieldNode->NodeType == ast_class_fielddef );

    // Emit Field definition
    s32 iFieldDef = EmitFieldDef( pFieldNode->pSymbol->pParentScope->GetOwningSymbol()->Name,
                                  pFieldNode->pSymbol->Name,
                                  pFieldNode->pSymbol->Signature,
                                  pFieldNode->pSymbol->StorageOffset );
}

//==============================================================================
