///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_CREATE_OBJECT_FROM_TEMPLATE
#define _TRIGGER_ACTIONS_CREATE_OBJECT_FROM_TEMPLATE

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// CREATE_OBJECT_FROM_TEMPLATE
//=========================================================================

class create_object_from_template : public actions_base
{
public:
                    create_object_from_template     ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Create Template Object"; }
    virtual         const char*         GetTypeInfo ( void )    { return "Creates an object from a template (blueprint)."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_CREATE_OBJECT_FROM_TEMPLATE;}

#ifdef WIN32
    virtual         void                EditorPreGame   ( void );
#endif
    
protected:

    big_string  m_TemplateID;                        //Template ID
    vector3     m_Position;                          //Position
    radian3     m_Orientation;                       //Direction
    vector3     m_RandomVectorExtent;                //Random Vector extent.
};


#endif
