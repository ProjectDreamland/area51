///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_CHANGE_PLAYER_HEALTH
#define _TRIGGER_ACTIONS_CHANGE_PLAYER_HEALTH

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// CHANGE_PLAYER_HEALTH
//=========================================================================

class change_player_health : public actions_base
{
public:
                    change_player_health            ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Change the Players Health"; }
    virtual         const char*         GetTypeInfo ( void )    { return "Changes the players health."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_CHANGE_PLAYER_HEALTH;}

protected:
    
    f32         m_Health;                           // Players Health to change by
    xbool       m_ShakeView;                        // Flag which will shake the players view...
	f32			m_ShakeTime;						// How long to shake the view.
	f32			m_ShakeForce;						// Amount of force to shake the view

};

#endif
