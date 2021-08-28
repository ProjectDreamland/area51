//==============================================================================
//
//  xsc_parser
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
//  xsc_parser
//==============================================================================

xsc_parser::xsc_parser( const xwstring&         Source,
                        const xsc_tokenizer&    Tokenizer,
                        xsc_errors&             Errors,
                        xsc_symbol_table&       SymbolTable,
                        xsc_ast&                AST ) : m_Source     ( Source ),
                                                        m_Tokenizer  ( Tokenizer ),
                                                        m_Errors     ( Errors ),
                                                        m_SymbolTable( SymbolTable ),
                                                        m_AST        ( AST )
{
}

//==============================================================================
//  ~xsc_parser
//==============================================================================

xsc_parser::~xsc_parser( void )
{
}

//==============================================================================
//  Parser
//==============================================================================

xbool xsc_parser::Parse( void )
{
    xbool   Success = TRUE;

    // Reset timer
    m_ParseTimer.Start();

    // Create global symbol scope and push onto stack
    xsc_scope* pGlobalScope = m_SymbolTable.NewScope();
    m_SymbolTable.PushScope( pGlobalScope );

    // Add builtin types into symbol table
    AddBuiltinTypes( );

    // Setup token indices and read first token
    m_EOFIndex      = m_Tokenizer.GetNumTokens();
    m_Index         = 0;
    ReadToken();
    s32 PrevIndex   = m_Index-1;

    // Create a new AST node
    m_pModuleNode = m_AST.NewNode( ast_module );
    if( m_pModuleNode )
    {
        m_pModuleNode->NodeType = ast_module;

        // Loop until the tokens are exhausted
        while( !IsEOF(m_t) && (m_Index != PrevIndex) )
        {
            // Set previous index so we can detect no progress conditions
            PrevIndex = m_Index;

            // Is this an import directive
            if( m_t->Code == T_IMPORT )
            {
                ParseImport();
            }
            // Is this a typedef directive
            else if( m_t->Code == T_TYPEDEF )
            {
                ParseTypedef();
            }
            // Is this a class definition
            else if( m_t->Code == T_CLASS )
            {
                // Parse the class definition
                xsc_ast_node* pClassNode = ParseClass( );

                // Add to the module node
                if( pClassNode )
                    m_pModuleNode->Children.Append() = pClassNode;
            }
            else
            {
                // Output an error
                m_Errors.Error( err_syntax, m_t, "Expecting keyword 'class'" );
            }
        }
    }
    else
    {
        // Output an error
        m_Errors.Error( err_memory, m_t, "Out of memory" );
    }

    // Check for successful end of file
    if( !IsEOF(m_t) )
    {
        m_Errors.Error( err_internal, m_t, "Internal compiler error" );
    }

    // Assert that we are at the right scope on exit
    ASSERT( m_SymbolTable.PopScope() == pGlobalScope );

    // Record time taken to parse
    m_ParseTimer.Stop();

    // Return success code
    return Success;
}

//==============================================================================
//  GetModuleNode
//==============================================================================

xsc_ast_node* xsc_parser::GetModuleNode( void ) const
{
    return m_pModuleNode;
}

//==============================================================================
//  AddBuiltinTypes
//==============================================================================

void xsc_parser::AddBuiltinTypes( void )
{
    g_pTvoid = m_SymbolTable.AddSymbol( xwstring(L"void"), NULL, symtype_type );
    g_pTvoid->StorageSize = 0;

    g_pTs32 = m_SymbolTable.AddSymbol( xwstring(L"s32"), NULL, symtype_type );
    g_pTs32->StorageSize = 4;

    g_pTf32 = m_SymbolTable.AddSymbol( xwstring(L"f32"), NULL, symtype_type);
    g_pTf32->StorageSize = 4;

    g_pTxbool = m_SymbolTable.AddSymbol( xwstring(L"xbool"), NULL, symtype_type );
    g_pTxbool->StorageSize = 4;

    g_pTstr = m_SymbolTable.AddSymbol( xwstring(L"str"), NULL, symtype_type );
    g_pTstr->StorageSize = 4;

/* TODO: Add these types in later
    pSymbol = m_SymbolTable.AddSymbol( xwstring(L"vector3"), NULL, symtype_type );

    pSymbol = m_SymbolTable.AddSymbol( xwstring(L"radian3"), NULL, symtype_type );

    pSymbol = m_SymbolTable.AddSymbol( xwstring(L"matrix4"), NULL, symtype_type );

    pSymbol = m_SymbolTable.AddSymbol( xwstring(L"quaternion"), NULL, symtype_type );
*/
}

//==============================================================================
//  TokenIsType
//==============================================================================

xbool xsc_parser::TokenIsType( const xsc_token* pToken )
{
    xbool   IsType = FALSE;

    if( pToken->Code == T_IDENTIFIER )
    {
        xsc_symbol* pTypeSymbol = m_SymbolTable.GetSymbol( pToken->ValStr );
        if( pTypeSymbol && ((pTypeSymbol->SymbolType == symtype_type) || (pTypeSymbol->SymbolType == symtype_typedef) || (pTypeSymbol->SymbolType == symtype_class)) )
            IsType = TRUE;
    }

    return IsType;
}

//==============================================================================
//  GetTypeFromToken
//==============================================================================

xsc_symbol* xsc_parser::GetTypeFromToken( const xsc_token* pToken )
{
    xsc_symbol* pTypeSymbol = m_SymbolTable.GetSymbol( pToken->ValStr );
    if( (pTypeSymbol == NULL) || ((pTypeSymbol->SymbolType != symtype_type) && (pTypeSymbol->SymbolType != symtype_typedef) && (pTypeSymbol->SymbolType != symtype_class)) )
    {
        // Symbol not found or not of the correct type
        pTypeSymbol = NULL;
    }
    else
    {
        // Resolve typedefs
        while( pTypeSymbol->SymbolType == symtype_typedef )
            pTypeSymbol = pTypeSymbol->Type.pType;

    }
     
    return pTypeSymbol;
}

//==============================================================================
//  GetSignatureFromType
//==============================================================================

xwstring xsc_parser::GetSignatureFromType( typeref Type )
{
    xwstring Signature;

    if( Type.IsReference )
        Signature = L"&";

    if( Type.pType == g_pTvoid )
    {
        Signature += L"V";
    }
    else if( Type.pType == g_pTs32 )
    {
        Signature += L"I";
    }
    else if( Type.pType == g_pTf32 )
    {
        Signature += L"F";
    }
    else if( Type.pType == g_pTxbool )
    {
        Signature += L"B";
    }
    else if( Type.pType == g_pTstr )
    {
        Signature += L"S";
    }
    else
    {
        Signature += xwstring("L");
        Signature += Type.pType->Name;
        Signature += xwstring(",");
    }

    // Return the signature
    return Signature;
}

//==============================================================================
//  GetTypeFromSignature
//==============================================================================

typeref xsc_parser::GetTypeFromSignature( xwstring& Signature,
                                          xbool     Chomp )
{
    typeref     Type(g_pTvoid);
    xwstring    s = Signature;

    // Consume reference flag
    Type.IsReference = (s[0] == '&');
    if( Type.IsReference )
        s = s.Right( s.GetLength()-1 );

    switch( s[0] )
    {
    case 'V':
        Type.pType = g_pTvoid;  s = s.Right( s.GetLength()-1 ); break;
    case 'I':
        Type.pType = g_pTs32;   s = s.Right( s.GetLength()-1 ); break;
    case 'F':
        Type.pType = g_pTf32;   s = s.Right( s.GetLength()-1 ); break;
    case 'B':
        Type.pType = g_pTxbool; s = s.Right( s.GetLength()-1 ); break;
    case 'S':
        Type.pType = g_pTstr;   s = s.Right( s.GetLength()-1 ); break;
    case 'L':
        {
            xwstring ClassName;
            s = s.Right( s.GetLength()-1 );
            s32 Index = s.Find( ',' );
            if( Index != -1 )
            {
                ClassName = s.Left( Index );
                s = s.Right( s.GetLength() - (Index+1) );
            }
            else
            {
                s.Clear();
            }
            Type.pType = m_SymbolTable.GetSymbol( ClassName );
            ASSERT( Type.pType );
            ASSERT( Type.pType->SymbolType == symtype_class );
        }
        break;
    }

    // If chomp was specified then modify signature
    if( Chomp )
        Signature = s;

    // Return the type
    return Type;
}

//==============================================================================
//  Expect
//==============================================================================

xbool xsc_parser::Expect( s32 TokenCode, s32 ErrorCode, const xwchar* pErrorString )
{
    xbool   Success = FALSE;

    // Does the token match the expected
    if( m_t->Code != TokenCode )
    {
        // Add Error
        m_Errors.Error( ErrorCode, m_t, pErrorString );
    }
    else
    {
        // Set Success
        Success = TRUE;
    }

    // Read next token
    ReadToken();

    return Success;
}

//==============================================================================
//  Token Functions
//==============================================================================

const xsc_token* xsc_parser::ReadToken( s32 Count )
{
    ASSERT( m_Index >= 0 );
    ASSERT( Count > 0 );

    while( Count-- > 0 )
    {
        // Read token
        if( m_Index < m_EOFIndex )
            m_t = m_Tokenizer.GetToken( m_Index );
        else
            m_t = m_Tokenizer.GetToken( m_EOFIndex-1 );

        // Advance next token index
        m_Index++;
    }

    // Return token
    return m_t;
}

//==============================================================================

void xsc_parser::UnreadToken( s32 Count )
{
    ASSERT( m_Index > 0 );

    m_Index -= Count;

    if( m_Index <= 0 )
        m_t = 0;
    else if( m_Index < m_EOFIndex )
        m_t = m_Tokenizer.GetToken( m_Index );
    else
        m_t = m_Tokenizer.GetToken( m_EOFIndex-1 );

    m_Index++;
}

//==============================================================================

const xsc_token* xsc_parser::LookAhead( s32 Offset ) const
{
    s32 Index = m_Index + Offset;
    const xsc_token* t = 0;

    ASSERT( Index >= 0 );

    // Read token
    if( Index < m_EOFIndex )
        t = m_Tokenizer.GetToken( Index );
    else
        t = m_Tokenizer.GetToken( m_EOFIndex-1 );

    // Return token
    return t;
}

//==============================================================================
//  Token Classification Functions
//==============================================================================

xbool xsc_parser::IsEOF( const xsc_token* t ) const
{
    return( (t->Code == T_EOF) );
}

//==============================================================================
//  DumpTree
//==============================================================================

void xsc_parser::Dump( void ) const
{
    x_printf( "Parsed in %f milliseconds\n", m_ParseTimer.ReadMs() );
}

//==============================================================================
