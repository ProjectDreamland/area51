//==============================================================================
//
//  xsc_codegen_expression
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
//  EmitExpression
//==============================================================================

void xsc_codegen::EmitExpression( xsc_ast_node*     pExpressionNode,
                                  const exp_type    eType )
{
    ASSERT( pExpressionNode->NodeType == ast_expression );

    // Verify we can provide an lvalue if requested
    if( LVALUE & eType )
    {
        if( !CanGetlvalue( pExpressionNode ) )
        {
            m_Errors.Error( err_syntax, pExpressionNode->pToken, "Not an lvalue expression" );
            return;
        }
    }

    // Emit the expression
    if( pExpressionNode->Children.GetCount() > 0 )
        EmitExpressionOp( pExpressionNode->Children[0], eType );
}

//==============================================================================
//  EmitExpressionOp
//==============================================================================

void xsc_codegen::EmitExpressionOp( xsc_ast_node*   pNode,
                                    const exp_type  eType)
{
    switch( pNode->NodeType )
    {
    case ast_method:
        EmitExpressionMethod( pNode, eType );
        break;
    case ast_const_xbool:
        EmitOpcode ( vm_iconst );
        EmitOperand( EmitConstInt( (pNode->pToken->Code == T_FALSE) ? 0 : 1 ) );
        break;
    case ast_const_int:
        EmitOpcode ( vm_iconst );
        EmitOperand( EmitConstInt( pNode->pToken->ValInt ) );
        break;
    case ast_const_flt:
        EmitOpcode ( vm_fconst );
        EmitOperand( EmitConstFlt( pNode->pToken->ValFlt ) );
        break;
    case ast_const_str:
        // TODO: Implement strings
        ASSERT( 0 );
        break;
    case ast_var_ref:
        EmitExpressionVarRefOp( pNode, eType );
        break;
    case ast_cast:
        EmitExpressionCastOp( pNode, eType );
        break;
    case ast_assign:
    case ast_mul_assign:
    case ast_div_assign:
    case ast_mod_assign:
    case ast_add_assign:
    case ast_sub_assign:
    case ast_shl_assign:
    case ast_shr_assign:
    case ast_and_assign:
    case ast_xor_assign:
    case ast_or_assign:
        EmitExpressionAssignOp( pNode, eType );
        break;
    case ast_log_or_op:
    case ast_log_and_op:
        EmitExpressionLogicalOp( pNode, eType );
        break;
    case ast_or_op:
    case ast_xor_op:
    case ast_and_op:
        EmitExpressionBitwiseOp( pNode, eType );
        break;
    case ast_eq_op:
    case ast_neq_op:
    case ast_lt_op:
    case ast_gt_op:
    case ast_le_op:
    case ast_ge_op:
        EmitExpressionRelationOp( pNode, eType );
        break;
    case ast_shl_op:
    case ast_shr_op:
        EmitExpressionShiftOp( pNode, eType );
        break;
    case ast_add_op:
    case ast_sub_op:
    case ast_mul_op:
    case ast_div_op:
    case ast_mod_op:
        EmitExpressionMulAddOp( pNode, eType );
        break;
    case ast_unary_sub_op:
    case ast_unary_not_op:
    case ast_unary_complement_op:
    case ast_pre_inc_op:
    case ast_pre_dec_op:
        EmitExpressionUnaryOp( pNode, eType );
        break;
    case ast_member_op:
        EmitExpressionMemberOp( pNode, eType );
        break;
    }
}

//==============================================================================
//  EmitExpressionCastOp
//==============================================================================

void xsc_codegen::EmitExpressionMethod( xsc_ast_node*   pNode,
                                        const exp_type  eType )
{
    ASSERT( pNode->Type.pType );
    xbool   HasThis = FALSE;

    // Load 'this' pointer if needed
    if( pNode->pSymbol->IsMember() && (!pNode->pSymbol->IsStatic()) )
    {
        HasThis = TRUE;
        if( !m_NoEmitThis )
            EmitOpcode( vm_this );
    }

    // Determine number of arguments for this function
    s32 nArgs = pNode->pSymbol->pChildScope->GetNumSymbols() - (HasThis ? 1 : 0);

    // Emit the children which are the arguments
    s32 ArgIndex = HasThis ? 1 : 0;
    for( s32 i=0 ; i<nArgs ; i++ )
    {
        // Get symbol for argument
        xsc_symbol*     pArgSymbol  = pNode->pSymbol->pChildScope->GetSymbol( ArgIndex+i );
        xbool           IsReference = pArgSymbol->Type.IsReference;

        // Emit expression for argument
        xsc_ast_node*   pChild = pNode->Children[i];
        ASSERT( pChild );
        EmitExpression( pChild, IsReference ? (exp_type)(LVALUE|AVALUE) : (exp_type)(RVALUE|AVALUE) );
    }

    // Emit function call
    if( pNode->pSymbol->IsNative() )
    {
        EmitOpcode ( vm_invokenative );
        EmitOperand( EmitMethodRef( pNode->pSymbol->pParentScope->GetOwningSymbol()->Name, pNode->pSymbol->Name, pNode->pSymbol->Signature ) );
    }
    else if( pNode->pSymbol->IsStatic() )
    {
        EmitOpcode ( vm_invokestatic );
        EmitOperand( EmitMethodRef( pNode->pSymbol->pParentScope->GetOwningSymbol()->Name, pNode->pSymbol->Name, pNode->pSymbol->Signature ) );
    }
    else
    {
        EmitOpcode ( vm_invoke );
        EmitOperand( EmitMethodRef( pNode->pSymbol->pParentScope->GetOwningSymbol()->Name, pNode->pSymbol->Name, pNode->pSymbol->Signature ) );
    }
}

//==============================================================================
//  EmitExpressionVarRefOp
//==============================================================================

void xsc_codegen::EmitExpressionVarRefOp( xsc_ast_node*     pNode,
                                          const exp_type    eType )
{
    ASSERT( pNode->Children.GetCount() == 0 );
    ASSERT( pNode->pSymbol );
    ASSERT( pNode->Type.pType );

    const xsc_symbol* pSymbol = pNode->pSymbol;
    const xsc_symbol* pType   = pNode->Type.pType;

    // Emit appropriate load operator
    switch( pSymbol->SymbolType )
    {
    case symtype_argument:
        {
            // Load l-value of argument
            EmitOpcode ( vm_aaddr );
            EmitOperand( pSymbol->StorageOffset );

            // If it's a reference then load the real l-value
            if( pSymbol->Type.IsReference )
            {
                EmitOpcode( vm_iload );
            }

            // Load r-value
            if( RVALUE & eType )
            {
                if( (pType == g_pTxbool) || (pType == g_pTs32) )
                    EmitOpcode ( vm_iload );
                else if( (pType == g_pTf32) )
                    EmitOpcode ( vm_fload );
                else
                {
                    // Load a class onto the stack
                    if( AVALUE & eType )
                    {
                        EmitOpcode ( vm_cload );
                        EmitOperand( EmitClassRef( pType->Name ) );
                    }
                }
            }
        }
        break;
    case symtype_field:
        {
            // Load this pointer if needed
            if( m_NoEmitThis == 0 )
                EmitOpcode( vm_this );

            // Load l-value of field
            EmitOpcode ( vm_faddr );
            EmitOperand( EmitFieldRef( pNode->pSymbol->pParentScope->GetOwningSymbol()->Name, pNode->pSymbol->Name ) );

            // Load r-value
            if( RVALUE & eType )
            {
                // TODO: Deal with static fields
                if( (pType == g_pTxbool) || (pType == g_pTs32) )
                    EmitOpcode ( vm_iload );
                else if( (pType == g_pTf32) )
                    EmitOpcode ( vm_fload );
                else
                {
                    // Load a class onto the stack
                    if( AVALUE & eType )
                    {
                        EmitOpcode ( vm_cload );
                        EmitOperand( EmitClassRef( pType->Name ) );
                    }
                }
            }
        }
        break;
    case symtype_local:
        {
            // Load l-value of local
            EmitOpcode ( vm_laddr );
            EmitOperand( pSymbol->StorageOffset );

            // Load r-value
            if( RVALUE & eType )
            {
                if( (pType == g_pTxbool) || (pType == g_pTs32) )
                    EmitOpcode ( vm_iload );
                else if( (pType == g_pTf32) )
                    EmitOpcode ( vm_fload );
                else
                {
                    // Load a class onto the stack
                    if( AVALUE & eType )
                    {
                        EmitOpcode ( vm_cload );
                        EmitOperand( EmitClassRef( pType->Name ) );
                    }
                }
            }
        }
        break;
    default:
        // TODO: Finish types of variables that can be loaded
        ASSERT( 0 );
    }
}

//==============================================================================
//  EmitExpressionCastOp
//==============================================================================

void xsc_codegen::EmitExpressionCastOp( xsc_ast_node*   pNode,
                                        const exp_type  eType )
{
    ASSERT( pNode->Type.pType );

    if( pNode->Children.GetCount() > 0 )
    {
        xsc_ast_node* pChild = pNode->Children[0];
        ASSERT( pChild );
        ASSERT( pChild->Type.pType );

        // Emit child first
        EmitExpressionOp( pChild, eType );

        // Emit Cast
        if( pNode->Type.pType == g_pTvoid )
        {
            // Just pop off whatever was on the stack
            if( pChild->Type.pType->StorageSize > 0 )
            {
                EmitOpcode ( vm_pop );
                EmitOperand( pChild->Type.pType->StorageSize );
            }
        }
        else if( pNode->Type.pType == g_pTs32 )
        {
            if( pChild->Type.pType == g_pTxbool )
            {
                // Nothing to do
            }
            else if( pChild->Type.pType == g_pTs32 )
            {
                // Nothing to do
            }
            else if( pChild->Type.pType == g_pTf32 )
            {
                EmitOpcode( vm_ftoi );
            }
        }
        else if( pNode->Type.pType == g_pTf32 )
        {
            if( pChild->Type.pType == g_pTxbool )
            {
                EmitOpcode( vm_itof );
            }
            else if( pChild->Type.pType == g_pTs32 )
            {
                EmitOpcode( vm_itof );
            }
            else if( pChild->Type.pType == g_pTf32 )
            {
                // Nothing to do
            }
        }
        else
        {
            // TODO: Check for an overloaded cast operator on the types in question
            // TODO: Better error
            m_Errors.Error( err_syntax, pNode->pToken, xfs("Unsupported cast '%ls' to '%ls'", pChild->Type.pType->Name, pNode->Type.pType->Name) );
        }
    }
}

//==============================================================================
//  EmitExpressionAssignmentOp
//==============================================================================

void xsc_codegen::EmitExpressionAssignOp( xsc_ast_node*     pNode,
                                          const exp_type    eType )
{
    ASSERT( pNode->Children.GetCount() == 2 );

    xsc_ast_node*       pLeft   = pNode->Children[0];
    xsc_ast_node*       pRight  = pNode->Children[1];
    const xsc_symbol*   pType   = pNode->Type.pType;

    // Check compatible types
    if( pLeft->Type.pType != pRight->Type.pType )
    {
        m_Errors.Error( err_syntax, pNode->pToken, xfs("Incompatible types for assignment '%ls' & '%ls'", pLeft->Type.pType->Name, pRight->Type.pType->Name) );
    }

    // Emit code
    switch( pNode->NodeType )
    {
    case ast_assign:
        {
            // Emit expression on right side to get r-value to assign
            EmitExpressionOp( pRight, RVALUE );

            // Emit expression on left side to get l-value to assign to
            if( !CanGetlvalue( pLeft ) )
            {
                m_Errors.Error( err_syntax, pLeft->pToken, "Not an lvalue expression" );
            }
            else
            {
                EmitExpressionOp( pLeft,  LVALUE );
            }

            // Generate code to store result
            if( (pType == g_pTxbool) || (pType == g_pTs32) )
            {
                EmitOpcode( vm_istore );
            }
            else if( pType == g_pTf32 )
            {
                EmitOpcode( vm_fstore );
            }
            else
            {
                // TODO: Class assignment operator : currently assumes reference on right side and
                //       non-reference on left side
                EmitOpcode ( vm_cstore );
                EmitOperand( EmitClassRef( pType->Name ) );
            }
        }
        break;
    default:
        {
            s32 Opcode = vm_nop;
            switch( pNode->NodeType )
            {
            case ast_mul_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_imul;
                else if( pType == g_pTf32 )
                    Opcode = vm_fmul;
                break;
            case ast_div_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_idiv;
                else if( pType == g_pTf32 )
                    Opcode = vm_fdiv;
                break;
            case ast_mod_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_imod;
                else if( pType == g_pTf32 )
                    Opcode = vm_fmod;
                break;
            case ast_add_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_iadd;
                else if( pType == g_pTf32 )
                    Opcode = vm_fadd;
                break;
            case ast_sub_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_isub;
                else if( pType == g_pTf32 )
                    Opcode = vm_fsub;
                break;
            case ast_shl_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_shl;
                break;
            case ast_shr_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_shr;
                break;
            case ast_and_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_bit_and;
                break;
            case ast_xor_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_xor;
                break;
            case ast_or_assign:
                if( pType == g_pTs32 )
                    Opcode = vm_bit_or;
                break;
            }

            ASSERT( Opcode != vm_nop );

            // Emit the expression
            EmitExpressionOp( pLeft,  RVALUE );
            EmitExpressionOp( pRight, RVALUE );
            EmitOpcode( Opcode );

            // Emit expression on left side to get l-value to assign to
            if( !CanGetlvalue( pLeft ) )
            {
                m_Errors.Error( err_syntax, pLeft->pToken, "Not an lvalue expression" );
            }
            else
            {
                EmitExpressionOp( pLeft,  LVALUE );
            }

            // Generate code to store result
            if( (pType == g_pTxbool) || (pType == g_pTs32) )
            {
                EmitOpcode( vm_istore );
            }
            else if( pType == g_pTf32 )
            {
                EmitOpcode( vm_fstore );
            }
            else
            {
                // TODO: More assignment types
                ASSERT( 0 );
            }
        }
    }
}

//==============================================================================
//  EmitExpressionLogicalOp
//==============================================================================

void xsc_codegen::EmitExpressionLogicalOp( xsc_ast_node*    pNode,
                                           const exp_type   eType )
{
    ASSERT( pNode->Children.GetCount() == 2 );

    EmitExpressionOp( pNode->Children[0], RVALUE );
    EmitExpressionOp( pNode->Children[1], RVALUE );

    if( (pNode->Children[0]->Type.pType == g_pTxbool) &&
        (pNode->Children[1]->Type.pType == g_pTxbool) )
    {
        switch( pNode->NodeType )
        {
        case ast_log_or_op:
            EmitOpcode( vm_log_or ); break;
        case ast_log_and_op:
            EmitOpcode( vm_log_and ); break;
        default:
            ASSERT( 0 );
        }
    }
    else
    {
        // TODO: Check for an overloaded operator on the types in question
        // TODO: Better error
        m_Errors.Error( err_syntax, pNode->pToken, "Logical operation needs xbool types" );
    }
}

//==============================================================================
//  EmitExpressionBitwiseOp
//==============================================================================

void xsc_codegen::EmitExpressionBitwiseOp( xsc_ast_node*    pNode,
                                           const exp_type   eType )
{
    ASSERT( pNode->Children.GetCount() == 2 );

    EmitExpressionOp( pNode->Children[0], RVALUE );
    EmitExpressionOp( pNode->Children[1], RVALUE );

    if( (pNode->Children[0]->Type.pType == g_pTs32) &&
        (pNode->Children[1]->Type.pType == g_pTs32) )
    {
        switch( pNode->NodeType )
        {
        case ast_or_op:
            EmitOpcode( vm_bit_or ); break;
        case ast_xor_op:
            EmitOpcode( vm_xor ); break;
        case ast_and_op:
            EmitOpcode( vm_bit_and ); break;
        default:
            ASSERT( 0 );
        }
    }
    else
    {
        // TODO: Check for an overloaded operator on the types in question
        // TODO: Better error
        m_Errors.Error( err_syntax, pNode->pToken, "Bitwise operation needs s32 types" );
    }
}

//==============================================================================
//  EmitExpressionRelationalOp
//==============================================================================

void xsc_codegen::EmitExpressionRelationOp( xsc_ast_node*   pNode,
                                            const exp_type  eType )
{
    ASSERT( pNode->Children.GetCount() == 2 );

    EmitExpressionOp( pNode->Children[0], RVALUE );
    EmitExpressionOp( pNode->Children[1], RVALUE );

    if( (pNode->Children[0]->Type.pType == g_pTs32) &&
        (pNode->Children[1]->Type.pType == g_pTs32) )
    {
        switch( pNode->NodeType )
        {
        case ast_eq_op:
            EmitOpcode( vm_icmp_eq ); break;
        case ast_neq_op:
            EmitOpcode( vm_icmp_ne ); break;
        case ast_lt_op:
            EmitOpcode( vm_icmp_lt ); break;
        case ast_gt_op:
            EmitOpcode( vm_icmp_gt ); break;
        case ast_le_op:
            EmitOpcode( vm_icmp_le ); break;
        case ast_ge_op:
            EmitOpcode( vm_icmp_ge ); break;
        default:
            ASSERT( 0 );
        }
    }
    else if( (pNode->Children[0]->Type.pType == g_pTf32) &&
             (pNode->Children[1]->Type.pType == g_pTf32) )
    {
        switch( pNode->NodeType )
        {
        case ast_eq_op:
            EmitOpcode( vm_fcmp_eq ); break;
        case ast_neq_op:
            EmitOpcode( vm_fcmp_ne ); break;
        case ast_lt_op:
            EmitOpcode( vm_fcmp_lt ); break;
        case ast_gt_op:
            EmitOpcode( vm_fcmp_gt ); break;
        case ast_le_op:
            EmitOpcode( vm_fcmp_le ); break;
        case ast_ge_op:
            EmitOpcode( vm_fcmp_ge ); break;
        default:
            ASSERT( 0 );
        }
    }
    else
    {
        // TODO: Check for an overloaded operator on the types in question
        // TODO: Better error
        m_Errors.Error( err_syntax, pNode->pToken, xfs("Illegal types for relational operators '%ls' & '%ls'", pNode->Children[0]->Type.pType->Name, pNode->Children[1]->Type.pType->Name) );
    }
}

//==============================================================================
//  EmitExpressionShiftOp
//==============================================================================

void xsc_codegen::EmitExpressionShiftOp( xsc_ast_node*  pNode,
                                         const exp_type eType)
{
    ASSERT( pNode->Children.GetCount() == 2 );

    EmitExpressionOp( pNode->Children[0], RVALUE );
    EmitExpressionOp( pNode->Children[1], RVALUE );

    if( (pNode->Children[0]->Type.pType == g_pTs32) &&
        (pNode->Children[1]->Type.pType == g_pTs32) )
    {
        switch( pNode->NodeType )
        {
        case ast_shl_op:
            EmitOpcode( vm_shl ); break;
        case ast_shr_op:
            EmitOpcode( vm_shr ); break;
        default:
            ASSERT( 0 );
        }
    }
    else
    {
        // TODO: Check for an overloaded operator on the types in question
        // TODO: Better error
        m_Errors.Error( err_syntax, pNode->pToken, "Shift operation needs s32 types" );
    }
}

//==============================================================================
//  EmitExpressionMulAddOp
//==============================================================================

void xsc_codegen::EmitExpressionMulAddOp( xsc_ast_node*     pNode,
                                          const exp_type    eType)
{
    ASSERT( pNode->Children.GetCount() == 2 );

    EmitExpressionOp( pNode->Children[0], RVALUE );
    EmitExpressionOp( pNode->Children[1], RVALUE );

    if( (pNode->Children[0]->Type.pType == g_pTs32) &&
        (pNode->Children[1]->Type.pType == g_pTs32) )
    {
        switch( pNode->NodeType )
        {
        case ast_add_op:
            EmitOpcode( vm_iadd ); break;
        case ast_sub_op:
            EmitOpcode( vm_isub ); break;
        case ast_mul_op:
            EmitOpcode( vm_imul ); break;
        case ast_div_op:
            EmitOpcode( vm_idiv ); break;
        case ast_mod_op:
            EmitOpcode( vm_imod ); break;
        default:
            ASSERT( 0 );
        }
    }
    else if( (pNode->Children[0]->Type.pType == g_pTf32) &&
             (pNode->Children[1]->Type.pType == g_pTf32) )
    {
        switch( pNode->NodeType )
        {
        case ast_add_op:
            EmitOpcode( vm_fadd ); break;
        case ast_sub_op:
            EmitOpcode( vm_fsub ); break;
        case ast_mul_op:
            EmitOpcode( vm_fmul ); break;
        case ast_div_op:
            EmitOpcode( vm_fdiv ); break;
        case ast_mod_op:
            EmitOpcode( vm_fmod ); break;
        default:
            ASSERT( 0 );
        }
    }
    else
    {
        // TODO: Check for an overloaded operator on the types in question
        // TODO: Better error
        m_Errors.Error( err_syntax, pNode->pToken, xfs("Type mismatch '%ls' & '%ls", pNode->Children[0]->Type.pType->Name, pNode->Children[1]->Type.pType->Name) );
    }
}

//==============================================================================
//  EmitExpressionUnaryOp
//==============================================================================

void xsc_codegen::EmitExpressionUnaryOp( xsc_ast_node*  pNode,
                                         const exp_type eType )
{
    ASSERT( pNode->Children.GetCount() == 1 );

    xsc_ast_node* pChild = pNode->Children[0];

    switch( pNode->NodeType )
    {
    case ast_unary_sub_op:
        {
            EmitExpressionOp( pChild, RVALUE );
            if( pChild->Type.pType == g_pTs32 )
                EmitOpcode( vm_ineg );
            else if( pChild->Type.pType == g_pTf32 )
                EmitOpcode( vm_fneg );
            else
                ASSERT( 0 );
            break;
        }
    case ast_unary_not_op:
        ASSERT( 0 );
    case ast_unary_complement_op:
        ASSERT( 0 );
    case ast_pre_inc_op:
        ASSERT( 0 );
    case ast_pre_dec_op:
        ASSERT( 0 );
    }
}

//==============================================================================
//  EmitExpressionMemberOp
//==============================================================================

void xsc_codegen::EmitExpressionMemberOp( xsc_ast_node*     pNode,
                                          const exp_type    eType )
{
    ASSERT( pNode->Children.GetCount() == 2 );

    xsc_ast_node* pChild1 = pNode->Children[0];
    xsc_ast_node* pChild2 = pNode->Children[1];

    if( !CanGetlvalue( pChild1 ) )
    {
        m_Errors.Error( err_syntax, pChild1->pToken, "Not an lvalue expression" );
    }
    else
    {
        EmitExpressionOp( pChild1, LVALUE );
    }

    m_NoEmitThis++;
    EmitExpressionOp( pChild2, eType );
    m_NoEmitThis--;
}

