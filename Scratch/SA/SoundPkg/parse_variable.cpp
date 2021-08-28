#include "parse.hpp"
#include "parse_variable.hpp"
#include "parse_envelope.hpp"

//-----------------------------------------------------------------------------
xbool ParseVariable(token_stream &Tok,parse_variables &Vars)
{
    s32 token;
    s32 ExpectSemicolon;

    token = ExpectToken(Tok,"Volume|Pitch|Priority|LoopCount|Flags|Delay|Weight|Falloff|Envelope|Pan|}");
    ExpectSemicolon = TRUE;
    switch (token)
    {
    case 0:             // Volume
        Vars.m_Volume = ExpectFloat(Tok);
        break;
    case 1:             // Pitch
        Vars.m_Pitch = ExpectFloat(Tok);
        break;
    case 2:             // Priority
        Vars.m_Priority = ExpectInteger(Tok);
        if (Vars.m_Priority > 127)
        {
            Error(Tok,"Priority must be -1, or 0..127");
        }
        break;
    case 3:             // LoopCount
        Vars.m_LoopCount = ExpectInteger(Tok);
        if (Vars.m_LoopCount > 127)
        {
            Error(Tok,"Loop count must be -1, or 0..127");
        }
        break;
    case 4:             // Flags
        Vars.m_Flags = ExpectFlags(Tok);
        ExpectSemicolon=FALSE;
        break;
    case 5:
        Vars.m_Delay = ExpectFloat(Tok);
        break;
    case 6:
        Vars.m_Weight = ExpectFloat(Tok);
        break;
    case 7:
        Vars.m_Falloff = ExpectFloat(Tok);
        if ( (Vars.m_Falloff < 0.01) || (Vars.m_Falloff > 2.0f) )
        {
            Error(Tok,"Falloff range must be between 0.01 and 2.0");
        }
        break;
    case 8:
        ParseEnvelope(Tok,Vars);
        ExpectSemicolon = FALSE;
        break;
    case 9:
        Vars.m_Pan = ExpectFloat(Tok);
        if ( (Vars.m_Pan < -1.0f) || (Vars.m_Pan > 1.0f) )
        {
            Error(Tok,"Pan must be between -1.0 (full left) and 1.0 (full right)");
        }
        break;
    case 10:
        return TRUE;
    default:
        ASSERT(FALSE);
    }
    if (ExpectSemicolon)
    {
        ExpectToken(Tok,";");
    }
    return FALSE;
}
