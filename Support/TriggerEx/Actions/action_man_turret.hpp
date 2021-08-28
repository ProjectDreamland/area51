///////////////////////////////////////////////////////////////////////////////
//
//  action_man_turret.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_man_turret_
#define _action_man_turret_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Support\Objects\WeaponTRA.hpp "

//=========================================================================
// Check Property
//=========================================================================

class action_man_turret : public actions_ex_base
{
public:
                    action_man_turret                       ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_MAN_TURRET;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Tel the player man a turret."; }

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
    virtual         const char*             GetDescription  ( void );

protected:

    guid            m_TurretObject;
    guid            m_TurretObject2;
    guid            m_TurretObject3;
    guid            m_AnchorObject;
    guid            m_LeftBoundary;
    guid            m_RightBoundary;
    guid            m_UpperBoundary;
    guid            m_LowerBoundary;

};

#endif
