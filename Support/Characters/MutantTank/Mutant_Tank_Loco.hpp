//=========================================================================
//
//  Mutant_Tank_Loco.hpp
//
//=========================================================================

#ifndef __MUTANT_TANK_LOCO_HPP__
#define __MUTANT_TANK_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"


//=========================================================================
// LOCO STATES
//=========================================================================

struct mutant_tank_loco_play_anim : public loco_play_anim
{
                            mutant_tank_loco_play_anim		( loco& Loco );
    virtual void            OnEnter					( void );
} ;

struct mutant_tank_loco_idle : public loco_idle
{
                            mutant_tank_loco_idle	        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct mutant_tank_loco_move : public loco_move
{
                            mutant_tank_loco_move	        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

//=========================================================================
// MUTANT_TANK LOCO
//=========================================================================

class mutant_tank_loco : public loco
{
// Defines
public:
    

// Functions
public:
                        mutant_tank_loco	( void );
    virtual void        OnInit              ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL ) ;
            void        ControlUpperBody    ( xbool bActive, f32 BlendTime = 0.5f );

    virtual void        OnEnumProp          ( prop_enum&    List );
    virtual xbool       OnProperty          ( prop_query&   I    );
            void        LoadTweaks          ( void );
            
// Private data
protected:

    mutant_tank_loco_play_anim   m_PlayAnim ;
    mutant_tank_loco_idle        m_Idle ;
    mutant_tank_loco_move        m_Move ;
};

//=========================================================================
// END
//=========================================================================
#endif
