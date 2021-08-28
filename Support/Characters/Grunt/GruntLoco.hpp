//=========================================================================
//
//  GruntLoco.cpp
//
//=========================================================================

#ifndef __GRUNT_LOCO_HPP__
#define __GRUNT_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"


//=========================================================================
// GRUNT STATES
//=========================================================================

struct grunt_loco_play_anim : public loco_play_anim
{
                            grunt_loco_play_anim   ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct grunt_loco_idle : public loco_idle
{
                            grunt_loco_idle        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

struct grunt_loco_move : public loco_move
{
                            grunt_loco_move        ( loco& Loco );
    virtual void            OnEnter                 ( void );
} ;

//=========================================================================
// GRUNT LOCO
//=========================================================================

class grunt_loco : public loco
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
                        grunt_loco        ( void );
    virtual void        OnInit              ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL ) ;
//            void        ControlUpperBody    ( xbool bActive, f32 BlendTime = 0.5f );

// Private data
protected:

    grunt_loco_play_anim  m_PlayAnim ;
    grunt_loco_idle       m_Idle ;
    grunt_loco_move       m_Move ;
    xbool                   m_bRunGunAim ;
};

//=========================================================================
// END
//=========================================================================
#endif
