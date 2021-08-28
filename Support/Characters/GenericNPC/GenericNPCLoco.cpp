//=========================================================================
//
//  GenericLoco.cpp
//
//=========================================================================


#include "GenericNPCLoco.hpp"
#include "GenericNPC.hpp"


//==============================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

generic_loco::generic_loco( void ) :
    loco(),
    m_PlayAnim  ( *this ),
    m_Idle      ( *this ),
    m_Move      ( *this )
{
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif



//==============================================================================
//==============================================================================
//==============================================================================
// GENERIC STATES
//==============================================================================
//==============================================================================
//==============================================================================

//=========================================================================
// PLAY ANIM
//=========================================================================

generic_loco_play_anim::generic_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

void generic_loco_play_anim::OnEnter( void )
{
    loco_play_anim::OnEnter() ;
}

//=========================================================================
// IDLE
//=========================================================================

generic_loco_idle::generic_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

void generic_loco_idle::OnEnter( void )
{
    loco_idle::OnEnter() ;
    
    generic_loco& Base = *(generic_loco*)&m_Base ;
    Base.SetAimerBoneMasks( TRUE, 0.5f ) ;
}

//=========================================================================
// MOVE
//=========================================================================

generic_loco_move::generic_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}

void generic_loco_move::OnEnter( void )
{
    loco_move::OnEnter() ;
    
    generic_loco& Base = *(generic_loco*)&m_Base ;
    Base.SetAimerBoneMasks( TRUE, 0.5f ) ;
}

//==============================================================================
//==============================================================================
//==============================================================================
// GENERIC LOCO
//==============================================================================
//==============================================================================
//==============================================================================
//GENERIC_RELX_FB_ANY_Idle01

//==============================================================================

void  generic_loco::OnInit( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid /*= NULL*/ )
{
    // Call base class
    loco::OnInit( pGeom, pAnimFileName, ObjectGuid ) ;

    // Setup controllers
    m_MaskController.SetBlendInTime (0.1f) ;
    m_MaskController.SetBlendOutTime(0.1f) ;

    // Initialize loco
    SetAimerBoneMasks( TRUE, 0.0f ) ;
    SetMoveStyle(MOVE_STYLE_WALK) ;

}
