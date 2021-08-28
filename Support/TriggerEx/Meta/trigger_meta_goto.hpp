///////////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_goto.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _trigger_meta_goto_
#define _trigger_meta_goto_

//=========================================================================
// INCLUDES
//=========================================================================

#include "trigger_meta_base.hpp"
#include "..\Affecters\conditional_affecter.hpp"

//=========================== ==============================================
// trigger_meta_goto
//=========================================================================

class trigger_meta_goto : public trigger_meta_base
{
public:
    CREATE_RTTI( trigger_meta_goto, trigger_meta_base, actions_ex_base )

                    trigger_meta_goto                   ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_META_GOTO;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Set a META GOTO (with possible condition)."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

protected:

    conditional_affecter                m_ConditionAffecter;
    s32                                 m_Label;

                    xbool               SetTriggerActionIndexToLabel        ( const char* pName );
};


#endif
