//==============================================================================
//
//  xsc_compiler
//
//==============================================================================
//  Notes:
//
//  Different systems use different newline encodings, here are the most common,
//  converting these to a single LF (0a) is handled in the Init function
//
//      MAC     CR (0d)
//      DOS     CR (0d) LF (0a)
//      UNIX    LF (0a)
//
//==============================================================================

#include "xsc_compiler.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Data
//==============================================================================

// Builtin Types
xsc_symbol*  g_pTvoid       = NULL;                 // void
xsc_symbol*  g_pTs32        = NULL;                 // s32
xsc_symbol*  g_pTf32        = NULL;                 // f32
xsc_symbol*  g_pTxbool      = NULL;                 // xbool
xsc_symbol*  g_pTstr        = NULL;                 // str

//==============================================================================
//  xsc_compiler
//==============================================================================

xsc_compiler::xsc_compiler( void ) : m_Errors       ( m_Source ),
                                     m_Tokenizer    ( m_Source, m_Errors ),
                                     m_AST          ( m_Source, m_Tokenizer, m_Errors, m_SymbolTable ),
                                     m_Parser       ( m_Source, m_Tokenizer, m_Errors, m_SymbolTable, m_AST ),
                                     m_CodeGenerator( m_Source, m_Tokenizer, m_Errors, m_SymbolTable, m_AST, m_Parser )
{
    m_Initialized = FALSE;
}

//==============================================================================
//  ~xsc_tokenizer
//==============================================================================

xsc_compiler::~xsc_compiler( void )
{
}

//==============================================================================
//  Init
//==============================================================================

xbool xsc_compiler::Init( const xstring& InputName )
{
    xwstring    TempSource;
    xbool       Success = FALSE;

    // Start Timer
    m_InitTimer.Start();

    // Save source name
    m_SourceName = InputName;
    m_Errors.SetSourceName( m_SourceName );

    // Load source
    xbool Loaded = TempSource.LoadFile( m_SourceName );
    if( Loaded )
    {
        s32 i;
        s32 j;
        s32 c;
        s32 SourceLen  = TempSource.GetLength();
        s32 SourceLen1 = SourceLen-1;

        // Convert end of line characters to standard (0x0a) CHAR_LF format
        i = 0;
        j = 0;
        c = TempSource[i];
        while( i < SourceLen )
        {
            if( (c == 0x0d) )
            {
                if( (i < SourceLen1) && (TempSource[i+1] != 0x0a) )
                    TempSource[j++] = 0x0a;
            }
            else
            {
                TempSource[j++] = c;
            }
            c = TempSource[++i];
        }

        // Copy temporary buffer to final buffer
        m_Source = TempSource.Left( j );

        // Success
        Success = TRUE;
    }
    else
    {
        // No - failed to load
//        x_printf( "Error - Can't load source '%s'\n", m_SourceName );
    }

    // Stop Timer
    m_InitTimer.Stop();

    // Return success code
    return Success;
}

//==============================================================================
//  Compile
//==============================================================================

xbool xsc_compiler::Compile( const xstring& OutputName )
{
    xbool   Success = TRUE;

    // Initialize subsystems
    m_Tokenizer.Init( m_Source.GetLength() );

    // Tokenize the source
    Success &= m_Tokenizer.Tokenize( );
    if( Success )
    {
        Success &= m_Parser.Parse( );
        if( Success )
        {
            // Generate Code
            if( m_Parser.GetModuleNode() )
            {
                // Emit Module to code generator
                m_CodeGenerator.EmitModule( m_Parser.GetModuleNode() );

                // Debug displays
//                m_Parser.DumpTree();
//                m_AST.DumpTree( m_pModuleNode );
//                m_Errors.DumpErrors();
//                m_CodeGenerator.Dump( );

                // If no errors then output binary
                if( m_Errors.GetNumErrors() == 0 )
                {
                    // Save compiled module
                    Success &= m_CodeGenerator.Save( OutputName+".xsc" );
                    m_SymbolTable.Save( OutputName+".sym" );
                }
                else
                {
                    // No Success if errors
                    Success = FALSE;
                }
            }
            else
            {
                // No success if no Module Node
                Success = FALSE;
            }
        }
    }

    // Return success code
    return Success;
}

//==============================================================================
//  GetNumErrors
//==============================================================================

s32 xsc_compiler::GetNumErrors( void ) const
{
    return m_Errors.GetNumErrors();
}

//==============================================================================
//  GetNumWarnings
//==============================================================================

s32 xsc_compiler::GetNumWarnings( void ) const
{
    return m_Errors.GetNumWarnings();
}

//==============================================================================
//  DumpErrors
//==============================================================================

xstring xsc_compiler::DumpErrors( void ) const
{
    return m_Errors.Dump();
}

//==============================================================================
//  DumpSymbolTable
//==============================================================================

xstring xsc_compiler::DumpSymbolTable( void ) const
{
    return m_SymbolTable.Dump();
}

//==============================================================================
//  DumpAST
//==============================================================================

xstring xsc_compiler::DumpAST( void ) const
{
    xstring Output = "AST\n"
                     "---\n";
    return Output + m_AST.Dump( m_Parser.GetModuleNode() );
}

//==============================================================================
