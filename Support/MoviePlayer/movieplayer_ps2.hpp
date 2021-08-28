#ifndef MOVIEPLAYER_PS2_HPP
#define MOVIEPLAYER_PS2_HPP

#include "x_target.hpp"

#if !defined(TARGET_PS2)
#error This file should only be included for PlayStation 2. Please check your exclusions on your project spec.
#endif

#include "x_files.hpp"
#include "x_bitmap.hpp"
#include "x_threads.hpp"

#include <libmpeg.h>

struct mpeg_av_stream
{
    xmesgq*       pqReady;
    xmesgq*       pqAvail;
    s32           Index;
    s32           Remain;
    u8            *pBuffer;
};

#define MOVIE_BLOCK_SIZE (256*1024)
class movie_private
{
public:
                    movie_private   (void);
    void            Init            (void);
    void            Kill            (void);
    xbool           Open            (const char* pFilename,xbool PlayResident, xbool IsLooped);
    void            Close           (void);
    s32             GetWidth        (void)      { return m_Width;       }
    s32             GetHeight       (void)      { return m_Height;      }
    xbool           IsRunning       (void)      { return m_IsRunning;   }
    xbool           IsFinished      (void)      { return m_IsFinished;  }
    s32             GetCurrentFrame (void)      { return m_CurrentFrame;}
    s32             GetFrameCount   (void)      { return m_FrameCount;  }
    s32             GetBitmapCount  (void)      { return m_nBitmaps;    }
    void            ReleaseBitmap   (xbitmap* pBitmap);
    xbool           Advance         (void);
    xbitmap*        Decode          (void);
    void            SetLanguage     (s32 Language) { m_Language = Language;}
    void            SetVolume       (f32 Volume);
    void            Pause           (void);
    void            Resume          (void);
    xbool           CachingComplete ( void );

private:
    void            InitStream      ( mpeg_av_stream& Stream, s32 buffcount );
    void            UpdateStream    (mpeg_av_stream &Stream,u8 *pData,s32 count);
    void            PeriodicUpdate  ( void );
    void            KillStream      ( mpeg_av_stream &Stream );
    void            PurgeStream     ( mpeg_av_stream &Stream );
    void            Restart         ( void );
    void            Cleanup         ( void );
    void            UpdateReadahead ( void );
    X_FILE*         m_Handle;
    s32             m_Length;
    xbool           m_IsRunning;            // Player background thread is running
    xbool           m_IsResident;           // Movie resides in memory
    xbool           m_IsLooped;
    f32             m_Volume;

    xbitmap*        m_pBitmaps[2];
    xmesgq*         m_pqFrameAvail;          // A bitmap has been decoded, rendered and ready for decoding again

    xbool           m_ShutdownStreamer;     // Streamer needs to shut itself down
    xbool           m_IsFinished;           // Movie is finished
    s32             m_Width;                // Width in pixels
    s32             m_Height;               // Height in pixels
    s32             m_CurrentFrame;         // Current frame #
    s32             m_FrameCount;           // Total frames
    xtimer          m_FrameTimer;           // Time to next frame

    byte*           m_pFileBuffer;
    s32             m_FileIndex;

    xthread*        m_pStreamerThread;
    s32             m_nBitmaps;
    xarray<byte*>   m_ResidentStreamBlocks;
    s32             m_ResidentStreamAvailable;
    s32             m_ResidentStreamReadBlock;
    xbool           m_AudioFirst[2];
    mpeg_av_stream  m_VideoStream;
    mpeg_av_stream  m_AudioStream[2];
    s32             m_AudioPlayBlock;
    s32             m_AudioReadBlock;
    xmesgq          m_AudioLeftReady;
    xmesgq          m_AudioRightReady;
    xmesgq*         m_pqStreamBuffers;       // Message queue holding the streaming buffers from cd
    void            *m_pReadBuffer;         // Ptr to block of memory to be used for read requests
    sceMpeg         m_mpeg;
    void*           m_CurrentBlock;
    void*           m_pDecodeBuffer;
    byte*           m_pWorkspace;
    byte*           m_pWorkspaceAligned;
    byte*           m_AudioData[2];
    s32             m_AudioChannel[2];
    s32             m_ReadBlock;
    s32             m_LastPlayingBlock;
    s32             m_Language;
    xbool           m_AudioDataAvailable;
    xbool           m_CanStartAudio;
    xbool           m_AudioIsRunning;
    xbool           m_AudioDataPresent;
    xbool           m_FirstBlockDecoded;
    io_request      m_ReadRequest;

    friend void     mpeg_Streamer           (s32, char**);
//    friend void     mpeg_Streamer           (void* pPrivate);
    friend s32      mpeg_Callback           (sceMpeg *,sceMpegCbData *cbdata,void *pData);
    friend s32      mpeg_Videostream        (sceMpeg *,sceMpegCbData *cbdata,void *pData);
    friend s32      mpeg_Audiostream        (sceMpegCbData *cbdata,void *pData,s32 channel);
};

#endif // MOVIEPLAYER_PS2_HPP
