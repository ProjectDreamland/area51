// IAL.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <mmreg.h>
#include "dsound.h"

#include "x_files.hpp"
#include "x_threads.hpp"
#include "x_log.hpp"

#include "IAL.h"

//==============================================================================

//#define WRITE_DEBUG_FILE

#ifdef cgalley
#define LOGGING_ENABLED 0
#else
#define LOGGING_ENABLED 0
#endif

//==============================================================================

#define IAL_MAX_CHANNELS        128
#define IAL_SAMPLE_RATE         22050
#define IAL_FRAME_TIME_MS       20
#define IAL_SAMPLES_PER_FRAME   (IAL_SAMPLE_RATE/(1000/IAL_FRAME_TIME_MS)*4)

#if (IAL_SAMPLE_RATE/(1000/IAL_FRAME_TIME_MS)*(1000/IAL_FRAME_TIME_MS)) != IAL_SAMPLE_RATE
#error IAL_SAMPLES_PER_FRAME not integer
#endif

//==============================================================================

#ifdef WRITE_DEBUG_FILE
X_FILE* s_pDebugFile = NULL;
#endif

xbool                   s_Initialized = FALSE;

IDirectSound8*          s_lpds;
IDirectSoundBuffer*     s_lpsb;
IDirectSoundBuffer8*    s_lpsb8;
DWORD                   s_BufferBytes;
xbool                   s_KillMixer = FALSE;
f32                     s_SystemVolume = 1.0f;

HWND        IAL_hWnd        = NULL;
xthread*    IAL_pMixThread  = NULL;

ial_channel IAL_Channels        [IAL_MAX_CHANNELS];
s32         IAL_MixL            [IAL_SAMPLES_PER_FRAME];
s32         IAL_MixR            [IAL_SAMPLES_PER_FRAME];
s16         IAL_Out             [IAL_SAMPLES_PER_FRAME*2];
s32         IAL_OutputAmplitude [2] = { 0, 0 };

CRITICAL_SECTION    IAL_CriticalSection;


#define MAX_COMB_SIZE   16384
s32         g_CombBufferL[MAX_COMB_SIZE];
s32         g_CombBufferR[MAX_COMB_SIZE];
s32         g_CombSize = MAX_COMB_SIZE;
s32         g_CombIndexIn = 0;
s32         g_CombIndexOut = 0;
s32         g_CombFactor = 0;


//==============================================================================

void IAL_GetMutex( void )
{
    if (!s_Initialized)
        return;
        
    EnterCriticalSection( &IAL_CriticalSection );
}

//==============================================================================

void IAL_ReleaseMutex( void )
{
    if (!s_Initialized)
        return;
        
    LeaveCriticalSection( &IAL_CriticalSection );
}

//==============================================================================

void IAL_MixChannel( ial_channel* pChannel, s32* pL, s32* pR, s32 nDstSamples )
{
    f32 VolumeL = pChannel->VolumeL;
    f32 VolumeR = pChannel->VolumeR;
    s32 NewVolL = (s32)(32767.0f * MINMAX(-1.0f, VolumeL, 1.0f));
    s32 NewVolR = (s32)(32767.0f * MINMAX(-1.0f, VolumeR, 1.0f));
    s32 VolL    = pChannel->MixedVolL;
    s32 VolR    = pChannel->MixedVolR;
    s32 dVolL   = (NewVolL - VolL) / nDstSamples;
    s32 dVolR   = (NewVolR - VolR) / nDstSamples;

	//s32 nSrcSamples =
    
	// Mix the samples
    s32 iDst = 0;
    while( (nDstSamples > 0) && (pChannel->State == IAL_PLAY) )
    {
        s32 SrcStop;
        s32 SrcLoopTo;

        // Get the stop point in the source samples
        if( pChannel->Looped )
        {
/*
			if( pChannel->Cursor > pChannel->LoopEnd )
			{
				SrcStop   = pChannel->nSamples;
				SrcLoopTo = 0;
			}
			else if( (pChannel->Cursor + nDstSamples) > pChannel->LoopEnd )
			{
				SrcStop   = pChannel->LoopEnd;
				SrcLoopTo = pChannel->LoopEnd-1;
				pChannel->Looped = FALSE;
			}
			else
*/
			{
				SrcStop   = pChannel->LoopEnd;
				SrcLoopTo = pChannel->LoopStart;
			}
        }
        else
        {
            SrcStop   = pChannel->nSamples;
            SrcLoopTo = pChannel->nSamples-1;
        }

        s32 Ratio  = (s32)(65536.0f * pChannel->SampleRate * pChannel->Pitch / IAL_SAMPLE_RATE);
        s32 RatioI = Ratio >> 16;
        s32 RatioF = Ratio & 65535;

        // Mix the samples into the mix buffers
        s32 iSrc     = pChannel->Cursor;
        s32 iSrcFrac = pChannel->Fraction;
        while( (nDstSamples > 0) && (iSrc < SrcStop) )
        {
//            ASSERT( iSrc >= 0 );
//            ASSERT( iSrc < pChannel->nSamples );
//            ASSERT( iDst < IAL_SAMPLES_PER_FRAME );
//            ASSERT( !pChannel->Looped || (iSrc < pChannel->LoopEnd) );

            s32 iSrc2 = iSrc+1;
            if( iSrc2 >= SrcStop )
                iSrc2 = SrcLoopTo;

            s32 s1 = (pChannel->pData[iSrc ] * (65535-iSrcFrac)) >> 16;
            s32 s2 = (pChannel->pData[iSrc2] * iSrcFrac) >> 16;
            s32 s  = s1 + s2;

            pL[iDst] += (s * VolL) >> 15;
            pR[iDst] += (s * VolR) >> 15;

            iSrc     += RatioI;
            iSrcFrac += RatioF;
            iSrc     += iSrcFrac >> 16;
            iSrcFrac &= 65535;

            iDst++;
            nDstSamples--;

            VolL += dVolL;
            VolR += dVolR;
        }

        // At the end of the sample or the end of the loop?
        bool AtEnd      = iSrc >= pChannel->nSamples;
        bool AtLoopEnd  = pChannel->Looped && (iSrc >= pChannel->LoopEnd);
        if( AtLoopEnd )
        {
            pChannel->Cursor    = iSrc - (pChannel->LoopEnd - pChannel->LoopStart);
            pChannel->Fraction  = iSrcFrac;
        }
        else if( AtEnd )
        {
            pChannel->Cursor   = pChannel->nSamples;
            pChannel->Fraction = 0;
            pChannel->State    = IAL_DONE;
        }
        else
        {
            pChannel->Cursor   = iSrc;
            pChannel->Fraction = iSrcFrac;
        }
    }

    pChannel->MixedVolL = NewVolL;
    pChannel->MixedVolR = NewVolR;
}

//==============================================================================

/* Simple implementation of Biquad filters -- Tom St Denis
*
* Based on the work

Cookbook formulae for audio EQ biquad filter coefficients
---------------------------------------------------------
by Robert Bristow-Johnson, pbjrbj@viconet.com  a.k.a. robert@audioheads.com

* Available on the web at

http://www.smartelectronix.com/musicdsp/text/filters005.txt

* Enjoy.
*
* This work is hereby placed in the public domain for all purposes, whether
* commercial, free [as in speech] or educational, etc.  Use the code and please
* give me credit if you wish.
*
* Tom St Denis -- http://tomstdenis.home.dhs.org
*/

/* this would be biquad.h */

#ifndef M_LN2
#define M_LN2	   0.69314718055994530942f
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846f
#endif

/* whatever sample type you want */
typedef float smp_type;

/* this holds the data required to update samples thru a filter */
typedef struct {
    smp_type a0, a1, a2, a3, a4;
    smp_type x1, x2, y1, y2;
}
biquad;

extern smp_type BiQuad(smp_type sample, biquad * b);
extern biquad *BiQuad_new(int type, smp_type dbGain, /* gain of filter */
                          smp_type freq,             /* center frequency */
                          smp_type srate,            /* sampling rate */
                          smp_type bandwidth);       /* bandwidth in octaves */

/* filter types */
enum {
    LPF, /* low pass filter */
    HPF, /* High pass filter */
    BPF, /* band pass filter */
    NOTCH, /* Notch Filter */
    PEQ, /* Peaking band EQ filter */
    LSH, /* Low shelf filter */
    HSH /* High shelf filter */
};

/* Below this would be biquad.c */
/* Computes a BiQuad filter on a sample */
smp_type BiQuad(smp_type sample, biquad * b)
{
    smp_type result;

    /* compute result */
    result = b->a0 * sample + b->a1 * b->x1 + b->a2 * b->x2 -
        b->a3 * b->y1 - b->a4 * b->y2;

    /* shift x1 to x2, sample to x1 */
    b->x2 = b->x1;
    b->x1 = sample;

    /* shift y1 to y2, result to y1 */
    b->y2 = b->y1;
    b->y1 = result;

    return result;
}

/* sets up a BiQuad Filter */
biquad *BiQuad_new(int type, smp_type dbGain, smp_type freq,
                   smp_type srate, smp_type bandwidth)
{
    biquad *b;
    smp_type A, omega, sn, cs, alpha, beta;
    smp_type a0, a1, a2, b0, b1, b2;

    b = (biquad*)malloc(sizeof(biquad));
    if (b == NULL)
        return NULL;

    /* setup variables */
    A = x_pow( 10.0f, dbGain / 40.0f );
    omega = 2.0f * M_PI * freq / srate;
    sn = x_sin( omega );
    cs = x_cos( omega );
    alpha = sn * sinh( M_LN2 / 2.0f * bandwidth * omega / sn );
    beta = x_sqrt( A + A );

    switch (type) {
    case LPF:
        b0 = (1 - cs) /2;
        b1 = 1 - cs;
        b2 = (1 - cs) /2;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case HPF:
        b0 = (1 + cs) /2;
        b1 = -(1 + cs);
        b2 = (1 + cs) /2;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case BPF:
        b0 = alpha;
        b1 = 0;
        b2 = -alpha;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case NOTCH:
        b0 = 1;
        b1 = -2 * cs;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case PEQ:
        b0 = 1 + (alpha * A);
        b1 = -2 * cs;
        b2 = 1 - (alpha * A);
        a0 = 1 + (alpha /A);
        a1 = -2 * cs;
        a2 = 1 - (alpha /A);
        break;
    case LSH:
        b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
        b1 = 2 * A * ((A - 1) - (A + 1) * cs);
        b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
        a0 = (A + 1) + (A - 1) * cs + beta * sn;
        a1 = -2 * ((A - 1) + (A + 1) * cs);
        a2 = (A + 1) + (A - 1) * cs - beta * sn;
        break;
    case HSH:
        b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
        a0 = (A + 1) - (A - 1) * cs + beta * sn;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - beta * sn;
        break;
    default:
        free(b);
        return NULL;
    }

    /* precompute the coefficients */
    b->a0 = b0 /a0;
    b->a1 = b1 /a0;
    b->a2 = b2 /a0;
    b->a3 = a1 /a0;
    b->a4 = a2 /a0;

    /* zero initial samples */
    b->x1 = b->x2 = 0;
    b->y1 = b->y2 = 0;

    return b;
}

struct bi_params
{
    int Type;
    f32 Gain;
    f32 Frequency;
    f32 SampleRate;
    f32 Bandwidth;
};

bi_params g_BiParams =
{
    NOTCH,
    1.0f,
    4000.0f,
    22050.0f,
    3.0f
};

bi_params g_BiParamsCopy = {0};

void CombFilter( s32* pInL, s32* pInR, s32 nSamples )
{
/*
    s32 i;

    s32 CombSize = g_CombSize;
    if( CombSize > MAX_COMB_SIZE )
        CombSize = MAX_COMB_SIZE;

    // Apply input to combo filter
    s32 Index = g_CombIndexIn;
    for( i=0 ; i<nSamples ; i++ )
    {
        g_CombBufferL[Index] = ((g_CombBufferL[Index] * g_CombFactor) >> 16) + pInL[i];
        g_CombBufferR[Index] = ((g_CombBufferR[Index] * g_CombFactor) >> 16) + pInR[i];
        if( ++Index >= CombSize )
            Index = 0;
    }
    g_CombIndexIn = Index;

    // Copy output from comb filter
    Index = g_CombIndexOut;
    for( i=0 ; i<nSamples ; i++ )
    {
        pInL[i] = g_CombBufferL[Index];
        pInR[i] = g_CombBufferR[Index];
        if( ++Index >= CombSize )
            Index = 0;
    }
    g_CombIndexOut = Index;
*/

    static biquad* pBL = NULL;
    static biquad* pBR = NULL;

    if( memcmp( &g_BiParams, &g_BiParamsCopy, sizeof(g_BiParams) ) != 0 )
    {
        g_BiParamsCopy = g_BiParams;
        free( pBL );
        free( pBR );
        pBL = BiQuad_new( g_BiParams.Type, g_BiParams.Gain, g_BiParams.Frequency, g_BiParams.SampleRate, g_BiParams.Bandwidth );
        pBR = BiQuad_new( g_BiParams.Type, g_BiParams.Gain, g_BiParams.Frequency, g_BiParams.SampleRate, g_BiParams.Bandwidth );
    }

    for( s32 i=0 ; i<nSamples ; i++ )
    {
        *pInL = (s32)BiQuad( smp_type(*pInL), pBL );
        *pInR = (s32)BiQuad( smp_type(*pInR), pBR );
        pInL++;
        pInR++;
    }
}

void IAL_MixFrame( void )
{
    static DWORD    WriteCursor = 0;
    s32             iChannel;
    s32             i;
    s32             nSamples;
    HRESULT         hr;

    // Exit if we have no D3D buffer or system is not initialized
    if (!s_Initialized || s_lpsb8 == NULL)
        return;

    // Read the play and write cursors
    DWORD PosPlay;
    DWORD PosWrite;
    if( s_lpsb8 )
        hr = s_lpsb8->GetCurrentPosition( &PosPlay, &PosWrite );

    // Determine the number of bytes we want to lock and update
    if( WriteCursor > PosPlay )
        PosPlay += s_BufferBytes;
    ASSERT( PosPlay >= WriteCursor );
    DWORD   WriteBytes = (PosPlay - WriteCursor) & ~3;

    // Lock the buffer
    s16*    p1;
    DWORD   c1;
    s16*    p2;
    DWORD   c2;

    if( s_lpsb8 )
    {
        hr = s_lpsb8->Lock( WriteCursor, WriteBytes, (LPVOID*)&p1, &c1, (LPVOID*)&p2, &c2, 0 );
        if( DSERR_BUFFERLOST == hr )
        {
            // Buffer was lost, attempt a restore
            hr = s_lpsb8->Restore();
            if( SUCCEEDED(hr) )
            {
                hr = s_lpsb8->Lock( WriteCursor, WriteBytes, (LPVOID*)&p1, &c1, (LPVOID*)&p2, &c2, 0 );
                CLOG_MESSAGE( LOGGING_ENABLED, "MixFrame", "Restore Buffer" );
            }
            else
            {
                CLOG_MESSAGE( LOGGING_ENABLED, "MixFrame", "Failed to restore buffer" );
                return;
            }
        }
        if( SUCCEEDED(hr) )
        {
            // Update our internal write cursor
            WriteCursor += c1+c2;
            if( WriteCursor >= s_BufferBytes )
                WriteCursor -= s_BufferBytes;

            // Verify that the byte counts are multiples of 4 bytes
            ASSERT( (c1 & 3) == 0 );
            ASSERT( (c2 & 3) == 0 );

            // Calculate number of samples we need to mix
            nSamples = (c1+c2) / 4;

            // Samples to mix?
            if( nSamples > 0 )
            {
                // Clear the mix buffer
                ZeroMemory( IAL_MixL, nSamples*sizeof(s32) );
                ZeroMemory( IAL_MixR, nSamples*sizeof(s32) );

                // Mix channels into the buffer
                for( iChannel=0 ; iChannel<IAL_MAX_CHANNELS ; iChannel++ )
                {
                    if( IAL_Channels[iChannel].State == IAL_PLAY )
                        IAL_MixChannel( &IAL_Channels[iChannel], IAL_MixL, IAL_MixR, nSamples );
                }

                // Output the mix to 16 bit buffers with clamping
                IAL_OutputAmplitude[0] = 0;
                IAL_OutputAmplitude[1] = 0;
                for( i=0 ; i<nSamples ; i++ )
                {
                    s32 s;

                    s = IAL_MixL[i];
                    if( s < -32768 ) s = -32768;
                    if( s >  32767 ) s =  32767;
                    IAL_Out[i*2] = (s16)s;
                    s = x_abs( s );
                    IAL_OutputAmplitude[0] = max( IAL_OutputAmplitude[0], s );

                    s = IAL_MixR[i];
                    if( s < -32768 ) s = -32768;
                    if( s >  32767 ) s =  32767;
                    IAL_Out[i*2+1] = (s16)s;
                    s = x_abs( s );
                    IAL_OutputAmplitude[1] = max( IAL_OutputAmplitude[1], s );
                }

                // Copy the samples into the buffer
                if (p1 && c1)
                {
                    x_memcpy( p1, &IAL_Out[0   ], c1 );
                }
                if (p2 && c2)
                {
                    x_memcpy( p2, &IAL_Out[c1/2], c2 );
                }
            }

            // Unlock the buffer
            if ( s_lpsb8 )
            {
                hr = s_lpsb8->Unlock(p1, c1, p2, c2);
            }
        }
    }
}

//==============================================================================

BOOL IAL_CreateBuffer( LPDIRECTSOUND8 lpDirectSound, LPDIRECTSOUNDBUFFER* lplpDsb )
{
    PCMWAVEFORMAT   pcmwf;
    DSBUFFERDESC    dsbdesc;
    HRESULT         hr;

    // Check for error and exit
    *lplpDsb = NULL;
    if( lpDirectSound == NULL )
        return FALSE;

    // Set up wave format structure
    memset( &pcmwf, 0, sizeof(PCMWAVEFORMAT) );
    pcmwf.wf.wFormatTag      = WAVE_FORMAT_PCM;
    pcmwf.wf.nChannels       = 2;
    pcmwf.wf.nSamplesPerSec  = IAL_SAMPLE_RATE;
    pcmwf.wf.nBlockAlign     = 4;
    pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample     = 16;

    // Set up DSBUFFERDESC structure
    memset( &dsbdesc, 0, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags       = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    dsbdesc.dwBufferBytes = IAL_SAMPLES_PER_FRAME * 4 * pcmwf.wf.nBlockAlign;
    dsbdesc.lpwfxFormat   = (LPWAVEFORMATEX)&pcmwf;

    s_BufferBytes = dsbdesc.dwBufferBytes;

    // Create buffer
    hr = lpDirectSound->CreateSoundBuffer( &dsbdesc, lplpDsb, NULL );
    if SUCCEEDED(hr)
    {
        // IDirectSoundBuffer interface is in *lplpDsb
        // Use QueryInterface to obtain IDirectSoundBuffer8
        return TRUE;
    }
    else
    {
        // Failed
        *lplpDsb = NULL;
        return FALSE;
    }
}

//==============================================================================

//void IAL_MixThread( void )
DWORD _stdcall IAL_MixThread( void* )
{
    HRESULT hr;
    
    if (!s_Initialized)
        return 0;
        
    // Create the DirectSound device
    hr = DirectSoundCreate8( NULL, &s_lpds, NULL );
    if( SUCCEEDED(hr) && s_lpds )
    {
        hr = s_lpds->SetCooperativeLevel( IAL_hWnd, DSSCL_NORMAL );

        // Create the secondary buffer
        if( IAL_CreateBuffer( s_lpds, &s_lpsb ) )
        {
            hr = s_lpsb->QueryInterface( IID_IDirectSoundBuffer8, (LPVOID*)&s_lpsb8 );
            if( SUCCEEDED(hr) && s_lpsb8 )
            {
                // Start the secondary buffer playing
                hr = s_lpsb8->SetVolume( DSBVOLUME_MAX );
                hr = s_lpsb8->SetPan   ( DSBPAN_CENTER );
                hr = s_lpsb8->Play     ( 0, 0, DSBPLAY_LOOPING );
            }
            else
            {
//                CLOG_ERROR( LOGGING_ENABLED, "IAL", "Failed to create DirectSoundBuffer8 interface" );
            }
        }
        else
        {
//            CLOG_ERROR( LOGGING_ENABLED, "IAL", "Failed to create DirectSound secondary buffer" );
        }
    }
    else
    {
//        CLOG_ERROR( LOGGING_ENABLED, "IAL", "Failed to create DirectSound device" );
    }

    // Loop forever
    while( !s_KillMixer )
    {
        Sleep( IAL_FRAME_TIME_MS );

        if (s_Initialized)
        {
            IAL_GetMutex();
            IAL_MixFrame();

            if (s_lpsb)
            {
                s32 Decibels = -10000;
                if (s_SystemVolume > 0.00001f)
                    Decibels = (s32)(2000*log10(s_SystemVolume));
                s_lpsb->SetVolume(Decibels);
            }

            IAL_ReleaseMutex();
        }
    }

    if( s_lpsb8 )
    {
        // Stop the secondary buffer
        hr = s_lpsb8->Stop();
    }

    if( s_lpsb )
    {
        // Release the secondary buffer
        hr = s_lpsb->Release();

        // Release the DirectSound device
        hr = s_lpds->Release();
    }

    return 0;
}

//==============================================================================

xbool IAL_Init( HWND hWnd )
{
    ASSERT( !s_Initialized );

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_Init( 0x%08X )", hWnd );

    // Save hWnd
    IAL_hWnd = hWnd;

    // Reset data
    x_memset( &IAL_Channels, 0, sizeof(IAL_Channels) );

    // Initialize the critical section object
    InitializeCriticalSection( &IAL_CriticalSection );

    // Create the mixer thread & set it's priority
    HANDLE hThread = CreateThread( NULL, 0, &IAL_MixThread, NULL, 0, NULL );
    SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );

//    IAL_pMixThread = new xthread( IAL_MixThread, "IAL_MixThread", 32768, THREAD_PRIORITY_TIME_CRITICAL );

    // Return the initialized state
    s_Initialized = TRUE;
    return s_Initialized;
}

//==============================================================================

void IAL_Kill( void )
{
    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_Kill()" );

    s_KillMixer = TRUE;
    x_DelayThread( 100 );

    delete IAL_pMixThread;

    DeleteCriticalSection( &IAL_CriticalSection );

    s_Initialized = FALSE;
}

//==============================================================================

void IAL_SetSystemVolume( f32 Volume )
{
    s_SystemVolume = Volume;
}

//==============================================================================

ial_hchannel IAL_allocate_channel( void )
{
    IAL_GetMutex();

    for( s32 i=0 ; i<IAL_MAX_CHANNELS ; i++ )
    {
        if( !IAL_Channels[i].Allocated )
        {
            IAL_Channels[i].Allocated = TRUE;
            IAL_Channels[i].Sequence++;
            IAL_ReleaseMutex();
            CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_allocate_channel() = %d", i + 1 );
            return( (IAL_Channels[i].Sequence << 16) + i + 1 );
        }
    }

    IAL_ReleaseMutex();

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_allocate_channel() = %d", 0 );

    return 0;
}

//==============================================================================

s32 IAL_hChannelToIndex( ial_hchannel hChannel )
{
    s32 Index       = (hChannel & 65535) - 1;
    s32 Sequence    = hChannel >> 16;

    if( Index < 0 )
        Index = IAL_MAX_CHANNELS;
    else if( Index >= IAL_MAX_CHANNELS )
        Index = IAL_MAX_CHANNELS;
    else if( IAL_Channels[Index].Sequence != Sequence )
        Index = IAL_MAX_CHANNELS;

    return Index;
}

//==============================================================================

void IAL_release_channel ( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;
  
    IAL_GetMutex();

    IAL_Channels[Index].State     = IAL_DONE;
    IAL_Channels[Index].Allocated = FALSE;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_release_channel() = %d", Index + 1 );

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_init_channel( ial_hchannel hChannel, void* pData, s32 nSamples, s32 LoopCount, s32 LoopStart, s32 LoopEnd,
                       ial_format Format, s32 SampleRate, f32 VolumeL, f32 VolumeR, f32 Pitch )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_init_channel( %d, 0x%08x, %d, %d, %d, %d, %d, %d, %f, %f, %f )", Index, (u32)pData, nSamples, LoopCount, LoopStart, LoopEnd, Format, SampleRate, VolumeL, VolumeR, Pitch );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    Channel.pData       = (s16*)pData;
    Channel.Cursor      = 0;
    Channel.Fraction    = 0;
    Channel.Looped      = (LoopCount != 0);
    Channel.LoopStart   = LoopStart;
    Channel.LoopEnd     = LoopEnd;
    Channel.SampleRate  = SampleRate;
    Channel.nSamples    = nSamples;
    Channel.State       = IAL_STOP;
    Channel.VolumeL     = VolumeL;
    Channel.VolumeR     = VolumeR;
    Channel.Pitch       = Pitch;

    s32 NewVolL = (s32)(32767.0f * MINMAX(-1.0f, VolumeL, 1.0f));
    s32 NewVolR = (s32)(32767.0f * MINMAX(-1.0f, VolumeR, 1.0f));

    Channel.MixedVolL   = NewVolL;
    Channel.MixedVolR   = NewVolR;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_start_channel( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_start_channel( %d )", Index );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    Channel.State = IAL_PLAY;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_stop_channel( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_stop_channel( %d )", Index );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    //if( Channel.State == IAL_PLAY )
        Channel.State = IAL_STOP;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_pause_channel( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_pause_channel( %d )", Index );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    if( Channel.State == IAL_PLAY )
        Channel.State = IAL_PAUSED;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_resume_channel( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_resume_channel( %d )", Index );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    if( Channel.State == IAL_PAUSED )
        Channel.State = IAL_PLAY;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_end_channel( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_end_channel( %d )", Index );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    Channel.State = IAL_DONE;

    IAL_ReleaseMutex();
}

//==============================================================================

ial_state IAL_channel_status( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return IAL_DONE;

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    ial_state State = Channel.State;

    IAL_ReleaseMutex();

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_channel_status( %d ) = %d", Index, State );

    return State;
}

//==============================================================================

s32 IAL_channel_position( ial_hchannel hChannel )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return 0;

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    s32 Position = Channel.Cursor;

    IAL_ReleaseMutex();

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_channel_position( %d )", Index, Position );

    return Position;
}

//==============================================================================

void IAL_set_channel_volume( ial_hchannel hChannel, f32 VolumeL, f32 VolumeR )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_set_channel_volume( %d, %f, %f )", Index, VolumeL, VolumeR );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    Channel.VolumeL = VolumeL;
    Channel.VolumeR = VolumeR;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_set_channel_pitch( ial_hchannel hChannel, f32 Pitch )
{
    s32 Index = IAL_hChannelToIndex( hChannel );
    if( Index >= IAL_MAX_CHANNELS )
        return;

    CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_set_channel_pitch( %d, %f )", Index, Pitch );

    ial_channel& Channel = IAL_Channels[Index];

    IAL_GetMutex();

    Channel.Pitch = Pitch;

    IAL_ReleaseMutex();
}

//==============================================================================

void IAL_set_channel_looped( ial_hchannel hChannel, bool Looped )
{
	s32 Index = IAL_hChannelToIndex( hChannel );
	if( Index >= IAL_MAX_CHANNELS )
		return;

	CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_set_channel_looped( %s )", Looped ? "TRUE" : "FALSE" );

	ial_channel& Channel = IAL_Channels[Index];

	IAL_GetMutex();

	Channel.Looped = Looped;

	IAL_ReleaseMutex();
}

//==============================================================================

s32 IAL_get_output_amplitude( s32 Channel )
{
    ASSERT( Channel >= 0 );
    ASSERT( Channel <= 1 );

    return IAL_OutputAmplitude[Channel];
}

//==============================================================================

void IAL_stop_loop( ial_hchannel hChannel, s32 nSamples )
{
	s32 Index = IAL_hChannelToIndex( hChannel );
	if( Index >= IAL_MAX_CHANNELS )
		return;

	CLOG_MESSAGE( LOGGING_ENABLED, "IAL", "IAL_stop_loop" );

	ial_channel& Channel = IAL_Channels[Index];

	IAL_GetMutex();

	if( Channel.Looped )
	{
		Channel.LoopEnd = nSamples;
	}

	IAL_ReleaseMutex();
}

