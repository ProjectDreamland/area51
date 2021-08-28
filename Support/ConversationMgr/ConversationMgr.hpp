#ifndef CONVERSATION_MGR_HPP
#define CONVERSATION_MGR_HPP

//==============================================================================
// INCLUDE
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"


//==============================================================================
//
//  This is the layer that the game talks to, the voice id that gets passed back  
//  is actually the handle in the Playlist array.  The voice id that is stored
//  in the converse base structure is the actual voice id passed from the audio
//  manager.
//
//==============================================================================


//==============================================================================
// DEFINES
//==============================================================================

#define MAX_CONVERSATION_STREAM     2
#define MAX_DESCRIPTOR_NAME         64
#define IMMEDIATE_PLAY              0.0f        
#define MAX_DELAY_PLAY              1.0f        // 1 secs
#define HOT_VOICE_BIT               (1<<13)
#define CHAIN_VOICE_BIT             (1<<14)
#define MIN_VOICE_TIME_EXTENSION    15.0f
#define MAX_VOICE_TIME_EXTENSION    20.0f
#define DEFAULT_INDEX               0
#define AUTO_START                  (1<<1)
#define VOICE_CHAIN                 (1<<2)
#define PLAY_2D                     (1<<3)
//==============================================================================
// CONVERSATION MANAGER
//==============================================================================

class conversation_mgr
{
public:

//==============================================================================
//
// We need to store the position, zone info, clip scales and volume of the stream 
// samaple because they don't get played immediately like the hot voices.  When we 
// request a stream sampled to be played it appends that request to the Playlist 
// which will get processed in the next update call.
//
//==============================================================================
    struct converse_base
    {   
        converse_base()
        {
            pIdentifier[0]      = '\0';
            Guid                = 0;
            Priority            = 0;
            VoiceID             = 0;
            ActiveVoice         = 0;
            TimeInQueue         = 0.0f;
            Index               = 0;
            NearClip            = 0.0f;
            FarClip             = 0.0f;
            Volume              = 0.0f;
            ChainIndex          = -1;
            Flags               = 0;
        }

        ~converse_base()
        {
            pIdentifier[0]      = '\0';
            Guid                = 0;
            Priority            = 0;
            VoiceID             = 0;
            ActiveVoice         = 0;
            TimeInQueue         = 0.0f;
            Index               = 0;
            NearClip            = 0.0f;
            FarClip             = 0.0f;
            Volume              = 0.0f;
            ChainIndex          = -1;
            Flags               = 0;
        }


        char    pIdentifier[MAX_DESCRIPTOR_NAME];
        guid    Guid;
        s16     Priority;
        s32     VoiceID;
        u32     ActiveVoice;
        f32     TimeInQueue;
        xhandle Handle;
        s32     Index;
        s16     ZoneID;
        f32     NearClip;
        f32     FarClip;
        vector3 Position;
        f32     Volume;
        xbool   AutoStart;
        s16     ChainIndex;
        u8      Flags;
    };

    struct stream_info
    {
        char    pIdentifier[MAX_DESCRIPTOR_NAME];
        s32     VoiceID;
        u32     ActiveVoice;
        s32     Priority;
    };

    struct stream_active_voice
    {
        u32     VoiceID;
        xhandle Handle;
    };

    struct hot_voice
    {
        char    pIdentifier[MAX_DESCRIPTOR_NAME];
        s32     Index;
        u32     VoiceID;
        u32     VoiceHandle;
        f32     ExtensionTime;
        u8      Flags;
    };

    struct voice_chain
    {
        xhandle VoiceHandle;
        s32     ChainIndex;
    };
    
public:

//==============================================================================    
            conversation_mgr    ( void );
           ~conversation_mgr    ( void );

    void    Init                ( void );
    void    Kill                ( void );
    void    Reset               ( void );
    
    u32     PlayStream          ( const char* pObjectName, const char* pAction, guid Guid,
                                  s16 ZoneID, const vector3& Position, f32 MaxInQueueTime = IMMEDIATE_PLAY, 
                                  xbool AutoStart = TRUE, u8 Flags = 0);
    
    u32     PlayStream          ( const char* pDescriptor, const vector3& Position, guid Guid, s16 ZoneID,
                                  f32 MaxInQueueTime = IMMEDIATE_PLAY, xbool AutoStart = TRUE,
                                  u8 Flags = 0);
    
    u32     PlayHotVoice        ( const char* pObjectName, const char* pAction, 
                                  const vector3& Position, s16 ZoneID, f32 Pitch, xbool AutoStart = TRUE, u8 Flags = 0 );

    u32     PlayHotVoice        ( const char* pDesc, const vector3& Position, s16 ZoneID, xbool AutoStart = TRUE, u8 Flags = 0 );

    xbool   HasDialog           ( const char* pObjectName, const char* pAction );
    xbool   HasDialog           ( const char* pDescriptor );

    void    StartSound          ( u32 VoiceID );
    void    Stop                ( u32 VoiceID, f32 ReleaseTime = 0.0f );

    xbool   IsActive            ( u32 VoiceID );
    xbool   IsPlaying           ( u32 VoiceID );
    xbool   IsReadyToPlay       ( u32 VoiceID );
    
    void    SetPosition         ( u32 VoiceID, vector3& Pos, s16 ZoneID );
    void    SetFalloff          ( u32 VoiceID, f32 NearScale, f32 FarScale );
    void    SetVolume           ( u32 VoiceID, f32 Volume );

    f32     GetVolume           ( u32 VoiceID );

    u32     GetBaseAudioVoiceID ( u32 VoiceID );

    void    Update              ( f32 DeltaTime );

protected:
//==============================================================================
    u32     PlayHotVoice        ( const char* pDesc, const vector3& Position,
                                    s16 ZoneID, f32 Pitch, xbool AutoStart, u8 Flags );

    void    StopHotVoice        ( u32 VoiceID, f32 ReleaseTime );

    xbool   IsHotVoiceActive    ( u32 VoiceID );
    xbool   IsHotVoicePlaying   ( u32 VoiceID );

    void    UpdateHotVoice      ( f32 DeltaTime );
    void    UpdateStream        ( f32 DeltaTime );

//==============================================================================

};

extern conversation_mgr g_ConverseMgr;

#endif