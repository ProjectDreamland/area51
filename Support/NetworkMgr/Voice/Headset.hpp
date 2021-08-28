#if !defined(HEADSET_HPP)
#define HEADSET_HPP

//==============================================================================
//
//  Headset.hpp
//
//==============================================================================
#include "x_types.hpp"
#include "x_threads.hpp"
#include "Network/fifo.hpp"

const s32 VOICE_SAMPLE_RATE = 8000;

#if defined(TARGET_XBOX)
#include "xonline.h"
#include "xhv.h"
#endif

//==============================================================================

class headset
{
public:

        //
        // Generic Public Functions
        //

        void            Init                        ( xbool EnableHardware );
        void            Kill                        ( void );
        void            Update                      ( f32 DeltaTime );

        s32             Read                        ( void* pBuffer, s32 MaxLength );
        s32             Write                       ( const void* pBuffer, s32 Length );
        void            SelectCodec                 ( s32 CodecId );
        s32             GetEncodedBlockSize         ( void ) const                          { return m_EncodeBlockSize;   }
        s32             GetDecodedBlockSize         ( void ) const                          { return m_DecodeBlockSize;   } 
        void            PeriodicUpdate              ( f32 DeltaTime );
        void            ProvideUpdate               ( netstream& BitStream, s32 MaxLength = 256 );
        void            AcceptUpdate                ( netstream& BitStream );
        xbool           IsTalking                   ( void ) const                          { return m_IsTalking;                   }
        void            SetTalking                  ( xbool IsTalking );
        xbool           IsHardwarePresent           ( void )                                { return m_HeadsetCount>0;              }
        xbool           IsBanned                    ( void )                                { return m_VoiceBanned;                 }
        xbool           IsEnabled                   ( void )                                { return m_VoiceEnabled;                }
        xbool           IsAudible                   ( void )                                { return m_VoiceAudible;                }
        xbool           IsThroughSpeaker            ( void )                                { return m_VoiceThroughSpeaker;         }
        void            SetVoiceBanned              ( xbool IsBanned  )                     { m_VoiceBanned         = IsBanned;     }
        void            SetVoiceEnabled             ( xbool IsEnabled )                     { m_VoiceEnabled        = IsEnabled;    }
        void            SetVoiceAudible             ( xbool IsAudible )                     { m_VoiceAudible        = IsAudible;    }
        void            SetThroughSpeaker           ( xbool IsEnabled )                     { m_VoiceThroughSpeaker = IsEnabled;    }
        void            SetLoopback                 ( xbool IsEnabled );
        void            SetSpeakerVolume            ( f32 SpeakerVolume );
        void            SetVolume                   ( f32 Headset, f32 MicrophoneSensitivity );
        void            SetActiveHeadset            ( s32 HeadsetIndex );
        void            GetVolume                   ( f32& Headset, f32& MicrophoneSensitivity );
        s32             GetNumBytesInWriteFifo      ( void );
        void            UpdateLoopBack              ( void );

        //
        // XBox Public Functions
        //

        #ifdef TARGET_XBOX

        enum
        {
            ENCODESIZE      =   10,                         // 20 ms of audio
            MAXPACKETS      =   50,                         // Enough for 1 second of audio
            FIFOSIZE        =   (ENCODESIZE * MAXPACKETS),
        };

        void            UpdateActiveHeadset         ( void );
        void            UpdateIncoming              ( void );
        void            UpdateOutputMode            ( void );
        void            UpdateVoiceMail             ( f32       DeltaTime               );

        xbool           InitVoiceRecording          ( void );
        void            KillVoiceRecording          ( void );
        void            StartVoiceRecording         ( void );
        void            StopVoiceRecording          ( xbool     DoForcefullStop = FALSE );
        f32             GetVoiceRecordingProgress   ( void );
        xbool           GetVoiceIsRecording         ( void );
        byte*           GetVoiceMessageRec          ( void );
        s32             GetVoiceNumBytesRec         ( void );
        s32             GetVoiceDurationMS          ( void );

        void            StartVoicePlaying           ( byte*     pVoiceMessage,
                                                      s32       DurationMS,
                                                      s32       NumBytes                );

        void            StopVoicePlaying            ( xbool     DoForcefullStop = FALSE );
        f32             GetVoicePlayingProgress     ( void );
        xbool           GetVoiceIsPlaying           ( void );

        void            EndVoiceMail                ( void );

        xbool           HeadsetJustInserted         ( void );
        void            ClearHeadsetJustInserted    ( void );

        #endif

private:

        //
        // Generic Private Variables
        //

        s32             m_EncodeBlockSize;
        s32             m_DecodeBlockSize;
        f32             m_HeadsetVolume;
        f32             m_MicrophoneSensitivity;
        xbool           m_VolumeChanged;
        s32             m_HeadsetCount;
        xbool           m_HardwareEnabled;
        s32             m_ActiveHeadset;
        xbool           m_IsTalking;
        xbool           m_LoopbackEnabled;
        byte*           m_pEncodeBuffer;
        byte*           m_pDecodeBuffer;
        fifo            m_ReadFifo;
        fifo            m_WriteFifo;
        xthread*        m_pThread;
        xbool           m_VoiceBanned;
        xbool           m_VoiceEnabled;
        xbool           m_VoiceAudible;
        xbool           m_VoiceThroughSpeaker;
        s32             m_HeadsetMask;

        void            OnHeadsetInsert             ( void );
        void            OnHeadsetRemove             ( void );
        //
        // PS2 Private Variables
        //

        #ifdef TARGET_PS2
        s16             m_SpeakerVoiceBuffer[2048];
        s32             m_SpeakerBufferIndex;
        s32             m_LgAudIrxHandle;
        s32             m_UsbIrxHandle;
        s32             m_DeviceHandle;
        #endif

        #ifdef TARGET_XBOX

        //
        // XBox Private Class
        //

        class titlexhv : public ITitleXHV
        {
            public:

            STDMETHODIMP CommunicatorStatusUpdate   ( DWORD                             Port,
                                                      XHV_VOICE_COMMUNICATOR_STATUS     Status  );

            STDMETHODIMP LocalChatDataReady         ( DWORD                             Port,
                                                      DWORD                             Size,
                                                      VOID*                             pData   );

            STDMETHODIMP VoiceMailDataReady         ( DWORD                             Port,
                                                      DWORD                             Duration,
                                                      DWORD                             Size    );

            STDMETHODIMP VoiceMailStopped           ( DWORD                             Port    );

            s32*            m_pHeadsetMask;
            xbool*          m_pIsTalking;
            xbool*          m_pLoopbackEnabled;
            fifo*           m_pOutgoingFifo;
            byte*           m_pVoiceMailData;
            s32             m_VoiceMailDuration;
            s32             m_VoiceMailSize;
            xbool           m_VoiceIsRecording;
            xbool           m_VoiceIsPlaying;
            xbool           m_VoiceCompleted;
            xbool           m_HeadsetUpdated;
            xbool           m_HeadsetJustInserted;
        };

        //
        // XBox Private Variables
        //

        XHVEngine*          m_pXHVEngine;
        titlexhv            m_ITitleXHV;
        XUID                m_RemoteID;
        s32                 m_OldActiveHeadset;
        byte*               m_pFifoBuffer;
        f32                 m_VoiceTimer;

        #endif
};

//=============================================================================
#endif
//=============================================================================
