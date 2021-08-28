///////////////////////////////////////////////////////////////////////////
// PostEffectMgr.cpp
///////////////////////////////////////////////////////////////////////////

#include "PostEffectMgr.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "Objects\LevelSettings.hpp"
#include "Render\Render.hpp"
#include "GameLib\RenderContext.hpp"
#include "AudioMgr\AudioMgr.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

///////////////////////////////////////////////////////////////////////////
// data
///////////////////////////////////////////////////////////////////////////

post_effect_mgr     g_PostEffectMgr;

static const f32    kNormalBlurOffset = 0.0f;
static const xcolor kNormalBlurColor(128,128,128,16);

///////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////

post_effect_mgr::post_effect_mgr( void )
{
    Init();
}

//=========================================================================

post_effect_mgr::~post_effect_mgr( void )
{
}

//=========================================================================

void post_effect_mgr::Init( void )
{
    // clear out everything
    m_StartFadeColor.Set(0,0,0,0);
    m_EndFadeColor.Set(0,0,0,0);
    m_FadeTimeElapsed = 0.0f;
    m_FadeTime        = 0.0f;
    m_bFadeAudio      = FALSE;

    s32 i;
    for( i = 0; i < MAX_LOCAL_PLAYERS; i++ )
    {
        m_bDoPainBlur[i]    = FALSE;
        m_PainBlurOffset[i] = kNormalBlurOffset;
        m_PainBlurColor[i]  = kNormalBlurColor;
    }
}

//=========================================================================

void post_effect_mgr::OnUpdate( f32 DeltaTime )
{
    s32 i;
    for( i = 0; i < MAX_LOCAL_PLAYERS; i++ )
    {
        ///////////////////////////////////////////////////////////////////
        // update the pain/filter blur
        ///////////////////////////////////////////////////////////////////
        f32 fTimedBlur = 30.0f * DeltaTime;
        s32 RedDelta   = (s32)MAX((((m_PainBlurColor[i].R - 128)/10) * fTimedBlur), 10.0f * fTimedBlur);
        s32 AlphaDelta = (s32)MAX((((m_PainBlurColor[i].A -  15)/10) * fTimedBlur), 10.0f * fTimedBlur);
        if( m_PainBlurOffset[i] > kNormalBlurOffset )
            m_PainBlurOffset[i] -= fTimedBlur;
        else
            m_PainBlurOffset[i] = kNormalBlurOffset;
       
        if( m_PainBlurColor[i].R > kNormalBlurColor.R )
            m_PainBlurColor[i].R -= RedDelta;
        else
            m_PainBlurColor[i].R  = kNormalBlurColor.R;
        if( m_PainBlurColor[i].A > kNormalBlurColor.A )
            m_PainBlurColor[i].A -= AlphaDelta;
        else
            m_PainBlurColor[i].A = kNormalBlurColor.A;

        if( (m_PainBlurOffset[i]  == kNormalBlurOffset ) &&
            (m_PainBlurColor[i].R == kNormalBlurColor.R) &&
            (m_PainBlurColor[i].A == kNormalBlurColor.A) )
        {
            m_bDoPainBlur[i] = FALSE;
        }
    }

    // update the screen fade
    if( m_FadeTime > 0.0f )
    {
        xbool FinishedFading = FALSE;

        m_FadeTimeElapsed += DeltaTime;
        if( m_FadeTimeElapsed > m_FadeTime )
        {
            m_FadeTime        = 0.0f;
            m_FadeTimeElapsed = 0.0f;
            m_StartFadeColor  = m_EndFadeColor;
            FinishedFading    = TRUE;
        }

        // HACK HACK HACK - THIS IS A MAJOR HACK-O-RAMA FOR E3 AND OFFENDS ME
        // GREATLY. WE NEED TO HAVE THE AUDIO FADE OUT WITH THE SCREEN AT
        // THE END OF A LEVEL. WE NEED THE AUDIO MANAGER TO HAVE SLIDER
        // CONTROL, AND THEN WE CAN DO THIS FROM AN AUDIO TRIGGER.
        if( m_bFadeAudio )
        {
            f32 T;
            
            if (FinishedFading)
                T = 1.0f;
            else
                T = m_FadeTimeElapsed / m_FadeTime;

            f32 A = (f32)m_StartFadeColor.A + T * ((f32)m_EndFadeColor.A - (f32)m_StartFadeColor.A);
            f32 Volume = 1.0f - (A / 255.0f);

            if( Volume > 0.5f )
                g_AudioMgr.SetMasterVolume( Volume );
            else
                g_AudioMgr.SetMasterVolume( 0.0f );

            if( FinishedFading )
                m_bFadeAudio = FALSE;
        }
    }
}

//=========================================================================

void post_effect_mgr::Render( void )
{
    CONTEXT( "post_effect_mgr::Render" );

    // don't do post-effects when we're in the pip
    if( g_RenderContext.m_bIsPipRender )
        return;

    // start doing post-effects
    render::BeginPostEffects();
    
    // do the self-illum glows
    if ( g_RenderContext.m_bIsMutated )
    {
        render::ApplySelfIllumGlows( 0.8f, 64 );
    }
    else
    {
        render::ApplySelfIllumGlows( 0.0f, 255 );
    }

    // handle normal pain/screen blurring
    s32 LocalPlayerIndex = g_RenderContext.LocalPlayerIndex;
    if ( m_bDoPainBlur[LocalPlayerIndex] )
    {
        // do a pain blur
        render::MipFilter( 2, m_PainBlurOffset[LocalPlayerIndex], render::FALLOFF_CONSTANT, m_PainBlurColor[LocalPlayerIndex], 0.0f, 0.0f, LocalPlayerIndex );
    }
    else
    {
        // do a very slight depth-of-field to remove some of the sparkles
        render::MipFilter( 2, 0.0f, render::FALLOFF_EXP, xcolor(128,128,128,128), 8.0f, 0.0f, LocalPlayerIndex );
    }

    // do the "mutant" vision mode
    if( g_RenderContext.m_bIsMutated )
    {
        // do a radial blur
        f32    Zoom       = 0.8f;
        radian Angle      = R_2;
        f32    AlphaSub   = 104.0f;
        f32    AlphaScale = 148.0f;
        render::RadialBlur( Zoom, Angle, AlphaSub, AlphaScale );
    }

    /*
    // KSS -- As per design, rip out screen warp on MSN projectiles.
    // go through all of the meson projectiles and render a "warp" effect
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_GRAV_CHARGE_PROJECTILE );
    while( SlotID != SLOT_NULL )
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pObj );
        render::AddScreenWarp( pObj->GetPosition(), 400.0f, 0.4f );
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    */

    // done
    render::EndPostEffects();
}

//=========================================================================

void post_effect_mgr::RenderFog( void )
{
    render::BeginMidPostEffects();
    render::ZFogFilter( render::FALLOFF_CUSTOM, g_RenderContext.LocalPlayerIndex );
    render::EndMidPostEffects();
}

//=========================================================================

void post_effect_mgr::RenderScreenFade( void )
{
    // screen fading
    xcolor FadeColor( m_EndFadeColor );
    if( m_FadeTime > 0.0f )
    {
        f32 T = m_FadeTimeElapsed / m_FadeTime;
        f32 R = (f32)m_StartFadeColor.R + T * ((f32)m_EndFadeColor.R - (f32)m_StartFadeColor.R);
        f32 G = (f32)m_StartFadeColor.G + T * ((f32)m_EndFadeColor.G - (f32)m_StartFadeColor.G);
        f32 B = (f32)m_StartFadeColor.B + T * ((f32)m_EndFadeColor.B - (f32)m_StartFadeColor.B);
        f32 A = (f32)m_StartFadeColor.A + T * ((f32)m_EndFadeColor.A - (f32)m_StartFadeColor.A);
        FadeColor.R = (u8)R;
        FadeColor.G = (u8)G;
        FadeColor.B = (u8)B;
        FadeColor.A = (u8)A;
    }
    if( FadeColor.A != 0 )
    {
        render::BeginPostEffects();
        render::ScreenFade( FadeColor );
        render::EndPostEffects();
    }
}

//=========================================================================

void post_effect_mgr::StartPainBlur( s32 LocalPlayerIndex, f32 InitialOffset, xcolor InitialColor )
{
    m_bDoPainBlur[LocalPlayerIndex]    = TRUE;
    m_PainBlurOffset[LocalPlayerIndex] = InitialOffset;
    m_PainBlurColor[LocalPlayerIndex]  = InitialColor;
}

//=========================================================================

void post_effect_mgr::StartScreenFade( xcolor FadeColor, f32 TimeToFade, xbool FadeAudio )
{
    m_FadeTimeElapsed = 0.0f;
    m_FadeTime        = TimeToFade;
    m_StartFadeColor  = m_EndFadeColor;
    m_EndFadeColor    = FadeColor;
    m_bFadeAudio      = FadeAudio;
    if( TimeToFade == 0.0f )
    {
        m_StartFadeColor = m_EndFadeColor;

        // HACK HACK HACK HACK HACK - SEE COMMENT IN UPDATE
        if( FadeAudio )
        {
            f32 Volume = 1.0f - ((f32)m_StartFadeColor.A / 255.0f);
            g_AudioMgr.SetMasterVolume( Volume );
            m_bFadeAudio = FALSE;
        }
    }
}
