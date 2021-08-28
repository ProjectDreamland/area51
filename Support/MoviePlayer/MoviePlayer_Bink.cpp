#include "x_target.hpp"
#if defined(TARGET_PS2)
#error This file should not be compiled for PlayStation 2. Please check your exclusions on your project spec.
#endif

#include "x_files.hpp"
#include "x_threads.hpp"
#include "Entropy.hpp"
#include "audio/audio_hardware.hpp"
#include "movieplayer.hpp"

movie_player Movie;

#if defined(TARGET_GCN)

void* ARAM_Alloc(u32 nBytes)
{
	return g_AudioHardware.AllocAudioRam(nBytes);
}

void ARAM_Free(void* Address)
{
	g_AudioHardware.FreeAudioRam(Address);
}
#endif

#ifdef TARGET_XBOX
#   include "Entropy/xbox/xbox_private.hpp"
#   define NUM_BUFFERS 1
#else
#   define NUM_BUFFERS 2
#endif

//------------------------------------------------------------------------------
movie_private::movie_private(void)
{
    m_pqFrameAvail = NULL;
}

//------------------------------------------------------------------------------
// We can add ram resident movie functionality later. It's not exactly hard!
//
void* RADLINK s_malloc(U32 bytes)
{
    return x_malloc(bytes);
}

void RADLINK s_free(void* ptr)
{
    x_free(ptr);
}

//------------------------------------------------------------------------------
void movie_private::Init(void)
{
#if !defined(TARGET_PC)
#if defined(TARGET_GCN)
    RADARAMCALLBACKS aram_callbacks = { ARAM_Alloc, ARAM_Free };  
    BinkSoundUseAX( &aram_callbacks );
#endif

    RADSetMemory(s_malloc,s_free);
    m_pqFrameAvail = new xmesgq(2);

	m_nMaxBitmaps = MOVIE_FIXED_WIDTH / MOVIE_STRIP_WIDTH;

#ifdef TARGET_GCN
    void*   pFrameBuffer;
    s32     nFrameBufferBytes;
    s32     i,j;

    nFrameBufferBytes = MOVIE_STRIP_WIDTH * MOVIE_STRIP_HEIGHT * sizeof(s32);

    for (i=0;i<NUM_BUFFERS;i++)
    {
    	m_pBitmaps[i] = new xbitmap[m_nMaxBitmaps];
        ASSERT( m_pBitmaps[i] );
        for (j=0;j<m_nMaxBitmaps;j++)
        {
            pFrameBuffer = x_malloc( nFrameBufferBytes );
            ASSERT(pFrameBuffer);

	        (m_pBitmaps[i])[j].Setup
            (
                MOVIE_BITMAP_FORMAT,
		        MOVIE_STRIP_WIDTH,
		        MOVIE_STRIP_HEIGHT,
		        TRUE,                       // xbitmap will release bitmap data on deletion
		        (byte*)pFrameBuffer
            );

            if (j&1)
            {
                x_memset(pFrameBuffer,0xff,nFrameBufferBytes);
            }
            else
            {
                x_memset(pFrameBuffer,0x00,nFrameBufferBytes);
            }

            (m_pBitmaps[i])[j].SetPreSwizzled();
            vram_Register( (m_pBitmaps[i])[j] );
            vram_SetFilterMode( (m_pBitmaps[i])[j],VRAM_POINT,VRAM_POINT);
        }
        m_pqFrameAvail->Send(m_pBitmaps[i],MQ_BLOCK);
    }
    gcneng_EnableFrameLock(FALSE);
#endif

#if defined(TARGET_XBOX)

/*  for (s32 i=0;i<NUM_BUFFERS;i++)
    {
        u32 nFrameBufferBytes = 640*480*sizeof(s32);
        m_pBitmaps[i] = new xbitmap;
        ASSERT( m_pBitmaps[i] );

        IDirect3DSurface8* Surface;
        texture_factory::handle Handle = g_TextureFactory.Create(
            "Movie player"      , // Resource name
            640*4               , // Pitch
            640                 , // Width
            480                 , // Height
            Surface             , // Surface
            D3DFMT_LIN_A8R8G8B8 , // Diffuse buffer format
            kPOOL_TILED );

	    m_pBitmaps[i]->Setup( MOVIE_BITMAP_FORMAT,640,480,FALSE,(byte*)Handle->GetPtr() );

        extern s32 vram_Register( texture_factory::handle );
        s32 ID = vram_Register( Handle );
        m_pBitmaps[i]->Preregistered();
        m_pBitmaps[i]->SetVRAMID(ID);
    }
    m_nBitmaps=1;
*/
#endif

    m_Language = XL_LANG_ENGLISH;   // defined as 0
    m_Volume = 1.0f;
#endif
}

//------------------------------------------------------------------------------
xbool movie_private::Open(const char* pFilename, xbool PlayResident, xbool IsLooped )
{
#if !defined(TARGET_GCN)&&!defined(TARGET_XBOX)
	return FALSE;
#else
    (void)PlayResident;
	// Attempt to open the file. If this fails, then we cannot find the file on disk.
	// Note: For non-PC architecture, this is a direct-to-disk file open request so
	// the bink files will need to be on the disk but not in a files.dat form.
    m_IsLooped = IsLooped;
#ifdef TARGET_XBOX
    //xbitmap* pBitmap = m_pBitmaps[0];
    //if( !pBitmap )
    //    Init();
    BinkSetSoundTrack(1,(unsigned long*)&m_Language);
#else
    BinkSetSoundTrack(1,(u32*)&m_Language);
#endif

#ifdef TARGET_XBOX
    #if CONFIG_IS_DEMO
    m_Handle = BinkOpen(xfs("D:\\Area51\\movies\\%s.bik",pFilename), BINKNOSKIP|BINKSNDTRACK);
    #else
    u32 Flags = BINKNOSKIP|BINKSNDTRACK;
    if( PlayResident )
        Flags |= BINKPRELOADALL;
    m_Handle = BinkOpen(xfs("D:\\movies\\%s.bik",pFilename), Flags);
    #endif
    // Load convertors
    BinkLoadConverter( BINKSURFACE32 );
#else
    m_Handle = BinkOpen(xfs("common/movies/%s.bik",pFilename),BINKNOSKIP|BINKSNDTRACK);
#endif
	// Could not open the movie file, just exit.
	if (!m_Handle)
	{
        m_IsFinished = TRUE;
		return FALSE;
	}

    U32 bins[ 2 ];
    bins[ 0 ] = DSMIXBIN_3D_FRONT_LEFT;
    bins[ 1 ] = DSMIXBIN_3D_FRONT_RIGHT;
    BinkSetMixBins( m_Handle, m_Language, bins, 2 );

    BinkSetVolume(m_Handle,m_Language,(s32)(m_Volume*32767));
	m_Width = m_Handle->Width;
	m_Height = m_Handle->Height;

#if defined(TARGET_GCN)
	m_nBitmaps = m_Width / MOVIE_STRIP_WIDTH;
#else
	m_nBitmaps = 1;
#endif

    ASSERTS(m_nBitmaps <= m_nMaxBitmaps,"Movie size is larger than decode buffer size");


	m_IsFinished = FALSE;
    m_IsRunning = TRUE;

	return TRUE;
#endif
}

//------------------------------------------------------------------------------
void movie_private::SetVolume(f32 Volume)
{
#if !defined(TARGET_PC)

    m_Volume = Volume;
    BinkSetVolume(m_Handle,m_Language,(s32)(m_Volume*32767));

#endif
}

//------------------------------------------------------------------------------
void movie_private::Close(void)
{
    if (!m_Handle)
        return;
#if !defined(TARGET_PC)
    BinkClose(m_Handle);
#endif
    m_Handle = NULL;
#ifdef TARGET_XBOX
    Kill();
#endif
}

//------------------------------------------------------------------------------
void movie_private::Pause(void)
{
    if (Movie.IsPlaying())
    {
#if !defined(TARGET_PC)
        BinkPause(m_Handle,1);
#endif
    }
}

//------------------------------------------------------------------------------
void movie_private::Resume(void)
{
    if (Movie.IsPlaying())
    {
#if !defined(TARGET_PC)
        BinkPause(m_Handle,0);
#endif
    }
}

//------------------------------------------------------------------------------
void movie_private::Kill(void)
{
    ASSERT(!m_Handle);	
    delete m_pqFrameAvail;
    m_pqFrameAvail = NULL;

#ifndef TARGET_XBOX
    ASSERT(m_pBitmaps);
    s32 i;
    s32 j;
    for (i=0;i<NUM_BUFFERS;i++)
    {
        for (j=0;j<m_nMaxBitmaps;j++)
        {
            vram_Unregister(m_pBitmaps[i][j] );
        }
	    delete []m_pBitmaps[i];
        m_pBitmaps[i] = NULL;
    }
#else
    /*
    for( s32 i=0;i<NUM_BUFFERS;i++ )
    {
        if( m_pBitmaps[i] )
        {
            vram_Unregister( *m_pBitmaps[i] );
            m_pBitmaps[i]->DePreregister();
	        delete m_pBitmaps[i];
            m_pBitmaps[i] = NULL;
        }
    }
    */
#endif
    m_IsRunning=FALSE;
#if defined(TARGET_GCN)
    // NOTE: Will need a version of this for xbox.
    gcneng_EnableFrameLock(TRUE);
#endif
}

//------------------------------------------------------------------------------
void movie_private::ReleaseBitmap(xbitmap* pBitmap)
{
    ASSERT(!m_pqFrameAvail->IsFull());
    m_pqFrameAvail->Send(pBitmap,MQ_BLOCK);
}

//------------------------------------------------------------------------------
xbitmap* movie_private::Decode(void)
{
#if !defined(TARGET_PC)

    ASSERT(!m_IsFinished);

    while (BinkWait(m_Handle))
    {
        x_DelayThread(1);
#if defined(TARGET_GCN_DVD)
        // RMB - God this is a hack
        extern volatile xbool g_StopMovie;
        if( g_StopMovie )
            return NULL;
#endif
    }

    BinkDoFrame(m_Handle);
    BinkNextFrame(m_Handle);
#ifndef TARGET_XBOX
    xbitmap* pBitmap;
    pBitmap = (xbitmap*)m_pqFrameAvail->Recv(MQ_BLOCK);
    ASSERT(pBitmap);
#else
    //xbitmap* pBitmap = m_pBitmaps[0];
    //ASSERT(pBitmap);
#endif

#if defined(TARGET_GCN)
    s32 i;
    for (i=0;i<m_nBitmaps;i++)
    {
        BinkCopyToBufferRect(m_Handle,                          // Handle
                        (void*)pBitmap[i].GetPixelData(),       // Dest address
                        pBitmap[i].GetWidth() * sizeof(s32),    // Dest Pitch
                        pBitmap[i].GetHeight(),                 // Dest Height
                        0,                                      // Dest X
                        0,                                      // Dest Y
                        i*pBitmap[i].GetWidth(),                // Src X
                        0,                                      // Src Y
                        pBitmap[i].GetWidth(),                  // Src W
                        m_Handle->Height,                       // Src H
                        BINK_BITMAP_FORMAT|BINKCOPYALL);
    	DCStoreRange((void*)pBitmap[i].GetPixelData(),pBitmap[i].GetDataSize());

    }
#endif

#if defined(TARGET_XBOX)

    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0,0 );

    IDirect3DSurface8* pBackBuffer;
    VERIFY( !g_pd3dDevice->GetBackBuffer( 0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer ));

    D3DSURFACE_DESC desc;
    VERIFY( !pBackBuffer->GetDesc( &desc ));

    D3DLOCKED_RECT LockedRect;
    VERIFY( !pBackBuffer->LockRect( &LockedRect,NULL,D3DLOCK_TILED ));

    u8* pBuffer = (u8*) LockedRect.pBits;
    BinkCopyToBuffer
    (
        m_Handle,
        pBuffer+(((desc.Height-m_Height)/2)*LockedRect.Pitch),
        LockedRect.Pitch,
        GetHeight( ),
        U32((f32(g_PhysW)-640.0f)/2.0f),
        U32((f32(g_PhysH)-480.0f)/2.0f),
        BINK_BITMAP_FORMAT|BINKCOPYALL
    );
    VERIFY( !pBackBuffer->UnlockRect( ));
    pBackBuffer->Release( );
#endif

    if ( m_Handle->FrameNum >= ( m_Handle->Frames - 1 ) )
	{
        if( m_IsLooped )
            BinkGoto( m_Handle,1,0 );
        else
            m_IsFinished = TRUE;
    }
#ifdef TARGET_XBOX
    return NULL;
#else
    return pBitmap;
#endif
#else
    return NULL;
#endif
}
