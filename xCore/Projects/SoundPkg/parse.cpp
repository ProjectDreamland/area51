#include <stdlib.h>
#include "x_files.hpp"
#include "parse.hpp"
#include "parse_variable.hpp"
#include "parse_complex.hpp"
//#include "../../support/audiomgr/audio.hpp"

#define FLAG_TOKENS "Looped|Surround|ChannelSaver"


//-----------------------------------------------------------------------------
void SyntaxError(token_stream &Tok,const char *pExpected)
{
    x_printf("%s(%d): Syntax error: Got '%s' when I expected [%s]\n",Tok.GetFilename(),Tok.GetLineNumber(),Tok.String(),pExpected);
    exit (-1 );
}

//-----------------------------------------------------------------------------
void SyntaxError(token_stream &Tok,const char Expected)
{
    x_printf("%s(%d): Syntax error: Got '%s' when I expected %c\n",Tok.GetFilename(),Tok.GetLineNumber(),Tok.String(),Expected);
    exit(-1);
}

//-----------------------------------------------------------------------------
void Error(token_stream &Tok,const char *pString)
{
    x_printf("%s(%d): %s\n",Tok.GetFilename(),Tok.GetLineNumber(),pString);
    exit(-1);
}

//-----------------------------------------------------------------------------
s32 ExpectToken(token_stream &Tok,const char *pTokens)
{
    char    LocalToken[128];
    char    *pLocal;
    s32     index;
    const char    *pTokenList;


    index=0;
    pTokenList=pTokens;
    if (Tok.Read() == token_stream::TOKEN_EOF)
    {
        return -1;
    }
    while (*pTokenList)
    {
        pLocal = LocalToken;

        if (*pTokenList=='|') pTokenList++;
        while ( *pTokenList )
        {
            if (*pTokenList=='|')
            {
                break;
            }
            *pLocal++=x_toupper(*pTokenList++);
        }
        *pLocal=0x0;

        if (x_stricmp(Tok.String(),LocalToken)==0)
            return index;
        index++;
    }
    SyntaxError(Tok,pTokens);
    return -1;
}

//-----------------------------------------------------------------------------
void ExpectString(token_stream &Tok,char *pString)
{
    if (Tok.Read() != token_stream::TOKEN_STRING)
    {
        SyntaxError(Tok,"Character String");
    }
    x_strcpy(pString,Tok.String());
}

//-----------------------------------------------------------------------------
s32 ExpectInteger(token_stream &Tok)
{
    if (Tok.Read() != token_stream::TOKEN_NUMBER)
    {
        SyntaxError(Tok,"Integer");
    }
    return Tok.Int();
}

//-----------------------------------------------------------------------------
f32 ExpectFloat(token_stream &Tok)
{
    if (Tok.Read() != token_stream::TOKEN_NUMBER)
    {
        SyntaxError(Tok,"Floating point number");
    }
    return Tok.Float();
}

//-----------------------------------------------------------------------------
s32 ExpectFlags(token_stream &Tok)
{
    s32     flags;
    xbool   Done;

    flags = 0;
    Done = FALSE;
    while (!Done)
    {
        switch (ExpectToken(Tok,FLAG_TOKENS))
        {
        case 0:                 // Looped
            flags |= AUDFLAG_LOOPED;
            break;
        case 1:                 // Surround
            flags |= AUDFLAG_3D_POSITION;
            break;
        case 2:                 // Channel Saver
            flags |= AUDFLAG_CHANNELSAVER;
            break;
        default:
            Error(Tok,xfs("Invalid flag definition '%s'",Tok.String()));
            break;
        }

        switch (ExpectToken(Tok,",|;"))
        {
        case 0:
            break;
        case 1:
            Done = TRUE;
            break;
        }
    }

    return flags;
}

//-----------------------------------------------------------------------------
void ParseControlFile(char *pFilename,parse_output &Output,parse_variables &Default)
{
    xbool               Status;
    token_stream        Tok;
    parse_variables     NewDefaults;        // This only gets used when we descend a level.
    xbool               ExpectSemicolon;
    xbool               Done;

    Status = Tok.OpenFile(pFilename);
    if (!Status)
    {
        x_printf("SoundPkg: Cannot open control file '%s'\n",pFilename);
        exit(-1);
    }
    
    Done = FALSE;
    while (!Done)
    {
        ExpectSemicolon = TRUE;
        switch (ExpectToken(Tok,"OutputFile|OutputLabelFile|Target|PackageId|Complex|Include|Default"))
        {
        case -1:                // End of file
            Done = TRUE;
            ExpectSemicolon = FALSE;
            break;
        case 0:
            if (Output.m_PackageFilename[0] != 0)
            {
                Error(Tok,"Duplicate OutputFile defined");
            }
            ExpectString(Tok,Output.m_PackageFilename);     
            break;
        case 1:
            if (Output.m_LabelFilename[0] != 0)
            {
                Error(Tok,"Duplicate OutputLabelFile defined");     
            }
            ExpectString(Tok,Output.m_LabelFilename);
            break;
        case 2:
            s32 target;
            target = ExpectToken(Tok,"PS2|PC|XBox|Gamecube");
            switch (target)
            {
            case 0:
                Output.m_TargetType = TARGET_TYPE_PS2;
                break;
            case 1:
                Output.m_TargetType = TARGET_TYPE_PC;
                break;
            case 2:
                Output.m_TargetType = TARGET_TYPE_XBOX;
                break;
            case 3:
                Output.m_TargetType = TARGET_TYPE_GCN;
                break;
            default:
                ASSERT(FALSE);
                break;
            }
            SetTargetPlatform(Output.m_TargetType);
            break;
        case 3:
            Output.m_PackageId = ExpectInteger(Tok);
            break;
        case 4:
            ParseComplex(Tok,Output,Default,FALSE);
            ExpectSemicolon=FALSE;
            break;
        case 5:
            char NewFilename[256];

            ExpectString(Tok,NewFilename);
            NewDefaults = Default;
            ParseControlFile(NewFilename,Output,NewDefaults);
            break;
        case 6:
            Status = ParseVariable(Tok,Default);
            ExpectSemicolon = FALSE;
            if (Status)
                Error(Tok,xfs("Syntax error - '%s'",Tok.String()));
            break;
        default:
            ASSERT(FALSE);
            break;
        }
        if (ExpectSemicolon)
        {
            ExpectToken(Tok,";");
        }
    }
    Tok.CloseFile();
}
