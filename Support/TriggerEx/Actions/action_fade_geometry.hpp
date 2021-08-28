///////////////////////////////////////////////////////////////////////////////
//
//  action_fade_geometry.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_fade_geometry_
#define _action_fade_geometry_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class action_fade_geometry : public actions_ex_base
{
public:
    action_fade_geometry                                    ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_FADE_GEOMETRY;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Fade an object in or out."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    enum fade_codes
    { 
        INAVLID_FADE_CODE = -1,
        CODE_FADE_IN,
        CODE_FADE_OUT,
        ACTIVATE_CODES_END
    };

    s32              m_FadeCode;
    f32              m_FadeTime;
    object_affecter  m_ObjectAffecter;
};

#endif // _action_fade_geometry_
