///////////////////////////////////////////////////////////////////////////////
//
//  action_object_move.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_object_move_
#define _action_object_move_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================== ==============================================
// action_object_move
//=========================================================================

class action_object_move : public actions_ex_base
{
public:
                    action_object_move                 ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_MOVE_OBJECT;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Move an object to a marker."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef0   ( xstring& Desc ) { Desc = "Object error: "; return &m_ObjectAffecter; }
    virtual         guid*               GetGuidRef0     ( xstring& Desc ) { Desc = "Marker object error: "; return &m_Marker; }
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    object_affecter m_ObjectAffecter;
    guid            m_Marker;                            //Position
    xbool           m_bAlignToMarker;
};


#endif
