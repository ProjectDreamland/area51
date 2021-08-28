///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Door_Helper_Object.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_DOOR_HELPER_OBJECT_
#define _TRIGGER_DOOR_HELPER_OBJECT_

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Trigger\Trigger_Object.hpp"
#include "Trigger\Trigger_Spatial_Object.hpp"

//=========================================================================
// SPATIAL_TRIGGER_OBJECT : helps door by waking them when a player or npc gets close..
//=========================================================================

class trigger_door_helper_object : public trigger_spatial_object
{
public:

     CREATE_RTTI( trigger_door_helper_object, trigger_spatial_object , object )
                             trigger_door_helper_object                 ( void );
                            ~trigger_door_helper_object                 ( void );
                            
    virtual         s32         GetMaterial				                ( void ) const { return MAT_TYPE_NULL; }
           
	virtual			void	    OnEnumProp				                ( prop_enum& rList );
	virtual			xbool	    OnProperty				                ( prop_query& rPropQuery );
	
    virtual const object_desc&  GetTypeDesc                             ( void ) const;
    static  const object_desc&  GetObjectType                           ( void );
    
    //Interface for door object..
    
                    void        Setup                                   ( const bbox& BBox, const guid& DoorGuid );
                    void        Render                                  ( u32 ParentAttribs );
                    void        Sync                                    ( const bbox& BBox );
protected:
             
    virtual         void        OnInit					                ( void );
    virtual         void        OnKill                                  ( void );
 
    virtual         void        ExecuteLogic                            ( f32 DeltaTime );
    virtual         void        ActivateTrigger			                ( void );

protected:

    guid        m_DoorGuid;     // Guid of the door were hooked too...
};

//=========================================================================
// END
//=========================================================================

#endif





















