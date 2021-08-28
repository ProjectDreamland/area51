//=========================================================================
//
//  GruntLoco.cpp
//
//=========================================================================


#include "GruntLoco.hpp"
#include "characters\Grunt\Grunt.hpp"


//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

grunt_loco::grunt_loco( void ) :
    loco(),
    m_PlayAnim  ( *this ),
    m_Idle      ( *this ),
    m_Move      ( *this ),
    m_bRunGunAim(FALSE)
{
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//==============================================================================

// TO DO - When anims are named with new scheme, this table can vanish!

// Lookup animation indices

static loco::anim_lookup s_AnimsLookup[] =
{
    //AS 12/8
    { loco::ANIM_WALK_IDLE                       ,   "RUN_IDLE"                  },
    { loco::ANIM_WALK_IDLE_TURN_LEFT             ,   "RUN_IDLE_TURN_LEFT"        },
    { loco::ANIM_WALK_IDLE_TURN_LEFT_180         ,   "RUN_IDLE_TURN_LEFT_180"    },
    { loco::ANIM_WALK_IDLE_TURN_RIGHT            ,   "RUN_IDLE_TURN_RIGHT"       },
    { loco::ANIM_WALK_IDLE_TURN_RIGHT_180        ,   "RUN_IDLE_TURN_RIGHT_180"   },

    { loco::ANIM_LOST_TARGET                     ,   "WALK_IDLE_FIDGET2"         },
    
    { loco::ANIM_NULL                            ,   NULL                        }
} ;

//==============================================================================
//==============================================================================
//==============================================================================
// GRUNT STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

grunt_loco_play_anim::grunt_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void grunt_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter() ;
}

//=========================================================================
// IDLE
//=========================================================================

grunt_loco_idle::grunt_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void grunt_loco_idle::OnEnter( void )
{
    loco_idle::OnEnter() ;
    
    grunt_loco& Base = *(grunt_loco*)&m_Base ;
    Base.SetAimerBoneMasks( TRUE, 0.5f ) ;
}

//=========================================================================
// MOVE
//=========================================================================

grunt_loco_move::grunt_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void grunt_loco_move::OnEnter( void )
{
    loco_move::OnEnter() ;
    
    grunt_loco& Base = *(grunt_loco*)&m_Base ;
    Base.SetAimerBoneMasks( TRUE, 0.5f ) ;
}

//==============================================================================
//==============================================================================
//==============================================================================
// GRUNT LOCO
//==============================================================================
//==============================================================================
//==============================================================================

void  grunt_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid ) ;

    // Lookup anims
    SetupAnimLookupTable(s_AnimsLookup) ;

    // Setup controllers
    m_MaskController.SetBlendInTime (0.1f) ;
    m_MaskController.SetBlendOutTime(0.1f) ;

    // Initialize loco
    SetAimerBoneMasks( TRUE, 0 ) ;
    SetMoveStyle(MOVE_STYLE_WALK) ;
}

//==============================================================================

