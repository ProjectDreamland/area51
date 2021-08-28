#ifndef MOVIEPLAYER_BINK_HPP
#define MOVIEPLAYER_BINK_HPP

#include "x_target.hpp"

#if defined(TARGET_PS2)
#error This file should not be included for PlayStation 2. Please check your exclusions on your project spec.
#endif


//#include "x_files.hpp"
//#include "x_threads.hpp"
//#include "Entropy.hpp"
//#include "audio/audio_hardware.hpp"

#include "entropy.hpp"

//------ Gamecube definitions and includes
#if defined(TARGET_GCN)
#include "dolphin/os.h"

#define MOVIE_FIXED_WIDTH   640
#define MOVIE_STRIP_WIDTH   64
#define MOVIE_STRIP_HEIGHT  512

#define MOVIE_BITMAP_FORMAT (xbitmap::FMT_32_ARGB_8888)
#define BINK_BITMAP_FORMAT	BINKSURFACE32
#define XBITMAP_FLAGS       (xbitmap::FLAG_GCN_DATA_SWIZZLED)
#define __RADNGC__
#ifndef GEKKO
#define GEKKO
#endif

//------ XBox definitions and includes
#elif defined(TARGET_XBOX)

#define MOVIE_FIXED_WIDTH   640
#define MOVIE_STRIP_WIDTH   640
#define MOVIE_STRIP_HEIGHT  512

#define MOVIE_BITMAP_FORMAT (xbitmap::FMT_32_ARGB_8888)
#define BINK_BITMAP_FORMAT	BINKSURFACE32
#define XBITMAP_FLAGS       (0)
#define __RADXBOX__

//------ PC definitions and includes
#elif defined(TARGET_PC)

#define MOVIE_BITMAP_FORMAT (xbitmap::FMT_32_RGBA_8888)
#define BINK_BITMAP_FORMAT	BINKSURFACE32
#define XBITMAP_FLAGS       (0)

#else

#error "Need bitmap type definitions for bink playback"

#endif

#include <3rdParty\BinkXbox\Include\bink.h>

class movie_private
{
public:
                    movie_private   (void);
    void            Init            (void);
    xbool           Open            (const char* pFilename,xbool PlayResident,xbool IsLooped );
    void            Close           (void);
    void            Kill            (void);
    void            Shutdown        (void);
    s32             GetWidth        (void)      { return m_Width;           }
    s32             GetHeight       (void)      { return m_Height;          }
    xbool           IsRunning       (void)      { return m_IsRunning;       }
    xbool           IsFinished      (void)      { return m_IsFinished;      }
    s32             GetCurrentFrame (void)      { return m_CurrentFrame;    }
    s32             GetFrameCount   (void)      { return m_FrameCount;      }
    void            ReleaseBitmap   (xbitmap* pBitmap);
    s32             GetBitmapCount  (void)      { return m_nBitmaps;        }
    xbitmap*        Decode          (void);
    void            SetLanguage     (s32 Language)  { m_Language = Language;}
    void            SetVolume       (f32 Volume);
    void            Pause           (void);
    void            Resume          (void);
    xbool           CachingComplete ( void )    { return TRUE;              }

#ifdef TARGET_XBOX
    void            SetPos          ( vector3& Pos ){ m_Pos = Pos; }
#endif

private:
    xbool           m_IsRunning;            // Player background thread is running
    xbitmap*        m_pBitmaps[2];
    xmesgq*         m_pqFrameAvail;          // A bitmap has been decoded, rendered and ready for decoding again
    xbool           m_IsFinished;           // Movie is complete
    s32             m_nMaxBitmaps;          // Number of bitmaps allocated (for 640 wide)
    s32             m_nBitmaps;             // Number of bitmaps used
    s32             m_Width;                // Width in pixels
    s32             m_Height;               // Height in pixels
    s32             m_CurrentFrame;         // Current frame number
    s32             m_FrameCount;           // Total frame count
    f32             m_Volume;
    s32             m_Language;
    xbool           m_IsLooped;
    HBINK           m_Handle;

#ifdef TARGET_XBOX
    vector3         m_Pos;
#endif
};


#endif // MOVIEPLAYER_HPP
