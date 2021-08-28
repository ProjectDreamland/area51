#ifndef _CHARACTER_COVER_STATE_HPP_
#define _CHARACTER_COVER_STATE_HPP_

#include "..\CharacterState.hpp"

class character_cover_state : public character_state
{
// enums
public:
    enum base_cover_phases
    {
        PHASE_COVER_GOTO_COVER = PHASE_BASE_COUNT,
        PHASE_COVER_FACE_EXACT,
        PHASE_COVER_ENTER_COVER,
        PHASE_COVER_IDLE,
        PHASE_COVER_SHOOT,
        PHASE_COVER_THROW_GRENADE,
        PHASE_COVER_ROLL_OUT,
        PHASE_COVER_OUT_IDLE,
        PHASE_COVER_OUT_SCAN,
        PHASE_COVER_OUT_GRENADE,
        PHASE_COVER_FULL_AUTO,
        PHASE_COVER_ROLL_IN,
        PHASE_COVER_EXIT_COVER,
        PHASE_COVER_ALIGN_FOR_MELEE,
        PHASE_COVER_MELEE,
        PHASE_COVER_STAND_AND_SHOOT,
        PHASE_COVER_GOTO_ROLLED_OUT,
        PHASE_COVER_FACE_FORWARD,
    };

    enum eLeaveCover
    {
        LEAVE_COVER_INVALID,
        LEAVE_COVER_WHEN_BROKEN,
        LEAVE_COVER_WHEN_DAMAGED,
        LEAVE_COVER_WHEN_CAN_REACH_TARGET,
    };
    

// Functions
public:
                    character_cover_state  ( character& ourCharacter, character_state::states State );
    virtual        ~character_cover_state  ( void );

    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual xbool   OnExit                  ( void );
    virtual void    OnThink                 ( void );
    virtual xbool   OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags );
    virtual void    OnCoverChanged          ( guid NewCover );
    
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;

    // character state specific virtual functions
    virtual xbool   UseRelativeMode         ( void );
            void    UpdateRelativeMode      ( void );
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState( f32 DeltaTime );
            xbool   GetIsInCover            ( void )        { return m_InCover; }
    virtual xbool   GetIsAtCover            ( void )        { return ( m_CurrentPhase != PHASE_COVER_GOTO_COVER ); }
    virtual xbool   ProvideAutofire         ( void );
            void    CancelAutofire          ( void );
            f32     GetTimeInCover          ( void )        { return m_TimeInCover; }
            f32     GetTimeTillNextAction   ( void )        { return m_DelayTillNextAction; }
            guid    GetCoverNode            ( void )        { return m_CurrentCover; }
            anim_group::handle GetCoverAnimGroupHandle ( void )             { return m_CoverAnimGroupHandle; }
            void    SetLeaveCondition       ( eLeaveCover leaveCondition )  { m_LeaveCoverCondition = leaveCondition; }
    virtual xbool   IgnoreFullBodyFlinches  ( void );
            void    CoverRequestDialogDone  ( void );
            void    UpdateBlocksLOF         ( void );
            xbool   GetRolledOutPosition    ( guid coverNode, vector3& rolledOutPos );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_COVER_COVER"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );
#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

protected:

            void    SetCurrentCover     ( guid Cover );
            void    SetInCover          ( xbool inCover );
            void    SetRolledOut        ( xbool rolledOut );
            xbool   HasAnimForCoverPhase( s32 coverPhase );
            void    UpdateUsedDelays    ( void );
    virtual xbool   GetAnimNameFromPhase( s32 nextPhase, char* pBufferSize32 );
    virtual s32     GetNextPhaseRolledOut();
    virtual s32     GetNextPhaseRolledIn();
    virtual s32     GetNextPhaseOutOfCover();
    virtual s32     GetFacePhase();
    virtual s32     GetGotoCoverPhase();


protected:
    guid        m_CurrentCover;
    anim_group::handle  m_CoverAnimGroupHandle; // the animgrouphandle for our current cover.    
    eLeaveCover m_LeaveCoverCondition;
    f32         m_TimeTillBored;
    
    states      m_DesiredState;

    vector3     m_RolledOutPosition; 
    radian      m_RolledOutFacing;

    f32         m_TimeRolledOut;
    f32         m_TimeInCover;
    f32         m_TimeSinceLastMelee;

    f32         m_ActionDelayMin;
    f32         m_ActionDelayMax;

    f32         m_UsedActionDelayMin;
    f32         m_UsedActionDelayMax;

    f32         m_DelayTillNextAction;
    f32         m_TimeSinceLastFullAuto;

    f32         m_ScanTime;
    f32         m_bRolledOutBlocksLOFTime;
    f32         m_TimeSinceLastEnterCover;

    s32         m_PeekWeight;
    s32         m_ShootWeight;
    s32         m_GrenadeWeight;



    u32         m_HasClearGrenadeThrow:1,
                m_InCover:1,
                m_RolledOut:1,
                m_AutofireRequested:1,
                m_AutofireCanceled:1,
                m_DoAutofireDialog:1,
                m_bRolledOutBlocksLOF:1,
                m_bRolledInBlocksLOF:1;
};

#endif
