//=========================================================================
//
//  Stage5Loco.cpp
//
//=========================================================================

#ifndef __STAGE5_LOCO_HPP__
#define __STAGE5_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"


//=========================================================================
// STAGE5 STATES
//=========================================================================

struct stage5_loco_play_anim : public loco_play_anim
{
                            stage5_loco_play_anim   ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct stage5_loco_idle : public loco_idle
{
                            stage5_loco_idle        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct stage5_loco_move : public loco_move
{
                            stage5_loco_move        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct stage5_loco_shoot : public loco_state
{
// Defines
    enum phase
    {
        PHASE_ENTER,
        PHASE_IDLE,
        PHASE_EXIT
    } ;

// Functions
                            stage5_loco_shoot       ( loco& Loco );
    virtual void            OnEnter                 ( void );
    virtual void            OnAdvance               ( f32 nSeconds ) ;
            
            void            SetShooting             ( xbool bShoot ) ;
            void            Exit                    ( loco::states ExitState ) ;
            xbool           IsReadyToShoot          ( void ) ;

// Data
    phase           m_Phase ;       // 0=Entering, 1=Shooting, 2=Exiting
    loco::states    m_ExitState ;   // If != STATE_NULL, then exit ASAP
    xbool           m_bShoot ;      // Flags shooting anim
} ;


//=========================================================================
// STAGE5 LOCO
//=========================================================================

class stage5_loco : public loco
{
// Defines
public:

// Functions
public:
                        stage5_loco         ( void );
    virtual void        OnInit              ( const char* pSkinGeomFileName, const char* pAnimFileName, guid ObjectGuid = NULL ) ;
    virtual s32         GetAnimIndex        ( loco::anims Anim ) ;
    virtual void        SetMoveStyle        ( move_style Style ) ;

            vector3     GetRightSpikePos    ( void ) ;
            vector3     GetLeftSpikePos     ( void ) ;
            
            void        SetShooting         ( xbool bShoot ) ;
            void        EnterShootState     ( void ) ;
            void        ExitShootState      ( void ) ;
            xbool       IsReadyToShoot      ( void ) ;

// Private data
protected:

    stage5_loco_play_anim   m_PlayAnim ;
    stage5_loco_idle        m_Idle ;
    stage5_loco_move        m_Move ;
    stage5_loco_shoot       m_Shoot ;

// Public static data
public:
    // Animations and bones - looked up just once during initialization
    static s32                              s_BoneIndexLeftSpike ;
    static s32                              s_BoneIndexRightSpike ;
    static bone_masks                       s_PlayAnimBoneMasks ;
    static bone_masks                       s_MoveBoneMasks ;
    static bone_masks                       s_ShootBoneMasks ;
    static general_anims                    s_GeneralAnims ;
    static move_anims                       s_ProwlAnims ;
    static move_anims                       s_WalkAnims ;
    static move_anims                       s_RunAnims ;
    static loco_aim_controller::aim_anims   s_AimAnims ;
    static loco_aim_controller::aim_anims   s_ShootAnims ;
};

//=========================================================================
// END
//=========================================================================
#endif
