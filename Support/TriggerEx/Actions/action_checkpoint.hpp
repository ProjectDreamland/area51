///////////////////////////////////////////////////////////////////////////////
//
//  action_checkpoint.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __action_checkpoint
#define __action_checkpoint

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================================================================

class action_checkpoint : public actions_ex_base
{
public:
                    action_checkpoint                       ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_CHECKPOINT;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Save a checkpoint."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
   
protected:
    
    s32     m_CPTableName;
    s32     m_CPTitleName;
    guid    m_TriggerGuidRestore;       
    guid    m_TriggerGuidDebugSkip;       
};

#endif
