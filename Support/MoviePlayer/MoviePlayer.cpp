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
#ifdef TARGET_PS2
    SetLanguage( x_GetLocale() );
#endif
    m_Private.Init();
#ifdef TARGET_XBOX
    SetLanguage( x_GetLocale() );
#endif
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
#if defined(TARGET_GCN)
    Priority = -1;
#else
    Priority = 0;
#endif
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
    {
        if( !InRenderLoop )
        {
            if( eng_Begin( "Movie" ) )
            {
                m_Private.SetPos( (vector3&)Pos );
                m_Private.Decode();
                eng_End();
            }
        }
        else
        {
            m_Private.SetPos( (vector3&)Pos );
            m_Private.Decode();
        }
    }
    #elif !defined TARGET_PC
    {
        vram_Flush();

        //////////////////////////////////////////////////////////////////////

        s32         nBitmaps;
        vector2     BitmapPos;
        vector2     BitmapSize;
        vector2     BitmapLowerRight;
        vector2     BitmapTopLeft;
        s32         i;

        if (m_pBitmaps)
        {
            m_Private.ReleaseBitmap(m_pBitmaps);
        }

        m_pBitmaps = m_Private.Decode();

        // RMB - more hack!!!
        if( !m_pBitmaps )
            return;
        ASSERT(m_pBitmaps);
        nBitmaps = m_Private.GetBitmapCount();

        // PS2 and GameCube have parametric uv's
        f32 V0 = 0.0f ;
        f32 V1 = (f32)m_Private.GetHeight() / (f32)m_pBitmaps->GetHeight();

        f32 U0 = 0.0f ;
        f32 U1 = 1.0f;

        #if defined(TARGET_GCN)
        BitmapSize       = vector2((Size.X / nBitmaps) + 1,(f32)Size.Y+1);
        #else
        BitmapSize       = vector2((Size.X / nBitmaps), (f32)Size.Y);
        #endif

        BitmapTopLeft    = vector2(U0,V0);
        BitmapLowerRight = vector2(U1,V1);

        //////////////////////////////////////////////////////////////////////

        xbool RenderMovie = TRUE;
        if( !InRenderLoop )
        {
            if( !eng_Begin( "movie_player" ) )
                RenderMovie = FALSE;
        }

        if( RenderMovie )
        {
            BitmapPos = Pos;

            for (i=0;i<nBitmaps;i++)
            {
                draw_Begin(DRAW_SPRITES,DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER);
                vram_Flush();
                draw_SetTexture(m_pBitmaps[i]);
                draw_DisableBilinear();
                //x_DebugMsg("%d: Bitmap at (%2.2f,%2.2f), size (%2.2f,%2.2f)\n",i,BitmapPos.X,BitmapPos.Y,BitmapSize.X,BitmapSize.Y);

#ifdef TARGET_PS2
                draw_SpriteImmediate( BitmapPos,
                                      BitmapSize,
                                      BitmapTopLeft,
                                      BitmapLowerRight,
                                      XCOLOR_WHITE );
#else
                draw_SpriteUV(  vector3( BitmapPos.X, BitmapPos.Y, 0.0f ),
                                BitmapSize,
                                BitmapTopLeft,
                                BitmapLowerRight,
                                XCOLOR_WHITE);
#endif
                #if defined(TARGET_GCN)
                BitmapPos.X += BitmapSize.X - 1;
                #else
                BitmapPos.X += BitmapSize.X;
                #endif
                draw_End();
            }
         
            if( !InRenderLoop )
            {
                eng_End();
            }
            draw_EnableBilinear();
        }
    }
    #endif
}

//------------------------------------------------------------------------------
void movie_player::SetLanguage(const s32 Language)
{
    m_Private.SetLanguage(Language);
}


//=========================================================================
s32 PlaySimpleMovie( const char* movieName )
{
#ifndef X_EDITOR
    view    View;
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
        View.SetXFOV( R_60 );
        View.SetPosition( vector3(0,0,200) );
        View.LookAtPoint( vector3(  0,  0,  0) );
        View.SetZLimits ( 0.1f, 1000.0f );
        eng_MaximizeViewport( View );
        eng_SetView         ( View ) ;
        eng_GetRes(width,height);
#ifdef TARGET_PS2
        Size.X = (f32)width;
        Size.Y = (f32)height;
#else
        Size.X = 640.0f;
        Size.Y = 480.0f;
#endif

        Pos.X = (width-Size.X)/2.0f;
        Pos.Y = (height-Size.Y)/2.0f;

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

#ifdef TARGET_XBOX
                //if( (g_ActiveConfig.GetExitReason()!=GAME_EXIT_CONTINUE) && (g_ActiveConfig.GetExitReason()!=GAME_EXIT_GAME_COMPLETE) )
                //{
                //  done = TRUE;
                //}
                if( input_WasPressed( INPUT_XBOX_BTN_START, -1 ) || input_WasPressed( INPUT_XBOX_BTN_A, -1 ) ) 
#else
                if( input_WasPressed( INPUT_PS2_BTN_CROSS, 0 ) || input_WasPressed( INPUT_PS2_BTN_CROSS, 1 ) ) 
#endif
                    done = TRUE;

            }
        }
        eng_PageFlip();
        Movie.Close();
    }
	Movie.Kill();
    return( ret );

#else
    return( FALSE );//-- Nothing to see.
#endif
}


