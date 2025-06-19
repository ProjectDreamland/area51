//-----------------------------------------------------------------------------
// This file contains the generic code for the movie player regardless of what
// backend is present for it.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include "Entropy.hpp"
#include "movieplayer.hpp"
#include "StateMgr\StateMgr.hpp"
#include "inputmgr\\inputmgr.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

//-----------------------------------------------------------------------------
movie_player::movie_player(void)
{
}

//-----------------------------------------------------------------------------
movie_player::~movie_player(void)
{
}

//-----------------------------------------------------------------------------
void movie_player::Init(void)
{
    // TODO: CTetrick - I'll clean this up... 
    // in a hurry now - I think PS2 needs the lang set prior to init.
    // will verify and clean up ASAP.    
    m_Private.Init();
    SetLanguage( x_GetLocale() );
}

//-----------------------------------------------------------------------------
xbool movie_player::Open(const char* pFilename, xbool IsResident, xbool IsLooped)
{
#ifdef TARGET_XBOX
IsResident = false;
#endif
    s32 Priority;
    // NOTE: The second parameter is whether or not the movie is loaded in to memory.
    // not used right now but will be when we have front-end load hiding.
    if ( !m_Private.Open(pFilename, IsResident, IsLooped) )
    {
        Kill();
        m_Finished = TRUE;
        return FALSE;
    }

    m_IsLooped  = IsLooped;
    m_Shutdown  = FALSE;
    m_Finished  = FALSE;
    m_pBitmaps  = NULL;

    // If we're using a modal version of the movie playback, we make the priority of the main thread
    // higher than the decoder so that it gets a chance to render the resulting bitmap within sufficient time
    // without being locked out due to the decode thread using all CPU time.

    Priority = 0;
    
    return TRUE;
}

//-----------------------------------------------------------------------------
void movie_player::Close(void)
{
    if (!m_Private.IsRunning())
    {
        return;
    }
    
    ASSERT(!m_Shutdown);

    if (m_pBitmaps)
    {
        m_Private.ReleaseBitmap(m_pBitmaps);
        m_pBitmaps=NULL;
    }

    m_Private.Close();
}

//-----------------------------------------------------------------------------
void movie_player::Kill(void)
{
    m_Private.Kill();
}

//------------------------------------------------------------------------------
// Note: This routine is used for PS2 and Bink playback. It assumes the controlling
// 'private' subsystem creates all bitmaps required. The PS2 requires the bitmaps to
// be in a strip format due to crappy hardware (no surprise there) and the gcn needs
// them to be powers of 2 in size.
//
// Note: The Xbox doesn't have either of these limitations so (hacky hacky) I decode
// directly to the back buffer.
//
void movie_player::Render(const vector2& Pos, const vector2& Size, xbool InRenderLoop)
{
#ifdef TARGET_XBOX
    if (!InRenderLoop)
    {
        if (eng_Begin("Movie"))
        {
            m_Private.SetPos((vector3&)Pos);
            m_Private.Decode();
            eng_End();
        }
    }
    else
    {
        m_Private.SetPos((vector3&)Pos);
        m_Private.Decode();
    }
#endif
#ifdef TARGET_PC
    m_Private.SetRenderSize(Size);
    m_Private.SetRenderPos(Pos);
    
    if (!InRenderLoop)
    {
        if (eng_Begin("Movie"))
        {
            m_Private.Decode();
            eng_End();
        }
    }
    else
    {
        m_Private.Decode();
    }
#endif
}
//------------------------------------------------------------------------------
void movie_player::SetLanguage(const s32 Language)
{
    m_Private.SetLanguage(Language);
}

//=========================================================================
s32 PlaySimpleMovie(const char* movieName)
{
    //view    View;
    s32     width;
    s32     height;
    vector2 Pos;
    vector2 Size;
    xbool   ret     = FALSE;
    
    Movie.Init();
    global_settings& Settings = g_StateMgr.GetActiveSettings();
    Movie.SetVolume( Settings.GetVolume( VOLUME_SPEECH ) / 100.0f );
 
    ret = Movie.Open(movieName, FALSE, FALSE);

    if( ret )
    {
        //View.SetXFOV( R_60 );
        //View.SetPosition( vector3(0,0,200) );
        //View.LookAtPoint( vector3(  0,  0,  0) );
        //View.SetZLimits ( 0.1f, 1000.0f );
        //eng_MaximizeViewport( View );
        //eng_SetView         ( View ) ;
        eng_GetRes(width,height);
        
        // Set size and position for fullscreen
        Size.X = (f32)width;
        Size.Y = (f32)height;
        Pos.X = 0.0f;
        Pos.Y = 0.0f;    

        xbool done = FALSE;

        while(!done)
        {
            if (ret)
            {
                if (!Movie.IsPlaying() )
                    break;
                g_InputMgr.Update  ( 1.0f / 60.0f );
                g_NetworkMgr.Update( 1.0f / 60.0f );
                
                Movie.Render(Pos,Size);
                eng_PageFlip();

#if defined(TARGET_XBOX)
                if( input_WasPressed( INPUT_XBOX_BTN_START, -1 ) || input_WasPressed( INPUT_XBOX_BTN_A, -1 ) ) 
#elif defined(TARGET_PS2)
                if( input_WasPressed( INPUT_PS2_BTN_CROSS, 0 ) || input_WasPressed( INPUT_PS2_BTN_CROSS, 1 ) ) 
#elif defined(TARGET_PC)
                if( input_WasPressed( INPUT_KBD_RETURN, 0 ) || input_WasPressed( INPUT_KBD_ESCAPE, 0 ) || input_WasPressed( INPUT_KBD_SPACE, 0 ) ) 
#endif
                done = TRUE;
            }
        }
        eng_PageFlip();
        Movie.Close();
    }
    Movie.Kill();
    return( ret );
}