//==============================================================================
//
//  EventMgr.cpp
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================

#include "EventMgr.hpp"
#include "Objects\Event.hpp"
#include "Sound\EventSoundEmitter.hpp"
#include "Objects\ParticleEventEmitter.hpp"
#include "Animation\AnimData.hpp"
#include "Objects\Player.hpp"
#include "Debris\Debris_mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================


//==============================================================================
//  DEFINES
//==============================================================================

#if !defined(X_RETAIL)
#define LOG_PARTICLE_EVENTS
#define LOG_AUDIO_EVENTS
#else
#undef LOG_PARTICLE_EVENTS
#undef LOG_AUDIO_EVENTS
#endif

//==============================================================================
//  STORAGE
//==============================================================================
event_mgr   g_EventMgr;

//==============================================================================
//  EXTERNS
//==============================================================================

s32 pain_event::CurrentEventID = 0;

//==============================================================================
//  FUNCTIONS
//==============================================================================


//==============================================================================

event_mgr::event_mgr( void )
{
#if !defined(X_RETAIL)
    m_bLogAudio = FALSE;
    m_bLogParticle = FALSE;
#endif
}


//==============================================================================

event_mgr::~event_mgr( void )
{
}

//==============================================================================

void event_mgr::HandleSuperEvents( loco_char_anim_player& CharAnimPlayer, loco_anim_controller& LocoAnimController, object* pObj )
{
    s32 i = 0;
       
    for (i = 0; i < LocoAnimController.GetNEvents(); i++)
    {
        // Send this event?
        if ( LocoAnimController.IsEventActive(i) )
        {
            // Lookup event and world position
            const anim_event& Event = LocoAnimController.GetEvent(i);
            HandleAnimEvents( Event, pObj, i, CharAnimPlayer );
        }
    }
}

//==============================================================================

void event_mgr::HandleSuperEvents( loco_char_anim_player& CharAnimPlayer, object* pObj )
{
    s32 i = 0;
        
    for (i = 0; i < CharAnimPlayer.GetNEvents(); i++)
    {
        // Send this event?
        if ( CharAnimPlayer.IsEventActive(i) )
        {
            const anim_event& Event = CharAnimPlayer.GetEvent(i);
            HandleAnimEvents( Event, pObj, i, CharAnimPlayer );
        }
    }

}

//==============================================================================

void event_mgr::HandleSuperEvents( char_anim_player& CharAnimPlayer, object* pObj )
{
    s32 i = 0;

    for (i = 0; i < CharAnimPlayer.GetNEvents(); i++)
    {
        // Send this event?
        if ( CharAnimPlayer.IsEventActive(i) )
        {
            // Lookup event and world position
            const anim_event& Event = CharAnimPlayer.GetEvent(i);
            HandleAnimEvents( Event, pObj, i, CharAnimPlayer );
        }
    }
}

//==============================================================================

void event_mgr::HandleSuperEvents( simple_anim_player& SimpleAnimPlayer, object* pObj )
{
    s32 i = 0;
        
    if( SimpleAnimPlayer.GetAnimGroup() == NULL )
        return;

    for (i = 0; i < SimpleAnimPlayer.GetNEvents(); i++)
    {
        // Send this event?
        if( SimpleAnimPlayer.IsEventActive(i) )
        {
            // Lookup event and world position
            const anim_event& Event = SimpleAnimPlayer.GetEvent(i);
            HandleAnimEvents( Event, pObj, i, SimpleAnimPlayer );
        }
    }
}

//==============================================================================

void event_mgr::HandleAnimEvents( const anim_event& AnimEvent, object* pObj, s32 EventIndex, base_player& BasePlayer )
{
    const char* EventType = AnimEvent.GetType();

    if (x_strcmp(EventType, "Old Event") == 0)
        return;

    if (x_strcmp(EventType, "Audio") == 0)
    {
        //-------------------------------------------------------
        // Audio
        //-------------------------------------------------------

        audio_event AudioEvent;
        AudioEvent.Type         = event::EVENT_AUDIO;
        AudioEvent.bUsePosition = FALSE;
        x_strncpy( AudioEvent.DescriptorName, AnimEvent.GetString( anim_event::STRING_IDX_AUDIO_SOUND_ID ), 64 );

        //-------------------------------------------------------
        // Produce the position if needed
        //-------------------------------------------------------
        vector3 Position;
        xbool UsePosition = FALSE;
        const char* Location = AnimEvent.GetString( anim_event::STRING_IDX_AUDIO_LOCATION );
        
        if ( (x_strlen( Location ) == 0) || !x_stricmp( Location, "Center of Object" ) )
        {
            Position = pObj->GetPosition();//pObj->GetBBox().GetCenter();
            UsePosition = TRUE;
        }
        else if ( !x_stricmp( Location, "Center of Event" ) )
        {
            Position                = BasePlayer.GetEventPosition( AnimEvent );
            UsePosition             =
            AudioEvent.bUsePosition = TRUE;
        }
        else if ( !x_stricmp( Location, "Globally" ) )
        {
            Position.Zero();
            UsePosition = FALSE;
        }
        
        AudioEvent.Position = Position;

        //-------------------------------------------------------
        // Get the type of audio event.
        //-------------------------------------------------------

        const char* Type = AnimEvent.GetString( anim_event::STRING_IDX_AUDIO_TYPE );
        
        if ( (x_strlen( Type ) == 0) || !x_stricmp( Type, "One Shot" ) )
        {        
            AudioEvent.SoundType = event_sound_emitter::SINGLE_SHOT;
        }
        else if( !x_stricmp( Type, "Looped" ) )
        {
            AudioEvent.SoundType = event_sound_emitter::LOOPED;
        }
        else if( !x_stricmp( Type, "Contact" ) )
        {        
            AudioEvent.SoundType = event_sound_emitter::CONTACT;
        }
        else
        {        
            ASSERTS(FALSE,xfs("Animation Audio Event is of invalid type") );
        }

        AudioEvent.Flag = AnimEvent.GetInt( anim_event::INT_IDX_AUDIO_DATA );
        
        #ifdef LOG_AUDIO_EVENTS
        {
            if( pObj && m_bLogAudio )
            {
                LOG_MESSAGE("AudioEvent","(%08X:%08X) (%2d) (%s) (%s) (%s)\n",
                    (u32)(pObj->GetGuid()>>32),
                    (u32)(pObj->GetGuid()),
                    EventIndex,
                    (const char*)AudioEvent.DescriptorName,
                    pObj->GetLogicalName(),
                    BasePlayer.GetAnimName()
                    );
            }
        }
        #endif

        HandleAudioEvent( AudioEvent, pObj, UsePosition );
    }
    else if(x_strcmp(EventType, "Particle") == 0)
    {
        //-------------------------------------------------------
        // Particle
        //-------------------------------------------------------

        particle_event ParticleEvent;
        
        ParticleEvent.Type = event::EVENT_PARTICLE;
        x_strncpy( ParticleEvent.ParticleName, AnimEvent.GetString( anim_event::STRING_IDX_HOTPOINT_TYPE ), 64 );
        
        ParticleEvent.EventActive        = AnimEvent.GetBool( anim_event::BOOL_IDX_PARTICLE_EVENT_ACTIVE );
        ParticleEvent.DoNotAppyTransform = AnimEvent.GetBool( anim_event::BOOL_IDX_PARTICLE_DONOT_APPLY_TRANSFORM );

        //-------------------------------------------------------
        // Produce the position if needed
        //-------------------------------------------------------
        ParticleEvent.Position = BasePlayer.GetEventPosition( AnimEvent );//.Set( EventPos.X, EventPos.Y, EventPos.Z );
       
        //-------------------------------------------------------
        // Get the orientation if needed
        //-------------------------------------------------------

        ParticleEvent.Orientation = BasePlayer.GetEventRotation( AnimEvent );//EventRotation;

        ParticleEvent.EventIndex  = EventIndex;
        
        HandleParticleEvent( ParticleEvent, pObj );

        #ifdef LOG_PARTICLE_EVENTS
        {
            if( pObj && m_bLogParticle )
            {       
                LOG_MESSAGE("ParticleEvent","(%08X:%08X) (%2d) (%s) (%s) (%s)\n",
                    (u32)(pObj->GetGuid()>>32),
                    (u32)(pObj->GetGuid()),
                    EventIndex,
                    (const char*)ParticleEvent.ParticleName,
                    pObj->GetLogicalName(),
                    BasePlayer.GetAnimName()
                    );
            }
        }
        #endif

    }
    else if(x_strcmp(EventType, "Hot Point") == 0)
    {
        //-------------------------------------------------------
        // Hot Points
        //-------------------------------------------------------

        hotpoint_event HotPointEvent;
        
        HotPointEvent.Type = event::EVENT_HOTPOINT;
        x_strncpy( HotPointEvent.HotPointType, AnimEvent.GetString( anim_event::STRING_IDX_HOTPOINT_TYPE ), 64 );

        //-------------------------------------------------------
        // Get the position + orientation if needed
        //-------------------------------------------------------        
        HotPointEvent.Position = BasePlayer.GetEventPosition( AnimEvent );//.Set( EventPos.X, EventPos.Y, EventPos.Z );
        HotPointEvent.Orientation = BasePlayer.GetEventRotation( AnimEvent );//EventRotation;

        //HotPointEvent.Orientation.Rotate( AnimEvent.GetPoint( anim_event::POINT_IDX_ROTATION ) );

        HandleHotPointEvent( HotPointEvent, pObj );
    }
    else if(x_strcmp(EventType, "Generic") == 0)
    {
        //-------------------------------------------------------
        // Generic
        //-------------------------------------------------------

        generic_event GenericEvent;
        
        GenericEvent.Type = event::EVENT_GENERIC;
        x_strncpy( GenericEvent.GenericType, AnimEvent.GetString( anim_event::STRING_IDX_GENERIC_TYPE ), 64 );
        GenericEvent.EventIndex = EventIndex;
        
        HandleGenericEvent( GenericEvent, pObj );
    }
    else if(x_strcmp(EventType, "Intensity") == 0)
    {
        //-------------------------------------------------------
        // Intensity
        //-------------------------------------------------------

        intensity_event IntensityEvent;

        //-------------------------------------------------------
        // Intensity controller event.
        //-------------------------------------------------------

        IntensityEvent.Type                 = event::EVENT_INTENSITY;
        IntensityEvent.ControllerIntensity  = MIN( AnimEvent.GetFloat( anim_event::FLOAT_IDX_CONTROLLER_INTENSITY ), 1.0f );
        IntensityEvent.ControllerDuration   = AnimEvent.GetFloat( anim_event::FLOAT_IDX_CONTROLLER_DURATION );
        IntensityEvent.CameraShakeTime      = AnimEvent.GetFloat( anim_event::FLOAT_IDX_CAMERA_SHAKE_TIME );
        IntensityEvent.CameraShakeAmount    = AnimEvent.GetFloat( anim_event::FLOAT_IDX_CAMERA_SHAKE_AMOUNT );
        IntensityEvent.CameraShakeSpeed     = AnimEvent.GetFloat( anim_event::FLOAT_IDX_CAMERA_SHAKE_SPEED );
        IntensityEvent.BlurIntensity        = AnimEvent.GetFloat( anim_event::FLOAT_IDX_BLUR_INTENSITY );
        IntensityEvent.BlurDuration         = AnimEvent.GetFloat( anim_event::FLOAT_IDX_BLUR_DURATION );

        HandleIntensityEvent( IntensityEvent, pObj );
    }
    else if( x_strcmp( EventType, "World Collision" ) == 0 )
    {
        //-------------------------------------------------------
        // World Collision
        //-------------------------------------------------------

        world_collision_event WorldCollisionEvent ;

        WorldCollisionEvent.Type                = event::EVENT_WORLD_COLLISION ;
        WorldCollisionEvent.bWorldCollisionOn   = AnimEvent.GetBool( anim_event::BOOL_IDX_WORLD_COLLISION );

        pObj->OnEvent( WorldCollisionEvent );
    }
    else if( x_strcmp( EventType, "Gravity" ) == 0 )
    {
        //-------------------------------------------------------
        // Gravity
        //-------------------------------------------------------

        gravity_event GravityEvent;
        
        GravityEvent.Type       = event::EVENT_GRAVITY;
        GravityEvent.bGravityOn = AnimEvent.GetBool( anim_event::BOOL_IDX_GRAVITY );

        pObj->OnEvent( GravityEvent );
    }
    else if( x_strcmp( EventType, "Weapon" ) == 0 )
    {
        weapon_event WeaponEvent;

        WeaponEvent.Pos         = BasePlayer.GetEventPosition( AnimEvent );//EventPos;
        WeaponEvent.Type        = event::EVENT_WEAPON;
        WeaponEvent.WeaponState = AnimEvent.GetInt( anim_event::INT_IDX_WEAPON_DATA );

        pObj->OnEvent( WeaponEvent );
    }
    else if( x_strcmp( EventType, "Pain" ) == 0 )
    {
        pain_event PainEvent;

        //-------------------------------------------------------
        // Pain event.
        //-------------------------------------------------------

        PainEvent.Position      = BasePlayer.GetEventPosition( AnimEvent );//EventPos;
        PainEvent.Rotation      = AnimEvent.GetPoint( anim_event::POINT_IDX_ROTATION ); 
        PainEvent.PainRadius    = AnimEvent.GetFloat( anim_event::FLOAT_IDX_RADIUS );
        PainEvent.Type          = event::EVENT_PAIN;
        PainEvent.PainType      = AnimEvent.GetInt( anim_event::INT_IDX_PAIN_TYPE );
/*        const char *painTypeName      = AnimEvent.GetString( anim_event::STRING_IDX_PAIN_TYPE );

        if( x_strlen( painTypeName ) == 0 || x_strcmp( painTypeName, "Melee" ) == 0)
        {
            PainEvent.PainType  = pain_event::EVENT_PAIN_MELEE ;
        }
        else if( x_strcmp( painTypeName, "Leap-Charge" ) == 0 )
        {
            PainEvent.PainType  = pain_event::EVENT_PAIN_LEAP_CHARGE ;
        }
        else if( x_strcmp( painTypeName, "Special" ) == 0 )
        {
            PainEvent.PainType  = pain_event::EVENT_PAIN_SPECIAL ;
        }
        else
        {
            ASSERTS( FALSE,"Compiling a Super Pain Event with a bad pain type" );
        }
*/
        HandlePainEvent( PainEvent, pObj );
    }
    else if( x_strcmp( EventType, "Debris" ) == 0 )
    {
        //-------------------------------------------------------
        // Debris
        //-------------------------------------------------------

        debris_event DebrisEvent;
        
        DebrisEvent.Position    = BasePlayer.GetEventPosition( AnimEvent );
        DebrisEvent.Orientation = BasePlayer.GetEventRotation( AnimEvent );
        DebrisEvent.MinVelocity = AnimEvent.GetFloat( anim_event::FLOAT_IDX_DEBRIS_MIN_VELOCITY );
        DebrisEvent.MaxVelocity = AnimEvent.GetFloat( anim_event::FLOAT_IDX_DEBRIS_MAX_VELOCITY );
        DebrisEvent.Life        = AnimEvent.GetFloat( anim_event::FLOAT_IDX_DEBRIS_LIFE );
        DebrisEvent.Bounce      = AnimEvent.GetBool( anim_event::BOOL_IDX_DEBRIS_BOUNCE );
        
        x_strncpy( DebrisEvent.MeshName, xfs( "%s.rigidgeom", AnimEvent.GetString( anim_event::STRING_IDX_DEBRIS_TYPE )), 64 );

        HandleDebrisEvent( DebrisEvent, pObj );
    }
    else if ( x_strcmp( EventType, "Set Mesh" ) == 0 )
    {
        // Turn on the mesh
        set_mesh_event SetMeshEvent;
        
        x_strncpy( SetMeshEvent.MeshName, AnimEvent.GetString( anim_event::STRING_IDX_SET_MESH ), 64 );
        SetMeshEvent.On = ( x_strcmp( AnimEvent.GetString( anim_event::STRING_IDX_SET_MESH_ON_OR_OFF ), "On" ) == 0 );

        HandleSetMeshEvent( SetMeshEvent, pObj );
    }
    else if ( x_strcmp( EventType, "Swap Mesh" ) == 0 )
    {
        swap_mesh_event SwapMeshEvent;
        x_strncpy( SwapMeshEvent.OnMeshName,  AnimEvent.GetString( anim_event::STRING_IDX_SWAP_MESH_ON  ), 64 );
        x_strncpy( SwapMeshEvent.OffMeshName, AnimEvent.GetString( anim_event::STRING_IDX_SWAP_MESH_OFF ), 64 );
        
        HandleSwapMeshEvent( SwapMeshEvent, pObj );
    }
    else if ( x_strcmp( EventType, "Swap Virtual Texture" ) == 0 )
    {
        swap_virtual_texture_event SwapVTEvent;
        x_strncpy( SwapVTEvent.SetTextureName, AnimEvent.GetString( anim_event::STRING_IDX_SET_TEXTURE  ), 64 );
        x_strncpy( SwapVTEvent.UseTextureName, AnimEvent.GetString( anim_event::STRING_IDX_USE_TEXTURE ), 64 );

        HandleSetVirtualTextureEvent( SwapVTEvent, pObj );
    }
    else if ( x_strcmp( EventType, "Fade Geometry" ) == 0 )
    {
        fade_geometry_event FadeGeomEvent;
        FadeGeomEvent.Direction = AnimEvent.GetInt( anim_event::INT_IDX_FADE_DIRECTION );
        FadeGeomEvent.FadeTime  = AnimEvent.GetFloat( anim_event::FLOAT_IDX_GEOMETRY_FADE_TIME );

        HandleFadeGeometryEvent( FadeGeomEvent, pObj );
    }
    else if ( x_strcmp( EventType, "Camera FOV" ) == 0 )
    {
        camera_fov_event CameraFOVEvent;
        CameraFOVEvent.CameraFOV    = AnimEvent.GetFloat( anim_event::FLOAT_IDX_CAMERA_FOV );
        CameraFOVEvent.CameraTime   = AnimEvent.GetFloat( anim_event::FLOAT_IDX_CAMERA_FOV_TIME );

        HandleCameraFOVEvent( CameraFOVEvent, pObj );
    }
}

//==============================================================================

void event_mgr::HandleAudioEvent( const event& Event, object* pParentObj, xbool UsePosition )
{
    const audio_event& AudioEvent = audio_event::GetSafeType( Event );
    //    
    // If its a looped sounds then we want to check if it already exist, if it does then
    // update its position instead of creating a new one.
    //
    if( AudioEvent.SoundType == event_sound_emitter::LOOPED )
    {
        if ( UsePosition )
        {
            slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_EVENT_SND_EMITTER );
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

            while( pObject )
            {
                event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pObject );
                if( (EventEmitter.GetParentGuid() == pParentObj->GetGuid()) && 
                  (x_strcmp(EventEmitter.GetDescriptorName(), (const char*)AudioEvent.DescriptorName) == 0) )
                    break;

                SlotID = g_ObjMgr.GetNext( SlotID );
                pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            }
        
            // Update the emitter.
            if ( pObject == NULL )
            {
                guid Guid = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
                pObject = g_ObjMgr.GetObjectByGuid( Guid );
                ASSERT( pObject );


                if ( pObject )
                {
                    event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pObject);
               
                    vector3 temp = AudioEvent.Position;     
                    EventEmitter.PlayEmitter( AudioEvent.DescriptorName,
                                            temp, 
                                            pParentObj->GetZone1(),
                                            (event_sound_emitter::sound_type)AudioEvent.SoundType, 
                                            pParentObj->GetGuid(), 
                                            AudioEvent.Flag,
                                            0.0f,
                                            FALSE,
                                            1.0f,
                                            !UsePosition ); 
                }
            }

            event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pObject );
            // Check if we can calculate a new bone position
            if( AudioEvent.bUsePosition )
            {
                EventEmitter.OnMove( AudioEvent.Position );
            }
            else
            {
                EventEmitter.OnMove( pParentObj->GetSubPosition(0) );
            }
            return;
        }
    }
    else 
    {
        //
        // Create a new event sound emitter object.
        //
        guid Guid = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
        object* pSndObj = g_ObjMgr.GetObjectByGuid( Guid );
        event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pSndObj );
   
            vector3 temp = AudioEvent.Position;     
            EventEmitter.PlayEmitter( AudioEvent.DescriptorName,
                                      temp, pParentObj->GetZone1(),
                                      (event_sound_emitter::sound_type)AudioEvent.SoundType, 
                                      pParentObj->GetGuid(), 
                                      AudioEvent.Flag,
                                      0.0f,
                                      FALSE,
                                      1.0f,
                                      !UsePosition ); 
    }
}

//==============================================================================

void event_mgr::HandleParticleEvent( const event& Event, object* pParentObj )
{
    const particle_event& ParticleEvent = particle_event::GetSafeType( Event );

    //
    // Check if a particle event for this object has already been created.
    //
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PARTICLE_EVENT_EMITTER );
    object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

    while( pObject )
    {
        particle_event_emitter& ParticleEventEmitter = particle_event_emitter::GetSafeType( *pObject );
        
        // If the particle emitter has the same parent, name and event id then that mean we just have to update it.
        if( (ParticleEventEmitter.GetParentGuid() == pParentObj->GetGuid()) && 
          (x_strcmp(ParticleEventEmitter.GetFxName(), (const char*)ParticleEvent.ParticleName) == 0) &&
          (ParticleEvent.EventIndex == ParticleEventEmitter.GetEventID()) )
            break;

        SlotID = g_ObjMgr.GetNext( SlotID );
        pObject = g_ObjMgr.GetObjectBySlot( SlotID );
    }
        
    // Update the emitter.
    if( pObject != NULL )
    {
        particle_event_emitter& ParticleEventEmitter = particle_event_emitter::GetSafeType( *pObject );
        
        if( ParticleEvent.DoNotAppyTransform == FALSE )
        {
            m_Tranform.Identity();

            //radian3 Orient;
            //Orient.Roll = R_0;
            //ParticleEvent.Orientation.GetPitchYaw( Orient.Pitch, Orient.Yaw );

            m_Tranform.SetTranslation ( ParticleEvent.Position    );
            m_Tranform.SetRotation    ( ParticleEvent.Orientation );

            ParticleEventEmitter.OnTransform( m_Tranform );
        }

        ParticleEventEmitter.EnableUpdate();

        return;
    }
    else
    {
        // Create a particle event object.
        guid Guid = g_ObjMgr.CreateObject( particle_event_emitter::GetObjectType() );
        object* pPrtEvtObj = g_ObjMgr.GetObjectByGuid( Guid );
        particle_event_emitter& ParticleEventEmitter = particle_event_emitter::GetSafeType( *pPrtEvtObj );
    
        ParticleEventEmitter.StartEmitter( ParticleEvent.ParticleName, ParticleEvent.Position,
                                            ParticleEvent.Orientation, pParentObj->GetZone1(), pParentObj->GetGuid(),
                                            ParticleEvent.EventIndex, ParticleEvent.EventActive ); 
    }
}

//==============================================================================

void event_mgr::HandleHotPointEvent( const event& Event, object* pParentObj )
{
    const hotpoint_event& HotPointEvent= hotpoint_event::GetSafeType( Event );
    (void)HotPointEvent;
    (void)pParentObj;
}

//==============================================================================

void event_mgr::HandleGenericEvent( const event& Event, object* pParentObj )
{    
//    const generic_event& GenericEvent = generic_event::GetSafeType( Event );
    pParentObj->OnEvent( Event );
}

//==============================================================================

void event_mgr::HandleIntensityEvent( const event& Event, object* pParentObj )
{
    const intensity_event& IntensityEvent = intensity_event::GetSafeType( Event );

    if( pParentObj && pParentObj->IsKindOf( new_weapon::GetRTTI() ) )
    {
        new_weapon* pWeapon = (new_weapon*)pParentObj;
        object* pObj = g_ObjMgr.GetObjectByGuid( pWeapon->GetParentGuid() );

        if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
        {
            player* pPlayer = (player*)pObj;
            pPlayer->OnEvent( IntensityEvent );
            return;
        }
    }
    else if( pParentObj && pParentObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pParentObj;
        pPlayer->OnEvent( IntensityEvent );
        return;
    }

    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( pPlayer )
        pPlayer->OnEvent( IntensityEvent );
}

//==============================================================================
#include "Objects\Player.hpp"

void event_mgr::HandleDebrisEvent( const event& Event, object* pParentObj )
{
    s32 BounceSound = -1;
    (void)pParentObj;    
    const debris_event& DebrisEvent = debris_event::GetSafeType( Event );

    // The vector needs to start facing Y up since that represents -Z in max and that the axis we are shooting down.
    vector3 RotateVector( 0.0f, 1.0f, 0.0f );
    RotateVector.Rotate( DebrisEvent.Orientation );
    //( DebrisEvent.Orientation.Pitch, DebrisEvent.Orientation.Yaw, DebrisEvent.Orientation.Roll );
    
    RotateVector.NormalizeAndScale( x_frand( DebrisEvent.MinVelocity, DebrisEvent.MaxVelocity ) );

    vector3 InheritedVelocity( 0.0f, 0.0f, 0.0f );
    if ( pParentObj && pParentObj->IsKindOf( new_weapon::GetRTTI() ) )
    {
        new_weapon& Weapon = *((new_weapon*)pParentObj); 
        InheritedVelocity = Weapon.GetVelocity();

        const f32 Radius = (DebrisEvent.Position - Weapon.GetPosition()).Length();
        const f32 ArcSpeed = Radius * Weapon.GetAngularSpeed(); // s = r(Theta)
        const f32 Yaw = Weapon.GetYaw();
        vector3 Dir;
        if ( ArcSpeed > 0 )
        {
            Dir.Set( 1.0f, 0.0f, 0.0f );
        }
        else
        {
            Dir.Set( -1.0f, 0.0f, 0.0f );
        }

        Dir.RotateY( Yaw );
        InheritedVelocity += (Dir * ArcSpeed);

        if( pParentObj->GetType() == object::TYPE_WEAPON_SHOTGUN )
        {
            BounceSound = g_StringMgr.Add("SHT_ShellDropConcrete");
        }
        else
        {
            BounceSound = g_StringMgr.Add("SMP_ShellDropConcrete");
        }
    }
    static const f32 ONE_FRAME = 0.03f;
    debris_mgr::GetDebrisMgr()->CreateDebris(   DebrisEvent.Position + (InheritedVelocity * ONE_FRAME),
                                                pParentObj->GetZones(),
                                                InheritedVelocity + RotateVector,
                                                DebrisEvent.MeshName,
                                                DebrisEvent.Life,
                                                DebrisEvent.Bounce,
                                                U32_MAX,
                                                BounceSound );
}

//==============================================================================

void event_mgr::HandlePainEvent( event& Event, object* pParentObj )
{
    pain_event& PainSuperEvent = pain_event::GetSafeType( Event );
    pParentObj->OnEvent( PainSuperEvent );
}

//==============================================================================

f32 event_mgr::ClosestPointToAABox (const vector3& Point, const bbox& Box, vector3& ClosestPoint)
{
    f32 SqrDistance = 0.0f;
    f32 Delta;

    if ( Point.GetX() <= Box.Min.GetX() )
    {
        Delta = Point.GetX() - Box.Min.GetX();
        SqrDistance += Delta*Delta;
        ClosestPoint.GetX() = Box.Min.GetX();
    }
    else if ( Point.GetX() > Box.Max.GetX() )
    {
        Delta = Point.GetX() - Box.Max.GetX();
        SqrDistance += Delta*Delta;
        ClosestPoint.GetX() = Box.Max.GetX();
    }
    else
    {
        ClosestPoint.GetX() = Point.GetX();
    }

    if ( Point.GetY() <= Box.Min.GetY() )
    {
        Delta = Point.GetY() - Box.Min.GetY();
        SqrDistance += Delta*Delta;
        ClosestPoint.GetY() = Box.Min.GetY();
    }
    else if ( Point.GetY() > Box.Max.GetY() )
    {
        Delta = Point.GetY() - Box.Max.GetY();
        SqrDistance += Delta*Delta;
        ClosestPoint.GetY() = Box.Max.GetY();
    }
    else
    {
        ClosestPoint.GetY() = Point.GetY();
    }

    if ( Point.GetZ() <= Box.Min.GetZ() )
    {
        Delta = Point.GetZ() - Box.Min.GetZ();
        SqrDistance += Delta*Delta;
        ClosestPoint.GetZ() = Box.Min.GetZ();
    }
    else if ( Point.GetZ() > Box.Max.GetZ() )
    {
        Delta = Point.GetZ() - Box.Max.GetZ();
        SqrDistance += Delta*Delta;
        ClosestPoint.GetZ() = Box.Max.GetZ();
    }
    else
    {
        ClosestPoint.GetZ() = Point.GetZ();
    }


    return SqrDistance;
}

//=============================================================================

void event_mgr::HandleSetMeshEvent( const event& Event, object* pParentObj )
{
    if ( pParentObj )
    {
        const set_mesh_event& SetMeshEvent = set_mesh_event::GetSafeType( Event );
        geom*                 pGeom        = pParentObj->GetGeomPtr();
        render_inst*          pRenderInst  = pParentObj->GetRenderInstPtr();

        if ( pGeom && pRenderInst )
        {
            s32 Index = pGeom->GetVMeshIndex( SetMeshEvent.MeshName );
            pRenderInst->SetVMeshBit( Index, SetMeshEvent.On );
        }
    }
}

//=============================================================================

void event_mgr::HandleSwapMeshEvent( const event& Event, object* pParentObj )
{
    if ( pParentObj )
    {
        const swap_mesh_event& SwapMeshEvent = swap_mesh_event::GetSafeType( Event );
        geom*           pGeom                = pParentObj->GetGeomPtr();
        render_inst*    pRenderInst          = pParentObj->GetRenderInstPtr();

        if ( pGeom && pRenderInst )
        {
            s32 Index = pGeom->GetVMeshIndex( SwapMeshEvent.OnMeshName );
            pRenderInst->SetVMeshBit( Index, TRUE );

            Index = pGeom->GetVMeshIndex( SwapMeshEvent.OffMeshName );
            pRenderInst->SetVMeshBit( Index, FALSE );
        }
    }
}

//=============================================================================

void event_mgr::HandleSetVirtualTextureEvent( const event& Event, object* pParentObj )
{
    if ( pParentObj )
    {
        const swap_virtual_texture_event &SwapVTEvent = swap_virtual_texture_event::GetSafeType( Event );
        render_inst*    pRenderInst          = pParentObj->GetRenderInstPtr();

        // this only works for actors.
        if ( pRenderInst && pParentObj->IsKindOf(actor::GetRTTI()) )
        {
            skin_inst *pSkinInst = (skin_inst*)pRenderInst;
            pSkinInst->SetVirtualTexture(SwapVTEvent.SetTextureName,SwapVTEvent.UseTextureName);
        }
    }
}

//=============================================================================

void event_mgr::HandleFadeGeometryEvent( const event& Event, object* pParentObj )
{
    if( pParentObj )
    {
        const fade_geometry_event& FadeGeomEvent = fade_geometry_event::GetSafeType( Event );
        render_inst*               pRenderInst   = pParentObj->GetRenderInstPtr();

        if ( pRenderInst )
        {
            pRenderInst->StartFade( (s8)FadeGeomEvent.Direction, FadeGeomEvent.FadeTime );
        }
    }
}

//=============================================================================

void event_mgr::HandleCameraFOVEvent( const event& Event, object* pParentObj )
{
    (void)pParentObj;
    (void)Event;
    // handle the event here I'm guessin.
}
