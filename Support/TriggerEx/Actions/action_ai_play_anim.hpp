///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_play_anim.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_play_anim_
#define _action_ai_play_anim_

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================== ==============================================
// action_ai_play_anim
//=========================================================================

class action_ai_play_anim : public action_ai_base
{
// Functions
public:
                    action_ai_play_anim               ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_PLAY_ANIM;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Play a cinematic anim."; } 
    virtual         const char*         GetDescription  ( void );

    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32*                GetAnimRef      ( xstring& Desc, s32& AnimName ) { Desc = "Animation error: "; AnimName  = m_AnimName; return &m_AnimGroupName; }
#endif

    virtual         ai_action_types     GetAIActionType ( void ) { return AI_PLAY_ANIMATION; }

    inline          const char*         GetAnimGroup    ( void );
    inline          const char*         GetAnimName     ( void );                       
    inline          f32                 GetAnimBlendTime( void ) { return m_AnimBlendTime ; }                       
    inline          f32                 GetAnimPlayTime ( void ) { return m_AnimPlayTime ; }                       
    inline          u32                 GetAnimFlags    ( void ) { return m_AnimFlags ;    }
    inline          xbool               GetInterrupAI   ( void ) { return m_InterruptAI; }    
    
    inline          s32                 GetNextState    ( void ) { return m_NextAiState;   }

                    guid                GetScaledTargetGuid( void );

protected:

    object_affecter m_TargetAffecter;

    xbool           m_InterruptAI;
    s32             m_AnimGroupName;    // Name of animation group
    s32             m_AnimName;         // Name of animation to play
    f32             m_AnimBlendTime;    // Blend time
    f32             m_AnimPlayTime ;    // Number of cycles/seconds to play
    u32             m_AnimFlags ;       // Animation control flags

    s32             m_NextAiState;      // AI State to change too at end..
};

//=========================================================================

inline const char* action_ai_play_anim::GetAnimGroup( void ) 
{ 
    if (m_AnimGroupName != -1)
        return g_StringMgr.GetString(m_AnimGroupName); 
    else
        return "";
}

//=========================================================================

inline const char* action_ai_play_anim::GetAnimName( void ) 
{ 
    if (m_AnimName != -1)
        return g_StringMgr.GetString(m_AnimName); 
    else
        return "";
}

#endif
