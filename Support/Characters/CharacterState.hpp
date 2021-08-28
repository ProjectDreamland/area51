// character_state : header file
/////////////////////////////////////////////////////////////////////////////

#ifndef __CHARACTERSTATE_HPP
#define __CHARACTERSTATE_HPP


//#include "Navigation\Nav_Map.hpp"
#include "AlertPackage.hpp"
#include "Conversation_Packet.hpp"
#include "loco\loco.hpp"

//=========================================================================
// PATCHVARIANT macro - this macros is used so from Visual Studio .NET we
// can auto fixup the character base references..
//=========================================================================

#ifndef TARGET_XBOX
    #define PATCHVARIANT(type,label) type& m_##label
#else
    #if _MSC_VER < 1300
        #define PATCHVARIANT(type,label) type& m_##label
    #else
        #define PATCHVARIANT(type,label) union  \
        {                                       \
            type* m_p##label;                   \
            type& m_##label;                    \
        };
    #endif
#endif

//=========================================================================
// CLASSES
//=========================================================================

const f32 k_MinAngleToMeleeFace = R_45;

//=========================================================================

class character;

// Base state class
class character_state : public prop_interface
{
// Enums
public:
// states
    enum states
    {
        STATE_NULL,
	    STATE_HOLD,

        STATE_IDLE,
        STATE_TRIGGER,
   
        STATE_ALERT,
        STATE_SEARCH,
    
        STATE_ATTACK,
        STATE_FLEE,
    
        STATE_DEATH,
        STATE_COVER,
    
        STATE_SURPRISE,
        STATE_FOLLOW,
        
        STATE_TURRET,
        STATE_ALARM,
    
        STATE_TOTAL
    };

    enum base_phases
    {
        PHASE_NONE = 0,
        PHASE_GOTO_NEAREST_CONNECTION,
        PHASE_DODGE_GRENADE_LEFT,
        PHASE_DODGE_GRENADE_RIGHT,
        PHASE_DODGE_GRENADE_RUN_AWAY,
        PHASE_GOTO_LEASH,
        PHASE_MESON_STUN,
        PHASE_PROJECTILE_ATTACHED, 
        PHASE_BASE_COUNT,
    };

    enum
    {
        NUM_PATH_NODES = 10
    };

    // Add/do what you want with this structure.
    // It's copied to the state as an optional parameter when calling
    // "character::SetupState(states State, character_state_info* pInfo )"
    struct character_state_info
    {
        // Data
        vector3             m_TargetPos;     // User position
        guid                m_Guid;          // User guid
        loco::anim_type     m_AnimType;      // Use animation
        loco::move_style    m_MoveStyle;   // Move style
        states              m_NextState;     // User state1
        u8                  m_bOverride:1,
                            m_bValid:1;    // TRUE if info is valid

        // Functions
        character_state_info()
        {
            Clear();
        }

        void Clear()
        {
            // Clear everything
            m_bValid    = FALSE;
            m_NextState = STATE_NULL;
            m_TargetPos.Zero();
            m_Guid      = 0;
            m_AnimType  = loco::ANIM_NULL;
            m_MoveStyle = loco::MOVE_STYLE_WALK;
            m_bOverride = FALSE;
        }
    };


// Functions
public:   
    character_state ( character& Character, states State );

    virtual void    OnInit          ( void )                        { }
    virtual void    OnEnter         ( void );
    virtual xbool   OnExit          ( void )                        { return TRUE; }
    virtual void    OnAdvance       ( f32 DeltaTime );  
    virtual void    OnThink         ( void );           
    virtual xbool   OnPain          ( const pain& Pain );
    virtual void    OnBeingShotAt   ( object::type Type , guid ShooterID );
    virtual void    OnHitByFriendly ( guid ShooterID  );
    virtual void    OnHitFriendly   ( guid FriendlyID );
    virtual xbool   OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags );
    virtual void    OnCoverChanged  ( guid NewCover ) { (void)NewCover; }
    virtual void    AutofireRequestSent( void )         {}
    virtual xbool   CanReload       ( void )            { return TRUE; }
//    virtual void    OnEntityProximity ( guid* EntityGuids , s32 EntityCount ) { (void)EntityGuids; (void)EntityCount; }    
    virtual void    OnEnumProp      ( prop_enum&    List )          { (void)List;             }
    virtual xbool   OnProperty      ( prop_query&   I    )          { (void)I; return FALSE; }
    virtual void            OnGrenadeAlert  ( alert_package& Package );
    virtual void    OnRequestCoverFire( alert_package& Package )    { (void)Package; }
    
    virtual const char*GetStateName ( void );
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );                        
    virtual f32     GetAimerBlendScale( void )                      { return 1.0f; }
    inline  s32     GetCurrentPhase ( void )                        { return m_CurrentPhase; }    

    // character state specific virtual functions

    // check if we want to be in a new phase
    virtual s32     UpdatePhase     ( f32 DeltaTime );
    // change to the new phase
    virtual void    ChangePhase     ( s32 newPhase );                
    // check if we want to be in a new state
    virtual states  UpdateState     ( f32 DeltaTime)        { (void)DeltaTime; return STATE_NULL; }
    // get called after all updates this tick.
    virtual void    PostUpdate      ( void );
   
            states  GetStateType    ( void )                { return m_State; }    
            void    SetStateType    ( states newType )      { m_State = newType; }    

    virtual xbool               IgnorePain      ( const pain& Pain ) { (void)Pain; return FALSE; }
    virtual xbool               IgnoreAlerts    ( void )            { return FALSE; }    
    virtual xbool               IgnoreSight     ( void )            { return FALSE; }
    virtual xbool               IgnoreSound     ( void )            { return FALSE; }    
    virtual xbool               IgnoreAttacks   ( void )            { return FALSE; }    

    virtual xbool               IgnoreFullBodyFlinches( void )            { return FALSE; }    
    virtual xbool               IgnoreFlinches  ( void )            { return FALSE; }    
    virtual xbool               IgnoreFalling   ( void )            { return FALSE; }
    virtual xbool               IsMeleeingPlayer( void )            { return FALSE; }
    
    virtual void                SetContext      ( void* pContext )  { (void)pContext; }
    
#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

            xbool               AllowStateSwitching( void )         { return m_AllowStateSwitching; }    
// Data
    
public:
    character_state_info    m_Info;    // State specific user info
    f32                     m_TimeInState;
    f32                     m_TimeInPhase;
    states                  m_State;   // Type of state
    loco::move_style        m_MoveStyle; // move style for this state
    character_state*        m_pNext;   // Next in list
protected:

    guid                    m_EvadeFromGuid;
    f32                     m_EvadeDelay;
    s32                     m_CurrentPhase;
    s32                     m_LastPhase;
    character&              m_CharacterBase;    
    u8                      m_TookPainThisTick:1,
                            m_ShotAtThisTick:1,
                            m_HitFriendlyThisTick:1,
                            m_HitByFriendlyThisTick:1,
                            m_WantsToEvade:1,
                            m_AllowStateSwitching:1;
};

#endif // __CHARACTERSTATE_HPP

