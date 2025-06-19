//==============================================================================
//  
//  dlg_LoadGame.hpp
//  
//==============================================================================

#ifndef DLG_LOAD_GAME_HPP
#define DLG_LOAD_GAME_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"

//==============================================================================
//  dlg_load_game
//==============================================================================

extern void     dlg_load_game_register  ( ui_manager* pManager );
extern ui_win*  dlg_load_game_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class dlg_load_game : public ui_dialog
{
public:

    // slideshow states we can be in
    // NOTE: This doesn't take into account the final fade out. We can fade out
    // either from the slide show or from the animated loading sequence, so that
    // is handled as a different state.
    enum slideshow_state
    {
        STATE_IDLE = 0,     // we haven't started doing anything yet
        STATE_SETUP,        // we are setting up
        STATE_SLIDESHOW,    // we are doing a slideshow
        STATE_LOADING,      // the slideshow has finished, and now we're just waiting
    };

    //  The standard dialog functions
                        dlg_load_game       ( void );
    virtual            ~dlg_load_game       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    //  Functions for doing the initial slide show setup
            void        StartLoadingProcess ( void );
            void        SetTextAnimInfo     ( f32               StartTextAnim );
            void        SetNSlides          ( s32               nSlides );
            void        SetSlideInfo        ( s32               Index,
                                              const char*       pTextureName,
                                              f32               StartFadeIn,
                                              f32               EndFadeIn,
                                              f32               StartFadeOut,
                                              f32               EndFadeOut,
                                              xcolor            SlideColor );
            void        SetVoiceID          ( s32               VoiceID );
            void        StartSlideshow      ( void );
            void        LoadingComplete     ( void );

    //  Query functions for the rest of the world to use
 slideshow_state        GetSlideShowState   ( void ) { return m_SlideshowState; }


protected:
    //  Internal helper functions for the effects rendering
            void        UpdateLightShaftEffect  ( f32               DeltaTime );
            void        CreateDropShadow        ( s32               FontIndex );
            void        RenderLightShaftEffect  ( void );

    //  Platform-specific functions for the effects rendering
    //  IMPORTANT NOTE: We can't use vector3's for any of the operations in the
    //  loading screen because vu0 registers aren't saved and restored during
    //  a context switch and the loading screen doesn't run on the main thread!
            enum vram_buffer
            {
                BUFFER_SCREEN = 0,
                BUFFER_LEVEL_NAME,      // buffer for using a source texture for the name      
                BUFFER_DROP_SHADOW_1,   // buffer for doing the drop shadow creation (1/2 resolution)
                BUFFER_DROP_SHADOW_2,   // buffer for doing the drop shadow creation (1/2 resolution)
                BUFFER_COUNT
            };

            #ifdef TARGET_XBOX
            void        xbox_ClipSprite             ( vector3&          UL,
                                                      vector2&          Size,
                                                      vector2&          UV0,
                                                      vector2&          UV1 );
            #endif

            #ifdef TARGET_PC
            void        pc_ClipSprite               ( vector3&          UL,
                                                      vector2&          Size,
                                                      vector2&          UV0,
                                                      vector2&          UV1 );
            #endif

            #ifdef TARGET_PS2
            void        ps2_CopyRG2BA               ( s32               XRes,
                                                      s32               YRes,
                                                      s32               FBP );
            #endif

            void        platform_Init               ( void );
            void        platform_Destroy            ( void );
            void        platform_LoadSlide          ( s32               Index,
                                                      s32               TextureIndex,
                                                      const char*       pTextureName );
            void        platform_FillScreen         ( xcolor            C );
            void        platform_RenderSlide        ( s32               SlideIndex,
                                                      xcolor            C );
            void        platform_GetBufferInfo      ( vram_buffer       BufferID,
                                                      s32&              MemOffset,
                                                      s32&              BufferW,
                                                      s32&              BufferH );
            void        platform_SetSrcBuffer       ( vram_buffer       BufferID );
            void        platform_SetDstBuffer       ( vram_buffer       BufferID,
                                                      xbool             EnableRGBChannel = TRUE,
                                                      xbool             EnableAlphaChannel = TRUE );
            void        platform_ClearBuffer        ( vram_buffer       BufferID,
                                                      xbool             EnableRGBChannel = TRUE,
                                                      xbool             EnableAlphaChannel = TRUE );
            void        platform_DrawSprite         ( const vector2&    UpperLeft,
                                                      const vector2&    Size,
                                                      const vector2&    UV0,
                                                      const vector2&    UV1,
                                                      xcolor            C,
                                                      xbool             Additive );
            void        platform_BeginFogRender     ( void );
            void        platform_EndFogRender       ( void );
            void        platform_DrawFogSprite      ( const vector2&    SpriteCenter,
                                                      const vector2&    WH,
                                                      const vector2&    UV0,
                                                      const vector2&    UV1,
                                                      xcolor            C,
                                                      radian            Rotation );
            void        platform_BeginShaftRender   ( void );
            void        platform_EndShaftRender     ( void );
            void        platform_DrawShaftQuad      ( const vector2*    pCorners,
                                                      const vector2*    pUVs,
                                                      const xcolor*     pColors );

    //  Slide data
    enum { NUM_SLIDES = 16 };
    struct slide_info
    {
        f32     StartFadeIn;
        f32     EndFadeIn;
        f32     StartFadeOut;
        f32     EndFadeOut;
        xcolor  SlideColor;
        xbool   HasImage;

        #if defined(TARGET_PS2)
        u64     Tex0;
        #elif defined(TARGET_XBOX)
        xbitmap BMP;
        #elif defined(TARGET_PC)
        xbitmap BMP;
        #endif
    };

    s32             m_VoiceID;
    s32             m_nSlides;
    s32             m_nTextures;
    slide_info      m_Slides[NUM_SLIDES];
    f32             m_StartTextAnim;
    xwstring        m_NameText;

    //  State management data
    slideshow_state     m_SlideshowState;
    xbool               m_LoadingComplete;
    xbool               m_FinalFadeoutStarted;
    f32                 m_ElapsedTime;
    f32                 m_FadeTimeElapsed;

    // data for the light shaft effect
    xbitmap     m_FogBMP;           // Fog texture
    xbool       m_FogLoaded;
    f32         m_Position;         // Current position of light shafts
    f32         m_HorizSpeed;       // Speed of light shafts
    radian      m_FogAngle;         // Current fog rotation
    f32         m_FogZoom;          // Current fog zoom
    irect       m_LightShaftArea;   // Area of screen we'll be doing effect on

    // Platform-specific data
    #ifdef TARGET_PS2
    s32             m_OrigPermanentSize;
    #endif

    #ifdef TARGET_XBOX
    u32             m_ColorWriteMask;
    s32             m_BufferW;
    s32             m_BufferH;
    #endif
    
    #ifdef TARGET_PC
    IDirect3DSurface9*   m_pBackBuffer;
    IDirect3DTexture9*   m_Buffers[4];
    u32                  m_ColorWriteMask;
    s32                  m_BufferW;
    s32                  m_BufferH;
    #endif
};

//==============================================================================
#endif // DLG_LOAD_GAME_HPP
//==============================================================================