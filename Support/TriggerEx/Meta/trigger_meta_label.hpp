///////////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_label.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _trigger_meta_label_
#define _trigger_meta_label_

//=========================================================================
// INCLUDES
//=========================================================================

#include "trigger_meta_base.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================== ==============================================
// trigger_meta_label
//=========================================================================

class trigger_meta_label : public trigger_meta_base
{
public:
    CREATE_RTTI( trigger_meta_label, trigger_meta_base, actions_ex_base )

                    trigger_meta_label                   ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_META_LABEL;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Set a META LABEL."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

    inline          const char*         GetLabel        ( void );

protected:

    s32             m_Label;
};


//=============================================================================

inline const char* trigger_meta_label::GetLabel( void )
{
    if (m_Label != -1)
        return g_StringMgr.GetString(m_Label); 
    else
        return "";
}

#endif
