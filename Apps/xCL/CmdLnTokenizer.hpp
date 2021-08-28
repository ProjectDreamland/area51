//==============================================================================
//
//  CmdLnTokenizer.hpp
//
//==============================================================================

#ifndef CMDLNTOKENIZER_HPP
#define CMDLNTOKENIZER_HPP

//==============================================================================

#define TOKEN_BUFFER_SIZE   8192

//==============================================================================

class cmd_line_tokenizer
{
public:
            cmd_line_tokenizer  ( void );
           ~cmd_line_tokenizer  ();

    void    Activate            ( int argc, char** argv );
    void    Reset               ( void );
    char*   NextToken           ( void );
    char*   CurrentToken        ( void );
    char*   GetResponseFile     ( void );    

private:
    char*   GetFileToken        ( void );

    char*   m_pToken;

    char    m_Buffer[ TOKEN_BUFFER_SIZE ];
    int     m_BufDataSize;
    int     m_BufCursor;

    bool    m_InFile;
    FILE*   m_pRFile;
    char    m_RFileName[ _MAX_PATH ];

    int     m_Arg;
    int     m_argc;
    char**  m_argv;
};

//==============================================================================
#endif // CMDLNTOKENIZER_HPP
//==============================================================================
