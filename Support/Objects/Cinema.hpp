#ifndef CINEMA_HPP
#define CINEMA_HPP

#include "Obj_mgr\obj_mgr.hpp"
#include "TriggerEx\Affecters\object_affecter.hpp"
#include "Animation\AnimAudioTimer.hpp"


class cinema_object : public object
{
public:
    enum audio_status
    {
        AUDIO_STATUS_OFF,       // Audio not started
        AUDIO_STATUS_WARMING,   // Audio warming up
        AUDIO_STATUS_PLAYING,   // Audio playing
        AUDIO_STATUS_DONE       // Audio complete
    };

    enum anim_status
    {
        ANIM_STATUS_OFF,        // Anims not started
        ANIM_STATUS_PLAYING,    // Anims playing
        ANIM_STATUS_DONE        // Anims complete
    };

    enum marker_type
    {
        BLOCKING,
        ACTIVATING,
    };

    enum guid_type
    {
        STATIC,
        VARIABLE,
    };

    struct position_marker 
    {
        f32             Time;
        object_affecter ObjectAffecter;

        position_marker();
        void Init( void );
    };

    struct action_marker
    {
        marker_type     MarkerType;
        f32             Time;
        object_affecter ObjectAffecter;
        char            Name[64];

        action_marker();
        void Init( void );
    };

    struct cinema_character
    {
        object_affecter     CharacterAffecter;      // Character guid
        object_affecter     StartMarkerAffecter;    // Start marker guid (if any)
        f32                 StartMarkerDistance;    // Start marker distance threshold
        radian              StartMarkerYaw;         // Start marker yaw threshold
        object_affecter     EndMarkerAffecter;      // End marker guid (if any)
        anim_group::handle  hAnimGroup;             // Animation group
        s32                 AnimGroupName;          // Animation group name
        s32                 AnimName;               // Animation name
        u32                 AnimFlags;              // Animation flags
        s32                 NextAiState;            // AI State to change too at end..
        anim_group::handle  hNextAnimGroup;         // Next animation group to resume back to
        s32                 iNextAnim;              // Next animation to resume back to

        cinema_character();
        void Init( void );
    };

public:

    CREATE_RTTI( cinema_object, object, object )

                                    cinema_object               ( void );
                                   ~cinema_object               ( void );

    virtual         bbox            GetLocalBBox                ( void ) const;
    virtual const   object_desc&    GetTypeDesc                 ( void ) const;
    static  const   object_desc&    GetObjectType               ( void );
    virtual         s32             GetMaterial                 ( void ) const { return MAT_TYPE_NULL; }

    virtual			void	        OnEnumProp		            ( prop_enum& List );
    virtual			xbool	        OnProperty		            ( prop_query& I );
    virtual         void            OnAdvanceLogic              ( f32 DeltaTime );
    virtual         void            OnActivate                  ( xbool Flag );
    
                    void            StartAnimAudioTimer         ( void );
                    xbool           IsPast                      ( char* Marker );
                    void            ProcessMarkers              ( void );
                    void            ResumeCharacterAi           ( cinema_character& CinChar );
                    void            ProcessAnims                ( void );
                    void            StartAnims                  ( void );
                    void            UpdateAnims                 ( void );
                    void            StopAnims                   ( void );
                    xbool           AreAnimsDone                ( void );
                    void            PopCharactersToEndMarkers   ( void );
                    
#ifdef X_EDITOR
                    void            UpdateEnumString            ( void );
                    char**          GetEnumString               ( void );
    virtual         s32             OnValidateProperties        ( xstring& ErrorMsg );
    virtual         void            OnDebugRender               ( void );

#endif // X_EDITOR

protected:

#ifdef X_EDITOR
    char*                       m_pEnumString;
#endif
    rhandle<char>               m_AudioPackage;
    f32                         m_AudioLength;
    xbool                       m_bCinemaActive;
    xbool                       m_bCinemaDone;
    xbool                       m_bIs2D;
    audio_status                m_AudioStatus;
    anim_status                 m_AnimStatus;
    s32                         m_VoiceID;
    char                        m_Descriptor[64];
    anim_audio_timer            m_Timer;
    f32                         m_PrevTime;
    f32                         m_CurrTime;
    s32                         m_nPositionMarkers;
    xarray<position_marker>     m_PositionMarkers;
    s32                         m_PositionMarkerIndex;
    s32                         m_nActionMarkers;
    xarray<action_marker>       m_ActionMarkers;  
    object_affecter             m_PlayerEndMarker;
    s32                         m_nCharacters;
    xarray<cinema_character>    m_Characters;
};
#endif // CINEMA_HPP

