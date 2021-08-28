//==============================================================================
//  
//  xsc_tokenizer.hpp
//  
//==============================================================================

#ifndef XSC_TOKENIZER_HPP
#define XSC_TOKENIZER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "xsc_symbol_table.hpp"

class xsc_errors;

//==============================================================================
//  Defines
//==============================================================================

// Useful character codes
enum character_code
{
    CHAR_HT         = 0x09,
    CHAR_LF         = 0x0a,
    CHAR_VT         = 0x0b,
    CHAR_FF         = 0x0c,
    CHAR_CR         = 0x0d,
    CHAR_EOF        = 0x1a,
};

// Token codes
enum token_code
{
    T_ILLEGAL,                  // Unknown Token
    T_EOF,                      // END OF FILE

    T_EOL_COMMENT,              // End of Line Comment
    T_COMMENT,                  // Multiline Comment

    T_PREPROCESSOR,             // Preprocessor directive
    T_IDENTIFIER,               // Identifier
    T_CONST_INT,                // Constant Integer
    T_CONST_FLT,                // Constant Float
    T_CONST_STR,                // Constant String

    T_EQ_OP,                    // ==
    T_ASSIGN,                   // =

    T_NEQ_OP,                   // !=
    T_NOT_OP,                   // !

    T_ADD_ASSIGN,               // +=
    T_INC_OP,                   // ++
    T_ADD_OP,                   // +

    T_SUB_ASSIGN,               // -=
    T_DEC_OP,                   // --
    T_PTR_OP,                   // ->
    T_SUB_OP,                   // -

    T_MUL_ASSIGN,               // *=
    T_MUL_OP,                   // *

    T_DIV_ASSIGN,               // /=
    T_DIV_OP,                   // /

    T_MOD_OP,                   // %
    T_MOD_ASSIGN,               // %=

    T_SHL_ASSIGN,               // <<=
    T_SHL_OP,                   // <<
    T_LE_OP,                    // <=
    T_LT_OP,                    // <

    T_SHR_ASSIGN,               // >>=
    T_SHR_OP,                   // >>
    T_GE_OP,                    // >=
    T_GT_OP,                    // >

    T_XOR_ASSIGN,               // ^=
    T_XOR_OP,                   // ^

    T_OR_ASSIGN,                // |=
    T_LOG_OR_OP,                // ||
    T_OR_OP,                    // |

    T_AND_ASSIGN,               // &=
    T_LOG_AND_OP,               // &&
    T_AND_OP,                   // &

    T_COLON2,                   // ::
    T_COLON,                    // :

    T_DOT_OP,                   // .

    T_COMMA_OP,                 // ,

    T_CONDITIONAL_OP,           // ?

    T_COMPLEMENT_OP,            // ~

    T_SEMICOLON,                // ;

    T_LPAREN,                   // (
    T_RPAREN,                   // )
    T_LBRACE,                   // {
    T_RBRACE,                   // }
    T_LSQUARE,                  // [
    T_RSQUARE,                  // ]

    // Directives
    T_IMPORT,                   // "import"
    T_TYPEDEF,                  // "typedef"

    // Statements
    T_IF,                       // "if"
    T_ELSE,                     // "else"
    T_FOR,                      // "for"
    T_WHILE,                    // "while"
    T_ENUM,                     // "enum"
    T_CONTINUE,                 // "continue"
    T_RETURN,                   // "return"
    T_CLASS,                    // "class"
    T_STRUCT,                   // "struct"
    T_PUBLIC,                   // "public"
    T_PRIVATE,                  // "private"
    T_PROTECTED,                // "protected"
    T_CONST,                    // "const"
    T_STATIC,                   // "static"
    T_VIRTUAL,                  // "virtual"
    T_NATIVE,                   // "native"

    // Misc
    T_OPERATOR,                 // "operator"
    T_FALSE,                    // "false"
    T_TRUE,                     // "true"

    // Debug statements
    T_ASSERT,                   // "assert"
    T_BREAKPOINT,               // "breakpoint"
};

//==============================================================================
//  Token
//==============================================================================

// Token structure
struct xsc_token
{
    token_code  Code;           // Token Code, see table above
    s32         StartIndex;     // Start index in source code
    s32         Length;         // Length in source code
    s32         ValInt;         // Value Integer
    f32         ValFlt;         // Value Float
    xwstring    ValStr;         // Value String
};

//==============================================================================
//  xsc_tokenizer
//==============================================================================

class xsc_tokenizer
{
//==============================================================================
//  Switches
//==============================================================================
    enum switches
    {
        sw_emit_comments,

        num_switches
    };

//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_tokenizer           ( const xwstring& Source, xsc_errors& Errors );
                       ~xsc_tokenizer           ( void );

    // Setup
    void                Init                    ( s32 NumTokens );
    xbool               GetSwitch               ( s32 Switch ) const;
    void                SetSwitch               ( s32 Switch, xbool State );
    void                AddKeyword              ( const xwstring& Keyword, s32 TokenCode );

    // Run the tokenizer
    xbool               Tokenize                ( void );

    // Token Accessors
    s32                 GetNumTokens            ( void ) const;             // Query for the number of tokens
    const xsc_token*    GetToken                ( s32 Index ) const;        // Get a const pointer to a token

    // Debugging
    void                DumpTokens              ( void ) const;             // Dump token stream to STDOUT

private:

    // Tokenizers
    void                TokenizeConstNumeric    ( void );
    void                TokenizeConstCharacter  ( void );
    void                TokenizeConstString     ( void );
    void                TokenizeMultiLineComment( void );

    // Character Access
    s32                 ReadChar                ( void );                   // Read next character and increment pointer
    void                UnreadChar              ( void );                   // Unread the last character read
    s32                 LookAhead               ( s32 Offset = 0 ) const;   // Take a peek at succeeding characters
    void                SkipWhitespace          ( void );                   // Skip whitespace

    // Character Classification
    xbool               IsAlpha                 ( s32 c ) const;
    xbool               IsDigit                 ( s32 c ) const;
    xbool               IsAlphaNumeric          ( s32 c ) const;
    xbool               IsHex                   ( s32 c ) const;
    xbool               IsIdentifier            ( s32 c ) const;
    xbool               IsIdentifierFirst       ( s32 c ) const;
    xbool               IsWhitespace            ( s32 c ) const;
    xbool               IsEOL                   ( s32 c ) const;
    xbool               IsEOF                   ( s32 c ) const;

    // Character Conversion Helpers
    s32                 CharToOctal             ( s32 c ) const;            // Convert character to octal digit
    s32                 CharToHex               ( s32 c ) const;            // Convert character to hex digit

    // Token emission
    xsc_token&          EmitToken               ( token_code Type );        // Emit Token to array
    void                DiscardToken            ( void );                   // Discard the token just recognized

//==============================================================================
//  Data
//==============================================================================

protected:
    const xwstring&     m_Source;                   // Reference to source code
    xsc_errors&         m_Errors;                   // Reference to errors

    xbool               m_Switches[num_switches];   // Tokenizer switches

    xtimer              m_TokenizeTimer;            // Timer for tokenizing
    xarray<xsc_token>   m_Tokens;                   // Tokens scanned
    xsc_scope           m_Keywords;                 // Keywords symbol table

    s32                 m_StartLine;                // Line number at start of token
    s32                 m_Line;                     // Current line number
    s32                 m_StartIndex;               // Start character index of token
    s32                 m_Index;                    // Current character index
    s32                 m_EOFIndex;                 // End of File Index
    s32                 m_c;                        // Current character
};

//==============================================================================
#endif // XSC_TOKENIZER_HPP
//==============================================================================
