//==============================================================================
//  
//  tokenizer.cpp
//
//==============================================================================

#include "x_types.hpp"
#include "x_stdio.hpp"
#include "x_memory.hpp"
#include "tokenizer.hpp"

#ifndef TARGET_PC
#define USING_STREAMING
#endif

static const s32            BUFFER_SIZE = 1024*32;

//==============================================================================
// USING STREAMING
//==============================================================================
#ifdef USING_STREAMING
//==============================================================================

char token_stream::CHAR( s32 FilePos )
{
    if( m_bBuffered )
    {
        if( (FilePos >= m_CurBufferStart) && (FilePos <= m_CurBufferEnd) )
        {
            return m_CurBuffer[ FilePos - m_CurBufferStart ];
        }
    
        // ELSE: reload a new buffer chunk
        //
        // Determine which BUFFER_SIZE'd block the filepos falls into
        s32 iChunk           = FilePos / BUFFER_SIZE;
        
        m_CurBufferStart     = iChunk * BUFFER_SIZE;

        s32 Size             = MIN( BUFFER_SIZE, m_FileSize - m_CurBufferStart );

        m_CurBufferEnd       = m_CurBufferStart + Size - 1;

        x_fseek( m_pFile, m_CurBufferStart + m_StartPosition, X_SEEK_SET );
        x_fread( m_CurBuffer, Size, 1, m_pFile );

        return m_CurBuffer[ FilePos - m_CurBufferStart ];
    }
    else
    {
        return m_FileBuffer[m_FilePos];
    }

}

//==============================================================================

xbool token_stream::OpenFile( const char* pFileName )
{
    // Clear class
    m_FileSize   = 0;
    m_FilePos    = 0;
    m_LineNumber = 1;
    m_Type = TOKEN_NONE;
    
    if( x_strlen(pFileName)>63 )
    {
        x_strcpy(m_Filename,pFileName+x_strlen(pFileName)-63);
    }
    else
    {
        x_strcpy(m_Filename,pFileName);
    }

    // Open file
    m_pFile = x_fopen( pFileName, "rb" );

    if( m_pFile == NULL )
        return FALSE;

    m_bBuffered      = TRUE;
    m_CurBufferStart = -1;
    m_CurBufferEnd   = -1;
    m_CurBuffer      = (char*)x_malloc(BUFFER_SIZE);
    ASSERT(m_CurBuffer);

    // Find how large the file is
    m_FileSize = x_flength(m_pFile);

    Rewind();

    m_StartPosition = 0;

    return( TRUE );
}

//==============================================================================

xbool token_stream::OpenFile( X_FILE *fp )
{
    // Clear class
    m_FileSize          = 0;
    m_FilePos           = 0;
    m_LineNumber        = 1;
    m_Type              = TOKEN_NONE;
    
    m_pFile = fp;

    if( m_pFile == NULL )
        return FALSE;

    m_bBuffered         = TRUE;
    m_CurBufferStart    = -1;
    m_CurBufferEnd      = -1;
    m_CurBuffer         = (char*)x_malloc(BUFFER_SIZE);
    ASSERT(m_CurBuffer);

    // Find how large the file is
    m_FileSize          = x_flength(m_pFile);
    m_StartPosition     = x_ftell(m_pFile);
    m_FileSize         -= x_ftell(m_pFile);

    Rewind();

    return( TRUE );
}

//==============================================================================
// NOT USING STREAMING
//==============================================================================
#else 
//==============================================================================

#define CHAR(s) m_FileBuffer[(s)]

//==============================================================================

xbool    token_stream::OpenFile    ( const char* pFileName )
{
    X_FILE* fp;

    // Clear class
    m_FileSize          = 0;
    m_FilePos           = 0;
    m_LineNumber        = 1;
    m_Type              = TOKEN_NONE;
    m_bBuffered         = FALSE;

    if( x_strlen(pFileName)>63 )
    {
        x_strcpy(m_Filename,pFileName+x_strlen(pFileName)-63);
    }
    else
    {
        x_strcpy(m_Filename,pFileName);
    }
    // Open file
    fp = x_fopen( pFileName, "rb" );
//    ASSERT(fp);           // The application should assert
    if( fp == NULL )
        return FALSE;

    // Find how large the file is
    m_FileSize = x_flength(fp);

    // Allocate a buffer to hold the file
    m_FileBuffer = (char*)x_malloc(m_FileSize+1);
    if( m_FileBuffer == NULL )
        return FALSE;

    // Load the entire file into the buffer
    m_FileBuffer[m_FileSize] = 0;
    if( x_fread( m_FileBuffer, 1, m_FileSize, fp ) != m_FileSize )
    {
        x_free(m_FileBuffer);
        return FALSE;
    }
    
    // Close the input file
    x_fclose( fp );

    Rewind();

    return( TRUE );
}

//==============================================================================

xbool token_stream::OpenFile( X_FILE* fp )
{

    // Clear class
    m_FileSize   = 0;
    m_FilePos    = 0;
    m_LineNumber = 1;
    m_Type = TOKEN_NONE;
    m_bBuffered = FALSE;

    if( fp == NULL )
        return FALSE;

    // Find how large the file is
    m_FileSize = x_flength(fp);

    // Avoid characters that have already been read
    m_FileSize -= x_ftell(fp);

    // Allocate a buffer to hold the file
    m_FileBuffer = (char*)x_malloc(m_FileSize+1);
    //Make sure the end of the array is touched to keep purify happy

    if( m_FileBuffer == NULL )
        return FALSE;

    // Load the entire file into the buffer
    m_FileBuffer[m_FileSize] = 0;
    //ARH I removed this line because it may not be true if the file pointer has been 
    //      partially used.
    //if( x_fread( m_FileBuffer, 1, m_FileSize, fp ) != m_FileSize )
    if( x_fread( m_FileBuffer, 1, m_FileSize, fp ) < 0 )
    {
        x_free(m_FileBuffer);
        return FALSE;
    }
    
    // Close the input file
    x_fclose( fp );

    Rewind();

    return( TRUE );
}

//==============================================================================
// END OF STREAMING / NONSTREAMING SECTIONS
//==============================================================================
#endif
//==============================================================================

token_stream::token_stream    (void)
{
    m_FileBuffer        = NULL;
    m_FileSize          = 0;
    m_LineNumber        = 1;
    m_CurBuffer         = NULL;
    m_bBuffered         = FALSE;
    m_CurBufferStart    = -1;
    m_CurBufferEnd      = -1;
    m_pFile             = NULL;

    // Fill out delimiters
    x_strcpy(m_DelimiterStr,TOKEN_DELIMITER_STR);

    // Setup number chars
    s32 i;
    char IStr[] = "0123456789-+";
    char FStr[] = "Ee.#QNABIFD";
    for( i=0; i<256; i++ )
        m_IsCharNumber[i] = 0;
    i=0;
    while( IStr[i] )
    {
        m_IsCharNumber[ IStr[i] ] = 1;
        i++;
    }
    i=0;
    while( FStr[i] )
    {
        m_IsCharNumber[ FStr[i] ] = 2;
        i++;
    }

    //x_strcpy(m_NumberStr,"0123456789-+Ee.#QNABIF");
}

//==============================================================================

token_stream::~token_stream   (void)
{
    if( m_FileBuffer )
    {
        x_free(m_FileBuffer);
        m_FileBuffer = NULL;
    }
    if( m_CurBuffer )
    {
        x_free(m_CurBuffer);
        m_CurBuffer = NULL;
    }
    m_bBuffered      = FALSE;
    m_CurBufferStart = -1;
    m_CurBufferEnd   = -1;
}

//==============================================================================

xbool token_stream::IsEOF( void ) const 
{
    return m_FilePos >= m_FileSize;
}


//==============================================================================

char* token_stream::GetDelimeter( void )
{
    return m_DelimiterStr;
}

//==============================================================================

void token_stream::SetDelimeter( char* pStr )
{
    ASSERT( x_strlen( pStr ) >= 2 );
    x_strcpy(m_DelimiterStr, pStr);
}

//==============================================================================

void    token_stream::CloseFile   ( void )
{
    if( m_bBuffered )
    {
        if( m_CurBuffer )
        {
            x_free( m_CurBuffer );
            if (m_pFile)
            {
                x_fclose(m_pFile);
            }
            m_pFile             = NULL;
            m_CurBuffer         = NULL;
            m_CurBufferStart    = -1;
            m_CurBufferEnd      = -1;
        }
    }
    else
    {
        x_free(m_FileBuffer);
        m_FileBuffer = NULL;
        m_FileSize = 0;
    }    
}

//==============================================================================

void    token_stream::Rewind      ( void )
{
    m_FilePos          = 0;
    m_LineNumber       = 1;
    m_Type             = TOKEN_NONE;
    m_Delimiter        = ' ';
    m_Float            = 0.0f;
    m_Int              = 0;
    m_String[0]        = 0;
}

//==============================================================================

void    token_stream::SetCursor      ( s32 Pos )
{
    ASSERT( (Pos>=0) && (Pos<m_FilePos) );
    m_FilePos          = Pos;
    m_LineNumber       = 1;
    m_Type             = TOKEN_NONE;
    m_Delimiter        = ' ';
    m_Float            = 0.0f;
    m_Int              = 0;
    m_String[0]        = 0;
}

//==============================================================================

s32    token_stream::GetCursor      ( void )
{
    return m_FilePos;
}

//==============================================================================

xbool   token_stream::Find        ( const char* TokenStr, xbool FromBeginning )
{
    if( FromBeginning )
        Rewind();

    Read();

    while( m_Type != TOKEN_EOF )
    {
        if( x_stricmp(m_String,TokenStr)==0 ) return TRUE;
        Read();
    }

    return FALSE;
}

//==============================================================================

void token_stream::SkipToNextLine( void )
{
    while( 1 )
    {
        if( m_FilePos >= m_FileSize )
            return;

        // Move forward to end of line
        {
            while( (CHAR(m_FilePos)!='\n') && (m_FilePos<m_FileSize))
                m_FilePos++;

            if( CHAR(m_FilePos)=='\n' ) 
            {
                m_LineNumber++;
                m_FilePos++;
                return;
            }
        }
    }
}

//==============================================================================

void token_stream::SkipWhitespace( void )
{
    while( 1 )
    {
        if( m_FilePos >= m_FileSize )
            return;

        // Skip whitespace and count EOLNs
        if( CHAR(m_FilePos)<=32 )
        {
            while( (m_FilePos<m_FileSize) && (CHAR(m_FilePos)<=32) )
            {
                if( CHAR(m_FilePos)=='\n' ) 
                {
                    m_LineNumber++;
                }
                m_FilePos++;
            }

            continue;
        }

        // Watch for line comment
        if( (CHAR(m_FilePos+0) == '/') && (CHAR(m_FilePos+1) == '/') )
        {
            m_FilePos += 2;

            while( (CHAR(m_FilePos)!='\n') && (m_FilePos<m_FileSize))
                m_FilePos++;

            if( CHAR(m_FilePos)=='\n' ) 
            {
                m_LineNumber++;
                m_FilePos++;
            }

            continue;
        }

        // Watch for block comment
        if( (CHAR(m_FilePos+0) == '/') && (CHAR(m_FilePos+1) == '*') )
        {
            m_FilePos += 2;

            while( m_FilePos <= m_FileSize-1 )
            {
                if( (CHAR(m_FilePos+0) == '*') && (CHAR(m_FilePos+1) == '/') )
                {
                    m_FilePos += 2;
                    break;
                }

                if( CHAR(m_FilePos)=='\n' ) 
                {
                    m_LineNumber++;
                }

                m_FilePos++;
            }

            continue;
        }

        // No whitespace found
        return;
    }
}

//==============================================================================

token_stream::type    token_stream::Read        ( s32 NTokens )
{
    while( NTokens-- )
    {
        s32     i,j;
        char    ch;

        // Clear the current settings
        m_Type          = TOKEN_NONE;
        m_Delimiter     = ' ';
        m_Float         = 0.0f;
        m_Int           = 0;
        m_IsFloat       = FALSE;
        m_String[0]     = 0;

        ASSERT(m_FilePos<=m_FileSize);

        // Skip whitespace and comments
        SkipWhitespace();

        // Watch for end of file
        if( m_FilePos >= m_FileSize )
        {
            m_Type = TOKEN_EOF;
            continue;
        }

        // Look for number first since we load a lot of these
        ch = CHAR(m_FilePos);
        if(((ch>='0') && (ch<='9')) || (ch=='-') || (ch=='+'))
        {
            // Copy number into string buffer
            i=0;
            m_IsFloat = FALSE;
            while( 1 )
            {
                ch = CHAR(m_FilePos);
                if( !m_IsCharNumber[ch] )
                    break;
                m_String[i] = ch;
                m_IsFloat |= m_IsCharNumber[ch];
                m_FilePos++;
                i++;
                ASSERT( i < TOKEN_STRING_SIZE-1 );
            }
            m_IsFloat >>= 1;
            m_String[i] = 0;
            
            // Generate float version
            if( m_IsFloat )
            {
                m_Float     = x_atof( m_String );
                m_Int       = (s32)m_Float;
                m_Type      = TOKEN_NUMBER;
            }
            else
            {
                m_Int       = x_atoi( m_String );
                m_Float     = (f32)m_Int;
                m_Type      = TOKEN_NUMBER;
            }
        
            continue;
        }

        // Watch for string
        if( CHAR(m_FilePos) == '"' )
        {
            m_FilePos++;        
            i=0;
            while( CHAR(m_FilePos)!='"' )
            {
                // Check for illegal ending of a string
                ASSERT((m_FilePos < m_FileSize) && "EOF in quote");
                ASSERT((i<TOKEN_STRING_SIZE-1) && "Quote too long");
                ASSERT((CHAR(m_FilePos)!='\n') && "EOLN in quote");
            
                m_String[i] = CHAR(m_FilePos);
                i++;
                m_FilePos++;
                ASSERT( i < TOKEN_STRING_SIZE-1 );
            }

            m_FilePos++;
            m_String[i]  = 0;
            m_Type       = TOKEN_STRING;

            continue;
        }

        // Look for delimiter
        ch = CHAR(m_FilePos);
        i=0;
        while( m_DelimiterStr[i] && (ch != m_DelimiterStr[i]) ) 
            i++;
        if( m_DelimiterStr[i] )
        {
            m_FilePos++;
            m_Type           = TOKEN_DELIMITER;
            m_Delimiter      = ch;
            m_String[0]      = ch;
            m_String[1]      = 0;
            continue;
        }

        // Treat this as a raw symbol
        {
            i=0;
            while( m_FilePos<m_FileSize )
            {
                j=0;
                if( i==TOKEN_STRING_SIZE-1 ) break;
                if( CHAR(m_FilePos)<=32 ) break;
                while( m_DelimiterStr[j] && (CHAR(m_FilePos) != m_DelimiterStr[j]) ) j++;
                if( m_DelimiterStr[j] ) break;

                m_String[i] = (char)CHAR(m_FilePos);
                i++;
                m_FilePos++;
            }

            m_String[i] = 0;
            m_Type      = TOKEN_SYMBOL;
            continue;
        }
    }

    return m_Type;
}

//==============================================================================

f32     token_stream::ReadFloat   ( void )
{
    SkipWhitespace();

    char ch = CHAR(m_FilePos);
    ASSERT(((ch>='0') && (ch<='9')) || (ch=='-') || (ch=='+'));

    // Copy number into string buffer
    s32 i=0;
    while( 1 )
    {
        ch = CHAR(m_FilePos);
        if( !m_IsCharNumber[ch] )
            break;
        m_String[i] = ch;
        m_FilePos++;
        i++;
        ASSERT( i < TOKEN_STRING_SIZE-1 );
    }
    
    // Generate float version
    m_String[i] = 0;
    m_Float     = x_atof( m_String );
    m_Int       = (s32)m_Float;
    m_Type      = TOKEN_NUMBER;
    m_IsFloat   = TRUE;

    return m_Float;
}

//==============================================================================

f32 token_stream::ReadF32FromString( void )
{
    ReadString();

    // Transform string into a float
    m_Float     = x_atof( m_String );
    m_Int       = (s32)m_Float;
    m_Type      = TOKEN_NUMBER;
    m_IsFloat   = TRUE;

    return m_Float;
}

//==============================================================================

s32 token_stream::ReadS32FromString( void )
{
    ReadString();

    // Transform string into a s32
    m_Int       = x_atoi( m_String );
    m_Float     = (f32)m_Int;
    m_Type      = TOKEN_NUMBER;
    m_IsFloat   = FALSE;

    return m_Int;
}

//==============================================================================

xbool token_stream::ReadBoolFromString( void )
{
    ReadString();

    // Transform string into an xbool
    m_Int       = x_atoi( m_String );
    m_Float     = (f32)m_Int;
    m_Type      = TOKEN_NUMBER;
    m_IsFloat   = FALSE;

    return (m_Int) ? (TRUE):(FALSE);
}

//==============================================================================

s32     token_stream::ReadInt     ( void )
{
    SkipWhitespace();

    char ch = CHAR(m_FilePos);
    ASSERT(((ch>='0') && (ch<='9')) || (ch=='-') || (ch=='+'));

    // Copy number into string buffer
    s32 i=0;
    while( 1 )
    {
        ch = CHAR(m_FilePos);
        if( !m_IsCharNumber[ch] )
            break;
        m_String[i] = ch;
        m_FilePos++;
        i++;
        ASSERT( i < TOKEN_STRING_SIZE-1 );
    }
    
    // Generate int version
    m_String[i] = 0;
    m_Int       = x_atoi( m_String );
    m_Float     = (f32)m_Int;
    m_Type      = TOKEN_NUMBER;
    m_IsFloat   = FALSE;

    //x_DebugMsg("Read integer: %d\n", m_Int);
    return m_Int;
}

//==============================================================================

s32     token_stream::ReadHex     ( void )
{
    SkipWhitespace();

    char ch = CHAR(m_FilePos);
    m_FilePos++;
    ASSERT( ch == '0' );
    ch = CHAR(m_FilePos);
    m_FilePos++;
    ASSERT( ch == 'x' );

    // Copy number into string buffer
    s32 Number = 0;
    s32 i=2;
    m_String[0] = '0';
    m_String[1] = 'x';
    while( 1 )
    {
        ch = x_toupper(CHAR(m_FilePos));
        if( x_ishex( ch ) )
        {
            s32 Digit = ch - '0';
            if( Digit >= 10 ) Digit -= 'A'-'0'-10;
            Number *= 16;
            Number += Digit;
            m_FilePos++;
            ASSERT( i < TOKEN_STRING_SIZE-1 );
            m_String[i++] = ch;
        }
        else
            break;
    }
    
    // Generate int version
    m_String[i] = 0;
    m_Int       = Number;
    m_Float     = (f32)m_Int;
    m_Type      = TOKEN_NUMBER;
    m_IsFloat   = FALSE;

    return m_Int;
}

//==============================================================================

char*   token_stream::ReadSymbol  ( void )
{
    s32 i,j;

    SkipWhitespace();

    i=0;
    while( m_FilePos<m_FileSize )
    {
        j=0;
        if( i==TOKEN_STRING_SIZE-1 ) break;
        if( CHAR(m_FilePos)<=32 ) break;
        while( m_DelimiterStr[j] && (CHAR(m_FilePos) != m_DelimiterStr[j]) ) j++;
        if( m_DelimiterStr[j] ) break;

        m_String[i] = (char)CHAR(m_FilePos);
        i++;
        m_FilePos++;
        ASSERT( i < TOKEN_STRING_SIZE-1 );
    }

    m_String[i] = 0;
    m_Type      = TOKEN_SYMBOL;
    return m_String;
}

//==============================================================================

char*   token_stream::ReadString  ( void )
{
    SkipWhitespace();

    ASSERT( CHAR(m_FilePos) == '"' );

    m_FilePos++;        
    s32 i=0;
    while( CHAR(m_FilePos)!='"' )
    {
        // Check for illegal ending of a string
        ASSERT((m_FilePos < m_FileSize) && "EOF in quote");
        ASSERT((i<TOKEN_STRING_SIZE-1) && "Quote too long");
        ASSERT((CHAR(m_FilePos)!='\n') && "EOLN in quote");
    
        m_String[i] = CHAR(m_FilePos);
        i++;
        m_FilePos++;
        ASSERT( i < TOKEN_STRING_SIZE-1 );
    }

    m_FilePos++;
    m_String[i]  = 0;
    m_Type       = TOKEN_STRING;
    return m_String;
}

//==============================================================================

char* token_stream::ReadLine( void )
{
    // Enf of file not reached or string buffer full?
    s32 i = 0 ;
    while((i < (TOKEN_STRING_SIZE-1)) && (m_FilePos<m_FileSize))
    {
        // Get char
        char C = CHAR(m_FilePos++) ;
       
        // End of line reached?
        if ((C == '\n') || (C == 13))
           break ;

        // Add to string
        m_String[i++] = C ;
    }

    // Terminate the string
    m_String[i] = 0;
    m_Type      = TOKEN_SYMBOL;

    // Skip end of line feeds
    while(m_FilePos<m_FileSize)
    {
        // Get char
        char C = CHAR(m_FilePos++) ;

        // End of line reached?
        if ((C != '\n') || (C != 13))
           break ;
    }

    return m_String;
}

//==============================================================================

char*   token_stream::ReadToSymbol ( char Sym )
{
    s32 i=0;
    while( CHAR(m_FilePos)!= Sym )
    {
        // Check for illegal ending of a string
        ASSERT((m_FilePos < m_FileSize) && "EOF in quote");
        ASSERT((i<TOKEN_STRING_SIZE-1) && "Quote too long");
    
        m_String[i] = CHAR(m_FilePos);
        i++;
        m_FilePos++;
    }

    m_FilePos++;
    m_String[i]  = 0;
    m_Type       = TOKEN_STRING;
    return m_String;
}

//==============================================================================

void token_stream::OpenText( const char* pTextString )
{
    // No file must be open
    ASSERT( m_pFile == NULL );
    ASSERT( pTextString );
    // Clear class
    m_FileSize   = 0;
    m_FilePos    = 0;
    m_LineNumber = 1;
    m_Type = TOKEN_NONE;

    x_strcpy( m_Filename, "<internal string>" );

    m_bBuffered     = FALSE;
    m_FileBuffer    = (char*)pTextString;

    // Find how large the file is
    m_FileSize = x_strlen(pTextString)+1;

    Rewind();

    m_StartPosition = 0;
  
}

//==============================================================================

void token_stream::CloseText( void )
{
    ASSERT( m_bBuffered == FALSE );
    ASSERT( m_pFile == NULL );
    ASSERT( m_CurBuffer == NULL );

    m_FileBuffer = NULL;
}