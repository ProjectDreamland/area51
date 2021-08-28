// friendly_scientist_loco : implementation file
//=========================================================================

#include "FriendlyScientistLoco.hpp"

//=========================================================================
// friendly_scientist_loco



//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

friendly_scientist_loco::friendly_scientist_loco( void ) :
    loco(),
    m_PlayAnim( *this ),
    m_Idle    ( *this ),
    m_Move    ( *this )
{
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//==============================================================================

friendly_scientist_loco::~friendly_scientist_loco()
{
}

//==============================================================================

// Lookup general animation indices
static loco::anim_lookup s_AnimsLookup[] =
{                      
    //AS 9/17
    { loco::ANIM_RUN_IDLE                      , "WALK_IDLE"},
    { loco::ANIM_RUN_IDLE_TURN_LEFT            , "WALK_IDLE_TURN_LEFT"},
    { loco::ANIM_RUN_IDLE_TURN_RIGHT           , "WALK_IDLE_TURN_RIGHT"},
    { loco::ANIM_RUN_IDLE_TURN_180             , "WALK_IDLE_TURN_180"},

    //AS TO DO update this, duplicate the run anims.
    // Prowl animations.
    { loco::ANIM_PROWL_IDLE               , "RUN_IDLE"  },
    { loco::ANIM_PROWL_IDLE_TURN_LEFT     , "RUN_IDLE_TURN_LEFT"    },
    { loco::ANIM_PROWL_IDLE_TURN_RIGHT    , "RUN_IDLE_TURN_RIGHT"   },
    { loco::ANIM_PROWL_IDLE_TURN_180      , "RUN_IDLE_TURN_180"     },
    
    { loco::ANIM_PROWL_MOVE_FRONT         , "RUN_MOVE_FRONT"    },
    { loco::ANIM_PROWL_MOVE_BACK          , "RUN_MOVE_BACK"     },
    { loco::ANIM_PROWL_MOVE_LEFT          , "RUN_MOVE_LEFT"     },
    { loco::ANIM_PROWL_MOVE_RIGHT         , "RUN_MOVE_RIGHT"    },
    
    //AS TO DO update all the crouch stuff. this can probably go...
    //maybe leave the crouch left/right stuff in as the walks.
    // Crouch
    { loco::ANIM_CROUCH_MOVE_LEFT            , "WALK_MOVE_LEFT"     },
    { loco::ANIM_CROUCH_MOVE_RIGHT           , "WALK_MOVE_RIGHT"    },


     //AS we need to get this stuff out of here.
    { loco::ANIM_UA_HEAD_NOD                  , "HEAD_NOD"     },
    { loco::ANIM_UA_HEAD_SHAKE                , "HEAD_SHAKE"      },

    { loco::ANIM_HAND_SIGNAL_ENEMY_FORWARD    , "SIGNAL_ENEMY_FORWARD"        },

    { loco::ANIM_NULL                         ,  NULL }
};

//=======================================================================================

//==============================================================================
// friendly scientist locomotion states.
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

friendly_scientist_loco_play_anim::friendly_scientist_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

//=========================================================================
// IDLE
//=========================================================================

friendly_scientist_loco_idle::friendly_scientist_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

//=========================================================================
// MOVE
//=========================================================================

friendly_scientist_loco_move::friendly_scientist_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}


//==============================================================================
// friendly scientist loco
//==============================================================================

//==============================================================================

void  friendly_scientist_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid );

    // Lookup anims
    SetupAnimLookupTable( s_AnimsLookup );

    // Found wheels in geometry?
    if( m_WheelController.Init( this,                               // pLoco
                                GetAnimGroupHandle(),               // hAnimGroup
                                "Wheelchair_Turn_Loop",             // pTurnLoopSfx
                                "Wheelchair_Turn_Stop",             // pTurnStopSfx
                                "Wheelchair_Move_Loop",             // pMoveLoopSfx
                                "Wheelchair_Move_Stop" ) )          // pMoveStopSfx
    {                                
        m_Player.SetTrack( 8, &m_WheelController );
        SetAllowMotion( loco::MOTION_LEFT,  FALSE );
        SetAllowMotion( loco::MOTION_RIGHT, FALSE );
    }
    
    // Initialize loco
    SetAimerBoneMasks( FALSE, 0.0f ) ;
    SetMoveStyle( MOVE_STYLE_WALK );
}

//==============================================================================

