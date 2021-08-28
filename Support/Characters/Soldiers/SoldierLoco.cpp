//=========================================================================
//
//  soldierLoco.cpp
//
//=========================================================================


#include "SoldierLoco.hpp"



//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

soldier_loco::soldier_loco( void ) :
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

static loco::anim_lookup s_AnimsLookup[] =
{
    { loco::ANIM_CROUCH_IDLE                    ,   "CROUCHAIM_IDLE"                   },
    { loco::ANIM_CROUCH_IDLE_TURN_LEFT          ,   "CROUCHAIM_IDLE_TURN_LEFT"         },
    { loco::ANIM_CROUCH_IDLE_TURN_RIGHT         ,   "CROUCHAIM_IDLE_TURN_RIGHT"        },
    { loco::ANIM_CROUCH_IDLE_TURN_180           ,   "CROUCHAIM_IDLE_TURN_180"          },
    { loco::ANIM_CROUCH_IDLE_TURN_LEFT_180      ,   "CROUCHAIM_IDLE_TURN_LEFT_180"     },
    { loco::ANIM_CROUCH_IDLE_TURN_RIGHT_180     ,   "CROUCHAIM_IDLE_TURN_RIGHT_180"    },
    { loco::ANIM_CROUCH_IDLE_FIDGET             ,   "CROUCHAIM_IDLE_FIDGET"            },
    { loco::ANIM_CROUCH_MOVE_FRONT              ,   "CROUCHAIM_MOVE_FRONT"             },  
    { loco::ANIM_CROUCH_MOVE_LEFT               ,   "CROUCHAIM_MOVE_LEFT"              },  
    { loco::ANIM_CROUCH_MOVE_BACK               ,   "CROUCHAIM_MOVE_BACK"              },  
    { loco::ANIM_CROUCH_MOVE_RIGHT              ,   "CROUCHAIM_MOVE_RIGHT"             },  

    { loco::ANIM_NULL                         ,  NULL }
} ;

//==============================================================================
//==============================================================================
//==============================================================================
// soldier STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

soldier_loco_play_anim::soldier_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void soldier_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter() ;
}

//=========================================================================
// IDLE
//=========================================================================

soldier_loco_idle::soldier_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void soldier_loco_idle::OnEnter( void )
{
    loco_idle::OnEnter() ;
    soldier_loco& Base = *(soldier_loco*)&m_Base ;
    Base.SetAimerBoneMasks( TRUE, 0.5f ) ;
}

//=========================================================================
// MOVE
//=========================================================================

soldier_loco_move::soldier_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void soldier_loco_move::OnEnter( void )
{
    loco_move::OnEnter() ;
    soldier_loco& Base = *(soldier_loco*)&m_Base ;
    Base.SetAimerBoneMasks( TRUE, 0.5f ) ;
}

//==============================================================================
//==============================================================================
//==============================================================================
// soldier LOCO
//==============================================================================
//==============================================================================
//==============================================================================
//soldier_RELX_FB_ANY_Idle01

//==============================================================================

void  soldier_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid) ;
    
    // Initialize loco
    SetAimerBoneMasks( TRUE, 0.0f ) ;
    SetupAnimLookupTable( s_AnimsLookup ) ;
    SetMoveStyle( MOVE_STYLE_WALK );
}

//==============================================================================

void soldier_loco::ControlUpperBody( xbool bActive, f32 BlendTime )
{
    ( void )bActive;
    ( void )BlendTime;
    /*
    if( bActive )
    {
        SetAimBoneMasks( s_ArmedBoneMasks, BlendTime );
    }
    else
    {
        SetAimBoneMasks( s_UnarmedBoneMasks, BlendTime );
    }
    */
}

//==============================================================================

