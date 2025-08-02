#ifndef VIDEOWALL_HPP
#define VIDEOWALL_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_files.hpp"
#include "Entropy.hpp"
#include "audio/audio_hardware.hpp"

// Add stupid bink code for Editor, for support playing videos inside it. 
// Important! In this case Editor will be require binkw32.dll
//#define X_BINK_EDITOR

#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
#include <3rdParty\BinkXBOX\Include\bink.h>
#endif

#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
#define BINK_BITMAP_FORMAT    BINKSURFACE32
#endif

//=========================================================================
// CLASS
//=========================================================================

class video_wall : public object
{
public:

    CREATE_RTTI( video_wall, object, object )
    
                            video_wall      ( void );
    virtual                ~video_wall      ( void );
    virtual bbox            GetLocalBBox    ( void ) const;
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnMove          ( const vector3& NewPos );
    virtual void            OnTransform     ( const matrix4& L2W );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    // Video control
    void                    StartVideo      ( void );
    void                    StopVideo       ( void );
    void                    PauseVideo      ( void );
    void                    ResumeVideo     ( void );
    xbool                   IsVideoPlaying  ( void ) const { return m_bVideoPlaying; }

protected:

    virtual void            OnRender        ( void );
    virtual void            OnAdvanceLogic  ( f32 DeltaTime );
    virtual void            OnInit          ( void );
    virtual void            OnKill          ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender   ( void );
#endif // X_RETAIL

    void                    InitializeBink      ( void );
    void                    ShutdownBink        ( void );
    void                    CreateRenderSurface ( void );
    void                    UpdateVideoFrame    ( void );
    void                    RenderVideoQuad     ( void );
    void                    UpdateCorners       ( void );

protected:

    f32                     m_Width;            // Width of the video wall.
    f32                     m_Height;           // Height of the video wall.
    char                    m_MovieName[64];    // Name of the movie file.
    xbool                   m_bAutoPlay;        // Auto start video on init.
    xbool                   m_bLoop;            // Loop the video.
    xbool                   m_bVideoPlaying;    // Current video state.
    f32                     m_Volume;           // Video volume.
    xbool                   m_bMaintainAspect;  // Maintain aspect ratio.
    xbool                   m_bRenderBothSides; // Render on both sides.
    vector3                 m_Corners[4];       // Local space corners.
	
#if (defined(TARGET_PC) && defined(X_BINK_EDITOR)) || defined(TARGET_XBOX)
    HBINK                   m_hBink;
    s32                     m_VideoWidth;
    s32                     m_VideoHeight;
#endif	

// This is here to get the Bink to shutup about needing this variable!
#if defined(TARGET_PC) && !defined(X_BINK_EDITOR)
    void*                   m_hBink;
#endif
	
#if defined(TARGET_PC) && defined(X_BINK_EDITOR)
    IDirect3DSurface9*      m_pVideoSurface;
    IDirect3DTexture9*      m_pVideoTexture;
#endif

#ifdef TARGET_XBOX
    IDirect3DSurface8*      m_pVideoSurface;
    IDirect3DTexture8*      m_pVideoTexture;
#endif

    // UV coordinates for video mapping.
    f32                     m_UVOffsetX;
    f32                     m_UVOffsetY;
    f32                     m_UVScaleX; 
    f32                     m_UVScaleY; 

#if defined(X_EDITOR) && defined(X_BINK_EDITOR)
    xbool                   m_bShowInEditor;
    f32                     m_EditorUpdateTime;
#endif
};

//=========================================================================
// END
//=========================================================================
#endif