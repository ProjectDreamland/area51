//==============================================================================
//
//  InvisWall.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================
#ifndef __INVISWALL__ 
#define __INVISWALL__ 

//==------------------------------------------------------------------------
// Includes
//==------------------------------------------------------------------------
#include "Obj_mgr\obj_mgr.hpp"

//==------------------------------------------------------------------------
// Defines
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Globals
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Prototypes
//==------------------------------------------------------------------------


// Class invisible_wall_obj
class invisible_wall_obj : public object
{
public:
    CREATE_RTTI( invisible_wall_obj, object, object )

	                            invisible_wall_obj          (void);
                               ~invisible_wall_obj          (void);

    virtual bbox                GetLocalBBox    ( void ) const;
    virtual s32                 GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    virtual void                OnEnumProp      ( prop_enum& List );
    virtual xbool               OnProperty      ( prop_query& I );
    virtual void                OnPolyCacheGather   ( void );
    virtual void                OnMove          (const vector3& NewPos );
    virtual void                OnTransform     (const matrix4& L2W );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

            xbool               GetOccluderPoints   ( vector3* pPoints ) const;

protected:

#if !defined( CONFIG_RETAIL )
    virtual void            OnDebugRender       ( void );
    virtual void            OnRender            ( void );
    virtual void            OnRenderTransparent ( void );
#endif // !defined( CONFIG_RETAIL )

    virtual void            OnColCheck          ( void );
    virtual void            OnKill              ( void );
    virtual void            OnColNotify         ( object& Object );    

    f32                     m_Width;
    f32                     m_Height;
    f32                     m_Depth;
    vector3                 m_CornerPos[8];
    u32                     m_bIsOccluder:1;

};


#endif // INVISWALL
