//=========================================================================
//
// VideoWall.cpp
//
//
//  TODO:
//  Fix occlution(VVIS) video rendering.
//  Make custom Editor icon.
//  Add a spatial audio.
//  Add smooth transitions, maybe fading, noise or other.
//  Make video folder selectable in Editor.
//  Get running on PS2 ????
//
//
//==============================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "VideoWall.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "x_files.hpp"
#include "x_threads.hpp"
#include "Entropy.hpp"
#include "audio/audio_hardware.hpp"

#ifdef TARGET_XBOX
#include "Entropy\xbox\xbox_private.hpp"
#endif

//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

#if defined(TARGET_PC) && defined(X_BINK_EDITOR)
static void* RADLINK s_malloc(U32 bytes)
{
    return x_malloc(bytes);
}

static void RADLINK s_free(void* ptr)
{
    x_free(ptr);
}
#endif

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct video_wall_desc : public object_desc
{
    video_wall_desc( void ) : object_desc( 
            object::TYPE_VIDEO_WALL, 
            "Video Wall", 
            "SCRIPT",

            object::ATTR_RENDERABLE         |
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_SPACIAL_ENTRY,

            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC ) {}
		
            //FLAGS_BURN_VERTEX_LIGHTING 

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new video_wall; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_CAMERA;
    }

#endif // X_EDITOR

} s_VideoWall_Desc;

//=========================================================================

const object_desc& video_wall::GetTypeDesc( void ) const
{
    return s_VideoWall_Desc;
}

//=========================================================================

const object_desc& video_wall::GetObjectType( void )
{
    return s_VideoWall_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

video_wall::video_wall( void )
{
    m_Width             = 400.0f;
    m_Height            = 300.0f;
    m_MovieName[0]      = 0;
    m_bAutoPlay         = FALSE;
    m_bLoop             = TRUE;
    m_bVideoPlaying     = FALSE;
    m_Volume            = 0.0f;
    m_bMaintainAspect   = TRUE;
    m_bRenderBothSides  = FALSE;
    
    m_UVOffsetX         = 0.0f;
    m_UVOffsetY         = 0.0f;
    m_UVScaleX          = 1.0f;
    m_UVScaleY          = 1.0f;
    
#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    m_bShowInEditor     = FALSE;
    m_EditorUpdateTime  = 0.0f;
#endif
    
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
    m_hBink             = NULL;
    m_pVideoSurface     = NULL;
    m_pVideoTexture     = NULL;
    m_VideoWidth        = 0;
    m_VideoHeight       = 0;
#endif

    UpdateCorners();
}

//=========================================================================

video_wall::~video_wall( void )
{
}

//=========================================================================

void video_wall::OnInit( void )
{
    InitializeBink();
    
    if( m_bAutoPlay && m_MovieName[0] )
    {
        StartVideo();
    }

#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    // In Editor, start video for visualization if movie name is set.
    if( m_bShowInEditor && m_MovieName[0] && !m_bVideoPlaying )
    {
        StartVideo();
    }
#endif
}

//=========================================================================

void video_wall::OnKill( void )
{
    StopVideo();
    ShutdownBink();
}

//=========================================================================

void video_wall::InitializeBink( void )
{
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
    static xbool s_BinkInitialized = FALSE;
    if( !s_BinkInitialized )
    {
        RADSetMemory( s_malloc, s_free );
		#ifdef TARGET_PC
        BinkSoundUseDirectSound( NULL );
		#endif
        s_BinkInitialized = TRUE;
    }
#endif
}

//=========================================================================

void video_wall::ShutdownBink( void )
{
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
    if( m_pVideoTexture )
    {
        m_pVideoTexture->Release();
        m_pVideoTexture = NULL;
    }
    
    if( m_pVideoSurface )
    {
        m_pVideoSurface->Release();
        m_pVideoSurface = NULL;
    }
#endif
}

//=========================================================================

bbox video_wall::GetLocalBBox( void ) const
{
    bbox Box;
    Box.Clear();
    
    // Add all corners to bbox.
    for( s32 i = 0; i < 4; i++ )
    {
        Box.AddVerts( &m_Corners[i], 1 );
    }
    
    return Box;
}

//=========================================================================

void video_wall::UpdateCorners( void )
{
    // Update corners based on width/height.
    m_Corners[0].Set( -m_Width*0.5f,  m_Height*0.5f, 0.0f );  // Top-left.
    m_Corners[1].Set(  m_Width*0.5f,  m_Height*0.5f, 0.0f );  // Top-right.
    m_Corners[2].Set(  m_Width*0.5f, -m_Height*0.5f, 0.0f );  // Bottom-right.
    m_Corners[3].Set( -m_Width*0.5f, -m_Height*0.5f, 0.0f );  // Bottom-left.
}

//=========================================================================

#ifndef X_RETAIL
void video_wall::OnDebugRender( void )
{
    CONTEXT( "video_wall::OnDebugRender" );

    matrix4 L2W = GetL2W();
    draw_SetL2W( L2W );
    
    // Draw the quad wireframe.
    xcolor LineColor = m_bVideoPlaying ? XCOLOR_GREEN : XCOLOR_WHITE;
    draw_Line( m_Corners[0], m_Corners[1], LineColor );
    draw_Line( m_Corners[1], m_Corners[2], LineColor );
    draw_Line( m_Corners[2], m_Corners[3], LineColor );
    draw_Line( m_Corners[3], m_Corners[0], LineColor );
    
    // Draw center point.
    vector3 Center = (m_Corners[0] + m_Corners[2]) * 0.5f;
    draw_Marker( Center, XCOLOR_YELLOW );
    
    // Draw normal vector.
    vector3 Normal = GetL2W().RotateVector( vector3(0,0,1) );
    draw_Line( Center, Center + Normal * 50.0f, XCOLOR_BLUE );
    
    draw_ClearL2W();
    
    // Draw bbox.
    draw_BBox( GetBBox(), XCOLOR_RED );
}
#endif // X_RETAIL

//=========================================================================

void video_wall::OnRender( void )
{
    CONTEXT( "video_wall::OnRender" );

    // Render video if playing, or in Editor mode.
    xbool bShouldRender = m_bVideoPlaying && m_hBink;

#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    // In Editor, always try to show video if enabled and movie name is set.
    bShouldRender = bShouldRender || (m_bShowInEditor && m_hBink);
#endif

    if( bShouldRender )
    {
        UpdateVideoFrame();
        RenderVideoQuad();
    }
}

//=========================================================================

void video_wall::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "video_wall::OnAdvanceLogic" );
    
#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    // In Editor, manage video for visualization.
    if( m_bShowInEditor && m_MovieName[0] )
    {
        if( !m_bVideoPlaying && !m_hBink )
        {
            m_EditorUpdateTime += DeltaTime;
            if( m_EditorUpdateTime >= 1.0f )
            {
                StartVideo();
                m_EditorUpdateTime = 0.0f;
            }
        }
    }
#endif
    
    // Update video state.
    if( m_bVideoPlaying && m_hBink )
    {
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
        // Check if video finished.
        if( m_hBink->FrameNum >= (m_hBink->Frames - 1) )
        {
            xbool bShouldLoop = m_bLoop;          
#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
            // In Editor, always loop for visualization.
            if( m_bShowInEditor )
                bShouldLoop = TRUE;
#endif
            if( bShouldLoop )
            {
                BinkGoto( m_hBink, 1, 0 );
            }
            else
            {
                StopVideo();
            }
        }
#endif
    }
}

//=========================================================================

void video_wall::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
}

//=========================================================================

void video_wall::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
}

//=========================================================================

void video_wall::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "VideoWall",                  "Video Wall Properties",                           0 );
    List.PropEnumFloat   ( "VideoWall\\Width",           "Width of the video wall",                         0 );
    List.PropEnumFloat   ( "VideoWall\\Height",          "Height of the video wall",                        0 );
    List.PropEnumString  ( "VideoWall\\MovieName",       "Name of the bink movie file (without extension)", 0 );
    List.PropEnumBool    ( "VideoWall\\AutoPlay",        "Auto start video on init",                        0 );
    List.PropEnumBool    ( "VideoWall\\Loop",            "Loop the video",                                  0 );
    List.PropEnumFloat   ( "VideoWall\\Volume",          "Video volume (in float)",                         0 );
    List.PropEnumBool    ( "VideoWall\\MaintainAspect",  "Maintain video aspect ratio",                     0 );
    List.PropEnumBool    ( "VideoWall\\RenderBothSides", "Render on both sides",                            0 );
    
    List.PropEnumHeader  ( "VideoWall\\UV",              "UV Mapping Properties",  0 );
    List.PropEnumFloat   ( "VideoWall\\UV\\OffsetX",     "UV offset X (in float)", 0 );
    List.PropEnumFloat   ( "VideoWall\\UV\\OffsetY",     "UV offset Y (in float)", 0 );
    List.PropEnumFloat   ( "VideoWall\\UV\\ScaleX",      "UV scale X (in float)",  0 );
    List.PropEnumFloat   ( "VideoWall\\UV\\ScaleY",      "UV scale Y (in float)",  0 );

#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    List.PropEnumHeader  ( "VideoWall\\Editor",               "Editor Properties",             0 );
    List.PropEnumBool    ( "VideoWall\\Editor\\ShowInEditor", "Show video in editor viewport", 0 );
#endif
}

//=========================================================================

xbool video_wall::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\Width", m_Width, 8.0f, 16384.0f ) )
    {
        if( !I.IsRead() )
        {
            UpdateCorners();
            OnMove( GetPosition() );
        }
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\Height", m_Height, 8.0f, 16384.0f ) )
    {
        if( !I.IsRead() )
        {
            UpdateCorners();
            OnMove( GetPosition() );
        }
        return TRUE;
    }
    
    if( I.VarString( "VideoWall\\MovieName", m_MovieName, 64 ) )
    {
#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
        // In Editor, restart video with new movie name.
        if( !I.IsRead() && m_bShowInEditor )
        {
            if( m_bVideoPlaying )
                StopVideo();
            
            if( m_MovieName[0] )
            {
                m_EditorUpdateTime = 0.0f;
                StartVideo();
            }
        }
#endif
        return TRUE;
    }
    
    if( I.VarBool( "VideoWall\\AutoPlay", m_bAutoPlay ) )
    {
        return TRUE;
    }
    
    if( I.VarBool( "VideoWall\\Loop", m_bLoop ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\Volume", m_Volume, 0.0f, 1.0f ) )
    {
        if( !I.IsRead() && m_bVideoPlaying && m_hBink )
        {
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
            BinkSetVolume( m_hBink, 0, (s32)(m_Volume * 32767) );
#endif
        }
        return TRUE;
    }
    
    if( I.VarBool( "VideoWall\\MaintainAspect", m_bMaintainAspect ) )
    {
        return TRUE;
    }
    
    if( I.VarBool( "VideoWall\\RenderBothSides", m_bRenderBothSides ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\UV\\OffsetX", m_UVOffsetX, -1.0f, 1.0f ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\UV\\OffsetY", m_UVOffsetY, -1.0f, 1.0f ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\UV\\ScaleX", m_UVScaleX, -10.0f, 10.0f ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "VideoWall\\UV\\ScaleY", m_UVScaleY, -10.0f, 10.0f ) )
    {
        return TRUE;
    }

#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    if( I.VarBool( "VideoWall\\Editor\\ShowInEditor", m_bShowInEditor ) )
    {
        if( !I.IsRead() )
        {
            if( m_bShowInEditor )
            {
                // Start video for Editor visualization.
                if( m_MovieName[0] && !m_bVideoPlaying )
                {
                    StartVideo();
                }
            }
            else
            {
                // Stop video when disabling Editor display.
                if( m_bVideoPlaying && !m_bAutoPlay )
                {
                    StopVideo();
                }
                m_EditorUpdateTime = 0.0f;
            }
        }
        return TRUE;
    }
#endif

    return FALSE;
}

//=========================================================================

void video_wall::StartVideo( void )
{
    if( m_MovieName[0] == 0 )
        return;
        
    StopVideo();

#if defined(TARGET_PC) && defined(X_BINK_EDITOR)
    u32 Flags = BINKNOSKIP | BINKSNDTRACK;
    m_hBink = BinkOpen( xfs("C:\\GameData\\A51\\Release\\PC\\%s.bik", m_MovieName), Flags );
    
    if( m_hBink )
    {
        m_VideoWidth  = m_hBink->Width;
        m_VideoHeight = m_hBink->Height;
        
        BinkSetVolume( m_hBink, 0, (s32)(m_Volume * 32767) );        
        
        CreateRenderSurface();
        
        m_bVideoPlaying = TRUE;
    }
#endif

#ifdef TARGET_XBOX
    u32 Flags = BINKNOSKIP | BINKSNDTRACK;
    m_hBink = BinkOpen( xfs("D:\\movies\\%s.bik", m_MovieName), Flags );
    
    if( m_hBink )
    {
        m_VideoWidth  = m_hBink->Width;
        m_VideoHeight = m_hBink->Height;
        
        BinkLoadConverter( BINKSURFACE32 );
        BinkSetVolume( m_hBink, 0, (s32)(m_Volume * 32767) );
        CreateRenderSurface();
        
        m_bVideoPlaying = TRUE;
    }
#endif
}

//=========================================================================

void video_wall::StopVideo( void )
{
    if( m_hBink )
    {
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
        BinkClose( m_hBink );
        m_hBink = NULL;
#endif
        m_bVideoPlaying = FALSE;
    }
    
    ShutdownBink();
}

//=========================================================================

void video_wall::PauseVideo( void )
{
    if( m_bVideoPlaying && m_hBink )
    {
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
        BinkPause( m_hBink, 1 );
#endif
    }
}

//=========================================================================

void video_wall::ResumeVideo( void )
{
    if( m_bVideoPlaying && m_hBink )
    {
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
        BinkPause( m_hBink, 0 );
#endif
    }
}

//=========================================================================

void video_wall::CreateRenderSurface( void )
{
#if defined(TARGET_PC) && defined(X_BINK_EDITOR)
    if( g_pd3dDevice && m_VideoWidth > 0 && m_VideoHeight > 0 )
    {
        if( m_pVideoTexture )
        {
            m_pVideoTexture->Release();
            m_pVideoTexture = NULL;
        }
        if( m_pVideoSurface )
        {
            m_pVideoSurface->Release(); 
            m_pVideoSurface = NULL;
        }
        
        HRESULT hr = g_pd3dDevice->CreateTexture(
            m_VideoWidth, m_VideoHeight, 1,
            0,                 
            D3DFMT_X8R8G8B8,   
            D3DPOOL_MANAGED,   
            &m_pVideoTexture,
            NULL );
            
        if( FAILED(hr) )
        {
            hr = g_pd3dDevice->CreateTexture(
                m_VideoWidth, m_VideoHeight, 1,
                0,
                D3DFMT_A8R8G8B8,
                D3DPOOL_MANAGED,
                &m_pVideoTexture,
                NULL );
        }
        
        if( SUCCEEDED(hr) && m_pVideoTexture )
        {
            m_pVideoTexture->GetSurfaceLevel( 0, &m_pVideoSurface );
        }
    }
#endif

#ifdef TARGET_XBOX
    if( g_pd3dDevice && m_VideoWidth > 0 && m_VideoHeight > 0 )
    {
        if( m_pVideoTexture )
        {
            m_pVideoTexture->Release();
            m_pVideoTexture = NULL;
        }
        if( m_pVideoSurface )
        {
            m_pVideoSurface->Release();
            m_pVideoSurface = NULL;
        }
        
        HRESULT hr = g_pd3dDevice->CreateTexture(
            m_VideoWidth, m_VideoHeight, 1, 0,
            D3DFMT_LIN_X8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_pVideoTexture );
            
        if( FAILED(hr) )
        {
            hr = g_pd3dDevice->CreateTexture(
                m_VideoWidth, m_VideoHeight, 1, 0,
                D3DFMT_LIN_A8R8G8B8,
                D3DPOOL_DEFAULT,
                &m_pVideoTexture );
        }
            
        if( SUCCEEDED(hr) && m_pVideoTexture )
        {
            m_pVideoTexture->GetSurfaceLevel( 0, &m_pVideoSurface );
        }
    }
#endif
}

//=========================================================================

void video_wall::UpdateVideoFrame( void )
{
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
    if( !m_hBink || !m_pVideoSurface )
        return;
        
    while( BinkWait(m_hBink) )
    {
        x_DelayThread(1);
    }
    
    BinkDoFrame( m_hBink );
    
    D3DLOCKED_RECT lockRect;
    if( SUCCEEDED( m_pVideoSurface->LockRect( &lockRect, NULL, 0 ) ) )
    {
        BinkCopyToBuffer(
            m_hBink,
            lockRect.pBits,
            lockRect.Pitch,
            m_VideoHeight,
            0, 0,
            BINK_BITMAP_FORMAT | BINKCOPYALL );
            
        m_pVideoSurface->UnlockRect();
    }   
    BinkNextFrame( m_hBink );
#endif
}

//=========================================================================

void video_wall::RenderVideoQuad( void )
{
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
    if( !m_pVideoTexture )
        return;

    // Get current matrix and transform corners to world space.
    matrix4 L2W = GetL2W();
    vector3 WorldCorners[4];
    L2W.Transform( WorldCorners, m_Corners, 4 );
    
    // Calculate proper UV coordinates.
    f32 u0 = 0.0f;
    f32 v0 = 0.0f; 
    f32 u1 = 1.0f;
    f32 v1 = 1.0f;
    
    // Handle aspect ratio if enabled.
    if( m_bMaintainAspect && m_VideoWidth > 0 && m_VideoHeight > 0 )
    {
        f32 videoAspect = (f32)m_VideoWidth / (f32)m_VideoHeight;
        f32 wallAspect  = m_Width / m_Height;
        
        if( videoAspect > wallAspect )
        {
            // Video is wider, crop top/bottom.
            f32 cropAmount = (1.0f - (wallAspect / videoAspect)) * 0.5f;
            v0 = cropAmount;
            v1 = 1.0f - cropAmount;
        }
        else
        {
            // Video is taller, crop left/right.  
            f32 cropAmount = (1.0f - (videoAspect / wallAspect)) * 0.5f;
            u0 = cropAmount;
            u1 = 1.0f - cropAmount;
        }
    }
    
    // Apply custom UV offset and scale.
    f32 uRange = u1 - u0;
    f32 vRange = v1 - v0;
    u0 = u0 + (m_UVOffsetX * uRange);
    u1 = u0 + (m_UVScaleX * uRange);
    v0 = v0 + (m_UVOffsetY * vRange);
    v1 = v0 + (m_UVScaleY * vRange);
    
#ifdef TARGET_PC
    DWORD oldLighting, oldCull, oldAlphaBlend, oldSrcBlend, oldDestBlend;
    DWORD oldZEnable, oldZWrite, oldAlphaTest;
    g_pd3dDevice->GetRenderState( D3DRS_LIGHTING, &oldLighting );
    g_pd3dDevice->GetRenderState( D3DRS_CULLMODE, &oldCull );
    g_pd3dDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &oldAlphaBlend );
    g_pd3dDevice->GetRenderState( D3DRS_SRCBLEND, &oldSrcBlend );
    g_pd3dDevice->GetRenderState( D3DRS_DESTBLEND, &oldDestBlend );
    g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &oldZEnable );
    g_pd3dDevice->GetRenderState( D3DRS_ZWRITEENABLE, &oldZWrite );
    g_pd3dDevice->GetRenderState( D3DRS_ALPHATESTENABLE, &oldAlphaTest );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, m_bRenderBothSides ? D3DCULL_NONE : D3DCULL_CCW );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

    g_pd3dDevice->SetTexture( 0, m_pVideoTexture );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

    g_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_TEX1 );

    struct Vertex { f32 x, y, z; f32 u, v; };
    Vertex verts[4];
    
    // Tri-strip order: top-left, top-right, bottom-left, bottom-right  
    verts[0].x = WorldCorners[0].GetX(); verts[0].y = WorldCorners[0].GetY(); verts[0].z = WorldCorners[0].GetZ();
    verts[0].u = u0; verts[0].v = v0;
    
    verts[1].x = WorldCorners[1].GetX(); verts[1].y = WorldCorners[1].GetY(); verts[1].z = WorldCorners[1].GetZ(); 
    verts[1].u = u1; verts[1].v = v0;
    
    verts[2].x = WorldCorners[3].GetX(); verts[2].y = WorldCorners[3].GetY(); verts[2].z = WorldCorners[3].GetZ();
    verts[2].u = u0; verts[2].v = v1;
    
    verts[3].x = WorldCorners[2].GetX(); verts[3].y = WorldCorners[2].GetY(); verts[3].z = WorldCorners[2].GetZ();
    verts[3].u = u1; verts[3].v = v1;
    
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, verts, sizeof(Vertex) );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, oldLighting );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, oldCull );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, oldAlphaBlend );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, oldSrcBlend );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, oldDestBlend );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, oldZEnable );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, oldZWrite );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, oldAlphaTest );
    
    g_pd3dDevice->SetTexture( 0, NULL );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
#endif

#ifdef TARGET_XBOX
    DWORD oldLighting, oldCull;
    g_pd3dDevice->GetRenderState( D3DRS_LIGHTING, &oldLighting );
    g_pd3dDevice->GetRenderState( D3DRS_CULLMODE, &oldCull );
    
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, m_bRenderBothSides ? D3DCULL_NONE : D3DCULL_CCW );
    
    g_pd3dDevice->SetTexture( 0, m_pVideoTexture );
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_TEX1 );

    struct Vertex { f32 x, y, z; f32 u, v; };
    Vertex verts[4];
    
	// Tri-strip order: top-left, top-right, bottom-left, bottom-right.  
    verts[0].x = WorldCorners[0].GetX(); verts[0].y = WorldCorners[0].GetY(); verts[0].z = WorldCorners[0].GetZ();
    verts[0].u = u0; verts[0].v = v0;
    
    verts[1].x = WorldCorners[1].GetX(); verts[1].y = WorldCorners[1].GetY(); verts[1].z = WorldCorners[1].GetZ();
    verts[1].u = u1; verts[1].v = v0;
    
    verts[2].x = WorldCorners[3].GetX(); verts[2].y = WorldCorners[3].GetY(); verts[2].z = WorldCorners[3].GetZ();
    verts[2].u = u0; verts[2].v = v1;
    
    verts[3].x = WorldCorners[2].GetX(); verts[3].y = WorldCorners[2].GetY(); verts[3].z = WorldCorners[2].GetZ();
    verts[3].u = u1; verts[3].v = v1;
    
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, verts, sizeof(Vertex) );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, oldLighting );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, oldCull );
    g_pd3dDevice->SetTexture( 0, NULL );
#endif
#endif
}