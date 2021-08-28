///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_attack_guid.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_attack_guid_
#define _action_ai_attack_guid_

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"

//=========================== ==============================================
// action_ai_attack_guid
//=========================================================================

class action_ai_attack_guid : public action_ai_base
{
public:
                    action_ai_attack_guid               ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_ATTACK_GUID;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Attack a particular guid."; } 
    virtual         const char*         GetDescription  ( void );

    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef1   ( xstring& Desc )  { Desc = "Target object error: "; return &m_TargetAffecter; }
#endif

    virtual         ai_action_types     GetAIActionType ( void ) { return AI_ATTACK_GUID; }

                    guid                GetTargetGuid   ( void );

protected:

    object_affecter m_TargetAffecter;

};


#endif
