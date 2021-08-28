//=========================================================================
//
//  SoldierLoco.cpp
//
//=========================================================================

#ifndef __SOLDIER_LOCO_HPP__
#define __SOLDIER_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"


//=========================================================================
// BlackOpps STATES
//=========================================================================

struct soldier_loco_play_anim : public loco_play_anim
{
                            soldier_loco_play_anim  ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct soldier_loco_idle : public loco_idle
{
                            soldier_loco_idle       ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct soldier_loco_move : public loco_move
{
                            soldier_loco_move       ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

//=========================================================================
// soldier LOCO
//=========================================================================

class soldier_loco : public loco
{
// Defines
public:
    

// Functions
public:
                        soldier_loco       ( void );
    virtual void        OnInit             ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL ) ;
            void        ControlUpperBody   ( xbool bActive, f32 BlendTime = 0.5f );

// Private data
protected:

    soldier_loco_play_anim   m_PlayAnim ;
    soldier_loco_idle        m_Idle ;
    soldier_loco_move        m_Move ;
};

//=========================================================================
// END
//=========================================================================
#endif
