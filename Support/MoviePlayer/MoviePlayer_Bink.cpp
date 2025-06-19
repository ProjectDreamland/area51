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
#define NUM_BUFFERS 1 // or 2 ?
#endif

//------------------------------------------------------------------------------
movie_private::movie_private(void)
{
    m_pqFrameAvail = NULL;
#ifdef TARGET_PC
    m_Surface = NULL;
    m_RenderSize.Set(640.0f, 480.0f); //Default render state.
    m_bForceStretch = TRUE; //Scaling video by screen size.
#endif
}

//------------------------------------------------------------------------------
// We can add ram resident movie functionality later. It's not exactly hard! (no)
//
void* RADLINK s_malloc(U32 bytes)
{
    return x_malloc(bytes);
}

//------------------------------------------------------------------------------
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

    m_Language = XL_LANG_ENGLISH;
    m_Volume = 1.0f;  
#ifdef TARGET_PC
    BinkSoundUseDirectSound(NULL);
#endif
}

//------------------------------------------------------------------------------
xbool movie_private::Open(const char* pFilename, xbool PlayResident, xbool IsLooped)
{
    m_IsLooped = IsLooped;
#ifdef TARGET_PC
    BinkSetSoundTrack(1, (unsigned long*)&m_Language);
    
    u32 Flags = BINKNOSKIP | BINKSNDTRACK;
    if (PlayResident) 
    {
        Flags |= BINKPRELOADALL;
    }
    m_Handle = BinkOpen(xfs("C:\\GameData\\A51\\Release\\PC\\%s.bik",pFilename), Flags);
#endif
#ifdef TARGET_XBOX
    BinkSetSoundTrack(1, (unsigned long*)&m_Language);

    u32 Flags = BINKNOSKIP | BINKSNDTRACK;
    if (PlayResident) 
    {
        Flags |= BINKPRELOADALL;
    }
    m_Handle = BinkOpen(xfs("D:\\movies\\%s.bik", pFilename), Flags);
    
    BinkLoadConverter( BINKSURFACE32 );
    
    U32 bins[ 2 ];
    bins[ 0 ] = DSMIXBIN_3D_FRONT_LEFT;
    bins[ 1 ] = DSMIXBIN_3D_FRONT_RIGHT;
    
    BinkSetMixBins( m_Handle, m_Language, bins, 2 );
#endif
    if (!m_Handle) 
    {
        m_IsFinished = TRUE;
        return FALSE;
    }
    
    BinkSetVolume(m_Handle, m_Language, (s32)(m_Volume * 32767));
    m_Width = m_Handle->Width;
    m_Height = m_Handle->Height;

    m_nBitmaps = 1;

    ASSERTS(m_nBitmaps <= m_nMaxBitmaps,"Movie size is larger than decode buffer size");
#ifdef TARGET_PC
    if (g_pd3dDevice && !m_Surface) 
    {
        HRESULT hr = g_pd3dDevice->CreateOffscreenPlainSurface(
            m_Width, m_Height,
            D3DFMT_A8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_Surface,
            NULL);
            
        if (FAILED(hr)) 
        {
            m_IsFinished = TRUE;
            return FALSE;
        }
    }
#endif
    m_IsFinished = FALSE;
    m_IsRunning = TRUE;
    
    return TRUE;
}

//------------------------------------------------------------------------------
void movie_private::SetVolume(f32 Volume)
{
    m_Volume = Volume;
    BinkSetVolume(m_Handle,m_Language,(s32)(m_Volume*32767));
}

//------------------------------------------------------------------------------
void movie_private::Close(void)
{
    if (!m_Handle)
        return;       
#ifdef TARGET_PC
    BinkSetSoundOnOff(m_Handle, 0);
#endif
    BinkClose(m_Handle);
    m_Handle = NULL;      
#ifdef TARGET_PC
    if (m_Surface) 
    {
        m_Surface->Release();
        m_Surface = NULL;
    }
#endif    
    Kill();
}

//------------------------------------------------------------------------------
void movie_private::Pause(void)
{
    if (Movie.IsPlaying())
    {
    BinkPause(m_Handle,1);
    }
}

//------------------------------------------------------------------------------
void movie_private::Resume(void)
{
    if (Movie.IsPlaying())
    {
    BinkPause(m_Handle,0);
    }
}

//------------------------------------------------------------------------------
void movie_private::Kill(void)
{
    ASSERT(!m_Handle);    
    delete m_pqFrameAvail;
    m_pqFrameAvail = NULL;
#ifdef TARGET_PC
    if (m_Surface) 
    {
        m_Surface->Release();
        m_Surface = NULL;
    }
#endif
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
    
    while (BinkWait(m_Handle)) 
    {
        x_DelayThread(1);
    }
    
    BinkDoFrame(m_Handle);
#ifdef TARGET_PC  
    s32 WindowWidth, WindowHeight;
    eng_GetRes(WindowWidth, WindowHeight);
    
    if (m_Surface) 
    {
        D3DLOCKED_RECT lockRect;
        if (SUCCEEDED(m_Surface->LockRect(&lockRect, NULL, 0))) 
        {
            BinkCopyToBuffer(
                m_Handle,
                lockRect.pBits,
                lockRect.Pitch,
                m_Height,
                0, 0,
                BINK_BITMAP_FORMAT | BINKCOPYALL
            );
            
            m_Surface->UnlockRect();
        }
        
        IDirect3DSurface9* pBackBuffer = NULL;
        g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        
        if (pBackBuffer) 
        {
            RECT destRect;
            
            if (m_bForceStretch) 
            {
                destRect.left = 0;
                destRect.top = 0;
                destRect.right = WindowWidth;
                destRect.bottom = WindowHeight;
            } 
            else 
            {
                float aspectRatio = (float)m_Width / (float)m_Height;
                float screenRatio = (float)WindowWidth / (float)WindowHeight;
                
                if (screenRatio > aspectRatio) 
                {
                    int targetWidth = (int)(WindowHeight * aspectRatio);
                    destRect.left = (WindowWidth - targetWidth) / 2;
                    destRect.top = 0;
                    destRect.right = destRect.left + targetWidth;
                    destRect.bottom = WindowHeight;
                } 
                else 
                {
                    int targetHeight = (int)(WindowWidth / aspectRatio);
                    destRect.left = 0;
                    destRect.top = (WindowHeight - targetHeight) / 2;
                    destRect.right = WindowWidth;
                    destRect.bottom = destRect.top + targetHeight;
                }
            }
            
            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
            
            RECT srcRect = {0, 0, m_Width, m_Height};
            g_pd3dDevice->StretchRect(m_Surface, &srcRect, pBackBuffer, &destRect, D3DTEXF_LINEAR);
            
            pBackBuffer->Release();
        }
    }
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
    BinkNextFrame(m_Handle);
    
    if ( m_Handle->FrameNum >= ( m_Handle->Frames - 1 ) )
    {
        if( m_IsLooped )
            BinkGoto( m_Handle,1,0 );
        else
            m_IsFinished = TRUE;
    }
    
    return NULL;
}