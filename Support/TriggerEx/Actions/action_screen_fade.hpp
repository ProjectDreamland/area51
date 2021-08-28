///////////////////////////////////////////////////////////////////////////////
//
//  action_screen_fade.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_screen_fade_
#define _action_screen_fade_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================================================================
// Check Property
//=========================================================================

class action_screen_fade : public actions_ex_base
{
public:
                    action_screen_fade                ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_SCREEN_FADE;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Full Screen Fading"; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

protected:
    f32             m_FadeTime;
    xcolor          m_FadeColor;
    xbool           m_bFadeAudio;
};

#endif
