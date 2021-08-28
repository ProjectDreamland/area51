//==============================================================================
//  
//  xsc_codegen.hpp
//  
//==============================================================================

#ifndef XSC_CODEGEN_HPP
#define XSC_CODEGEN_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "../ScriptVM/xsc_vm_fileformat.hpp"
#include "x_bytestream.hpp"

class   xsc_errors;
class   xsc_tokenizer;
struct  xsc_token;
class   xsc_symbol_table;
class   xsc_symbol;
class   xsc_ast;
class   xsc_ast_node;
class   xsc_parser;

//==============================================================================
//  Defines
//==============================================================================

enum exp_type
{
    RVALUE = (1<<0),
    LVALUE = (1<<1),
    AVALUE = (1<<2)
};

//==============================================================================
//  xsc_codegen
//==============================================================================

class xsc_codegen
{
//==============================================================================
//  Constant Str Record
//==============================================================================
    struct str_record
    {
        s32     Offset;
        s32     Length;
        xstring String;
    };

//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_codegen             ( const xwstring&       Source,
                                                  const xsc_tokenizer&  Tokenizer,
                                                  xsc_errors&           Errors,
                                                  xsc_symbol_table&     SymbolTable,
                                                  xsc_ast&              AST,
                                                  xsc_parser&           Parser );
                       ~xsc_codegen             ( void );

    xbool               Save                    ( const char*           pFileName );
    void                EmitModule              ( xsc_ast_node*         pModuleNode );

    // Debugging
    void                Dump                    ( void ) const;                     // Dump debug info

private:
    // AST Code Generation
    void                EmitClass               ( xsc_ast_node*         pClassNode );
    void                EmitMethod              ( xsc_ast_node*         pMethodNode );
    void                EmitField               ( xsc_ast_node*         pFieldNode );
    void                EmitCompoundStatement   ( xsc_ast_node*         pStatementNode );
    void                EmitStatement           ( xsc_ast_node*         pNode );
    void                EmitIfStatement         ( xsc_ast_node*         pStatementNode );
    void                EmitWhileStatement      ( xsc_ast_node*         pStatementNode );
    void                EmitForStatement        ( xsc_ast_node*         pStatementNode );
    void                EmitContinueStatement   ( xsc_ast_node*         pStatementNode );
    void                EmitReturnStatement     ( xsc_ast_node*         pStatementNode );
    void                EmitExpression          ( xsc_ast_node*         pExpressionNode,    const exp_type  eType = RVALUE );
    void                EmitExpressionOp        ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionMethod    ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionVarRefOp  ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionCastOp    ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionAssignOp  ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionLogicalOp ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionBitwiseOp ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionRelationOp( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionShiftOp   ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionMulAddOp  ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionUnaryOp   ( xsc_ast_node*         pNode,              const exp_type  eType );
    void                EmitExpressionMemberOp  ( xsc_ast_node*         pNode,              const exp_type  eType );

    // Code Gen utility functions
    xbool               CanGetlvalue            ( const xsc_ast_node*   pNode ) const;
    s32                 GetArgumentsByteSize    ( const xsc_symbol*     pSymbol ) const;


    // Primitive Code Generation
    s32                 EmitConstInt            ( s32                   ConstVal );
    s32                 EmitConstFlt            ( f32                   ConstVal );
    s32                 EmitConstStr            ( const xwstring&       ConstVal );
    s32                 EmitClassDef            ( const xwstring&       ClassName,
                                                  s32                   ByteSize );
    s32                 EmitClassRef            ( const xwstring&       ClassName );
    s32                 EmitMethodDef           ( const xwstring&       ClassName,
                                                  const xwstring&       MethodName,
                                                  const xwstring&       Signature,
                                                  s32                   ArgumentsSize,
                                                  s32                   StackSize,
                                                  s32                   LocalSize,
                                                  s32                   ReturnSize,
                                                  u32                   Flags );
    s32                 EmitMethodRef           ( const xwstring&       ClassName,
                                                  const xwstring&       MethodName,
                                                  const xwstring&       Signature );
    s32                 EmitFieldDef            ( const xwstring&       ClassName,
                                                  const xwstring&       FieldName,
                                                  const xwstring&       Signature,
                                                  s32                   FieldByteOffset );
    s32                 EmitFieldRef            ( const xwstring&       ClassName,
                                                  const xwstring&       FieldName );
    void                EmitOpcode              ( s32                   Opcode );
    void                EmitOperand             ( s32                   Operand );
    void                EmitOperandAt           ( s32                   Operand,
                                                  s32                   Address );

//==============================================================================
//  Data
//==============================================================================

protected:
    const xwstring&             m_Source;                   // Reference to source code
    const xsc_tokenizer&        m_Tokenizer;                // Reference to tokenizer
    xsc_errors&                 m_Errors;                   // Reference to errors
    xsc_symbol_table&           m_SymbolTable;              // Reference to symbol table
    xsc_ast&                    m_AST;                      // Reference to abstract syntax tree
    xsc_parser&                 m_Parser;                   // Reference to parser

    // Code Generation Data
    s32                         m_NoEmitThis;
    xsc_vm_header               m_Header;
    xarray<xsc_vm_classdef>     m_ClassDef;
    xarray<xsc_vm_classref>     m_ClassRef;
    xarray<xsc_vm_methoddef>    m_MethodDef;
    xarray<xsc_vm_methodref>    m_MethodRef;
    xarray<xsc_vm_fielddef>     m_FieldDef;
    xarray<xsc_vm_fieldref>     m_FieldRef;
    xarray<s32>                 m_ConstantInt;
    xarray<f32>                 m_ConstantFlt;
    xbytestream                 m_ConstantStr;
    xarray<str_record>          m_ConstantStrIndex;
    xbytestream                 m_Methods;
    xarray<xsc_vm_linenumber>   m_LineNumber;
    xarray<xsc_vm_symbol>       m_Symbol;
};

//==============================================================================
#endif // XSC_CODEGEN_HPP
//==============================================================================
