#include "x_target.hpp"
#ifdef TARGET_PS2
#error This file should not be compiled for PlayStation 2. Please check your exclusions on your project spec.
#endif

#include "x_files.hpp"
#include "x_threads.hpp"
#include "Entropy.hpp"
#include "audio/audio_hardware.hpp"
#include "movieplayer.hpp"

movie_player Movie;

#ifdef TARGET_XBOX
#include "Entropy/xbox/xbox_private.hpp"
#define NUM_BUFFERS 1
#endif

#ifdef TARGET_PC
#define NUM_BUFFERS 2 // or 1 ?
#endif

//------------------------------------------------------------------------------
movie_private::movie_private(void)
{
    m_pqFrameAvail = NULL;
}

//------------------------------------------------------------------------------
// We can add ram resident movie functionality later. It's not exactly hard! (no)
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
    RADSetMemory(s_malloc,s_free);
    m_pqFrameAvail = new xmesgq(2);

    m_nMaxBitmaps = MOVIE_FIXED_WIDTH / MOVIE_STRIP_WIDTH;
#ifdef TARGET_XBOX
/*
    for (s32 i=0;i<NUM_BUFFERS;i++)
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
}

//------------------------------------------------------------------------------
xbool movie_private::Open(const char* pFilename, xbool PlayResident, xbool IsLooped)
{
    m_IsLooped = IsLooped;

#ifdef TARGET_PC
    BinkSetSoundTrack(1, (unsigned long*)&m_Language);
    
    u32 Flags = BINKNOSKIP | BINKSNDTRACK;
    if (PlayResident) {
        Flags |= BINKPRELOADALL;
    }
    m_Handle = BinkOpen(xfs("C:\\GameData\\A51\\Release\\PC\\%s.bik",pFilename), Flags);
#endif
#ifdef TARGET_XBOX
    BinkSetSoundTrack(1, (unsigned long*)&m_Language);

    u32 Flags = BINKNOSKIP | BINKSNDTRACK;
    if (PlayResident) {
        Flags |= BINKPRELOADALL;
    }
    m_Handle = BinkOpen(xfs("D:\\movies\\%s.bik", pFilename), Flags);
    
    BinkLoadConverter( BINKSURFACE32 );
    
    U32 bins[ 2 ];
    bins[ 0 ] = DSMIXBIN_3D_FRONT_LEFT;
    bins[ 1 ] = DSMIXBIN_3D_FRONT_RIGHT;
    
    BinkSetMixBins( m_Handle, m_Language, bins, 2 );
#endif
    if (!m_Handle) {
        m_IsFinished = TRUE;
        return FALSE;
    }

    BinkSetVolume(m_Handle, m_Language, (s32)(m_Volume * 32767));
    m_Width = m_Handle->Width;
    m_Height = m_Handle->Height;

    m_nBitmaps = 1;

    ASSERTS(m_nBitmaps <= m_nMaxBitmaps,"Movie size is larger than decode buffer size");

    m_IsFinished = FALSE;
    m_IsRunning = TRUE;
    
    return TRUE;
}

//------------------------------------------------------------------------------
void movie_private::SetVolume(f32 Volume)
{
#if defined(TARGET_PC)
    m_Volume = Volume;
    BinkSetVolume(m_Handle,m_Language,(s32)(m_Volume*32767));
#endif
}

//------------------------------------------------------------------------------
void movie_private::Close(void)
{
    if (!m_Handle)
        return;
    m_Handle = NULL;  
    Kill();
}

//------------------------------------------------------------------------------
void movie_private::Pause(void)
{
    if (Movie.IsPlaying())
    {
#ifdef TARGET_PC
    BinkPause(m_Handle,1);
#endif
    }
}

//------------------------------------------------------------------------------
void movie_private::Resume(void)
{
    if (Movie.IsPlaying())
    {
#ifdef TARGET_PC
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

#ifdef TARGET_XBOX   
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
#endif
#ifdef TARGET_PC
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
#endif
    m_IsRunning=FALSE;
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
    ASSERT(!m_IsFinished);
    
    while (BinkWait(m_Handle)) {
        x_DelayThread(1);
    }
    
    BinkDoFrame(m_Handle);
    BinkNextFrame(m_Handle);

#ifdef TARGET_PC  
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
    
    IDirect3DSurface9* pBackBuffer;
    VERIFY(!g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer));

    D3DSURFACE_DESC desc;
    VERIFY(!pBackBuffer->GetDesc(&desc));

    D3DLOCKED_RECT LockedRect;
    VERIFY(!pBackBuffer->LockRect(&LockedRect, NULL, 0));
	
	s32 WindowWidth, WindowHeight;
    eng_GetRes(WindowWidth, WindowHeight);
	x_printf("Window Width: %d, Window Height: %d\n", WindowWidth, WindowHeight);

    u8* pBuffer = (u8*)LockedRect.pBits;
    BinkCopyToBuffer
    (
        m_Handle, 
        pBuffer + (((WindowHeight - m_Height) / 2) * LockedRect.Pitch),
        LockedRect.Pitch,
        m_Height,
        U32((f32(WindowWidth) - f32(m_Width)) / 2.0f),
        0,
        BINK_BITMAP_FORMAT | BINKCOPYALL
    );

    VERIFY(!pBackBuffer->UnlockRect());
    pBackBuffer->Release();
    
    //g_pd3dDevice->Present(NULL, NULL, NULL, NULL); //Какого хуя эта хуйня не рендерится бля
#endif
#ifdef TARGET_XBOX    
    xbitmap* pBitmap;
    pBitmap = (xbitmap*)m_pqFrameAvail->Recv(MQ_BLOCK);
    ASSERT(pBitmap);

    g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 0, 0);

    IDirect3DSurface8* pBackBuffer;
    VERIFY(!g_pd3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer));

    D3DSURFACE_DESC desc;
    VERIFY(!pBackBuffer->GetDesc(&desc));

    D3DLOCKED_RECT LockedRect;
    VERIFY(!pBackBuffer->LockRect(&LockedRect, NULL, D3DLOCK_TILED));

    u8* pBuffer = (u8*)LockedRect.pBits;
    BinkCopyToBuffer
    (
        m_Handle, 
        pBuffer + (((desc.Height - m_Height) / 2) * LockedRect.Pitch), 
        LockedRect.Pitch, 
        GetHeight(), 
        U32((f32(g_PhysW) - 640.0f) / 2.0f), 
        U32((f32(g_PhysH) - 480.0f) / 2.0f), 
        BINK_BITMAP_FORMAT | BINKCOPYALL
    );

    VERIFY(!pBackBuffer->UnlockRect());
    pBackBuffer->Release();
#endif
    if ( m_Handle->FrameNum >= ( m_Handle->Frames - 1 ) )
    {
        if( m_IsLooped )
            BinkGoto( m_Handle,1,0 );
        else
            m_IsFinished = TRUE;
    }
    
    return NULL;
}
