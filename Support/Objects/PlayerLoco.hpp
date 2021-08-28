//=========================================================================
//
//  PlayerLoco.cpp
//
//=========================================================================

#ifndef __PLAYER_LOCO_HPP__
#define __PLAYER_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"
#include "Loco\LocoIKSolver.hpp"


//=========================================================================
// PLAYER STATES
//=========================================================================

struct player_loco_play_anim : public loco_play_anim
{
                            player_loco_play_anim   ( loco& Loco );
    virtual void            OnEnter                 ( void );
};

struct player_loco_idle : public loco_idle
{
                            player_loco_idle        ( loco& Loco );
    virtual void            OnEnter                 ( void );
};

struct player_loco_move : public loco_move
{
                            player_loco_move        ( loco& Loco );
    virtual void            OnEnter                 ( void );
};

//=========================================================================
// PLAYER LOCO
//=========================================================================

class player_loco : public loco
{
// Defines
public:
    CREATE_RTTI( player_loco, loco, loco )

    // KSS -- TO ADD NEW WEAPON 
    // SB  -- Only if it can be used in multi-player!

    // Multi-player weapon animation types supported
    // NOTE: You only need to add to this list if a new set of 3rd person anims are needed!
    enum mp_weapon
    {
        MP_WEAPON_RFL,     // Covers smp, sniper, shotgun
        MP_WEAPON_PST,     // Covers eagle
        MP_WEAPON_MSN,     // Covers mason cannon
        MP_WEAPON_DUAL,    // Covers dual smp, dual eagle, dual shotgun
        MP_WEAPON_MUT,     // Covers mutant

        MP_WEAPON_COUNT
    };

    // Multi-player anims that are required per weapon type
    enum mp_anim
    {
        MP_ANIM_WALK_IDLE,
        MP_ANIM_WALK_IDLE_TURN_LEFT,
        MP_ANIM_WALK_IDLE_TURN_RIGHT,
        MP_ANIM_WALK_MOVE_FRONT,
        MP_ANIM_WALK_MOVE_LEFT,
        MP_ANIM_WALK_MOVE_BACK,
        MP_ANIM_WALK_MOVE_RIGHT,

        MP_ANIM_RUN_IDLE,
        MP_ANIM_RUN_IDLE_TURN_LEFT,
        MP_ANIM_RUN_IDLE_TURN_RIGHT,
        MP_ANIM_RUN_MOVE_FRONT,
        MP_ANIM_RUN_MOVE_LEFT,
        MP_ANIM_RUN_MOVE_BACK,
        MP_ANIM_RUN_MOVE_RIGHT,

        MP_ANIM_CROUCH_IDLE,
        MP_ANIM_CROUCH_IDLE_TURN_LEFT,
        MP_ANIM_CROUCH_IDLE_TURN_RIGHT,
        MP_ANIM_CROUCH_MOVE_FRONT,
        MP_ANIM_CROUCH_MOVE_LEFT,
        MP_ANIM_CROUCH_MOVE_BACK,
        MP_ANIM_CROUCH_MOVE_RIGHT,

        MP_ANIM_JUMP,
        MP_ANIM_GRENADE,
        MP_ANIM_MELEE,

        MP_ANIM_STAND_LEAN_LEFT,
        MP_ANIM_STAND_LEAN_RIGHT,
        MP_ANIM_CROUCH_LEAN_LEFT,
        MP_ANIM_CROUCH_LEAN_RIGHT,

        MP_ANIM_COUNT
    };


// Functions
public:
                        player_loco         ( void );
    virtual void        OnInit              ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL );
    virtual xbool       SetWeapon           ( inven_item InvenWeapon );
            void        UpdateAnims         ( f32               DeltaTime,
                                              xbool             bIsAirborn,
                                              xbool             bIsCrouching,
                                              f32               Lean );
                                              
#if !defined( CONFIG_RETAIL )
    // Renders debug info
    virtual void        RenderInfo          ( xbool bRenderLoco     = TRUE, 
                                              xbool bLabelLoco      = FALSE,
                                              xbool bRenderSkeleton = FALSE, 
                                              xbool bLabelSkeleton  = FALSE );
#endif // !defined( CONFIG_RETAIL )

// Private init functions
private:
    void                InitIK                      ( void );
    
    void                InitAnimIndices             ( void );
    mp_weapon           GetMPWeapon                 ( inven_item InvenWeapon );
    
    anim_type           GetMPReloadAnimType         ( inven_item InvenWeapon );
    const char*         GetMPReloadAnimName         ( inven_item InvenWeapon );
    
    anim_type           GetMPShootPrimaryAnimType   ( inven_item InvenWeapon );
    const char*         GetMPShootPrimaryAnimName   ( inven_item InvenWeapon );
    
    anim_type           GetMPShootSecondaryAnimType ( inven_item InvenWeapon );
    const char*         GetMPShootSecondaryAnimName ( inven_item InvenWeapon );
    
    const char*         GetMPWeaponName             ( mp_weapon MPWeapon );
    const char*         GetMPAnimName               ( mp_anim   MPAnim );
    
// Private data
protected:

    // Animation vars
    s16                             m_MPAnimIndex[MP_WEAPON_COUNT * MP_ANIM_COUNT];     // Anim indices for each weapon anims
    player_loco_play_anim           m_PlayAnim;             // Play anim state
    player_loco_idle                m_Idle;                 // Idle anim state
    player_loco_move                m_Move;                 // Move anim state
    inven_item                      m_CurrentWeaponAnims;   // Current weapon anims that are being used
    
    // IK vars (used to correctly position the left hand on the weapon)
    loco_ik_solver                  m_IKSolver;             // IK solver for left arm
    loco_ik_solver::bone_mapping    m_IKBoneMappings[3];    // 1)Hand  2)ForeArm  3)UpperArm
    loco_ik_solver::constraint      m_IKConstraints[4];     // 1)Weapon->Hand 2)Hand->LowerArm, 3)LowerArm->UpperArm 4)UpperArm->Shoulder
};

//=========================================================================
// END
//=========================================================================
#endif
