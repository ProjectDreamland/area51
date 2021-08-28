//==============================================================================
//
//  GZCoreObj.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================
#ifndef __GZCOREOBJ__ 
#define __GZCOREOBJ__ 

//==------------------------------------------------------------------------
// Includes
//==------------------------------------------------------------------------
#include "obj_mgr\obj_mgr.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "audiomgr\AudioMgr.hpp"
#include "objects\superdestructible.hpp"

//==------------------------------------------------------------------------
// Defines
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Globals
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Prototypes
//==------------------------------------------------------------------------


// Class ground_zero_core_obj
class ground_zero_core_obj : public object
{
    CREATE_RTTI( ground_zero_core_obj, object, object )

public:

    ground_zero_core_obj        ( void );
    ~ground_zero_core_obj        ( void );

    virtual bbox        GetLocalBBox        ( void ) const;

    virtual void        OnInit              ( void );
    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        OnPain              ( const pain& Pain );

    virtual void        OnDebugRender       ( void );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual s32                 GetMaterial     ( void ) const { return MAT_TYPE_CONCRETE;}
    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );
    virtual const char*         GetLogicalName  ( void )   { return "GZ_CORE_OBJ"; }

    object*                     GetSafeObject   ( guid GUID );

private:

    guid    m_MarkerStart;
    guid    m_MarkerEnd;
    guid    m_SuperDGUID;
    guid    m_GroupShieldGUID;
    guid    m_FxGUID;
    guid    m_FxGUIDShort;

    guid    m_SoundEmitterShield;
    guid    m_SoundEmitterFlare;

    xbool   m_LogicActive;
    xbool   m_ShieldDetected;

    super_destructible_obj*     m_SuperD_obj;

};


#endif // GZCOREOBJ
