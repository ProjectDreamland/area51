/*============================================================================
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       decoder.h
 *  Content:    The main definitions for the XMV decoder.
 *
 ****************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The following two callbacks are used with XMVCreateDecoderForPackets to
 * supply the decoder with AV packet. 
 *
 * The implementation of these methods should follow this pseudocode:
 *
 * GetNextPacket:
 *
 *   if (Context->NextPacketHasFinishedLoading)
 *   {
 *       Context->PendingPacket  = Context->DecodingPacket;
 *       Context->DecodingPacket = Context->LoadingPacket;
 *       Context->LoadingPacket  = NULL;
 *
 *       *ppPacket = Context->DecodingPacket;
 *       *pOffsetToNextPacket = Context->DecodingPacketSize;
 *   }
 *   else
 *   {
 *       *ppPacket = NULL;
 *       *pOffsetToNextPacket = 0;
 *   }
 * 
 * ReleasePreviousPacket:
 *
 *   if (NextPacketSize)
 *   {
 *       Context->LoadingPacket = Context->PendingPacket;
 *       Context->PendingPacket = NULL;
 *
 *       LoadNextPacketFromOffset(Context->LoadingPacket, NextReadByteOffset, NextPacketSize);
 *   }
 *
 * The loading of the first packet must be done manually as the decoder
 * requires the first 4096 bytes to be loaded so it can create itself.  The
 * decoder will first call the pfnGetNextCallback callback to get the first
 * packet and will then call pfnReleasePreviousPacket when it is safe to start
 * loading the next packet over the previous packet.
 *
 * The memory for the packets must be allocated as contiguous memory via 
 * XPhysicalAlloc if the movie contains ADPCM or 5.1 audio.  
 *
 * During the normal process of decoding, the decoder will alternate calls to
 * pfnGetNextCallback and pfnReleasePreviousPacket, starting with a call to 
 * pfnGetNextCallback to obtain the first packet.  The only exception is if
 * XMVReset is called after pfnReleasePreviousPacket is called at the end
 * of the video with a NextPacketSize of zero.  In this case,
 * pfnReleasePreviousPacket will be called a second time with a nonzero
 * NextPacketSize before pfnGetNextCallback is called again.
 *
 * NextReadByteOffset starts at zero and is incremented by *pOffsetToNextPacket
 * each time pfnGetNextCallback is called.  XMVReset will cause this value to 
 * reset back to zero and looping (when implemented) will cause it to get reset
 * to known values...but the decoder will normally just increment down the file
 * in an orderly fashion.
 */

typedef HRESULT (CALLBACK *PFNXMVGETNEXTPACKET)(DWORD Context, void **ppPacket, DWORD *pOffsetToNextPacket);
typedef HRESULT (CALLBACK *PFNXMVRELEASEPREVIOUSPACKET)(DWORD Context, LONGLONG NextReadByteOffset, DWORD NextPacketSize);

/*
 * Just use an enum for the result of XMVGetNextFrame as there are only
 * 4 posibilities.
 */

typedef enum _XMVRESULT
{
    XMV_NOFRAME,
    XMV_NEWFRAME,
    XMV_ENDOFFILE,
    XMV_FAIL,

    XMV_FORCELONG = 0xFFFFFFFF
}
XMVRESULT;

/*
 * Decoder creation flags.
 */

// No flags.
#define XMVFLAG_NONE                        0x00000000

// Only useful with XMVGetNextFrame, this flag causes the decoder to return
// a frame with every call to this API regardless of timing except when
// we're waiting for data to load.  
//
#define XMVFLAG_UNSYNCHRONIZED_PLAYBACK     0x00000001

// Can be passed to either XMVCreateDecoder* or XMVPlay to have the decoder
// continuously loop through the entire video until XMVDecoder_TerminateLoop 
// or XMVDecoder_TerminatePlayback is called.
//
#define XMVFLAG_FULL_LOOP                   0x00000002

// When passed to XMVCreateDecoder, this flag tells the decoder to try to
// synchronize playback on the next vblank.  It is useful for obtaining frames
// early so they can be prepared and displayed for the next vertical blank.
//
#define XMVFLAG_SYNC_ON_NEXT_VBLANK         0x00000004

/* 
 * XMVAUDIO_DESC flags
 */

// Indicates whether this stream belongs to a 5.1 audio stream set and
// as which part.
//
#define XMVAUDIODESC_51_ADPCM                          0x00000007
#define XMVAUDIODESC_51_ADPCM_FRONT_LEFT_RIGHT         0x00000001
#define XMVAUDIODESC_51_ADPCM_CENTER_LOW_FREQUENCY     0x00000002
#define XMVAUDIODESC_51_ADPCM_REAR_LEFT_RIGHT          0x00000004

/*
 * Describes the general attributes of the XMV file.
 */

typedef struct _XMVVIDEO_DESC
{
    // The geometry of the video.  The surface that each frame is rendered
    // onto must be exactly this size.  If width and height are zero then 
    // there is no video stream in this file.
    //
    DWORD Width;
    DWORD Height;

    // The frame rate of the file.
    DWORD FramesPerSecond;

    // The number of audio streams encoded in this file.
    DWORD AudioStreamCount;
}
XMVVIDEO_DESC;

/*
 * The audio descriptor of the XMV files.
 */

typedef struct _XMVAUDIO_DESC
{
    // The WAVE_FORMAT tag that describes how the audio data in the stream is
    // encoded.  This can be either WAVE_FORMAT_PCM or WAVE_FORMAT_XBOX_ADPCM.
    //
    DWORD WaveFormat;

    // The number of channels in the audio stream.  Can be 1, 2, 4 or 6.
    DWORD ChannelCount;

    // The number of samples per second (Hz) in the audio stream.
    DWORD SamplesPerSecond;

    // The number of bits in each sample.  
    DWORD BitsPerSample;

    // XMVAUDIODESC flags
    DWORD Flags;
}
XMVAUDIO_DESC;

/*
 * The XMV decoder structure.
 */

typedef struct XMVDecoder XMVDecoder;

#ifdef __cplusplus

struct XMVDecoder
{
    //
    // See the commments on the non-member functions below for a description
    // of these helper-members functions.
    //

    void      CloseDecoder();
    
    void      GetVideoDescriptor(XMVVIDEO_DESC *pVideoDescriptor);
    void      GetAudioDescriptor(DWORD AudioStream, XMVAUDIO_DESC *pAudioDescriptor);

    HRESULT   EnableAudioStream(DWORD AudioStream, DWORD Flags, DSMIXBINS *pMixBins, IDirectSoundStream **ppStream);
    void      DisableAudioStream(DWORD AudioStream);
    void      GetAudioStream(DWORD AudioStream, IDirectSoundStream **ppStream);
    DWORD     GetSynchronizationStream();
    void      SetSynchronizationStream(DWORD AudioStream);
  
    void      Reset();

    HRESULT   Play(DWORD Flags, RECT *pRect);
    void      TerminateLoop();
    void      TerminatePlayback();
    void      TerminateImmediately();

    HRESULT   GetNextFrame(IDirect3DSurface8 *pSurface, XMVRESULT *pResult, DWORD *pTimeOfFrame);

    DWORD     GetTimeFromStart();
};

#endif __cplusplus

/*
 * Load a movie in a file into the decoder.  This will allocate space to hold
 * two packets in one virtual allocation.  It also allocates one more virtual 
 * allocation to hold internal decoder data.  The size of these allocations 
 * depends on the video.
 */

HRESULT __stdcall XMVDecoder_CreateDecoderForFile(DWORD Flags, LPCSTR szFileName, XMVDecoder **ppDecoder);

/* 
 * Loads a movie in a packet into the decoder.  Take a pointer to the first
 * 4096 bytes of the first packet which contains all of the header information
 * needed to finish setting up the decoder.  pfnGetNextPacket will be called
 * for the first packet when decoding starts.
 *
 * The packet must contain one complete packet as formatted by xmvtool in one
 * block of memory.  The output of xmvtool is formatted as follows:
 *
 * First packet:
 *    DWORD NextPacketSize  // The size of the next packet
 *    DWORD ThisPacketSize  // The size of this packet
 *    DWORD MaxPacketSize   // The size of the largest packet in the file
 *    ...packet data
 *
 * All other packets:
 *    DWORD NextPacketSize; // The size of the next packet in the file
 *    ...packet data
 *
 * All sizes include the above headers.
 *
 * The decoder calls pfnGetNextPacket after it finishes decoding
 * the final image from the current packet to get the next packet from which 
 * to decode.  The previous packet may still be in use and cannot be destroyed
 * until pfnReleaseLastPacket is called.  If the data is not ready, this 
 * API should return NULL.  pfnGetNextPacket will be called often until a 
 * non-null packet is returned at which point it will not be called again
 * until after pfnReleasePreviousPacket is called.
 *
 * The decoder calls pfnReleasePreviousPacket it is completely done with a 
 * packet.  The memory for this packet can then be safely reclaimed for other
 * uses.  This will happen after pfnGetNextPacket is called to obtain the next 
 * packet to decode from.
 *
 * There needs to be an overlap between the previous packet and the current 
 * packet to ensure that there is no hitch in the audio stream during playback.
 * pfnReleaseLastPacket will always be called after every successful
 * call to pfnGetNextPacket (one that doesn't return NULL) even if there is
 * no previous packet to be released...such as for the first frame.
 *
 * This routine allocates one virtual allocation that contains all internal
 * datastructures.
 */

HRESULT __stdcall XMVDecoder_CreateDecoderForPackets(DWORD Flags, void *pFirst4096, DWORD Context, PFNXMVGETNEXTPACKET pfnGetNextPacket, PFNXMVRELEASEPREVIOUSPACKET pfnReleasePreviousPacket, XMVDecoder **ppDecoder);   

/*
 * Destroy decoder, freeing all memory and closing any open input file.
 */

void __stdcall XMVDecoder_CloseDecoder(XMVDecoder *pDecoder);

/*
 * Get the general information about the file.
 */

void __stdcall XMVDecoder_GetVideoDescriptor(XMVDecoder *pDecoder, XMVVIDEO_DESC *pVideoDescriptor);

/*
 * Get information about a specific audio stream.
 */

void __stdcall XMVDecoder_GetAudioDescriptor(XMVDecoder *pDecoder, DWORD AudioStream, XMVAUDIO_DESC *pAudioDescriptor);

/*
 * Tell the decoder to play one of the audio tracks in the XMV file.  The 
 * caller can optionally get the IDirectSoundStream interface to manually
 * adjust the sound parameters.
 *
 * This API can be called multiple times for different streams to allow
 * them to be played at the same time.  No audio will play if at least one
 * stream is not enabled.  
 *
 * If the video output is not being synchronized on an audio stream then
 * the stream enabled by this call will be used for synchronization. 
 *
 * The Flags and pMixBins parameters are used when creating the stream.  
 * These can be safely set to zero and NULL, respectively.
 *
 * ppStream may be NULL if the caller does not want direct access to the
 * stream.
 *
 * A 5.1 stream compressed with ADPCM actually is encoded as three consecutive 
 * streams, the first for the front left-right channels, the second for the
 * center and low-frequency channels with the third containing the rear
 * left-right channels.  These will be indicate by the presence of one of the
 * XMVAUDIODESC_51_ADPCM flags in the XMVAUDIO_DESC structure returned from
 * XMVDecoder_GetAudioDescriptor.
 *
 * If this API is called on the first stream of an ADPCM 5.1 set (the front 
 * left-right stream) with no flags, NULL passed in for pMixBins and NULL 
 * passed in for ppStream then the decoder will automatically enable all three
 * streams, otherwise the caller needs to make sure that each stream is 
 * individually enabled.
 */

HRESULT __stdcall XMVDecoder_EnableAudioStream(XMVDecoder *pDecoder, DWORD AudioStream, DWORD Flags, DSMIXBINS *pMixBins, IDirectSoundStream **ppStream);

/*
 * Disables an audio stream.  
 *
 * If the audio stream being disabled is the current synchronization stream
 * then the video will play back unsynchronized until either another audio
 * stream is enabled or the synchronization is explicitly set to another
 * stream.
 */

void __stdcall XMVDecoder_DisableAudioStream(XMVDecoder *pDecoder, DWORD AudioStream);

/* 
 * Returns the audio stream or NULL if that stream is not enabled.
 */

void __stdcall XMVDecoder_GetAudioStream(XMVDecoder *pDecoder, DWORD AudioStream, IDirectSoundStream **ppStream);

/*
 * Returns the current synchronization stream or -1 if no audio stream is being
 * use to synchronize the video.
 */

DWORD __stdcall XMVDecoder_GetSynchronizationStream(XMVDecoder *pDecoder);

/*
 * Synchronize the video off of a specific audio stream.  The stream must
 * be enabled.
 *
 * Passing -1 as the audio stream will disable all synchronization.
 */

void __stdcall XMVDecoder_SetSynchronizationStream(XMVDecoder *pDecoder, DWORD AudioStream);

/*
 * Resets the movie playback back to the beginning.  This only resets the
 * internal datastructures and will reset back to the beginning of the file if
 * that is how we're opened.  
 */

HRESULT __stdcall XMVDecoder_Reset(XMVDecoder *pDecoder);

/*
 * Play an entire video stream, blocking until the video and audio portion
 * of the stream is complete.  This method creates a surface that is the
 * size of the video and plays it back on the screen.  It uses the D3D
 * overlay functionality so therefore can be run on background thead without
 * interfering with other D3D rendering APIs.
 *
 * The rectangle specifies what part of the screen to output the video on with
 * NULL meaning to render to the full screen.
 *
 * This API is one of the two ways to show a movie.  Either it should be used
 * or XMVDecoder_GetNextFrame, but not both.
 *
 * Passing in XMVPLAY_LOOP as Flags will cause the movie to be looped until
 * XMVDecoder_TerminateLoop or XMVDecoder_TerminatePlayback is called.
 *
 * This API cannot be used with a movie opened with 
 * XMVFLAG_UNSYNCHRONIZED_PLAYBACK.
 *
 * D3D must have been initialized before this API is called.
 */

HRESULT __stdcall XMVDecoder_Play(XMVDecoder *pDecoder, DWORD Flags, RECT *pRect);

/*
 * Tell the decoder to exit at the end of the current loop.  This can be used 
 * to nicely break out of a playback loop.
 *
 * This routine can be safely called from a different thread than
 * the decoder is running on.
 */

void __stdcall XMVDecoder_TerminateLoop(XMVDecoder *pDecoder);

/*
 * Tell the decoder to end the playback as soon as possible.  This may not
 * happen immediately.
 *
 * This routine can be safely called from a different thread than
 * the decoder is running on.
 */

void __stdcall XMVDecoder_TerminatePlayback(XMVDecoder *pDecoder);

/*
 * Terminate the playback of the movie immediately.  This routine must be 
 * called on the same thread that the decoder is running on and therefore
 * cannot be used to terminate Play.
 */

void __stdcall XMVDecoder_TerminateImmediately(XMVDecoder *pDecoder);

/*
 * Get the next frame to display.
 *
 * This method returns four possible values from the pResult out
 * parameter:
 *
 *   XMV_NOFRAME - There is no new data and the passed-in surface
 *       is unchanged.
 *
 *   XMV_NEWFRAME - The surface contains a new frame that needs to be
 *       displayed.
 *
 *   XMV_ENDOFFILE - The movie is over, throw your trash away and go home.
 *
 *   XMV_FAIL - Some major catastrophy happened that prevents the video from
 *       being decoded any further.  This only happens with major data 
 *       corruption or some kind of read error and will always be accompanied
 *       by a failure HRESULT.
 *
 * The Width and Height of the surface must match that of the video.
 *
 * pSurface can be NULL if you do not want any video.
 *
 * pTimeOfFrame can be NULL if you are allowing the library to do
 * synchronization, otherwise it returns the number of milliseconds
 * after the start of the movie that the returned frame (if any) should
 * be displayed at, otherwise the value is unchanged.  This value is
 * a DWORD and will wrap if the video played for more than 49.7 days.
 *
 * This API is one of the two ways to show a movie.  Either it should be used
 * or XMVDecoder_Play, but not both.
 *
 * If Audio is being played while this API is being used then
 * DirectSoundDoWork must be called occasionally.
 */

HRESULT __stdcall XMVDecoder_GetNextFrame(XMVDecoder *pDecoder, IDirect3DSurface8 *pSurface, XMVRESULT *pResult, DWORD *pTimeOfFrame);

/*
 * Return the number of milliseconds since the this XMV file started playing.
 */

DWORD __stdcall XMVDecoder_GetTimeFromStart(XMVDecoder *pDecoder);

/*
 * Inlined implementation of the methods fo rthe XMVDecoder class
 */

#ifdef __cplusplus

inline void      XMVDecoder::CloseDecoder() { XMVDecoder_CloseDecoder(this); }

inline void      XMVDecoder::GetVideoDescriptor(XMVVIDEO_DESC *pVideoDescriptor) { XMVDecoder_GetVideoDescriptor(this, pVideoDescriptor); }
inline void      XMVDecoder::GetAudioDescriptor(DWORD AudioStream, XMVAUDIO_DESC *pAudioDescriptor) { XMVDecoder_GetAudioDescriptor(this, AudioStream, pAudioDescriptor); }

inline HRESULT   XMVDecoder::EnableAudioStream(DWORD AudioStream, DWORD Flags, DSMIXBINS *pMixBins, IDirectSoundStream **ppStream) { return XMVDecoder_EnableAudioStream(this, AudioStream, Flags, pMixBins, ppStream); }
inline void      XMVDecoder::DisableAudioStream(DWORD AudioStream) { XMVDecoder_DisableAudioStream(this, AudioStream); }
inline void      XMVDecoder::GetAudioStream(DWORD AudioStream, IDirectSoundStream **ppStream) { XMVDecoder_GetAudioStream(this, AudioStream, ppStream); }
inline DWORD     XMVDecoder::GetSynchronizationStream() { return XMVDecoder_GetSynchronizationStream(this); }
inline void      XMVDecoder::SetSynchronizationStream(DWORD AudioStream) { XMVDecoder_SetSynchronizationStream(this, AudioStream); }

inline void      XMVDecoder::Reset() { XMVDecoder_Reset(this); }

inline HRESULT   XMVDecoder::Play(DWORD Flags, RECT *pRect) { return XMVDecoder_Play(this, Flags, pRect); }

inline void      XMVDecoder::TerminateLoop() { XMVDecoder_TerminateLoop(this); }
inline void      XMVDecoder::TerminatePlayback() { XMVDecoder_TerminatePlayback(this); }
inline void      XMVDecoder::TerminateImmediately() { XMVDecoder_TerminateImmediately(this); }

inline HRESULT   XMVDecoder::GetNextFrame(IDirect3DSurface8 *pSurface, XMVRESULT *pxr, DWORD *pTimeOfFrame) { return XMVDecoder_GetNextFrame(this, pSurface, pxr, pTimeOfFrame); }

inline DWORD     XMVDecoder::GetTimeFromStart() { return XMVDecoder_GetTimeFromStart(this); }

#endif __cplusplus

#ifdef __cplusplus
}
#endif
