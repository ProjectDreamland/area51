//=========================================================================
//
//  GrayLoco.cpp
//
//=========================================================================


#include "GrayLoco.hpp"


//==============================================================================
//==============================================================================
//==============================================================================
// HAZMAT STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

gray_loco_play_anim::gray_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void gray_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter() ;
} 

//=========================================================================
// IDLE
//=========================================================================

gray_loco_idle::gray_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void gray_loco_idle::OnEnter( void )
{
    loco_idle::OnEnter() ;
}

//=========================================================================
// MOVE
//=========================================================================

gray_loco_move::gray_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void gray_loco_move::OnEnter( void )
{
    loco_move::OnEnter() ;
}


//==============================================================================
//==============================================================================
//==============================================================================
// GRAY LOCO
//==============================================================================
//==============================================================================
//==============================================================================


//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

gray_loco::gray_loco( void ) :
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

// TO DO - When anims are named with new scheme, this table can vanish!

// Lookup general animation indices
static loco::anim_lookup s_AnimsLookup[] =
{
    { loco::ANIM_WALK_IDLE                      ,   "RUN_IDLE"                  },
    { loco::ANIM_WALK_IDLE_TURN_LEFT            ,   "RUN_IDLE_TURN_LEFT"        },
    { loco::ANIM_WALK_IDLE_TURN_RIGHT           ,   "RUN_IDLE_TURN_RIGHT"       },
    { loco::ANIM_WALK_IDLE_TURN_180             ,   "RUN_IDLE_TURN_180"         },
   
    { loco::ANIM_NULL                          ,  NULL                          }
} ;

//==============================================================================

void  gray_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid ) ;

    // Lookup anims
    SetupAnimLookupTable(s_AnimsLookup) ;
    
    // Initialize loco
    SetAimerBoneMasks( TRUE, 0.5f ) ;
    SetMoveStyle(MOVE_STYLE_WALK) ;
}

//==============================================================================
