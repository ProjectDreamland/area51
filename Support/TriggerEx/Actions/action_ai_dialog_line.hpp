///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_dialog_line.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_dialog_line_
#define _action_ai_dialog_line_


//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================================================================
// action_ai_dialog_line
//=========================================================================

class action_ai_dialog_line : public action_ai_base
{
public:
                    action_ai_dialog_line               ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_DIALOG_LINE;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Say a line of dialog."; } 
    virtual         const char*         GetDescription  ( void );

    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32*                GetAnimRef      ( xstring& Desc, s32& AnimName  );
    virtual         rhandle<char>*      GetSoundRef     ( xstring& Desc, s32& SoundName ) { Desc = "Sound error: "; SoundName = m_SoundName; return &m_hAudioPackage; }
#endif

    virtual         ai_action_types     GetAIActionType ( void ) { return AI_DIALOG_LINE; }

    inline          s32                 GetNextState    ( void ) { return m_NextAiState; }
    inline          const char*         GetAnimGroupName( void ); 
    inline          const char*         GetAnimName     ( void );
    inline          u32                 GetAnimFlags    ( void ) { return m_AnimFlags ; }
    inline          const char*         GetSoundName    ( void );
    inline          xbool               GetInterrupAI   ( void ) { return m_InterruptAI; }    
    inline          xbool               GetBlockOnDialog( void ) { return m_BlockOnDialog; }    
    inline          xbool               GetKillAnim     ( void ) { return m_KillAnim; }    
    inline          f32                 GetBlendTime    ( void ) { return m_BlendTime; }    
    inline          u8                  GetSoundFlags   ( void ) { return m_SoundFlags; }
    inline          void                InitPreDelay( void );
    inline          void                InitPostDelay( void );
    inline          xbool               CheckPreDelay( f32 DeltaTime );
    inline          xbool               CheckPostDelay( f32 DeltaTime );
    
protected:
    
    s32             m_AnimGroupName;   
    s32             m_AnimName;
    u32             m_AnimFlags;
    
    rhandle<char>   m_hAudioPackage;        // Audio resource for this object.
    s32             m_SoundName;

    xbool           m_InterruptAI;
    xbool           m_BlockOnDialog;
    xbool           m_KillAnim;
    xbool           m_SoundFlags;
    f32             m_BlendTime;
    
    f32             m_PreDialogDelay;
    f32             m_PostDialogDelay;
    f32             m_DelayPreCountDown;
    f32             m_DelayPostCountDown;
    s32             m_NextAiState;          // AI State to change too at end..
};

//=========================================================================

inline const char* action_ai_dialog_line::GetAnimGroupName( void ) 
{ 
    if (m_AnimGroupName != -1)
        return g_StringMgr.GetString(m_AnimGroupName); 
    else
        return "";
}

//=========================================================================

inline const char* action_ai_dialog_line::GetAnimName( void ) 
{ 
    if (m_AnimName != -1)
        return g_StringMgr.GetString(m_AnimName); 
    else
        return "";
}

//=========================================================================

inline const char* action_ai_dialog_line::GetSoundName( void ) 
{ 
    if (m_SoundName != -1)
        return g_StringMgr.GetString(m_SoundName); 
    else
        return "";
}

//=========================================================================

inline void action_ai_dialog_line::InitPreDelay( void )
{
    m_DelayPreCountDown = m_PreDialogDelay;
}

//=========================================================================

inline void action_ai_dialog_line::InitPostDelay( void )
{
    m_DelayPostCountDown = m_PostDialogDelay;
}

//=========================================================================

inline xbool action_ai_dialog_line::CheckPreDelay( f32 DeltaTime )
{
    if(m_DelayPreCountDown <= 0.0f)
	    return TRUE;

    m_DelayPreCountDown -= DeltaTime;
    return FALSE;
}
//=========================================================================

inline xbool action_ai_dialog_line::CheckPostDelay( f32 DeltaTime )
{
    if(m_DelayPostCountDown <= 0.0f)
	    return TRUE;

    m_DelayPostCountDown -= DeltaTime;
    return FALSE;
}

#endif
