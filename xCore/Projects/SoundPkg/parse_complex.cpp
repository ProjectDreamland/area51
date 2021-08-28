#include "parse_complex.hpp"
#include "parse_variable.hpp"
#include "x_files.hpp"
#include "decode.hpp"
#include "encode.hpp"
//#include "../../support/audiomgr/audio.hpp"

sample_body *FindSample(token_stream &Tok,char *pFilename,parse_output &Output,xbool ForceLooped,s32 type);

extern xbool verbose_flag;

void AddChild(complex_effect *pParent,complex_effect *pChild)
{
    complex_effect *pPrev,*pCurr;

    pPrev = NULL;
    pCurr = pParent->m_pChildren;
    while (pCurr)
    {
        pPrev = pCurr;
        pCurr = pCurr->m_pNext;
    }
    if (pPrev)
    {
        pPrev->m_pNext = pChild;
    }
    else
    {
        pParent->m_pChildren = pChild;
    }
}
//-----------------------------------------------------------------------------
void ParseComplex(token_stream &Tok,parse_output &Output,parse_variables &Default,xbool SkipLabel)
{
    xbool   Done;
    xbool   ExpectSemicolon;
    parse_variables LocalDefault;

    complex_effect *pEffect,*pChild;

    pEffect = new complex_effect(Default);

    pEffect->m_Type = CFXTYPE_COMPLEX;
    pEffect->m_Id = Output.m_CfxCount++;
    pEffect->m_Owner = 0;

    if (!SkipLabel)
    {
        ExpectString(Tok,pEffect->m_Label);
        x_strcpy(pEffect->m_Comment,pEffect->m_Label);
    }

    // Top level priority & weight are the only defaults propigated to
    // the offspring of the parent level sound.
    LocalDefault.m_Volume       = 1.0f;
    LocalDefault.m_Delay        = 0.0f;
    LocalDefault.m_LoopCount    = 0;
    LocalDefault.m_Priority     = Default.m_Priority;
    LocalDefault.m_Weight       = Default.m_Weight;
    LocalDefault.m_Pitch        = 1.0f;
    LocalDefault.m_Falloff      = 1.0f;

    switch (ExpectToken(Tok,"Hidden|{"))
    {
    case 0:         // Hidden modifier
        pEffect->m_Hidden = TRUE;
        ExpectToken(Tok,"{");
        break;
    case 1:         // Open brace
        break;
    };

    Done = FALSE;
    while (!Done)
    {
        ExpectSemicolon = TRUE;

        switch (ExpectToken(Tok,"SoundFile|Volume|Pitch|Loop|Priority|Flags|Complex|Hybrid|Streamed|Comment|}|Falloff"))
        {
        case 0:         // SoundFile
            
            pChild = ParseComplexSimple(Tok,Output,CFXTYPE_ELEMENT,pEffect->m_Id,LocalDefault);
            AddChild(pEffect,pChild);
            ExpectSemicolon = FALSE;
            pEffect->m_Count++;

            break;
        case 1:         // Volume
            pEffect->m_Vars.m_Volume = ExpectFloat(Tok);
            break;
        case 2:         // Pitch
            pEffect->m_Vars.m_Pitch = ExpectFloat(Tok);    
            break;
        case 3:         // Loop
            pEffect->m_Vars.m_LoopCount = ExpectInteger(Tok);
            if (pEffect->m_Vars.m_LoopCount > 127)
            {
                Error(Tok,"Loop count must be -1, or 0..127");
            }
            ParseComplex(Tok,Output,Default,TRUE);
            ExpectSemicolon=FALSE;
            break;
        case 4:         // Priority
            pEffect->m_Vars.m_Priority = ExpectInteger(Tok);
            if (pEffect->m_Vars.m_Priority > 127)
            {
                Error(Tok,"Priority must be -1, or 0..127");
            }
            break;
        case 5:         // Flags
            pEffect->m_Vars.m_Flags = ExpectFlags(Tok);
            ExpectSemicolon = FALSE;
            break;
        case 6:         // Complex
            pChild = ParseComplexComplex(Tok,Output,pEffect->m_Id,LocalDefault);
            AddChild(pEffect,pChild);
            pEffect->m_Count++;
            ExpectSemicolon = FALSE;
            break;
        case 7:         // Hybrid
            ExpectToken(Tok,"SoundFile");
            pChild = ParseComplexSimple(Tok,Output,CFXTYPE_ELEMENT_HYBRID,pEffect->m_Id,LocalDefault);
            pChild->m_Type = CFXTYPE_ELEMENT_HYBRID;
            AddChild(pEffect,pChild);
            ExpectSemicolon = FALSE;
            pEffect->m_Count++;
            break;

        case 8:         // Streamed
            ExpectToken(Tok,"SoundFile");
            pChild = ParseComplexSimple(Tok,Output,CFXTYPE_ELEMENT_STREAM,pEffect->m_Id,LocalDefault);
            AddChild(pEffect,pChild);
            ExpectSemicolon = FALSE;
            pEffect->m_Count++;
            break;
        case 9:         // Comment
            ExpectString(Tok,pEffect->m_Comment);
            break;
        case 10:         // '}'
            Done = TRUE;
            ExpectSemicolon=FALSE;
            break;
        case 11:        // Falloff
            pEffect->m_Vars.m_Falloff = ExpectFloat(Tok);
            if ( (pEffect->m_Vars.m_Falloff < 0.01) || (pEffect->m_Vars.m_Falloff > 2.0f) )
            {
                Error(Tok,"Falloff range must be between 0.01 and 2.0");
            }
            break;
        default:
            Error(Tok,xfs("Syntax error - '%s'",Tok.String()));
            break;
        }

        if (ExpectSemicolon)
        {
            ExpectToken(Tok,";");
        }
    }

    //
    // Scan through list to see if we have a duplicate
    //
    {
        complex_effect *pList;
        
        pList = Output.m_pEffectHead;
        while (pList)
        {
            if ( x_strcmp(pList->m_Label,pEffect->m_Label)==0)
            {
                Error(Tok,xfs("Duplicate label definition '%s'",pEffect->m_Label));
            }
            pList = pList->m_pNext;
        }
    }

    if (!Output.m_pEffectHead)
    {
        Output.m_pEffectHead = pEffect;
        Output.m_pEffectTail = pEffect;
    }
    else
    {
        Output.m_pEffectTail->m_pNext = pEffect;
        Output.m_pEffectTail = pEffect;
    }

}

//-----------------------------------------------------------------------------
complex_effect *ParseComplexComplex(token_stream &Tok,parse_output &Output,s32 ownerid,parse_variables &Default)
{
    complex_effect  *pEffect;
    parse_variables Vars;


    pEffect = new complex_effect(Default);
    pEffect->m_Id = Output.m_CfxCount++;
    pEffect->m_Type = CFXTYPE_COMPLEX;
    pEffect->m_Owner = ownerid;
    ExpectString(Tok,pEffect->m_Label);

    switch (ExpectToken(Tok,"{|;"))
    {
    case 0:                             // Got '{'
        while (!ParseVariable(Tok,pEffect->m_Vars));
        break;
    case 1:                             // Got ';'
        break;
    default:
        ASSERT(FALSE);
    }
    return pEffect;
}

complex_effect *ParseComplexSimple(token_stream &Tok,parse_output &Output,s32 type,s32 ownerid,parse_variables &Default)
{
    complex_effect *pChild;
    xbool           Done;

    pChild = new complex_effect(Default);
    ASSERT(pChild);
    pChild->m_Id = Output.m_CfxCount++;
    pChild->m_Type = type;
    pChild->m_Owner = ownerid;

    ExpectString(Tok,pChild->m_Label);
    pChild->m_pSample = FindSample(Tok,pChild->m_Label,Output,FALSE,type);

    Done = FALSE;
    while (!Done)
    {
        switch (ExpectToken(Tok,"{|;"))
        {
        case 0:                                         // Got '{' - we have modifiers
            while (!ParseVariable(Tok,pChild->m_Vars));
            Done = TRUE;
           break;
        case 1:                                         // Got ';' - no modifiers
            Done = TRUE;
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
    return pChild;
}

sample_body *FindSample(token_stream &Tok,char *pFilename,parse_output &Output,xbool ForceLooped,s32 type)
{
    sample_body     *pSample;
    sample_body     *pPrev;
    X_FILE          *fp;
    s32             AlignedLength;
    t_DecodeHeader  Header;
    s32             CompressedLength;

    pSample = Output.m_pSamples;

    // If the sample already exists, just return that information rather
    // than duplicate
    while (pSample)
    {
        if (x_stricmp(pFilename,pSample->m_Filename)==0)
        {
            x_printf("Duplicate sample %s\n",pFilename);
            return pSample;
        }
        pSample = pSample->m_pNext;
    }


    pSample = new sample_body;
    x_strcpy(pSample->m_Filename,pFilename);
    fp = x_fopen(pFilename,"rb");
    if (!fp)
    {
        Error(Tok,xfs("Cannot open sample file %s",pFilename));
    }
    else
    {
        x_fclose(fp);
    }

    x_memset(&Header,0,sizeof(t_DecodeHeader));
    pSample->m_Type = type;
    Header.Type = type;
    DecodeToPcm(pFilename,&Header);
    if (!Header.pLeft)
    {
        Error(Tok,xfs("Unable to decode file %s",pFilename));
    }

    pSample->m_SampleRate       = Header.SampleRate;
    pSample->m_Length           = Header.Length;
    pSample->m_Flags            = Header.Flags;
    //
    // Output data cannot be any bigger than the decoded pcm so that's
    // the buffer size we allocate.
    //
    pSample->m_pData = (byte *)x_malloc(pSample->m_Length+4096);
    if (!pSample->m_pData)
    {
        Error(Tok,xfs("Unable to allocate %d bytes for encoded data\n",pSample->m_Length));
    }

    if (ForceLooped)
    {
        Header.LoopStart = 0;
        Header.LoopEnd  = pSample->m_Length;
    }

    if (verbose_flag) x_printf("Encoding Sample....");
    if ( verbose_flag && (Header.LoopStart != Header.LoopEnd))
    {
        x_printf("\nLoop start = %d, Loop End = %d\n",Header.LoopStart,Header.LoopEnd);
    }

    switch (GetTargetPlatform())
    {
    case TARGET_TYPE_PS2:
        CompressedLength = ps2_EncodeToAdpcm( pSample->m_Length,(s16 *)pSample->m_pData,&Header);
        break;
    case TARGET_TYPE_GCN:
        CompressedLength = gcn_EncodeToAdpcm( pSample->m_Length,(s16 *)pSample->m_pData,&Header);
        break;
	case TARGET_TYPE_PC:
		CompressedLength = pc_Encode( pSample->m_Length, pSample->m_pData, (s8 *)pSample->m_Filename);
		break;
    default:
        ASSERTS(FALSE,"No Target encode routine defined for this platform\n");
        break;
    }

    if (!CompressedLength)
    {
        Error(Tok,xfs("Encoding failed for file %s",pFilename));
    }

    AlignedLength = (CompressedLength + VOICE_ALIGNMENT - 1) & ~(VOICE_ALIGNMENT - 1);
    if (verbose_flag) x_printf("Done. Encoded length is %d bytes.\n",AlignedLength);
    if (Header.LoopStart != Header.LoopEnd)
    {
        pSample->m_Flags |= AUDFLAG_HARDWARE_LOOP;
    }
    x_free(Header.pLeft);
    if (Header.Flags & AUDFLAG_STEREO)
    {
        ASSERT(Header.pRight);
    }
    else
    {
        ASSERT(!Header.pRight);
    }
	pSample->m_Length = CompressedLength;
	pSample->m_MediaLocation = Output.m_SampleOffset;
	Output.m_SampleOffset += (pSample->m_Length+4095)&~4095;

	pPrev = Output.m_pSamples;
    if (pPrev)
    {
        while (pPrev->m_pNext)
        {
            pPrev = pPrev->m_pNext;
        }
        pPrev->m_pNext = pSample;
    }
    else
    {
        Output.m_pSamples = pSample;
    }

    return pSample;
}
 