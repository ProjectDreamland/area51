// friendly_scientist_loco : header file
/////////////////////////////////////////////////////////////////////////////

#ifndef __FRIENDLYSCIENTISTLOCO_HPP
#define __FRIENDLYSCIENTISTLOCO_HPP

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"
#include "Loco\LocoWheelController.hpp"

/////////////////////////////////////////////////////////////////////////////
// friendly_scientist_loco class

//=========================================================================
// friendly scientist loco states
//=========================================================================

struct friendly_scientist_loco_play_anim : public loco_play_anim
{
                     friendly_scientist_loco_play_anim   ( loco& Loco );
};

//=========================================================================

struct friendly_scientist_loco_idle : public loco_idle
{
                     friendly_scientist_loco_idle       ( loco& Loco );
};

//=========================================================================

struct friendly_scientist_loco_move : public loco_move
{
                     friendly_scientist_loco_move       ( loco& Loco );
};

//=========================================================================

class friendly_scientist_loco : public loco
{
public:
    enum sci_anims
    {
        SIT_PC_TYPE,
        REPAIR_HANG_UNIT,
        PROGRAM_WARHEAD,
        MONITOR_CABINET,
        COMP_DESK_ONE,
        COMP_DESK_TWO,
        COMP_PANEL_ONE,
        USE_CLIPBOARD,
        USE_MICROSCOPE
    };
    
// Construction / destruction
public:

	// Constructs a friendly_scientist_loco object.
	friendly_scientist_loco();

	// Destroys a friendly_scientist_loco object, handles cleanup and de-allocation.
	virtual ~friendly_scientist_loco();

public:
    virtual void    OnInit           ( const geom* pGeom, const char* pAnimFileName, guid ObjectGuid = NULL );
    
// protected data
protected:
    
    // Loco controllers
    loco_wheel_controller               m_WheelController;
    
    // Loco states
    friendly_scientist_loco_play_anim   m_PlayAnim;
    friendly_scientist_loco_idle        m_Idle;
    friendly_scientist_loco_move        m_Move;
};

//=========================================================================

#endif // __FRIENDLYSCIENTISTLOCO_HPP

