//=========================================================================
//
//  GenericLoco.cpp
//
//=========================================================================

#ifndef __GENERIC_LOCO_HPP__
#define __GENERIC_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"


//=========================================================================
// GENERIC STATES
//=========================================================================

struct generic_loco_play_anim : public loco_play_anim
{
                            generic_loco_play_anim   ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct generic_loco_idle : public loco_idle
{
                            generic_loco_idle        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct generic_loco_move : public loco_move
{
                            generic_loco_move        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

//=========================================================================
// GENERIC LOCO
//=========================================================================

class generic_loco : public loco
{
// Defines
public:

// Structures
public:

    // Movement lookup anims
    struct move_anims
    {
        // Data
        s32 iI, iTL, iTR, iT180 ;   // Idle anims
        s32 iF, iB,  iL,  iR ;      // Move anims

        // Functions
        move_anims()
        {
            // Clear anims
            x_memset(this, -1, sizeof(move_anims)) ;
        }
    } ;


// Functions
public:
                        generic_loco        ( void );
    virtual void        OnInit              ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL ) ;

// Private data
protected:

    generic_loco_play_anim  m_PlayAnim ;
    generic_loco_idle       m_Idle ;
    generic_loco_move       m_Move ;

};

//=========================================================================
// END
//=========================================================================
#endif
