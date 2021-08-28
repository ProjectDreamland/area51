//==============================================================================
//  
//  tokenizer.hpp
//
//==============================================================================

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

//==============================================================================

#include "x_math.hpp"

#include "x_types.hpp"
#include "x_stdio.hpp"
//==============================================================================
#define TOKEN_STRING_SIZE 1024
#define TOKEN_DELIMITER_STR ",{}()<>;"

class token_stream
{   
public:

    enum type
    {
        TOKEN_NONE,
        TOKEN_NUMBER,
        TOKEN_DELIMITER,
        TOKEN_SYMBOL,
        TOKEN_STRING,
        TOKEN_EOF = 0x7FFFFFFF,
    };

public:

                        token_stream        ( void );
                       ~token_stream        ( void );

    // Start
    xbool               OpenFile            ( const char* pFileName );
    xbool               OpenFile            ( X_FILE* fp );
    void                OpenText            ( const char* pText );
    void                CloseText           ( void );
    xbool               IsEOF               ( void ) const;

    void                CloseFile           ( void );
    char*               GetDelimeter        ( void );
    void                SetDelimeter        ( char* pStr );

    // Move through tokens in file
    void                Rewind              ( void );
    type                Read                ( s32 NTokens=1 );
    f32                 ReadFloat           ( void );
    s32                 ReadInt             ( void );
    s32                 ReadHex             ( void );
    char*               ReadSymbol          ( void );
    char*               ReadString          ( void );
    char*               ReadLine            ( void );
    char*               ReadToSymbol        ( char Sym );
    f32                 ReadF32FromString   ( void );
    s32                 ReadS32FromString   ( void );
    xbool               ReadBoolFromString  ( void );

    void                SkipToNextLine      ( void );

    // Complex requests
    xbool               Find                ( const char* TokenStr, xbool FromBeginning=FALSE );
    s32                 GetCursor           ( void );
    void                SetCursor           ( s32 Pos );
    s32                 GetLineNumber       ( void )                                        { return m_LineNumber;      }
    char*               GetFilename         ( void )                                        { return m_Filename;        }

    // Interrogate about current token
    type                Type                ( void )                                        { return m_Type;            }
    f32                 Float               ( void )                                        { return m_Float;           }
    s32                 Int                 ( void )                                        { return m_Int;             }
    char                Delimiter           ( void )                                        { return m_Delimiter;       }
    char*               String              ( void )                                        { return m_String;          }
    xbool               IsFloat             ( void )                                        { return m_IsFloat;         }

protected:

    void                SkipWhitespace      ( void );
    char                CHAR                ( s32 FilePos );

protected:

    s32                 m_FileSize;
    char*               m_FileBuffer;
    s32                 m_FilePos;
    s32                 m_LineNumber;

    char                m_Filename[64];
    char                m_DelimiterStr[16];
    byte                m_IsCharNumber[256];

    type                m_Type;
    char                m_String[TOKEN_STRING_SIZE];
    f32                 m_Float;
    s32                 m_Int;
    xbool               m_IsFloat;
    char                m_Delimiter;

    xbool               m_bBuffered;
    s32                 m_CurBufferStart;
    s32                 m_CurBufferEnd;
    char*               m_CurBuffer;
    s32                 m_StartPosition;
    X_FILE*             m_pFile;

};

//==============================================================================
#endif
