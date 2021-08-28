#include "parse_envelope.hpp"
#include "parse.hpp"

/*
        Envelope
        {
            AttackRate      0.0;            // Milliseconds
            DecayRate       0.0;            // Milliseconds
            SustainLevel    1.0;            // Relative volume level
            SustainRate     0.0;            // Milliseconds
            ReleaseRate     5.0;            // Milliseconds
        };
    };
*/
  //-----------------------------------------------------------------------------
void ParseEnvelope(token_stream &Tok,parse_variables &Vars)
{
    s32 token;
    xbool done;

    ExpectToken(Tok,"{");
    done = FALSE;
    while (!done)
    {
        token = ExpectToken(Tok,"AttackRate|DecayRate|SustainLevel|SustainRate|ReleaseRate|}");
        switch (token)
        {
        case 0:                 // AttackRate
            Vars.m_Envelope.m_AttackRate = ExpectFloat(Tok);
            break;
        case 1:                 // DecayRate
            Vars.m_Envelope.m_DecayRate = ExpectFloat(Tok);
            break;
        case 2:                 // SustainLevel
            Vars.m_Envelope.m_SustainLevel = ExpectFloat(Tok);
            break;
        case 3:                 // SustainRate
            Vars.m_Envelope.m_SustainRate = ExpectFloat(Tok);
            break;
        case 4:                 // ReleaseRate
            Vars.m_Envelope.m_ReleaseRate = ExpectFloat(Tok);
            break;
        case 5:                 // }
            done = TRUE;
            break;
        default:
            ASSERT(FALSE);
        }
        if (!done)
        {
            ExpectToken(Tok,";");
        }
    }
}
