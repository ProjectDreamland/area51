//==============================================================================
//  
//  xsc_parser.hpp
//  
//==============================================================================

#ifndef XSC_PARSER_HPP
#define XSC_PARSER_HPP

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
class   xsc_ast;
class   xsc_ast_node;

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  xsc_parser
//==============================================================================

class xsc_parser
{
//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_parser              ( const xwstring&       Source,
                                                  const xsc_tokenizer&  Tokenizer,
                                                  xsc_errors&           Errors,
                                                  xsc_symbol_table&     SymbolTable,
                                                  xsc_ast&              AST );
                       ~xsc_parser              ( void );

    xbool               Parse                   ( void );

    xsc_ast_node*       GetModuleNode           ( void ) const;                 // Get module node pointer

    // Debugging
    void                Dump                    ( void ) const;                 // Dump to STDOUT

private:
    // Import Parsing
    void                ParseImport             ( void );                       // Parse import directive
    void                ParseTypedef            ( void );                       // Parse import typedef

    // Class Parsing
    xsc_ast_node*       ParseClass              ( void );                       // Parse a class definition
    void                ParseClassMember        ( void );                       // Parse a class member definition

    // Statement Parsing
    xsc_ast_node*       ParseStatement          ( void );                       // Parse a statement
    xsc_ast_node*       ParseCompoundStatement  ( xbool NewScope = TRUE );      // { statement-list }
    xsc_ast_node*       ParseIfStatement        ( void );
    xsc_ast_node*       ParseWhileStatement     ( void );
    xsc_ast_node*       ParseForStatement       ( void );
    xsc_ast_node*       ParseContinueStatement  ( void );
    xsc_ast_node*       ParseReturnStatement    ( void );

    // Expression Parsing
    xsc_ast_node*       ParseExpression         ( xbool VoidResult );           // Parse an expression
    xsc_ast_node*       ParseExpAssignment      ( void );
    xsc_ast_node*       ParseExpConditional     ( void );
    xsc_ast_node*       ParseExpLogicalOR       ( void );
    xsc_ast_node*       ParseExpLogicalAND      ( void );
    xsc_ast_node*       ParseExpOR              ( void );
    xsc_ast_node*       ParseExpXOR             ( void );
    xsc_ast_node*       ParseExpAND             ( void );
    xsc_ast_node*       ParseExpEquality        ( void );
    xsc_ast_node*       ParseExpRelational      ( void );
    xsc_ast_node*       ParseExpShift           ( void );
    xsc_ast_node*       ParseExpAdditive        ( void );
    xsc_ast_node*       ParseExpMultiplicative  ( void );
    xsc_ast_node*       ParseExpUnary           ( void );
    xsc_ast_node*       ParseExpPrimary         ( void );
    xsc_ast_node*       ParseExpPrimaryIdent    ( xsc_scope* pScope = NULL );

    // Types
    void                AddBuiltinTypes         ( void );                           // Add builtin types to symbol table
    xbool               TokenIsType             ( const xsc_token*  pToken );       // TRUE if supplied token is a type
    xsc_symbol*         GetTypeFromToken        ( const xsc_token*  pToken );       // Get Type from Token
    xwstring            GetSignatureFromType    ( typeref           Type );         // Get Signature from Type
    typeref             GetTypeFromSignature    ( xwstring&         Signature,
                                                  xbool             Chomp = FALSE); // Get Type from Signature

    // Consume expected token, adding an error if it is incorrect
    xbool               Expect                  ( s32           TokenCode,
                                                  s32           ErrorCode,
                                                  const xwchar* ErrorString );  // Check for expected token in stream
    // Token Access
    const xsc_token*    ReadToken               ( s32 Count = 1 );              // Read next token and increment pointer
    void                UnreadToken             ( s32 Count = 1 );              // Unread the last token read
    const xsc_token*    LookAhead               ( s32 Offset = 0 ) const;       // Take a peek at succeeding tokens

    // Token Classification
    xbool               IsEOF                   ( const xsc_token* t ) const;

//==============================================================================
//  Data
//==============================================================================

protected:
    const xwstring&         m_Source;                   // Reference to source code
    const xsc_tokenizer&    m_Tokenizer;                // Reference to tokenizer
    xsc_errors&             m_Errors;                   // Reference to errors
    xsc_symbol_table&       m_SymbolTable;              // Reference to symbol table
    xsc_ast&                m_AST;                      // Reference to abstract syntax tree

    xtimer                  m_ParseTimer;               // Timer for parsing

    // Token consumption
    s32                     m_Index;                    // Current token index
    s32                     m_EOFIndex;                 // End of File token Index
    const xsc_token*        m_t;                        // Current token

    // Parsing State information
    xsc_ast_node*           m_pModuleNode;              // Node representing module
    xsc_symbol*             m_pClassSymbol;             // Symbol representing current class
    xsc_ast_node*           m_pClassNode;               // Node representing class
    xsc_symbol*             m_pMethodSymbol;            // Symbol representing current method
    xsc_scope*              m_pMethodScope;             // Scope representing current method
};

//==============================================================================
#endif // XSC_PARSER_HPP
//==============================================================================
