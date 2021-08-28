///////////////////////////////////////////////////////////////////////////////
//
//  action_exit_turret.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_exit_turret_
#define _action_exit_turret_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Support\Objects\WeaponTRA.hpp "

//=========================================================================
// Check Property
//=========================================================================

class action_exit_turret : public actions_ex_base
{
public:
                    action_exit_turret                      ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_EXIT_TURRET;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Tel the player man a turret."; }

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual         const char*             GetDescription  ( void );

protected:

    guid            m_TurretObject;
    guid            m_AnchorObject;

};

#endif
