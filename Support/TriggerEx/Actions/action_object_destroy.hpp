///////////////////////////////////////////////////////////////////////////////
//
//  action_object_destroy.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_object_destroy_
#define _action_object_destroy_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class action_object_destroy : public actions_ex_base
{
public:
                    action_object_destroy                   ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_DESTORY_OBJECT;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Destroy an object."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Object error: "; return &m_ObjectAffecter; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    object_affecter  m_ObjectAffecter;

};

#endif
