#include "x_target.hpp"

#if !defined(bwatson)
#define X_SUPPRESS_LOGS
#endif

#if !defined(TARGET_PS2)
#error This file should only be compiled for PlayStation 2. Please check your exclusions on your project spec.
#endif

#include "entropy.hpp"
#include "Entropy/PS2/ps2_spad.hpp"
#include "Entropy/PS2/ps2_misc.hpp"
#include "x_threads.hpp"
#include "movieplayer/movieplayer.hpp"
#include "resourcemgr/resourcemgr.hpp"
#include "audio/hardware/audio_hardware_ps2_private.hpp"
#include "audio/audio_hardware.hpp"
#include "IOManager/io_mgr.hpp"

#define AVG_FRAMES      20
#ifdef X_DEBUG
#define SHOW_PLAYTIMES
#endif

movie_player Movie;

#define MPEG_BITMAP_HEIGHT      448
#define MPEG_BITMAP_WIDTH       1024
#define MPEG_FILE_BUFFER_SIZE   131072
#define MPEG_BUFFER_BLOCK_SIZE  2048
#define MPEG_AUDIO_BUFFER_SIZE  32768
#define MPEG_NUM_DISPLAY_BUFFERS 2
// The video and audio buffer counts should be sized so that 1 seconds worth of audio
// and video data is in memory at a time. 384KB/sec video, 22Khz adpcm stereo audio requires
// 384KBytes/sec (192 buffers) for video and 25KB/sec (12 buffers). It's all rounded up
// and over-generous to try and prevent glitching problems due to seek timing etc.
#define MPEG_VIDEO_BUFFER_COUNT 300
#define MPEG_AUDIO_BUFFER_COUNT 50

        s32     mpeg_Callback           (sceMpeg *mp,sceMpegCbData *cbdata,void *pData);
        s32     mpeg_Videostream        (sceMpeg *,sceMpegCbData *cbdata,void *pData);
static  s32     mpeg_AudiostreamLeft    (sceMpeg *,sceMpegCbData *cbdata,void *pData);
static  s32     mpeg_AudiostreamRight   (sceMpeg *,sceMpegCbData *cbdata,void *pData);
        void    mpeg_Streamer           (s32 argc, char** argv);

static sceIpuDmaEnv s_Env;

//------------------------------------------------------------------------------
static s32 s_AllocateAudioChannel(void)
{
    s32 i;
    s32 id;

    id = -1;
    g_AudioHardware.Lock();
    for( i=0; i<g_AudioHardware.NumChannels(); i++ )
    {
        if( s_ChannelManager.GetState(i)==CHANSTAT_IDLE )
        {
            s_ChannelManager.SetState( i, CHANSTAT_STOPPED );
            s_ChannelManager.SetPitch( i, 1.0f );
            s_ChannelManager.SetOwner( i, 0 );
            id = i;
            break;
        }
    }
    LOG_MESSAGE("s_AllocateAudioChannel","Allocated channel %d",id);
    g_AudioHardware.Unlock();
    return id;
}

//------------------------------------------------------------------------------
static void s_FreeAudioChannel(s32 channel)
{
    g_AudioHardware.Lock();
    s_ChannelManager.SetLoopEnd( channel, NULL );
    // If we didn't actually have any audio streams associated with this movie,
    // then it'll never get set to cooling state and it will always be stopped
    // so we then just release it.
    if( s_ChannelManager.GetState(channel)==CHANSTAT_STOPPED )
    {
        s_ChannelManager.SetState( channel, CHANSTAT_IDLE );
    }
    else
    {
        s_ChannelManager.SetState( channel, CHANSTAT_COOLING );
    }
    g_AudioHardware.Unlock();
}

//------------------------------------------------------------------------------
movie_private::movie_private(void) : m_AudioLeftReady(MAX_DEFAULT_MESSAGES), m_AudioRightReady(MAX_DEFAULT_MESSAGES)
{
}

//-----------------------------------------------------------------------------
void movie_private::Init(void)
{
    m_Volume = 1.0f;
}

//-----------------------------------------------------------------------------
void movie_private::Kill(void)
{
}

//-----------------------------------------------------------------------------
xbool movie_private::Open( const char *pFilename, xbool PlayResident, xbool IsLooped )
{
    s32     bufflength;
    s32     i;
    xtimer  t;

    ASSERT( !m_IsRunning );

    x_MemSanity();

    t.Start();
    m_Handle = x_fopen(xfs("%s\\%s.pss", g_RscMgr.GetRootDirectory(), pFilename),"rb");
    if (!m_Handle)
        return FALSE;
    g_IOFSMgr.EnableChecksum( (io_open_file*)m_Handle, FALSE );

    m_Width             = 640;
    m_Height            = 448;
    m_AudioFirst[0]     = TRUE;
    m_AudioFirst[1]     = TRUE;
    m_FileIndex         = 0;
    m_CurrentFrame      = 0;
    m_IsFinished        = FALSE;
    m_pqFrameAvail      = new xmesgq(2);
    m_AudioDataPresent  = TRUE;             // Assume audio data is available
    m_FirstBlockDecoded = FALSE;

    ASSERT(m_Width <= MPEG_BITMAP_WIDTH);
    ASSERT(m_Height <= MPEG_BITMAP_HEIGHT);

    bufflength = SCE_MPEG_BUFFER_SIZE(m_Width,m_Height) + 64*1024;

    m_pDecodeBuffer = x_malloc(bufflength);
    ASSERT(m_pDecodeBuffer);

    m_pWorkspace = (byte*)x_malloc(MPEG_NUM_DISPLAY_BUFFERS*sizeof(sceIpuRGB32)*(m_Width / 16)*(m_Height/16) + 64);

    ASSERT(m_pWorkspace);

    m_pWorkspaceAligned = (byte*) ((s32)(m_pWorkspace+63) & ~63);

    LOG_MESSAGE("movie_private::Open", "MovieInit Tag 3:%2.02fms", t.ReadMs() );

    m_pReadBuffer = x_malloc(MPEG_BUFFER_BLOCK_SIZE * (MPEG_VIDEO_BUFFER_COUNT + MPEG_AUDIO_BUFFER_COUNT * 2) + 64);
    ASSERT(m_pReadBuffer);

    m_pFileBuffer = (u8 *)x_malloc(MPEG_FILE_BUFFER_SIZE);
    ASSERT(m_pFileBuffer);

    LOG_MESSAGE("movie_private::Open", "MovieInit Tag 1:%2.02fms", t.ReadMs() );
    m_IsResident = PlayResident;
    m_IsLooped   = IsLooped;

    x_fseek(m_Handle,0,X_SEEK_END);
    m_Length = x_ftell(m_Handle);
    x_fseek(m_Handle,0,X_SEEK_SET);

    m_ResidentStreamBlocks.Clear();
    if( m_IsResident )
    {
        s32 nBlocks = (m_Length+MOVIE_BLOCK_SIZE-1)/MOVIE_BLOCK_SIZE;
        s32 LengthRemaining;
        m_ResidentStreamBlocks.SetCapacity( nBlocks );
        LengthRemaining = m_Length;
        s32 i;
        i=0;
        while( LengthRemaining )
        {
            s32 BlockSize = MIN( LengthRemaining, MOVIE_BLOCK_SIZE );
            m_ResidentStreamBlocks.Append( (byte*)x_malloc( BlockSize ) );
            ASSERT( m_ResidentStreamBlocks[i] );
            LengthRemaining -= BlockSize;
            i++;
        }
        m_ResidentStreamAvailable = 0;
        m_ResidentStreamReadBlock = 0;
        // Start new transfer request.

        io_open_file* pFile = (io_open_file*)m_Handle;
        s32 BlockSize = MIN( m_Length, MOVIE_BLOCK_SIZE );
        LOG_MESSAGE( "movie_private::Open", "Initial read queued request. Length:0x%08x", BlockSize );
        m_ReadRequest.SetRequest( pFile, m_ResidentStreamBlocks[0], pFile->Offset, BlockSize, io_request::LOW_PRIORITY, FALSE, 0, 0, io_request::READ_OP ); 
        g_IoMgr.QueueRequest( &m_ReadRequest );

    }
    LOG_MESSAGE("movie_private::Open", "MovieInit Tag 2:%2.02fms", t.ReadMs() );


    Restart();


    m_FrameCount = 1024;
    m_IsRunning = TRUE;
    //
    // Dummy decode so we can get the width/height of the mpeg stream
    //
    LOG_MESSAGE("movie_private::Open", "MovieInit Tag 9:%2.02fms", t.ReadMs() );

    // Let everyone know that scratchpad is in use
    DLIST.LockScratchpad();
    SPAD.Lock();

    sceMpegGetPicture(  &m_mpeg,
                        (sceIpuRGB32 *)m_pWorkspaceAligned,
                        (m_Width / 16) * (m_Height / 16));

    // we don't need scratchpad anymore
    SPAD.Unlock();
    DLIST.UnlockScratchpad();

    m_Width  = m_mpeg.width;
    m_Height = m_mpeg.height;

    u8* pBuffer;
    s32 j;

    pBuffer = (u8 *)m_pWorkspaceAligned;
    m_nBitmaps = m_Width / 16;

    for( j=0; j<MPEG_NUM_DISPLAY_BUFFERS; j++ )
    {
        m_pBitmaps[j] = new xbitmap[m_nBitmaps];

        for( i=0; i<m_nBitmaps; i++ )
        {
            m_pBitmaps[j][i].Setup(xbitmap::FMT_32_ABGR_8888,16,512/*1024*/,FALSE,pBuffer);
            vram_Register(m_pBitmaps[j][i]);
            pBuffer += (16*sizeof(u32)*m_Height);
        }

    }

    LOG_MESSAGE("movie_private::Open", "MovieInit Tag 10:%2.02fms", t.ReadMs() );
    // This is slightly different from the bink decompressor. We decoded the first
    // frame to get the proper bitmap width & height, so that is marked as already
    // decoded, so we put it in the ready block
    m_pqFrameAvail->Send(m_pBitmaps[0],MQ_BLOCK);
    m_pqFrameAvail->Send(m_pBitmaps[1],MQ_BLOCK);

    while( (m_AudioStream[0].pqReady->ValidEntries() < 2) && m_AudioDataPresent )
    {
        x_DelayThread( 32 );
    }

    LOG_MESSAGE("movie_private::Open", "MovieInit Tag 11:%2.02fms", t.ReadMs() );
    if( m_AudioDataPresent == FALSE )
    {
        // If we don't have any audio present, then we get rid of the audio channel allocations.
        // The reason we do this here instead of during the close process is that we reset the IOP
        // during the going online process and we cannot have any audio resources allocated when
        // we do this. The movie played while going online will NEVER have audio data associated
        // with it.
        LOG_MESSAGE( "movie_private::Open", "Removed audio streams as there is no audio data." );
        for( i=0; i<2; i++ )
        {
            s_FreeAudioChannel(m_AudioChannel[i]);
            g_AudioHardware.FreeAudioRam(m_AudioData[i]);
        }
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
xbool movie_private::CachingComplete( void )
{
    return ( m_Handle == NULL );
}

//-----------------------------------------------------------------------------
void movie_private::Restart( void )
{
    s32 bufflength;
    s32 i;
    u8* pBuff;

    ASSERT( m_pFileBuffer );

    m_AudioFirst[0]     = TRUE;
    m_AudioFirst[1]     = TRUE;
    pBuff = (u8 *)(((u32)m_pReadBuffer+63) &~ 63);

    InitStream(m_VideoStream,MPEG_VIDEO_BUFFER_COUNT);
    InitStream(m_AudioStream[0],MPEG_AUDIO_BUFFER_COUNT);
    InitStream(m_AudioStream[1],MPEG_AUDIO_BUFFER_COUNT);
    for( i=0; i<MPEG_VIDEO_BUFFER_COUNT; i++ )
    {
        m_VideoStream.pqAvail->Send( pBuff, MQ_BLOCK );
        pBuff += MPEG_BUFFER_BLOCK_SIZE;
    }
    for( i=0; i<MPEG_AUDIO_BUFFER_COUNT; i++ )
    {
        m_AudioStream[0].pqAvail->Send(pBuff,MQ_BLOCK);
        pBuff += MPEG_BUFFER_BLOCK_SIZE;
        m_AudioStream[1].pqAvail->Send(pBuff,MQ_BLOCK);
        pBuff += MPEG_BUFFER_BLOCK_SIZE;
    }

    ASSERT(MPEG_FILE_BUFFER_SIZE >= MPEG_AUDIO_BUFFER_SIZE);
    x_memset(m_pFileBuffer,0x02,MPEG_AUDIO_BUFFER_SIZE);
    // When we zero the buffer, we also have to make sure that the loop point flag
    // is set at the end of the buffer
    for( i=0; i<MPEG_AUDIO_BUFFER_SIZE; i+=MPEG_BUFFER_BLOCK_SIZE )
    {
        m_pFileBuffer[i+MPEG_BUFFER_BLOCK_SIZE-16+1]=0x03;
    }

    FlushCache(0);
    // This is pretty hacky. The AudioDataPresent flag will always be set the very first time in to this Restart routine.
    // Then, once the first block of data is decoded, we will know if we have audio data present at all. Then when the
    // movie restarts itself on a loop, it can determine if it should, indeed, allocate audio again. This is all part
    // of the hack that is done to allow movies to be played while IOP is rebooting.
    if( m_AudioDataPresent )
    {
        for( i=0; i<2; i++ )
        {
            // We are faking an audio channel so we can get a properly allocated
            // channel from it. It's a hack but we have to do it so that the
            // base audio system does not attempt to allocate it again.

            m_AudioChannel[i] = s_AllocateAudioChannel();
            ASSERTS(m_AudioChannel[i]>=0, "Unable to allocate a hardware audio channel");

            m_AudioData[i] = (byte*)g_AudioHardware.AllocAudioRam(MPEG_AUDIO_BUFFER_SIZE);
            // Zero the buffer
            xmesgq msgq(1);

            s_ChannelManager.PushData(m_pFileBuffer,m_AudioData[i],MPEG_AUDIO_BUFFER_SIZE,&msgq);
            msgq.Recv(MQ_BLOCK);

            // Initialize the channel
            if( i==0 )
            {
                s_ChannelManager.SetVolume( m_AudioChannel[i], m_Volume, 0.0f );
            }
            else
            {
                s_ChannelManager.SetVolume( m_AudioChannel[i], 0.0f, m_Volume );
            }
            s_ChannelManager.SetPitch     ( m_AudioChannel[i], 1.0f );
            s_ChannelManager.SetSampleRate( m_AudioChannel[i], 22050 );
            s_ChannelManager.SetLength    ( m_AudioChannel[i], MPEG_AUDIO_BUFFER_SIZE );
            s_ChannelManager.SetSample    ( m_AudioChannel[i], m_AudioData[i] );
            s_ChannelManager.SetLoopStart ( m_AudioChannel[i], m_AudioData[i] );
            s_ChannelManager.SetLoopEnd   ( m_AudioChannel[i], m_AudioData[i]+MPEG_AUDIO_BUFFER_SIZE );

        }
    }

    bufflength = SCE_MPEG_BUFFER_SIZE( m_Width, m_Height ) + 64*1024 - 64;

    m_FileIndex = 0;
    m_ReadBlock = 0;
    m_CurrentBlock = NULL;

    sceMpegInit( );
    sceMpegCreate( &m_mpeg, (u8*)(((s32)m_pDecodeBuffer+63)&~63), bufflength);

    sceMpegAddCallback( &m_mpeg, sceMpegCbNodata,     mpeg_Callback, this );
    sceMpegAddCallback( &m_mpeg, sceMpegCbStopDMA,    mpeg_Callback, this );
    sceMpegAddCallback( &m_mpeg, sceMpegCbRestartDMA, mpeg_Callback, this );
    // Now add the decode callback for the PSS streamed data
    sceMpegAddStrCallback( &m_mpeg, sceMpegStrM2V,   0,             mpeg_Videostream,     this );
    sceMpegAddStrCallback( &m_mpeg, sceMpegStrADPCM, m_Language*2,  mpeg_AudiostreamLeft, this );
    sceMpegAddStrCallback( &m_mpeg, sceMpegStrADPCM, m_Language*2+1,mpeg_AudiostreamRight,this );

    //
    // Decode initial file header so we can get the file length info
    //
    m_ShutdownStreamer=FALSE;

    m_pStreamerThread = new xthread(mpeg_Streamer,"Data decode streamer",32768,-1,1,(char**)this); 
}

//-----------------------------------------------------------------------------
void movie_private::Cleanup( void )
{
    s32 i;
    s32 ShutdownDelay;

    if( m_AudioDataPresent )
    {
        for( i=0; i<2; i++ )
        {
            s_FreeAudioChannel(m_AudioChannel[i]);
            g_AudioHardware.FreeAudioRam(m_AudioData[i]);
        }
    }

    if( IsFinished() == FALSE )
    {
        LOG_MESSAGE("movie_private::Cleanup","Movie is still running. Shutting down gracefully.");
        m_ShutdownStreamer = TRUE;
        if( m_ReadRequest.GetStatus() != io_request::COMPLETED )
        {
            LOG_WARNING( "movie_private::Cleanup", "Movie close delayed due to stream request pending." );
            LOG_FLUSH();
            while( m_ReadRequest.GetStatus() != io_request::COMPLETED )
            {
                x_DelayThread(32);
            }
            LOG_WARNING( "movie_private::Cleanup", "Streaming complete." );
            LOG_FLUSH();
        }

        for( ShutdownDelay=0; ShutdownDelay<60; ShutdownDelay++ )
        {
            if( (m_ShutdownStreamer==FALSE) || (m_pStreamerThread->IsActive()==FALSE) )
                break;
            PurgeStream(m_VideoStream);
            PurgeStream(m_AudioStream[0]);
            PurgeStream(m_AudioStream[1]);
            x_DelayThread(10);
        }
        LOG_MESSAGE( "movie_private::Close","Shutdown complete. Delay=%d, Flag=%d",ShutdownDelay, m_ShutdownStreamer );
        // Push request may be in progress so we have to wait until the avail
        // queues are full.

    }

    for( ShutdownDelay=0; ShutdownDelay<60; ShutdownDelay++ )
    {
        if (m_AudioStream[0].pqAvail->IsFull() &&
            m_AudioStream[1].pqAvail->IsFull() )
        {
            break;
        }
        x_DelayThread(1);
        if( (m_AudioLeftReady.IsEmpty()==FALSE) && (m_AudioRightReady.IsEmpty()==FALSE) )
        {
            m_AudioStream[0].pqAvail->Send( m_AudioLeftReady.Recv( MQ_BLOCK ), MQ_BLOCK );
            m_AudioStream[1].pqAvail->Send( m_AudioRightReady.Recv( MQ_BLOCK ), MQ_BLOCK );
        }
    }
    LOG_MESSAGE( "movie_private::Close","Shutdown audio streams. Delay:%d",ShutdownDelay );

    delete m_pStreamerThread;
    m_pStreamerThread = NULL;

    *D4_CHCR = 0x0;
    sceMpegReset( &m_mpeg );
    sceMpegDelete( &m_mpeg );

    KillStream(m_VideoStream);
    KillStream(m_AudioStream[0]);
    KillStream(m_AudioStream[1]);
}

//-----------------------------------------------------------------------------
void movie_private::Close( void )
{
    s32 i;

    ASSERT( m_IsRunning );
    ASSERT( m_pWorkspace );
    ASSERT( m_pDecodeBuffer );

    x_MemSanity();

    // We need to try and shut down the streamer thread gracefully since it
    // may be in the middle of read requests when the movie is to quit. This
    // would be bad. We consume the target message queue entries so we can
    // make sure it will shut down (since it may be blocked on one of the
    // queues being full)
//    audio_KillStream();

    Cleanup();

    delete m_pqFrameAvail;

    if( m_IsResident )
    {
        for( i=0; i<m_ResidentStreamBlocks.GetCount(); i++ )
        {
            x_free( m_ResidentStreamBlocks[i] );
        }
        m_ResidentStreamBlocks.Clear();
    }

    if( m_Handle )
    {
        x_fclose( m_Handle );
    }

    x_free(m_pWorkspace);
    x_free(m_pDecodeBuffer);
    s32 j;


    for( j=0; j<MPEG_NUM_DISPLAY_BUFFERS; j++ )
    {
        for( i=0; i<m_nBitmaps; i++ )
        {
            vram_Unregister(m_pBitmaps[j][i]);
        }
        delete []m_pBitmaps[j];
    }
    x_free(m_pFileBuffer);

    ASSERT(m_pReadBuffer);
    x_free(m_pReadBuffer);

    m_pDecodeBuffer = NULL;
    m_pReadBuffer   = NULL;
    m_IsRunning     = FALSE;
    m_IsFinished    = TRUE;
    m_FrameTimer.Stop();
    m_FrameTimer.Reset();
}

//------------------------------------------------------------------------------
void movie_private::ReleaseBitmap( xbitmap* pBitmap )
{
    ASSERT(!m_pqFrameAvail->IsFull());
    m_pqFrameAvail->Send(pBitmap,MQ_BLOCK);
}

f32 s_DecodeTime;
byte* ChannelPos[3];

//-----------------------------------------------------------------------------
xbitmap* movie_private::Decode( void )
{
    byte*       pAudioLeft;
    byte*       pAudioRight;
    xbitmap*    pBitmap;

    ASSERT(!sceMpegIsEnd(&m_mpeg));

    pBitmap = (xbitmap*)m_pqFrameAvail->Recv(MQ_BLOCK);
    // This can only happen if the thread is being shutdown.
    if( pBitmap==NULL )
    {
        return NULL;
    }

    // Let everyone know that scratchpad is in use
    DLIST.LockScratchpad();
    SPAD.Lock();

    // To get the address of the buffer, we kinda cheat. Even though the 'bitmap' returned from
    // framepending is an array, the first entry will point to the resulting bitmap data for all
    // entries within the array.
    FlushCache(0);
    sceMpegGetPicture(  &m_mpeg,
                        (sceIpuRGB32 *)pBitmap->GetPixelData(),
                        (m_Width / 16) * (m_Height / 16));

    // we don't need scratchpad anymore
    SPAD.Unlock();
    DLIST.UnlockScratchpad();

    // Get the current play position for the audio buffer
    // If we have transitioned from one buffer to the next,
    // queue up a new block of audio data

    ChannelPos[0] = s_ChannelManager.GetPlayPosition(m_AudioChannel[0]);
    ChannelPos[1] = s_ChannelManager.GetPlayPosition(m_AudioChannel[1]);
    ChannelPos[2] = (byte*)(ChannelPos[1] - ChannelPos[0]);
    s32 PlayingBlock;

    PlayingBlock = (s_ChannelManager.GetPlayPosition(m_AudioChannel[0]) - m_AudioData[0])/MPEG_BUFFER_BLOCK_SIZE;

    if( (m_AudioLeftReady.IsEmpty()==FALSE) && (m_AudioRightReady.IsEmpty()==FALSE) )
    {
        m_AudioStream[0].pqAvail->Send( m_AudioLeftReady.Recv( MQ_BLOCK ), MQ_BLOCK );
        m_AudioStream[1].pqAvail->Send( m_AudioRightReady.Recv( MQ_BLOCK ), MQ_BLOCK );

        if( s_ChannelManager.GetState(m_AudioChannel[0]) == CHANSTAT_STOPPED )
        {
            LOG_MESSAGE("movie_private::Decode","Started audio channels");
            s_ChannelManager.SetState(m_AudioChannel[0],CHANSTAT_WARMING);
            s_ChannelManager.SetState(m_AudioChannel[1],CHANSTAT_WARMING);
        }
    }

    if( (PlayingBlock != m_ReadBlock) || (s_ChannelManager.GetState(m_AudioChannel[0]) == CHANSTAT_STOPPED) )
    {
        // Get data block for left and right channels (need both at the same time)
        pAudioLeft = (byte*)m_AudioStream[0].pqReady->Recv(MQ_NOBLOCK);
        pAudioRight = (byte*)m_AudioStream[1].pqReady->Recv(MQ_NOBLOCK);
        if( pAudioLeft && pAudioRight )
        {
            // Each 2K page block needs to have the loop endpoint flag set so that
            // it will properly loop on a 2K block transition
            pAudioLeft[2048-16+1]=0x03;
            pAudioRight[2048-16+1]=0x03;
            FlushCache(0);

            // Upload the decoded data to audio memory space
            LOG_MESSAGE("movie_private::Decode", "Pushing data to left channel, source=0x%08x, dest=0x%08x, read position=0x%08x, state=%d, read blk=%d, play blk=%d",pAudioLeft,m_AudioData[0]+m_ReadBlock*MPEG_BUFFER_BLOCK_SIZE,s_ChannelManager.GetPlayPosition(m_AudioChannel[0]),s_ChannelManager.GetState(m_AudioChannel[0]),m_ReadBlock,PlayingBlock);
            s_ChannelManager.PushData(pAudioLeft,m_AudioData[0]+m_ReadBlock*MPEG_BUFFER_BLOCK_SIZE,MPEG_BUFFER_BLOCK_SIZE, &m_AudioLeftReady );
            s_ChannelManager.PushData(pAudioRight,m_AudioData[1]+m_ReadBlock*MPEG_BUFFER_BLOCK_SIZE,MPEG_BUFFER_BLOCK_SIZE,&m_AudioRightReady );
            m_ReadBlock++;
            if( m_ReadBlock >= MPEG_AUDIO_BUFFER_SIZE / MPEG_BUFFER_BLOCK_SIZE )
            {
                m_ReadBlock = 0;
            }
        }
        else
        {
            // If we could not get BOTH left and right, then push them back to the
            // head of their respective ready queues to be used next time round.
            if( pAudioLeft )
            {
                m_AudioStream[0].pqReady->Send(pAudioLeft,MQ_JAM);
            }

            if( pAudioRight )
            {
                m_AudioStream[1].pqReady->Send(pAudioRight,MQ_JAM);
            }
        }
    }


    m_IsFinished = sceMpegIsEnd(&m_mpeg);
    if( m_IsFinished && m_IsLooped )
    {
        LOG_WARNING("movie_private::Decode","Restarting movie.");
        Cleanup();
        Restart();
        LOG_MESSAGE("movie_private::Decode","Restart complete.");
        m_IsFinished = FALSE;
    }

    m_CurrentFrame++;
    return pBitmap;

}

//-----------------------------------------------------------------------------
void movie_private::PeriodicUpdate(void)
{
    s32         length;
    xthread*    pThread;

    pThread = x_GetCurrentThread();
    ASSERT( pThread );

    if( (pThread->IsActive()==FALSE) || m_ShutdownStreamer )
    {
        m_ShutdownStreamer = FALSE;
        pThread->Kill();
        ASSERT( FALSE );
    }

    if( m_FileIndex == m_Length )
    {
        x_DelayThread(5);
        return;
    }

    length = MIN( m_Length-m_FileIndex,MPEG_FILE_BUFFER_SIZE );
    ASSERT(length > 0);

    if( m_IsResident )
    {
        m_ResidentStreamAvailable = (m_ResidentStreamReadBlock*MOVIE_BLOCK_SIZE)+m_ReadRequest.GetChunkOffset();
        while( m_ResidentStreamAvailable < m_FileIndex+length )
        {
            ASSERT( m_Handle );

            m_ResidentStreamAvailable = (m_ResidentStreamReadBlock*MOVIE_BLOCK_SIZE)+m_ReadRequest.GetChunkOffset();
            x_DelayThread( 4 );
            UpdateReadahead();
        }

        if( m_Handle && (m_ResidentStreamAvailable == m_Length) )
        {
            x_fclose( m_Handle );
            m_Handle = NULL;
            LOG_MESSAGE( "movie_private::PeriodicUpdate", "Movie stream completed caching" );
            LOG_FLUSH();
        }
        else
        {
            UpdateReadahead();
        }
        s32     RemainingInBlock    = length;
        byte*   pBuffer             = m_pFileBuffer;            

        while( RemainingInBlock )
        {
            s32     BlockDataIsIn       = m_FileIndex / MOVIE_BLOCK_SIZE;
            s32     BlockOffset         = m_FileIndex % MOVIE_BLOCK_SIZE;
            s32     AvailableInBlock    = MOVIE_BLOCK_SIZE-BlockOffset;
            byte*   pDataBlock          = m_ResidentStreamBlocks[BlockDataIsIn];
            s32     BlockSize           = MIN( RemainingInBlock, AvailableInBlock );

            LOG_MESSAGE( "movie_private::PeriodicUpdate", "Copy from BlockOffset:0x%08x, Size:0x%08x, FilePos:0x%08x", BlockOffset, BlockSize, m_FileIndex );
            LOG_FLUSH();
            x_memcpy( pBuffer, pDataBlock+BlockOffset, BlockSize );
            RemainingInBlock    -= BlockSize;
            pBuffer             += BlockSize;
        }
    }
    else
    {
        ASSERT( m_Handle );
        io_open_file* pFile = (io_open_file*)m_Handle;
        m_ReadRequest.SetRequest( pFile, m_pFileBuffer, pFile->Offset+m_FileIndex, length, io_request::LOW_PRIORITY, TRUE, 0, 0, io_request::READ_OP ); 
        LOG_MESSAGE( "movie_private::PeriodicUpdate", "Queued IO Request" );
        g_IoMgr.QueueRequest( &m_ReadRequest );
        m_ReadRequest.AcquireSemaphore();
    }

    if( (pThread->IsActive()==FALSE) || m_ShutdownStreamer )
    {
        m_ShutdownStreamer = FALSE;
        pThread->Kill();
        ASSERT( FALSE );
    }

    if( length != MPEG_FILE_BUFFER_SIZE )
    {
        x_memset(m_pFileBuffer+length,0,MPEG_FILE_BUFFER_SIZE-length);
    }
    m_FileIndex += length;

    FlushCache(0);
    s32 DecodeLength;
    DecodeLength = sceMpegDemuxPss(&m_mpeg,m_pFileBuffer,length);
    // Because the stupid Sony PSS file format does not contain a number of streams within the file, 
    // we need to determine if audio is present by seeing whether or not something was added to the 
    // audio stream queue. This should always occur after a DemuxPss call.
    if( m_FirstBlockDecoded == FALSE )
    {
        m_AudioDataPresent = (m_AudioStream[0].pqReady->IsEmpty() == FALSE);
        if( m_AudioDataPresent )
        {
            LOG_MESSAGE( "movie_private::UpdateStream", "No audio data was detected in movie stream" );
        }
        m_FirstBlockDecoded = TRUE;
    }

    if( (DecodeLength != length) || (m_FileIndex == m_Length) )
    {
        if( m_VideoStream.pBuffer )
        {
            m_VideoStream.pqReady->Send(m_VideoStream.pBuffer,MQ_BLOCK);
            m_VideoStream.pBuffer = NULL;
        }

        if( m_AudioStream[0].pBuffer )
        {
            m_AudioStream[0].pqReady->Send(m_AudioStream[0].pBuffer,MQ_BLOCK);
            m_AudioStream[0].pBuffer = NULL;
        }

        if( m_AudioStream[1].pBuffer )
        {
            m_AudioStream[1].pqReady->Send(m_AudioStream[1].pBuffer,MQ_BLOCK);
            m_AudioStream[1].pBuffer = NULL;
        }

        x_DelayThread(1);             // Give the main thread some time to decode
        return;
    }
}
//-----------------------------------------------------------------------------
//------- Seperate thread for updating the streaming data coming from CD
//-----------------------------------------------------------------------------
// This is a very simple thread. It's entire goal in life is to read data from
// the storage media and split it in to the seperate audio/video streams to be
// consumed by the main thread when it calls sceMpegGetPicture. There is no explicit
// delays placed in here for other thread activation since it will implictly block
// when the audio or video streams become full. We do not have to deal with initial
// startup buffering because when the second file read request is issued, this thread
// should block sufficiently long enough to allow the frame decoding to start. This
// will implictly call the mpeg_AudioStream and mpeg_VideoStream callbacks. They do 
// most of the work :)
void mpeg_Streamer(s32 argc, char** argv)
{
    (void)argc;
    movie_private* pMovie=(movie_private*)argv;
    xthread* pThread = x_GetCurrentThread();

    ASSERT(argc==1);

    while( pThread->IsActive() )
    {
        pMovie->PeriodicUpdate();
    }
    LOG_MESSAGE("mpeg_Streamer","Noticed streamer is supposed to be shutdown");

    pMovie->m_ShutdownStreamer=FALSE;

    //-- jhowa Exit Thread
//  x_ExitThread(-1);
}


//-----------------------------------------------------------------------------
//------- CALLBACK FUNCTIONS NEEDED FOR PS2
//-----------------------------------------------------------------------------
// This is called from the sceMpegGetPicture codeblock. A very messed up way to
// do things. Very very clumsy.
s32 mpeg_Callback( sceMpeg *mp,sceMpegCbData *cbdata,void *pData )
{
    movie_private *pThis = (movie_private *)pData;
    xthread* pThread=x_GetCurrentThread();

    ASSERT( pThread );

    (void)mp;
    (void)cbdata;
    (void)pData;

    ASSERT(pThis);
    switch( cbdata->type )
    {
    case sceMpegCbError:
        break;
    case sceMpegCbNodata:

        if( pThis->m_CurrentBlock )
        {
            pThis->m_VideoStream.pqAvail->Send(pThis->m_CurrentBlock,MQ_BLOCK);
        }
        pThis->m_CurrentBlock = pThis->m_VideoStream.pqReady->Recv(MQ_BLOCK);
        if( pThread->IsActive()==FALSE )
        {
            pThread->Kill();
            ASSERT( FALSE );
        }
        ASSERT(pThis->m_CurrentBlock);
        ASSERT( ((u32)pThis->m_CurrentBlock & 63)==0 );

        FlushCache(0);
        *D4_MADR = (u32)pThis->m_CurrentBlock;
        *D4_QWC  = MPEG_BUFFER_BLOCK_SIZE >> 4;
        *D4_CHCR = 0x100;

          //  RestartAllDma();
        break;
    case sceMpegCbStopDMA:
        sceIpuStopDMA(&s_Env);
        break;
    case sceMpegCbRestartDMA:
        {
            u32 bp;
            u32 ifc;
            u32 fp;

            bp  = (s_Env.ipubp & IPU_BP_BP_M) >> IPU_BP_BP_O;
            ifc = (s_Env.ipubp & IPU_BP_IFC_M) >> IPU_BP_IFC_O;
            fp  = (s_Env.ipubp & IPU_BP_FP_M) >> IPU_BP_FP_O;
            s_Env.d4qwc     += (fp + ifc);
            s_Env.d4madr    -= 16 * (fp + ifc);
            s_Env.ipubp     &= IPU_BP_BP_M;
            sceIpuRestartDMA(&s_Env);
        }
        break;
    case sceMpegCbTimeStamp:
        ((sceMpegCbDataTimeStamp *)cbdata)->pts = -1;
        ((sceMpegCbDataTimeStamp *)cbdata)->dts = -1;
        break;
    default:
        ASSERT(FALSE);
    }

    return 1;
}

//-----------------------------------------------------------------------------
void movie_private::UpdateStream(mpeg_av_stream &Stream,u8 *pData,s32 count)
{
    s32 copylen;
    xthread* pThread=x_GetCurrentThread();

    ASSERT( pThread );

    //
    // Collect up a buffers worth of data and send it to the
    // appropriate message queue
    //
    while( count )
    {
        if( Stream.Remain==0 )
        {
            if( Stream.pBuffer )
            {
                FlushCache(0);//SyncDCache(Stream.pBuffer,Stream.pBuffer+MPEG_BUFFER_BLOCK_SIZE-1);
                Stream.pqReady->Send(Stream.pBuffer,MQ_BLOCK);
                Stream.pBuffer = NULL;
            }
        }

        if( Stream.pBuffer == NULL )
        {
            Stream.pBuffer = (u8 *)Stream.pqAvail->Recv(MQ_BLOCK);
            // This will return NULL if the thread is to kill itself.
            if( pThread->IsActive()==FALSE )
            {
                pThread->Kill();
                ASSERT( FALSE );
            }
            ASSERT( Stream.pBuffer );
            Stream.Remain=MPEG_BUFFER_BLOCK_SIZE;
            Stream.Index=0;
        }
        copylen = count;
        if( copylen > Stream.Remain )
        {
            copylen = Stream.Remain;
        }
        x_memcpy(Stream.pBuffer+Stream.Index,pData,copylen);
        Stream.Index+=copylen;
        Stream.Remain-=copylen;
        pData+=copylen;
        count-=copylen;
    }
}

//-----------------------------------------------------------------------------
s32 mpeg_Videostream   (sceMpeg *,sceMpegCbData *cbdata,void *pData)
{
    movie_private *pThis = (movie_private *)pData;

//    LOG_MESSAGE("mpeg_Videostream","Data:0x%08x, Length:0x%08x",cbdata->str.data, cbdata->str.len );
    pThis->UpdateStream(pThis->m_VideoStream,cbdata->str.data,cbdata->str.len);

    return 1;
}

//-----------------------------------------------------------------------------
s32 mpeg_Audiostream   (sceMpegCbData *cbdata,void *pData,s32 channel)
{
    movie_private *pThis = (movie_private *)pData;

    if( pThis->m_AudioFirst[channel] )
    {
        pThis->m_AudioFirst[channel] = FALSE;
        cbdata->str.data+=40;
        cbdata->str.len -=40;
    }

//    LOG_MESSAGE("mpeg_Audiostream","Channel:%d, Data:0x%08x, Length:0x%08x", channel, cbdata->str.data, cbdata->str.len );

    cbdata->str.data+=4;
    cbdata->str.len-=4;
    if( cbdata->str.len <=0 )
        return 1;
    pThis->UpdateStream(pThis->m_AudioStream[channel],cbdata->str.data,cbdata->str.len);

    return 1;
}

//-----------------------------------------------------------------------------
static  s32 mpeg_AudiostreamLeft   (sceMpeg *,sceMpegCbData *cbdata,void *pData)
{
    return mpeg_Audiostream(cbdata,pData,0);
}

//-----------------------------------------------------------------------------
static  s32 mpeg_AudiostreamRight  (sceMpeg *,sceMpegCbData *cbdata,void *pData)
{
   return mpeg_Audiostream(cbdata,pData,1);
}

//-----------------------------------------------------------------------------
void movie_private::InitStream(mpeg_av_stream &Stream,s32 buffcount)
{
    Stream.Remain  = 0;
    Stream.Index   = 0;
    Stream.pBuffer = NULL;
    Stream.pqReady = new xmesgq(buffcount);
    Stream.pqAvail = new xmesgq(buffcount);
}

//-----------------------------------------------------------------------------
void movie_private::KillStream(mpeg_av_stream &Stream)
{
    delete Stream.pqAvail;
    delete Stream.pqReady;
}

//-----------------------------------------------------------------------------
void movie_private::PurgeStream(mpeg_av_stream &Stream)
{
    void *pMsg;

    pMsg = Stream.pqReady->Recv(MQ_NOBLOCK);
    if( pMsg )
    {
        Stream.pqAvail->Send(pMsg,MQ_BLOCK);
    }
}


//-----------------------------------------------------------------------------
void movie_private::SetVolume(f32 Volume)
{
    m_Volume = Volume;

    s_ChannelManager.SetVolume( m_AudioChannel[0], Volume, 0.0f );
    s_ChannelManager.SetVolume( m_AudioChannel[1], 0.0f, Volume );
}
//-----------------------------------------------------------------------------
void movie_private::Pause(void)
{
}

//-----------------------------------------------------------------------------
void movie_private::Resume(void)
{
}

//-----------------------------------------------------------------------------
void movie_private::UpdateReadahead( void )
{
    xthread* pThread=x_GetCurrentThread();

    if( (pThread->IsActive()==FALSE) || m_ShutdownStreamer )
    {
        m_ShutdownStreamer = FALSE;
        pThread->Kill();
        ASSERT( FALSE );
    }

   if( (m_ReadRequest.GetStatus() == io_request::COMPLETED) && (m_ResidentStreamAvailable != m_Length) )
    {
        ASSERT( m_Handle );
        if( m_Handle == NULL )
        {
            return;
        }

        s32             Offset          = (m_ResidentStreamReadBlock+1)*MOVIE_BLOCK_SIZE;
        s32             RemainingLength = m_Length-Offset;
        if( RemainingLength <= 0 )
        {
            return;
        }
        m_ResidentStreamReadBlock++;

        ASSERT( m_ResidentStreamReadBlock != m_ResidentStreamBlocks.GetCount() );
        io_open_file*   pFile           = (io_open_file*)m_Handle;

        s32 BlockSize = MIN( RemainingLength, MOVIE_BLOCK_SIZE );
        m_ReadRequest.SetRequest( pFile, m_ResidentStreamBlocks[m_ResidentStreamReadBlock], pFile->Offset+Offset, BlockSize, io_request::LOW_PRIORITY, FALSE, 0, 0, io_request::READ_OP ); 
        LOG_MESSAGE( "movie_private::UpdateReadahead", "Queued request. Offset:0x%08x, Length:0x%08x", Offset, BlockSize );
        g_IoMgr.QueueRequest( &m_ReadRequest );
        LOG_FLUSH();
    }
}