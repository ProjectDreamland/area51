#ifndef PROJTEXTUREMGR_HPP
#define PROJTEXTUREMGR_HPP

#include "x_math.hpp"
#include "e_View.hpp"
#include "Texture.hpp"

class proj_texture_mgr
{
/*
public:
             proj_texture_mgr( void );
    virtual ~proj_texture_mgr( void );

    void    SetProjLight    ( const matrix4& L2W, radian FOV, f32 Length, texture::handle Texture );
    void    SetProjShadow   ( const matrix4& L2W, radian FOV, f32 Length, texture::handle Texture );

    void    BeginShadowAccumulation ( void );
    void    AccumulateCharShadow    
    */

public:
    enum
    {
        MAX_PROJ_LIGHTS = 10,
        MAX_PROJ_SHADOWS = 10,
    };


             proj_texture_mgr( void );
    virtual ~proj_texture_mgr( void );

    void    ClearProjTextures       ( void );
    void    AddProjLight            ( const matrix4& L2W, radian FOV, f32 Length, texture::handle Texture );
    void    AddProjShadow           ( const matrix4& L2W, radian FOV, f32 Length, texture::handle Texture );
    
    s32     CollectLights           ( const matrix4& L2W, const bbox& B, s32 MaxLightCount = 1 );
    void    GetCollectedLight       ( matrix4& LightMatrix, xbitmap*& pBitmap );

    s32     CollectShadows          ( const matrix4& L2W, const bbox& B, s32 MaxShadowCount = 1 );
    void    GetCollectedShadow      ( matrix4& ShadMatrix, xbitmap*& pBitmap );

protected:
    struct projection
    {
        view            ProjView;
        matrix4         ProjMatrix;
        texture::handle ProjTexture;
    };

    void        SetupProjection ( projection&     Dest,
                                  const matrix4&  L2W,
                                  radian          FOV,
                                  f32             Length,
                                  texture::handle Texture );

    s32         m_NLightProjections;
    s32         m_NShadowProjections;
    projection  m_LightProjections[MAX_PROJ_LIGHTS];
    projection  m_ShadowProjections[MAX_PROJ_SHADOWS];
};

extern proj_texture_mgr    g_ProjTextureMgr;

#endif // PROJTEXTUREMGR_HPP