//==============================================================================
//  
//  xsc_ast.hpp
//  
//==============================================================================

#ifndef XSC_AST_HPP
#define XSC_AST_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

#include "xsc_symbol_table.hpp"

class   xsc_errors;
class   xsc_tokenizer;
struct  xsc_token;
class   xsc_symbol_table;
class   xsc_symbol;
class   xsc_scope;

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Node Types
//==============================================================================

enum ast_node_type
{
    ast_null,

    ast_module,                 // Module Defintion

    ast_class,                  // Class Definition
    ast_class_fielddef,         // Class Field Definition
    ast_class_methoddef,        // Class Method Definition

    ast_var_defs,               // Group of Variable Definitions
    ast_var_def,                // Variable Definition

    ast_compound_statement,     // Compound statement {}
    ast_if_statement,           // If statement
    ast_while_statement,        // While statement
    ast_for_statement,          // For statement
    ast_continue_statement,     // Continue statement
    ast_return_statement,       // Return statement
    ast_empty_statement,        // Empty statement

    ast_expression,             // Expression

    ast_assign,                 // Expression assigment
    ast_mul_assign,             // Expression assigment
    ast_div_assign,             // Expression assigment
    ast_mod_assign,             // Expression assigment
    ast_add_assign,             // Expression assigment
    ast_sub_assign,             // Expression assigment
    ast_shl_assign,             // Expression assigment
    ast_shr_assign,             // Expression assigment
    ast_and_assign,             // Expression assigment
    ast_xor_assign,             // Expression assigment
    ast_or_assign,              // Expression assigment
    
    ast_log_or_op,              // Expression logical or
    ast_log_and_op,             // Expression logical and
    
    ast_or_op,                  // Expression or
    ast_xor_op,                 // Expression xor
    ast_and_op,                 // Expression and
    
    ast_eq_op,                  // Expression ==
    ast_neq_op,                 // Expression !=
    ast_lt_op,                  // Expression <
    ast_gt_op,                  // Expression >
    ast_le_op,                  // Expression <=
    ast_ge_op,                  // Expression >=

    ast_shl_op,                 // Expression <<
    ast_shr_op,                 // Expression >>
    
    ast_add_op,                 // Expression +
    ast_sub_op,                 // Expression -
    ast_mul_op,                 // Expression *
    ast_div_op,                 // Expression /
    ast_mod_op,                 // Expression %
    
    ast_unary_sub_op,           // Expression -
    ast_unary_not_op,           // Expression !
    ast_unary_complement_op,    // Expression ~
    ast_pre_inc_op,             // Expression ++
    ast_pre_dec_op,             // Expression --

    ast_const_xbool,            // Boolean constant
    ast_const_int,              // Integer constant
    ast_const_flt,              // Float constant
    ast_const_str,              // String constant

    ast_var_ref,                // Variable reference
    ast_method,                 // Method

    ast_member_op,              // .

    ast_cast,                   // cast operation
};

//==============================================================================
//  xsc_ast_node
//==============================================================================

class xsc_ast_node
{
public:
    ast_node_type           NodeType;               // Type of node
    const xsc_token*        pToken;                 // Token for this node
    xsc_symbol*             pSymbol;                // Symbol for this node
    typeref                 Type;                   // Type for this node
    xsc_scope*              pScope;                 // Symbol scope for this node
    xarray<xsc_ast_node*>   Children;               // Child nodes
};

//==============================================================================
//  xsc_ast
//==============================================================================

class xsc_ast
{
//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_ast                 ( const xwstring&       Source,
                                                  const xsc_tokenizer&  Tokenizer,
                                                  xsc_errors&           Errors,
                                                  xsc_symbol_table&     SymbolTable );
                       ~xsc_ast                 ( void );

    // Abstract Syntax Tree
    xsc_ast_node*       NewNode                 ( ast_node_type NodeType );     // Create a new AST node
    void                DeleteNode              ( xsc_ast_node* pNode );        // Delete an AST node & subtree

    // Tree processing
    void                AllocateStorage         ( xsc_ast_node* pNode );        // Allocate storage within scopes
    s32                 GetOperandStackSize     ( xsc_ast_node* pMethodNode );  // Get Size of operand stack needed

    // Debugging
    xstring             Dump                    ( xsc_ast_node* pNode ) const;  // Dump AST tree to STDOUT

//==============================================================================
//  Classes
//==============================================================================
private:
    struct pool
    {
        s32                 nNodes;
        s32                 iNode;
        xsc_ast_node*       pNodes;
    };

//==============================================================================
//  Data
//==============================================================================
protected:
    const xwstring&         m_Source;                   // Reference to source code
    const xsc_tokenizer&    m_Tokenizer;                // Reference to tokenizer
    xsc_errors&             m_Errors;                   // Reference to errors
    xsc_symbol_table&       m_SymbolTable;              // Reference to symbol table

    xarray<pool>            m_Pools;                    // Pools of AST nodes
};

//==============================================================================
#endif // XSC_AST_HPP
//==============================================================================
