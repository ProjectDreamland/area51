//==============================================================================
//  Event.hpp
//==============================================================================

#ifndef EVENT_HPP
#define EVENT_HPP


//==============================================================================
//  INCLUDES
//==============================================================================

#include "MiscUtils\RTTI.hpp"

//==============================================================================
//  DEFINES
//==============================================================================
#define GUN_SHOT_EVENT      1
#define EXPLOSION_EVENT     2
#define VOICE_EVENT         3

//==============================================================================
//  TYPES
//==============================================================================
    
    struct event
    {
        CREATE_RTTI_BASE( event );

        enum event_type
        {
            EVENT_AUDIO,
            EVENT_PARTICLE,
            EVENT_HOTPOINT,
            EVENT_VOICE,
            EVENT_GENERIC,
            EVENT_INTENSITY,
            EVENT_WORLD_COLLISION,
            EVENT_GRAVITY,
            EVENT_WEAPON,
            EVENT_PAIN,
            EVENT_DIALOG,
            EVENT_TEXTURE_SWAP,
            EVENT_CAMERA_FOV,
        }; 

        event_type  Type;
    };
    
    //--------------------------------------------------------------------

    struct audio_event : public event
    {
        CREATE_RTTI( audio_event, event, event );
        
        char        DescriptorName[64];
        vector3     Position;
        u32         SoundType;
        u32         Flag;
        xbool       bUsePosition;
    };

    //--------------------------------------------------------------------

    struct particle_event : public event
    {
        CREATE_RTTI( particle_event, event, event );
        
        char        ParticleName[64];
        vector3     Position;
        radian3     Orientation;
        s32         lParams;
        u32         Flags;
        s32         EventIndex;
        xbool       EventActive;
        xbool       DoNotAppyTransform;
    };

    //--------------------------------------------------------------------

    struct hotpoint_event : public event
    {
        CREATE_RTTI( hotpoint_event, event, event );
        
        char        HotPointType[64];
        vector3     Position;
        vector3     Orientation;
    };

    //--------------------------------------------------------------------

    struct voice_event : public event
    {
        CREATE_RTTI( voice_event, event, event );

        s32         VoiceID;
    };

    //--------------------------------------------------------------------

    struct dialog_event : public event
    {
        CREATE_RTTI( dialog_event, event, event );

        xbool       HotVoice;
        char        DialogName[64];
    };

    //--------------------------------------------------------------------
    struct generic_event : public event
    {
        CREATE_RTTI( generic_event, event, event );

        char        GenericType[64];
        s32         EventIndex;
    };

    //--------------------------------------------------------------------

    struct intensity_event : public event
    {
        CREATE_RTTI( intensity_event, event, event );

        f32         ControllerIntensity;
        f32         ControllerDuration;
        f32         CameraShakeTime;
        f32         CameraShakeAmount;
        f32         CameraShakeSpeed;
        f32         BlurIntensity;
        f32         BlurDuration;
    };

    //--------------------------------------------------------------------

    struct world_collision_event : public event
    {
        CREATE_RTTI( world_collision_event, event, event );

        xbool       bWorldCollisionOn;
    };
    
    //--------------------------------------------------------------------

    struct gravity_event : public event
    {
        CREATE_RTTI( gravity_event, event, event );

        xbool       bGravityOn;
    };
    
    //--------------------------------------------------------------------

    struct weapon_event : public event
    {
        CREATE_RTTI( weapon_event, event, event );

        vector3   Pos;
        s32       WeaponState;
    };

    //--------------------------------------------------------------------

    struct pain_event : public event
    {
        CREATE_RTTI( pain_event, event, event );

        enum event_pain_type
        {
            EVENT_PAIN_MELEE,
            EVENT_PAIN_LEAP_CHARGE,
            EVENT_PAIN_SPECIAL,
        };
        vector3     Position;
        vector3     Rotation;
        f32         PainRadius;
        s32         PainType;

        s32         PainEventID;

static  s32         CurrentEventID;
    };

    //--------------------------------------------------------------------

    struct debris_event : public event
    {
        CREATE_RTTI( debris_event, event, event );

        char        MeshName[64];
        vector3     Position;
        radian3     Orientation;
        f32         MinVelocity;
        f32         MaxVelocity;
        f32         Life;
        xbool       Bounce;
    };

    //--------------------------------------------------------------------

    struct set_mesh_event : public event
    {
        CREATE_RTTI( set_mesh_event, event, event );

        char        MeshName[64];
        xbool       On;
    };

    //--------------------------------------------------------------------

    struct swap_mesh_event : public event
    {
        CREATE_RTTI( swap_mesh_event, event, event );
        
        char        OnMeshName[64];
        char        OffMeshName[64];
    };

    //--------------------------------------------------------------------

    struct fade_geometry_event : public event
    {
        CREATE_RTTI( fade_geometry_event, event, event );

        s32         Direction;
        f32         FadeTime;
    };

    //--------------------------------------------------------------------

    struct swap_virtual_texture_event : public event
    {
        CREATE_RTTI( swap_virtual_texture_event, event, event );

        char        SetTextureName[64];
        char        UseTextureName[64];
    };

    struct camera_fov_event : public event
    {
        CREATE_RTTI( camera_fov_event, event, event );

        f32         CameraFOV;
        f32         CameraTime;
    };

    //==============================================================================

#endif



