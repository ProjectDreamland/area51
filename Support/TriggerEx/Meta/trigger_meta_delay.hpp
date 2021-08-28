///////////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_delay.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _trigger_meta_delay_
#define _trigger_meta_delay_

//=========================================================================
// INCLUDES
//=========================================================================

#include "trigger_meta_base.hpp"

//=========================== ==============================================
// trigger_meta_delay
//=========================================================================

class trigger_meta_delay : public trigger_meta_base
{
public:
    CREATE_RTTI( trigger_meta_delay, trigger_meta_base, actions_ex_base )

                    trigger_meta_delay                   ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_META_DELAY;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Set a META DELAY."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );
    virtual         void                OnActivate      ( xbool Flag );

protected:

    f32             m_DelayTime;
    f32             m_TimeDelayingSoFar;
};


#endif
