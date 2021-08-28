//==============================================================================
//
//  feedbackemitter.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================
#ifndef __FEEDBACKEMITTER__ 
#define __FEEDBACKEMITTER__ 

//==------------------------------------------------------------------------
// Includes
//==------------------------------------------------------------------------
#include "Obj_mgr\obj_mgr.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\Support\TriggerEx\Affecters\object_affecter.hpp"
//==------------------------------------------------------------------------
// Defines
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Globals
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Prototypes
//==------------------------------------------------------------------------


// Class feedback_emitter
class feedback_emitter : public object
{

public:

    feedback_emitter            ( void );
    ~feedback_emitter            ( void );

    enum feedback_emitter_spatial_types
    {
        SPATIAL_TYPES_INVALID = -1,
        SPATIAL_TYPE_AXIS_CUBE,
        SPATIAL_TYPE_SPHERICAL,
        SPATIAL_TYPES_END
    };

    CREATE_RTTI( feedback_emitter, object, object )

    virtual bbox        GetLocalBBox        ( void ) const;

    virtual	void	    OnColCheck			( void );
    virtual void        OnColNotify         ( object& Object );
    virtual void        OnActivate          ( xbool Flag );  
    void                LogicCheckOnActivate( void );
    virtual void        OnInit              ( void );
    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    xbool               QueryPlayerInVolume ( void );
    xbool               QueryObjectInVolume ( object* pObject );


#if !defined( CONFIG_RETAIL )
    void                OnRenderSpatial     ( void );
#endif // !defined( CONFIG_RETAIL )

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual s32                 GetMaterial     ( void ) const { return MAT_TYPE_CONCRETE;}
    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );
    virtual const char*         GetLogicalName  ( void )   { return "GZ_CORE_OBJ"; }

    xbool                       m_DrawActivationIcon;                       // Debug functionality

private:

    f32                         m_Dimensions[3];
    xbool                       m_bActive;
    f32                         m_TimeDelay;
    f32                         m_TimeSinceLastDamage;
    xbool                       m_bDoingFeedBack;
    f32                         m_RumbleStrangth;

    object_affecter             m_FeedBackAnchorAffecter;
    f32                         m_YOffset;


    feedback_emitter_spatial_types                      m_SpatialType;
    static enum_table<feedback_emitter_spatial_types>   m_SpatialTypeList; 

};


#endif // FEEDBACKEMITTER
