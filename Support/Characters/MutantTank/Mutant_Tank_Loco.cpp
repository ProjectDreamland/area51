//=========================================================================
//
//  Mutant_Tank_Loco.cpp
//
//=========================================================================


#include "Mutant_Tank_Loco.hpp"


//==============================================================================
//==============================================================================
//==============================================================================
// LOCO STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

mutant_tank_loco_play_anim::mutant_tank_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void mutant_tank_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter();
}

//=========================================================================
// IDLE
//=========================================================================

mutant_tank_loco_idle::mutant_tank_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void mutant_tank_loco_idle::OnEnter( void )
{
    // Call base class
    loco_idle::OnEnter();

    // Turn on the aimer    
    m_Base.SetAimerWeight( 1.0f, 0.5f );
}

//=========================================================================
// MOVE
//=========================================================================

mutant_tank_loco_move::mutant_tank_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void mutant_tank_loco_move::OnEnter( void )
{
    // Call base class
    loco_move::OnEnter();
    
    // Turn on the aimer    
    m_Base.SetAimerWeight( 1.0f, 0.5f );
}


//==============================================================================
//==============================================================================
//==============================================================================
// MUTANT_TANK LOCO
//==============================================================================
//==============================================================================
//==============================================================================


//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

mutant_tank_loco::mutant_tank_loco( void ) :
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

// Lookup general animation indices
static loco::anim_lookup s_AnimsLookup[] =
{
    //AS 8/28 this is here for compatablility with the current code.
    { loco::ANIM_RUN_IDLE                   ,       "WALK_IDLE"                 },
    { loco::ANIM_RUN_IDLE_TURN_180          ,       "WALK_IDLE_TURN_180"        },
    { loco::ANIM_RUN_IDLE_TURN_LEFT_180         ,   "WALK_IDLE_TURN_LEFT_180"   },
    { loco::ANIM_RUN_IDLE_TURN_RIGHT_180        ,   "WALK_IDLE_TURN_RIGHT_180"  },
    { loco::ANIM_RUN_IDLE_TURN_LEFT_180         ,   "WALK_IDLE_TURN_180_LEFT"   },
    { loco::ANIM_RUN_IDLE_TURN_RIGHT_180        ,   "WALK_IDLE_TURN_180_RIGHT"  },
    { loco::ANIM_RUN_IDLE_TURN_LEFT         ,       "WALK_IDLE_TURN_LEFT"       },
    { loco::ANIM_RUN_IDLE_TURN_RIGHT        ,       "WALK_IDLE_TURN_RIGHT"      },
    { loco::ANIM_RUN_MOVE_LEFT              ,       "WALK_MOVE_LEFT"            },
    { loco::ANIM_RUN_MOVE_RIGHT             ,       "WALK_MOVE_RIGHT"           },
    { loco::ANIM_RUN_MOVE_BACK              ,       "WALK_MOVE_BACK"            },

    { loco::ANIM_DEATH_HARD_SHOT_IN_BACK_HIGH   ,   "DEATH_SMALL_HEAD_BACK"     },
    { loco::ANIM_DEATH_HARD_SHOT_IN_BACK_MED    ,   "DEATH_SMALL_TORSO_BACK"    },
    { loco::ANIM_DEATH_HARD_SHOT_IN_BACK_LOW    ,   "DEATH_SMALL_TORSO_BACK"    },

    { loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_HIGH  ,   "DEATH_SMALL_HEAD_FRONT"    },
    { loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_MED   ,   "DEATH_SMALL_TORSO_FRONT"   },
    { loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_LOW   ,   "DEATH_SMALL_TORSO_FRONT"   },

    { loco::ANIM_CHARGE_IDLE                        ,   "WALK_IDLE"                 },
    { loco::ANIM_CHARGE_IDLE_TURN_180               ,   "WALK_IDLE_TURN_180"        },
    { loco::ANIM_CHARGE_IDLE_TURN_LEFT_180          ,   "WALK_IDLE_TURN_LEFT_180"   },
    { loco::ANIM_CHARGE_IDLE_TURN_RIGHT_180         ,   "WALK_IDLE_TURN_RIGHT_180"  },
    { loco::ANIM_CHARGE_IDLE_TURN_LEFT_180          ,   "WALK_IDLE_TURN_180_LEFT"   },
    { loco::ANIM_CHARGE_IDLE_TURN_RIGHT_180         ,   "WALK_IDLE_TURN_180_RIGHT"  },
    { loco::ANIM_CHARGE_IDLE_TURN_LEFT              ,   "WALK_IDLE_TURN_LEFT"       },
    { loco::ANIM_CHARGE_IDLE_TURN_RIGHT             ,   "WALK_IDLE_TURN_RIGHT"      },

    { loco::ANIM_CHARGE_FAST_IDLE                   ,   "WALK_IDLE"                 },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_180          ,   "WALK_IDLE_TURN_180"        },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_LEFT_180     ,   "WALK_IDLE_TURN_LEFT_180"   },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_RIGHT_180    ,   "WALK_IDLE_TURN_RIGHT_180"  },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_LEFT_180     ,   "WALK_IDLE_TURN_180_LEFT"   },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_RIGHT_180    ,   "WALK_IDLE_TURN_180_RIGHT"  },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_LEFT         ,   "WALK_IDLE_TURN_LEFT"       },
    { loco::ANIM_CHARGE_FAST_IDLE_TURN_RIGHT        ,   "WALK_IDLE_TURN_RIGHT"      },

    { loco::ANIM_DEATH_LIGHT_SHOT_IN_BACK_LOW   ,   "DEATH_SMALL_TORSO_BACK"    },
    { loco::ANIM_DEATH_LIGHT_SHOT_IN_FRONT_LOW  ,   "DEATH_SMALL_TORSO_FRONT"   },

    { loco::ANIM_NULL                       ,  NULL                             }
};

//==============================================================================

void  mutant_tank_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid );

    // Lookup anims
    SetupAnimLookupTable(s_AnimsLookup);

    // Initialize loco
    SetAimerBoneMasks( TRUE, 0.0f );
    SetMoveStyle(MOVE_STYLE_WALK) ;
    SetAllowMotion( loco::MOTION_LEFT,  FALSE );
    SetAllowMotion( loco::MOTION_RIGHT, FALSE );

    // Load tweaks
    LoadTweaks();
}

//==============================================================================

void mutant_tank_loco::ControlUpperBody( xbool bActive, f32 BlendTime )
{
    SetAimerBoneMasks( bActive, BlendTime );
}

//==============================================================================

void mutant_tank_loco::OnEnumProp( prop_enum& List )
{
    // Do nothing because all theta loco properties are now in tweak excel spreadsheet
    (void)List;
}

//==============================================================================

xbool mutant_tank_loco::OnProperty( prop_query& I )
{
    // Do nothing because all theta loco properties are now in tweak excel spreadsheet
    (void)I;
    return FALSE;
}

//==============================================================================

void mutant_tank_loco::LoadTweaks( void )
{
    // Load physics tweaks
    m_Physics.SetColHeight( GetTweakF32( "THETA_CollisionHeight", m_Physics.GetColHeight() ) );
    m_Physics.SetColRadius( GetTweakF32( "THETA_CollisionRadius", m_Physics.GetColRadius() ) );
    m_Physics.SetActorCollisionRadius( GetTweakF32( "THETA_ActorCollisionRadius", m_Physics.GetActorCollisionRadius() ) );
}

//==============================================================================

