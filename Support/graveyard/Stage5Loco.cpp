//=========================================================================
//
//  Stage5Loco.cpp
//
//=========================================================================


#include "Stage5Loco.hpp"


//==============================================================================
//==============================================================================
//==============================================================================
// STAGE5 STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

stage5_loco_play_anim::stage5_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void stage5_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter() ;
    m_Base.SetAimBoneMasks(stage5_loco::s_PlayAnimBoneMasks, 0.5f) ;
}

//=========================================================================
// IDLE
//=========================================================================

stage5_loco_idle::stage5_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void stage5_loco_idle::OnEnter( void )
{
    loco_idle::OnEnter() ;
    m_Base.SetAimBoneMasks(stage5_loco::s_MoveBoneMasks, 0.5f) ;
}

//=========================================================================
// MOVE
//=========================================================================

stage5_loco_move::stage5_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void stage5_loco_move::OnEnter( void )
{
    loco_move::OnEnter() ;
    m_Base.SetAimBoneMasks(stage5_loco::s_MoveBoneMasks, 0.5f) ;
}

//=========================================================================
// MOVE
//=========================================================================

stage5_loco_shoot::stage5_loco_shoot( loco& Loco ) :
    loco_state(Loco, loco::STATE_SHOOT)
{
}

void stage5_loco_shoot::OnEnter( void )
{
    // Blend to shooting masks
    m_Base.SetAimBoneMasks(stage5_loco::s_ShootBoneMasks, 0.5f) ;

    // Setup pase
    m_Phase     = PHASE_ENTER ;
    m_ExitState = loco::STATE_NULL ;
    m_bShoot    = FALSE ;

    // Play "Getup" anim
    m_Base.m_Player.SetAnim(stage5_loco::s_GeneralAnims.Index[loco::ANIM_ATTACK_STANCE_ENTER], 0.25f) ;
}

void stage5_loco_shoot::OnAdvance( f32 nSeconds )
{
    (void)nSeconds ;

    // Which phase?
    switch(m_Phase)
    {
        // Geting up
        case PHASE_ENTER:
        {
            // Goto next phase?
            if (m_Base.m_Player.IsAtEnd())
            {
                m_Base.m_Player.SetAnim(stage5_loco::s_GeneralAnims.Index[loco::ANIM_ATTACK_STANCE_IDLE], 0.25f) ;
                m_Phase = PHASE_IDLE ;
            }
        }
        break ;

        // Idling ready to shoot
        case PHASE_IDLE:
        {
            // Get out?
            if (m_ExitState != loco::STATE_NULL)
            {
                m_Base.m_AimController.Init(stage5_loco::s_AimAnims) ;
                m_Base.m_Player.SetAnim(stage5_loco::s_GeneralAnims.Index[loco::ANIM_ATTACK_STANCE_EXIT], 0.25f) ;
                m_Phase = PHASE_EXIT ;
            }
            else
            {
                // Setup aim anims
                if (m_bShoot)
                    m_Base.m_AimController.Init(stage5_loco::s_ShootAnims) ;
                else
                    m_Base.m_AimController.Init(stage5_loco::s_AimAnims) ;
            }
        }
        break ;

        // Getting down
        case PHASE_EXIT:
        {
            // If done playing anim, goto next state
            if (m_Base.m_Player.IsAtEnd())
                m_Base.SetState(m_ExitState) ;
        }
        break ;
    }
}

void stage5_loco_shoot::SetShooting( xbool bShoot )
{
    m_bShoot = bShoot ;
}

void stage5_loco_shoot::Exit( loco::states ExitState )
{
    m_ExitState = ExitState ;
}

xbool stage5_loco_shoot::IsReadyToShoot( void )
{
    return (m_Phase == 1) ;
}



//==============================================================================
//==============================================================================
//==============================================================================
// STAGE5 LOCO
//==============================================================================
//==============================================================================
//==============================================================================

// Static data
s32                             stage5_loco::s_BoneIndexLeftSpike = -1 ;
s32                             stage5_loco::s_BoneIndexRightSpike = -1 ;
stage5_loco::bone_masks         stage5_loco::s_PlayAnimBoneMasks ;
stage5_loco::bone_masks         stage5_loco::s_MoveBoneMasks ;
stage5_loco::bone_masks         stage5_loco::s_ShootBoneMasks ;
stage5_loco::general_anims      stage5_loco::s_GeneralAnims ;
stage5_loco::move_anims         stage5_loco::s_ProwlAnims ;
stage5_loco::move_anims         stage5_loco::s_WalkAnims ;
stage5_loco::move_anims         stage5_loco::s_RunAnims ;
loco_aim_controller::aim_anims  stage5_loco::s_AimAnims ;
loco_aim_controller::aim_anims  stage5_loco::s_ShootAnims ;


//==============================================================================

stage5_loco::stage5_loco( void ) :
    loco(),
    m_PlayAnim( *this ),
    m_Idle    ( *this ),
    m_Move    ( *this ),
    m_Shoot   ( *this )
{
}

//==============================================================================

// Lookup general animation indices
static loco::anim_lookup s_GeneralAnimsLookup[] =
{
    { loco::ANIM_DAMAGE_STEP_BACK             , "HM_5_IMPACT_STEP_BACK"    },
    { loco::ANIM_DAMAGE_STEP_FORWARD          , "HM_5_IMPACT_STEP_BACK" },
    { loco::ANIM_DAMAGE_STEP_LEFT             , "HM_5_IMPACT_STEP_BACK"    },
    { loco::ANIM_DAMAGE_STEP_RIGHT            , "HM_5_IMPACT_STEP_BACK"   },
    
    { loco::ANIM_DEATH                        , "HM_5_DEATH_SLOW"  },
    { loco::ANIM_DEATH_HEAD_SHOT              , "HM_5_DEATH_QUICK" },

    { loco::ANIM_ATTACK_HOWL                  , "HM_5_ATTACK_HOWL" },
    { loco::ANIM_ATTACK_CLAW                  , "HM_5_ATTACK_CLAW" },
    
    { loco::ANIM_ATTACK_STANCE_ENTER          , "HM_5_ATTACK_ENTER_STANCE"  },
    { loco::ANIM_ATTACK_STANCE_EXIT           , "HM_5_ATTACK_EXIT_STANCE"   },
    { loco::ANIM_ATTACK_STANCE_IDLE           , "HM_5_ATTACK_IDLE"          },

    { loco::ANIM_NULL                         ,  NULL }
} ;


// Play anim bone masks
static loco::bone_lookup s_PlayAnimBoneMasksLookup[] =
{
    { "mutant5_neck"     , 0.0f },
    { "mutant5_head"     , 0.0f },    

    { NULL, 0.0f}
} ;

// Move anim bone masks
static loco::bone_lookup s_MoveBoneMasksLookup[] =
{
    { "mutant5_neck"     , 0.3f },
    { "mutant5_head"     , 1.0f },    

    { NULL, 0.0f}
} ;

// Shoot anim bone masks
static loco::bone_lookup s_ShootBoneMasksLookup[] =
{
    { "mutant5_neck"     , 0.3f },
    { "mutant5_head"     , 1.0f },    

    { NULL, 0.0f}
} ;

//==============================================================================

void  stage5_loco::OnInit( const char* pSkinGeomFileName, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )

{
    // Call base class
    loco::OnInit(pSkinGeomFileName, pAnimFileName, ObjectGuid) ;

    // Lookup special bone indices
    s_BoneIndexLeftSpike  = m_Player.GetBoneIndex("mutant5_clawspike3L") ;
    s_BoneIndexRightSpike = m_Player.GetBoneIndex("mutant5_clawspike3R") ;

    // Lookup bone masks
    SetupBoneMasks(s_PlayAnimBoneMasks, s_PlayAnimBoneMasksLookup) ;
    SetupBoneMasks(s_MoveBoneMasks,     s_MoveBoneMasksLookup) ;
    SetupBoneMasks(s_ShootBoneMasks,    s_ShootBoneMasksLookup) ;

    // Lookup anims
    SetupAnims(s_GeneralAnims, s_GeneralAnimsLookup) ;

    // Lookup prowl animation indices
    s_ProwlAnims.iI    = m_Player.GetAnimIndex ( "HM_5_DEFAULT_IDLE") ;
    s_ProwlAnims.iTL   = m_Player.GetAnimIndex ( "HM_5_DEFAULT_TURN_LEFT") ;
    s_ProwlAnims.iTR   = m_Player.GetAnimIndex ( "HM_5_DEFAULT_TURN_RIGHT") ;
    s_ProwlAnims.iT180 = m_Player.GetAnimIndex ( "HM_5_DEFAULT_TURN_180") ;

    s_ProwlAnims.iF   = m_Player.GetAnimIndex( "HM_5_WALK_FORWARD") ;
    s_ProwlAnims.iF2R = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_F2R") ;
    s_ProwlAnims.iF2L = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_F2L") ;
    
    s_ProwlAnims.iB   = m_Player.GetAnimIndex( "HM_5_WALK_BACKWARD") ;
    s_ProwlAnims.iB2R = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_B2R") ;
    s_ProwlAnims.iB2L = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_B2L") ;

    s_ProwlAnims.iL   = m_Player.GetAnimIndex( "HM_5_PROWL_STRAFE_L") ;
    s_ProwlAnims.iL2B = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_L2B") ;
    s_ProwlAnims.iL2F = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_L2F") ;
    
    s_ProwlAnims.iR   = m_Player.GetAnimIndex( "HM_5_PROWL_STRAFE_R") ;
    s_ProwlAnims.iR2B = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_R2B") ;
    s_ProwlAnims.iR2F = m_Player.GetAnimIndex( "HM_5_PROWL_BLEND_R2F") ;

    // Lookup walk
    s_WalkAnims.iF = m_Player.GetAnimIndex( "HM_5_WALK_FORWARD") ;
    s_WalkAnims.iB = m_Player.GetAnimIndex( "HM_5_WALK_BACKWARD") ;

    // Lookup run animations
    s_RunAnims.iI    = m_Player.GetAnimIndex ( "HM_5_DEFAULT_IDLE") ;
    s_RunAnims.iTL   = m_Player.GetAnimIndex ( "HM_5_DEFAULT_TURN_LEFT") ;
    s_RunAnims.iTR   = m_Player.GetAnimIndex ( "HM_5_DEFAULT_TURN_RIGHT") ;
    s_RunAnims.iT180 = m_Player.GetAnimIndex ( "HM_5_DEFAULT_TURN_180") ;

    s_RunAnims.iF   = m_Player.GetAnimIndex( "HM_5_RUN_FORWARD") ;
    s_RunAnims.iF2R = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_F2R") ;
    s_RunAnims.iF2L = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_F2L") ;
    
    s_RunAnims.iB   = m_Player.GetAnimIndex( "HM_5_WALK_BACKWARD") ;
    s_RunAnims.iB2R = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_B2R") ;
    s_RunAnims.iB2L = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_B2L") ;

    s_RunAnims.iL   = m_Player.GetAnimIndex( "HM_5_RUN_STRAFE_L") ;
    s_RunAnims.iL2B = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_L2B") ;
    s_RunAnims.iL2F = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_L2F") ;
    
    s_RunAnims.iR   = m_Player.GetAnimIndex( "HM_5_RUN_STRAFE_R") ;
    s_RunAnims.iR2B = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_R2B") ;
    s_RunAnims.iR2F = m_Player.GetAnimIndex( "HM_5_RUN_BLEND_R2F") ;

    // Lookup aim animations
    s_AimAnims.iLT = m_Player.GetAnimIndex("HM_5_LOOK_POS1") ; 
    s_AimAnims.iLC = m_Player.GetAnimIndex("HM_5_LOOK_POS2") ; 
    s_AimAnims.iLB = m_Player.GetAnimIndex("HM_5_LOOK_POS3") ; 
    s_AimAnims.iCT = m_Player.GetAnimIndex("HM_5_LOOK_POS4") ; 
    s_AimAnims.iCC = m_Player.GetAnimIndex("HM_5_LOOK_POS5") ; 
    s_AimAnims.iCB = m_Player.GetAnimIndex("HM_5_LOOK_POS6") ; 
    s_AimAnims.iRT = m_Player.GetAnimIndex("HM_5_LOOK_POS7") ; 
    s_AimAnims.iRC = m_Player.GetAnimIndex("HM_5_LOOK_POS8") ; 
    s_AimAnims.iRB = m_Player.GetAnimIndex("HM_5_LOOK_POS9") ;                              

    // Lookup shoot animations
    s_ShootAnims.iLT = m_Player.GetAnimIndex("HM_5_SHOOT_POS1") ; 
    s_ShootAnims.iLC = m_Player.GetAnimIndex("HM_5_SHOOT_POS2") ; 
    s_ShootAnims.iLB = m_Player.GetAnimIndex("HM_5_SHOOT_POS3") ; 
    s_ShootAnims.iCT = m_Player.GetAnimIndex("HM_5_SHOOT_POS4") ; 
    s_ShootAnims.iCC = m_Player.GetAnimIndex("HM_5_SHOOT_POS5") ; 
    s_ShootAnims.iCB = m_Player.GetAnimIndex("HM_5_SHOOT_POS6") ; 
    s_ShootAnims.iRT = m_Player.GetAnimIndex("HM_5_SHOOT_POS7") ; 
    s_ShootAnims.iRC = m_Player.GetAnimIndex("HM_5_SHOOT_POS8") ; 
    s_ShootAnims.iRB = m_Player.GetAnimIndex("HM_5_SHOOT_POS9") ;   

    // Limit turning since we don't have transitions
    m_MaxDeltaYaw    = DEG_TO_RAD(180.0f) ;
    m_MoveBlendTime  = 0.25f ;

    // Setup blend times
    m_BlendTimeMoveToTrans  = 0.1f ;
    m_BlendTimeMoveNoTrans  = 0.5f ;
    m_BlendTimeIdle         = 0.25f ;
    m_BlendTimeFromPlayAnim = 0.5f ;

    // Initialize loco
    SetAimAnims(s_AimAnims) ;
    SetMoveAnims(s_RunAnims) ;
    SetAimBoneMasks(s_MoveBoneMasks, 0) ;
    SetMoveStyle(MOVE_STYLE_WALK) ;
}

//==============================================================================

s32 stage5_loco::GetAnimIndex( loco::anims Anim )
{
    // Lookup animation index 
    ASSERT(Anim >= 0) ;
    ASSERT(Anim < loco::ANIM_TOTAL) ;
    s32 Index = s_GeneralAnims.Index[Anim] ;

    // Return index
    return Index ;
}

//==============================================================================

void stage5_loco::SetMoveStyle( move_style Style )
{
    // Update?
    if (Style != m_MoveStyle)
    {
        // Set anims for new style
        switch(Style)
        {
            case MOVE_STYLE_RUN:
                m_MoveAnims = s_RunAnims ;

                // Limit turning
                m_MaxDeltaYaw    = DEG_TO_RAD(180) ;
                m_MoveBlendTime  = 0.5f ;
                break ;

            case MOVE_STYLE_WALK:
                m_MoveAnims = s_WalkAnims ;

                // Limit turning
                m_MaxDeltaYaw    = DEG_TO_RAD(45) ;
                m_MoveBlendTime  = 1.0f ;
                break ;

            case MOVE_STYLE_PROWL:
                m_MoveAnims = s_ProwlAnims ;

                // Limit turning
                m_MaxDeltaYaw    = DEG_TO_RAD(45) ;
                m_MoveBlendTime  = 1.0f ;
                break ;
        }

        // Force new anim to play
        m_MoveStyle    = Style ;
    }
}

//==============================================================================

vector3 stage5_loco::GetRightSpikePos( void )
{
    // Not there?
    if (s_BoneIndexRightSpike == -1)
        return GetPosition() ;

    // Use bone
    return m_Player.GetBonePosition( s_BoneIndexRightSpike );
}

//==============================================================================

vector3 stage5_loco::GetLeftSpikePos( void )
{
    // Not there?
    if (s_BoneIndexLeftSpike == -1)
        return GetPosition() ;

    // Use bone
    return m_Player.GetBonePosition( s_BoneIndexLeftSpike );
}

//==============================================================================

void stage5_loco::SetShooting( xbool bShoot )
{
    m_Shoot.SetShooting(bShoot) ;
}

//==============================================================================

void stage5_loco::EnterShootState( void )
{
    SetState(STATE_SHOOT) ;
}

//==============================================================================

void stage5_loco::ExitShootState( void )
{
    m_Shoot.Exit(STATE_IDLE) ;
}

//==============================================================================

xbool stage5_loco::IsReadyToShoot( void )
{
    // Shooting?
    if (GetState() != STATE_SHOOT)
        return FALSE ;

    return m_Shoot.IsReadyToShoot() ;
}

//==============================================================================
