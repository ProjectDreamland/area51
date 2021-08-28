///////////////////////////////////////////////////////////////////////////////
//
//  action_object_move_relative.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_object_move_relative_
#define _action_object_move_relative_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================== ==============================================
// action_object_move
//=========================================================================

class action_object_move_relative : public actions_ex_base
{
public:
                    action_object_move_relative         ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_MOVE_OBJECT_RELATIVE;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Move an object to a marker, relative to a 2nd marker."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef0   ( xstring& Desc ) { Desc = "Object error: "; return &m_ObjectAffecter; }
    virtual         guid*               GetGuidRef0     ( xstring& Desc ) { Desc = "Destination marker error: "; return &m_DestMarker; }
    virtual         guid*               GetGuidRef1     ( xstring& Desc ) { Desc = "Relative marker error: "; return &m_RelativeMarker; }
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    object_affecter m_ObjectAffecter;
    guid            m_DestMarker;
    guid            m_RelativeMarker;
};


#endif
