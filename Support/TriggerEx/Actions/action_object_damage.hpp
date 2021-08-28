///////////////////////////////////////////////////////////////////////////////
//
//  action_object_damage.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_object_damage_
#define _action_object_damage_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"

//=========================================================================
// Check Property
//=========================================================================


class action_object_damage : public actions_ex_base
{
public:

    enum hit_location_type
    {
        HIT_HEAD,
        HIT_CENTER,
        HIT_FEET,
    };
                    action_object_damage                    ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_DAMAGE_OBJECT;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Cause pause to an object."; }
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
    generic_pain_type               m_GenericPainType;   
    hit_location_type               m_HitLocationType;   
};

#endif
