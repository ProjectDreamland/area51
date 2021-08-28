///////////////////////////////////////////////////////////////////////////////
//
//  action_create_template.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_create_template_
#define _action_create_template_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================== ==============================================
// action_create_template
//=========================================================================

class action_create_template : public actions_ex_base
{
public:
                    action_create_template              ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_CREATE_TEMPLATE;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Create an object from a template."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32*                GetTemplateRef  ( xstring& Desc ) { Desc = "Template error: "; return &m_TemplateIndex; }
    virtual         guid*               GetGuidRef0     ( xstring& Desc ) { Desc = "Marker object error: "; return &m_Marker; }
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL

#ifdef X_EDITOR
    virtual         void                EditorPreGame   ( void );
#endif // X_EDITOR

protected:
                    xbool               AddGuidToGroup  ( const guid& Guid );


protected:

    s32         m_StorageIndex;
    s32         m_TemplateIndex;
    
    guid        m_Marker;                            //Position
    guid        m_Group;    
};


#endif
