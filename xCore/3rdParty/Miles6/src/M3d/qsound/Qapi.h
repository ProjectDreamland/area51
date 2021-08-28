//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  QAPI.H: Abstraction for QSound QMIXER and QMDX libraries              ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 10-Apr-99: Initial                                    ##
//##                                                                        ##
//##  Author: Dan Teven                                                     ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#if !defined(QAPI_H_INCLUDED)

#if defined(QMDX) && QMDX
   #define  QSOUND_PROVIDER_NAME          "QSound QMDX for Miles"
	#include "qmixer.h"
   #define  QSoundActivate                QMDX_Activate
   #define  QSoundCalculateChannelParams  QMDX_CalculateChannelParams
   #define  QSoundCloseChannel            QMDX_CloseChannel
   #define  QSoundCloseSession            QMDX_CloseSession
   #define  QSoundEnableChannel           QMDX_EnableChannel
   #define  QSoundFlushChannel            QMDX_FlushChannel
   #define  QSoundFreeWave                QMDX_FreeWave
   #define  QSoundGetChannelParams        QMDX_GetChannelParams
   #define  QSoundGetChannelStatus        QMDX_GetChannelStatus
   #define  QSoundGetDirectSoundBuffer    QMDX_GetDirectSoundBuffer
   #define  QSoundGetErrorText            QMDX_GetErrorText
   #define  QSoundGetInfoEx               QMDX_GetInfoEx
   #define  QSoundGetLastError            QMDX_GetLastError
   #define  QSoundGetOptions              QMDX_GetOptions
   #define  QSoundGetPlayPosition         QMDX_GetPlayPosition 
   #define  QSoundGetRoomSize             QMDX_GetRoomSize
   #define  QSoundGetSourceCone2          QMDX_GetSourceCone2
   #define  QSoundGetSpeakerPlacement     QMDX_GetSpeakerPlacement
   #define  QSoundGetSpeedOfSound         QMDX_GetSpeedOfSound
   #define  QSoundGetVolume               QMDX_GetVolume
   #define  QSoundInitEx                  QMDX_InitEx
   #define  QSoundIsChannelDone           QMDX_IsChannelDone
   #define  QSoundOpenChannel             QMDX_OpenChannel
   #define  QSoundOpenWaveEx              QMDX_OpenWaveEx
   #define  QSoundPauseChannel            QMDX_PauseChannel
   #define  QSoundPlayEx                  QMDX_PlayEx
   #define  QSoundPump                    QMDX_Pump
   #define  QSoundRestartChannel          QMDX_RestartChannel
   #define  QSoundSetChannelParams        QMDX_SetChannelParams
   #define  QSoundSetDistanceMapping      QMDX_SetDistanceMapping
   #define  QSoundSetFrequency            QMDX_SetFrequency
   #define  QSoundSetListenerOrientation  QMDX_SetListenerOrientation
   #define  QSoundSetListenerPosition     QMDX_SetListenerPosition
   #define  QSoundSetListenerRolloff      QMDX_SetListenerRolloff
   #define  QSoundSetListenerVelocity     QMDX_SetListenerVelocity
   #define  QSoundSetOptions              QMDX_SetOptions
   #define  QSoundSetPanRate              QMDX_SetPanRate
   #define  QSoundSetRoomSize             QMDX_SetRoomSize
   #define  QSoundSetSourceCone2          QMDX_SetSourceCone2
   #define  QSoundSetSourcePosition       QMDX_SetSourcePosition
   #define  QSoundSetSourceVelocity       QMDX_SetSourceVelocity
   #define  QSoundSetSpeakerPlacement     QMDX_SetSpeakerPlacement
   #define  QSoundSetSpeedOfSound         QMDX_SetSpeedOfSound
   #define  QSoundSetVolume               QMDX_SetVolume
   #define  QSoundStopChannel             QMDX_StopChannel
#elif defined(QMIXMSS) && QMIXMSS
   #define  QSOUND_PROVIDER_NAME          "QSound QMixer for Miles"
   #include "qmixer.h"
   #define  QSoundActivate                QMixMSS_Activate
   #define  QSoundCalculateChannelParams  QMixMSS_CalculateChannelParams
   #define  QSoundCloseChannel            QMixMSS_CloseChannel
   #define  QSoundCloseSession            QMixMSS_CloseSession
   #define  QSoundEnableChannel           QMixMSS_EnableChannel
   #define  QSoundFlushChannel            QMixMSS_FlushChannel
   #define  QSoundFreeWave                QMixMSS_FreeWave
   #define  QSoundGetChannelParams        QMixMSS_GetChannelParams
   #define  QSoundGetChannelStatus        QMixMSS_GetChannelStatus
   #define  QSoundGetDirectSoundBuffer    QMixMSS_GetDirectSoundBuffer
   #define  QSoundGetErrorText            QMixMSS_GetErrorText
   #define  QSoundGetInfoEx               QMixMSS_GetInfoEx
   #define  QSoundGetLastError            QMixMSS_GetLastError
   #define  QSoundGetOptions              QMixMSS_GetOptions
   #define  QSoundGetPlayPosition         QMixMSS_GetPlayPosition
   #define  QSoundGetRoomSize             QMixMSS_GetRoomSize
   #define  QSoundGetSourceCone2          QMixMSS_GetSourceCone2
   #define  QSoundGetSpeakerPlacement     QMixMSS_GetSpeakerPlacement
   #define  QSoundGetSpeedOfSound         QMixMSS_GetSpeedOfSound
   #define  QSoundGetVolume               QMixMSS_GetVolume
   #define  QSoundInitEx                  QMixMSS_InitEx
   #define  QSoundIsChannelDone           QMixMSS_IsChannelDone
   #define  QSoundOpenChannel             QMixMSS_OpenChannel
   #define  QSoundOpenWaveEx              QMixMSS_OpenWaveEx
   #define  QSoundPauseChannel            QMixMSS_PauseChannel
   #define  QSoundPlayEx                  QMixMSS_PlayEx
   #define  QSoundPump                    QMixMSS_Pump
   #define  QSoundRestartChannel          QMixMSS_RestartChannel
   #define  QSoundSetChannelParams        QMixMSS_SetChannelParams
   #define  QSoundSetDistanceMapping      QMixMSS_SetDistanceMapping
   #define  QSoundSetFrequency            QMixMSS_SetFrequency
   #define  QSoundSetListenerOrientation  QMixMSS_SetListenerOrientation
   #define  QSoundSetListenerPosition     QMixMSS_SetListenerPosition
   #define  QSoundSetListenerRolloff      QMixMSS_SetListenerRolloff
   #define  QSoundSetListenerVelocity     QMixMSS_SetListenerVelocity
   #define  QSoundSetOptions              QMixMSS_SetOptions
   #define  QSoundSetPanRate              QMixMSS_SetPanRate
   #define  QSoundSetRoomSize             QMixMSS_SetRoomSize
   #define  QSoundSetSourceCone2          QMixMSS_SetSourceCone2
   #define  QSoundSetSourcePosition       QMixMSS_SetSourcePosition
   #define  QSoundSetSourceVelocity       QMixMSS_SetSourceVelocity
   #define  QSoundSetSpeakerPlacement     QMixMSS_SetSpeakerPlacement
   #define  QSoundSetSpeedOfSound         QMixMSS_SetSpeedOfSound
   #define  QSoundSetVolume               QMixMSS_SetVolume
   #define  QSoundStopChannel             QMixMSS_StopChannel
#else
   #define  QSOUND_PROVIDER_NAME          "QSound QMixer for Miles"
   #include "qmixer.h"
   #define  QSoundActivate                QSWaveMixActivate
   #define  QSoundCalculateChannelParams  QSWaveMixCalculateChannelParams
   #define  QSoundCloseChannel            QSWaveMixCloseChannel
   #define  QSoundCloseSession            QSWaveMixCloseSession
   #define  QSoundEnableChannel           QSWaveMixEnableChannel
   #define  QSoundFlushChannel            QSWaveMixFlushChannel
   #define  QSoundFreeWave                QSWaveMixFreeWave
   #define  QSoundGetChannelParams        QSWaveMixGetChannelParams
   #define  QSoundGetChannelStatus        QSWaveMixGetChannelStatus
   #define  QSoundGetDirectSoundBuffer    QSWaveMixGetDirectSoundBuffer
   #define  QSoundGetErrorText            QSWaveMixGetErrorText
   #define  QSoundGetInfoEx               QSWaveMixGetInfoEx
   #define  QSoundGetLastError            QSWaveMixGetLastError
   #define  QSoundGetOptions              QSWaveMixGetOptions
   #define  QSoundGetPlayPosition         QSWaveMixGetPlayPosition   
   #define  QSoundGetRoomSize             QSWaveMixGetRoomSize
   #define  QSoundGetSourceCone2          QSWaveMixGetSourceCone2
   #define  QSoundGetSpeakerPlacement     QSWaveMixGetSpeakerPlacement
   #define  QSoundGetSpeedOfSound         QSWaveMixGetSpeedOfSound
   #define  QSoundGetVolume               QSWaveMixGetVolume
   #define  QSoundInitEx                  QSWaveMixInitEx
   #define  QSoundIsChannelDone           QSWaveMixIsChannelDone
   #define  QSoundOpenChannel             QSWaveMixOpenChannel
   #define  QSoundOpenWaveEx              QSWaveMixOpenWaveEx
   #define  QSoundPauseChannel            QSWaveMixPauseChannel
   #define  QSoundPlayEx                  QSWaveMixPlayEx
   #define  QSoundPump                    QSWaveMixPump
   #define  QSoundRestartChannel          QSWaveMixRestartChannel
   #define  QSoundSetChannelParams        QSWaveMixSetChannelParams
   #define  QSoundSetDistanceMapping      QSWaveMixSetDistanceMapping
   #define  QSoundSetFrequency            QSWaveMixSetFrequency
   #define  QSoundSetListenerOrientation  QSWaveMixSetListenerOrientation
   #define  QSoundSetListenerPosition     QSWaveMixSetListenerPosition
   #define  QSoundSetListenerRolloff      QSWaveMixSetListenerRolloff
   #define  QSoundSetListenerVelocity     QSWaveMixSetListenerVelocity
   #define  QSoundSetOptions              QSWaveMixSetOptions
   #define  QSoundSetPanRate              QSWaveMixSetPanRate
   #define  QSoundSetRoomSize             QSWaveMixSetRoomSize
   #define  QSoundSetSourceCone2          QSWaveMixSetSourceCone2
   #define  QSoundSetSourcePosition       QSWaveMixSetSourcePosition
   #define  QSoundSetSourceVelocity       QSWaveMixSetSourceVelocity
   #define  QSoundSetSpeakerPlacement     QSWaveMixSetSpeakerPlacement
   #define  QSoundSetSpeedOfSound         QSWaveMixSetSpeedOfSound
   #define  QSoundSetVolume               QSWaveMixSetVolume
   #define  QSoundStopChannel             QSWaveMixStopChannel
#endif

#if defined(CHECK_API_RETURN_VALUES)
   #define  CHECK_MMRESULT(result)                       \
      if (result != 0)                                   \
         {                                               \
         diag_printf ("API call failed (%x)\n", result); \
         __asm int 3                                     \
         }
#else
   #define  CHECK_MMRESULT(result)
#endif

#define  QAPI_H_INCLUDED
#endif
