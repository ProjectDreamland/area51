// character_trigger_state : header file
/////////////////////////////////////////////////////////////////////////////

#ifndef __character_trigger_state_H
#define __character_trigger_state_H


#include "..\CharacterState.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_base.hpp"
#include "Characters\ResponseList.hpp"

/////////////////////////////////////////////////////////////////////////////
// character_trigger_state class

class character_trigger_state : public character_state
{
public:
    enum base_trigger_phases
    {
        PHASE_TRIGGER_ATTACK_TARGET = PHASE_BASE_COUNT,
        PHASE_TRIGGER_GOTO_TARGET,
        PHASE_TRIGGER_LOOKAT_TARGET,
        PHASE_TRIGGER_TURNTO_LOCATION,
        PHASE_TRIGGER_DIALOG,
        PHASE_TRIGGER_DO_ACTION,
        PHASE_TRIGGER_SEARCH_FOR_TARGET,
        PHASE_TRIGGER_DEATH,
    };

    struct PathfindData
    {
        f32         m_Distance;
        radian      m_YawDifference;
        xbool       m_Retreating;
        loco::move_style    m_MoveStyle;
    };
    
    struct LookatData
    {
        xbool       m_LookatHead;
        f32         m_LookatFOV;
        f32         m_LookatDistance;
    };

    struct DialogData
    {
        char        m_AnimGroupName[64];
        char        m_AnimName[64];   
        char        m_SoundName[64];
        f32         m_AnimBlendTime;
        u32         m_AnimFlags;  
        xbool       m_BlockOnDialog;
        xbool       m_KillAnim;
        u8          m_SoundFlags;
    };

    struct PlayAnimData
    {
        char        m_AnimGroupName[64];
        char        m_AnimName[64];   
        f32         m_AnimBlendTime;
        f32         m_AnimPlayTime;
        u32         m_AnimFlags;  
    };

    struct DeathData
    {
        char        m_AnimGroupName[64];
        char        m_AnimName[64]; 
        s32         m_DeathType;  
        f32         m_RagdollForceAmount;    
        f32         m_RagdollForceRadius;    
    };
    
    union UnionData
    {
        PathfindData    m_PathfindData;
        LookatData      m_LookatData;
        DialogData      m_DialogData;
        PlayAnimData    m_PlayAnimData;
        DeathData       m_DeathData;
    };

    struct TriggerData
    {
        TriggerData()   
        {
            m_ActionFocus   = 0;
            m_NextAiState   = 0;
            m_TriggerGuid   = 0;
            m_TaskListGuid  = 0;
            m_MustSucceed   = FALSE;
            m_Blocking      = FALSE;
            m_ActionType    = action_ai_base::AI_ACTION_NULL;
        }
        UnionData           m_UnionData;
        guid                m_ActionFocus;      //for pathfid, attack, look at, and as the scaled target for play anim
        s32                 m_NextAiState;
        guid                m_TriggerGuid;
        guid                m_TaskListGuid;
        response_list       m_ResponseList;
        xbool               m_MustSucceed;
        xbool               m_Blocking;

        action_ai_base::ai_action_types     m_ActionType;
    };

// Construction / destruction
	character_trigger_state   ( character& Character, states State );

    virtual void        OnEnter                 ( void ) ;
    virtual s32         UpdatePhase             ( f32 DeltaTime );
    virtual void        ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState( f32 DeltaTime );

#ifndef X_RETAIL
    virtual void        OnDebugRender   ( void );
#endif // X_RETAIL
    
    virtual void        OnThink             ( void );
    virtual xbool       OnExit              ( void ) ;
     
    virtual void        OnRender            ( void );
   
    virtual void        OnEnumProp          ( prop_enum&    List ) ;
    virtual xbool       OnProperty          ( prop_query&   rPropQuery ) ;

    virtual xbool       IgnorePain          ( const pain& Pain )        { (void)Pain; return m_TriggerData.m_ResponseList.HasFlags(response_list::RF_INVINCIBLE); }
    virtual xbool       IgnoreAlerts        ( void )                    { return m_TriggerData.m_ResponseList.HasFlags(response_list::RF_IGNORE_ALERTS); }    
    virtual xbool       IgnoreSight         ( void )                    { return m_TriggerData.m_ResponseList.HasFlags(response_list::RF_IGNORE_SIGHT); }    
    virtual xbool       IgnoreSound         ( void )                    { return m_TriggerData.m_ResponseList.HasFlags(response_list::RF_IGNORE_SOUND); }    
    virtual xbool       IgnoreAttacks       ( void )                    { return m_TriggerData.m_ResponseList.HasFlags(response_list::RF_IGNORE_ATTACKS); }    
    virtual xbool       IgnoreFlinches      ( void );            

    virtual xbool       OnPain              ( const pain& Pain );
//    virtual void        SetContext          ( void* pContext );
            void        SetTriggerData      ( TriggerData triggerData )         { m_TriggerData = triggerData; }
            TriggerData GetTriggerData      ( void )                            { return m_TriggerData; }
            xbool       GetTaskCompleted    ( void )                            { return m_TaskCompleted; }
            guid        GetTriggerGuid      ( void )                            { return m_TriggerData.m_TriggerGuid; }

    virtual const char*GetStateName ( void )                        { return "TRIGGER STATE"; }
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );
            void        SetPostTriggerTarget ( guid target ) { m_PostTriggerTarget = target; }
    

protected:
    
//    character&              m_Base ;  // Character owner


    TriggerData             m_TriggerData;
    xbool                   m_TaskCompleted;
    f32                     m_BaseAnimPlayRate;
    guid                    m_PostTriggerTarget;
};

/////////////////////////////////////////////////////////////////////////////

#endif // __CHARACTER_TRIGGER_MOVE_TO_STATE_H

