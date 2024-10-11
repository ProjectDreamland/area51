#ifndef MOVIEPLAYER_HPP
#define MOVIEPLAYER_HPP

#include "x_target.hpp"
#include "x_types.hpp"
#include "x_math.hpp"

//
// The audio stream should use THIS order when adding the languages to the movie stream.
// This order is defined in x_locale.hpp
// XL_LANG_ENGLISH = 0,
// XL_LANG_FRENCH,
// XL_LANG_GERMAN,
// XL_LANG_ITALIAN,
// XL_LANG_SPANISH,

// XL_LANG_DUTCH,
// XL_LANG_JAPANESE,
// XL_LANG_KOREAN,
// XL_LANG_PORTUGUESE,
// XL_LANG_TCHINESE,
//

s32 PlaySimpleMovie( const char* movieName );

#ifdef TARGET_PS2
#include "movieplayer_ps2.hpp"
#else
#include "movieplayer_bink.hpp"
#endif

class movie_player
{
public:
                movie_player    (void);
               ~movie_player    (void);
    xbool       Open            (const char* pFilename, xbool PlayResident, xbool IsLooped);
    void        Close           (void);
    void        Init            (void);
    void        Kill            (void);
    void        Play            (void);
    void        Stop            (void);
    void        Render          (const vector2& Pos, const vector2& Size, xbool InRenderLoop = FALSE);
    void        SetVolume       (f32 Volume)                        { m_Private.SetVolume(Volume);          };
    void        SetLanguage     (const s32 Language);
    void        Pause           (void)                              { m_Private.Pause();                    };
    void        Resume          (void)                              { m_Private.Resume();                   };

    xbool       IsPlaying       (void)                              { return !m_Private.IsFinished();       };
    s32         GetWidth        (void)                              { return m_Private.GetWidth();          };
    s32         GetHeight       (void)                              { return m_Private.GetHeight();         };
    xbool       CachingComplete ( void )                            { return m_Private.CachingComplete();   };

private:
    movie_private   m_Private;
    xbool           m_Shutdown;
    byte*           m_pMovieBuffer;
    xbool           m_IsLooped;
    xbool           m_Finished;
    xbool           m_FramePending;
    xbitmap*        m_pBitmaps;
    friend  void    s_MoviePlayerThread(void*);
};

#ifdef TARGET_PC
extern movie_player Movie;
#endif

xbool       movie_Play          (const char* pFilename,const xbool IsLooped=FALSE,const xbool Modal=TRUE);
void        movie_Stop          (void);
void        movie_Pause         (void);
xbitmap*    movie_GetBitmap     (void);
xbool       movie_IsPlaying     (void);
xbool       movie_LockBitmap    (void);
void        movie_UnlockBitmap  (void);

#endif // MOVIEPLAYER_HPP
