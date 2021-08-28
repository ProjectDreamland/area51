#ifndef __PARSE_VARIABLE_H
#define __PARSE_VARIABLE_H

#include "../../support/tokenizer/tokenizer.hpp"

struct parse_envelope
{
    f32     m_AttackRate;
    f32     m_DecayRate;
    f32     m_SustainLevel;
    f32     m_SustainRate;
    f32     m_ReleaseRate;

    parse_envelope(void)
    {
        m_AttackRate    = 0.0f;
        m_DecayRate     = 0.0f;
        m_SustainLevel  = 1.0f;
        m_SustainRate   = 0.0f;
        m_ReleaseRate   = 5.0f;
    }
};

struct parse_variables
{
    f32             m_Volume;
    f32             m_Pitch;
    f32             m_Delay;
    f32             m_Weight;
    f32             m_Falloff;
    f32             m_Pan;
    s32             m_Priority;
    s32             m_LoopCount;
    s32             m_Flags;
    parse_envelope  m_Envelope;

    parse_variables(void)
    {
        m_Volume    = 1.0f;
        m_Pitch     = 1.0f;
        m_Delay     = 0.0f;
        m_Weight    = 0.0f;
        m_Pan       = 0.0f;
        m_Priority  = 100;
        m_LoopCount = 0;
        m_Flags     = 0;
        m_Falloff   = 1.0f;
    }
};


xbool ParseVariable(token_stream &Tok,parse_variables &Vars);

#endif