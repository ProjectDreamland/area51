//=========================================================================
//
//  dlg_load_game.cpp
//
//=========================================================================

//=========================================================================
// Includes
//=========================================================================

#include "entropy.hpp"
#include "dlg_LoadGame.hpp"
#include "StateMgr\StateMgr.hpp"

#include "ui\ui_font.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

#ifdef TARGET_XBOX
#include "Render\xbox_render.hpp"
#endif

//=========================================================================
// defines for the slide show
//=========================================================================

#define MAX_TEXTURES            5
#define FINAL_FADE_OUT_TIME     1.0f
#define SLIDESHOW_IMAGE_WIDTH   512
#define SLIDESHOW_IMAGE_HEIGHT  512

//=========================================================================
// defines for the text positioning
//=========================================================================

#define TEXT_IMAGE_WIDTH    512
#define TEXT_IMAGE_HEIGHT   64

#ifdef TARGET_XBOX
    #define TEXT_X_POSITION   50
    #define TEXT_Y_POSITION  330
extern void RedirectTextureAllocator( void );
extern void RestoreTextureAllocator ( void );
extern u32 g_PhysW;
extern u32 g_PhysH;
#elif defined( TARGET_PS2 )
    #define TEXT_X_POSITION  -50
    #define TEXT_Y_POSITION  313
#elif defined( TARGET_PC ) //TODO: Make it RES scalable!!!!!!!!!!!!
    #define TEXT_X_POSITION  400
    #define TEXT_Y_POSITION  525
#else //For future.
    #define TEXT_X_POSITION  -50
    #define TEXT_Y_POSITION  313
#endif

#define HIGHLIGHT_SCALE             1.2f

//=========================================================================
// defines for the text animation
//=========================================================================

#define SHADOW_SCALE_BEGIN_TIME     0.0f
#define SHADOW_SCALE_END_TIME       1.3333333f
#define SHADOW_SCALE_BEGIN          8.6666666f
#define SHADOW_SCALE_END            1.0f
#define SHADOW_SLIDE_BEGIN_TIME     1.3333333f
#define SHADOW_SLIDE_END_TIME       5.8f
#define SHADOW_SLIDE_BEGIN_X        0
#define SHADOW_SLIDE_END_X          -64
#define SHADOW_SLIDE_BEGIN_Y        0
#define SHADOW_SLIDE_END_Y          20
#define SHADOW_FADEIN_BEGIN_TIME    5.8f
#define SHADOW_FADEIN_END_TIME      6.8f

xcolor g_DropShadowTweak( 187, 255, 187, 255 );

//=========================================================================
// defines for the light shaft animation
//=========================================================================

#define LIGHT_SHAFT_PIXEL_WIDTH     64.0f           // Source pixel width of light shafts
#define LIGHT_SHAFT_HORIZ_SCALE     0.5f            // Horizontal shaft spread amount
#define LIGHT_SHAFT_VERT_SCALE      12.0f           // Vertical shaft spread amount
#define LIGHT_SHAFT_ALPHA           0.15f           // Alpha of each layer
#define LIGHT_SHAFT_NUM_PASSES      15              // # of passes
#define LIGHT_SHAFT_FOG_ROT_SPEED   R_5             // Speed of rotation
#define LIGHT_SHAFT_FOG_ALPHA       0.3f            // Alpha of each layer
#define LIGHT_SHAFT_FOG_SIZE        (1024.0f)       // Max size of fog layer
#define LIGHT_SHAFT_FOG_ZOOM_SPEED  0.2f            // Speed of zoom
#define LIGHT_SHAFT_NUM_FOG_PASSES  15              // # of passes

// NOTE: We DON'T use PRELOAD_FILE, because we don't want to give up
// the memory in the game. We should be okay with this because the editor
// has special-case code that should've been nice enough to put the image
// into same DFS file as the slides.
#define LIGHT_SHAFT_FOG_NAME        "UI_LoadScreen_Fog.xbmp"

//=========================================================================
// platform-specific defines
//=========================================================================

// Handy debugging macro
#define PS2_BREAK_FOR_VRAM_SNAP \
{                               \
    eng_End();                  \
    DLIST.WaitForTasks();       \
    DLIST.EndFrame();           \
    DLIST.Disable();            \
    BREAK;                      \
    DLIST.Enable();             \
    DLIST.BeginFrame();         \
    eng_Begin();                \
}

#define PS2_SCISSOR_X    (2048-(VRAM_FRAME_BUFFER_WIDTH/2))
#define PS2_SCISSOR_Y    (2048-(VRAM_FRAME_BUFFER_HEIGHT/2))

//=========================================================================
//  Load Game Dialog
//=========================================================================

enum controls
{
    IDC_LEVEL_NAME,
};

ui_manager::control_tem LoadGameControls[] =
{
    { IDC_LEVEL_NAME, "IDS_NULL", "text", 0, 308, 480, 30, 0, 0, 1, 1, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  }
};

ui_manager::dialog_tem LoadGameDialog =
{
    "IDS_LOAD_GAME",
    1, 9,
    sizeof(LoadGameControls)/sizeof(ui_manager::control_tem),
    &LoadGameControls[0],
    0
};

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Registration function
//=========================================================================

void dlg_load_game_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "load game", &LoadGameDialog, &dlg_load_game_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_load_game_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_load_game* pDialog = new dlg_load_game;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_load_game
//=========================================================================

dlg_load_game::dlg_load_game( void )
{
    // Initialize platform-specific data to safe defaults
#ifdef TARGET_PS2
    m_OrigPermanentSize = -1;
#endif

#ifdef TARGET_XBOX
    m_ColorWriteMask = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    m_BufferW = 0;
    m_BufferH = 0;
#endif
    
#ifdef TARGET_PC
    m_pBackBuffer = NULL;
    x_memset( m_Buffers, 0, sizeof(m_Buffers) );
    m_ColorWriteMask = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    m_BufferW = 0;
    m_BufferH = 0;
#endif

    // Initialize other members to safe defaults
    m_VoiceID = 0;
    m_nSlides = 0;
    m_nTextures = 0;
    m_StartTextAnim = 0.0f;
    m_NameText.Clear();
    
    m_SlideshowState = STATE_IDLE;
    m_LoadingComplete = FALSE;
    m_FinalFadeoutStarted = FALSE;
    m_ElapsedTime = 0.0f;
    m_FadeTimeElapsed = 0.0f;
    
    m_FogLoaded = FALSE;
    m_Position = -0.5f;
    m_HorizSpeed = 0.15f;
    m_FogAngle = R_0;
    m_FogZoom = 0.0f;
    m_LightShaftArea.Clear();
    
    // Initialize slide array
    x_memset( m_Slides, 0, sizeof(m_Slides) );
}

//=========================================================================

dlg_load_game::~dlg_load_game( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_load_game::Create( s32                        UserID,
                               ui_manager*                pManager,
                               ui_manager::dialog_tem*    pDialogTem,
                               const irect&               Position,
                               ui_win*                    pParent,
                               s32                        Flags,
                               void*                      pUserData )
{
    xbool   Success = FALSE;

    (void)pUserData;

    ASSERT( pManager );

    // Do dialog creation
    Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    // Make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // initialize the slide data
    m_VoiceID       = 0;
    m_nSlides       = 0;
    m_nTextures     = 0;
    m_StartTextAnim = 0;
    m_NameText      = xwstring("");

    // Initialize the state management data
    m_SlideshowState      = STATE_IDLE;
    m_LoadingComplete     = FALSE;
    m_FinalFadeoutStarted = FALSE;
    m_ElapsedTime         = 0.0f;
    m_FadeTimeElapsed     = 0.0f;

    // Initialize the light shaft effect data
    m_FogLoaded        = FALSE;
    m_Position         = -0.5f;
    m_HorizSpeed       = 0.15f;
    m_FogAngle         = R_0;
    m_FogZoom          = 0.0f;
    m_LightShaftArea.l = 0;
    m_LightShaftArea.r = 1;
    m_LightShaftArea.t = 0;
    m_LightShaftArea.b = 1;

    // Platform-specific data
    #ifdef TARGET_PS2
    m_OrigPermanentSize = -1;
    #endif

    // Return success code
    return Success;
}

//=========================================================================

void dlg_load_game::Destroy( void )
{
    ui_dialog::Destroy();

    // Kill screen wipe
    if( g_UiMgr )
        g_UiMgr->ResetScreenWipe();

    // Do the platform-specific cleanup
    platform_Destroy();

    // Kill the light shaft effect
    if( m_FogLoaded )
    {
        m_FogLoaded = FALSE;

        // Kill fog texture
        vram_Unregister( m_FogBMP );
        m_FogBMP.Kill();
    }
    
    // Reset state variables to safe defaults
    m_SlideshowState = STATE_IDLE;
    m_LoadingComplete = FALSE;
    m_FinalFadeoutStarted = FALSE;
    m_VoiceID = 0;
    m_nSlides = 0;
    m_nTextures = 0;
}

//=========================================================================

void dlg_load_game::Render( s32 ox, s32 oy )
{
    (void)ox;
    (void)oy;

    // do nothing if the slideshow hasn't started yet
    if( (m_SlideshowState == STATE_IDLE) || (m_SlideshowState == STATE_SETUP) )
    {
        return;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Clear the screen
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    // render a big black quad so that any text and/or screens has
    // something to blend with
    platform_FillScreen( XCOLOR_BLACK );

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Calculate any rendering parameters based on the slide state
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    // figure out the alpha values based on the slideshow state
    s32 i;
    s32 SlideAlphas[NUM_SLIDES];
    s32 NameAlpha = 0;
    f32 ShadowOffsetX     = SHADOW_SLIDE_BEGIN_X;
    f32 ShadowOffsetY     = SHADOW_SLIDE_BEGIN_Y;
    f32 ShadowVertScale   = SHADOW_SCALE_END;
    f32 ShadowAlpha       = 0.0f;
    x_memset( SlideAlphas, 0, sizeof(s32)*NUM_SLIDES );
    if( m_SlideshowState == STATE_SLIDESHOW )
    {
        f32 CurrTime = m_ElapsedTime;
        
        for( i = 0; i < m_nSlides; i++ )
        {
            // figure out the alpha value for this slide based on where it is
            // in comparison to the audio
            if( CurrTime < m_Slides[i].StartFadeIn )
            {
                // nothing to display, and alpha is already set to zero
            }
            else if( (CurrTime >= m_Slides[i].StartFadeIn) && (CurrTime < m_Slides[i].EndFadeIn) )
            {
                // this slide is fading in, figure out where it is in the fade
                f32 FadeDuration = m_Slides[i].EndFadeIn - m_Slides[i].StartFadeIn;
                if( FadeDuration > 0.0f )
                {
                    SlideAlphas[i] = (s32)(255.0f * (CurrTime-m_Slides[i].StartFadeIn) / FadeDuration);
                    SlideAlphas[i] = MINMAX( 0, SlideAlphas[i], 255 );
                }
            }
            else if( (CurrTime >= m_Slides[i].EndFadeIn) && (CurrTime < m_Slides[i].StartFadeOut) )
            {
                // This slide is fully faded in and hasn't started fading out yet
                SlideAlphas[i] = 255;
            }
            else if( (CurrTime >= m_Slides[i].StartFadeOut) && (CurrTime < m_Slides[i].EndFadeOut) )
            {
                // This slide is fading out. Figure out how far along in its fade it should be.
                f32 FadeDuration = m_Slides[i].EndFadeOut - m_Slides[i].StartFadeOut;
                if( FadeDuration > 0.0f )
                {
                    SlideAlphas[i] = 255 - (s32)(255.0f * (CurrTime-m_Slides[i].StartFadeOut) / FadeDuration);
                    SlideAlphas[i] = MINMAX( 0, SlideAlphas[i], 255 );
                }
            }
        }

        // figure out the drop shadow offset
        if( CurrTime > m_StartTextAnim )
        {
            f32 TextAnimTime = CurrTime - m_StartTextAnim;
            if( TextAnimTime < SHADOW_SCALE_END_TIME )
            {
                f32 T = (TextAnimTime-SHADOW_SCALE_BEGIN_TIME)/(SHADOW_SCALE_END_TIME-SHADOW_SCALE_BEGIN_TIME);
                ShadowVertScale = SHADOW_SCALE_BEGIN + T * (SHADOW_SCALE_END-SHADOW_SCALE_BEGIN);
                ShadowAlpha     = T;
                NameAlpha       = (s32)(255.0f*T);
                NameAlpha       = MINMAX( 0, NameAlpha, 255 );
            }
            else if( TextAnimTime < SHADOW_SLIDE_END_TIME )
            {
                f32 T = (TextAnimTime-SHADOW_SLIDE_BEGIN_TIME)/(SHADOW_SLIDE_END_TIME-SHADOW_SLIDE_BEGIN_TIME);
                ShadowAlpha   = 1.0f - T;
                ShadowOffsetX = SHADOW_SLIDE_BEGIN_X + T * (SHADOW_SLIDE_END_X - SHADOW_SLIDE_BEGIN_X);
                ShadowOffsetY = SHADOW_SLIDE_BEGIN_Y + T * (SHADOW_SLIDE_END_Y - SHADOW_SLIDE_BEGIN_Y);
                NameAlpha     = 255;
            }
            else if( TextAnimTime < SHADOW_FADEIN_END_TIME )
            {
                f32 T = (TextAnimTime-SHADOW_FADEIN_BEGIN_TIME)/(SHADOW_FADEIN_END_TIME-SHADOW_FADEIN_BEGIN_TIME);
                ShadowAlpha     = T;
                NameAlpha       = 255;
                ShadowVertScale = HIGHLIGHT_SCALE;
            }
            else
            {
                ShadowAlpha     = 1.0f;
                NameAlpha       = 255;
                ShadowVertScale = HIGHLIGHT_SCALE;
            }
        }
    }
    else
    {
        // We must be in the loading state or we never got proper data
        // set up. So just render the name full on with a highlight
        // scrolling across to show that we haven't crashed.
        NameAlpha       = 255;
        ShadowOffsetX   = SHADOW_SLIDE_BEGIN_X;
        ShadowOffsetY   = SHADOW_SLIDE_BEGIN_Y;
        ShadowVertScale = HIGHLIGHT_SCALE;
        ShadowAlpha     = 1.0f;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Render any slides
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    for( i = 0; i < m_nSlides; i++ )
    {
        if( SlideAlphas[i] > 0 )
        {
            xcolor C( m_Slides[i].SlideColor.R,
                      m_Slides[i].SlideColor.G,
                      m_Slides[i].SlideColor.B,
                      SlideAlphas[i] );

            if( m_Slides[i].HasImage )
                platform_RenderSlide( i, C );
            else
                platform_FillScreen( C );
        }
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Render the name and drop shadow
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    if( NameAlpha > 0 )
    {
        // figure out which font we're using
        s32 FontIndex = g_UiMgr->FindFont( "loadscr" );
        ASSERT( FontIndex >= 0 );

        // create a drop shadow to be placed under the text
        CreateDropShadow( FontIndex );

        // clear the level name buffer in preparation for what we're about to do
        platform_ClearBuffer( BUFFER_LEVEL_NAME );
        platform_ClearBuffer( BUFFER_DROP_SHADOW_2 );

        if( m_SlideshowState == STATE_LOADING )
        {
            // render the drop shadow to the name texture
            vector2 Center = vector2( TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2 );
            vector2 UL     = Center - vector2( TEXT_IMAGE_WIDTH/2, ShadowVertScale*TEXT_IMAGE_HEIGHT/2 );
            u8      ShadA  = (u8)(255.0f*ShadowAlpha);
            platform_SetSrcBuffer( BUFFER_DROP_SHADOW_1 );
            platform_SetDstBuffer( BUFFER_LEVEL_NAME );
            platform_DrawSprite( UL,
                                 vector2( TEXT_IMAGE_WIDTH, ShadowVertScale*TEXT_IMAGE_HEIGHT ),
                                 vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
                                 xcolor( g_DropShadowTweak.R, g_DropShadowTweak.G, g_DropShadowTweak.B, ShadA ),
                                 TRUE );

            // render the text to the name texture
            irect Rect;
            Rect.l = 0;
            Rect.r = TEXT_IMAGE_WIDTH - 16;     // leave 16 so there is room to blur
            Rect.t = 0;
            Rect.b = TEXT_IMAGE_HEIGHT - 16;    // leave 16 so there is room to blur
            platform_SetDstBuffer( BUFFER_LEVEL_NAME );
            g_UiMgr->RenderText( FontIndex, Rect, ui_font::h_right|ui_font::v_bottom, XCOLOR_WHITE, m_NameText );

            // render the light shaft effect
            RenderLightShaftEffect();
        }
        else
        {
            // render the drop shadow to the screen
            vector2 Center = vector2( TEXT_X_POSITION+ShadowOffsetX, TEXT_Y_POSITION+ShadowOffsetY ) +
                             vector2( TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2 );
            vector2 UL     = Center - vector2( TEXT_IMAGE_WIDTH/2, ShadowVertScale*TEXT_IMAGE_HEIGHT/2 );
            u8      ShadA  = (u8)(255.0f*ShadowAlpha);
            platform_SetSrcBuffer( BUFFER_DROP_SHADOW_1 );
            platform_SetDstBuffer( BUFFER_SCREEN );
            platform_DrawSprite( UL, vector2( TEXT_IMAGE_WIDTH, TEXT_IMAGE_HEIGHT*ShadowVertScale ),
                                 vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
                                 xcolor( g_DropShadowTweak.R, g_DropShadowTweak.G, g_DropShadowTweak.B, ShadA ),
                                 TRUE );

            // render the text to the name texture
            irect Rect;
            Rect.l = 0;
            Rect.r = TEXT_IMAGE_WIDTH - 16;     // leave 16 so there is room to blur
            Rect.t = 0;
            Rect.b = TEXT_IMAGE_HEIGHT - 16;    // leave 16 so there is room to blur
            platform_SetDstBuffer( BUFFER_LEVEL_NAME );
            g_UiMgr->RenderText( FontIndex, Rect, ui_font::h_right|ui_font::v_bottom, XCOLOR_WHITE, m_NameText );

            // render the text to the screen
            platform_SetSrcBuffer( BUFFER_LEVEL_NAME );
            platform_SetDstBuffer( BUFFER_SCREEN );
            UL.Set( TEXT_X_POSITION, TEXT_Y_POSITION );
            platform_DrawSprite( UL,
                                 vector2( TEXT_IMAGE_WIDTH, TEXT_IMAGE_HEIGHT ),
                                 vector2( 0.0f, 0.0f ),
                                 vector2( 1.0f, 1.0f ),
                                 xcolor( 255, 255, 255, NameAlpha ),
                                 TRUE );
        }
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Render the screen fade
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    // Now if we're doing a complete screen fade-out, render a black quad
    // over the screen. (Yeah, yeah, we're doing more passes than strictly
    // necessary, but it's just a loading screen. Who cares?)
    if( m_FinalFadeoutStarted )
    {
        f32 T = m_FadeTimeElapsed / FINAL_FADE_OUT_TIME;
        T = MINMAX( 0.0f, T, 1.0f );
        u8 A = (u8)(255.0f * T);

        platform_FillScreen( xcolor( 0, 0, 0, A ) );
    }
}

//=========================================================================

void dlg_load_game::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // skip to the loading state if we're not already there
    if( m_SlideshowState == STATE_SLIDESHOW )
    {
        // kill the audio
        if( g_AudioMgr.IsValidVoiceId(m_VoiceID) )
        {
            g_AudioMgr.Release( m_VoiceID, 0.0f );
        }

        // go into the the loading state
        m_SlideshowState = STATE_LOADING;
        m_ElapsedTime    = 0.0f;
    }
}

//=========================================================================

void dlg_load_game::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;

    if( (m_SlideshowState != STATE_IDLE) &&
        (m_SlideshowState != STATE_SETUP) )
    {
        // update input (since we are on a background thread during loading,
        // the the normal input update function isn't called)
        if( g_StateMgr.IsBackgroundThreadRunning() )
        {
            // make sure audio gets a chance to do its thing
            g_AudioMgr.Update( DeltaTime );

            input_UpdateState();
            g_UiMgr->ProcessInput( DeltaTime );
        }
    }

    // handle the various state updates
    if( m_SlideshowState == STATE_SLIDESHOW )
    {
        if( !g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
        {
            // If we don't have a valid voice ID, then we must increment
            // the elapsed time manually.
            m_ElapsedTime += DeltaTime;
        }
        else
        {
            // otherwise get the elapsed time from the audio
            m_ElapsedTime = g_AudioMgr.GetCurrentPlayTime( m_VoiceID );
        }

        // Have we finished loading and are we at least far enough in the
        // name animation that we can start fading the screen out with
        // it still looking nice?
        if( m_LoadingComplete &&
            !m_FinalFadeoutStarted &&
            m_ElapsedTime > (m_StartTextAnim+SHADOW_SLIDE_BEGIN_TIME) )
        {
            m_FinalFadeoutStarted = TRUE;
        }

        // we can transition to the loading state, iff
        // a) We've passed the point where the shadow has faded back
        //    in in preparation for the loading state, AND
        // b) the last slide has completely faded out
        s32 i;
        f32 EndTime = m_StartTextAnim+SHADOW_FADEIN_END_TIME;
        for( i = 0; i < m_nSlides; i++ )
        {
            EndTime = MAX( EndTime, m_Slides[i].EndFadeOut );
        }

        // Transition to the "loading" state?
        if( m_ElapsedTime > EndTime )
        {
            m_SlideshowState = STATE_LOADING;
            m_ElapsedTime    = 0.0f;
        }
    }
    else if( m_SlideshowState == STATE_LOADING )
    {
        // just update the elapsed time
        m_ElapsedTime += DeltaTime;

        // Have we finished loading? If so, then start fading out the screen
        // so we can get into the game!
        if( m_LoadingComplete && !m_FinalFadeoutStarted )
        {
            m_FinalFadeoutStarted = TRUE;
        }

        // And update the light shaft effect
        UpdateLightShaftEffect( DeltaTime );
    }
    
    // Now update the final screen fade-out (we'll keep whatever animation
    // that happens to be going, but fade the whole thing out so we get a nice
    // smooth transition regardless of what we were doing before).
    if( m_FinalFadeoutStarted )
    {
        m_FadeTimeElapsed += DeltaTime;
        // We're tacking on a little extra time to allow the fade to reach 
        // total black
        if( m_FadeTimeElapsed > (FINAL_FADE_OUT_TIME + 0.11f))
        {
            m_State = DIALOG_STATE_EXIT;
        }
    }
}

//=========================================================================

void dlg_load_game::StartLoadingProcess( void )
{
    // advance the state
    ASSERT( m_SlideshowState == STATE_IDLE );
    m_SlideshowState = STATE_SETUP;

    // do platform-specific initialization
    platform_Init();

    // figure out the name text
    m_NameText.Clear();
    m_NameText += g_ActiveConfig.GetLevelName();

    s32 i;
    for( i = 0; i < m_NameText.GetLength(); i++ )
    {
        if( m_NameText[i] == '\\' )
        {
            // HACK HACK - We shouldn't need to do this, but the font doesn't have a backslash
            // right now, and this will work around the problem so we don't assert.
            m_NameText[i] = '/';
        }
    }

    // Load the light shaft fog image
    m_FogLoaded = m_FogBMP.Load( xfs( "%s\\%s", g_RscMgr.GetRootDirectory(),
                                 LIGHT_SHAFT_FOG_NAME ) );
    if( m_FogLoaded )
    {
        vram_Register( m_FogBMP );
    }

    // figure out the area of the screen that we should perform the light
    // shaft effect on
    s32   FontIndex = g_UiMgr->FindFont( "loadscr" );
    ASSERT( FontIndex >= 0 );
    irect Rect;
    g_UiMgr->TextSize( FontIndex, Rect, m_NameText, m_NameText.GetLength() );
    m_LightShaftArea.r = TEXT_X_POSITION + TEXT_IMAGE_WIDTH;
    m_LightShaftArea.l = m_LightShaftArea.r - (Rect.r-Rect.l) - 32; // -32 pads for blurring
    m_LightShaftArea.t = TEXT_Y_POSITION;
    m_LightShaftArea.b = TEXT_Y_POSITION + TEXT_IMAGE_HEIGHT;
}

//=========================================================================

void dlg_load_game::SetTextAnimInfo( f32 StartTextAnim )
{
    ASSERT( m_SlideshowState == STATE_SETUP );
    m_StartTextAnim = StartTextAnim;
}

//=========================================================================

void dlg_load_game::SetVoiceID( s32 VoiceID )
{
    ASSERT( m_SlideshowState == STATE_SETUP );
    m_VoiceID = VoiceID;
}

//=========================================================================

void dlg_load_game::SetNSlides( s32 nSlides )
{
    ASSERT( m_SlideshowState == STATE_SETUP );
    m_nSlides = nSlides;

    s32 i;
    for( i = 0; i < m_nSlides; i++ )
    {
        m_Slides[i].StartFadeIn  = 0.0f;
        m_Slides[i].EndFadeIn    = 1.0f;
        m_Slides[i].StartFadeOut = 2.0f;
        m_Slides[i].EndFadeOut   = 3.0f;
        m_Slides[i].SlideColor   = XCOLOR_WHITE;
        m_Slides[i].HasImage     = FALSE;
    }
}

//=========================================================================

void dlg_load_game::SetSlideInfo( s32         Index,
                                  const char* pTextureName,
                                  f32         StartFadeIn,
                                  f32         EndFadeIn,
                                  f32         StartFadeOut,
                                  f32         EndFadeOut,
                                  xcolor      SlideColor )
{
    ASSERT( m_SlideshowState == STATE_SETUP );

    ASSERT( (Index>=0) && (Index < m_nSlides) );
    m_Slides[Index].StartFadeIn  = StartFadeIn;
    m_Slides[Index].EndFadeIn    = EndFadeIn;
    m_Slides[Index].StartFadeOut = StartFadeOut;
    m_Slides[Index].EndFadeOut   = EndFadeOut;
    m_Slides[Index].SlideColor   = SlideColor;
    if( pTextureName && x_stricmp(pTextureName,"<NULL>") )
    {
        m_Slides[Index].HasImage = TRUE;
        platform_LoadSlide( Index, m_nTextures, pTextureName );
        m_nTextures++;
    }
    else
    {
        m_Slides[Index].HasImage = FALSE;
    }
}

//=========================================================================

void dlg_load_game::StartSlideshow( void )
{
    // advance the state
    ASSERT( m_SlideshowState == STATE_SETUP );
    m_SlideshowState = STATE_SLIDESHOW;
    m_ElapsedTime    = 0.0f;

    // if we have no slides, skip to the loading state
    if( m_nSlides == 0 )
    {
        m_SlideshowState = STATE_LOADING;
    }
}

//=========================================================================

void dlg_load_game::LoadingComplete( void )
{
    m_LoadingComplete = TRUE;

    if( m_SlideshowState == STATE_IDLE )
        m_State = DIALOG_STATE_EXIT;
}

//=========================================================================

void dlg_load_game::UpdateLightShaftEffect( f32 DeltaTime )
{
    // Update rotation
    m_FogAngle += DeltaTime * LIGHT_SHAFT_FOG_ROT_SPEED;

    // Update pos
    m_Position += m_HorizSpeed * DeltaTime;
    if( m_Position > 1.5f )
    {
        m_Position = 1.5f;
        m_HorizSpeed = -m_HorizSpeed;
    }
    else if( m_Position < -0.5f )
    {
        m_Position = -0.5f;
        m_HorizSpeed = -m_HorizSpeed;
    }

    // Update zoom
    m_FogZoom -= LIGHT_SHAFT_FOG_ZOOM_SPEED * DeltaTime;
    if( m_FogZoom < 0.0f )
        m_FogZoom += 1.0f;
}

//=========================================================================

void dlg_load_game::CreateDropShadow( s32 FontIndex )
{
    s32 i;

    // clear the buffers to start
    platform_ClearBuffer( BUFFER_LEVEL_NAME );
    platform_ClearBuffer( BUFFER_DROP_SHADOW_1 );
    platform_ClearBuffer( BUFFER_DROP_SHADOW_2 );

    // render the text to the level name buffer
    irect Rect;
    Rect.l = 0;
    Rect.r = TEXT_IMAGE_WIDTH - 16;     // leave 16 so there is room to blur
    Rect.t = 0;
    Rect.b = TEXT_IMAGE_HEIGHT - 16;    // leave 16 so there is room to blur
    platform_SetDstBuffer( BUFFER_LEVEL_NAME );
    g_UiMgr->RenderText( FontIndex, Rect, ui_font::h_right|ui_font::v_bottom, XCOLOR_WHITE, m_NameText );

    // now shrink the image as the first step in blurring
    platform_SetSrcBuffer( BUFFER_LEVEL_NAME );
    platform_SetDstBuffer( BUFFER_DROP_SHADOW_1 );
    platform_DrawSprite( vector2(-0.5f,-0.5f),
                         vector2(TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2),
                         vector2(0.0f,0.0f),
                         vector2(1.0f,1.0f),
                         XCOLOR_WHITE,
                         TRUE );

    // now blur the image horizontally
    platform_SetSrcBuffer( BUFFER_DROP_SHADOW_1 );
    platform_SetDstBuffer( BUFFER_DROP_SHADOW_2 );
    static s32 HorzAlphas[]  = { 16, 48, 64, 70, 64, 48, 16 };
    static s32 HorzOffsets[] = { -3, -2, -1, 0, 1, 2, 3 };
    for( i = 0; i < 7; i++ )
    {
        platform_DrawSprite( vector2(HorzOffsets[i]-0.5f, -0.5f),
                             vector2(TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2),
                             vector2(0.0f,0.0f), vector2(1.0f,1.0f),
                             xcolor(255,255,255,HorzAlphas[i]),
                             TRUE );
    }

    // now blur the image vertically
    platform_ClearBuffer( BUFFER_DROP_SHADOW_1 );
    platform_SetSrcBuffer( BUFFER_DROP_SHADOW_2 );
    platform_SetDstBuffer( BUFFER_DROP_SHADOW_1 );
    static s32 VertAlphas[] = { 16, 48, 64, 70, 64, 48, 16 };
    static s32 VertOffsets[] = { -3, -2, -1, 0, 1, 2, 3 };
    for( i = 0; i < 7; i++ )
    {
        platform_DrawSprite( vector2(-0.5f, VertOffsets[i]-0.5f),
                             vector2(TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2),
                             vector2(0.0f,0.0f), vector2(1.0f,1.0f),
                             xcolor(255,255,255,VertAlphas[i]),
                             TRUE );
    }
}

//==============================================================================

void dlg_load_game::RenderLightShaftEffect( void )
{
    // we can't do the effect if we don't have a proper fog texture
    if( !m_FogLoaded )
        return;

    // Compute the section we'll send light through
    f32 w = (f32)(m_LightShaftArea.r - m_LightShaftArea.l);
    f32 h = (f32)(m_LightShaftArea.b - m_LightShaftArea.t);
    f32 c = m_Position * (f32)(w - LIGHT_SHAFT_PIXEL_WIDTH);
    vector2 FogCenter( m_LightShaftArea.l + c + LIGHT_SHAFT_PIXEL_WIDTH*0.5f,
                       m_LightShaftArea.t + h*0.5f );

    // Render rotating, zooming fog into alpha channel. Each layer gets bigger and more transparent
    platform_BeginFogRender();
    s32 i;
    for( i = 0; i < LIGHT_SHAFT_NUM_FOG_PASSES; i++ )
    {
        // Lookup shaft # with zoom
        f32 nShaft = i + (m_FogZoom * (f32)LIGHT_SHAFT_NUM_FOG_PASSES);
        while( nShaft >= (f32)LIGHT_SHAFT_NUM_FOG_PASSES )
        {
            nShaft -= (f32)LIGHT_SHAFT_NUM_FOG_PASSES;
        }

        // Compute color
        f32 T    = nShaft / (f32)LIGHT_SHAFT_NUM_FOG_PASSES;
        f32 InvT = 1.0f - T;
        u8  Col  = (u8)(InvT * LIGHT_SHAFT_FOG_ALPHA * 255.0f);

        // Compute rotation speed and radius
        f32 Speed = T;
        f32 R     = LIGHT_SHAFT_FOG_SIZE * Speed;
        f32 U     = 0.0f;

        // Flip direction?
        if( i & 1 )
        {
            Speed = -Speed;
            U     = 1.0f;
        }

        // Draw the fog layer
        platform_DrawFogSprite( FogCenter,
                                vector2( R, R ),
                                vector2( U, 0.0f ),
                                vector2( 1.0f - U, 1.0f ),
                                xcolor( Col, Col, Col, Col ),
                                ( nShaft * R_5 ) + ( m_FogAngle * Speed ) );
    }
    platform_EndFogRender();

    // now just draw the main image over the screen, but don't clobber the
    // alpha channel at all
    platform_SetSrcBuffer( BUFFER_LEVEL_NAME );
    platform_SetDstBuffer( BUFFER_SCREEN, TRUE, FALSE );
    vector2 UL( TEXT_X_POSITION, TEXT_Y_POSITION );
    platform_DrawSprite( UL,
                         vector2( TEXT_IMAGE_WIDTH, TEXT_IMAGE_HEIGHT ),
                         vector2( 0.0f, 0.0f ),
                         vector2( 1.0f, 1.0f ),
                         XCOLOR_WHITE,
                         TRUE );

    // Compute section of light shaft to draw
    f32 X0 = (f32)m_LightShaftArea.l + m_Position * (w - (f32)LIGHT_SHAFT_PIXEL_WIDTH);
    f32 X1 = X0 + (f32)LIGHT_SHAFT_PIXEL_WIDTH;
    f32 U0 = (X0-TEXT_X_POSITION) / (f32)TEXT_IMAGE_WIDTH;
    f32 U1 = (X1-TEXT_X_POSITION) / (f32)TEXT_IMAGE_WIDTH;

    // Render light shafts by rendering layers of scaled up text (each layer more transparent)
    // To remove abrupt horizontal lines, the layers are rendered with 3 quads, with the left/right
    // quads having a completely transparent edge.
    platform_BeginShaftRender();
    for( i = 0; i < LIGHT_SHAFT_NUM_PASSES; i++ )
    {
        // Render single light shaft ( multiply by alpha in frame buffer)
        f32 T    = (f32)i / (f32)LIGHT_SHAFT_NUM_PASSES;
        f32 InvT = 1.0f - T;
        u8  A    = (u8)( 255.0f * LIGHT_SHAFT_ALPHA * InvT );
        f32 WO   = w * ( T * LIGHT_SHAFT_HORIZ_SCALE );
        f32 HO   = h * ( T * LIGHT_SHAFT_VERT_SCALE );
        xcolor   MidCol ( A, A, A, A );
        xcolor   EdgeCol( 0, 0, 0, 0 );

        // Compute rect values
        f32 u0 = U0;
        f32 u1 = U1;
        f32 v0 = 0.0f;
        f32 v1 = 1.0f;
        f32 x0 = (X0-WO);
        f32 y0 = (f32)m_LightShaftArea.t - HO;
        f32 x1 = (X1 + WO);
        f32 y1 = (f32)m_LightShaftArea.t + (h + HO);

        // Compute interior edge values
        f32 F  = 0.2f;
        f32 xi0 = x0 + ( F * ( x1 - x0 ) );
        f32 ui0 = u0 + ( F * ( u1 - u0 ) );
        f32 xi1 = x0 + ( ( 1.0f - F ) * ( x1 - x0 ) );
        f32 ui1 = u0 + ( ( 1.0f - F ) * ( u1 - u0 ) );

        xcolor  Colors[4];
        vector2 UVs[4];
        vector2 Corners[4];

        // Draw left quad (fades from transparent -> opaque)
        Colors[0] = EdgeCol;    UVs[0].Set( u0,  v0 );  Corners[0].Set( x0,  y0 );
        Colors[1] = MidCol;     UVs[1].Set( ui0, v0 );  Corners[1].Set( xi0, y0 );
        Colors[2] = MidCol;     UVs[2].Set( ui0, v1 );  Corners[2].Set( xi0, y1 );
        Colors[3] = EdgeCol;    UVs[3].Set( u0,  v1 );  Corners[3].Set( x0,  y1 );
        platform_DrawShaftQuad( Corners, UVs, Colors );

        // Draw mid (all opaque)
        Colors[0] = MidCol;     UVs[0].Set( ui0, v0 );  Corners[0].Set( xi0, y0 );
        Colors[1] = MidCol;     UVs[1].Set( ui1, v0 );  Corners[1].Set( xi1, y0 );
        Colors[2] = MidCol;     UVs[2].Set( ui1, v1 );  Corners[2].Set( xi1, y1 );
        Colors[3] = MidCol;     UVs[3].Set( ui0, v1 );  Corners[3].Set( xi0, y1 );
        platform_DrawShaftQuad( Corners, UVs, Colors );

        // Draw right quad (fades from opaque -> transparent)
        Colors[0] = MidCol;     UVs[0].Set( ui1, v0 );  Corners[0].Set( xi1, y0 );
        Colors[1] = EdgeCol;    UVs[1].Set( u1,  v0 );  Corners[1].Set( x1,  y0 );
        Colors[2] = EdgeCol;    UVs[2].Set( u1,  v1 );  Corners[2].Set( x1,  y1 );
        Colors[3] = MidCol;     UVs[3].Set( ui1, v1 );  Corners[3].Set( xi1, y1 );
        platform_DrawShaftQuad( Corners, UVs, Colors );
    }
    platform_EndShaftRender();
}

//==============================================================================
//  PLATFORM-SPECIFIC HELPER FUNCTIONS
//==============================================================================

#ifdef TARGET_XBOX
void dlg_load_game::xbox_ClipSprite( vector3&          UL,
                                     vector2&          Size,
                                     vector2&          UV0,
                                     vector2&          UV1 )
{
    // don't clip collapsed sprites--that will cause divide-by-zeroes
    if( (Size.X == 0.0f) || (Size.Y == 0.0f) )
    {
        return;
    }

    vector3 BR = UL + vector3( Size.X, Size.Y, 0 );

    // clip to the top edge
    if( UL.GetY() < 0.0f )
    {
        f32 T     = (0.0f - UL.GetY()) / (BR.GetY() - UL.GetY());
        UL.GetY() = UL.GetY() + T * (BR.GetY() - UL.GetY());
        UV0.Y     = UV0.Y + T * (UV1.Y - UV0.Y);
    }

    // clip to the bottom edge
    if( BR.GetY() > (f32)m_BufferH )
    {
        f32 T     = ((f32)m_BufferH - UL.GetY()) / (BR.GetY() - UL.GetY());
        BR.GetY() = UL.GetY() + T * (BR.GetY() - UL.GetY());
        UV1.Y     = UV0.Y + T * (UV1.Y - UV0.Y);
    }

    // clip to the left edge
    if( UL.GetX() < 0.0f )
    {
        f32 T     = (0.0f - UL.GetX()) / (BR.GetX() - UL.GetX());
        UL.GetX() = UL.GetX() + T * (BR.GetX() - UL.GetX());
        UV0.X     = UV0.X + T * (UV1.X - UV0.X);
    }

    // clip to the right edge
    if( BR.GetX() > (f32)m_BufferW )
    {
        f32 T     = ((f32)m_BufferW - UL.GetX()) / (BR.GetX() - UL.GetX());
        BR.GetX() = UL.GetX() + T * (BR.GetX() - UL.GetX());
        UV1.X     = UV0.X + T * (UV1.X - UV0.X);
    }

    Size.X = BR.GetX() - UL.GetX();
    Size.Y = BR.GetY() - UL.GetY();
}
#endif // TARGET_XBOX

//==============================================================================

#ifdef TARGET_PC
void dlg_load_game::pc_ClipSprite(   vector3&          UL,
                                     vector2&          Size,
                                     vector2&          UV0,
                                     vector2&          UV1 )
{
    // don't clip collapsed sprites--that will cause divide-by-zeroes
    if( (Size.X == 0.0f) || (Size.Y == 0.0f) )
    {
        return;
    }

    vector3 BR = UL + vector3( Size.X, Size.Y, 0 );

    // clip to the top edge
    if( UL.GetY() < 0.0f )
    {
        f32 T     = (0.0f - UL.GetY()) / (BR.GetY() - UL.GetY());
        UL.GetY() = UL.GetY() + T * (BR.GetY() - UL.GetY());
        UV0.Y     = UV0.Y + T * (UV1.Y - UV0.Y);
    }

    // clip to the bottom edge
    if( BR.GetY() > (f32)m_BufferH )
    {
        f32 T     = ((f32)m_BufferH - UL.GetY()) / (BR.GetY() - UL.GetY());
        BR.GetY() = UL.GetY() + T * (BR.GetY() - UL.GetY());
        UV1.Y     = UV0.Y + T * (UV1.Y - UV0.Y);
    }

    // clip to the left edge
    if( UL.GetX() < 0.0f )
    {
        f32 T     = (0.0f - UL.GetX()) / (BR.GetX() - UL.GetX());
        UL.GetX() = UL.GetX() + T * (BR.GetX() - UL.GetX());
        UV0.X     = UV0.X + T * (UV1.X - UV0.X);
    }

    // clip to the right edge
    if( BR.GetX() > (f32)m_BufferW )
    {
        f32 T     = ((f32)m_BufferW - UL.GetX()) / (BR.GetX() - UL.GetX());
        BR.GetX() = UL.GetX() + T * (BR.GetX() - UL.GetX());
        UV1.X     = UV0.X + T * (UV1.X - UV0.X);
    }

    Size.X = BR.GetX() - UL.GetX();
    Size.Y = BR.GetY() - UL.GetY();
}
#endif // TARGET_PC

//==============================================================================

#ifdef TARGET_PS2
void dlg_load_game::ps2_CopyRG2BA( s32 XRes, s32 YRes, s32 FBP )
{
    // Need to copy double the height since we are copying 16-bit instead of 32-bit
    (void)XRes;
    s32 CopyHeight = YRes*2;

    // Copy 8 pixel wide columns at a time
    s32 ColumnWidth = 8;
    s32 nColumns    = VRAM_FRAME_BUFFER_WIDTH / ColumnWidth;

    // Setup GS registers for the copy
    gsreg_Begin( 11 + 4*(nColumns/2) );
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( FBP, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT16, 0x00003FFF ) );
    gsreg_Set( SCE_GS_ZBUF_1,    SCE_GS_SET_ZBUF     ( FBP, SCE_GS_PSMZ24, 1 ) );
    gsreg_Set( SCE_GS_TEST_1,    SCE_GS_SET_TEST_1   ( 0, 0, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS ) );
    gsreg_SetClamping(FALSE, FALSE);
    gsreg_Set( SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR_1( 0, VRAM_FRAME_BUFFER_WIDTH-1, 0, CopyHeight-1 ) );
    gsreg_Set( SCE_GS_TEXFLUSH,  0 );
    gsreg_Set( SCE_GS_TEXA,      SCE_GS_SET_TEXA     ( 0x00, 0, 0x80 ) );
    gsreg_Set( SCE_GS_TEX1_1,    SCE_GS_SET_TEX1_1   ( 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1,    SCE_GS_SET_TEX0_1   ( FBP*32, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT16, 10, 10, 1, 1, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_PRIM,      SCE_GS_SET_PRIM     ( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,     SCE_GS_SET_RGBAQ    ( 128, 128, 128, 128, 0x3F800000 ) );

    s32 X  = 0;
    s32 Z  = 0;
    s32 V0 = 0;
    s32 Y0 = PS2_SCISSOR_Y;
    s32 V1 = V0 + CopyHeight;
    s32 Y1 = Y0 + CopyHeight;

    // Copy every column to the right
    for( s32 i=0; i<nColumns; i+=2 )
    {
        s32 U0 = X;
        s32 X0 = PS2_SCISSOR_X + X + ColumnWidth;

        s32 U1 = U0 + ColumnWidth;
        s32 X1 = X0 + ColumnWidth;

        gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U0 << 4) + 8, (V0 << 4) + 8 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4),     (Y0 << 4),  Z ) );
        gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U1 << 4) + 8, (V1 << 4) + 8 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4),     (Y1 << 4),  Z ) );

        X += ColumnWidth * 2;
    }

    gsreg_End();

    // reset the screen buffer
    platform_SetDstBuffer( BUFFER_SCREEN );

    // reset the zbuffer
    gsreg_Begin( 1 );
    gsreg_Set( SCE_GS_ZBUF_1, SCE_GS_SET_ZBUF_1( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 1 ) );
    gsreg_End();
}
#endif

//==============================================================================

void dlg_load_game::platform_Init( void )
{
#if defined( TARGET_PS2 )
    // let vram eat up the z-buffer in addition to the normal vram space
    vram_SetStartAddress( VRAM_ZBUFFER_START/256 );

    // store out what the system defaults to having for the permanent
    // vram area, because we'll be overriding that and need to restore it
    m_OrigPermanentSize = vram_GetPermanentSize();

    // now allocate enough vram to handle five 512x512 images, plus a
    // page for their palettes, plus a workspace area for the text
    // (512x64), plus two workspace areas for the drop shadow (256x32).
    // This is the area that we'll be managing manually rather than letting
    // the vram system do it.
    s32 nPages = (SLIDESHOW_IMAGE_WIDTH*SLIDESHOW_IMAGE_HEIGHT*MAX_TEXTURES)/8192 +
                 1 +
                 (TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4)/8192 +
                 ((TEXT_IMAGE_WIDTH/2)*(TEXT_IMAGE_HEIGHT/2)*4*2)/8192;
    if( m_OrigPermanentSize )
        vram_FreePermanent();
    vram_AllocatePermanent( nPages );
    vram_Flush();

    // disable the page flip screen clear (because it also wipes the
    // z-buffer that we are using, we will handle any screen clears
    // ourselves)
    eng_EnableScreenClear( FALSE );

#elif defined( TARGET_XBOX )
    // Set up the default write mask
    m_ColorWriteMask = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    
    g_RenderTarget.Reset();
#elif defined( TARGET_PC )
    // Make sure everything is initialized to NULL first
    m_pBackBuffer = NULL;
    x_memset( m_Buffers, 0, sizeof(m_Buffers) );
    
    // Set up the default write mask
    m_ColorWriteMask = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    
    // Only proceed if we have a valid device
    if( !g_pd3dDevice )
        return;
    
    // Create render target textures for each buffer
    D3DFORMAT format = D3DFMT_A8R8G8B8;
    HRESULT hr;
    
    // Level name buffer
    hr = g_pd3dDevice->CreateTexture( TEXT_IMAGE_WIDTH, TEXT_IMAGE_HEIGHT, 
                                     1, D3DUSAGE_RENDERTARGET, format, 
                                     D3DPOOL_DEFAULT, &m_Buffers[BUFFER_LEVEL_NAME], NULL);
    if( FAILED(hr) )
    {
        m_Buffers[BUFFER_LEVEL_NAME] = NULL;
    }
                              
    // Drop shadow buffers (2)
    hr = g_pd3dDevice->CreateTexture( TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2, 
                                     1, D3DUSAGE_RENDERTARGET, format, 
                                     D3DPOOL_DEFAULT, &m_Buffers[BUFFER_DROP_SHADOW_1], NULL);
    if( FAILED(hr) )
    {
        m_Buffers[BUFFER_DROP_SHADOW_1] = NULL;
    }
                              
    hr = g_pd3dDevice->CreateTexture( TEXT_IMAGE_WIDTH/2, TEXT_IMAGE_HEIGHT/2, 
                                     1, D3DUSAGE_RENDERTARGET, format, 
                                     D3DPOOL_DEFAULT, &m_Buffers[BUFFER_DROP_SHADOW_2], NULL);
    if( FAILED(hr) )
    {
        m_Buffers[BUFFER_DROP_SHADOW_2] = NULL;
    }
    
    // Init buffer dimensions
    m_BufferW = TEXT_IMAGE_WIDTH;
    m_BufferH = TEXT_IMAGE_HEIGHT;
#endif
}

//==============================================================================

void dlg_load_game::platform_Destroy( void )
{
#if defined(TARGET_PS2)
    // restore the permanent allocation area if necessary
    if( m_OrigPermanentSize != -1 )
    {
        vram_FreePermanent();
        vram_AllocatePermanent( m_OrigPermanentSize );
    }

    // let vram start AFTER the zbuffer now
    vram_SetStartAddress( VRAM_FREE_MEMORY_START/256 );

    // re-enable the page flip screen clear
    eng_EnableScreenClear( TRUE );
#elif defined(TARGET_XBOX)
    g_RenderTarget.Reset();

    // Make sure we destroy any slideshow images
    // There's no need to redirect the texture allocator here
    // because all redirected allocations are aliases of tiled RAM
    for( s32 i=0;i<m_nSlides;i++ )
    {
        if( !m_Slides[i].HasImage )
            continue;
        vram_Unregister( m_Slides[i].BMP );
        m_Slides[i].HasImage = FALSE;
        m_Slides[i].BMP.Kill();
    }
#elif defined( TARGET_PC )
    // Release render target textures
    if( m_pBackBuffer )
    {
        m_pBackBuffer->Release();
        m_pBackBuffer = NULL;
    }
    
    // Release all render target buffers
    for( s32 i = 0; i < BUFFER_COUNT; i++ )
    {
        if( m_Buffers[i] )
        {
            m_Buffers[i]->Release();
            m_Buffers[i] = NULL;
        }
    }
    
    // Make sure we destroy any slideshow images
    for( s32 i = 0; i < m_nSlides; i++ )
    {
        if( !m_Slides[i].HasImage )
            continue;
        vram_Unregister( m_Slides[i].BMP );
        m_Slides[i].HasImage = FALSE;
        m_Slides[i].BMP.Kill();
    }
    
    // Reset platform-specific variables
    m_ColorWriteMask = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    m_BufferW = 0;
    m_BufferH = 0;
#endif
}

//=========================================================================

void dlg_load_game::platform_LoadSlide( s32         Index,
                                        s32         TextureIndex,
                                        const char* pTextureName )
{
#if defined( TARGET_PS2 )
    // The PS2 doesn't have enough RAM to handle all the level data plus
    // the slides. So instead what it will do is load up the slides and
    // stuff them into vram. This makes the whole process rather annoying,
    // since now we can't refer to the images as standard BMP's, but it's
    // worth it for the memory savings. Other plats will just have to
    // suck it up and eat the memory.
    eng_Begin( "LoadSlideshowBMP" );

    ASSERT( (Index>=0) && (Index<m_nSlides) );

    // figure out where this image will go in vram
    s32 ClutAddr  = vram_GetPermanentArea();
    s32 ImageAddr = ClutAddr + 32;
    ClutAddr  += 4 * TextureIndex;
    ImageAddr += ((SLIDESHOW_IMAGE_WIDTH*SLIDESHOW_IMAGE_HEIGHT)/256) * TextureIndex;

    // load the bitmap (don't bother with the resource manager,
    // this is a case where the bitmap will be loaded, moved into
    // vram and immediately thrown away)
    xbitmap InputBMP;
    xbool   Success = InputBMP.Load( xfs( "%s\\%s", g_RscMgr.GetRootDirectory(), pTextureName ) );
    if( !Success )
    {
        m_Slides[Index].Tex0 = 0;
        m_Slides[Index].HasImage = FALSE;
    }
    else
    {
        // assert on valid data
        if( (InputBMP.GetWidth() != SLIDESHOW_IMAGE_WIDTH) ||
            (InputBMP.GetHeight() != SLIDESHOW_IMAGE_HEIGHT) ||
            (InputBMP.GetBPP() > 8) )
        {
            ASSERTS( FALSE, "Invalid slideshow image" );
            m_Slides[Index].Tex0 = 0;
        }
        else
        {
            // because the data we are ref'ing is only temporary, we need to immediately
            // flush the cache
            FlushCache( WRITEBACK_DCACHE );

            // upload the bitmap data into the permanent area
            vram_LoadTextureImage( SLIDESHOW_IMAGE_WIDTH,
                SLIDESHOW_IMAGE_HEIGHT,
                SLIDESHOW_IMAGE_WIDTH/64,
                InputBMP.GetPixelData(),
                (InputBMP.GetBPP() == 8) ? SCE_GS_PSMT8 : SCE_GS_PSMT4,
                ImageAddr );
            vram_LoadClutData( InputBMP.GetBPP(), ClutAddr, InputBMP.GetClutData() );

            // now build the tex0 register that will be used for activating this
            // texture before render
            m_Slides[Index].Tex0 = SCE_GS_SET_TEX0( ImageAddr,
                SLIDESHOW_IMAGE_WIDTH/64,
                (InputBMP.GetBPP() == 8) ? SCE_GS_PSMT8 : SCE_GS_PSMT4,
                vram_GetLog2( SLIDESHOW_IMAGE_WIDTH ),
                vram_GetLog2( SLIDESHOW_IMAGE_HEIGHT ),
                1,
                0,
                ClutAddr,
                SCE_GS_PSMCT32,
                0, 0, 1 );

            // because the data we are ref'ing is only temporary, we need to immediately
            // wait for the dlist to handle it
            DLIST.Flush();
            DLIST.WaitForTasks();

            // we are done with this image now that it has been moved into vram
            InputBMP.Kill();
        }
    }

    // finished
    eng_End();
#elif defined( TARGET_XBOX )
    (void)TextureIndex;

    // Other targets we don't really care about giving up the memory.
    // Just load it as though it were any other image.
    xbool Success = m_Slides[Index].BMP.Load( xfs( "%s\\%s", g_RscMgr.GetRootDirectory(), pTextureName ) );
    if( Success )
    {
        vram_Register( m_Slides[Index].BMP );
    #ifdef X_DEBUG
        vram_Activate( m_Slides[Index].BMP );
    #endif
    }
    else
    {
        m_Slides[Index].HasImage = FALSE;
    }
#elif defined( TARGET_PC )
    (void)TextureIndex;

    // Other targets we don't really care about giving up the memory.
    // Just load it as though it were any other image.
    xbool Success = m_Slides[Index].BMP.Load( xfs( "%s\\%s", g_RscMgr.GetRootDirectory(), pTextureName ) );
    if( Success )
    {
        vram_Register( m_Slides[Index].BMP );
    //#ifdef X_DEBUG
    //    vram_Activate( m_Slides[Index].BMP );
    //#endif
    }
    else
    {
        m_Slides[Index].HasImage = FALSE;
    }
#endif
}

//==============================================================================

void dlg_load_game::platform_FillScreen( xcolor C )
{
#if defined( TARGET_PS2 )
    irect Rect;
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    platform_SetDstBuffer( BUFFER_SCREEN );
    Rect.Set( 0, 0, XRes, YRes );
    draw_RectImmediate( Rect, C );
#elif defined( TARGET_XBOX )
    irect Rect;
    Rect.Set( 0, 0, g_PhysW, g_PhysH );
    draw_Rect( Rect, C, FALSE );
#elif defined( TARGET_PC )
    irect Rect;
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    Rect.Set( 0, 0, XRes, YRes );
    draw_Rect( Rect, C, FALSE );
#endif
}

//=========================================================================

void dlg_load_game::platform_RenderSlide( s32 SlideIndex, xcolor C )
{
#if defined( TARGET_PS2 )
    // start up the drawing mode
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_BLEND_ADD );

    // activate the sprite texture, and use a custom blend setting
    gsreg_Begin( 4 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1, m_Slides[SlideIndex].Tex0 );
    gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_SRC, C_ZERO, 0 ) );
    gsreg_End();

    // crop the image based on whether or not we're PAL or NTSC
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    f32 CropAmount = (f32)((VRAM_FRAME_BUFFER_HEIGHT-YRes)/2) / (f32)VRAM_FRAME_BUFFER_HEIGHT;

    // draw the sprite
    draw_SpriteImmediate( vector2(0.0f, 0.0f),
                          vector2(XRes, YRes),
                          vector2(0.0f,CropAmount),
                          vector2(1.0f,1.0f-CropAmount),
                          C );
    draw_End();
#elif defined( TARGET_XBOX )
    // Start up the drawing mode
    draw_EnableBilinear();
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_BLEND_ADD );

    // Activate the sprite texture
    draw_SetTexture( m_Slides[SlideIndex].BMP );

    // Draw the sprite
    draw_Sprite( vector3( 0.0f, 0.0f, 0.0f ),
                 vector2( (f32)g_PhysW, (f32)g_PhysH ),
                 C );

    // Finished
    draw_End();
#elif defined( TARGET_PC )
    // Start up the drawing mode
    draw_EnableBilinear();
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_BLEND_ADD );

    // Activate the sprite texture
    draw_SetTexture( m_Slides[SlideIndex].BMP );

    // Draw the sprite
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    draw_Sprite( vector3( 0.0f, 0.0f, 0.0f ),
                 vector2( (f32)XRes, (f32)YRes ),
                 C );

    // Finished
    draw_End();
#endif
}

//==============================================================================

void dlg_load_game::platform_GetBufferInfo( vram_buffer       BufferID,
                                            s32&              MemOffset,
                                            s32&              BufferW,
                                            s32&              BufferH )
{
#if defined(TARGET_PS2)
    s32 TextBufferStart = vram_GetPermanentArea() +
                          (SLIDESHOW_IMAGE_WIDTH*SLIDESHOW_IMAGE_HEIGHT*MAX_TEXTURES)/256 +
                          32; // custom area start + slide images + slide cluts
    switch( BufferID )
    {
    default:
        ASSERT(FALSE);
        break;
    case BUFFER_SCREEN:
        MemOffset = eng_GetFrameBufferAddr(0) / 64;
        eng_GetRes( BufferW, BufferH );
        break;
    case BUFFER_LEVEL_NAME:
        MemOffset = TextBufferStart;
        BufferW   = TEXT_IMAGE_WIDTH;
        BufferH   = TEXT_IMAGE_HEIGHT;
        break;
    case BUFFER_DROP_SHADOW_1:
        MemOffset = TextBufferStart +
                    (TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4)/256;
        BufferW   = TEXT_IMAGE_WIDTH/2;
        BufferH   = TEXT_IMAGE_HEIGHT/2;
        break;
    case BUFFER_DROP_SHADOW_2:
        MemOffset = TextBufferStart +
                    (TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4)/256 +
                    ((TEXT_IMAGE_WIDTH/2)*(TEXT_IMAGE_HEIGHT/2)*4)/256;
        BufferW   = TEXT_IMAGE_WIDTH/2;
        BufferH   = TEXT_IMAGE_HEIGHT/2;
        break;
    }
#elif defined( TARGET_XBOX )
    switch( BufferID )
    {
    default:
        ASSERT( FALSE );
        break;
    case BUFFER_SCREEN:
        MemOffset = 0;
        BufferW   = g_PhysW;
        BufferH   = g_PhysH;
        break;
    case BUFFER_LEVEL_NAME:
        MemOffset = 0;
        BufferW   = TEXT_IMAGE_WIDTH;
        BufferH   = TEXT_IMAGE_HEIGHT;
        break;
    case BUFFER_DROP_SHADOW_1:
        MemOffset = TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4;
        BufferW   = TEXT_IMAGE_WIDTH/2;
        BufferH   = TEXT_IMAGE_HEIGHT/2;
        break;
    case BUFFER_DROP_SHADOW_2:
        MemOffset = TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4 +
                    (TEXT_IMAGE_WIDTH/2)*(TEXT_IMAGE_HEIGHT/2)*4;
        BufferW   = TEXT_IMAGE_WIDTH/2;
        BufferH   = TEXT_IMAGE_HEIGHT/2;
        break;
    }
#elif defined( TARGET_PC )
    switch( BufferID )
    {
    default:
        ASSERT( FALSE );
        break;
    case BUFFER_SCREEN:
        MemOffset = 0;
        eng_GetRes( BufferW, BufferH );
        break;
    case BUFFER_LEVEL_NAME:
        MemOffset = 0;
        BufferW   = TEXT_IMAGE_WIDTH;
        BufferH   = TEXT_IMAGE_HEIGHT;
        break;
    case BUFFER_DROP_SHADOW_1:
        MemOffset = TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4;
        BufferW   = TEXT_IMAGE_WIDTH/2;
        BufferH   = TEXT_IMAGE_HEIGHT/2;
        break;
    case BUFFER_DROP_SHADOW_2:
        MemOffset = TEXT_IMAGE_WIDTH*TEXT_IMAGE_HEIGHT*4 +
                    (TEXT_IMAGE_WIDTH/2)*(TEXT_IMAGE_HEIGHT/2)*4;
        BufferW   = TEXT_IMAGE_WIDTH/2;
        BufferH   = TEXT_IMAGE_HEIGHT/2;
        break;
    }
#endif
}

//=============================================================================

void dlg_load_game::platform_SetSrcBuffer( vram_buffer BufferID )
{
#if defined(TARGET_PS2)

    // grab out the buffer info
    s32 BufferAddr;
    s32 BufferW;
    s32 BufferH;
    platform_GetBufferInfo( BufferID, BufferAddr, BufferW, BufferH );

    // set up the texture registers
    gsreg_Begin( 3 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1, SCE_GS_SET_TEX0( BufferAddr,
                                               BufferW/64,
                                               SCE_GS_PSMCT32,
                                               vram_GetLog2( BufferW ),
                                               vram_GetLog2( BufferH ),
                                               0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_End();

#elif defined(TARGET_XBOX)
    ASSERT( BufferID != BUFFER_SCREEN );

    // Figure out the buffer info
    s32 BufferW;
    s32 BufferH;
    s32 MemOffset;
    platform_GetBufferInfo( BufferID, MemOffset, BufferW, BufferH );

    // Get base of tiled memory
    u8* BaseTiledPtr = (u8*)g_TextureFactory.GetTiledPool().GetBase();
    BaseTiledPtr += MemOffset;

    // Alias some other texture buffer memory from the system
    texture_factory::handle Handle = g_TextureFactory.Alias(
        BufferW*4,
        (u32)BufferW,
        (u32)BufferH,
        D3DFMT_A8R8G8B8,
        BaseTiledPtr,
        texture_factory::ALIAS_FROM_SCRATCH
    );

    ASSERT( Handle );
    g_Texture.Set( 0, Handle );
#elif defined( TARGET_PC )
    ASSERT(BufferID != BUFFER_SCREEN);

    // Figure out the buffer info
    s32 BufferW;
    s32 BufferH;
    s32 MemOffset;
    platform_GetBufferInfo( BufferID, MemOffset, BufferW, BufferH );

    // Set up the texture
    IDirect3DTexture9* pTexture = ( IDirect3DTexture9* )m_Buffers[BufferID];
    g_pd3dDevice->SetTexture( 0, pTexture );
#endif
}

//==============================================================================

void dlg_load_game::platform_SetDstBuffer( vram_buffer BufferID,
                                           xbool       EnableRGBChannel,
                                           xbool       EnableAlphaChannel )
{
#if defined(TARGET_PS2)

    // figure out the frame mask
    u32 Mask = 0x00000000;
    if( !EnableRGBChannel )
        Mask |= 0x00FFFFFF;
    if( !EnableAlphaChannel )
        Mask |= 0xFF000000;

    // grab out the buffer info
    s32 BufferAddr;
    s32 BufferW;
    s32 BufferH;
    platform_GetBufferInfo( BufferID, BufferAddr, BufferW, BufferH );

    // set up the frame registers
    gsreg_Begin( 2 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( BufferAddr/32, BufferW/64, SCE_GS_PSMCT32, Mask ) );
    gsreg_SetScissor( 0, 0, BufferW, BufferH );
    gsreg_End();

#elif defined(TARGET_XBOX)
    if( BufferID == BUFFER_SCREEN )
    {
        g_pPipeline->SetRenderTarget( pipeline_mgr::kLAST, -1 );
        m_BufferW = g_PhysW;
        m_BufferH = g_PhysH;
    }
    else
    {
        // Figure out the buffer info
        s32 MemOffset;
        platform_GetBufferInfo( BufferID, MemOffset, m_BufferW, m_BufferH );

        // Get base of tiled memory
        u8* BaseTiledPtr = (u8*)g_TextureFactory.GetTiledPool().GetBase();
        BaseTiledPtr += MemOffset;

        // Alias some other texture buffer memory from the system
        texture_factory::handle Handle = g_TextureFactory.Alias(
            m_BufferW*4,
            (u32)m_BufferW,
            (u32)m_BufferH,
            D3DFMT_A8R8G8B8,
            BaseTiledPtr,
            texture_factory::ALIAS_FROM_SCRATCH
        );
        ASSERT( Handle );
        IDirect3DSurface8* Surface;
        if( Handle )
            Handle->GetSurfaceLevel( 0,&Surface );
        g_RenderTarget.Set( Surface,NULL );
        Surface->Release();
    }

    // Set up the color write mask
    m_ColorWriteMask = 0;
    if( EnableRGBChannel )
        m_ColorWriteMask |= D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    if( EnableAlphaChannel )
        m_ColorWriteMask |= D3DCOLORWRITEENABLE_ALPHA;
#elif defined( TARGET_PC )
    if( BufferID == BUFFER_SCREEN )
    {
        if ( m_pBackBuffer != NULL )
        {
            g_pd3dDevice->SetRenderTarget( 0, m_pBackBuffer );
            m_pBackBuffer->Release();
            m_pBackBuffer = NULL;
        }
        s32 Width, Height;
        eng_GetRes( Width, Height );
        m_BufferW = Width;
        m_BufferH = Height;
    }
    else
    {
        // Figure out the buffer info
        s32 MemOffset;
        platform_GetBufferInfo( BufferID, MemOffset, m_BufferW, m_BufferH );
        
        IDirect3DTexture9* pTexture = ( IDirect3DTexture9* )m_Buffers[BufferID];
        IDirect3DSurface9* pSurface = NULL;
        pTexture->GetSurfaceLevel( 0, &pSurface );
        
        if( m_pBackBuffer == NULL )
        {
            g_pd3dDevice->GetRenderTarget( 0, &m_pBackBuffer );
        }
        
        g_pd3dDevice->SetRenderTarget( 0, pSurface );
        pSurface->Release();
    }

    // Set up the color write mask
    m_ColorWriteMask = 0;
    if( EnableRGBChannel )
        m_ColorWriteMask |= D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
    if( EnableAlphaChannel )
        m_ColorWriteMask |= D3DCOLORWRITEENABLE_ALPHA;
    
    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, m_ColorWriteMask );
#endif
}

//==============================================================================

void dlg_load_game::platform_ClearBuffer( vram_buffer BufferID, xbool EnableRGBChannel, xbool EnableAlphaChannel )
{
#if defined( TARGET_PS2 )
    s32 BufferAddr, BufferW, BufferH;
    platform_GetBufferInfo( BufferID, BufferAddr, BufferW, BufferH );
    platform_SetDstBuffer( BufferID, EnableRGBChannel, EnableAlphaChannel );
    irect Rect;
    Rect.Set( 0, 0, BufferW, BufferH );
    draw_RectImmediate( Rect, XCOLOR_BLACK );
#elif defined( TARGET_XBOX )
    platform_SetDstBuffer( BufferID, EnableRGBChannel, EnableAlphaChannel );
    u32 Flags = 0;
    if( EnableRGBChannel )
        Flags |= D3DCLEAR_TARGET_R | D3DCLEAR_TARGET_G | D3DCLEAR_TARGET_B;
    if( EnableAlphaChannel )
        Flags |= D3DCLEAR_TARGET_A;
    g_pd3dDevice->Clear( 0,0,Flags,0,0.0f,0 );
#elif defined( TARGET_PC )
    platform_SetDstBuffer( BufferID, EnableRGBChannel, EnableAlphaChannel ); 
    u32 Flags = 0;
    if( EnableRGBChannel )
        Flags |= D3DCLEAR_TARGET;
    if( EnableAlphaChannel )
        Flags |= D3DCLEAR_TARGET;
    g_pd3dDevice->Clear( 0,0,Flags,0,0.0f,0 );
#endif
}

//=========================================================================

void dlg_load_game::platform_DrawSprite( const vector2& UpperLeft,
                                         const vector2& Size,
                                         const vector2& UV0,
                                         const vector2& UV1,
                                         xcolor         C,
                                         xbool          Additive )
{
#if defined( TARGET_PS2 )
    u32 DrawFlags = DRAW_2D | DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE;
    if( Additive )
        DrawFlags |= DRAW_BLEND_ADD;
    draw_Begin( DRAW_SPRITES, DrawFlags );
    draw_SpriteImmediate( UpperLeft,
                          Size,
                          UV0,
                          UV1,
                          C );
    draw_End();
#elif defined( TARGET_XBOX )
    // Xbox doesn't seem to like sprites that go outside the view bounds.
    // We'll clip it manually.

    vector3 ClippedUL  ( UpperLeft.X, UpperLeft.Y, 0.0f );
    vector2 ClippedSize( Size );
    vector2 ClippedUV0 ( UV0 );
    vector2 ClippedUV1 ( UV1 );
    xbox_ClipSprite( ClippedUL, ClippedSize, ClippedUV0, ClippedUV1 );

    u32 DrawFlags = DRAW_2D             |
                    DRAW_USE_ALPHA      |
                    DRAW_CULL_NONE      |
                    DRAW_TEXTURED       |
                    DRAW_NO_ZBUFFER     |
                    DRAW_NO_ZWRITE      |
                    DRAW_XBOX_NO_BEGIN;
                    
    if( Additive )
        DrawFlags |= DRAW_BLEND_ADD;
    
    draw_EnableBilinear();
    draw_Begin( DRAW_SPRITES, DrawFlags );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE, m_ColorWriteMask );
    
    g_TextureStageState.Set( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    
    draw_Begin( DRAW_SPRITES, DRAW_KEEP_STATES );
    draw_SpriteUV( ClippedUL,
                   ClippedSize,
                   ClippedUV0,
                   ClippedUV1,
                   C );
    draw_End();
#elif defined( TARGET_PC )
    // PC doesn't seem to like sprites that go outside the view bounds.
    // We'll clip it manually.

    vector3 ClippedUL  ( UpperLeft.X, UpperLeft.Y, 0.0f );
    vector2 ClippedSize( Size );
    vector2 ClippedUV0 ( UV0 );
    vector2 ClippedUV1 ( UV1 );
    pc_ClipSprite( ClippedUL, ClippedSize, ClippedUV0, ClippedUV1 );
    
    u32 DrawFlags =   DRAW_2D         | 
                      DRAW_USE_ALPHA  | 
                      DRAW_CULL_NONE  | 
                      DRAW_TEXTURED   | 
                      DRAW_NO_ZBUFFER | 
                      DRAW_NO_ZWRITE;
    
    if( Additive )
        DrawFlags |= DRAW_BLEND_ADD;
        
    draw_EnableBilinear();
    draw_Begin( DRAW_SPRITES, DrawFlags );
    
    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, m_ColorWriteMask );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
    
    draw_SpriteUV( ClippedUL,
                   ClippedSize,
                   ClippedUV0,
                   ClippedUV1,
                   C );
                  
    draw_End();
#endif
}

//=========================================================================

void dlg_load_game::platform_BeginFogRender( void )
{
#if defined( TARGET_PS2 )
    // set the fog bmp texture
    draw_SetTexture( m_FogBMP );

    // render to the screen
    platform_SetDstBuffer( BUFFER_SCREEN );

    // turn on texture clamping and set up an additive render mode
    gsreg_Begin( 2 );
    gsreg_Set( SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_SRC, C_DST, 0x80 ) );
    gsreg_End();
#elif defined( TARGET_XBOX )
    // Make sure the screen is cleared to start
    platform_ClearBuffer( BUFFER_SCREEN, FALSE, TRUE );

    // Render to the screen, but mask out everything except alpha
    platform_SetDstBuffer( BUFFER_SCREEN, FALSE, TRUE );

    // Begin drawing
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA     |
                              DRAW_TEXTURED      |
                              DRAW_2D            |
                              DRAW_NO_ZBUFFER    |
                              DRAW_NO_ZWRITE     |
                              DRAW_BLEND_ADD     |
                              DRAW_XBOX_NO_BEGIN );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE, m_ColorWriteMask );
    draw_Begin( DRAW_SPRITES, DRAW_KEEP_STATES );
    draw_SetTexture( m_FogBMP );
#elif defined( TARGET_PC )
    // Make sure the screen is cleared to start
    platform_ClearBuffer( BUFFER_SCREEN, FALSE, TRUE );

    // Render to the screen, but mask out everything except alpha
    platform_SetDstBuffer( BUFFER_SCREEN, FALSE, TRUE );

    // Begin drawing
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | 
                              DRAW_TEXTURED  | 
                              DRAW_2D        | 
                              DRAW_NO_ZBUFFER| 
                              DRAW_NO_ZWRITE | 
                              DRAW_BLEND_ADD );                           
    draw_SetTexture( m_FogBMP );
#endif
}

//=========================================================================

void dlg_load_game::platform_EndFogRender( void )
{
#if defined( TARGET_PS2 )
    // On the ps2 we can't blend with the frame buffer alpha, so instead
    // we have to blend the colors, then copy that into the alpha channel.
    // And once we've finished with that, we better clear the screen again.
    s32   XRes, YRes;
    xbool PALMode;
    eng_GetRes( XRes, YRes );
    eng_GetPALMode( PALMode );
    s32 FBP = eng_GetFrameBufferAddr(0) / 2048;
    if( PALMode )
    {
        // The texture range is too limited to do this function in
        // one go at the higher resolution, so in PAL we need to do
        // this in two steps
        ps2_CopyRG2BA( XRes, YRes/2, FBP );
        ps2_CopyRG2BA( XRes, YRes/2, FBP + (VRAM_FRAME_BUFFER_WIDTH*VRAM_FRAME_BUFFER_HEIGHT*2/8192) );
    }
    else
    {
        ps2_CopyRG2BA( XRes, YRes, FBP );
    }

   
    platform_ClearBuffer( BUFFER_SCREEN, TRUE, FALSE );
#elif defined( TARGET_XBOX )
    draw_End();
#elif defined( TARGET_PC )
    draw_End();
#endif
}

//=========================================================================

void dlg_load_game::platform_DrawFogSprite( const vector2&    SpriteCenter,
                                            const vector2&    WH,
                                            const vector2&    UV0,
                                            const vector2&    UV1,
                                            xcolor            C,
                                            radian            Rotation )
{
#if defined( TARGET_PS2 )
    // IMPORTANT NOTE:
    // We can't use the draw system here because it would require vector3's.
    // See the note in the header file about why we can't use vector3!
    vector2 Corners[4];
    vector2 UVs[4];
    s32     CornerX[4];
    s32     CornerY[4];
    u32     Us[4];
    u32     Vs[4];
    Corners[0].Set( -WH.X/2, -WH.Y/2 ); UVs[0].Set( UV0.X, UV0.Y );
    Corners[1].Set(  WH.X/2, -WH.Y/2 ); UVs[1].Set( UV1.X, UV0.Y );
    Corners[2].Set(  WH.X/2,  WH.Y/2 ); UVs[2].Set( UV1.X, UV1.Y );
    Corners[3].Set( -WH.X/2,  WH.Y/2 ); UVs[3].Set( UV0.X, UV1.Y );
    for( s32 i = 0; i < 4; i++ )
    {
        // rotate and translate to position the corner
        Corners[i].Rotate( Rotation );
        Corners[i] += SpriteCenter;

        // convert to a ps2-friendly format
        CornerX[i] = (s32)(Corners[i].X*16.0f) + (PS2_SCISSOR_X<<4);
        CornerY[i] = (s32)(Corners[i].Y*16.0f) + (PS2_SCISSOR_Y<<4);
        Us[i]      = reinterpret_cast<u32&>(UVs[i].X);
        Vs[i]      = reinterpret_cast<u32&>(UVs[i].Y);
    }

    // make the color ps2-friendly
    C.R = (C.R==255) ? 128 : (C.R>>1);
    C.G = (C.G==255) ? 128 : (C.G>>1);
    C.B = (C.B==255) ? 128 : (C.B>>1);
    C.A = (C.A==255) ? 128 : (C.A>>1);

    // now we are armed and ready to render a small triangle strip
    gsreg_Begin( 13 );
    gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP, 1, 1, 0, 1, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[0], Vs[0] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( C.R, C.G, C.B, C.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ3,  SCE_GS_SET_XYZ     ( CornerX[0], CornerY[0], 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[1], Vs[1] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( C.R, C.G, C.B, C.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ3,  SCE_GS_SET_XYZ     ( CornerX[1], CornerY[1], 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[3], Vs[3] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( C.R, C.G, C.B, C.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ     ( CornerX[3], CornerY[3], 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[2], Vs[2] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( C.R, C.G, C.B, C.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ     ( CornerX[2], CornerY[2], 0 ) );
    gsreg_End();
#elif defined( TARGET_XBOX )
    draw_SpriteUV( vector3( SpriteCenter.X, SpriteCenter.Y, 0.0f ),
                   WH,
                   UV0,
                   UV1,
                   C,
                   Rotation );
#elif defined( TARGET_PC )
    draw_SpriteUV( vector3( SpriteCenter.X, SpriteCenter.Y, 0.0f ),
                   WH,
                   UV0,
                   UV1,
                   C,
                   Rotation );
#endif
}

//=========================================================================

void dlg_load_game::platform_BeginShaftRender( void )
{
#if defined( TARGET_PS2 )
    // set the level name as our texture, and the screen as our render
    // target, but mask out writing to the alpha channel
    platform_SetDstBuffer( BUFFER_SCREEN, TRUE, FALSE );
    platform_SetSrcBuffer( BUFFER_LEVEL_NAME );

    // set up a dest alpha blend and test (this will get it blending with the fog)
    gsreg_Begin( 2 );
    gsreg_Set( SCE_GS_TEST_1, SCE_GS_SET_TEST_1( 0, 0, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS ) );
    gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_DST, C_DST, 0x80 ) );
    gsreg_End();
#elif defined( TARGET_XBOX )
    // Set the level name as our texture, and the screen as our render
    // target, but mask out writing to the alpha channel
    platform_SetDstBuffer( BUFFER_SCREEN, TRUE, FALSE );
    platform_SetSrcBuffer( BUFFER_LEVEL_NAME );

    draw_Begin( DRAW_QUADS, DRAW_CULL_NONE  |
                            DRAW_2D         |
                            DRAW_NO_ZBUFFER |
                            DRAW_TEXTURED   |
                            DRAW_USE_ALPHA  |
                            DRAW_BLEND_ADD  |
                            DRAW_U_CLAMP    |
                            DRAW_V_CLAMP    |
                            DRAW_XBOX_NO_BEGIN );

    g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );
    g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
    g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_DESTALPHA );
    g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE, m_ColorWriteMask );
    
    g_TextureStageState.Set( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE  );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE  );
    g_TextureStageState.Set( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
    g_TextureStageState.Set( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

    draw_Begin( DRAW_QUADS, DRAW_KEEP_STATES );
#elif defined( TARGET_PC )
    // Set the level name as our texture, and the screen as our render
    // target, but mask out writing to the alpha channel
    platform_SetDstBuffer( BUFFER_SCREEN, TRUE, FALSE );
    platform_SetSrcBuffer( BUFFER_LEVEL_NAME );
    
    draw_Begin( DRAW_QUADS, DRAW_CULL_NONE  |
                            DRAW_2D         |
                            DRAW_NO_ZBUFFER |
                            DRAW_TEXTURED   |
                            DRAW_USE_ALPHA  |
                            DRAW_BLEND_ADD  |
                            DRAW_U_CLAMP    |
                            DRAW_V_CLAMP    );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );                                  
    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, m_ColorWriteMask );
    
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
#endif
}

//=========================================================================

void dlg_load_game::platform_EndShaftRender( void )
{
#if defined( TARGET_PS2 )
    // Nothing to do...
#elif defined( TARGET_XBOX )
    draw_End();
#elif defined( TARGET_PC )
    draw_End();
#endif
}

//=========================================================================

void dlg_load_game::platform_DrawShaftQuad( const vector2* pCorners,
                                            const vector2* pUVs,
                                            const xcolor*  pColors )
{
#if defined( TARGET_PS2 )
    s32 i;

    // convert the data to ps2-friendly format
    s32    CornerX[4];
    s32    CornerY[4];
    u32    Us[4];
    u32    Vs[4];
    xcolor PS2Colors[4];
    for( i = 0; i < 4; i++ )
    {
        CornerX[i] = (s32)(pCorners[i].X*16.0f) + (PS2_SCISSOR_X<<4);
        CornerY[i] = (s32)(pCorners[i].Y*16.0f) + (PS2_SCISSOR_Y<<4);
        Us[i]      = reinterpret_cast<const u32&>(pUVs[i].X);
        Vs[i]      = reinterpret_cast<const u32&>(pUVs[i].Y);
        PS2Colors[i].Set( (pColors[i].R==255) ? 128 : (pColors[i].R>>1),
                          (pColors[i].G==255) ? 128 : (pColors[i].G>>1),
                          (pColors[i].B==255) ? 128 : (pColors[i].B>>1),
                          (pColors[i].A==255) ? 128 : (pColors[i].A>>1) );
    }

    // now we are armed and ready to render a small triangle strip
    gsreg_Begin( 13 );
    gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP, 1, 1, 0, 1, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[0], Vs[0] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( PS2Colors[0].R, PS2Colors[0].G, PS2Colors[0].B, PS2Colors[0].A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ3,  SCE_GS_SET_XYZ     ( CornerX[0], CornerY[0], 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[1], Vs[1] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( PS2Colors[1].R, PS2Colors[1].G, PS2Colors[1].B, PS2Colors[1].A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ3,  SCE_GS_SET_XYZ     ( CornerX[1], CornerY[1], 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[3], Vs[3] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( PS2Colors[3].R, PS2Colors[3].G, PS2Colors[3].B, PS2Colors[3].A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ     ( CornerX[3], CornerY[3], 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST      ( Us[2], Vs[2] ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ   ( PS2Colors[2].R, PS2Colors[2].G, PS2Colors[2].B, PS2Colors[2].A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ     ( CornerX[2], CornerY[2], 0 ) );
    gsreg_End();
#elif defined( TARGET_XBOX )
    draw_Color( pColors[0] );  draw_UV( pUVs[0] );  draw_Vertex( pCorners[0].X,  pCorners[0].Y, 0.0f );
    draw_Color( pColors[1] );  draw_UV( pUVs[1] );  draw_Vertex( pCorners[1].X,  pCorners[1].Y, 0.0f );
    draw_Color( pColors[2] );  draw_UV( pUVs[2] );  draw_Vertex( pCorners[2].X,  pCorners[2].Y, 0.0f );
    draw_Color( pColors[3] );  draw_UV( pUVs[3] );  draw_Vertex( pCorners[3].X,  pCorners[3].Y, 0.0f );
#elif defined( TARGET_PC )
    draw_Color( pColors[0] );  draw_UV( pUVs[0] );  draw_Vertex( pCorners[0].X,  pCorners[0].Y, 0.0f );
    draw_Color( pColors[1] );  draw_UV( pUVs[1] );  draw_Vertex( pCorners[1].X,  pCorners[1].Y, 0.0f );
    draw_Color( pColors[2] );  draw_UV( pUVs[2] );  draw_Vertex( pCorners[2].X,  pCorners[2].Y, 0.0f );
    draw_Color( pColors[3] );  draw_UV( pUVs[3] );  draw_Vertex( pCorners[3].X,  pCorners[3].Y, 0.0f );
#endif
}

//=========================================================================