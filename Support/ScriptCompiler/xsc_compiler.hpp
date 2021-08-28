//==============================================================================
//  
//  xsc_compiler.hpp
//  
//==============================================================================

#ifndef XSC_COMPILER_HPP
#define XSC_COMPILER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

#include "xsc_errors.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_parser.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "xsc_codegen.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Externals
//==============================================================================

// Builtin Types
extern xsc_symbol*  g_pTvoid;                   // void
extern xsc_symbol*  g_pTs32;                    // s32
extern xsc_symbol*  g_pTf32;                    // f32
extern xsc_symbol*  g_pTxbool;                  // xbool
extern xsc_symbol*  g_pTstr;                    // str

//==============================================================================
//  xsc_compiler
//==============================================================================

class xsc_compiler
{
//==============================================================================
//  Functions
//==============================================================================
public:
                    xsc_compiler        ( void );
                   ~xsc_compiler        ( void );

    xbool           Init                ( const xstring& InputName );

    xbool           Compile             ( const xstring& OutputName );

    s32             GetNumErrors        ( void ) const;
    s32             GetNumWarnings      ( void ) const;
    xstring         DumpErrors          ( void ) const;
    xstring         DumpSymbolTable     ( void ) const;
    xstring         DumpAST             ( void ) const;

//==============================================================================
//  Data
//==============================================================================

protected:
    xbool                   m_Initialized;              // TRUE when initialized
    xtimer                  m_InitTimer;                // Initialization timer

    xstring                 m_SourceName;               // Source code pathname
    xwstring                m_Source;                   // Source code

    xsc_errors              m_Errors;                   // Error / Warning Tracker
    xsc_tokenizer           m_Tokenizer;                // Tokenizer
    xsc_symbol_table        m_SymbolTable;              // Symbol Table
    xsc_parser              m_Parser;                   // Parser
    xsc_ast                 m_AST;                      // Abstract Syntax Tree
    xsc_codegen             m_CodeGenerator;            // Code Generator
public:
};

//==============================================================================
#endif // XSC_COMPILER_HPP
//==============================================================================
