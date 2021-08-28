
#ifndef POSTEFFECTMGR_HPP
#define POSTEFFECTMGR_HPP

//=========================================================================

#include "x_color.hpp"
#include "Objects\Player.hpp"

//=========================================================================

class post_effect_mgr
{
public:
    post_effect_mgr( void );
    ~post_effect_mgr( void );

    void    Init        ( void );
    void    OnUpdate    ( f32 DeltaTime );
    void    Render      ( void );

    // Fog must be done separately for particles to get fogged properly
    void    RenderFog   ( void );

    // screen fade must be done separately so that it can happen after the hud
    void    RenderScreenFade    ( void );

    // post-effects that are specific to a particular player
    void    StartPainBlur       ( s32 LocalPlayerIndex, f32 InitialOffset, xcolor InitialColor );
    
    // NOTE: This will be the entire screen...not separate for split-screen modes!
    void    StartScreenFade     ( xcolor FadeColor, f32 TimeToFade, xbool FadeAudio = FALSE );

protected:
    // data related to doing pain blurs
    xbool   m_bDoPainBlur[MAX_LOCAL_PLAYERS];
    f32     m_PainBlurOffset[MAX_LOCAL_PLAYERS];
    xcolor  m_PainBlurColor[MAX_LOCAL_PLAYERS];

    // data related to screen fades (for begin/end of level campaign effects)
    xcolor  m_StartFadeColor;
    xcolor  m_EndFadeColor;
    f32     m_FadeTimeElapsed;
    f32     m_FadeTime;
    xbool   m_bFadeAudio;           // WARNING: HACK - SEE CPP FILE FOR DETAILS
};

//=========================================================================

extern post_effect_mgr g_PostEffectMgr;

//=========================================================================

#endif // POSTEFFECTMGR_HPP

