//=========================================================================
//
//  Mutant_Tank.hpp
//
//=========================================================================

#ifndef __MUTANT_TANK_HPP__
#define __MUTANT_TANK_HPP__


//=========================================================================
// INCLUDES
//=========================================================================

#include "Mutant_Tank_Loco.hpp"
#include "..\Character.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"

#include "Characters\BaseStates\Character_Idle_State.hpp"
#include "Characters\BaseStates\Character_Alert_State.hpp"
#include "Characters\BaseStates\Character_Search_State.hpp"
#include "Characters\BaseStates\Character_Attack_State.hpp"
#include "Characters\BaseStates\Character_Death_State.hpp"
#include "MutantTank_Attack_State.hpp"

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================
class super_destructible_obj;

//=========================================================================
// MUTANT_TANK CLASS
//=========================================================================

class mutant_tank : public character
{
// Real time type information
public:
    CREATE_RTTI( mutant_tank, character, actor );
    
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
// Defines
//=========================================================================
public:
    
    enum defines
    {
        SPORE_COUNT    = 6,     // Max # of spores on mutants back
        CANISTER_COUNT = 4,     // Max # of canisters
//        STARGATE_HOOK_COUNT = 2,// Max # of stargate_hooks
        HOP_POINT_COUNT = 4,    // Max # of hop points
        PARASITE_COUNT = 48,    // Max # of parasites in shield
    };

//=========================================================================
// Structures
//=========================================================================
public:

    // Spores on back of theta    
    struct spore
    {
        // Vars
        guid        m_Guid;             // Guid of spore
        s32         m_BoneName;         // Bone attached too
        s32         m_BoneIndex;        // Index of attached bone
        f32         m_FlashTimer;       // Flash timer
        fx_handle   m_DestroyedFx;      // Destroyed fx
        fx_handle   m_RegenerateFx;     // Regenerate fx
        fx_handle   m_HitFx;            // Hit fx
        
        // Functions
        spore()
        {
            m_Guid              = 0;
            m_BoneName          = -1;
            m_BoneIndex         = -1;
            m_FlashTimer        = 0.0f;
        }
    };
    
    // Canisters that theta jumps up to
    struct canister
    {
        // Vars
        guid    m_CenterGuid;   // Guid of canister center
        guid    m_AttachGuid;   // Guid of canister marker
        guid    m_TriggerGuid;  // Guid of trigger that smashes canister
        
        // Functions
        canister()
        {
            m_CenterGuid  = 0;
            m_AttachGuid  = 0;
            m_TriggerGuid = 0;        
        }
    };

/*    struct stargate_hooks
    {
        // Vars
        guid    m_AttachGuid;   // Guid of canister center

        // Functions
        stargate_hooks()
        {
            m_AttachGuid  = 0;
        }
    };*/

    // Parasite the shield is made of
    struct parasite
    {
        // Vars
        s32     m_iBone;            // Bone particle is attached to
        vector3 m_BonePos;          // Position in bone space
        radian3 m_BoneRot;          // Rotation in bone space
        guid    m_EmitterGuid;      // Guid of particle event emitter;
        guid    m_ParticleGuid;     // Guid of particle

        vector3 m_WorldPos;         // Position in world space
        xbool   m_bVisible;         // Visible status
        f32     m_Alpha;            // Alpha value
        
        // Functions
        parasite()
        {
            x_memset( this, 0, sizeof( this ) );
        }
    };


//=========================================================================
// Class functions
//=========================================================================
public:
                    mutant_tank ();
    virtual         ~mutant_tank();

//=========================================================================
// Private functions
//=========================================================================
private:
            void    InitFx                  ( rhandle<char>& hFx, fx_handle& Fx );
            void    KillFx                  ( fx_handle& Fx );
            
            void    SetFxActive             ( fx_handle& Fx, xbool bActive );
            void    RestartFx               ( fx_handle& Fx );
            void    MoveFx                  ( fx_handle& Fx, const matrix4& L2W );
            void    MoveFx                  ( fx_handle& Fx, object* pParent );
            void    MoveFx                  ( fx_handle& Fx, s32 iBone );
            void    AdvanceFx               ( fx_handle& Fx, f32 DeltaTime );
            void    RenderFx                ( fx_handle& Fx );
            
            void    InitFx                  ( void );
            void    KillFx                  ( void );
            
            void    MoveObject              ( guid Guid, s32 iBone );
            
            void    UpdateSporesAndFx       ( f32 DeltaTime );
            void    RenderFx                ( void );
            
//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:
    virtual void    OnEnumProp              ( prop_enum&    List );
    virtual xbool   OnProperty              ( prop_query&   I    );
    virtual void    KillMe                  ( void );

    virtual void    OnInit                  ( void );
    virtual void    AdvanceLoco             ( f32 DeltaTime );

    virtual s32     GetNumberVoiceActors    ( void )    { return 1; }
            void    LaunchParasite          ( const vector3& Pos );
            void    LaunchProjectile        ( const vector3& Pos );
            s32     GetCurrentAttackPhase   ( void );
    virtual void    OnEvent                 ( const event& Event );
    virtual void    OnPain                  ( const pain& Pain );
    virtual xbool   OnChildPain             ( guid ChildGuid, const pain& Pain );
    virtual void    PlayImpactAnim          ( const pain& Pain, eHitType HitType );
    virtual xbool   TakeDamage              ( const pain& Pain );
    virtual void    OnRender                ( void );
    virtual void    OnRenderTransparent     ( void );
    virtual void    OnAdvanceLogic          ( f32 DeltaTime );      
//    virtual guid    GetHopPoint             ( void );

    
    virtual const char*         GetLogicalName      ( void ) { return "THETA";}
    
            // Spore functions
            s32                     GetSporeCount           ( void ) const;
            super_destructible_obj* GetSporeObject          ( s32 iSpore );
            super_destructible_obj* GetRandomSporeObject    ( void );
            vector3                 GetRandomSporePosition  ( void );
            f32                     GetSporeHealth          ( s32 iSpore ) const;
            f32                     GetAllSporesHealth      ( void ) const;
            s32                     GetSporeIndex           ( guid SporeGuid ) const;
            s32                     GetClosestSpore         ( const vector3& Position, 
                                                                    f32      MaxDistSqr,
                                                                    f32      MinHealth );
            s32                     GetSporeWithHealth      ( s32 iHintSpore );
            xbool                   ApplyPainToSpore        ( s32 iSpore, const pain& Pain );
            void                    SetRegenerateSpores     ( xbool bRegenerate );
            
            // Grate functions
//            stargate_hooks&         GetGrateHook            ( s32 iHook );         
            guid                    GetGrateGuid            ( void )            { return m_GrateGuid; }            

            // Perch functions
//            stargate_hooks&         GetPerchHook            ( s32 iHook );         
            guid                    GetPerchGuid            ( void )            { return m_PerchGuid; }            
//            guid                    GetPerchEndGuid         ( void )            { return m_PerchEndGuid; }            

            guid                    GetPerchScript          ( void )            { return m_PerchScriptGuid; }
            guid                    GetGrateScript          ( void )            { return m_GrateScriptGuid; }

            // Canister functions
            s32                     GetCanisterCount        ( void ) const;
            canister&               GetCanister             ( s32 iCanister );
            s32                     GetClosestCanister      ( const vector3& Position, 
                                                                    f32      MinDistSqr,
                                                                    f32      MaxDistSqr ) const;
/*            s32                     GetClosestGrateHook     ( const vector3& Position, 
                                                                    f32      MinDistSqr,
                                                                    f32      MaxDistSqr ) const;
                                                                    
            s32                     GetClosestPerchHook     ( const vector3& Position, 
                                                                    f32      MinDistSqr,
                                                                    f32      MaxDistSqr ) const;*/
            xbool                   GetWithinGrateHoppointRadius( void );
            guid                    GetGrateHoppoint            ( void )    { return m_GrateHoppoint; }
            xbool                   GetWithinPerchHoppointRadius( void );
            guid                    GetPerchHoppoint            ( void )    { return m_PerchHoppoint; }

            // Jump functions
            void                    BeginJump               (        loco::anim_type AnimType, 
                                                               const vector3&        EndPos,
                                                                     radian          EndYaw,
                                                                     xbool           bInterpYaw,
                                                                     xbool           bInterpVert );
            
            void                    BeginJump               ( loco::anim_type AnimType, 
                                                              guid            DestMarkerGuid,
                                                              xbool           bInterpYaw,
                                                              xbool           bInterpVert );
                                                              
            xbool                   UpdateJump              ( void );
            
            // Parasite shield functions
            void                    InitParasiteShield              ( void );
            void                    UpdateParasiteShield            ( f32 DeltaTime );
            guid                    ScanForPlayerProjectile         ( void );
            f32                     GetParasiteShieldPercent        ( void );
            void                    RegenParasiteShield             ( f32 Percent );
            void                    CreateParasiteShieldContagion   ( void );
            void                    CreateContagion                 ( const vector3& Position, 
                                                                      const radian3& Rotation,
                                                                      const vector3& Velocity,
                                                                      const char*    pPainHandle,
                                                                            f32      AttackTime,
                                                                            guid     TargetGuid );
                                                                                                                                   
//=========================================================================
// Data
//=========================================================================
protected:
    void                InitializeDeathState    ( const pain& Pain, geom::bone::hit_location hitLocation );

    // Locomotion
    mutant_tank_loco            m_Loco;

    // States
    character_idle_state        m_Idle;
    character_alert_state       m_Alert;
    character_search_state      m_Search;
    mutanttank_attack_state     m_Attack;
    character_death_state       m_Death;

    // Templates
    s32                         m_ProjectileTemplateID;         // Blue-print of meson projectile
    s32                         m_ParasiteTemplateID;           // Blue-print of parasite
    
    // Spore vars
    spore                       m_Spores[SPORE_COUNT];          // Array of spores
    xbool                       m_bRegenerateSpores;            // Spores regenerate state
    
    // Shield fx vars
    rhandle<char>               m_hShieldFx;                    // Handle to shield fx
    s32                         m_ShieldFxBoneName;             // Bone attached too
    s32                         m_ShieldFxBoneIndex;            // Index of attached bone
    fx_handle                   m_ShieldFx;                     // Shield fx
    
    // Spore fx vars
    rhandle<char>               m_hSporeHitFx;                  // Handle to spore being hit fx
    rhandle<char>               m_hSporeDestroyedFx;            // Handle to destroyed fx
    rhandle<char>               m_hSporeRegenerateFx;           // Handle to regenerate fx
    
    // Pain vars
    f32                         m_PainLightTimer;               // Timer for light pain anim
    f32                         m_PainHardTimer;                // Timer for hard pain anim
    guid                        m_PainOrigin;                   // Last pain origin
    s32                         m_PainFrame;                    // Last pain receive frame
    
    // Canister vars
    canister                    m_Canisters[CANISTER_COUNT];    // Array of canisters

    // stargate vars
//    stargate_hooks              m_PerchHooks[STARGATE_HOOK_COUNT];  // Array of stargate_hooks
//    stargate_hooks              m_GrateHooks[STARGATE_HOOK_COUNT];  // Array of stargate_hooks
//    guid                        m_HopPoints[HOP_POINT_COUNT];       // Array of points to hop to
//    guid                        m_HopPointCenter;

    guid                        m_PerchGuid;
    guid                        m_PerchHoppoint;
    f32                         m_PerchHoppointRadius;

    guid                        m_GrateGuid;
    guid                        m_GrateHoppoint;
    f32                         m_GrateHoppointRadius;

    guid                        m_PerchScriptGuid;
    guid                        m_GrateScriptGuid;
    
    // Jump vars
    s32                         m_iJumpAnim;                    // Current jump anim
    vector3                     m_JumpStartPos;                 // Start position of jump
    radian                      m_JumpStartYaw;                 // Start yaw of jump
    vector3                     m_JumpEndPos;                   // End position for jump
    radian                      m_JumpEndYaw;                   // End yaw for jump
    xbool                       m_bJumpInterpYaw;               // Should yaw be interpolated?
    xbool                       m_bJumpInterpVert;              // Should vertical be interpolated?

    // Parasite shield vars    
    parasite                    m_Parasites[PARASITE_COUNT];    // Array of parasites
    s32                         m_nParasites;                   // # of parasites
    f32                         m_ParasiteShieldPercent;        // Current shield percentage
    vector3                     m_ParasiteRegenSite;            // Next regeneration site
    
    // 
    fx_handle                   m_FXHandle;                     // Shield hit effect.
    s32                         m_ShieldHitBoneIndex;
};

//=========================================================================


#endif
