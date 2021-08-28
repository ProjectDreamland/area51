/*==========================================================================;
 *
 *  xhv.h -- This module defines the XBox High-Level Voice APIs
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#ifndef __XHV__
#define __XHV__

#ifndef __XONLINE__
#error You need to include xonline.h first!
#endif 

#ifdef __XVOICE__
#error High-level and low-level voice APIs cannot be used in the same time!
#endif 


#pragma warning( push )
#pragma warning( disable : 4100 ) // unreferenced formal parameter

#ifndef XHVINLINE
#define XHVINLINE __forceinline
#endif

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//
//  Typedefs, Structures & Consts
//
//-----------------------------------------------------------------------------
#define XHV_MAX_REMOTE_TALKERS                  30
#define XHV_MAX_SPEECH_RECOGNITION_RESULTS      1
#define XHV_VOICE_MASK_PARAM_DISABLED          (-1.0f)

#define XHV_MAX_LOCAL_TALKERS                   4
#define XHV_PLAYBACK_TO_SPEAKERS                XHV_MAX_LOCAL_TALKERS
#define XHV_MAX_PLAYBACK_TARGETS               (XHV_MAX_LOCAL_TALKERS + 1)
#define XHV_LOCAL_TALKER_MAX_MIXBINS            1
#define XHV_SPEAKERS_MAX_MIXBINS                4

#define XHV_MAX_PLAYBACK_STREAMS                16
#define XHV_PLAYBACK_PRIORITY_MAX               0
#define XHV_PLAYBACK_PRIORITY_MIN               0xFFFF
#define XHV_PLAYBACK_PRIORITY_NEVER             0xFFFFFFFF

//
// Raw XHV voice packet data constants
//
#define XHV_VOICE_FRAMETIME                     20                            // 20 ms frames
#define XHV_SAMPLE_RATE                         16000                         // 16 kHz
#define XHV_BYTES_PER_SAMPLE                    2                             //  2 bytes per sample
#define XHV_NUMBER_OF_SAMPLES                  (XHV_VOICE_FRAMETIME * XHV_SAMPLE_RATE / 1000)
#define XHV_PCM_FRAMELENGTH                    (XHV_NUMBER_OF_SAMPLES * XHV_BYTES_PER_SAMPLE)

//
// Voice Mail data constants
//
#define XHV_MAX_VOICEMAIL_DURATION_MS           15000 // 15 seconds
#define XHV_MIN_VOICEMAIL_DURATION_MS           1000  // 1 second
#define XHV_WMAVOICE_MAXBITSPERSEC              5000  //bps
#define XHV_WMAVOICE_MAXBYTESPERSEC            (XHV_WMAVOICE_MAXBITSPERSEC / 8)
#define XHV_WMAVOICE_WAVHEADER_SIZE             92

#define XHVGetVoiceMailBufferSize(DurationMS)    (XHV_WMAVOICE_WAVHEADER_SIZE + 2 * DurationMS * XHV_WMAVOICE_MAXBYTESPERSEC / 1000) // Duration in ms

// Predefined voice masks            fSpecEnergyWeight               fPitchScale                    fWhisperValue                  fRoboticValue
#define XHV_VOICE_MASK_NONE         { XHV_VOICE_MASK_PARAM_DISABLED, XHV_VOICE_MASK_PARAM_DISABLED, XHV_VOICE_MASK_PARAM_DISABLED, XHV_VOICE_MASK_PARAM_DISABLED }
#define XHV_VOICE_MASK_ANONYMOUS    { 0.95f,                         0.50f,                         0.35f,                         XHV_VOICE_MASK_PARAM_DISABLED }

typedef DWORD                                         XHV_SR_VOCAB_SELECTION;
typedef PVOID                                         XHV_PROCESSING_MODE;
typedef DWORD                                         XHV_PLAYBACK_PRIORITY;

extern const XHV_PROCESSING_MODE                     _xhv_inactive_mode;
extern const XHV_PROCESSING_MODE                     _xhv_loopback_mode;
extern const XHV_PROCESSING_MODE                     _xhv_voicechat_mode;
extern const XHV_PROCESSING_MODE                     _xhv_sr_mode;
extern const XHV_PROCESSING_MODE                     _xhv_voicemail_mode;

#define XHV_INACTIVE_MODE                            _xhv_inactive_mode
#define XHV_LOOPBACK_MODE                            _xhv_loopback_mode
#define XHV_VOICECHAT_MODE                           _xhv_voicechat_mode
#define XHV_SR_MODE                                  _xhv_sr_mode
#define XHV_VOICEMAIL_MODE                           _xhv_voicemail_mode

typedef enum _XHV_VOICE_COMMUNICATOR_STATUS
{
    XHV_VOICE_COMMUNICATOR_STATUS_INSERTED = 0,
    XHV_VOICE_COMMUNICATOR_STATUS_REMOVED
    
} XHV_VOICE_COMMUNICATOR_STATUS, *PXHV_VOICE_COMMUNICATOR_STATUS;


#pragma pack(push, 4)

typedef struct _XHV_VOICE_MASK
{
     FLOAT                                fSpecEnergyWeight;
     FLOAT                                fPitchScale;
     FLOAT                                fWhisperValue;
     FLOAT                                fRoboticValue;

} XHV_VOICE_MASK, *PXHV_VOICE_MASK;

typedef struct _XHV_RUNTIME_PARAMS 
{
    DWORD                                 dwMaxRemoteTalkers;
    DWORD                                 dwMaxLocalTalkers;
    DWORD                                 dwMaxCompressedBuffers;
    DWORD                                 dwFlags;
    DSEFFECTIMAGEDESC                    *pEffectImageDesc;
    DWORD                                 dwEffectsStartIndex;
    DWORD                                 dwOutOfSyncThreshold;
    BOOL                                  bCustomVADProvided;
    BOOL                                  bHeadphoneAlwaysOn;
 
} XHV_RUNTIME_PARAMS, *PXHV_RUNTIME_PARAMS;


typedef struct _XHV_LOCAL_TALKER_STATUS
{
    XHV_VOICE_COMMUNICATOR_STATUS         communicatorStatus;
    BOOL                                  bIsTalking;
    
} XHV_LOCAL_TALKER_STATUS, *PXHV_LOCAL_TALKER_STATUS;

typedef struct _XHV_SR_ITEM
{
    DWORD                                 dwWordID;
    FLOAT                                 fConfidence;
    
} XHV_SR_ITEM, *PXHV_SR_ITEM;

#pragma pack(pop)

#pragma pack (push, 1)

typedef struct _XHV_CODEC_HEADER
{
    WORD                                  bMsgNo :  4;
    WORD                                  wSeqNo : 12;

} XHV_CODEC_HEADER, *PXHV_CODEC_HEADER;

#pragma pack (pop)

//-----------------------------------------------------------------------------
//
//  Interfaces
//
//-----------------------------------------------------------------------------

typedef struct XHVEngine                  XHVEngine;

//
// Compatibility typedefs.
//
#define IXHVEngine                        XHVEngine

//
// Pointer typedefs.
//
typedef struct XHVEngine                 *LPXHVENGINE, *PXHVENGINE;

//
//  ITitleXHV interface
//
//-----------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE ITitleXHV

DECLARE_INTERFACE(ITitleXHV)
{
     STDMETHOD(LocalChatDataReady)(       THIS_   IN  DWORD                           dwLocalPort,
                                                  IN  DWORD                           dwSize,
                                                  IN  VOID                           *pData ) 
#if defined(__cplusplus)                                                  
                                                  {
                                                      return(E_NOTIMPL);
                                                  }
# else
                                                  PURE;
#endif                                                                                                    

     STDMETHOD(CommunicatorStatusUpdate)(THIS_    IN  DWORD                           dwLocalPort,
                                                  IN  XHV_VOICE_COMMUNICATOR_STATUS   communicatorStatus ) 
#if defined(__cplusplus)                                                  
                                                  {
                                                      return(E_NOTIMPL);
                                                  }
# else
                                                  PURE;
#endif                                                                                                    

     STDMETHOD(SpeechRecognized)(         THIS_   IN  DWORD                           dwLocalPort,
                                                  IN  XHV_SR_ITEM                    *pSRArray,
                                                  IN  DWORD                           dwSRItemsCount )
#if defined(__cplusplus)                                                  
                                                  {
                                                      return(E_NOTIMPL);
                                                  }
# else
                                                  PURE;
#endif                                                                                                    

     STDMETHOD(MicrophoneRawDataReady)(   THIS_   IN  DWORD                           dwLocalPort,
                                                  IN  DWORD                           dwSize,
                                                  IN  VOID                           *pData,
                                                  OUT BOOL                           *pVoiceDetected ) 
#if defined(__cplusplus)                                                  
                                                  {
                                                      return(E_NOTIMPL);
                                                  }
# else
                                                  PURE;
#endif   

     STDMETHOD(VoiceMailDataReady)(       THIS_   IN  DWORD                           dwLocalPort,
                                                  IN  DWORD                           dwDuration,
                                                  IN  DWORD                           dwSize ) 
#if defined(__cplusplus)                                                  
                                                  {
                                                      return(E_NOTIMPL);
                                                  }
# else
                                                  PURE;
#endif   
 
     STDMETHOD(VoiceMailStopped)(         THIS_   IN  DWORD                           dwLocalPort) 
#if defined(__cplusplus)                                                  
                                                  {
                                                      return(E_NOTIMPL);
                                                  }
# else
                                                  PURE;
#endif   

};

#if defined(__cplusplus) && !defined(CINTERFACE)

#define ITitleXHV_LocalChatDataReady(p, a, b, c)        p->LocalChatDataReady(a, b, c)
#define ITitleXHV_CommunicatorStatusUpdate(p, a, b)     p->CommunicatorStatusUpdate(a, b)
#define ITitleXHV_SpeechRecognized(p, a, b, c)          p->SpeechRecognized(a, b, c)
#define ITitleXHV_MicrophoneRawDataReady(p, a, b, c, d) p->MicrophoneRawDataReady(a, b, c, d)
#define ITitleXHV_VoiceMailDataReady(p, a, b, c)        p->VoiceMailDataReady(a, b, c)
#define ITitleXHV_VoiceMailStopped(p, a)                p->VoiceMailStopped(a)

#else // defined(__cplusplus) && !defined(CINTERFACE)

#define ITitleXHV_LocalChatDataReady(p, a, b, c)        p->lpVtbl->LocalChatDataReady(p, a, b, c)
#define ITitleXHV_CommunicatorStatusUpdate(p, a, b)     p->lpVtbl->CommunicatorStatusUpdate(p, a, b)
#define ITitleXHV_SpeechRecognized(p, a, b, c)          p->lpVtbl->SpeechRecognized(p, a, b, c)
#define ITitleXHV_MicrophoneRawDataReady(p, a, b, c, d) p->lpVtbl->MicrophoneRawDataReady(p, a, b, c, d)
#define ITitleXHV_VoiceMailDataReady(p, a, b, c)        p->lpVtbl->VoiceMailDataReady(p, a, b, c)
#define ITitleXHV_VoiceMailStopped(p, a)                p->lpVtbl->VoiceMailStopped(p, a)

#endif // defined(__cplusplus) && !defined(CINTERFACE)

typedef struct ITitleXHV                       *LPTITLEXHV, *PTITLEXHV;

//-----------------------------------------------------------------------------
// NOTE: The C version of the methods for all of the interfaces
//       are named "<interfacename>_<method name>" and have an
//       explicit pointer to the interface as the first parameter.
//       The actual definition of these methods is at the end
//       of each interface section.
//-----------------------------------------------------------------------------

//
//  XHVEngine, IXHVEngine interface
//
//-----------------------------------------------------------------------------

#ifdef __cplusplus

struct XHVEngine
{
    ULONG   WINAPI AddRef();
    
    ULONG   WINAPI Release();
    
    HRESULT WINAPI EnableProcessingMode(           IN XHV_PROCESSING_MODE                 processingMode);
    
    HRESULT WINAPI SetCallbackInterface(           IN ITitleXHV                          *pITitleXHV);
    
    HRESULT WINAPI GetCallbackInterface(           OUT ITitleXHV                        **ppITitleXHV);
    
    HRESULT WINAPI DoWork();
      
    HRESULT WINAPI RegisterLocalTalker(            IN  DWORD                              dwLocalPort); 
    
    HRESULT WINAPI UnregisterLocalTalker(          IN  DWORD                              dwLocalPort); 
    
    HRESULT WINAPI SetProcessingMode(              IN  DWORD                              dwLocalPort,
                                                   IN  XHV_PROCESSING_MODE                processingMode);
                                                   
    HRESULT WINAPI GetProcessingMode(              IN  DWORD                              dwLocalPort,
                                                   IN  XHV_PROCESSING_MODE               *pProcessingMode);                                                   
                                      
    HRESULT WINAPI SetVoiceMask(                   IN  DWORD                              dwLocalPort,
                                                   IN  const XHV_VOICE_MASK              *pVoiceMask); 
    
    HRESULT WINAPI GetLocalTalkerStatus(           IN  DWORD                              dwLocalPort,
                                                   OUT XHV_LOCAL_TALKER_STATUS           *pLocalTalkerStatus); 
        
    HRESULT WINAPI RegisterRemoteTalker(           IN  XUID                               xuidRemoteTalker); 
    
    HRESULT WINAPI UnregisterRemoteTalker(         IN  XUID                               xuidRemoteTalker); 
    
    HRESULT WINAPI GetRemoteTalkers(               OUT DWORD                             *pdwRemoteTalkersCount,
                                                   OUT XUID                              *pxuidRemoteTalkers); 
    
    HRESULT WINAPI SubmitIncomingVoicePacket(      IN XUID                                xuidRemoteTalker, 
                                                   IN VOID                               *pvData,
                                                   IN DWORD                               dwSize); 
                                  
    BOOL    WINAPI IsTalking(                      IN  XUID                               xuidRemoteTalker); 
                                                   
    HRESULT WINAPI SetMixBinMapping(               IN  XUID                               xuidRemoteTalker, 
                                                   IN  DWORD                              dwLocalPort,
                                                   IN  const DSMIXBINS                   *pMixBins);
                                                   
    HRESULT WINAPI SetMixBinVolumes(               IN  XUID                               xuidRemoteTalker, 
                                                   IN  const DSMIXBINS                   *pMixBins);
                        
    HRESULT WINAPI SetPlaybackPriority(            IN  XUID                               xuidRemoteTalker, 
                                                   IN  DWORD                              dwLocalPort, 
                                                   IN  XHV_PLAYBACK_PRIORITY              playbackPriority );

    HRESULT WINAPI SetMaxPlaybackStreamsCount(     IN  DWORD                              dwStreamsCount);
     
    HRESULT WINAPI RegisterSpeechRecognitionBank(  IN  VOID                              *pvSRBank); 
   
    HRESULT WINAPI SelectVocabulary(               IN  DWORD                              dwLocalPort,
                                                   IN  DWORD                              dwVocabsCount,
                                                   IN  const XHV_SR_VOCAB_SELECTION      *pVocabSelections); 

    HRESULT WINAPI VoiceMailRecord(                IN  DWORD                              dwLocalPort,
                                                   IN  DWORD                              dwMaxTimeMs,
                                                   IN  DWORD                              dwOutputBufferSize,
                                                   OUT BYTE                              *pbOutputBuffer);
                                            
    HRESULT WINAPI VoiceMailPlay(                  IN  DWORD                              dwLocalPort,
                                                   IN  DWORD                              dwInputBufferSize,
                                                   IN  const BYTE                        *pbInputBuffer,
                                                   IN  BOOL                               bOutputToSpeakers);

    HRESULT WINAPI VoiceMailStop(                  IN  DWORD                              dwLocalPort);                                                   
};

#endif __cplusplus

ULONG   WINAPI XHVEngine_AddRef(                          IN  XHVEngine                          *pThis);

ULONG   WINAPI XHVEngine_Release(                         IN  XHVEngine                          *pThis);

HRESULT WINAPI XHVEngine_EnableProcessingMode(            IN  XHVEngine                          *pThis,
                                                          IN  XHV_PROCESSING_MODE                 processingMode);
                                                          
HRESULT WINAPI XHVEngine_SetCallbackInterface(            IN  XHVEngine                          *pThis,
                                                          IN  ITitleXHV                          *pITitleXHV);

HRESULT WINAPI XHVEngine_GetCallbackInterface(            IN  XHVEngine                          *pThis,
                                                          OUT ITitleXHV                         **ppITitleXHV);                                                                                                                   

HRESULT WINAPI XHVEngine_DoWork(                          IN  XHVEngine                          *pThis);
                                                          
HRESULT WINAPI XHVEngine_RegisterLocalTalker(             IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort); 

HRESULT WINAPI XHVEngine_UnregisterLocalTalker(           IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort); 
                                                          
HRESULT WINAPI XHVEngine_SetProcessingMode(               IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort,
                                                          IN  XHV_PROCESSING_MODE                 processingMode);

HRESULT WINAPI XHVEngine_GetProcessingMode(               IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort,
                                                          IN  XHV_PROCESSING_MODE                *pProcessingMode);
                                                          
HRESULT WINAPI XHVEngine_SetVoiceMask(                    IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort,
                                                          IN  const XHV_VOICE_MASK               *pVoiceMask); 
                                                          
HRESULT WINAPI XHVEngine_GetLocalTalkerStatus(            IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort,
                                                          OUT XHV_LOCAL_TALKER_STATUS            *pLocalTalkerStatus); 
                                                          
HRESULT WINAPI XHVEngine_RegisterRemoteTalker(            IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker); 
                                                          
HRESULT WINAPI XHVEngine_UnregisterRemoteTalker(          IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker); 
                                                          
HRESULT WINAPI XHVEngine_GetRemoteTalkers(                IN  XHVEngine                          *pThis,
                                                          OUT DWORD                              *pdwRemoteTalkersCount,
                                                          OUT XUID                               *pxuidRemoteTalkers); 
                                                          
HRESULT WINAPI XHVEngine_SubmitIncomingVoicePacket(       IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker, 
                                                          IN  VOID                               *pvData,
                                                          IN  DWORD                               dwSize); 
                                                          
BOOL    WINAPI XHVEngine_IsTalking(                       IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker); 
                                                          
HRESULT WINAPI XHVEngine_SetMixBinMapping(                IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker, 
                                                          IN  DWORD                               dwLocalPort,
                                                          IN  const DSMIXBINS                    *pMixBins);

HRESULT WINAPI XHVEngine_SetMixBinVolumes(                IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker, 
                                                          IN  const DSMIXBINS                    *pMixBins);

HRESULT WINAPI XHVEngine_SetPlaybackPriority(             IN  XHVEngine                          *pThis,
                                                          IN  XUID                                xuidRemoteTalker, 
                                                          IN  DWORD                               dwLocalPort, 
                                                          IN  XHV_PLAYBACK_PRIORITY               playbackPriority );

HRESULT WINAPI XHVEngine_SetMaxPlaybackStreamsCount(      IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwStreamsCount);
                                                          
HRESULT WINAPI XHVEngine_RegisterSpeechRecognitionBank(   IN  XHVEngine                          *pThis,
                                                          IN  VOID                               *pvSRBank); 
                                                          
HRESULT WINAPI XHVEngine_SelectVocabulary(                IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort, 
                                                          IN  DWORD                               dwVocabsCount,
                                                          IN  const XHV_SR_VOCAB_SELECTION       *pVocabSelections); 

HRESULT WINAPI XHVEngine_VoiceMailRecord(                 IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort,
                                                          IN  DWORD                               dwMaxTimeMs,
                                                          IN  DWORD                               dwOutputBufferSize,
                                                          OUT BYTE                               *pbOutputBuffer);
                                            
HRESULT WINAPI XHVEngine_VoiceMailPlay(                   IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort,
                                                          IN  DWORD                               dwInputBufferSize,
                                                          IN  const BYTE                         *pbInputBuffer,
                                                          IN  BOOL                                bOutputToSpeakers);

HRESULT WINAPI XHVEngine_VoiceMailStop(                   IN  XHVEngine                          *pThis,
                                                          IN  DWORD                               dwLocalPort); 
                                                          
//
// Compatibility wrappers.
//
XHVINLINE ULONG   WINAPI IXHVEngine_AddRef(XHVEngine *pThis) 
                         { return XHVEngine_AddRef(pThis); }
XHVINLINE ULONG   WINAPI IXHVEngine_Release(XHVEngine *pThis) 
                         { return XHVEngine_Release(pThis); }
XHVINLINE HRESULT WINAPI IXHVEngine_EnableProcessingMode(XHVEngine *pThis, XHV_PROCESSING_MODE processingMode)
                         { return XHVEngine_EnableProcessingMode(pThis, processingMode);}
XHVINLINE HRESULT WINAPI IXHVEngine_SetCallbackInterface( XHVEngine *pThis, ITitleXHV *pITitleXHV)
                         { return XHVEngine_SetCallbackInterface( pThis, pITitleXHV); }
XHVINLINE HRESULT WINAPI IXHVEngine_GetCallbackInterface( XHVEngine *pThis, ITitleXHV **ppITitleXHV)
                         { return XHVEngine_GetCallbackInterface( pThis, ppITitleXHV); }
XHVINLINE HRESULT WINAPI IXHVEngine_DoWork(XHVEngine *pThis) 
                         { return XHVEngine_DoWork(pThis); }                         
XHVINLINE HRESULT WINAPI IXHVEngine_RegisterLocalTalker(XHVEngine *pThis, DWORD dwLocalPort)
                         { return XHVEngine_RegisterLocalTalker(pThis, dwLocalPort);}
XHVINLINE HRESULT WINAPI IXHVEngine_UnregisterLocalTalker(XHVEngine *pThis, DWORD dwLocalPort)
                         { return XHVEngine_UnregisterLocalTalker(pThis, dwLocalPort);}
XHVINLINE HRESULT WINAPI IXHVEngine_SetProcessingMode(XHVEngine *pThis, DWORD dwLocalPort, XHV_PROCESSING_MODE processingMode)
                         { return XHVEngine_SetProcessingMode(pThis, dwLocalPort, processingMode);}
XHVINLINE HRESULT WINAPI IXHVEngine_GetProcessingMode(XHVEngine *pThis, DWORD dwLocalPort, XHV_PROCESSING_MODE *pProcessingMode)
                         { return XHVEngine_GetProcessingMode(pThis, dwLocalPort, pProcessingMode);}
XHVINLINE HRESULT WINAPI IXHVEngine_SetVoiceMask( XHVEngine *pThis, DWORD dwLocalPort, const XHV_VOICE_MASK *pVoiceMask)
                         { return XHVEngine_SetVoiceMask(pThis, dwLocalPort, pVoiceMask);}
XHVINLINE HRESULT WINAPI IXHVEngine_GetLocalTalkerStatus( XHVEngine *pThis, DWORD dwLocalPort, XHV_LOCAL_TALKER_STATUS *pLocalTalkerStatus)
                         { return XHVEngine_GetLocalTalkerStatus( pThis, dwLocalPort, pLocalTalkerStatus); }
XHVINLINE HRESULT WINAPI IXHVEngine_RegisterRemoteTalker( XHVEngine *pThis, XUID xuidRemoteTalker)
                         { return XHVEngine_RegisterRemoteTalker(pThis, xuidRemoteTalker);}
XHVINLINE HRESULT WINAPI IXHVEngine_UnregisterRemoteTalker( XHVEngine *pThis, XUID xuidRemoteTalker)
                         { return XHVEngine_UnregisterRemoteTalker(pThis, xuidRemoteTalker);}
XHVINLINE HRESULT WINAPI IXHVEngine_GetRemoteTalkers( XHVEngine *pThis, DWORD *pdwRemoteTalkersCount, XUID *pxuidRemoteTalkers)
                         { return XHVEngine_GetRemoteTalkers( pThis, pdwRemoteTalkersCount, pxuidRemoteTalkers); }
XHVINLINE HRESULT WINAPI IXHVEngine_SubmitIncomingVoicePacket( XHVEngine *pThis, XUID xuidRemoteTalker, VOID *pvData, DWORD dwSize)
                         { return XHVEngine_SubmitIncomingVoicePacket(pThis, xuidRemoteTalker, pvData, dwSize);}
XHVINLINE BOOL    WINAPI IXHVEngine_IsTalking( XHVEngine *pThis, XUID xuidRemoteTalker)
                         { return XHVEngine_IsTalking(pThis, xuidRemoteTalker);}
XHVINLINE HRESULT WINAPI IXHVEngine_SetMixBinMapping( XHVEngine *pThis, XUID xuidRemoteTalker, DWORD dwLocalPort, const DSMIXBINS *pMixBins)
                         { return XHVEngine_SetMixBinMapping(pThis, xuidRemoteTalker, dwLocalPort, pMixBins);}
XHVINLINE HRESULT WINAPI IXHVEngine_SetMixBinVolumes( XHVEngine *pThis, XUID xuidRemoteTalker, const DSMIXBINS *pMixBins)
                         { return XHVEngine_SetMixBinVolumes(pThis, xuidRemoteTalker, pMixBins);}
XHVINLINE HRESULT WINAPI IXHVEngine_SetPlaybackPriority( XHVEngine *pThis, XUID xuidRemoteTalker, DWORD dwLocalPort, XHV_PLAYBACK_PRIORITY playbackPriority)
                         { return XHVEngine_SetPlaybackPriority( pThis, xuidRemoteTalker, dwLocalPort, playbackPriority); }
XHVINLINE HRESULT WINAPI IXHVEngine_SetMaxPlaybackStreamsCount( XHVEngine *pThis, DWORD dwStreamsCount)
                         { return XHVEngine_SetMaxPlaybackStreamsCount( pThis, dwStreamsCount); }
XHVINLINE HRESULT WINAPI IXHVEngine_RegisterSpeechRecognitionBank( XHVEngine *pThis, VOID *pvSRBank)
                         { return XHVEngine_RegisterSpeechRecognitionBank(pThis, pvSRBank);}
XHVINLINE HRESULT WINAPI IXHVEngine_SelectVocabulary( XHVEngine *pThis, DWORD dwLocalPort,DWORD dwVocabsCount, const XHV_SR_VOCAB_SELECTION *pVocabSelections)
                         { return XHVEngine_SelectVocabulary( pThis, dwLocalPort, dwVocabsCount, pVocabSelections); }
XHVINLINE HRESULT WINAPI IXHVEngine_VoiceMailRecord( XHVEngine *pThis, DWORD dwLocalPort, DWORD dwMaxTimeMs, DWORD dwOutputBufferSize, BYTE *pbOutputBuffer)
                         { return XHVEngine_VoiceMailRecord( pThis, dwLocalPort, dwMaxTimeMs, dwOutputBufferSize, pbOutputBuffer);}
XHVINLINE HRESULT WINAPI IXHVEngine_VoiceMailPlay( XHVEngine *pThis, DWORD dwLocalPort, DWORD dwInputBufferSize, const BYTE *pbInputBuffer, BOOL bOutputToSpeakers)
                         { return XHVEngine_VoiceMailPlay( pThis, dwLocalPort, dwInputBufferSize, pbInputBuffer, bOutputToSpeakers);}
XHVINLINE HRESULT WINAPI IXHVEngine_VoiceMailStop( XHVEngine *pThis, DWORD dwLocalPort)
                         { return XHVEngine_VoiceMailStop( pThis, dwLocalPort);}

#ifdef __cplusplus

XHVINLINE ULONG   WINAPI IXHVEngine::AddRef() 
                         { return XHVEngine_AddRef(this); }
XHVINLINE ULONG   WINAPI IXHVEngine::Release() 
                         { return XHVEngine_Release(this); }
XHVINLINE HRESULT WINAPI IXHVEngine::EnableProcessingMode( XHV_PROCESSING_MODE processingMode)
                         { return XHVEngine_EnableProcessingMode(this, processingMode);}
XHVINLINE HRESULT WINAPI IXHVEngine::SetCallbackInterface( ITitleXHV *pITitleXHV)
                         { return XHVEngine_SetCallbackInterface( this, pITitleXHV); }
XHVINLINE HRESULT WINAPI IXHVEngine::GetCallbackInterface( ITitleXHV **ppITitleXHV)
                         { return XHVEngine_GetCallbackInterface( this, ppITitleXHV); }
XHVINLINE HRESULT WINAPI IXHVEngine::DoWork() 
                         { return XHVEngine_DoWork(this); }                         
XHVINLINE HRESULT WINAPI IXHVEngine::RegisterLocalTalker( DWORD dwLocalPort)
                         { return XHVEngine_RegisterLocalTalker(this, dwLocalPort);}
XHVINLINE HRESULT WINAPI IXHVEngine::UnregisterLocalTalker( DWORD dwLocalPort)
                         { return XHVEngine_UnregisterLocalTalker(this, dwLocalPort);}
XHVINLINE HRESULT WINAPI IXHVEngine::SetProcessingMode( DWORD dwLocalPort, XHV_PROCESSING_MODE processingMode)
                         { return XHVEngine_SetProcessingMode(this, dwLocalPort, processingMode);}
XHVINLINE HRESULT WINAPI IXHVEngine::GetProcessingMode( DWORD dwLocalPort, XHV_PROCESSING_MODE *pProcessingMode)
                         { return XHVEngine_GetProcessingMode(this, dwLocalPort, pProcessingMode);}
XHVINLINE HRESULT WINAPI IXHVEngine::SetVoiceMask( DWORD dwLocalPort, const XHV_VOICE_MASK *pVoiceMask)
                         { return XHVEngine_SetVoiceMask(this, dwLocalPort, pVoiceMask);}
XHVINLINE HRESULT WINAPI IXHVEngine::GetLocalTalkerStatus( DWORD dwLocalPort, XHV_LOCAL_TALKER_STATUS *pLocalTalkerStatus)
                         { return XHVEngine_GetLocalTalkerStatus( this, dwLocalPort, pLocalTalkerStatus); }
XHVINLINE HRESULT WINAPI IXHVEngine::RegisterRemoteTalker( XUID xuidRemoteTalker)
                         { return XHVEngine_RegisterRemoteTalker(this, xuidRemoteTalker);}
XHVINLINE HRESULT WINAPI IXHVEngine::UnregisterRemoteTalker( XUID xuidRemoteTalker)
                         { return XHVEngine_UnregisterRemoteTalker(this, xuidRemoteTalker);}
XHVINLINE HRESULT WINAPI IXHVEngine::GetRemoteTalkers( DWORD *pdwRemoteTalkersCount, XUID *pxuidRemoteTalkers)
                         { return XHVEngine_GetRemoteTalkers( this, pdwRemoteTalkersCount, pxuidRemoteTalkers); }
XHVINLINE HRESULT WINAPI IXHVEngine::SubmitIncomingVoicePacket( XUID xuidRemoteTalker, VOID *pvData, DWORD dwSize)
                         { return XHVEngine_SubmitIncomingVoicePacket(this, xuidRemoteTalker, pvData, dwSize);}
XHVINLINE BOOL    WINAPI IXHVEngine::IsTalking( XUID xuidRemoteTalker)
                         { return XHVEngine_IsTalking(this, xuidRemoteTalker);}
XHVINLINE HRESULT WINAPI IXHVEngine::SetMixBinMapping( XUID xuidRemoteTalker, DWORD dwLocalPort, const DSMIXBINS *pMixBins)
                         { return XHVEngine_SetMixBinMapping(this, xuidRemoteTalker, dwLocalPort, pMixBins);}
XHVINLINE HRESULT WINAPI IXHVEngine::SetMixBinVolumes( XUID xuidRemoteTalker, const DSMIXBINS *pMixBins)
                         { return XHVEngine_SetMixBinVolumes(this, xuidRemoteTalker, pMixBins);}
XHVINLINE HRESULT WINAPI IXHVEngine::SetPlaybackPriority( XUID xuidRemoteTalker, DWORD dwLocalPort, XHV_PLAYBACK_PRIORITY playbackPriority)
                         { return XHVEngine_SetPlaybackPriority( this, xuidRemoteTalker, dwLocalPort, playbackPriority); }
XHVINLINE HRESULT WINAPI IXHVEngine::SetMaxPlaybackStreamsCount( DWORD dwStreamsCount)
                         { return XHVEngine_SetMaxPlaybackStreamsCount( this, dwStreamsCount); }
XHVINLINE HRESULT WINAPI IXHVEngine::RegisterSpeechRecognitionBank( VOID *pvSRBank)
                         { return XHVEngine_RegisterSpeechRecognitionBank(this, pvSRBank);}
XHVINLINE HRESULT WINAPI IXHVEngine::SelectVocabulary( DWORD dwLocalPort,DWORD dwVocabsCount, const XHV_SR_VOCAB_SELECTION *pVocabSelections)
                         { return XHVEngine_SelectVocabulary( this, dwLocalPort, dwVocabsCount, pVocabSelections); }
XHVINLINE HRESULT WINAPI IXHVEngine::VoiceMailRecord( DWORD dwLocalPort, DWORD dwMaxTimeMs, DWORD dwOutputBufferSize, BYTE *pbOutputBuffer)
                         { return XHVEngine_VoiceMailRecord( this, dwLocalPort, dwMaxTimeMs, dwOutputBufferSize, pbOutputBuffer);}
XHVINLINE HRESULT WINAPI IXHVEngine::VoiceMailPlay( DWORD dwLocalPort, DWORD dwInputBufferSize, const BYTE *pbInputBuffer, BOOL bOutputToSpeakers)
                         { return XHVEngine_VoiceMailPlay( this, dwLocalPort, dwInputBufferSize, pbInputBuffer, bOutputToSpeakers);}
XHVINLINE HRESULT WINAPI IXHVEngine::VoiceMailStop( DWORD dwLocalPort)
                         { return XHVEngine_VoiceMailStop( this, dwLocalPort);}

#endif __cplusplus

//-----------------------------------------------------------------------------
//
//  XHV APIs
//
//-----------------------------------------------------------------------------

XBOXAPI
HRESULT
WINAPI
XHVEngineCreate(
    IN  PXHV_RUNTIME_PARAMS               pParams,
    OUT PXHVENGINE                       *ppEngine
    );


#ifdef __cplusplus
}
#endif

#pragma warning( pop )

#endif // __XHV__
