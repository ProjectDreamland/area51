#ifndef __PARSE_HPP
#define __PARSE_HPP

#include "../../support/tokenizer/tokenizer.hpp"
#include "parse_variable.hpp"
#include "iopaud_container.h"
#include "main.hpp"

struct sample_body
{
    sample_body *m_pNext;
    char        m_Filename[256];
    s32         m_Length;
    s32         m_SampleRate;
    s32         m_Flags;
    s32         m_MediaLocation;
    s32         m_Type;
    byte        *m_pData;
    byte        *m_pAlternate;          // Only used when it's a stereo sample
    sample_body(void)
    {
        m_Filename[0]   = 0x0;
        m_pNext         = NULL;
        m_Length        = 0;
        m_SampleRate    = 0;
        m_Flags         = 0;
        m_pAlternate    = NULL;
        m_pData         = NULL;
        m_Type          = 0;
    }
};

struct complex_effect
{
    complex_effect  *m_pNext;
    complex_effect  *m_pChildren;
    char            m_Label[256];
    char            m_Comment[256];
    s32             m_Id;               // Cfx ID
    s32             m_Owner;            // Cfx id of owner
    s32             m_Type;             // Type of cfx, COMPLEX,ELEMENT,ELEMENT_STREAMED,ELEMENT_HYBRID
    parse_variables m_Vars;             // Variables that can be set within the scripts
    sample_body     *m_pSample;         // Ptr to sample associated with this effect
    s32             m_Count;            // Number of attach cfx elements (children)
    xbool           m_Hidden;           // If set, we do not export to header text file

    complex_effect(parse_variables &Default)
    {
        m_Label[0]      = 0x0;
        m_Id            = -1;
        m_Count         = 0;
        m_Hidden        = FALSE;
        m_pSample       = NULL;
        m_pNext         = NULL;
        m_pChildren     = NULL;
        m_Vars          = Default;
    }
};

struct parse_output
{
    char            m_PackageFilename[256];
    char            m_LabelFilename[256];
    target_type     m_TargetType;
    s32             m_PackageId;
    s32             m_CfxCount;
    complex_effect   *m_pEffectHead;
    complex_effect   *m_pEffectTail;
    s32             m_SampleOffset;
    sample_body     *m_pSamples;

    parse_output ( void )
    {
        m_PackageFilename[0]=0x0;
        m_LabelFilename[0]=0x0;
        m_TargetType = TARGET_TYPE_UNDEFINED;
        m_PackageId = -1;
        m_pEffectHead = NULL;
        m_pEffectTail = NULL;
        m_CfxCount = 0;
        m_pSamples = NULL;
        m_SampleOffset = 0;
    };
};

void    ParseControlFile(char *pFilename,parse_output &Output,parse_variables &Defaults);

void    SyntaxError(token_stream &Tok,const char *pExpected);
void    SyntaxError(token_stream &Tok,const char Expected);
void    Error(token_stream &Tok,const char *pString);

s32     ExpectToken(token_stream &Tok,const char *pTokens);
void    ExpectString(token_stream &Tok,char *pString);
s32     ExpectInteger(token_stream &Tok);
f32     ExpectFloat(token_stream &Tok);
s32     ExpectFlags(token_stream &Tok);
void    ExpectSemicolon(void);

#endif // __PARSE_HPP