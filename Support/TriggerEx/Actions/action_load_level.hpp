///////////////////////////////////////////////////////////////////////////////
//
//  action_load_level.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_load_level_
#define _action_load_level_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================== ==============================================
// action_load_level
//=========================================================================

class action_load_level : public actions_ex_base
{
public:
                    action_load_level              ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_LOAD_LEVEL;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Load a new level (GameSide only)"; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    s32         m_StorageIndex;
};


#endif
