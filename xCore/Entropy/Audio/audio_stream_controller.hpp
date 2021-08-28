#ifndef AUDIO_STREAM_CONTROLLER_HPP
#define AUDIO_STREAM_CONTROLLER_HPP

#ifndef AUDIO_MGR_HPP
#include "e_Audio.hpp"
#endif // AUDIO_MGR_HPP

#ifndef AUDIO_STREAM_MGR_HPP
#include "audio_stream_mgr.hpp"
#endif AUDIO_STREAM_MGR_HPP

class audio_stream_controller 
{
    struct controller_data
    {
        voice_id    VoiceID;
        s32         Priority;
    };

    //------------------------------------------------------------------------------
    // Public functions.

public:

    audio_stream_controller             ( void );
    ~audio_stream_controller            ( void );

    void            Init                ( void );
    void            Kill                ( void );

    void            ReserveStreams      ( s32 nStreams );

    voice_id        Play                ( const char*       pIdentifier,
                                          xbool             AutoStart = TRUE );

    voice_id        Play                ( const char*       pIdentifier,
                                          const vector3&    Position,
                                          s32               ZoneID,
                                          xbool             AutoStart = TRUE );

    voice_id        PlayVolumeClipped   ( const char*       pIdentifier,
                                          const vector3&    Position,
                                          s32               ZoneID,
                                          xbool             AutoStart = TRUE );

    voice_id        Play                ( const char*       pIdentifier, 
                                          xbool             AutoStart, 
                                          const vector3&    Position, 
                                          s32               ZoneID,
                                          xbool             IsPositional, 
                                          xbool             bVolumeClip );

    s32             GetAvailableStreams ( void );

    s32             GetMaxStreams       ( void );

    void            ClearStreams        ( void );

private:

    s32             m_nStreams;         
    controller_data m_Data[ MAX_AUDIO_STREAMS ];

};

#endif // AUDIO_STREAM_CONTROLLER_HPP