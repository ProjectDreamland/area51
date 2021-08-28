// friendly_scientist : header file
/////////////////////////////////////////////////////////////////////////////

#ifndef __FRIENDLYSCIENTIST_H
#define __FRIENDLYSCIENTIST_H

#include "Characters\Character.hpp"
#include "Characters\Scientist\FriendlyScientistLoco.hpp"
#include "Characters\BaseStates\Character_Idle_State.hpp"
#include "Characters\BaseStates\Character_Alert_State.hpp"
#include "Characters\BaseStates\Character_Flee_State.hpp"
#include "Characters\BaseStates\Character_Cover_State.hpp"
#include "Characters\BaseStates\Character_Alarm_State.hpp"
#include "Characters\BaseStates\Character_Death_State.hpp"

//=====================================================================================
// friendly_scientist class
class friendly_scientist : public character
{
// Construction / destruction
public:

	// Constructs a friendly_scientist object.
    CREATE_RTTI( friendly_scientist, character, object )

    // object descriptor stuff
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

  
//=========================================================================
// Statics
//=========================================================================

	// Destroys a friendly_scientist object, handles cleanup and de-allocation.
	friendly_scientist();
	virtual ~friendly_scientist();

// Member functions
public:
    virtual void    OnInit                      ( void ) ;

    virtual void    OnEnumProp                  ( prop_enum&  rList     ) ;
    virtual xbool   OnProperty                  ( prop_query& rPropQuery ) ;

    virtual s32     GetNumberVoiceActors    ( void )    { return 1; }
    
// Member variables
protected:
    // Locomotion
    friendly_scientist_loco         m_Loco ;

    // States
    character_idle_state                m_Idle;
    character_alert_state               m_Alert;
    character_flee_state                m_Flee;
    character_cover_state               m_Cover;
    character_alarm_state               m_Alarm;
    character_death_state               m_Death;

};

//=================================================================================

/*inline
xbool friendly_scientist::GetCanFollow( void )
{
    return m_bCanFollow ;
}*/

//=================================================================================

#endif // __FRIENDLYSCIENTIST_H

