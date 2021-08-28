#ifndef AUDIO_VOICE_MGR_HPP
#define AUDIO_VOICE_MGR_HPP

#include "audio_private.hpp"

class audio_voice_mgr
{

friend class audio_channel_mgr;
friend class audio_package;
friend class audio_mgr;

//------------------------------------------------------------------------------
// Public functions.

public:

                        audio_voice_mgr             ( void );
                       ~audio_voice_mgr             ( void );
                                                    
        void            Init                        ( void );
        void            Kill                        ( void );
                                                    
        voice*          AcquireVoice                ( s32               Priority,
                                                      f32               AbsoluteVolume );
        void            ReleaseVoice                ( voice*            pVoice,
                                                      f32               Time );
        void            ReleaseVoiceLoop            ( voice*            pVoice );
        void            ReleaseAllVoices            ( void );

        void            ReleasePackagesVoices       ( audio_package*    pPackage );
                                                             
        void            StartVoice                  ( voice*            pVoice );
        xbool           Segue                       ( voice*            pVoice,
                                                      voice*            pVoiceToQ );

        xbool           SetReleaseTime              ( voice*            VoiceID,
                                                      f32               Time );
        void            PauseVoice                  ( voice*            pVoice );
        void            PauseAllVoices              ( void );
        void            ResumeVoice                 ( voice*            pVoice );
        void            ResumeAllVoices             ( void );
                                                    
        xbool           IsVoicePlaying              ( voice*            pVoice );
        xbool           IsVoiceStarting             ( voice*            pVoice );
        xbool           IsVoiceReleasing            ( voice*            pVoice );
        xbool           IsVoiceReady                ( voice*            pVoice );

        void            UpdateAllVoiceVolumes       ( void );
        
        s32             GetVoicePriority            ( voice*            pVoice );
                                                    
        f32             GetVoiceUserVolume          ( voice*            pVoice );
        xbool           SetVoiceUserVolume          ( voice*            pVoice,
                                                      f32               UserVolume );
                                                    
        f32             GetVoiceRelativeVolume      ( voice*            pVoice );
        xbool           SetVoiceRelativeVolume      ( voice*            pVoice,
                                                      f32               RelativeVolume );
                                                    
        f32             GetVoicePan                 ( voice*            pVoice );
        xbool           SetVoicePan                 ( voice*            pVoice,
                                                      f32               Pan );
                                                    
        f32             GetVoiceUserPitch           ( voice*            pVoice );
        xbool           SetVoiceUserPitch           ( voice*            pVoice,
                                                      f32               Pitch );
                                                    
        f32             GetVoiceRelativePitch       ( voice*            pVoice );
        xbool           SetVoiceRelativePitch       ( voice*            pVoice,
                                                      f32               Pitch );
                                                    
        xbool           GetVoicePosition            ( voice*            pVoice, 
                                                      vector3&          Position,
                                                      s32&              ZoneID );
                                                    
        xbool           SetVoicePosition            ( voice*            pVoice,
                                                      const vector3&    Position,
                                                      s32               ZoneID );
                                                    
        xbool           SetVoiceUserFalloff         ( voice*            pVoice,
                                                      f32               Near,
                                                      f32               Far );
                                                    
        xbool           SetVoiceRelativeFalloff     ( voice*            pVoice,
                                                      f32               Near,
                                                      f32               Far );
                                                    
        xbool           SetVoiceUserDiffuse         ( voice*            pVoice,
                                                      f32               Near,
                                                      f32               Far );
        f32             GetVoiceUserEffectSend      ( voice*            pVoice );
        xbool           SetVoiceUserEffectSend      ( voice*            pVoice,
                                                      f32               EffectSend );

        f32             GetVoiceRelativeEffectSend  ( voice*            pVoice );
        xbool           SetVoiceRelativeEffectSend  ( voice*            pVoice,
                                                      f32               EffectSend );

        xbool           HasLipSync                  ( voice*            pVoice );
        f32             GetLipSync                  ( voice*            pVoice );
        s32             GetBreakPoints              ( voice*            pVoice,
                                                      f32* &            BreakPoints );
        xbool           GetIsReady                  ( voice*            pVoice );
        f32             GetCurrentPlayTime          ( voice*            pVoice );
        const char*     GetVoiceDescriptor          ( voice*            pVoice );

        void            Update                      ( f32 DeltaTime );
        void            InitSingleVoice             ( voice*            pVoice,
                                                      audio_package*    pPackage );
        void            InitSingleElement           ( element*          pElement );

inline  xbool           IsValidVoice                ( voice* pVoice )       { return (pVoice >= m_FirstVoice) && (pVoice <= m_LastVoice);}
inline  xbool           IsValidElement              ( element* pElement )   { return (pElement >= m_FirstElement) && (pElement <= m_LastElement);}
inline  s32             GetNumVoices                ( void )                { return m_NumVoices; }
inline  s32             GetNumElements              ( void )                { return m_NumElements; }
inline  voice*          GetVoiceBuffer              ( void )                { return m_FirstVoice; }
        f32             GetVoiceTime                ( voice* pVoice );
        void            Lock                        ( void )                { m_Mutex.Acquire(); m_LockLevel++; }
        void            Unlock                      ( void )                { m_Mutex.Release(); m_LockLevel--; }
        void            UpdateCheckQueued           ( void );
        void            SetPitchLock                ( voice* pVoice, xbool bPitchLock );

//------------------------------------------------------------------------------
// Private functions.

private:

inline  voice*          FreeVoices              ( void )    { return &m_FreeVoices; }
inline  voice*          UsedVoices              ( void )    { return &m_UsedVoices; }
inline  element*        FreeElements            ( void )    { return &m_FreeElements; }
inline  element*        UsedElements            ( void )    { return &m_UsedElements; }
        void            PrioritizeVoice         ( voice* pVoice, xbool RemoveFromList );
        void            FreeVoice               ( voice* pVoice, xbool PutInFreeList );
        element*        AquireElement           ( void );
        void            ReleaseElement          ( element* pElement, xbool ReleaseChannel );
inline  void            StartElement            ( element* pElement );
inline  void            PauseElement            ( element* pElement );
inline  void            ResumeElement           ( element* pElement );
inline  void            ApplyElementVolume      ( element* pElement );
inline  void            ApplyElementPan         ( element* pElement );
inline  void            ApplyElementPitch       ( element* pElement );
inline  void            ApplyElementEffectSend  ( element* pElement );
inline  void            UpdateStartPending      ( voice* pVoice );
inline  voice*          UpdateCheckElements     ( voice* pVoice );
inline  voice*          UpdateStateStarting     ( voice* pVoice );
inline  voice*          UpdateStateResuming     ( voice* pVoice );
inline  voice*          UpdateStateRunning      ( voice* pVoice, f32 DeltaTime );
inline  void            UpdateStatePausing      ( voice* pVoice, f32 DeltaTime );
inline  void            UpdateVoice3d           ( voice* pVoice );
inline  void            UpdateVoiceVolume       ( voice* pVoice );
inline  void            UpdateVoicePan          ( voice* pVoice );
inline  void            UpdateVoicePitch        ( voice* pVoice );
inline  void            UpdateVoiceVolumeAndPan ( voice* pVoice );
inline  void            UpdateVoiceEffectSend   ( voice* pVoice );
inline  voice*          UpdateCheckStreams      ( voice* pVoice );
        void            UpdateVoiceVolume       ( audio_package* pPackage );
        void            UpdateReleaseTime       ( voice* pVoice );
        void            UpdateVoicePitch        ( audio_package* pPackage );
        void            UpdateVoiceEffectSend   ( audio_package* pPackage );
        void            SetPackageVoicesDirty   ( audio_package* pPackage, u32 Bits );
        void            AppendElementToVoice    ( element* pElement, voice* pVoice );
        void            InstantiateStreamSample ( audio_stream* pStream, s32 WhichChannel );

//------------------------------------------------------------------------------
// Private variables.

        voice           m_UsedVoices;
        voice           m_FreeVoices;
        voice*          m_FirstVoice;
        voice*          m_LastVoice;
        s32             m_NumVoices;
        s32             m_NumElements;

        element         m_UsedElements;
        element         m_FreeElements;
        element*        m_FirstElement;
        element*        m_LastElement;

        xmutex          m_Mutex;
public:
        s32             m_LockLevel;
};

extern audio_voice_mgr g_AudioVoiceMgr;

#endif
