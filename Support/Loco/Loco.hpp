//=========================================================================
//
//  Loco.hpp
//
//=========================================================================
#ifndef __LOCO_HPP__
#define __LOCO_HPP__

//=========================================================================
// INLCLUDES
//=========================================================================
#include "LocoCharAnimPlayer.hpp"
#include "LocoAimController.hpp"
#include "LocoMaskController.hpp"
#include "LocoAdditiveController.hpp"
#include "LocoLipSyncController.hpp"
#include "LocoEyeController.hpp"
#include "Locomotion/CharacterPhysics.hpp"
#include "Inventory\Inventory2.hpp"


//=========================================================================
// SPECIAL COMPILER SWITCHES
//=========================================================================

#ifdef TARGET_PC
// Get rid of "warning C4355: 'this' used in base member initializer list"
#pragma warning( disable : 4355 ) 
#endif


//=========================================================================
// CLASSES
//=========================================================================
class loco ;
class loco_state ;
class skin_inst;

//=========================================================================
// LOCOMOTION CLASS
//=========================================================================
class loco : public prop_interface
{
public:

    CREATE_RTTI_BASE( object );


//=========================================================================
// Defines
//=========================================================================

    // List of available states
    enum state
    {
        STATE_NULL = -1,    // Undefined

        STATE_IDLE,         // Idle and turning
        STATE_MOVE,         // Moving
        STATE_PLAY_ANIM,    // Playing an animation

        STATE_TOTAL
    };

    // List of motions
    enum motion
    {
        // NOTE: Do not change the order of FORWARD/LEFT/BACK/RIGHT or you'll break the locomotion!
        MOTION_FRONT,
        MOTION_LEFT,
        MOTION_BACK,
        MOTION_RIGHT,

        MOTION_IDLE,
        MOTION_IDLE_TURN_LEFT,
        MOTION_IDLE_TURN_RIGHT,
        MOTION_IDLE_TURN_180,

        MOTION_TRANSITION,
        MOTION_NULL,
    };

    // Sets type of movement
    // NOTE: See SteveB if you want to add to this list
    enum move_style
    {
        MOVE_STYLE_NULL = -1,

        MOVE_STYLE_WALK,
        MOVE_STYLE_RUN,
        MOVE_STYLE_RUNAIM,
        MOVE_STYLE_PROWL,
        MOVE_STYLE_CROUCH,
        MOVE_STYLE_CROUCHAIM,
        MOVE_STYLE_CHARGE,
        MOVE_STYLE_CHARGE_FAST,
        
        MOVE_STYLE_COUNT
    } ;

    // NOTE: You MUST add the anim set defines to the "anim_type" enum 
    //       list if you add to this enum list. 
    //       (See SteveB if you want to add to this list)
    enum move_style_anim
    {
        MOVE_STYLE_ANIM_NULL = -1,    // -1 So we can use "ANIM_TYPE_COUNT * MoveStyle"
        
        // NOTE: These defines should match the defines for a style!
        MOVE_STYLE_ANIM_IDLE,
        MOVE_STYLE_ANIM_IDLE_TURN_LEFT,
        MOVE_STYLE_ANIM_IDLE_TURN_RIGHT,
        MOVE_STYLE_ANIM_IDLE_TURN_180,
        MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180,
        MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180,
        MOVE_STYLE_ANIM_IDLE_FIDGET,
        MOVE_STYLE_ANIM_MOVE_FRONT,
        MOVE_STYLE_ANIM_MOVE_LEFT,
        MOVE_STYLE_ANIM_MOVE_BACK,
        MOVE_STYLE_ANIM_MOVE_RIGHT,
        MOVE_STYLE_ANIM_COUNT
    };

    // List of bone mask types
    enum bone_masks_type
    {
        BONE_MASKS_TYPE_FULL_BODY,
        BONE_MASKS_TYPE_UPPER_BODY,
        BONE_MASKS_TYPE_FACE,
        BONE_MASKS_TYPE_AIM_VERT,
        BONE_MASKS_TYPE_AIM_HORIZ,
        BONE_MASKS_TYPE_NO_AIM_VERT,
        BONE_MASKS_TYPE_NO_AIM_HORIZ,
        BONE_MASKS_TYPE_RELOAD_SHOOT,
        
        BONE_MASKS_TYPE_COUNT
    };
    
    // List of animation types that map to an index
    enum anim_type
    {
        ANIM_NULL = -1,

        // NOTE: Each move style, must match the "move_style_anim" enums!
        //       See SteveB if you want to add to this list

        // "MOVE_STYLE_WALK" style anims
        ANIM_WALK_IDLE,
        ANIM_WALK_IDLE_TURN_LEFT,
        ANIM_WALK_IDLE_TURN_RIGHT,
        ANIM_WALK_IDLE_TURN_180,
        ANIM_WALK_IDLE_TURN_LEFT_180,
        ANIM_WALK_IDLE_TURN_RIGHT_180,
        ANIM_WALK_IDLE_FIDGET,
        ANIM_WALK_MOVE_FRONT,
        ANIM_WALK_MOVE_LEFT,
        ANIM_WALK_MOVE_BACK,
        ANIM_WALK_MOVE_RIGHT,

        // "MOVE_STYLE_RUN" style anims
        ANIM_RUN_IDLE,
        ANIM_RUN_IDLE_TURN_LEFT,
        ANIM_RUN_IDLE_TURN_RIGHT,
        ANIM_RUN_IDLE_TURN_180,
        ANIM_RUN_IDLE_TURN_LEFT_180,
        ANIM_RUN_IDLE_TURN_RIGHT_180,
        ANIM_RUN_IDLE_FIDGET,
        ANIM_RUN_MOVE_FRONT,
        ANIM_RUN_MOVE_LEFT,
        ANIM_RUN_MOVE_BACK,
        ANIM_RUN_MOVE_RIGHT,

        // "MOVE_STYLE_RUNAIM" style anims
        ANIM_RUNAIM_IDLE,
        ANIM_RUNAIM_IDLE_TURN_LEFT,
        ANIM_RUNAIM_IDLE_TURN_RIGHT,
        ANIM_RUNAIM_IDLE_TURN_180,
        ANIM_RUNAIM_IDLE_TURN_LEFT_180,
        ANIM_RUNAIM_IDLE_TURN_RIGHT_180,
        ANIM_RUNAIM_IDLE_FIDGET,
        ANIM_RUNAIM_MOVE_FRONT,
        ANIM_RUNAIM_MOVE_LEFT,
        ANIM_RUNAIM_MOVE_BACK,
        ANIM_RUNAIM_MOVE_RIGHT,

        // "PROWL" style anims
        ANIM_PROWL_IDLE,
        ANIM_PROWL_IDLE_TURN_LEFT,
        ANIM_PROWL_IDLE_TURN_RIGHT,
        ANIM_PROWL_IDLE_TURN_180,
        ANIM_PROWL_IDLE_TURN_LEFT_180,
        ANIM_PROWL_IDLE_TURN_RIGHT_180,
        ANIM_PROWL_IDLE_FIDGET,
        ANIM_PROWL_MOVE_FRONT,
        ANIM_PROWL_MOVE_LEFT,
        ANIM_PROWL_MOVE_BACK,
        ANIM_PROWL_MOVE_RIGHT,

        // "MOVE_STYLE_CROUCH" style anims
        ANIM_CROUCH_IDLE,
        ANIM_CROUCH_IDLE_TURN_LEFT,
        ANIM_CROUCH_IDLE_TURN_RIGHT,
        ANIM_CROUCH_IDLE_TURN_180,
        ANIM_CROUCH_IDLE_TURN_LEFT_180,
        ANIM_CROUCH_IDLE_TURN_RIGHT_180,
        ANIM_CROUCH_IDLE_FIDGET,
        ANIM_CROUCH_MOVE_FRONT,
        ANIM_CROUCH_MOVE_LEFT,
        ANIM_CROUCH_MOVE_BACK,
        ANIM_CROUCH_MOVE_RIGHT,
        
        // "MOVE_STYLE_CROUCH" style anims
        ANIM_CROUCHAIM_IDLE,
        ANIM_CROUCHAIM_IDLE_TURN_LEFT,
        ANIM_CROUCHAIM_IDLE_TURN_RIGHT,
        ANIM_CROUCHAIM_IDLE_TURN_180,
        ANIM_CROUCHAIM_IDLE_TURN_LEFT_180,
        ANIM_CROUCHAIM_IDLE_TURN_RIGHT_180,
        ANIM_CROUCHAIM_IDLE_FIDGET,
        ANIM_CROUCHAIM_MOVE_FRONT,
        ANIM_CROUCHAIM_MOVE_LEFT,
        ANIM_CROUCHAIM_MOVE_BACK,
        ANIM_CROUCHAIM_MOVE_RIGHT,

        // "MOVE_STYLE_CHARGE" style anims
        ANIM_CHARGE_IDLE,
        ANIM_CHARGE_IDLE_TURN_LEFT,
        ANIM_CHARGE_IDLE_TURN_RIGHT,
        ANIM_CHARGE_IDLE_TURN_180,
        ANIM_CHARGE_IDLE_TURN_LEFT_180,
        ANIM_CHARGE_IDLE_TURN_RIGHT_180,
        ANIM_CHARGE_IDLE_FIDGET,
        ANIM_CHARGE_MOVE_FRONT,
        ANIM_CHARGE_MOVE_LEFT,
        ANIM_CHARGE_MOVE_BACK,
        ANIM_CHARGE_MOVE_RIGHT,

        // "MOVE_STYLE_CHARGE_FAST" style anims
        ANIM_CHARGE_FAST_IDLE,
        ANIM_CHARGE_FAST_IDLE_TURN_LEFT,
        ANIM_CHARGE_FAST_IDLE_TURN_RIGHT,
        ANIM_CHARGE_FAST_IDLE_TURN_180,
        ANIM_CHARGE_FAST_IDLE_TURN_LEFT_180,
        ANIM_CHARGE_FAST_IDLE_TURN_RIGHT_180,
        ANIM_CHARGE_FAST_IDLE_FIDGET,
        ANIM_CHARGE_FAST_MOVE_FRONT,
        ANIM_CHARGE_FAST_MOVE_LEFT,
        ANIM_CHARGE_FAST_MOVE_BACK,
        ANIM_CHARGE_FAST_MOVE_RIGHT,

        // Cover Anims
        ANIM_COVER_IDLE,
        ANIM_COVER_SHOOT,
        ANIM_COVER_PEEK,
        ANIM_COVER_GRENADE,
        ANIM_COVER_COVERINGFIRE,
        
        // Evade anims
        ANIM_EVADE_LEFT,
        ANIM_EVADE_RIGHT,
        ANIM_GRENADE_EVADE_LEFT,
        ANIM_GRENADE_EVADE_RIGHT,

        // Grenade animations
        ANIM_GRENADE_THROW_LONG,
        ANIM_GRENADE_THROW_SHORT,
        ANIM_GRENADE_THROW_OVER_OBJECT,
        
        // Melee animations
        ANIM_MELEE_BACK_LEFT,
        ANIM_MELEE_BACK_RIGHT,
        ANIM_MELEE_SHORT,
        ANIM_MELEE_LONG,
        ANIM_MELEE_LEAP,

        // Misc animations
        ANIM_SPOT_TARGET,
        ANIM_HEAR_TARGET,
        ANIM_LOST_TARGET,
        ANIM_ADD_REACT_RAGE,
        ANIM_FACE_IDLE,
        ANIM_DRAIN_LIFE,
        ANIM_REQUEST_COVER,
        ANIM_REQUEST_ATTACK,
        ANIM_RESPONSE,
        
        // Death animations
        ANIM_DEATH,
        ANIM_DEATH_SIMPLE,

        ANIM_DEATH_HARD_SHOT_IN_BACK_HIGH,
        ANIM_DEATH_HARD_SHOT_IN_BACK_MED,
        ANIM_DEATH_HARD_SHOT_IN_BACK_LOW,

        ANIM_DEATH_HARD_SHOT_IN_FRONT_HIGH,
        ANIM_DEATH_HARD_SHOT_IN_FRONT_MED,
        ANIM_DEATH_HARD_SHOT_IN_FRONT_LOW,

        ANIM_DEATH_LIGHT_SHOT_IN_BACK_HIGH,
        ANIM_DEATH_LIGHT_SHOT_IN_BACK_MED,
        ANIM_DEATH_LIGHT_SHOT_IN_BACK_LOW,

        ANIM_DEATH_LIGHT_SHOT_IN_FRONT_HIGH,
        ANIM_DEATH_LIGHT_SHOT_IN_FRONT_MED,
        ANIM_DEATH_LIGHT_SHOT_IN_FRONT_LOW,
        
        ANIM_DEATH_CROUCH,
        ANIM_DEATH_EXPLOSION,

        // Toss weapon
        ANIM_TOSS_WEAPON,

        // Reload animations
        ANIM_RELOAD,
        ANIM_RELOAD_SMP,
        ANIM_RELOAD_DUAL_SMP,
        ANIM_RELOAD_DUAL_SHT,
        ANIM_RELOAD_SNIPER,
        ANIM_RELOAD_SHOTGUN,
        ANIM_RELOAD_GAUSS,
        ANIM_RELOAD_DESERT_EAGLE,
        ANIM_RELOAD_MSN,
        ANIM_RELOAD_BBG,
        ANIM_RELOAD_TRA,
        ANIM_RELOAD_SCN,

        // Shoot animations
        ANIM_SHOOT,
        ANIM_SHOOT_SECONDARY,
        ANIM_SHOOT_SMP,
        ANIM_SHOOT_DUAL_SMP,
        ANIM_SHOOT_DUAL_SHT,
        ANIM_SHOOT_DUAL_SHT_SECONDARY,
        ANIM_SHOOT_SNIPER,

// KSS -- TO ADD NEW WEAPON ??
        ANIM_SHOOT_SHOTGUN,

        ANIM_SHOOT_GAUSS,
        ANIM_SHOOT_DESERT_EAGLE,
        ANIM_SHOOT_MHG,
        ANIM_SHOOT_MSN,
        ANIM_SHOOT_BBG,        
        ANIM_SHOOT_TRA,
        ANIM_SHOOT_MUTANT,
        ANIM_SHOOT_SCN,

        ANIM_SHOOT_SECONDARY_SMP,
        ANIM_SHOOT_SECONDARY_SNIPER,

// KSS -- TO ADD NEW WEAPON
        ANIM_SHOOT_SECONDARY_SHOTGUN,

        ANIM_SHOOT_SECONDARY_GAUSS,
        ANIM_SHOOT_SECONDARY_DESERT_EAGLE,
        ANIM_SHOOT_SECONDARY_MHG,
        ANIM_SHOOT_SECONDARY_MSN,
        ANIM_SHOOT_SECONDARY_MUTANT,
        ANIM_SHOOT_SECONDARY_SCN,
        
        // Idle Shoot animations
// KSS -- TO ADD NEW WEAPON
        ANIM_SHOOT_IDLE_SMP,
        ANIM_SHOOT_IDLE_SNIPER,
        ANIM_SHOOT_IDLE_SHOTGUN,
        ANIM_SHOOT_IDLE_GAUSS,
        ANIM_SHOOT_IDLE_DESERT_EAGLE,
        ANIM_SHOOT_IDLE_MHG,
        ANIM_SHOOT_IDLE_MSN,
        ANIM_SHOOT_IDLE_BBG,
        ANIM_SHOOT_IDLE_SCN,
        ANIM_SHOOT_IDLE_TRA,
        ANIM_SHOOT_IDLE_MUTANT,

        // Idle Crounching Shoot animations
// KSS -- TO ADD NEW WEAPON
        ANIM_SHOOT_CROUCH_IDLE_SMP,
        ANIM_SHOOT_CROUCH_IDLE_SNIPER,
        ANIM_SHOOT_CROUCH_IDLE_SHOTGUN,
        ANIM_SHOOT_CROUCH_IDLE_GAUSS,
        ANIM_SHOOT_CROUCH_IDLE_DESERT_EAGLE,
        ANIM_SHOOT_CROUCH_IDLE_MHG,
        ANIM_SHOOT_CROUCH_IDLE_MSN,
        ANIM_SHOOT_CROUCH_IDLE_BBG,
        ANIM_SHOOT_CROUCH_IDLE_SCN,
        ANIM_SHOOT_CROUCH_IDLE_TRA,
        ANIM_SHOOT_CROUCH_IDLE_MUTANT,

        // Damage animations
        ANIM_DAMAGE_STEP_BACK,
        ANIM_DAMAGE_STEP_FORWARD,
        ANIM_DAMAGE_STEP_LEFT,
        ANIM_DAMAGE_STEP_RIGHT,
        
        ANIM_DAMAGE_SHOCK,
        ANIM_DAMAGE_PARASITE,

        ANIM_DAMAGE_PLAYER_MELEE_0,
        ANIM_MESON_STUN,

        // idle pain anims
        ANIM_PAIN_IDLE_FRONT,
        ANIM_PAIN_IDLE_BACK,
        
        ANIM_PROJECTILE_ATTACHED,

        // Additive impact anims
        ANIM_ADD_IMPACT_HEAD_FRONT,
        ANIM_ADD_IMPACT_HEAD_BACK,
        ANIM_ADD_IMPACT_TORSO_FRONT,
        ANIM_ADD_IMPACT_TORSO_BACK,
        ANIM_ADD_IMPACT_SHOULDER_LEFT_FRONT,
        ANIM_ADD_IMPACT_SHOULDER_RIGHT_FRONT,
        ANIM_ADD_IMPACT_SHOULDER_LEFT_BACK,
        ANIM_ADD_IMPACT_SHOULDER_RIGHT_BACK,
        
        // SB - These are used by the frienldy scientist - they need
        //      to be made into a common naming scheme like the above defines...
        ANIM_UA_HEAD_SHAKE,
        ANIM_UA_HEAD_NOD,
        ANIM_UA_CONVERSATION1,
        ANIM_UA_CONVERSATION2,
        ANIM_UA_CONVERSATION3,
        ANIM_UA_CHAIR_SEATED_LOOKAROUND,
        ANIM_UA_CHAIR_SEATED_IDLE,
        ANIM_UA_COME_HERE,
        ANIM_UA_COWER_TO_DEATH,
        ANIM_HAND_SIGNAL_ENEMY_FORWARD,

        // Mutant tank (Theta) specific anims
        ANIM_ATTACK_HOWL,
        ANIM_ATTACK_CLAW,
        ANIM_ATTACK_CHARGE_SWING,
        ANIM_ATTACK_CHARGE_MISS,
        ANIM_ATTACK_RANGED_ATTACK,
        ANIM_ATTACK_BUBBLE,
        ANIM_STAGE0_RAGE,
        ANIM_STAGE1_RAGE,
        ANIM_STAGE2_RAGE,
        ANIM_STAGE3_RAGE,
        ANIM_CANISTER_TO,
        ANIM_CANISTER_IDLE,
        ANIM_CANISTER_SMASH,
        ANIM_CANISTER_FROM,
        ANIM_SHIELD_ON,
        ANIM_SHIELD_SHOOT,
        ANIM_SHIELD_REGEN,

        ANIM_GRATE_TO,
        ANIM_GRATE_SMASH,
        ANIM_GRATE_FROM,
        ANIM_PERCH_TO,
        ANIM_PERCH_FROM,
        ANIM_THETA_CROUCH,
        ANIM_THETA_JUMP,

        // SB - These are not used by any current animations but are here
        //      so the old code compiles...
        ANIM_EMOTION1,
        ANIM_EMOTION2,
        ANIM_ALRT_SMP_FRENZY,
        ANIM_ALRT_SHT_FRENZY,
        ANIM_CONVULSE1,

        // Lip sync anims
        ANIM_LIP_SYNC_TEST,

        // Jumping anims
        ANIM_JUMP_OVER,
        ANIM_JUMP_UP,
        ANIM_JUMP_DOWN,
        ANIM_JUMP,
        
        // Misc multi-player anims
        ANIM_FALL,
        ANIM_GRENADE,
        ANIM_MELEE,
        
        ANIM_CROUCH_ENTER,
        ANIM_CROUCH_EXIT,
        
        ANIM_STAND_LEAN_LEFT,
        ANIM_STAND_LEAN_RIGHT,
        
        ANIM_CROUCH_LEAN_LEFT,
        ANIM_CROUCH_LEAN_RIGHT,

        // END OF WHOLE LIST ENUM
        ANIM_TOTAL,
    } ;

    // Animation flags
    enum amim_flags
    {
        //--------------------------------------------------------------------------------------------
        // Type flags used by "PlayAnim" functions
        //--------------------------------------------------------------------------------------------

        // If SET, the loco will play the animation for the specified number of cycles
        // (DEFAULT if no play type flags are set)
        ANIM_FLAG_PLAY_TYPE_CYCLIC          = ( 1 << 0 ),

        // If SET, the loco will play the animation for the specified number of seconds
        ANIM_FLAG_PLAY_TYPE_TIMED           = ( 1 << 1 ),

        // All the type flags combines
        // NOTE: Update me when any new type flags are added/modified
        ANIM_FLAG_PLAY_TYPE_ALL = (ANIM_FLAG_PLAY_TYPE_CYCLIC | ANIM_FLAG_PLAY_TYPE_TIMED),

        //--------------------------------------------------------------------------------------------
        // State flags used by "PlayAnim" functions only
        //--------------------------------------------------------------------------------------------

        // If SET, the loco will resume the previous state upon completion of animation playback
        // (DEFAULT if no end state flags are set)
        ANIM_FLAG_END_STATE_RESUME          = ( 1 << 2 ),
        
        // If SET, the loco will remain in the STATE_PLAY_ANIM upon completion of animation playback
        ANIM_FLAG_END_STATE_HOLD            = ( 1 << 3 ),

        // NOTE: Update me when any new state flags are added/modified
        ANIM_FLAG_END_STATE_ALL = (ANIM_FLAG_END_STATE_RESUME | ANIM_FLAG_END_STATE_HOLD),


        //--------------------------------------------------------------------------------------------
        // Misc flags used by "PlayAnim" functions only
        //--------------------------------------------------------------------------------------------

        // If SET, the animation play will begin immediately, even if there is a blend currently happending
        // (which will cause a pop). If NOT SET, the animation will play as soon any current blend has ended -
        // if there is a blend currently happending, then the anim is queued up (NOTE: only the last request is queued)
        // The only reason I could see this being SET is for death animations from explosions etc - 
        // for all other cases, the blend is so fast and smooth, that you will not notice any delay
        ANIM_FLAG_INTERRUPT_BLEND           = ( 1 << 4 ),

        // If SET, the controller blend between the blend anim and the current anims goes the longest route
        ANIM_FLAG_REVERSE_YAW_BLEND         = ( 1 << 5 ),
        
        // If SET, the aimer controller is blended out over a short time,
        // then blended back in at the very end of the animation
        ANIM_FLAG_TURN_OFF_AIMER            = ( 1 << 6 ),

        // If SET, we will not blend into this anim even if 
        // a blend time is set on the anim info
        ANIM_FLAG_DO_NO_BLENDING            = ( 1 << 7 ),

        //--------------------------------------------------------------------------------------------
        // Misc flags used by all functions
        //--------------------------------------------------------------------------------------------

        // If SET, and same animation is playing, the animation will be restarted from the beginning
        ANIM_FLAG_RESTART_IF_SAME_ANIM      = ( 1 << 8 ),
        
        // If SET, the new animation is started from the current frame position
        ANIM_FLAG_START_ON_SAME_FRAME       = ( 1 << 9 ),

        // Applies to when there are multiple animations with the same name.
        // If SET and play anim by name is used, the first animation in the list will be played.
        // If SET and play anim by index is used, the specified index will be played
        // If NOT SET, then a random animation from all animations of the same name will be played
        ANIM_FLAG_TURN_OFF_RANDOM_SELECTION = ( 1 << 10 ),

        // These are user flags that you can use for priority testing in your AI code
        ANIM_FLAG_PRIORITY_LOW              = ( 1 << 11 ),
        ANIM_FLAG_PRIORITY_MEDIUM           = ( 1 << 12 ),
        ANIM_FLAG_PRIORITY_HIGH             = ( 1 << 13 ),

        //--------------------------------------------------------------------------------------------
        // Mask type flags used by "PlayLipSyncAnim" functions only
        //--------------------------------------------------------------------------------------------

        // If SET, playback will be full body
        // (DEFAULT if no mask flags are set)
        ANIM_FLAG_MASK_TYPE_FULL_BODY       = ( 1 << 14 ),

        // If SET, the whole upper body gets the animation
        ANIM_FLAG_MASK_TYPE_UPPER_BODY      = ( 1 << 15 ),

        // If SET, just the face gets the animation
        ANIM_FLAG_MASK_TYPE_FACE            = ( 1 << 16 ),

        // If SET, just the face gets the animation
        ANIM_FLAG_MASK_TYPE_DYNAMIC         = ( 1 << 17 ),

        // NOTE: Update me when any new mask flags are added/modified
        ANIM_FLAG_MASK_TYPE_ALL = (ANIM_FLAG_MASK_TYPE_FULL_BODY | ANIM_FLAG_MASK_TYPE_UPPER_BODY | ANIM_FLAG_MASK_TYPE_FACE | ANIM_FLAG_MASK_TYPE_DYNAMIC ),

        //--------------------------------------------------------------------------------------------
        // Controller flags used by "PlayAdditiveAnim" functions only
        //--------------------------------------------------------------------------------------------

        // These specify which controller to use (defaults to controller0 if no controller flags are set).
        // There are currently 2 controllers.
        ANIM_FLAG_CONTROLLER0               = ( 1 << 18 ),
        ANIM_FLAG_CONTROLLER1               = ( 1 << 19 ),
        
        //--------------------------------------------------------------------------------------------
        // Audio flags used by "PlayLipSync" functions only
        //--------------------------------------------------------------------------------------------

        // Tells lip sync controller playback is in artist viewer so that
        // audio can be triggered when event is reached (if present)
        ANIM_FLAG_ARTIST_VIEWER             = ( 1 << 20 ),
        
        // Tells lip sync controller that this came from a cinema and not to turn the voice off
        ANIM_FLAG_CINEMA                    = ( 1 << 21 ),
        
        // Turns cinema relative mode on
        ANIM_FLAG_CINEMA_RELATIVE_MODE      = ( 1 << 22 ),
        
        // Turns cover relative mode on
        ANIM_FLAG_COVER_RELATIVE_MODE       = ( 1 << 23 ),
    } ;

//=========================================================================
// Structures
//=========================================================================
public:
    // Animation indices and blend times for move style
    struct move_style_info_default
    {
        move_style_info_default();

        // Anim group
        anim_group::handle  m_hAnimGroup;           // Animation group

        // Animations
        s16     m_iAnims[ MOVE_STYLE_ANIM_COUNT ];  // Idle and move animation indices

        // Blend related
        f32     m_IdleBlendTime ;                   // Blend times when in idle state
        f32     m_MoveBlendTime ;                   // Blend times when in move state
        f32     m_FromPlayAnimBlendTime ;           // Blend time after playing a generic animation
        
        // Turning info                             
        radian  m_MoveTurnRate ;                    // Max delta yaw when moving and turning
    };

    struct move_style_info
    {
        // Anim group
        anim_group::handle  m_hAnimGroup;           // Animation group
        
        // Animations
        s16     m_iAnims[ MOVE_STYLE_ANIM_COUNT ];  // Idle and move animation indices

        // Blend related
        f32     m_IdleBlendTime ;                   // Blend times when in idle state
        f32     m_MoveBlendTime ;                   // Blend times when in move state
        f32     m_FromPlayAnimBlendTime ;           // Blend time after playing a generic animation
        
        // Turning info                             
        radian  m_MoveTurnRate ;                    // Max delta yaw when moving and turning

        // Misc
        f32     m_AimerBlendSpeed ;                 // Controls aim blending speed (default 1.0f)
        radian  m_IdleDeltaYawMin ;                 // Min delta yaw threshold before playing idle anim
        radian  m_IdleDeltaYawMax ;                 // Max delta yaw threshold before playing idle anim
        radian  m_IdleTurnDeltaYawMin ;             // Min delta yaw threshold before playing idle turn anim
        radian  m_IdleTurnDeltaYawMax ;             // Max delta yaw threshold before playing idle turn anim
        radian  m_IdleTurn180DeltaYawMin ;          // Min delta yaw threshold before playing idle turn 180 anim
        radian  m_IdleTurn180DeltaYawMax ;          // Max delta yaw threshold before playing idle turn 180 anim

        // Constructor (in loco.cpp)
        move_style_info() ;

        // Initializer
        void    InitDefaults        ( move_style_info_default& Defaults );
    } ;

    

    // Animation lookup table. Maps type to actual index
    struct anim_lookup_table
    {
        // List of animation indices
        s16 m_Index[ANIM_TOTAL] ;

        // Constructor
        anim_lookup_table()
        {
            Clear();
        }

        // Clear all entries
        void Clear( void )
        {
            // Set all anims to -1
            x_memset(this, -1, sizeof(*this)) ;
        }
    } ;

    // Simple structure used to setup anim indices
    struct anim_lookup
    {
              anim_type m_AnimType ;    // Loco enum type
        const char*     m_pName ;       // Name of animation
    } ;

    // Simple structure used to setup bone masks
    struct bone_lookup
    {
        char* m_pName ;     // Name of bone
        f32   m_Weight ;    // Weight of bone
    } ;
    

//=========================================================================
// Private functions
//=========================================================================
private:
            // These functions is only used internally - never call this outside of loco!
            xbool           HasArrivedAtPosition        ( const vector3& Pos, f32 ArriveDistSqr ) ;
            f32             ComputeExactArriveDistSqr   ( void ) ;

//=========================================================================
// Public functions
//=========================================================================
public:

            // Static property functions
    static  const char*     GetMoveStyleName     ( s32 MoveStyle );   
    static  const char*     GetMoveStyleHeader   ( s32 MoveStyle );
    static  const char*     GetMoveStyleAnimName ( s32 MoveStyleAnim );
    static  const char*     GetMoveStyleEnum     ( void );
    static  move_style      GetMoveStyleByName   ( const char* pName );
    

            // Position related functions
                            loco                ( void );

#if !defined( CONFIG_RETAIL )
virtual     void            RenderInfo          ( xbool bRenderLoco     = TRUE, 
                                                  xbool bLabelLoco      = FALSE,
                                                  xbool bRenderSkeleton = FALSE, 
                                                  xbool bLabelSkeleton  = FALSE );
#endif // !defined( CONFIG_RETAIL )

      const matrix4*        ComputeL2W          ( void ) ;
      const vector3&        GetPosition         ( void ) const { return m_Player.GetPosition(); }
            void            SetPosition         ( const vector3& Position ) ;                                                
            void            SetPitch            ( radian Pitch );
            
      const vector3&        GetMoveAt           ( void ) const { return m_MoveAt; }
      const vector3&        GetHeadLookAt       ( void ) const { return m_HeadLookAt; }
      const vector3&        GetBodyLookAt       ( void ) const { return m_BodyLookAt; }
            xbool           IsExactMove         ( void ) const { return m_bExactMove ; }
            xbool           IsExactMoveBlending ( void ) const { return m_bExactMoveBlending; }            
            xbool           IsExactLook         ( void ) const { return m_bExactLook ; }
            xbool           IsMoveAtSnap        ( void ) const { return m_bMoveAtSnap ; }

            // Bone related function
    const   vector3&        GetEyeOffset        ( void ) const;
            vector3         GetEyePosition      ( void ) const;
            radian          GetSightYaw         ( void ) const;
            
    const   matrix4&        GetWeaponBoneL2W        ( s32 iHand = 0 );
            matrix4         GetWeaponL2W            ( s32 iHand = 0 );
    const   matrix4&        GetGrenadeBoneL2W       ( void );
            vector3         GetGrenadeBonePosition  ( void );

            matrix4         GetFlagL2W              ( void );

            s32             GetWeaponBoneIndex      ( s32 iHand = 0 )  { return m_Player.m_iWeaponBone[iHand]; }
            s32             GetGrenadeBoneIndex     ( void )  { return m_Player.m_iGrenadeBone; }
            s32             GetFlagBoneIndex        ( void )  { return m_Player.m_iFlagBone; }

            // Destination functions
            xbool           IsAtDestination     ( void );
            xbool           IsAtPosition        ( const vector3& Pos ) ;
            xbool           IsAtPosition        ( const vector3& Pos,  f32 ArriveDistSqr ) ;
            void            SetArriveDist       ( f32 ArriveDist ) ;
            void            SetArriveDistSqr    ( f32 ArriveDistSqr ) ;

            // Look at functions
            xbool           IsExactLookComplete ( void );

            // Delta position functions
      const vector3&        GetDeltaPos         ( void ) const { return m_DeltaPos ; }            
            void            SetDeltaPosScale    ( const vector3& Scale );
            radian          GetDeltaYaw         ( void ) const { return m_DeltaYaw; }

            // Bone functions
            s32             GetNBones           ( void );
            s32             GetNActiveBones     ( void );
            xbool           IsAnimLoaded        ( void );

            // Actor functions
            f32             GetActorCollisionRadius( void );
            f32             GetActorCollisionHeight( void );

            // Move and look functions
            // NOTE: "SetLookAt" sets both the head and body look at to the same location
            void            ResetMoveAndLookAt  ( void );
            void            SetMoveAt           ( const vector3& Target );
            void            SetHeadLookAt       ( const vector3& Target, xbool bSetEyesLookAt = TRUE );
            void            SetBodyLookAt       ( const vector3& Target );
            void            SetLookAt           ( const vector3& Target, xbool bSetEyesLookAt = TRUE );
            void            SetEyesLookAt       ( const vector3& Target );
            void            SetExactMove        ( xbool bExact ) ;
            void            SetExactLook        ( xbool bExact ) ;
            void            SetMoveAtSnap       ( xbool bSnap  ) { m_bMoveAtSnap = bSnap ; }
            void            SetAimerBlendSpeed  ( f32 AimerBlendSpeed );    // Default = 1.0f
            void            SetAimerWeight      ( f32 Weight, f32 BlendTime = 0.0f );
            void            ClearMoveFlags      ( void );
            
            // Cinema relative mode functions
            void            SetCinemaRelativeMode     ( xbool bEnable );
            void            SetCinemaRelativeInfo     ( const vector3& Pos, radian Yaw ) ;
            const vector3&  GetCinemaRelativePos      ( void ) const { return m_CinemaRelativePos; }
            radian          GetCinemaRelativeYaw      ( void ) const { return m_CinemaRelativeYaw; }

            // Cover relative mode functions
            void            SetCoverRelativeMode     ( xbool bEnable );
            void            SetCoverRelativeInfo     ( const vector3& Pos, radian Yaw ) ;
            const vector3&  GetCoverRelativePos      ( void ) const { return m_CoverRelativePos; }
            radian          GetCoverRelativeYaw      ( void ) const { return m_CoverRelativeYaw; }

            // Ghost mode functions
            void            SetGhostMode        ( xbool bEnable );
            xbool           GetGhostMode        ( void ) const { return m_bGhostMode; }
            void            SetGhostIsMoving    ( xbool bMoving );
            xbool           GetGhostIsMoving    ( void ) const { return m_bGhostIsMoving; }

            // Initialize and main logic functions
    virtual void            OnInit              ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL ) ;
    virtual void            OnAdvance           ( f32  nSeconds );

            // Weapon functions
    virtual xbool           SetWeapon           ( inven_item InvenWeapon ) { (void)InvenWeapon; return FALSE; }
                            
            // Initialization functions
            void                SetupAnimLookupTable( anim_lookup AnimLookups[] ) ;
            s32                 GetAnimIndex        ( loco::anim_type AnimType ) ;
            s32                 GetMoveStyleAnimIndex ( loco::move_style_anim MoveStyleAnim ) ;
            anim_lookup_table&  GetAnimLookupTable  ( void ) { return m_AnimLookupTable; }
            void                InitBoneMasks       ( const geom* pGeom );
            const geom::bone_masks& GetBoneMasks    ( loco::bone_masks_type Type );
            void                InitProperties      ( const geom* pGeom );
                                        
            // State functions
            const char*     GetStateName        ( loco::state State ) const ;
            xbool           SetState            ( loco::state State ) ;
            loco::state     GetState            ( void ) const ;
            const char*     GetStateName        ( void ) const ;
            loco::state     GetPrevState        ( void ) const ;
            const char*     GetPrevStateName    ( void ) const ;
            void            SetStateAnimRate    ( loco::state State, f32 Rate ) ;
            f32             GetStateAnimRate    ( loco::state State ) const ;

            // Anim functions
            void            SetAimerBoneMasks   ( xbool bAiming, f32 BlendTime );
            
            // Play animation functions
            // IN:
            //      hAnimGroup   = Handle to specific anim group to use
            //      iAnim        = Index of specific animation to play
            //      pAnim        = Name of specific animation to play
            //      BlendInTime  = Time it takes to blend this animation in
            //      BlendOutTime = Time it takes to blend this animation out
            //      BlendTime    = Time it takes to blend out the old animation and blend in this new animation
            //      Flags        = Various control flags (see anim_flags anum)

            // Masked controller functions
            xbool           PlayMaskedAnim      ( const char* pAnimGroup, const char* pAnim, f32 BlendTime, u32 Flags = 0 );
            xbool           PlayMaskedAnim      ( s32 iAnim,                bone_masks_type MaskType, f32 BoneMaskBlendTime, u32 Flags = 0 ) ;
            xbool           PlayMaskedAnim      ( loco::anim_type AnimType, bone_masks_type MaskType, f32 BoneMaskBlendTime, u32 Flags = 0 ) ;
            
            // Additive controller functions
            xbool           PlayAdditiveAnim    ( s32 iAnim,                f32 BlendInTime = 0.1f, f32 BlendOutTime = 0.1f, u32 Flags = 0 ) ;
            xbool           PlayAdditiveAnim    ( loco::anim_type AnimType, f32 BlendInTime = 0.1f, f32 BlendOutTime = 0.1f, u32 Flags = 0 ) ;
            xbool           PlayAdditiveAnim    ( const char* pAnim,        f32 BlendInTime = 0.1f, f32 BlendOutTime = 0.1f, u32 Flags = 0 ) ;

            // Lip sync controller functions
            xbool           PlayLipSyncAnim     ( const anim_group::handle& hAnimGroup, s32         iAnim,     u32         VoiceID,    u32 Flags = ANIM_FLAG_MASK_TYPE_FACE ) ;
            xbool           PlayLipSyncAnim     ( const anim_group::handle& hAnimGroup, const char* pAnimName, const char* pAudioName, u32 Flags = ANIM_FLAG_MASK_TYPE_FACE ) ;
            xbool           PlayLipSyncAnim     ( const anim_group::handle& hAnimGroup, const char* pAnimName, u32         VoiceID,    u32 Flags = ANIM_FLAG_MASK_TYPE_FACE ) ;
            
            xbool           PlayLipSyncAnim     ( s32             iAnim,     const char* pAudioName, u32 Flags = ANIM_FLAG_MASK_TYPE_FACE ) ;
            xbool           PlayLipSyncAnim     ( loco::anim_type AnimType,  const char* pAudioName, u32 Flags = ANIM_FLAG_MASK_TYPE_FACE ) ;
            xbool           PlayLipSyncAnim     ( const char*     pAnimName, const char* pAudioName, u32 Flags = ANIM_FLAG_MASK_TYPE_FACE ) ;
            
            void            UpdateFaceIdle      ( f32 DeltaTime );
            void            UpdateEyeTracking   ( f32 DeltaTime );
            void            UpdateDynamicLipSyncBoneMasks( void );

            // Main motion controller functions
            xbool           PlayAnim            ( const anim_group::handle& hAnimGroup, s32         iAnim, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
            xbool           PlayAnim            ( const anim_group::handle& hAnimGroup, const char* pAnim, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
            xbool           PlayAnim            ( const char* pAnimGroup, const char* pAnim, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
            xbool           PlayAnim            ( s32 iAnim,                f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
            xbool           PlayAnim            ( const char* pAnim,        f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
            xbool           PlayAnim            ( loco::anim_type AnimType, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
    virtual xbool           PlayDeathAnim       ( loco::anim_type AnimType, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = ANIM_FLAG_INTERRUPT_BLEND ) ;
    virtual xbool           PlayDeathAnim       ( const anim_group::handle& hAnimGroup, const char* pAnim, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = ANIM_FLAG_INTERRUPT_BLEND ) ;
            xbool           IsPlayAnimComplete  ( void ) const ;
            u32             GetPlayAnimFlags    ( void ) const ;

            // Controller access functions
            loco_motion_controller&     GetMotionController     ( u32 AnimFlags = 0 ) ;
            loco_aim_controller&        GetAimController        ( u32 AnimFlags = 0 ) ;
            loco_mask_controller&       GetMaskController       ( u32 AnimFlags = 0 ) ;
            loco_additive_controller&   GetAdditiveController   ( u32 AnimFlags = 0 ) ;
            loco_lip_sync_controller&   GetLipSyncController    ( u32 AnimFlags = 0 ) ;
            loco_eye_controller&        GetEyeController        ( u32 AnimFlags = 0 ) ;
            
            // Misc
    virtual void            SetMoveStyle                ( move_style Style );
            void            SetBlendMoveStyle           ( move_style Style );
            void            SetBlendMoveStyleAmount     ( f32        Amount );
           
            void            SwitchMoveStyleSmoothly     ( move_style Style, move_style_anim Anim ) ;
            void            UpdateMoveStyle             ( void );
            
            move_style_anim GetCurrentMoveStyleAnim     ( void );
            s32             GetMoveStyleAnimIndex       ( move_style Style, move_style_anim Anim );
    virtual xbool           IsValidMoveStyle            ( move_style Style );
    virtual move_style      GetValidMoveStyle           ( move_style Style );
            void            SetMoveStyleDefaults        ( move_style Style, const move_style_info_default& Defaults );
    
            // Internal math functions
            void            ComputeHeadAim      ( radian FacingYawBias, radian& H, radian& V );
            void            ComputeBodyAim      ( radian FacingYawBias, radian& H );
            void            ComputeMotion       ( xbool           bAllowDir[4],    // F L B R
                                                  radian          LookYaw,
                                                  radian          MoveYaw,
                                                  loco::motion&   Motion,
                                                  radian&         DeltaYaw ) ;
            void            ComputeValidMotion  ( loco::motion& Motion, radian& DeltaYaw ) ;
            radian          ComputeMoveDir      ( void );

            // Motion functions
            motion          GetMotion           ( void ) const      { return m_Motion ; }
            radian          GetMotionYaw        ( motion Motion ) const ;
            const char*     GetMotionName       ( motion Motion ) const ;
            vector3         GetMotionLookPoint  ( void ) const ;
            void            SetAllowMotion      ( motion Motion, xbool bEnable );
            xbool           IsMotionAllowed     ( motion Motion );

            // Yaw functions. NOTE: All yaws are in game world space
            void            ApplyDeltaYaw       ( radian DeltaYaw ) ;
            radian          GetYaw              ( void ) ;
            void            SetYaw              ( radian Yaw ) ;
            void            SetYawFacingTarget  ( radian TargetYaw, radian MaxDeltaYaw ) ;
            radian          GetAimerYaw         ( void );

            // Convenient local to world matrix functions
            void            SetL2W              ( const matrix4& L2W ) ;
            matrix4         GetL2W              ( void ) ;

            void            ChangeToPreviousState( void );

            // Misc query functions
            xbool           HasPassedMoveAt     ( void ) const      { return m_bPassedMoveAt ;  }
            void            SetDead             ( xbool bDead )     { m_bDead = bDead ;         }
            void            SetFaceIdleEnabled  ( xbool bEnabled )  { m_bFaceIdleEnabled = bEnabled; }
            xbool           IsFaceIdleEnabled   ( void ) const      { return m_bFaceIdleEnabled; }
            xbool           IsStuck             ( void ) const  { return m_bLocoIsStuck ;   }
            move_style      GetMoveStyle        ( void ) const  { return m_MoveStyle ;      }
            move_style      GetBlendMoveStyle   ( void ) const  { return m_BlendMoveStyle ; }
            f32             GetBlendMoveStyleAmount( void ) const  { return m_BlendMoveStyleAmount ; }
            const anim_group::handle& GetAnimGroupHandle( void ) const  { return m_hAnimGroup ;     }

            // use run aim?
            void            SetUseAimMoveStyles ( xbool bUseAimStyles )        { m_bUseAimMoveStyles = bUseAimStyles; }

            void            SetDoAdvancePhysics( xbool bDoAdvancePhysics ) { m_bAdvancePhysics = bDoAdvancePhysics; }

            vector3         GetBonePosition    ( s32 Bone );
            s32             GetRandomBone      ( void );            

//=========================================================================
// Data
//=========================================================================
public:
    loco_char_anim_player           m_Player ;                  // The big animation player handles all animation
    character_physics               m_Physics ;                 // Handles movement
    
protected:
    loco_aim_controller             m_AimController ;           // Controller responsible for aiming
    loco_mask_controller            m_MaskController ;          // Masked controller
    loco_additive_controller        m_AdditiveController[2] ;   // Additive controllers
    loco_lip_sync_controller        m_LipSyncController ;       // Lip sync controller
    loco_eye_controller             m_EyeController ;           // Additive eye controller

//=========================================================================
// Private data
//=========================================================================
protected:

    u32                             m_bDead                     : 1,    // TRUE if character is dead
                                    m_bLocoIsStuck              : 1,    // TRUE if character has been stuck for a while
                                    m_bAllowFrontMotion         : 1,    // Controls if character can play MOVE_FRONT animations
                                    m_bAllowLeftMotion          : 1,    // Controls if character can play MOVE_LEFT animations
                                    m_bAllowBackMotion          : 1,    // Controls if character can play MOVE_BACK animations
                                    m_bAllowRightMotion         : 1,    // Controls if character can play MOVE_RIGHT animations
                                    m_bPassedMoveAt             : 1,    // TRUE if NPC has gone thru the move at
                                    m_bExactMove                : 1,    // Use for scripted stuff - pixel perfect if TRUE!
                                    m_bExactLook                : 1,    // Use for scripted stuff - pixel perfect if TRUE!
                                    m_bExactLookComplete        : 1,    // TRUE if exact look at is complete
                                    m_bUseAimMoveStyles         : 1,    // TRUE if we want to use RUNAIM in place of RUN.
                                    m_bMoveAtSnap               : 1,    // TRUE if NPC should snap if passing move at
                                    m_bExactMoveBlending        : 1,    // TRUE if blending from MOVE->IDLE with exact move on
                                    m_bExactMoveBlendingStarted : 1,    // TRUE if blending has begun.
                                    m_bAdvancePhysics           : 1,    // TRUE if physics should be advanced
                                    m_bGhostMode                : 1,    // TRUE if loco is being used for a net ghost character
                                    m_bGhostIsMoving            : 1,    // TRUE if ghost should be moving ie. not idle
                                    m_bFrameMatchMoveAnim       : 1,    // Used for smooth move style switching
                                    m_bDynamicLipSyncAnim       : 1,    // TRUE if our lipsync anims switches masks depending upon loco state.
                                    m_bStateChangedThisTick     : 1,    // TRUE if our state changed this tick.
                                    m_bFaceIdleEnabled          : 1;    // TRUE if blinking etc is allowed

    anim_group::handle              m_hAnimGroup ;                      // Assigned animation group
    state                           m_AnimState ;                       // Current animation state
    u32                             m_PlayAnimFlags ;                   // Current play animation flags
    vector3                         m_DeltaPosScale ;                   // Use to slide delta pos
    vector3                         m_DeltaPos ;                        // Current delta position
    radian                          m_DeltaYaw;                         // Current delta yaw

    radian                          m_GhostYaw;                         // Current ghost yaw
    radian                          m_GhostPitch;                       // Current ghost pitch

    vector3                         m_MoveAt ;                          // Location to move to
    vector3                         m_ExactMoveBlendPos ;               // Position to blend from
                                                                        
    f32                             m_ArriveDistSqr ;                   // Min distance squared to get from move at
    vector3                         m_HeadLookAt ;                      // Head location to look at
    vector3                         m_BodyLookAt ;                      // Body location to look at

    motion                          m_Motion ;                          // Current motion 
    motion                          m_ToTransition ;                    // Current transition motion
                                                                        
    loco_state*                     m_pHead ;                           // Head of state list
    loco_state*                     m_pActive ;                         // Currently active state
    loco_state*                     m_pPrev ;                           // Previously active state
                                                                        
    move_style                      m_MoveStyle ;                       // Current move style
    move_style                      m_BlendMoveStyle;                   // Blend move style
    f32                             m_BlendMoveStyleAmount;             // 0 = m_MoveStyle, 1 = m_BlendMoveStyle
    move_style_info                 m_MoveStyleInfo ;                   // Current move style info (anims to use etc)
    move_style_info_default         m_MoveStyleInfoDefault[ MOVE_STYLE_COUNT ];  // Default values per movestyle
    
    f32                             m_StateAnimRate[STATE_TOTAL] ;      // Anim rate modifier               

    vector3                         m_CinemaRelativePos ;               // Cinema relative position
    radian                          m_CinemaRelativeYaw ;               // Cinema relative yaw

    vector3                         m_CoverRelativePos ;                // Cover relative position
    radian                          m_CoverRelativeYaw ;                // Cover relative yaw

    anim_lookup_table               m_AnimLookupTable ;                 // Anim type -> index lookup
    
    const geom*                     m_pGeom;                                     // Geom that loco is controlling
    const geom::bone_masks*         m_pBoneMasks[ loco::BONE_MASKS_TYPE_COUNT ]; // List of bone masks

    f32                             m_FaceIdleTimer;                    // Time before next face idle
    f32                             m_FaceIdleMinInterval;              // Min time before next face idle
    f32                             m_FaceIdleMaxInterval;              // Max time before next face idle
    
    friend class loco_state ;
    friend struct loco_play_anim ;
    friend struct loco_idle ;
    friend struct loco_move ;

//=========================================================================
// Editor
//=========================================================================
public:
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   I    ) ;

#ifdef X_EDITOR
    virtual s32     OnValidateProperties    ( const skin_inst& SkinInst, xstring& ErrorMsg );
#endif


//=========================================================================
// Public static data
//=========================================================================

static geom::bone_masks s_ZeroBoneMasks;
static geom::bone_masks s_OneBoneMasks;
};


//=========================================================================
// LOCOMOTION STATES
//=========================================================================
class loco_state 
{
public:

                            loco_state              ( loco& Loco, loco::state State );
    virtual void            OnAdvance               ( f32 nSeconds ) { (void)nSeconds; } 
    virtual void            OnEnter                 ( void ) {}
    virtual xbool           OnExit                  ( void ) { return TRUE ;}
    virtual void            OnInit                  ( void ) {}
    virtual xbool           IsComplete              ( void ) { return TRUE ; }

            loco&           m_Base;
            loco::state     m_State;
            loco_state*     m_pNext;
};

//=========================================================================

struct loco_play_anim : public loco_state
{
                            loco_play_anim  ( loco& Loco );
    virtual void            OnAdvance       ( f32 nSeconds );
    virtual void            OnEnter         ( void );
    virtual xbool           OnExit          ( void );
    virtual xbool           IsComplete      ( void ) { return m_bComplete ; }
            xbool           PlayAnim        ( const anim_group::handle& hAnimGroup, s32 iAnim, f32 BlendTime, u32 Flags, f32 PlayTime ) ;

            loco::state     m_PrevState ;    // Previous state before this one was entered
            u32             m_Flags ;       // Play anim flags
            f32             m_PlayTime ;    // How long animation should play in secs or cylces
            f32             m_Timer ;       // Time in state
            xbool           m_bComplete ;   // TRUE if play animation is complete
} ;

//=========================================================================

struct loco_idle : public loco_state
{
                        loco_idle    ( loco& Loco );
    virtual void        OnAdvance    ( f32 nSeconds );
    virtual void        OnEnter      ( void );
    virtual xbool       OnExit       ( void ) ;

    f32                     m_Timer;        // General timer
    f32                     m_FidgetTimer;  // Fidget timer
    loco::move_style_anim   m_IdleAnim;     // Idle animation to play
};

//=========================================================================

struct loco_move : public loco_state
{
                        loco_move   ( loco& Loco );
    virtual void        OnAdvance   ( f32 nSeconds );
    virtual void        OnEnter     ( void );
    virtual xbool       OnExit      ( void );

    f32                 m_Timer;                // General timer
    xbool               m_bFirstTime;           // TRUE if first time advance has been called in state
};


//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

inline radian loco::GetYaw( void )
{
    return m_Player.GetFacingYaw() ;
}

//=========================================================================

inline void loco::ChangeToPreviousState( void )
{
    if(m_pPrev) 
    {
        SetState( m_pPrev->m_State ) ;
    }
    else
    {
        SetState( STATE_IDLE );
    }
}

//=========================================================================

inline
void loco::SetDeltaPosScale( const vector3& Scale )
{
    // Sanity check
    ASSERT( Scale.GetX() >= -20.0f );
    ASSERT( Scale.GetY() >= -20.0f );
    ASSERT( Scale.GetZ() >= -20.0f );
    ASSERT( Scale.GetX() <= 20.0f );
    ASSERT( Scale.GetY() <= 20.0f );
    ASSERT( Scale.GetZ() <= 20.0f );
    
    m_DeltaPosScale = Scale;
}

//=========================================================================

inline f32 loco::GetActorCollisionRadius()
{
    return m_Physics.GetActorCollisionRadius();
}

//=========================================================================

inline f32 loco::GetActorCollisionHeight()
{
    return m_Physics.GetColHeight();
}

//==============================================================================

inline xbool loco::PlayLipSyncAnim( const char* pAnimName, const char* pAudioName, u32 Flags )
{
    return PlayLipSyncAnim(m_hAnimGroup, pAnimName, pAudioName, Flags) ;
}

//==============================================================================

inline
void loco::SetAimerBoneMasks( xbool bAiming, f32 BlendTime )
{
    // Use aiming masks?
    if( bAiming )
    {
        // Set aiming masks
        m_AimController.SetBoneMasks( GetBoneMasks( BONE_MASKS_TYPE_AIM_VERT ),
                                      GetBoneMasks( BONE_MASKS_TYPE_AIM_HORIZ ),
                                      BlendTime );
    }
    else
    {
        // Set no aiming masks
        m_AimController.SetBoneMasks( GetBoneMasks( BONE_MASKS_TYPE_NO_AIM_VERT ),
                                      GetBoneMasks( BONE_MASKS_TYPE_NO_AIM_HORIZ ),
                                      BlendTime );
    }
}

//==============================================================================

inline xbool loco::PlayAnim( s32 iAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    return PlayAnim(m_hAnimGroup, iAnim, BlendTime, Flags, PlayTime) ;
}

//=========================================================================

inline u32 loco::GetPlayAnimFlags( void ) const
{
    return m_PlayAnimFlags ;
}

//=========================================================================

inline loco_aim_controller& loco::GetAimController( u32 AnimFlags )
{
    // Not presently used
    (void)AnimFlags ;

    return m_AimController ;
}

//=========================================================================

inline loco_mask_controller& loco::GetMaskController( u32 AnimFlags )
{
    // Not presently used
    (void)AnimFlags ;

    return m_MaskController ;
}

//=========================================================================

inline loco_lip_sync_controller& loco::GetLipSyncController( u32 AnimFlags )
{
    // Not presently used
    (void)AnimFlags ;

    return m_LipSyncController ;
}

//=========================================================================

inline loco_eye_controller& loco::GetEyeController( u32 AnimFlags )
{
    // Not presently used
    (void)AnimFlags ;

    return m_EyeController ;
}

//=========================================================================

inline void loco::SetArriveDist( f32 ArriveDist )
{
    ASSERT(x_isvalid(ArriveDist)) ;
    m_ArriveDistSqr = x_sqr(ArriveDist) ;
}

//=========================================================================

inline void loco::SetArriveDistSqr( f32 ArriveDistSqr )
{
    ASSERT(x_isvalid(ArriveDistSqr)) ;
    m_ArriveDistSqr = ArriveDistSqr ;
}

//=========================================================================

inline xbool loco::IsAtDestination( void )
{
    // Arrived within distance?
    return IsAtPosition( m_MoveAt, m_ArriveDistSqr ) ;
}

//=========================================================================

inline xbool loco::IsAtPosition( const vector3& Pos )
{
    // Use default arrive distance
    return IsAtPosition( Pos, m_ArriveDistSqr ) ;
}

//=========================================================================

inline xbool loco::IsExactLookComplete ( void )
{
    return ( m_bExactLook && m_bExactLookComplete );
}

//=========================================================================

inline const matrix4* loco::ComputeL2W( void )
{
    return m_Player.GetBoneL2Ws() ;
}

//=========================================================================

inline const vector3& loco::GetEyeOffset( void ) const
{
    return m_Player.m_AimAtOffset;
}

//=========================================================================

inline vector3 loco::GetEyePosition( void ) const
{    
    const vector3& Pos    = GetPosition();
    const vector3& Offset = GetEyeOffset();
    return Pos + Offset;
}

//=========================================================================

inline radian loco::GetSightYaw( void ) const
{
    return m_Player.m_HeadL2W.GetRotation().Yaw + R_180;
}

//=========================================================================

inline const matrix4& loco::GetWeaponBoneL2W( s32 iHand )
{
    return m_Player.GetBoneL2W( m_Player.m_iWeaponBone[iHand] );
}

//=========================================================================

inline const matrix4& loco::GetGrenadeBoneL2W( void )
{
    return m_Player.GetBoneL2W( m_Player.m_iGrenadeBone );
}

//=========================================================================

inline vector3 loco::GetGrenadeBonePosition( void )
{
    return m_Player.GetBonePosition( m_Player.m_iGrenadeBone );
}

//=========================================================================

inline s32 loco::GetNBones( void )
{
    return( m_Player.GetNBones() );
}

//=========================================================================

inline s32 loco::GetNActiveBones( void )
{
    return m_Player.GetNActiveBones() ;
}

//=========================================================================

inline xbool loco::IsAnimLoaded( void )
{
    return( m_hAnimGroup.IsLoaded() );
}

//=========================================================================

inline void loco::SetEyesLookAt( const vector3& Target )
{
    ASSERT(Target.IsValid()) ;
    m_EyeController.SetLookAt(Target) ;
}

//=========================================================================

inline void loco::SetExactMove( xbool bExact )
{
    m_bExactMove = bExact ;
}

//=========================================================================

inline void loco::SetExactLook( xbool bExact )
{
    m_bExactLook         = bExact ;
}

//=========================================================================

inline void loco::SetGhostIsMoving( xbool bMoving )
{
    m_bGhostIsMoving = bMoving;
}

//=========================================================================
// END
//=========================================================================
#endif
