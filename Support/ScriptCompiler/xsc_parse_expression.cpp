//==============================================================================
//
//  xsc_parse_expression
//
//==============================================================================

#include "xsc_parser.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "xsc_compiler.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  ParseExpression
//==============================================================================

xsc_ast_node* xsc_parser::ParseExpression( xbool VoidResult )
{
    // Create Node
    xsc_ast_node* pExpressionNode = m_AST.NewNode( ast_expression );

    // If VoidResult is requested then add a void cast
    if( VoidResult )
    {
        xsc_ast_node* pVoidCastNode   = m_AST.NewNode( ast_cast );
        pVoidCastNode->Type = typeref(g_pTvoid);
        xsc_ast_node* pExpAssignment = ParseExpAssignment();
        if( pExpAssignment )
        {
            pVoidCastNode->Children.Append() = pExpAssignment;
        }
        pExpressionNode->Children.Append() = pVoidCastNode;
        pExpressionNode->Type = typeref(g_pTvoid);
    }
    else
    {
        xsc_ast_node* pExpAssignment = ParseExpAssignment();
        if( pExpAssignment )
        {
            pExpressionNode->Children.Append() = pExpAssignment;
            pExpressionNode->Type = pExpAssignment->Type;
        }
    }

    return pExpressionNode;
}

xsc_ast_node* xsc_parser::ParseExpAssignment( void )
{
    // Implement = *= /= %= += -= <<= >>= &= ^= |=
    xsc_ast_node* pLeftNode = ParseExpConditional();

    // If valid expression
    if( pLeftNode )
    {
        // Check for assignment operator
        if( (m_t->Code == T_ASSIGN    ) || (m_t->Code == T_MUL_ASSIGN) || (m_t->Code == T_DIV_ASSIGN) ||
            (m_t->Code == T_MOD_ASSIGN) || (m_t->Code == T_ADD_ASSIGN) || (m_t->Code == T_SUB_ASSIGN) ||
            (m_t->Code == T_SHL_ASSIGN) || (m_t->Code == T_SHR_ASSIGN) ||
            (m_t->Code == T_AND_ASSIGN) || (m_t->Code == T_XOR_ASSIGN) || (m_t->Code == T_OR_ASSIGN ) )
        {
            // Save operator token
            const xsc_token* pOperatorToken = m_t;

            // Read right side of assignment expression
            ReadToken();
            xsc_ast_node* pRightNode = ParseExpAssignment();

            // If valid expression
            if( pRightNode )
            {
                ast_node_type NodeType;

                switch( pOperatorToken->Code )
                {
                case T_ASSIGN:
                    NodeType = ast_assign; break;
                case T_MUL_ASSIGN:
                    NodeType = ast_mul_assign; break;
                case T_DIV_ASSIGN:
                    NodeType = ast_div_assign; break;
                case T_MOD_ASSIGN:
                    NodeType = ast_mod_assign; break;
                case T_ADD_ASSIGN:
                    NodeType = ast_add_assign; break;
                case T_SUB_ASSIGN:
                    NodeType = ast_sub_assign; break;
                case T_SHL_ASSIGN:
                    NodeType = ast_shl_assign; break;
                case T_SHR_ASSIGN:
                    NodeType = ast_shr_assign; break;
                case T_AND_ASSIGN:
                    NodeType = ast_and_assign; break;
                case T_XOR_ASSIGN:
                    NodeType = ast_xor_assign; break;
                case T_OR_ASSIGN:
                    NodeType = ast_or_assign; break;
                default:
                    ASSERT( 0 );
                }

                // Create node for operation
                xsc_ast_node* pOperatorNode      = m_AST.NewNode( NodeType );
                pOperatorNode->pToken            = pOperatorToken;
                pOperatorNode->Children.Append() = pLeftNode;
                pOperatorNode->Children.Append() = pRightNode;
                pOperatorNode->Type              = pRightNode->Type;

                // Return operation node
                return pOperatorNode;
            }
            else
            {
                m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of assignment" );
            }
        }
    }

    // Return left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpConditional( void )
{
    // Implement ?:
    return ParseExpLogicalOR();
}

xsc_ast_node* xsc_parser::ParseExpLogicalOR( void )
{
    // Implement ||

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpLogicalAND();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_LOG_OR_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpLogicalAND();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( ast_log_or_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTxbool);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpLogicalAND( void )
{
    // Implement &&

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpOR();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_LOG_AND_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpOR();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( ast_log_and_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTxbool);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpOR( void )
{
    // Implement |

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpXOR();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_OR_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpXOR();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( ast_or_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTs32);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpXOR( void )
{
    // Implement ^

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpAND();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_XOR_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpAND();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( ast_xor_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTs32);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpAND( void )
{
    // Implement &

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpEquality();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_AND_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpEquality();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( ast_and_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTs32);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpEquality( void )
{
    // Implement == !=

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpRelational();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_EQ_OP) || (m_t->Code == T_NEQ_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpRelational();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( (pOperatorToken->Code == T_EQ_OP) ? ast_eq_op : ast_neq_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTxbool);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpRelational( void )
{
    // Implement < > <= >=

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpShift();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_LT_OP) || (m_t->Code == T_GT_OP) || 
                         (m_t->Code == T_LE_OP) || (m_t->Code == T_GE_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpShift();

        // If valid expression
        if( pRightNode )
        {
            ast_node_type NodeType;

            switch( pOperatorToken->Code )
            {
            case T_LT_OP:
                NodeType = ast_lt_op; break;
            case T_GT_OP:
                NodeType = ast_gt_op; break;
            case T_LE_OP:
                NodeType = ast_le_op; break;
            case T_GE_OP:
                NodeType = ast_ge_op; break;
            default:
                ASSERT( 0 );
            }

            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( NodeType );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTxbool);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpShift( void )
{
    // Implement << >>

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpAdditive();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_SHL_OP) || (m_t->Code == T_SHR_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpAdditive();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( (pOperatorToken->Code == T_SHL_OP) ? ast_shl_op : ast_shr_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = typeref(g_pTs32);

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpAdditive( void )
{
    // Implement + -

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpMultiplicative();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_ADD_OP) || (m_t->Code == T_SUB_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpMultiplicative();

        // If valid expression
        if( pRightNode )
        {
            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( (pOperatorToken->Code == T_ADD_OP) ? ast_add_op : ast_sub_op );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = pLeftNode->Type; // TODO: Add correct type resolution

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpMultiplicative( void )
{
    // Implement * / %

    // Parse left side
    xsc_ast_node* pLeftNode = ParseExpUnary();

    // If valid expression
    while( pLeftNode && ((m_t->Code == T_MUL_OP) || (m_t->Code == T_DIV_OP) || (m_t->Code == T_MOD_OP)) )
    {
        // Save operator token
        const xsc_token* pOperatorToken = m_t;

        // Read right side of assignment expression
        ReadToken();
        xsc_ast_node* pRightNode = ParseExpUnary();

        // If valid expression
        if( pRightNode )
        {
            ast_node_type NodeType;

            switch( pOperatorToken->Code )
            {
            case T_MUL_OP:
                NodeType = ast_mul_op; break;
            case T_DIV_OP:
                NodeType = ast_div_op; break;
            case T_MOD_OP:
                NodeType = ast_mod_op; break;
            default:
                ASSERT( 0 );
            }

            // Create node for operation
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( NodeType );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pLeftNode;
            pOperatorNode->Children.Append() = pRightNode;
            pOperatorNode->Type              = pLeftNode->Type; // TODO: Add correct type resolution

            // Set new Left node and go back around the while loop
            pLeftNode = pOperatorNode;
        }
        else
        {
            m_Errors.Error( err_syntax, pOperatorToken, "No expression on right side of operator" );
        }
    }

    // Return the left node
    return pLeftNode;
}

xsc_ast_node* xsc_parser::ParseExpUnary( void )
{
    // Implement + - ! ~ ++x --x (T)x

    // Check for a cast operator of form "( [const] <type> [&] )"
    // TODO: Finish to recognize type correctly, currently does not do qualifier or reference op
    if( (m_t->Code == T_LPAREN) & TokenIsType( LookAhead(0) ) && (LookAhead(1)->Code == T_RPAREN) )
    {
        // Comsume and keep type token around
        ReadToken();
        const xsc_token* pTypeToken = m_t;
        ReadToken();
        ReadToken();

        // Find symbol from type token
        xsc_symbol* pTypeSymbol = m_SymbolTable.GetSymbol( pTypeToken->ValStr );
        if( pTypeSymbol == NULL )
        {
            // Undefined type
            m_Errors.Error( err_syntax, pTypeToken, xfs("Undefined type '%ls'", pTypeToken->ValStr) );
            pTypeSymbol = g_pTs32;
        }

        // Parse following expression
        xsc_ast_node* pExpNode = ParseExpUnary();

        // Add cast into tree
        if( pExpNode )
        {
            // Add cast operator into tree
            xsc_ast_node* pCastNode = m_AST.NewNode( ast_cast );
            pCastNode->Children.Append() = pExpNode;
            pCastNode->pToken  = pTypeToken;
            pCastNode->pSymbol = pTypeSymbol;
            pCastNode->Type    = typeref(pTypeSymbol);
            return pCastNode;
        }

        // Just return the expression
        return pExpNode;
    }

    // Unary Plus
    if( m_t->Code == T_ADD_OP )
    {
        // Do nothing for Unary Plus
        ReadToken();
        return ParseExpPrimary();
    }

    // Unary operators
    if( (m_t->Code == T_SUB_OP) || (m_t->Code == T_NOT_OP) || (m_t->Code == T_COMPLEMENT_OP) || 
        (m_t->Code == T_INC_OP) || (m_t->Code == T_DEC_OP) )
    {
        // Save token
        const xsc_token* pOperatorToken = m_t;
        ReadToken();

        // Parse primary expression following unary
        xsc_ast_node* pOperandNode = ParseExpPrimary();
        if( pOperandNode )
        {
            ast_node_type NodeType;

            switch( pOperatorToken->Code )
            {
            case T_SUB_OP:
                NodeType = ast_unary_sub_op; break;
            case T_NOT_OP:
                NodeType = ast_unary_not_op; break;
            case T_COMPLEMENT_OP:
                NodeType = ast_unary_complement_op; break;
            case T_INC_OP:
                NodeType = ast_pre_inc_op; break;
            case T_DEC_OP:
                NodeType = ast_pre_dec_op; break;
            default:
                ASSERT( 0 );
            }

            // Create syntax tree node
            xsc_ast_node* pOperatorNode      = m_AST.NewNode( NodeType );
            pOperatorNode->pToken            = pOperatorToken;
            pOperatorNode->Children.Append() = pOperandNode;
            pOperatorNode->Type              = pOperandNode->Type;
            return pOperatorNode;
        }
        else
        {
            // No valid expression followed the unary operator
            m_Errors.Error( err_syntax, m_t, "Expecting Primary expression" );
            return NULL;
        }
    }

    return ParseExpPrimary();
}

xsc_ast_node* xsc_parser::ParseExpPrimary( void )
{
    xsc_ast_node*   pReturnNode = NULL;

    // Implement (x) constant x f(x) x.y
    // TODO: a[x] x++ x-- new typeof sizeof checked unchecked

    // Check for "(x)"
    if( m_t->Code == T_LPAREN )
    {
        // Just an expression in parens
        ReadToken();
        xsc_ast_node* pExpNode = ParseExpAssignment();
        Expect( T_RPAREN, err_syntax, L"Expecting ')'" );
        pReturnNode = pExpNode;
    }

    // Check for constant xbool, integer, float or string
    else if( (m_t->Code == T_FALSE) ||
             (m_t->Code == T_TRUE) ||
             (m_t->Code == T_CONST_INT) ||
             (m_t->Code == T_CONST_FLT) ||
             (m_t->Code == T_CONST_STR) )
    {
        ast_node_type   NodeType = ast_null;
        xsc_symbol*     pType = NULL;

        switch( m_t->Code )
        {
        case T_FALSE:
            NodeType = ast_const_xbool; pType = g_pTxbool;  break;
        case T_TRUE:
            NodeType = ast_const_xbool; pType = g_pTxbool;  break;
        case T_CONST_INT:
            NodeType = ast_const_int; pType = g_pTs32;      break;
        case T_CONST_FLT:
            NodeType = ast_const_flt; pType = g_pTf32;      break;
        case T_CONST_STR:
            NodeType = ast_const_str; pType = g_pTstr;      break;
        default:
            ASSERT( 0 );
        }

        xsc_ast_node* pConstNode = m_AST.NewNode( NodeType );
        pConstNode->pToken = m_t;
        pConstNode->Type   = typeref(pType);
        ReadToken();

        // TODO: Need to flag node as const
        pReturnNode = pConstNode;
    }

    // Check for an identifier
    else if( m_t->Code == T_IDENTIFIER )
    {
        // Parse for primary identifier f(x) or x
        xsc_ast_node* pNode = ParseExpPrimaryIdent();
        if( pNode )
        {
            // Check for . operator
            while( m_t->Code == T_DOT_OP )
            {
                const xsc_token* pMemberOpToken = m_t;

                // Get next token
                ReadToken();

                // Check if pNode is a class type
                if( pNode->Type.pType->SymbolType != symtype_class )
                {
                    m_Errors.Error( err_syntax, pNode->pToken, xfs("'%ls' is not a class", pNode->pToken->ValStr) );
                    break;
                }

                // Check if it's an identifier
                if( m_t->Code == T_IDENTIFIER )
                {
                    // Get member access node
                    xsc_ast_node* pMemberNode = ParseExpPrimaryIdent( pNode->Type.pType->pChildScope );
                    if( pMemberNode )
                    {
                        // Create new node to combine the 2
                        xsc_ast_node* pMemberOpNode = m_AST.NewNode( ast_member_op );
                        pMemberOpNode->Children.Append() = pNode;
                        pMemberOpNode->Children.Append() = pMemberNode;
                        pMemberOpNode->pToken = pMemberOpToken;
                        pMemberOpNode->Type   = pMemberNode->Type;
                        pNode = pMemberOpNode;
                    }
                    else
                    {
                        // TODO: Need anything here
                    }
                }
            }

            // Return the node;
            pReturnNode = pNode;
        }
    }

    // TODO: Process rest of possibilities

    // Check for post increment or decrement operators
    if( pReturnNode && ((m_t->Code == T_INC_OP) || (m_t->Code == T_DEC_OP)) )
    {
        ASSERT( 0 );
    }

    return pReturnNode;
}

xsc_ast_node* xsc_parser::ParseExpPrimaryIdent( xsc_scope* pScope )
{
    // Implement f(x) or x

    xsc_symbol* pSymbol = NULL;

    // Lookup in symbol table
    if( pScope )
        pSymbol = pScope->GetSymbol( m_t->ValStr );
    else
        pSymbol = m_SymbolTable.GetSymbol( m_t->ValStr );

    // Found?
    if( pSymbol )
    {
        // Is it a Method?
        if( pSymbol->SymbolType == symtype_method )
        {
            // Setup Method AST node
            xsc_ast_node* pMethodNode = m_AST.NewNode( ast_method );
            pMethodNode->pToken  = m_t;
            pMethodNode->pSymbol = pSymbol;
            pMethodNode->Type    = pSymbol->Type;

            ReadToken();
            Expect( T_LPAREN, err_syntax, L"Expecting '('" );

            // TODO: Need to locate symbol after arguments have been read so we can
            //       match up the function signatures

            // Process arguments
            while( (m_t->Code != T_RPAREN) && (m_t->Code != T_EOF) )
            {
                xsc_ast_node* pNode = ParseExpression( FALSE );
                if( pNode )
                    pMethodNode->Children.Append() = pNode;

                // Exit if last argument
                if( m_t->Code == T_RPAREN )
                    break;

                // Expect a comma seperating arguments
                Expect( T_COMMA_OP, err_syntax, L"Expecting ','" );
            }

            // Expect a closing paren
            Expect( T_RPAREN, err_syntax, L"Expecting ')'" );

            // Return the method node
            return pMethodNode;
        }
        else if( (pSymbol->SymbolType == symtype_field) ||
                 (pSymbol->SymbolType == symtype_argument) ||
                 (pSymbol->SymbolType == symtype_local) )
        {
            // TOO: Finish for array and member access
            xsc_ast_node* pVarNode = m_AST.NewNode( ast_var_ref );
            pVarNode->pToken  = m_t;
            pVarNode->pSymbol = pSymbol;
            pVarNode->Type    = typeref(pSymbol->Type);

            ReadToken();

            return pVarNode;
        }
    }
    else
    {
        m_Errors.Error( err_syntax, m_t, xfs("Undefined Identifier '%ls'", m_t->ValStr) );
    }

    // Return nothing
    return NULL;
}

//==============================================================================
