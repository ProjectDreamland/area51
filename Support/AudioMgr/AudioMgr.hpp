#ifndef AUDIO_MANAGER_HPP
#define AUDIO_MANAGER_HPP

//==============================================================================
// INCLUDE
//==============================================================================
#include "Entropy.hpp"
#include "Characters\AlertPackage.hpp"

//==============================================================================
// DEFINES
//==============================================================================

#define AUDIO_ENABLE
#define NEAR_CLIP               50
#define FAR_CLIP                5000
#define ZONE_INTERPOLATE_DIST   700
#define STORED_SOUND_STACK_SIZE 50


//==============================================================================
//==============================================================================
          
//==============================================================================
// AUDIO MANAGER
//==============================================================================

class audio_manager
{
public:

//==============================================================================

    enum sound_type
    {
        NULL_SOUND,
        FOOT_STEP,
        BULLET_IMPACTS,
        GUN_SHOT,
        EXPLOSION,
    };

    struct receiver
    {
        vector3     Pos;
        vector3     OriginalPos;
        xtick       Time;
        guid        Guid;
        s16         ZoneID;
        sound_type  Type;
        voice_id    VoiceID;
    };
    
    struct propagation_ear
    {
        vector3     Position;
        s16         ZoneID;
        s32         EarID;
    };

//==============================================================================

public:
                        audio_manager           ( void );
                       ~audio_manager           ( void );
                                                
        void            Init                    ( s32 MemSize );
        void            ResizeMemory            ( s32 MemSize );
        voice_id        Play(const char* pIdentifier,
            sound_type SoundType,
            const vector3& Position,
            s32 ZoneID,
            guid Guid,
            xbool AutoStart,
            xbool VolumeClipped);

        void            Render                  ( void );
                                                
                                                
        void            AppendAlertReceiver     ( const vector3&                Position,
                                                  f32                           AlertRadius,
                                                  alert_package::alert_type     AlertType,
                                                  alert_package::alert_targets  AlertTarget,
                                                  guid&                         Origin,
                                                  guid&                         Cause,
                                                  factions                      Factions );
                                                  
        void            AppendAlertReceiver     ( alert_package&    AlertPackage );
        
        void            NewAudioAlert           ( voice_id         VoiceID, 
                                                  sound_type       SoundType, 
                                                  const vector3&   Position,
                                                  s32              ZoneID, 
                                                  guid             Guid );
        void            NewAudioAlert           ( voice_id          VoiceID, 
                                                  sound_type        SoundType, 
                                                  guid Guid );
                                                                                                                                                                
//------------------------------------------------------------------------------
// RECEIVER FUNCTIONS
//------------------------------------------------------------------------------
        
        void            ClearReceiverQueue      ( void );
        receiver*       GetReceiverQueue        ( void );
        receiver*       GetFirstReceiverItem    ( xtick Time );
        receiver*       GetNextReceiverItem     ( void );

        void            ClearAlertReceiverQueue      ( void );
        alert_package*  GetAlertReceiverQueue        ( void );
        alert_package*  GetFirstAlertReceiverItem    ( xtick Time );
        alert_package*  GetAlertNextReceiverItem     ( void );

private:        
    
        vector3         m_PlayerPos;
        s32             m_PlayerZoneID;
        f32             m_NearClip;
        f32             m_FarClip;

        receiver        m_Receiver[STORED_SOUND_STACK_SIZE];
        s32             m_CurrentReceiverCursor;
        s32             m_ReceiverSendCursor;
        s32             m_ReceiverFirstSendCursor;
        xtick           m_ReceiverSendCursorTime;

        alert_package   m_AlertReceiver[STORED_SOUND_STACK_SIZE];
        s32             m_CurrentAlertReceiverCursor;
        s32             m_AlertReceiverSendCursor;
        s32             m_AlertReceiverFirstSendCursor;
        xtick           m_AlertReceiverSendCursorTime;
};

//==============================================================================
// EXTERNAL
//==============================================================================
extern audio_manager g_AudioManager;

//==============================================================================
// END
//==============================================================================

#endif
