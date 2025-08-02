#ifndef MOVIEPLAYER_BINK_HPP
#define MOVIEPLAYER_BINK_HPP

#include "x_target.hpp"

#ifdef TARGET_PS2
#error This file should not be included for PlayStation 2. Please check your exclusions on your project spec.
#endif

#include "x_files.hpp"
#include "Entropy.hpp"
#include "audio/audio_hardware.hpp"

//------ XBox definitions and includes
#ifdef TARGET_XBOX
#define MOVIE_FIXED_WIDTH   640
#define MOVIE_STRIP_WIDTH   640
#define MOVIE_STRIP_HEIGHT  512

#define MOVIE_BITMAP_FORMAT (xbitmap::FMT_32_ARGB_8888)
#define BINK_BITMAP_FORMAT    BINKSURFACE32
#define XBITMAP_FLAGS       (0)
#define __RADXBOX__

#include <3rdParty\BinkXbox\Include\bink.h>
#endif

//------ PC definitions and includes
#ifdef TARGET_PC
#define MOVIE_FIXED_WIDTH   640
#define MOVIE_STRIP_WIDTH   640
//#define MOVIE_STRIP_HEIGHT  512

#define MOVIE_BITMAP_FORMAT (xbitmap::FMT_32_URGB_8888)
#define BINK_BITMAP_FORMAT    BINKSURFACE32
#define XBITMAP_FLAGS       (0)
#define __RADWIN__

#include <3rdParty\BinkXBOX\Include\bink.h>

//Actually Bink XBOX is multi-platform, so we can use its libraries for PC, except binkw32.lib and binkw32.dll
//#include <3rdParty\BinkSDK\bink.h>
#endif

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
#ifdef TARGET_PC     
    void            SetRenderSize   (const vector2& Size) { m_RenderSize = Size;             }
    void            SetRenderPos    (const vector2& Pos)  { m_RenderPos = Pos;               }
    void            SetForceStretch (xbool bForceStretch) { m_bForceStretch = bForceStretch; }
    xbool           IsForceStretch  (void)                { return m_bForceStretch;          }
#endif

#ifdef TARGET_XBOX
    void            SetPos          ( vector3& Pos ){ m_Pos = Pos; }
#endif

private:
    xbool           m_IsRunning;            // Player background thread is running
    xbitmap*        m_pBitmaps[2];
    xmesgq*         m_pqFrameAvail;         // A bitmap has been decoded, rendered and ready for decoding again
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
#ifdef TARGET_PC 
    vector2         m_RenderSize;
    vector2         m_RenderPos;
    xbool           m_bForceStretch;
    
    IDirect3DSurface9* m_Surface;
#endif

#ifdef TARGET_XBOX
    vector3         m_Pos;
#endif
};

#endif // MOVIEPLAYER_BINK_HPP