///////////////////////////////////////////////////////////////////////////////
//
//  action_screen_fade.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_screen_fade.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\Render\PostEffectMgr.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_screen_fade::action_screen_fade( guid ParentGuid ):
    actions_ex_base(  ParentGuid ),
    m_FadeTime  ( 3.0f ),
    m_FadeColor (0,0,0,255),
    m_bFadeAudio( FALSE )
{
}

//=============================================================================

xbool action_screen_fade::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    g_PostEffectMgr.StartScreenFade( m_FadeColor, m_FadeTime, m_bFadeAudio );
    return TRUE;
}    

//=============================================================================

void action_screen_fade::OnEnumProp ( prop_enum& rPropList )
{ 
    actions_ex_base::OnEnumProp( rPropList );

    rPropList.PropEnumFloat( "FadeTime",  "Time it takes for the screen to fade (in seconds)", 0 );
    rPropList.PropEnumColor( "FadeColor", "Color that we are fading to (alpha is important!)", 0 );
    rPropList.PropEnumBool ( "FadeAudio", "Fade the audio based on the alpha value", 0 );
}

//=============================================================================

xbool action_screen_fade::OnProperty ( prop_query& rPropQuery )
{
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if( rPropQuery.VarFloat( "FadeTime", m_FadeTime, 0.0f, F32_MAX ) )
        return TRUE;

    if( rPropQuery.VarColor( "FadeColor", m_FadeColor ) )
        return TRUE;

    if( rPropQuery.VarBool( "FadeAudio", m_bFadeAudio ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_screen_fade::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Screen Fade");
    return Info.Get();
}
