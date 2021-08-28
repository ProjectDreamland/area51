//=========================================================================
//
//  GrayLoco.hpp
//
//=========================================================================

#ifndef __GRAY_LOCO_HPP__
#define __GRAY_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"


//=========================================================================
// GRAY STATES
//=========================================================================

struct gray_loco_play_anim : public loco_play_anim
{
                            gray_loco_play_anim		( loco& Loco );
    virtual void            OnEnter					( void );
} ;

struct gray_loco_idle : public loco_idle
{
                            gray_loco_idle	        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct gray_loco_move : public loco_move
{
                            gray_loco_move	        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

//=========================================================================
// GRAY LOCO
//=========================================================================

class gray_loco : public loco
{
// Defines
public:
    

// Functions
public:
                        gray_loco	    ( void );
    virtual void        OnInit          ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL ) ;

// Private data
protected:

    gray_loco_play_anim   m_PlayAnim ;
    gray_loco_idle        m_Idle ;
    gray_loco_move        m_Move ;
};

//=========================================================================
// END
//=========================================================================
#endif
