///////////////////////////////////////////////////////////////////////////////
//
//  action_mission_failed.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_mission_failed_
#define _action_mission_failed_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================================================================
// Check Property
//=========================================================================

class action_mission_failed : public actions_ex_base
{
public:
                    action_mission_failed                   ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_MISSION_FAILED;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Set off the \"mission failed\" death sequence."; }

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
    virtual         const char*             GetDescription  ( void );

protected:
    s32     m_TableName;
    s32     m_ReasonName;


};

#endif
