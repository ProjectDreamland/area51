///////////////////////////////////////////////////////////////////////////////
//
//  trigger_spatial_viewable.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_SPATIAL_VIEWABLE_
#define _TRIGGER_SPATIAL_VIEWABLE_

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Trigger\Trigger_Object.hpp"
#include "Trigger\Trigger_Spatial_Object.hpp"

//=========================================================================
// SPATIAL_TRIGGER_OBJECT : helps door by waking them when a player or npc gets close..
//=========================================================================

class trigger_spatial_viewable : public trigger_spatial_object
{
public:

     CREATE_RTTI( trigger_spatial_viewable, trigger_spatial_object , object )
                             trigger_spatial_viewable                 ( void );
                            ~trigger_spatial_viewable                 ( void );
                            
    virtual         s32         GetMaterial				                ( void ) const { return MAT_TYPE_NULL; }
           
	virtual			void	    OnEnumProp				                ( prop_enum& rList );
	virtual			xbool	    OnProperty				                ( prop_query& rPropQuery );

    virtual const object_desc&  GetTypeDesc                             ( void ) const;
    static  const object_desc&  GetObjectType                           ( void );
    
protected:
             
    virtual         void        OnInit					                ( void );
    virtual         void        OnKill                                  ( void );

#ifndef X_RETAIL
    virtual         void        OnDebugRender                           ( void );
#endif // X_RETAIL

    virtual         void        ExecuteLogic                            ( f32 DeltaTime );
    virtual         void        ActivateTrigger			                ( void );

protected:

    xbool                       m_IsRunning;                // Flag to determine if the object is in game mode
};

//=========================================================================
// END
//=========================================================================

#endif





















