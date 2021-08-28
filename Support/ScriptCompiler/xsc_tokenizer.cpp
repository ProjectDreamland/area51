//==============================================================================
//
//  xsc_tokenizer
//
//==============================================================================

#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  xsc_tokenizer
//==============================================================================

xsc_tokenizer::xsc_tokenizer( const xwstring& Source, xsc_errors& Errors ) : m_Source( Source ),
                                                                             m_Errors( Errors )
{
    // Clear all switches
    x_memset( m_Switches, 0, sizeof(m_Switches) );
}

//==============================================================================
//  ~xsc_tokenizer
//==============================================================================

xsc_tokenizer::~xsc_tokenizer( void )
{
}

//==============================================================================
//  GetSwitch
//==============================================================================

void xsc_tokenizer::Init( s32 NumTokens )
{
    // Set minimum token capacity
    m_Tokens.SetCapacity( MAX( NumTokens, m_Tokens.GetCapacity() ) );
}

//==============================================================================
//  GetSwitch
//==============================================================================

xbool xsc_tokenizer::GetSwitch( s32 Switch ) const
{
    ASSERT( (Switch >= 0) && (Switch < num_switches) );

    return m_Switches[Switch];
}

//==============================================================================
//  SetSwitch
//==============================================================================

void xsc_tokenizer::SetSwitch( s32 Switch, xbool State )
{
    ASSERT( (Switch >= 0) && (Switch < num_switches) );

    m_Switches[Switch] = State;
}

//==============================================================================
//  AddKeyword
//==============================================================================

void xsc_tokenizer::AddKeyword( const xwstring& Keyword, s32 TokenCode )
{
    xsc_symbol* pSymbol = m_Keywords.AddSymbol( Keyword, NULL, symtype_keyword );
    ASSERT( pSymbol );
    pSymbol->enumValue = TokenCode;
}

//==============================================================================
//  Tokenize
//==============================================================================

xbool xsc_tokenizer::Tokenize( void )
{
    xbool   Success = TRUE;

    // Reset timer
    m_TokenizeTimer.Start();

    // Create keyword table
    AddKeyword( xwstring(L"import"),     T_IMPORT );
    AddKeyword( xwstring(L"typedef"),    T_TYPEDEF );

    AddKeyword( xwstring(L"if"),         T_IF );
    AddKeyword( xwstring(L"else"),       T_ELSE );
    AddKeyword( xwstring(L"for"),        T_FOR );
    AddKeyword( xwstring(L"while"),      T_WHILE );
    AddKeyword( xwstring(L"enum"),       T_ENUM );
    AddKeyword( xwstring(L"continue"),   T_CONTINUE );
    AddKeyword( xwstring(L"return"),     T_RETURN );

    AddKeyword( xwstring(L"class"),      T_CLASS );
    AddKeyword( xwstring(L"struct"),     T_STRUCT );
    AddKeyword( xwstring(L"public"),     T_PUBLIC );
    AddKeyword( xwstring(L"private"),    T_PRIVATE );
    AddKeyword( xwstring(L"protected"),  T_PROTECTED );
    AddKeyword( xwstring(L"const"),      T_CONST );
    AddKeyword( xwstring(L"static"),     T_STATIC );
    AddKeyword( xwstring(L"virtual"),    T_VIRTUAL );
    AddKeyword( xwstring(L"native"),     T_NATIVE );

    AddKeyword( xwstring(L"operator"),   T_OPERATOR );
    AddKeyword( xwstring(L"false"),      T_FALSE );
    AddKeyword( xwstring(L"true"),       T_TRUE );

    AddKeyword( xwstring(L"assert"),     T_ASSERT );
    AddKeyword( xwstring(L"breakpoint"), T_BREAKPOINT );

    // Setup character indices and read first character
    m_EOFIndex      = m_Source.GetLength();
    m_Index         = 0;
    m_Line          = 1;
    ReadChar();
    SkipWhitespace();
    m_StartIndex    = m_Index-1;
    m_StartLine     = m_Line;

    // Loop until the file is exhausted
    while( !IsEOF(m_c) )
    {
        if( m_c == '#' )
        {
            // Preprocessor directive
            while( IsAlpha(ReadChar()) );
            // TODO: Identify preprocessor directive
            EmitToken( T_PREPROCESSOR );
        }
        else if( IsIdentifierFirst( m_c ) )
        {
            // Read identifier or keyword
            while( IsIdentifier(ReadChar()) );

            // Check for keyword
            xwstring    Identifier  = m_Source.Mid( m_StartIndex, m_Index - m_StartIndex - 1 );
            xsc_symbol* pSymbol     = m_Keywords.GetSymbol( Identifier );
            if( pSymbol )
            {
                // Emit KEYWORD token
                xsc_token&  Token = EmitToken( (token_code)pSymbol->enumValue );
            }
            else
            {
                // Emit IDENTIFIER token
                xsc_token&  Token = EmitToken( T_IDENTIFIER );
                Token.ValStr = Identifier;
            }
        }
        else if( IsDigit( m_c ) || ((m_c == '.') && IsDigit(LookAhead())) )
        {
            // Constant Numeric
            TokenizeConstNumeric( );
        }
        else if( m_c == '\'' )
        {
            // Constant Character
            TokenizeConstCharacter( );
        }
        else if( m_c == '"' )
        {
            // Constant String
            TokenizeConstString( );
        }
        else if( m_c == '=' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_EQ_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_ASSIGN );
            }
        }
        else if( m_c == '!' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_NEQ_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_NOT_OP );
            }
        }
        else if( m_c == '+' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_ADD_ASSIGN );
            }
            else if( LookAhead() == '+' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_INC_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_ADD_OP );
            }
        }
        else if( m_c == '-' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_SUB_ASSIGN );
            }
            else if( LookAhead() == '-' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_DEC_OP );
            }
            else if( LookAhead() == '>' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_PTR_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_SUB_OP );
            }
        }
        else if( m_c == '*' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_MUL_ASSIGN );
            }
            else
            {
                ReadChar();
                EmitToken( T_MUL_OP );
            }
        }
        else if( m_c == '/' )
        {
            if( LookAhead() == '/' )
            {
                // Skip EOL comment
                while( !IsEOL(m_c) )
                    ReadChar();
                if( m_Switches[sw_emit_comments] )
                    EmitToken( T_EOL_COMMENT );
                else
                    DiscardToken();
            }
            else if( LookAhead() == '*' )
            {
                ReadChar();
                ReadChar();
                TokenizeMultiLineComment();
            }
            else if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_DIV_ASSIGN );
            }
            else
            {
                ReadChar();
                EmitToken( T_DIV_OP );
            }
        }
        else if( m_c == '%' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_MOD_ASSIGN );
            }
            else
            {
                ReadChar();
                EmitToken( T_MOD_OP );
            }
        }
        else if( m_c == '<' )
        {
            if( (LookAhead() == '<') )
            {
                ReadChar();
                if( LookAhead() == '=' )
                {
                    ReadChar();
                    ReadChar();
                    EmitToken( T_SHL_ASSIGN );
                }
                else
                {
                    ReadChar();
                    EmitToken( T_SHL_OP );
                }
            }
            else if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_LE_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_LT_OP );
            }
        }
        else if( m_c == '>' )
        {
            if( (LookAhead() == '>') )
            {
                ReadChar();
                if( LookAhead() == '=' )
                {
                    ReadChar();
                    ReadChar();
                    EmitToken( T_SHR_ASSIGN );
                }
                else
                {
                    ReadChar();
                    EmitToken( T_SHR_OP );
                }
            }
            else if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_GE_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_GT_OP );
            }
        }
        else if( m_c == '^' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_XOR_ASSIGN );
            }
            else
            {
                ReadChar();
                EmitToken( T_XOR_OP );
            }
        }
        else if( m_c == '|' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_OR_ASSIGN );
            }
            else if( LookAhead() == '|' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_LOG_OR_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_OR_OP );
            }
        }
        else if( m_c == '&' )
        {
            if( LookAhead() == '=' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_AND_ASSIGN );
            }
            else if( LookAhead() == '&' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_LOG_AND_OP );
            }
            else
            {
                ReadChar();
                EmitToken( T_AND_OP );
            }
        }
        else if( m_c == ':' )
        {
            if( LookAhead() == ':' )
            {
                ReadChar();
                ReadChar();
                EmitToken( T_COLON2 );
            }
            else
            {
                ReadChar();
                EmitToken( T_COLON );
            }
        }
        else if( (m_c == '.') && (!IsDigit(LookAhead())) )
        {
            ReadChar();
            EmitToken( T_DOT_OP );
        }
        else if( m_c == ',' )
        {
            ReadChar();
            EmitToken( T_COMMA_OP );
        }
        else if( m_c == '?' )
        {
            ReadChar();
            EmitToken( T_CONDITIONAL_OP );
        }
        else if( m_c == '~' )
        {
            ReadChar();
            EmitToken( T_COMPLEMENT_OP );
        }
        else if( m_c == ';' )
        {
            ReadChar();
            EmitToken( T_SEMICOLON );
        }
        else if( m_c == '(' )
        {
            ReadChar();
            EmitToken( T_LPAREN );
        }
        else if( m_c == ')' )
        {
            ReadChar();
            EmitToken( T_RPAREN );
        }
        else if( m_c == '{' )
        {
            ReadChar();
            EmitToken( T_LBRACE );
        }
        else if( m_c == '}' )
        {
            ReadChar();
            EmitToken( T_RBRACE );
        }
        else if( m_c == '[' )
        {
            ReadChar();
            EmitToken( T_LSQUARE );
        }
        else if( m_c == ']' )
        {
            ReadChar();
            EmitToken( T_RSQUARE );
        }
        else
        {
            // Unrecognized Token
            ReadChar();
            EmitToken( T_ILLEGAL );
        }
    }

    // Emit end of File Token
    EmitToken( T_EOF );

    // Record time taken to tokenize
    m_TokenizeTimer.Stop();

    // Return success code
    return Success;
}

//==============================================================================

void xsc_tokenizer::TokenizeConstNumeric( void )
{
    token_code  Code        = T_CONST_INT;
    s32         Base        = 10;
    s32         ValInt      = 0;
    f32         ValFlt      = 0.0f;

    // Check for HEX
    if( (m_c == '0') && ((LookAhead() == 'x') || (LookAhead() == 'X')) )
    {
        ReadChar();
        ReadChar();
        Base = 16;
    }

    // Read Integer Part & Copy to Float
    while( IsDigit(m_c) || (IsHex(m_c) && (Base == 16)) )
    {
        ValInt *= Base;
        ValInt += CharToHex(m_c);
        ReadChar();
    }
    ValFlt = (f32)ValInt;

    // Process Any Fractional Part of Number
    if( (m_c == '.') && (Base == 10) )
    {
        f32 Divisor = 1;

        // Read Fractional Part
        Code = T_CONST_FLT;
        ReadChar();
        while( IsDigit(m_c) )
        {
            Divisor *= 10;
            ValFlt += (f32)(m_c - '0') / Divisor;
            ReadChar();
        }
    }

    // Process Exponent Part of Number
    if( ((m_c == 'e') || (m_c == 'E')) && (Base == 10) )
    {
        // Emit an error
        m_Errors.Error( err_syntax, m_Index, "Exponent notation not supported" );

        // Error recovery
        ReadChar();
        if( m_c == '-' )
            ReadChar();
        while( IsDigit(m_c) )
            ReadChar();
    }

    // Process suffix
    if( ((Code == T_CONST_FLT) && ((m_c == 'f') || (m_c == 'F'))) ||
        ((m_c == 'l') || (m_c == 'L')) )
    {
        ReadChar();
    }

    // Emit Token
    xsc_token& Token = EmitToken( Code );
    Token.Code   = Code;
    Token.ValInt = ValInt;
    Token.ValFlt = ValFlt;
}

//==============================================================================

void xsc_tokenizer::TokenizeConstCharacter( void )
{
    s32 Value   = 0;
    s32 ChValue;

    // Emit Token
    xsc_token& Token = EmitToken( T_CONST_INT );

    ReadChar();
    while( (m_c != '\'') && !IsEOL(m_c) )
    {
        // Process Escape codes
        if( m_c == '\\' )
        {
            ReadChar();

            if     ( m_c == 'a'  ) { ChValue = '\a'; }
            else if( m_c == 'b'  ) { ChValue = '\b'; }
            else if( m_c == 'f'  ) { ChValue = '\f'; }
            else if( m_c == 'n'  ) { ChValue = '\n'; }
            else if( m_c == 'r'  ) { ChValue = '\r'; }
            else if( m_c == 't'  ) { ChValue = '\t'; }
            else if( m_c == 'v'  ) { ChValue = '\v'; }
            else if( m_c == '\'' ) { ChValue = '\''; }
            else if( m_c == '\"' ) { ChValue = '\"'; }
            else if( m_c == '\\' ) { ChValue = '\\'; }
            else if( m_c == '?'  ) { ChValue = '\?'; }

            else if( IsDigit(m_c) )
            {
                ChValue = 0;
                while( IsDigit(m_c) )
                {
                    ChValue <<= 3;
                    ChValue += CharToOctal( m_c );
                    ReadChar();
                }
                UnreadChar();
            }

            else if( m_c == 'x' )
            {
                ChValue = 0;
                while( IsHex(m_c) )
                {
                    ChValue <<= 4;
                    ChValue += CharToHex( m_c );
                    ReadChar();
                }
                UnreadChar();
            }
        }
        else
        {
            // Just a regular character
            ChValue = m_c;
        }

        // Output an error for character constants out of range 0->255
        if( (ChValue < 0) || (ChValue > 255) )
            m_Errors.Error( err_syntax, &Token, "Character constant out of range 0-255" );

        // Sum character into value
        Value <<= 8;
        Value |= ChValue;

        // Read next character
        ReadChar();
    }

    // Check for unterminated character constant
    if( m_c != '\'' )
    {
        m_Errors.Error( err_syntax, &Token, "Character constant not terminated" );
    }
    else
    {
        ReadChar();
    }

    // Configure Token
    Token.ValInt = Value;
}

//==============================================================================

void xsc_tokenizer::TokenizeConstString( void )
{
    xwstring    String;
    s32         ChValue;

    ReadChar();
    while( (m_c != '\"') && !IsEOL(m_c) )
    {
        // Process Escape codes
        if( m_c == '\\' )
        {
            ReadChar();

            if     ( m_c == 'a'  ) { ChValue = '\a'; }
            else if( m_c == 'b'  ) { ChValue = '\b'; }
            else if( m_c == 'f'  ) { ChValue = '\f'; }
            else if( m_c == 'n'  ) { ChValue = '\n'; }
            else if( m_c == 'r'  ) { ChValue = '\r'; }
            else if( m_c == 't'  ) { ChValue = '\t'; }
            else if( m_c == 'v'  ) { ChValue = '\v'; }
            else if( m_c == '\'' ) { ChValue = '\''; }
            else if( m_c == '\"' ) { ChValue = '\"'; }
            else if( m_c == '\\' ) { ChValue = '\\'; }
            else if( m_c == '?'  ) { ChValue = '\?'; }

            else if( IsDigit(m_c) )
            {
                ChValue = 0;
                while( IsDigit(m_c) )
                {
                    ChValue <<= 3;
                    ChValue += CharToOctal( m_c );
                    ReadChar();
                }
                UnreadChar();
            }

            else if( m_c == 'x' )
            {
                ChValue = 0;
                while( IsHex(m_c) )
                {
                    ChValue <<= 4;
                    ChValue += CharToHex( m_c );
                    ReadChar();
                }
                UnreadChar();
            }
        }
        else
        {
            // Just a regular character
            ChValue = m_c;
        }

        // Output an error for character constants out of range 0->255
        if( (ChValue < 0) || (ChValue > 65535) )
            m_Errors.Error( err_syntax, m_StartIndex, "String constant out of range 0-255" );

        // Add character to string
        String += ChValue;

        // Read next character
        ReadChar();
    }

    // Check for unterminated string constant
    if( m_c != '\"' )
    {
        m_Errors.Error( err_syntax, m_StartIndex, "String constant not terminated" );
    }
    else
    {
        ReadChar();
    }

    // Check if previous token was also a CONST_STR and merge if so
    if( m_Tokens[m_Tokens.GetCount()-1].Code == T_CONST_STR )
    {
        // Append String
        xsc_token& Token = m_Tokens[m_Tokens.GetCount()-1];
        Token.ValStr += String;
        Token.Length  = (m_Index-1) - Token.StartIndex;
    }
    else
    {
        // Emit new constant string token
        xsc_token& Token = EmitToken( T_CONST_STR );
        Token.ValStr = String;
    }
}

//==============================================================================


void xsc_tokenizer::TokenizeMultiLineComment( void )
{
    s32 LastChar = m_c;
    ReadChar();

    // Loop until */ to end comment
    while( !((m_c == '*') && (LookAhead() == '/')) && !IsEOF(m_c) )
    {
        // Check for /* comments within /* comments
        if( (m_c == '*') && (LastChar == '/') )
            m_Errors.Error( err_syntax, m_StartIndex, "Nested comment" );

        // Advance keeping track of last character
        LastChar = m_c;
        ReadChar();
    }

    // Read / in */ and next character
    ReadChar();
    ReadChar();

    // Emit Token
    if( m_Switches[sw_emit_comments] )
        EmitToken( T_COMMENT );
    else
        DiscardToken();
}

//==============================================================================
//  Character Functions
//==============================================================================

s32 xsc_tokenizer::ReadChar( void )
{
    ASSERT( m_Index >= 0 );

    // Read character
    if( m_Index < m_EOFIndex )
        m_c = m_Source[m_Index];
    else
        m_c = CHAR_EOF;

    // Advance line counter
    if( m_c == CHAR_LF )
        m_Line++;

    // Advance next character index
    m_Index++;

    // Return character
    return m_c;
}

//==============================================================================

void xsc_tokenizer::UnreadChar( void )
{
    ASSERT( m_Index > 1 );

    m_Index--;

    // Back up a line if necessary
    if( m_c == CHAR_LF )
        m_Line--;

    if( m_Index < m_EOFIndex )
        m_c = m_Source[m_Index-1];
    else
        m_c = CHAR_EOF;
}

//==============================================================================

s32 xsc_tokenizer::LookAhead( s32 Offset ) const
{
    s32 Index = m_Index + Offset;
    s32 c = CHAR_EOF;

    // Clamp Index
    if( Index < m_StartIndex ) Index = m_StartIndex;

    // Read character
    if( Index < m_EOFIndex )
        c = m_Source[Index];

    // Return character
    return c;
}

//==============================================================================

void xsc_tokenizer::SkipWhitespace( void )
{
    // Skip over whitespace characters
    if( !IsEOF(m_c) && IsWhitespace(m_c) )
    {
        while( !IsEOF(m_c) && IsWhitespace(m_c) )
            ReadChar();
    }
}

//==============================================================================
//  Character Classification Functions
//==============================================================================

xbool xsc_tokenizer::IsAlpha( s32 c ) const
{
    return( ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <='z')) );
}

xbool xsc_tokenizer::IsDigit( s32 c ) const
{
    return( (c >= '0') && (c <= '9') );
}

xbool xsc_tokenizer::IsAlphaNumeric( s32 c ) const
{
    return( IsAlpha(c) || IsDigit(c) );
}

xbool xsc_tokenizer::IsHex( s32 c ) const
{
    return( ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'z')) || IsDigit(c) );
}

xbool xsc_tokenizer::IsIdentifier( s32 c ) const
{
    return( IsAlpha(c) || IsDigit(c) || (c == '_') );
}

xbool xsc_tokenizer::IsIdentifierFirst( s32 c ) const
{
    return( IsAlpha(c) || (c == '_') );
}

xbool xsc_tokenizer::IsWhitespace( s32 c ) const
{
    return( (c == ' ') || ((c >= CHAR_HT) && (c <= CHAR_CR)) );
}

xbool xsc_tokenizer::IsEOL( s32 c ) const
{
    return( (c == CHAR_LF) || (c == CHAR_EOF) );
}

xbool xsc_tokenizer::IsEOF( s32 c ) const
{
    return( (c == CHAR_EOF) );
}

//==============================================================================
//  Character Conversion Helpers
//==============================================================================

s32 xsc_tokenizer::CharToOctal( s32 c ) const
{
    s32 Value;
    Value = c - '0';
    ASSERT( (c >= 0) && (c <= 7) );
    return Value;
}

s32 xsc_tokenizer::CharToHex( s32 c ) const
{
    s32 Value;
    
    ASSERT( IsHex(c) );

    if( IsDigit(c) )
        Value = c - '0';
    else if( (c >= 'A') && (c <= 'F') )
        Value = c - 'A' + 10;
    else if( (c >= 'a') && (c <= 'z') )
        Value = c - 'a' + 10;

    return Value;
}

//==============================================================================
//  Token Emission
//==============================================================================

xsc_token& xsc_tokenizer::EmitToken( token_code Code )
{
    // Record Token
    xsc_token& Token = m_Tokens.Append();
    Token.Code       = Code;
    Token.StartIndex = m_StartIndex;
    Token.Length     = (m_Index-1) - m_StartIndex;

    // Skip any whitespace
    SkipWhitespace();

    // Setup for next token
    m_StartLine  = m_Line;
    m_StartIndex = m_Index-1;

    // Return Token
    return Token;
}

//==============================================================================

void xsc_tokenizer::DiscardToken( void )
{
    // Skip any whitespace
    SkipWhitespace();

    // Setup for next token
    m_StartLine  = m_Line;
    m_StartIndex = m_Index-1;
}

//==============================================================================
//  Token Accessors
//==============================================================================

s32 xsc_tokenizer::GetNumTokens( void ) const
{
    return m_Tokens.GetCount();
}

const xsc_token* xsc_tokenizer::GetToken( s32 Index ) const
{
    ASSERT( (Index >= 0) && (Index < m_Tokens.GetCount()) );

    return &m_Tokens[Index];
}

//==============================================================================
//  DumpTokens
//==============================================================================

void xsc_tokenizer::DumpTokens( void ) const
{
    s32 i;
    s32 j;

    for( i=0 ; i<m_Tokens.GetCount() ; i++ )
    {
        x_printf( "%2d - ", m_Tokens[i].Code );
        for( j=0 ; j<m_Tokens[i].Length ; j++ )
            x_printf( "%c", m_Source[m_Tokens[i].StartIndex+j] );
        x_printf( "\n" );
    }

    x_printf( "\n" );
    x_printf( "%d Lines\n", m_Line );
    x_printf( "%d Tokens in %f milliseconds\n", m_Tokens.GetCount(), m_TokenizeTimer.ReadMs() );
}

//==============================================================================
