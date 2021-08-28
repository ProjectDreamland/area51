//==============================================================================
//  VolumetricLight.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is for generating a "volumetric light". That is, it fakes a light
//  source projecting through dust, mist, or whatever that causes the light to
//  be scattered. Artists have been faking this with a sprite effect, but this
//  version will *hopefully* be a little bit less fill-rate intensive.
//==============================================================================

#ifndef __VOLUMETRICLIGHT_HPP__
#define __VOLUMETRICLIGHT_HPP__

#include "Obj_mgr\obj_mgr.hpp"
#include "Render\Texture.hpp"

class volumetric_light_obj : public object
{
public:

    CREATE_RTTI( volumetric_light_obj, object, object )

                            volumetric_light_obj    ( void );
    virtual                ~volumetric_light_obj    ( void );
    virtual bbox            GetLocalBBox            ( void ) const;
    virtual s32             GetMaterial             ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp              ( prop_enum& List );
    virtual xbool           OnProperty              ( prop_query& I );

    virtual const object_desc&  GetTypeDesc         ( void ) const;
    static  const object_desc&  GetObjectType       ( void );

#ifdef X_EDITOR
    virtual void            OnDebugRender   ( void );
#endif // X_EDITOR

protected:
    virtual void            OnRender            ( void );
    virtual void            OnRenderTransparent ( void );

    void                    SetupData( void );

    f32                     m_StartSize;
    f32                     m_EndSize;
    f32                     m_Length;
    xcolor                  m_StartColor;
    xcolor                  m_EndColor;
    s32                     m_nSprites;
    texture::handle         m_ProjTexHandle;
    xbool                   m_bRender;

    s32                     m_nBytesAlloced;
    byte*                   m_pData;
};

#endif // __VOLUMETRICLIGHT_HPP__