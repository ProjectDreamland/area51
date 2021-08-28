#include "ProjTextureMgr.hpp"

//=========================================================================
// The global object
//=========================================================================

proj_texture_mgr    g_ProjTextureMgr;

//=========================================================================
// FUNCTIONS
//=========================================================================

proj_texture_mgr::proj_texture_mgr( void ) :
m_NLightProjections(0),
m_NShadowProjections(0)
{
}

//=========================================================================

proj_texture_mgr::~proj_texture_mgr( void )
{
}

//=========================================================================

void proj_texture_mgr::ClearProjTextures( void )
{
    m_NLightProjections  = 0;
    m_NShadowProjections = 0;
}

//=========================================================================

void proj_texture_mgr::AddProjLight( const matrix4&  L2W,
                                     radian          FOV,
                                     f32             Length,
                                     texture::handle Texture )
{
    ASSERT( m_NLightProjections < MAX_PROJ_LIGHTS );
    SetupProjection( m_LightProjections[m_NLightProjections], L2W, FOV, Length, Texture );
    m_NLightProjections++;
}

//=========================================================================

void proj_texture_mgr::AddProjShadow( const matrix4&  L2W,
                                      radian          FOV,
                                      f32             Length,
                                      texture::handle Texture )
{
    ASSERT( m_NShadowProjections < MAX_PROJ_SHADOWS );
    SetupProjection( m_ShadowProjections[m_NShadowProjections], L2W, FOV, Length, Texture );
    m_NShadowProjections++;
}

//=========================================================================

s32 proj_texture_mgr::CollectLights( const matrix4& L2W, const bbox& B, s32 MaxLightCount )
{
    (void)L2W;
    (void)B;
    (void)MaxLightCount;
    //#### TODO: Finish this function

    return 0;
}

//=========================================================================

void proj_texture_mgr::GetCollectedLight( matrix4& LightMatrix, xbitmap*& pBitmap )
{
    (void)LightMatrix;
    (void)pBitmap;
    //#### TODO: Finish this function
}

//=========================================================================

s32 proj_texture_mgr::CollectShadows( const matrix4& L2W, const bbox& B, s32 MaxLightCount )
{
    (void)L2W;
    (void)B;
    (void)MaxLightCount;
    //#### TODO: Finish this function

    return 0;
}

//=========================================================================

void proj_texture_mgr::GetCollectedShadow( matrix4& ShadMatrix, xbitmap*& pBitmap )
{
    (void)ShadMatrix;
    (void)pBitmap;
    //#### TODO: Finish this function
}


//=========================================================================

void proj_texture_mgr::SetupProjection ( projection&     Dest,
                                         const matrix4&  L2W,
                                         radian          FOV,
                                         f32             Length,
                                         texture::handle Texture )
{
    // set up the bitmap
    Dest.ProjTexture = Texture;
    
    // set up the view
    texture* pProjTexture = Texture.GetPointer();
    ASSERT( pProjTexture );
    xbitmap& ProjBMP = pProjTexture->m_Bitmap;
    Dest.ProjView.SetXFOV( FOV );
    Dest.ProjView.SetZLimits( 1.0f, Length );
    Dest.ProjView.SetViewport( 0, 0, ProjBMP.GetWidth(), ProjBMP.GetHeight() );
    Dest.ProjView.SetV2W( L2W );

    // set up the projection matrix
    Dest.ProjMatrix.Identity();
    //Dest.ProjMatrix.Scale( vector3( 0.5f, -0.5f, 1.0f ) );
    Dest.ProjMatrix.Scale( vector3( 0.5f, -0.5f, 1.0f ) );
    Dest.ProjMatrix.Translate( vector3( 0.5f, 0.5f, 0.0f ) );
    Dest.ProjMatrix *= Dest.ProjView.GetW2C();
}
