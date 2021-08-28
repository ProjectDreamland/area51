//==============================================================================
//
//  xsc_ast
//
//==============================================================================

#include "xsc_ast.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_compiler.hpp"

//==============================================================================
//  Defines
//==============================================================================

#define NODES_PER_POOL  1024

//==============================================================================
//  xsc_ast
//==============================================================================

xsc_ast::xsc_ast( const xwstring&         Source,
                  const xsc_tokenizer&    Tokenizer,
                  xsc_errors&             Errors,
                  xsc_symbol_table&       SymbolTable) : m_Source     ( Source ),
                                                         m_Tokenizer  ( Tokenizer ),
                                                         m_Errors     ( Errors ),
                                                         m_SymbolTable( SymbolTable )
{
}

//==============================================================================
//  ~xsc_ast
//==============================================================================

xsc_ast::~xsc_ast( void )
{
}

//==============================================================================
//  NewNode
//==============================================================================

xsc_ast_node* xsc_ast::NewNode( ast_node_type NodeType )
{
    s32 iPool = m_Pools.GetCount()-1;

    // Create a new pool if necessary
    if( (iPool < 0) || (m_Pools[iPool].iNode == m_Pools[iPool].nNodes) )
    {
        pool& Pool = m_Pools.Append();
        Pool.nNodes = NODES_PER_POOL;
        Pool.iNode  = 0;
        Pool.pNodes = new xsc_ast_node[Pool.nNodes];
        iPool = m_Pools.GetCount()-1;
    }

    // Create new node, TODO: Add a pool system for node allocation
    xsc_ast_node* pASTnode = &m_Pools[iPool].pNodes[m_Pools[iPool].iNode++];
    ASSERT( pASTnode );

    // Initialize the node
    pASTnode->NodeType  = NodeType;
    pASTnode->pToken    = NULL;
    pASTnode->pSymbol   = NULL;
    pASTnode->Type      = typeref(g_pTvoid);
    pASTnode->pScope    = NULL;

    // Return the new node
    return pASTnode;
}

//==============================================================================
//  DeleteNode
//==============================================================================

void xsc_ast::DeleteNode( xsc_ast_node* pNode )
{
    // TODO: Implement
    ASSERT( 0 );
}

//==============================================================================
//  Dump
//==============================================================================

static xstring GetSymbolName( const xsc_symbol* pSymbol )
{
    xstring String = "<null>";

    if( pSymbol )
        String = xstring(pSymbol->Name);
    return String;
}

xstring xsc_ast::Dump( xsc_ast_node* pNode ) const
{
    static s32  Indent = 0;
    xstring     Output;
    s32         i;

    if( pNode )
    {
        // Display this node
        { for( i=0 ; i<Indent ; i++ ) Output.AddFormat( " " ); }

        switch( pNode->NodeType )
        {
        case ast_module:
            Output.AddFormat( "<module>\n" ); break;
        case ast_class:
            Output.AddFormat( "<class> '%s' %d\n", (const char*)GetSymbolName(pNode->pSymbol), pNode->pSymbol->StorageSize ); break;

        case ast_class_fielddef:
            Output.AddFormat( "<class_fielddef> '%s %s' %d %d\n", (const char*)GetSymbolName(pNode->Type.pType), (const char*)GetSymbolName(pNode->pSymbol), pNode->pSymbol->StorageOffset, pNode->pSymbol->StorageSize ); break;

        case ast_class_methoddef:
            {
                Output.AddFormat( "<class_methoddef> '%s %s' %d %d\n", (const char*)GetSymbolName(pNode->Type.pType), (const char*)GetSymbolName(pNode->pSymbol), pNode->pSymbol->StorageOffset, pNode->pSymbol->StorageSize );
                for( s32 i=0 ; i<pNode->pSymbol->pChildScope->GetNumSymbols() ; i++ )
                {
                    xsc_symbol* pArg = pNode->pSymbol->pChildScope->GetSymbol(i);
                    { for( s32 j=0 ; j<Indent ; j++ ) Output.AddFormat( " " ); }
                    Output.AddFormat( "                  '%s %s' %d %d\n",  (const char*)GetSymbolName(pArg->Type.pType), (const char*)GetSymbolName(pArg), pArg->StorageOffset, pArg->StorageSize );
                }
                break;
            }

        case ast_var_defs:
            Output.AddFormat( "<var_defs>\n" ); break;

        case ast_var_def:
            Output.AddFormat( "<var_def> '%s %s' %d %d\n", (const char*)GetSymbolName(pNode->pSymbol->Type.pType), (const char*)GetSymbolName(pNode->pSymbol), pNode->pSymbol->StorageOffset, pNode->pSymbol->StorageSize ); break;

        case ast_compound_statement:
            {
                Output.AddFormat( "<compound_statement>\n" );
                if( pNode->pScope )
                {
                    for( s32 i=0 ; i<pNode->pScope->GetNumSymbols() ; i++ )
                    {
                        xsc_symbol* pSymbol = pNode->pScope->GetSymbol(i);
                        { for( s32 j=0 ; j<Indent ; j++ ) Output.AddFormat( " " ); }
                        Output.AddFormat( "                    '%s %s' %d %d\n",  (const char*)GetSymbolName(pSymbol->Type.pType), (const char*)GetSymbolName(pSymbol), pSymbol->StorageOffset, pSymbol->StorageSize );
                    }
                }
                break;
            }
        case ast_if_statement:
            Output.AddFormat( "<if_statement>\n" ); break;
        case ast_while_statement:
            Output.AddFormat( "<while_statement>\n" ); break;
        case ast_for_statement:
            Output.AddFormat( "<for_statement>\n" ); break;
        case ast_continue_statement:
            Output.AddFormat( "<continue_statement>\n" ); break;
        case ast_return_statement:
            Output.AddFormat( "<return_statement>\n" ); break;
        case ast_empty_statement:
            Output.AddFormat( "<empty_statement>\n" ); break;

        case ast_expression:
            Output.AddFormat( "<expression> '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_assign:
            Output.AddFormat( "= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_mul_assign:
            Output.AddFormat( "*= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_div_assign:
            Output.AddFormat( "/= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_mod_assign:
            Output.AddFormat( "%= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_add_assign:
            Output.AddFormat( "+= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_sub_assign:
            Output.AddFormat( "-= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_shl_assign:
            Output.AddFormat( "<<= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_shr_assign:
            Output.AddFormat( ">>= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_and_assign:
            Output.AddFormat( "&= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_xor_assign:
            Output.AddFormat( "^= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_or_assign:
            Output.AddFormat( "|= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_log_or_op:
            Output.AddFormat( "|| '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_log_and_op:
            Output.AddFormat( "&& '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_or_op:
            Output.AddFormat( "| '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_xor_op:
            Output.AddFormat( "^ '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_and_op:
            Output.AddFormat( "& '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_eq_op:
            Output.AddFormat( "== '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_neq_op:
            Output.AddFormat( "!= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_lt_op:
            Output.AddFormat( "< '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_gt_op:
            Output.AddFormat( "> '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_le_op:
            Output.AddFormat( "<= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_ge_op:
            Output.AddFormat( ">= '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_shl_op:
            Output.AddFormat( "<< '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_shr_op:
            Output.AddFormat( ">> '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_add_op:
            Output.AddFormat( "+ '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_sub_op:
            Output.AddFormat( "- '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_mul_op:
            Output.AddFormat( "* '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_div_op:
            Output.AddFormat( "/ '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_mod_op:
            Output.AddFormat( "%% '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_unary_sub_op:
            Output.AddFormat( "- '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_unary_not_op:
            Output.AddFormat( "! '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_unary_complement_op:
            Output.AddFormat( "~ '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_pre_inc_op:
            Output.AddFormat( "++ '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_pre_dec_op:
            Output.AddFormat( "-- '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_const_xbool:
            Output.AddFormat( "%s", (pNode->pToken->Code == T_FALSE) ? "<false>" : "<true>" ); break;
        case ast_const_int:
            Output.AddFormat( "%d '%s'\n", pNode->pToken->ValInt, (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_const_flt:
            Output.AddFormat( "%f '%s'\n", pNode->pToken->ValFlt, (const char*)GetSymbolName(pNode->Type.pType) ); break;
        case ast_const_str:
            Output.AddFormat( "'%ls' '%s'\n", (const xwchar*)pNode->pToken->ValStr, (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_var_ref:
            Output.AddFormat( "<var_ref> '%s' '%s %s' %d %d\n", (const char*)GetSymbolName(pNode->Type.pType), (const char*)GetSymbolName(pNode->pSymbol->Type.pType), (const char*)GetSymbolName(pNode->pSymbol), pNode->pSymbol->StorageOffset, pNode->pSymbol->StorageSize ); break;

        case ast_method:
            Output.AddFormat( "<method> '%s' '%s %s'\n", (const char*)GetSymbolName(pNode->Type.pType), (const char*)GetSymbolName(pNode->pSymbol->Type.pType), (const char*)GetSymbolName(pNode->pSymbol) ); break;

        case ast_member_op:
            Output.AddFormat( "<member_op> '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        case ast_cast:
            Output.AddFormat( "<cast> '%s'\n", (const char*)GetSymbolName(pNode->Type.pType) ); break;

        default:
            Output.AddFormat( "<unknown node>\n"); break;
        }

        // Display children
        Indent += 2;
        for( i=0 ; i<pNode->Children.GetCount() ; i++ )
            Output += Dump( pNode->Children[i] );
        Indent -= 2;
    }
    else
    {
        // Display null node
        { for( i=0 ; i<Indent ; i++ ) Output.AddFormat( " " ); }
        Output.AddFormat( "<null node>\n" );
    }

    // Return dump
    return Output;
}

//==============================================================================
