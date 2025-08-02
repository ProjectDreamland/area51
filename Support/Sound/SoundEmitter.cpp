//==============================================================================
// SOUND EMITTERS
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "SoundEmitter.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\CollisionPrimatives.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\ZoneMgr\ZoneMgr.hpp"
#include "gamelib\StatsMgr.hpp"

#define STREAM_EMITTER      (1<<1)
#define TRIGGER_ACTIVATE    (1<<2)
#define ACTIVATED           (1<<3)
#define CONTINUE            (1<<4)
#define DEACTIVATE          (1<<5)
#define DESTROY             (1<<6)


//=========================================================================
// GLOBALS
//=========================================================================

xcolor g_NearClipColor      =   XCOLOR_BLUE;
xcolor g_FarClipColor       =   XCOLOR_GREEN;
xcolor g_Trigger            =   XCOLOR_WHITE;
xcolor g_EmitterSel         =   XCOLOR_AQUA;
xcolor g_Emitter            =   XCOLOR_GREY;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
static struct sound_emitter_desc : public object_desc
{
    sound_emitter_desc( void ) : object_desc( 
        object::TYPE_SND_EMITTER, 
        "Sound Emitter", 
        "SOUND",

            object::ATTR_COLLIDABLE             |
            object::ATTR_COLLISION_PERMEABLE    |
            object::ATTR_BLOCKS_PLAYER          |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_SOUND_SOURCE,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC  ) {}         

    //---------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new sound_emitter;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR
    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        
        if( (Object. GetAttrBits() & object::ATTR_EDITOR_SELECTED) || 
            (Object.GetAttrBits() & object::ATTR_EDITOR_PLACEMENT_OBJECT) )
            EditorIcon_Draw(EDITOR_ICON_SPEAKER, Object.GetL2W(), TRUE, XCOLOR_WHITE );
        else
            EditorIcon_Draw(EDITOR_ICON_SPEAKER, Object.GetL2W(), FALSE, XCOLOR_WHITE );
        
        Object.OnDebugRender();

        return -1;
    }
#endif // X_EDITOR

} s_SoundEmitter_Desc;

//=========================================================================

const object_desc&  sound_emitter::GetTypeDesc( void ) const
{
    return s_SoundEmitter_Desc;
}

//=========================================================================

const object_desc&  sound_emitter::GetObjectType( void )
{
    return s_SoundEmitter_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

sound_emitter::sound_emitter( void ) 
{
    m_bForceActive  = FALSE;
    m_EmitterType   = POINT;
    m_EmitterShape  = SHAPE_ORIENTED_BOX;
    m_NearClip      = 0.25f;
    m_FarClip       = 0.25f;  
    m_Label[0]      = '\0';
    m_MinVolume     = 1.0f;
    m_MaxVolume     = 1.0f;
    m_CurrentVolume = 1.0f;
    m_MinInterval   = 0.0f;
    m_MaxInterval   = 0.01f;
    m_XScale        = 0.5f;
    m_YScale        = 0.5f;
    m_ZScale        = 0.5f;
    m_VoiceID       = 0;

    m_PitchDepth    = 0.0f;
    m_PitchSpeed    = 0.0f;
    m_PitchDepthVar = 0.0f;
    m_PitchSpeedVar = 0.0f;
    m_VolumeDepth   = 0.0f;
    m_VolumeSpeed   = 0.0f;
    m_VolumeDepthVar= 0.0f;
    m_VolumeSpeedVar= 0.0f;
    m_Flags         = 0;
    m_Flags         |= CONTINUE;
    m_ReleaseTime   = 0.0f;

    m_PitchBaseDepth    = 0.0f;
    m_PitchBaseSpeed    = 0.0f;
    m_VolumeBaseDepth   = 0.0f;
    m_VolumeBaseSpeed   = 0.0f;

    m_CurrentPitchTime  = 0.0f;
    m_CurrentVolumeTime = 0.0f;  

    m_AmbientTransition = 0.0f;
    m_VolumeFadeTime    = 1.0f;
    m_bReverbEnable     = FALSE;
    m_WetDryMix         = 0.20f;
    
    m_EnableEndingRoutineCheck = FALSE;
    
    g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );
    
    m_TriggerArea   = m_AudioMgrFarClip/5.0f;
    
    // Convert to meters, then multiply the emitter scale.
    m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
    m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;

    m_FarClipBox.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);
    m_NearClipBox.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);

    matrix4 Scale;

    Scale.Zero();
    Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

    // Set the scale and the object rotation.
    m_FarClipBox.Max = Scale * m_FarClipBox.Max;
    m_FarClipBox.Min = Scale * m_FarClipBox.Min;

    m_FarClipBox.Max = GetL2W() * m_FarClipBox.Max;
    m_FarClipBox.Min = GetL2W() * m_FarClipBox.Min;

    m_NearClipBox.Max = Scale * m_NearClipBox.Max;
    m_NearClipBox.Min = Scale * m_NearClipBox.Min;

    m_NearClipBox.Max = GetL2W() * m_NearClipBox.Max;
    m_NearClipBox.Min = GetL2W() * m_NearClipBox.Min;
    
    m_Collided = FALSE;

    m_Debug = TRUE;
  
    TestPoint.Set( 0.0f, 0.0f, 0.0f );
    TestPoint2.Set( 0.0f, 0.0f, 0.0f );
    TestPoint3.Set( 0.0f, 0.0f, 0.0f );

    NearDraw.Set( vector3( 0.0f,0.0f,0.0f), vector3( 0.0f,0.0f,0.0f) );
    FarDraw.Set( vector3( 0.0f,0.0f,0.0f), vector3( 0.0f,0.0f,0.0f) );

    m_IntervalTime = 0.1f;

    m_bCollisionActivate = TRUE;

    TurnAttrBitsOff( object::ATTR_NEEDS_LOGIC_TIME );
}
//==============================================================================

#ifdef X_EDITOR

s32 sound_emitter::OnValidateProperties( xstring& ErrorMsg )
{
    // Audio package not found?
    const char* pPackage = m_hAudioPackage.GetName();
    if( ( !pPackage ) || ( pPackage[0] == 0 ) || ( x_stricmp( pPackage, "<null>" ) == 0 ) )
    {
        ErrorMsg += "No audio package assigned.\n";
        return 1;
    }        

    // Audio package not loaded?
    if( m_hAudioPackage.GetPointer() == NULL )
    {
        ErrorMsg += "Audio package [" + xstring( pPackage ) + "] not loaded.\n";
        return 1;
    }        

    // Invalid descriptor?
    if( g_AudioMgr.IsValidDescriptor( m_Label ) == FALSE )
    {
        ErrorMsg += "Sound [" + xstring( m_Label ) + "] does not exist.\n";
        return 1;
    }

    // Is it steamed?
    if( m_Flags & STREAM_EMITTER )
    {
        if( !g_AudioMgr.IsCold( m_Label ) )
        {
            ErrorMsg += "Sound [" + xstring( m_Label ) + "] is NOT cold.\n";
            return 1;
        }
    }
    else
    {
        if( g_AudioMgr.IsCold( m_Label ) )
        {
            ErrorMsg += "Sound [" + xstring( m_Label ) + "] IS cold.\n";
            return 1;
        }
    }

    return 0;
}

#endif // X_EDITOR

//=========================================================================

void sound_emitter::OnActivate( xbool Flag )
{
    if( Flag )
    {
        if( m_Flags & TRIGGER_ACTIVATE )
        {
            m_Flags |= ACTIVATED;
        }
        TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
    }
    else
    {
        if( m_Flags & TRIGGER_ACTIVATE )
        {
            m_Flags &= ~ACTIVATED;
        }
    }
}

//=========================================================================

void sound_emitter::OnKill( void )
{
    StopSoundEmitter();

    object::OnKill();
}

//=========================================================================

void sound_emitter::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
    TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
}

//=========================================================================

void sound_emitter::OnMoveRel( const vector3& DeltaPos )  
{
    object::OnMoveRel( DeltaPos );
    TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
}

//=========================================================================

void sound_emitter::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
    TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
}

//=========================================================================

void sound_emitter::OnTriggerTransform( const matrix4& L2W )
{
    object::OnTriggerTransform( L2W );
    TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
}

//=========================================================================

#ifndef X_RETAIL
void sound_emitter::OnDebugRender( void )
{
    CONTEXT( "sound_emitter::OnDebugRender" );

    if( !(GetAttrBits() & ATTR_EDITOR_SELECTED) && !(GetAttrBits() & ATTR_EDITOR_PLACEMENT_OBJECT) )
    {
        return;
    }

    // Get the audio manager properties.
    g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );
  
    // Convert to meters, then multiply the emitter scale.
    m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
    m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;
    
    bbox WorldBox = GetBBox();    

    // We only want do draw the near and far clip if the emitter type is point.
    if( m_EmitterType == POINT )
    {
        if( m_EmitterShape == SHAPE_SPHERE )
        {
            draw_Sphere( GetPosition(), (m_AudioMgrNearClip), g_NearClipColor ); 
            draw_Sphere( GetPosition(), (m_AudioMgrFarClip), g_FarClipColor ); 
        }
        else
        {
            draw_SetL2W( GetL2W() );

            // Get our boxes which are axis aligned.
            bbox Near( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);
            bbox Far( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);

            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

            // Scale.
            Near.Max = Scale * Near.Max;
            Near.Min = Scale * Near.Min;

            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;

            draw_BBox( Near, g_NearClipColor );
            draw_BBox( Far, g_FarClipColor );
            
            draw_ClearL2W();
        }
    }
    else
    {
        if (m_EmitterShape == SHAPE_SPHERE )
            draw_Sphere( WorldBox.GetCenter(), WorldBox.GetRadius(), XCOLOR_WHITE );
        else
            draw_BBox(WorldBox, XCOLOR_WHITE );
    }

    if( m_Debug )
    {
        if( m_EmitterType == POINT )
        {
    
            //matrix4 t = W2V;
            //t.InvertSRT();
            //vector3 Pos = g_AudioManager.GetPlayerPos();//t.GetTranslation();
            //x_printfxy( 0,0, "X:%f , Y:%f , Z:%f ", Pos.X, Pos.Y, Pos.Z );
            //x_printfxy( 0,1, "Label:%s", m_Label );

            //draw_Marker( Pos, XCOLOR_BLUE );
            draw_Line( TestPoint, TestPoint2, XCOLOR_GREEN );
            draw_Line( TestPoint, TestPoint3, XCOLOR_GREEN );
            draw_Marker( TestPoint, XCOLOR_YELLOW );
            draw_Marker( TestPoint2, XCOLOR_WHITE );
        }
        else
        {
            draw_Marker( TestPoint3, XCOLOR_RED );
            //draw_BBox( FarDraw, XCOLOR_BLUE );
            draw_Marker( TestPoint, XCOLOR_GREEN );
        }
        
        //draw_Marker( m_FinalPosTest, XCOLOR_BLUE );
        //draw_Marker( m_PojectectedFinalPosTest, XCOLOR_BLUE );
        //draw_Line( g_AudioManager.GetPlayerPos(), m_FinalPosTest, XCOLOR_RED );
        //draw_Line( g_AudioManager.GetPlayerPos(), m_PojectectedFinalPosTest, XCOLOR_RED );        
    }

#ifdef sansari
    draw_BBox( GetBBox(), XCOLOR_RED );
#endif
}
#endif // X_RETAIL

//=========================================================================

void sound_emitter::OnTeleportActivate( const vector3& Position )
{
    // Get the audio manager properties.
    g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

    // Get the collision box.
    bbox ColBox = GetBBox();

    // If its a sphere the radius is going to be bigger.
    if( m_EmitterShape == SHAPE_SPHERE )
    {
        ColBox.Set( ColBox.GetCenter(), m_AudioMgrFarClip*m_FarClip );
    }

    ColBox.Inflate( 100.0f, 100.0f, 100.0f );

    if( ColBox.Intersect( Position ) )
    {
        if( m_bCollisionActivate )
        {
            TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
        }
    }
}

//=========================================================================

void sound_emitter::OnAdvanceLogic ( f32 DeltaTime )
{
    CONTEXT( "sound_emitter::OnAdvanceLogic" );

    LOG_STAT(k_stats_Sound);

    //eng_Begin( "Draw Debug Sound Emitter" );
        //if( eng_GetNActiveViews() > 0 )
            //OnDebugRender();
    //eng_End();
//    return;
    
    //
    // Check if the emitter is activated by triggers or not.
    //
    
    if( m_Flags & TRIGGER_ACTIVATE )
    {
        if( !(m_Flags & ACTIVATED) )
        {
            TurnAttrBitsOff( object::ATTR_NEEDS_LOGIC_TIME );
            StopSoundEmitter();
            return;
        }
    }

    //
    // Check the ending routine.
    // 
    if( m_Flags & DEACTIVATE )
    {
        if( (IsSoundActive() == FALSE) && (m_EnableEndingRoutineCheck) )
        {
            TurnAttrBitsOff( object::ATTR_NEEDS_LOGIC_TIME );
            StopSoundEmitter();
            return;
        }            
    }
    else if( m_Flags & DESTROY )
    {
        if( (IsSoundActive() == FALSE) && (m_EnableEndingRoutineCheck) )
        {
            StopSoundEmitter();
            g_ObjMgr.DestroyObject( GetGuid() );
            return;
        }                    
    }
    
    m_CurrentPitchTime += DeltaTime;
    m_CurrentVolumeTime += DeltaTime;

    // Get the audio manager properties.
    g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );
   
    // Get the collision box.
    bbox ColBox = GetBBox();
    
    // If its a sphere the radius is going to be bigger.
    if( m_EmitterShape == SHAPE_SPHERE )
    {
        ColBox.Set( ColBox.GetCenter(), m_AudioMgrFarClip*m_FarClip );
    }
    
    ColBox.Inflate( 100.0f, 100.0f, 100.0f );

    // If we leave a random type emitter we want to smoothly ramp the sound down before destroying the sound emitter.
    if( m_EmitterType != AMBIENT )
        m_AmbientTransition = 0.0f;
    
    
    // How long till we fade the volume.
    f32 VolumeFade = 1.0f;

    //
    // If the player is no longer in the sound emitter stop updating it.
    //
    xbool  bCollided = FALSE;
    ear_id EarID = g_AudioMgr.GetFirstEar();

    while( EarID )
    {
        matrix4 W2V;
        vector3 Position; 
        s32     Zone;
        f32     Volume;

        g_AudioMgr.GetEar( EarID, W2V, Position, Zone, Volume );

        if( Volume && ColBox.Intersect( Position ) )
        {
            bCollided = TRUE;
            EarID = 0;
        }
        else
        {
            EarID = g_AudioMgr.GetNextEar();
        }
    }

    // Clear the current ear.
    g_AudioMgr.ResetCurrentEar();
    
    // If we aren't colliding with any ear then we need to start shutting out selves down.
    if( !bCollided )
    {
        if(  m_AmbientTransition <= 0.0f )
        {
            if( !m_bForceActive )
            {
                TurnAttrBitsOff( object::ATTR_NEEDS_LOGIC_TIME );
                if( IsSoundActive() )
                {
                    StopSoundEmitter();
                }
            }
            return;
        }
        else
        {
            m_AmbientTransition -= DeltaTime;
            if( m_AmbientTransition < 0 )
                m_AmbientTransition = 0;

            VolumeFade = MAX( m_AmbientTransition / m_VolumeFadeTime, 0.0f);
            //m_AmbientTransition -= DeltaTime;
        }
    }
    else
    {
        if( m_bReverbEnable )
            g_AudioMgr.SetReverbWetDryMix( m_WetDryMix );

        if( m_AmbientTransition < m_VolumeFadeTime )
        {
            m_AmbientTransition += DeltaTime;
            if( m_AmbientTransition > m_VolumeFadeTime )
                m_AmbientTransition = m_VolumeFadeTime;

            VolumeFade = MIN( m_AmbientTransition / m_VolumeFadeTime, 1.0f);
        }
    }

    if( m_EmitterType == POINT )
    {
        f32     PositionalVolume;
        vector3 Pos = GetPosition();

        g_AudioMgr.Calculate3dVolume( m_NearClip, m_FarClip, 0, Pos, GetZone1(), PositionalVolume );

        if( PositionalVolume <= 0.01f )
            return;
        
        UpdateSound( Pos, 1.0f, PositionalVolume );
    }
    else if( m_EmitterType == RANDOM )
    {
        //
        // RANDOM TYPE EMITTER.
        //
            
        // Check if the sound is already playing.
        if( (IsSoundActive() == FALSE) && (m_IntervalTime <= 0.0f) )
        {
            // Make sure that we don't get to close to the player.
            f32 DistanceX = x_frand( m_FarClipBox.Min.GetX(), m_FarClipBox.Max.GetX() );
            f32 DistanceY = x_frand( m_FarClipBox.Min.GetY(), m_FarClipBox.Max.GetY() );
            f32 DistanceZ = x_frand( m_FarClipBox.Min.GetZ(), m_FarClipBox.Max.GetZ() );

            vector3 Pos = GetPosition();
            vector3 Offset( DistanceX, DistanceY, DistanceZ );

            Pos += Offset;

            UpdateSound( Pos, 1.0f, 1.0f );
        }
    }
    else
    {
        //
        // AMBIENT TYPE EMITTER
        //

        // Check if the sound is already playing.
        Update2DSound( VolumeFade );

        f32 Pitch = 1.0f - (m_PitchDepth * x_sin( (m_CurrentPitchTime/m_PitchSpeed) * (2*PI) ));

        // Get the new Depth and Volume setttings.
        if( m_CurrentPitchTime > m_PitchSpeed )
        {
            m_CurrentPitchTime = m_CurrentPitchTime - m_PitchSpeed;
            m_PitchDepth = m_PitchBaseDepth + x_frand( -m_PitchDepthVar, m_PitchDepthVar );
            m_PitchSpeed = m_PitchBaseSpeed + x_frand( -m_PitchSpeedVar, m_PitchSpeedVar );
        }

        // Keep the pitch in range 2^-6  -->  2^2
        if( Pitch < 0.015625f )
            Pitch = 0.015625f;
        else if( Pitch > 4.0f )
            Pitch = 4.0f;

        f32 Volume = 1.0f - (m_VolumeDepth * x_sin( (m_CurrentVolumeTime/m_VolumeSpeed) * (2*PI) ));
        
        // Get the new Depth and Volume setttings.
        if( m_CurrentVolumeTime > m_VolumeSpeed )
        {
            m_CurrentVolumeTime = m_CurrentVolumeTime - m_VolumeSpeed;
            m_VolumeDepth = m_VolumeBaseDepth + x_frand( -m_VolumeDepthVar, m_VolumeDepthVar );
            m_VolumeSpeed = m_VolumeBaseSpeed + x_frand( -m_VolumeSpeedVar, m_VolumeSpeedVar );
        }

        // Keep the volume in range 0.0f -> 1.0f
        if( Volume < 0.0f )
            Volume = 0.0f;
        else if( Volume > 1.0f )
            Volume = 1.0f;

        if( m_Flags & STREAM_EMITTER )
        {
            g_ConverseMgr.SetVolume( m_VoiceID, Volume );
        }
        else
        {
            g_AudioMgr.SetPitch( m_VoiceID, Pitch );
            g_AudioMgr.SetVolume( m_VoiceID, Volume*VolumeFade );             
        }
    }
    
    if( (m_MaxInterval > 0.0f) && (m_IntervalTime < 0.0f) )
        m_IntervalTime = x_frand( m_MinInterval, m_MaxInterval );

    if( IsSoundActive() ==  FALSE )
        m_IntervalTime -= DeltaTime;
}

//=========================================================================

xbool sound_emitter::IsSoundActive( void )
{
    if( m_Flags & STREAM_EMITTER )
    {
        if( g_ConverseMgr.IsActive( m_VoiceID ) )
            return TRUE;
    }
    else
    {
        if( g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
            return TRUE;
    }
    
    return FALSE;
}

//=========================================================================

void sound_emitter::StopSoundEmitter( void )
{
    if( m_Flags & STREAM_EMITTER )
    {
        g_ConverseMgr.Stop( m_VoiceID, m_ReleaseTime );
    }
    else
    {
        g_AudioMgr.Release( m_VoiceID, m_ReleaseTime );
    }

    m_VoiceID           = 0;
    m_CurrentPitchTime  = 0.0f;
    m_CurrentVolumeTime = 0.0f; 
    m_EnableEndingRoutineCheck = FALSE;
}

//=========================================================================

void sound_emitter::UpdateSound( vector3& VirtualPos, f32 EmitterClipScale, f32 FinalVolume )
{
    if( m_Flags & STREAM_EMITTER )
    {
        if( (IsSoundActive() == FALSE) && (m_IntervalTime <= 0.0f) )
        {
            m_EnableEndingRoutineCheck = TRUE;

            m_CurrentVolume = x_frand( m_MinVolume, m_MaxVolume );  

            //
            // If we are in the near clip then don't set the Emitter scales.
            // PASS 0 AS THE PORTAL ID SO THE SOUND WON'T BE PROPAGATED SINCE WE HAVE ALREADY DONE THAT!!!!
            //

            m_VoiceID = g_ConverseMgr.PlayStream( m_Label, VirtualPos, GetGuid(), GetZone1() );

            if( EmitterClipScale != 0.0f )
            {
                g_ConverseMgr.SetFalloff( m_VoiceID, 0.0f, EmitterClipScale );
            }

    	    g_ConverseMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );
        }
        else
        {
            g_ConverseMgr.SetPosition( m_VoiceID, VirtualPos, GetZone1() );
            g_ConverseMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );

            if( EmitterClipScale != 0.0f )
            {                
                g_ConverseMgr.SetFalloff( m_VoiceID, 0.0f, EmitterClipScale );   
            }                
        }            
    }
    else
    {            

	    // Check if the sound is already playing.
        if( (IsSoundActive() == FALSE) && (m_IntervalTime <= 0.0f) )
        {   
            m_EnableEndingRoutineCheck = TRUE;
            m_CurrentVolume = x_frand( m_MinVolume, m_MaxVolume );  

            //
            // If we are in the near clip then don't set the Emitter scales.
            // PASS 0 AS THE PORTAL ID SO THE SOUND WON'T BE PROPAGATED SINCE WE HAVE ALREADY DONE THAT!!!!
            //

            m_VoiceID = g_AudioMgr.PlayVolumeClipped( m_Label, VirtualPos, GetZone1(), FALSE );

            if( EmitterClipScale != 0.0f )
            {
                g_AudioMgr.SetFalloff( m_VoiceID, 0.0f, EmitterClipScale );
            }

    	    g_AudioMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );
            g_AudioMgr.Start( m_VoiceID );
        }
        else
        {
            g_AudioMgr.SetPosition( m_VoiceID, VirtualPos, GetZone1() );
            g_AudioMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );

            if( EmitterClipScale != 0.0f )
            {                
                g_AudioMgr.SetFalloff( m_VoiceID, 0.0f, EmitterClipScale );   
            }
        }
    }
}

//=========================================================================

void sound_emitter::Update2DSound( f32 FinalVolume )
{
    if( m_Flags & STREAM_EMITTER )
    {
        if( (IsSoundActive() == FALSE) && (m_IntervalTime <= 0.0f) )
        {
            m_EnableEndingRoutineCheck = TRUE;

            m_CurrentVolume = x_frand( m_MinVolume, m_MaxVolume );  

            //
            // If we are in the near clip then don't set the Emitter scales.
            // PASS 0 AS THE PORTAL ID SO THE SOUND WON'T BE PROPAGATED SINCE WE HAVE ALREADY DONE THAT!!!!
            //

            vector3 ZeroPos( 0.0f, 0.0f, 0.0f );
            m_VoiceID = g_ConverseMgr.PlayStream( m_Label, ZeroPos, GetGuid(), GetZone1(), IMMEDIATE_PLAY, TRUE, PLAY_2D );

    	    g_ConverseMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );
        }
        else
        {
            g_ConverseMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );

        }            
    }
    else
    {            

	    // Check if the sound is already playing.
        if( (IsSoundActive() == FALSE) && (m_IntervalTime <= 0.0f) )
        {   
            m_EnableEndingRoutineCheck = TRUE;
            m_CurrentVolume = x_frand( m_MinVolume, m_MaxVolume );  

            //
            // If we are in the near clip then don't set the Emitter scales.
            // PASS 0 AS THE PORTAL ID SO THE SOUND WON'T BE PROPAGATED SINCE WE HAVE ALREADY DONE THAT!!!!
            //

            m_VoiceID = g_AudioMgr.Play( m_Label, FALSE );

    	    g_AudioMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );
            g_AudioMgr.Start( m_VoiceID );
        }
        else
        {
            g_AudioMgr.SetVolume( m_VoiceID, FinalVolume * m_CurrentVolume );
        }
    }
}

//=========================================================================

void sound_emitter::OnColCheck( void )
{
    if( g_CollisionMgr.IsUsingHighPoly() )
    {
        // This is because of the editor. In reality this should not be here
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplySphere( GetPosition(), 100.0f );
        g_CollisionMgr.EndApply();
    }
    else
    {
        g_CollisionMgr.StartApply( GetGuid() );

        // If its a sphere the radius is going to cause the box to be bigger that the normal worldbox.
        if( m_EmitterShape == SHAPE_SPHERE )
        {
            g_CollisionMgr.ApplySphere( GetPosition(), GetBBox().GetRadius() );
        }
        else
        {
            g_CollisionMgr.ApplyAABBox( GetBBox() );
        }

        g_CollisionMgr.EndApply();    
    }
}

//=========================================================================

void sound_emitter::OnColNotify ( object& Object )
{
    (void)Object;
    if( m_bCollisionActivate )
    {
        m_Collided = TRUE;
        TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
    }
}

//=========================================================================

bbox sound_emitter::GetLocalBBox ( void ) const 
{ 
    if( m_EmitterType == POINT )
    {
        // Get the audio manager properties.
        f32 NearClip, FarClip;
        g_AudioMgr.GetClip( NearClip, FarClip );

        // Multiply the emitter scale.
        FarClip  = (FarClip)*m_FarClip;

        bbox Far( vector3(0.0f,0.0f,0.0f),FarClip+10);

        // Multiply the clip scales if its a box.
        if( m_EmitterShape == SHAPE_ORIENTED_BOX )
        {
            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

            // Scale.
            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;
        }

	    return Far;
    }
    else
    {
        bbox Trigger( vector3( 0.0f, 0.0f, 0.0f ), m_TriggerArea );

        // Multiply the clip scales if its a box.
        if( m_EmitterShape == SHAPE_ORIENTED_BOX )
        {
            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

            Trigger.Max = Scale * Trigger.Max;
            Trigger.Min = Scale * Trigger.Min;
        }
        
//        if( m_Debug )
//        {
//            FarDraw.Set( Trigger.Max, Trigger.Min );
//        }

        return Trigger;
    }
}

//=========================================================================

void sound_emitter::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "Sound Emitter", "Sound Emitter Properties", 0 );
    List.PropEnumString  ( "Sound Emitter\\Audio Package", "The audio package associated with this prop object.", PROP_TYPE_READ_ONLY );
    List.PropEnumString  ( "Sound Emitter\\Audio Package Resource", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumExternal( "Sound Emitter\\Audio Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumExternal( "Sound Emitter\\Label", "Sound\0soundemitter\0","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    List.PropEnumEnum    ( "Sound Emitter\\Type", "POINT\0RANDOM\0AMBIENT\0", "The type of sound emitter (Static, Random) ", PROP_TYPE_MUST_ENUM );
    List.PropEnumEnum    ( "Sound Emitter\\Shape","SPHERE\0ORIENTED BOX\0", "The shape of the sound emitter (Sphere, Box) ", PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "Sound Emitter\\MinInterval", "Minimum interval between the samples (Units == Seconds)", 0 );
    List.PropEnumFloat   ( "Sound Emitter\\MaxInterval", "Maximum interval between the samples (Units == Seconds), 0 == singal shot", 0 );
    List.PropEnumFloat   ( "Sound Emitter\\Release Time", "How long to release the sound over", 0 );

    List.PropEnumEnum    ( "Sound Emitter\\Ending Routine", "Continue\0Deactivate\0Destroy\0",
                      "Continue allows the emitter to run its normal. "
                      "Deactivate just stops running its logic, and Destroy means its gone for the "
                      "life of the game", 0 );                       

    if( m_EmitterShape == SHAPE_ORIENTED_BOX )
    {
        List.PropEnumFloat   ( "Sound Emitter\\ClipXScale", "The X clip scale, Range( 0.0 -- 1.0 )", 0 );
        List.PropEnumFloat   ( "Sound Emitter\\ClipYScale", "The Y clip scale, Range( 0.0 -- 1.0 )", 0 );
        List.PropEnumFloat   ( "Sound Emitter\\ClipZScale", "The Z clip scale, Range( 0.0 -- 1.0 )", 0 );
    }
    
    if( m_EmitterType == POINT )
    {
        List.PropEnumFloat   ( "Sound Emitter\\NearClip", "The near clip scale, Range( 0.0 -- 10.0 )", 0 );
        List.PropEnumFloat   ( "Sound Emitter\\FarClip", "The far clip scale, Range( 0.0 -- 10.0 )", 0 );
    }
    else
    {
        List.PropEnumFloat   ( "Sound Emitter\\TriggerArea", "The trigger area. (units meters )", 0 );        
        
        if( m_EmitterType == AMBIENT )
        {
            List.PropEnumFloat   ( "Sound Emitter\\VolumeFadeTime", "The volume FadeIn and FadeOut time for the sound emitter", 0 );

            List.PropEnumHeader  ( "Sound Emitter\\Pitch Modulation", "Pitch Modulation properties", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Pitch Modulation\\Depth", "Min = 1-Depth and Max = 1+Depth (0 -> 1)", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Pitch Modulation\\Speed", "The time frame to vary the sample over (seconds, min 0.1 )", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Pitch Modulation\\Depth Var", "Variation in depth", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Pitch Modulation\\Speed Var", "Variation in speed", 0 );

            List.PropEnumHeader  ( "Sound Emitter\\Volume Modulation", "Volume Modulation properties", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Volume Modulation\\Depth", "Min = 1-Depth and Max = 1+Depth, (0 -> 1)", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Volume Modulation\\Speed", "The time frame to vary the sample over (seconds, min 0.1 )", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Volume Modulation\\Depth Var", "Variation in depth", 0 );
            List.PropEnumFloat   ( "Sound Emitter\\Volume Modulation\\Speed Var", "Variation in speed", 0 );

            List.PropEnumBool    ( "Sound Emitter\\Reverb Enable", "Does this emitter affect reverb wet/dry mix?", PROP_TYPE_MUST_ENUM );

            if( m_bReverbEnable )
            {
                List.PropEnumFloat( "Sound Emitter\\Reverb Wet/Dry Mix", "Reverb wet/dry mix", 0 );
            }
        }
    }

    List.PropEnumBool   ( "Sound Emitter\\Stream Emitter", "Is this a streamed emitter", 0 );

    List.PropEnumBool   ( "Sound Emitter\\Trigger Activate", "Is this emitter going to get activated by a trigger?", PROP_TYPE_MUST_ENUM );
    
    if( m_Flags & TRIGGER_ACTIVATE )
        List.PropEnumBool("Sound Emitter\\Start Active", "Is this trigger activated sound emitter going to start active?", 0 );

    List.PropEnumBool   ( "Sound Emitter\\Debug", "Turn on debug data.", 0 );
    List.PropEnumBool   ( "Sound Emitter\\Force Active", "Keep the emitter active even if the player goes out of range", 0 );
    List.PropEnumBool   ( "Sound Emitter\\Collision Activate", "Can collions activate the trigger?", 0 );
}

//=============================================================================

xbool sound_emitter::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
        // Get the audio manager properties.
        g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

        // Convert to meters, then multiply the emitter scale.
        m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
        m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;

        bbox Near( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);
        bbox Far( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);

        matrix4 Scale;

        Scale.Zero();
        Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

        // Scale.
        Near.Max = Scale * Near.Max;
        Near.Min = Scale * Near.Min;

        Far.Max = Scale * Far.Max;
        Far.Min = Scale * Far.Min;

        // Rotation.
        Near.Max = GetL2W() * Near.Max;
        Near.Min = GetL2W() * Near.Min;

        Far.Max = GetL2W() * Far.Max;
        Far.Min = GetL2W() * Far.Min;

        m_FarClipBox = Far;
        m_NearClipBox = Near;

        return TRUE;
    }

    // External
    if( I.IsVar( "Sound Emitter\\Label" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_Label, 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                if( String == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                    m_Label[0] = 0;
                }
                else
                {
                    s32 PkgIndex = String.Find( '\\', 0 );
                    
                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );

                        m_hAudioPackage.SetName( Pkg );                

                        // Load the audio package.
                        if( m_hAudioPackage.IsLoaded() == FALSE )
                            m_hAudioPackage.GetPointer();
                    }

                    x_strncpy( m_Label, String, 64 );
                }
            }
        }
        return( TRUE );
    }
    
    // External
    if( I.IsVar( "Sound Emitter\\Audio Package External Resource" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );        
    }

    if( I.IsVar( "Sound Emitter\\Audio Package" ) )
    {
        // You can only read.
        if( I.IsRead() )
        {
            I.SetVarString( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        return TRUE;
    }

    if( I.IsVar( "Sound Emitter\\Audio Package Resource" ) )
    {
        // You can only read.
        if( I.IsRead() )
        {
            I.SetVarString( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            m_hAudioPackage.SetName( I.GetVarString() );

            // Load the audio package.
            if( m_hAudioPackage.IsLoaded() == FALSE )
                m_hAudioPackage.GetPointer();
        }
        return TRUE;
    }

    // The Type.
    if( I.IsVar( "Sound Emitter\\Type" ) )
    {
        if( I.IsRead () )
        {
            switch( m_EmitterType )
            {
                case POINT      : I.SetVarEnum( "POINT" ); break;
                case RANDOM     : I.SetVarEnum( "RANDOM" ); break;
                case AMBIENT    : I.SetVarEnum( "AMBIENT" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "POINT", I.GetVarEnum()) )
            {
                m_EmitterType = POINT;
            }
            else if( !x_stricmp( "RANDOM", I.GetVarEnum() ) )
            {
                m_EmitterType = RANDOM;
            }
            else if( !x_stricmp( "AMBIENT", I.GetVarEnum() ) )
            {
                m_EmitterType = AMBIENT;
            }

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    // The Shape.
    if( I.IsVar( "Sound Emitter\\Shape" ) )
    {
        if( I.IsRead () )
        {
            switch( m_EmitterShape )
            {
                case SHAPE_SPHERE       : I.SetVarEnum( "SPHERE" ); break;
                case SHAPE_ORIENTED_BOX : I.SetVarEnum( "ORIENTED BOX" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "SPHERE", I.GetVarEnum()) )
            {
                m_EmitterShape = SHAPE_SPHERE;
            }
            else if( !x_stricmp( "ORIENTED BOX", I.GetVarEnum() ) )
            {
                m_EmitterShape = SHAPE_ORIENTED_BOX;
            }

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    // The MinVolume.
    if( I.VarFloat ( "Sound Emitter\\MinVolume", m_MinVolume ) )
        return TRUE;

    // The MaxVolume.
    if( I.VarFloat ( "Sound Emitter\\MaxVolume", m_MaxVolume ) )
        return TRUE;

    // The MinInterval.
    if( I.VarFloat ( "Sound Emitter\\MinInterval", m_MinInterval ) )
        return TRUE;

    // The MaxInterval.
    if( I.VarFloat ( "Sound Emitter\\MaxInterval", m_MaxInterval ) )
        return TRUE;

    // The ClipHorizontal.
    if( I.VarFloat ( "Sound Emitter\\ClipXScale", m_XScale ) )
    {
        if( !I.IsRead() )
        {
            bbox Near;
            bbox Far;
            
            // Is it a random or a point emitter.
            if( m_EmitterType != POINT )
            {
                Far.Set( vector3(0.0f,0.0f,0.0f), m_TriggerArea);
            }
            else
            {
                // Get the audio manager properties.
                g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

                // Convert to meters, then multiply the emitter scale.
                m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
                m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;

                Near.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);
                Far.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);
            }
            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

            // Scale.
            Near.Max = Scale * Near.Max;
            Near.Min = Scale * Near.Min;

            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;

            // Rotation.
            Near.Max = GetL2W() * Near.Max;
            Near.Min = GetL2W() * Near.Min;

            Far.Max = GetL2W() * Far.Max;
            Far.Min = GetL2W() * Far.Min;

            m_FarClipBox = Far;
            m_NearClipBox = Near;

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    // The ClipVertical.
    if( I.VarFloat ( "Sound Emitter\\ClipYScale", m_YScale ) )
    {
        if( !I.IsRead() )
        {
            bbox Near;
            bbox Far;
            
            // Is it a random or a point emitter.
            if( m_EmitterType != POINT )
            {
                Far.Set( vector3(0.0f,0.0f,0.0f),m_TriggerArea);
            }
            else
            {
                // Get the audio manager properties.
                g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

                // Convert to meters, then multiply the emitter scale.
                m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
                m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;

                Near.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);
                Far.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);
            }

            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

            // Scale.
            Near.Max = Scale * Near.Max;
            Near.Min = Scale * Near.Min;

            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;

            // Rotation.
            Near.Max = GetL2W() * Near.Max;
            Near.Min = GetL2W() * Near.Min;

            Far.Max = GetL2W() * Far.Max;
            Far.Min = GetL2W() * Far.Min;

            m_FarClipBox = Far;
            m_NearClipBox = Near;

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    // The ClipVertical.
    if( I.VarFloat ( "Sound Emitter\\ClipZScale", m_ZScale ) )
    {
        if( !I.IsRead() )
        {
            bbox Near;
            bbox Far;
            
            // Is it a random or a point emitter.
            if( m_EmitterType != POINT )
            {
                Far.Set( vector3(0.0f,0.0f,0.0f), m_TriggerArea);
            }
            else
            {
                // Get the audio manager properties.
                g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

                // Convert to meters, then multiply the emitter scale.
                m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
                m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;

                Near.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);
                Far.Set( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);
            }

            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

            // Scale.
            Near.Max = Scale * Near.Max;
            Near.Min = Scale * Near.Min;

            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;

            // Rotation.
            Near.Max = GetL2W() * Near.Max;
            Near.Min = GetL2W() * Near.Min;

            Far.Max = GetL2W() * Far.Max;
            Far.Min = GetL2W() * Far.Min;

            m_FarClipBox = Far;
            m_NearClipBox = Near;

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    // The TriggerArea.
    if( I.VarFloat ( "Sound Emitter\\TriggerArea", m_TriggerArea ) )
    {
        if( !I.IsRead() )
        {
            bbox Far( vector3(0.0f,0.0f,0.0f),m_TriggerArea);

            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );
        
            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;

            Far.Max = GetL2W() * Far.Max;
            Far.Min = GetL2W() * Far.Min;
            
            m_FarClipBox = Far;

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }
    
    // The NearClip.
    if( I.VarFloat ( "Sound Emitter\\NearClip", m_NearClip ) )
    {
        if( !I.IsRead() )
        {
            // Get the audio manager properties.
            g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

            // Convert to meters, then multiply the emitter scale.
            m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
            m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;
            
            if( m_AudioMgrNearClip >= m_AudioMgrFarClip )
            {
                m_NearClip -= 0.01f;
                m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
            }

            bbox Near( vector3(0.0f,0.0f,0.0f),m_AudioMgrNearClip);

            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );
        
            Near.Max = Scale * Near.Max;
            Near.Min = Scale * Near.Min;

            Near.Max = GetL2W() * Near.Max;
            Near.Min = GetL2W() * Near.Min;
            
            m_NearClipBox = Near;

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }

    // The FarClip.
    if( I.VarFloat ( "Sound Emitter\\FarClip", m_FarClip ) )
    {
        if( !I.IsRead() )
        {
            // Get the audio manager properties.
            g_AudioMgr.GetClip( m_AudioMgrNearClip, m_AudioMgrFarClip );

            // Convert to meters, then multiply the emitter scale.
            m_AudioMgrNearClip = (m_AudioMgrNearClip)*m_NearClip;
            m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;

            if( m_AudioMgrNearClip >= m_AudioMgrFarClip )
            {
                m_FarClip += 0.01f;
                m_AudioMgrFarClip  = (m_AudioMgrFarClip)*m_FarClip;
            }

            bbox Far( vector3(0.0f,0.0f,0.0f),m_AudioMgrFarClip);

            matrix4 Scale;

            Scale.Zero();
            Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );
        
            Far.Max = Scale * Far.Max;
            Far.Min = Scale * Far.Min;

            Far.Max = GetL2W() * Far.Max;
            Far.Min = GetL2W() * Far.Min;
            
            m_FarClipBox = Far;

            // Force the world box to update.
            OnMove( GetPosition() );
        }

        return TRUE;
    }   

    if( I.VarFloat( "Sound Emitter\\Release Time", m_ReleaseTime ) )
    {
        m_ReleaseTime = MAX( 0.0f, m_ReleaseTime );
        return TRUE;
    }

    // What to do after finishing playing the sample.
    if( I.IsVar( "Sound Emitter\\Ending Routine" ) )
    {
        if( I.IsRead () )
        {
            if( m_Flags & DEACTIVATE )
                I.SetVarEnum( "Deactivate" );
            else if( m_Flags & DESTROY )
                I.SetVarEnum( "Destroy" );
            else if( m_Flags & CONTINUE )
                I.SetVarEnum( "Continue" );
            else
                ASSERTS( FALSE, "Didn't set the type"  );
        }
        else
        {
            if( !x_stricmp( "Deactivate", I.GetVarEnum()) )
            {
                m_Flags |= DEACTIVATE;
                m_Flags &= ~DESTROY;
                m_Flags &= ~CONTINUE;
            }
            else if( !x_stricmp( "Destroy", I.GetVarEnum() ) )
            {
                m_Flags |= DESTROY;
                m_Flags &= ~DEACTIVATE;
                m_Flags &= ~CONTINUE;
            }
            else if( !x_stricmp( "Continue", I.GetVarEnum() ) )
            {
                m_Flags |= CONTINUE;
                m_Flags &= ~DEACTIVATE;
                m_Flags &= ~DESTROY;
            }
            else
            {
                ASSERTS( FALSE, "Didn't set the type"  );
            }
        }

        return( TRUE );
    }


    if( I.VarFloat ( "Sound Emitter\\VolumeFadeTime", m_VolumeFadeTime ) )
    {
        if( m_VolumeFadeTime < 0.0f )
            m_VolumeFadeTime = 0.0f;

        return TRUE;
    }
    
    if( I.VarFloat ( "Sound Emitter\\Pitch Modulation\\Depth", m_PitchBaseDepth ) )
    {
        if( m_PitchBaseDepth < 0.0f )
            m_PitchBaseDepth = 0.0f;
        else if( m_PitchBaseDepth > 1.0f )
            m_PitchBaseDepth = 1.0f;
        
        m_PitchDepth = m_PitchBaseDepth;

        return TRUE;
    }

    if( I.VarFloat ( "Sound Emitter\\Pitch Modulation\\Speed", m_PitchBaseSpeed ) )
    {
        if( m_PitchBaseSpeed < 0.1f )
            m_PitchBaseSpeed = 0.1f;

        m_PitchSpeed = m_PitchBaseSpeed;                

        return TRUE;
    }

    if( I.VarFloat ( "Sound Emitter\\Pitch Modulation\\Depth Var", m_PitchDepthVar ) )
    {
        return TRUE;
    }

    if( I.VarFloat ( "Sound Emitter\\Pitch Modulation\\Speed Var", m_PitchSpeedVar ) )
    {
        return TRUE;
    }            

    if( I.VarFloat ( "Sound Emitter\\Volume Modulation\\Depth", m_VolumeBaseDepth ) )
    {
        if( m_VolumeBaseDepth < 0.0f )
            m_VolumeBaseDepth = 0.0f;
        else if( m_VolumeBaseDepth > 1.0f )
            m_VolumeBaseDepth = 1.0f;
        
        m_VolumeDepth = m_VolumeBaseDepth;

        return TRUE;
    }

    if( I.VarFloat ( "Sound Emitter\\Volume Modulation\\Speed", m_VolumeBaseSpeed ) )
    {
        if( m_VolumeBaseSpeed < 0.1f )
            m_VolumeBaseSpeed = 0.1f;

        m_VolumeSpeed = m_VolumeBaseSpeed;

        return TRUE;
    }

    if( I.VarFloat ( "Sound Emitter\\Volume Modulation\\Depth Var", m_VolumeDepthVar ) )
    {
        return TRUE;
    }

    if( I.VarFloat ( "Sound Emitter\\Volume Modulation\\Speed Var", m_VolumeSpeedVar ) )
    {
        return TRUE;
    }            

    if( I.IsVar( "Sound Emitter\\Stream Emitter" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( (m_Flags & STREAM_EMITTER) );
        }
        else
        {
            if( I.GetVarBool() )
                m_Flags |= STREAM_EMITTER;
            else
                m_Flags &= ~STREAM_EMITTER;
        }   

        return TRUE;
    }

    if( I.IsVar( "Sound Emitter\\Trigger Activate" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( (m_Flags & TRIGGER_ACTIVATE) );
        }
        else
        {
            if( I.GetVarBool() )
                m_Flags |= TRIGGER_ACTIVATE;
            else
                m_Flags &= ~TRIGGER_ACTIVATE;
        }   

        return TRUE;
    }


    if( I.IsVar( "Sound Emitter\\Start Active" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( (m_Flags & ACTIVATED) );
        }
        else
        {
            if( I.GetVarBool() )
                m_Flags |= ACTIVATED;
            else
                m_Flags &= ~ACTIVATED;
        }   

        return TRUE;
    }

    // The Debug.
    if( I.VarBool ( "Sound Emitter\\Debug", m_Debug ) )
    {
        return TRUE;
    }

    if( I.IsVar( "Sound Emitter\\Force Active" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( m_bForceActive );
        }
        else
        {
            m_bForceActive = I.GetVarBool();
        }

        return TRUE;
    }

    if( I.IsVar( "Sound Emitter\\Collision Activate" ) )
    {
        if(  I.IsRead() )
        {
            I.SetVarBool( m_bCollisionActivate );
        }
        else
        {
            m_bCollisionActivate = I.GetVarBool();
        }

        return TRUE;
    }

    if( I.IsVar( "Sound Emitter\\Reverb Enable" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarBool( m_bReverbEnable );
        }
        else
        {
            m_bReverbEnable = I.GetVarBool();
        }

        return TRUE;
    }

    if( I.IsVar( "Sound Emitter\\Reverb Wet/Dry Mix" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarFloat( m_WetDryMix);
        }
        else
        {
            m_WetDryMix = I.GetVarFloat();
        }

        return TRUE;
    }

    return FALSE;
}

//=============================================================================

f32 sound_emitter::ClosestPointToAABox (const vector3& Point, const bbox& Box, vector3& ClosestPoint)
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

f32 sound_emitter::DistanceBetweenNearandFarClipOBox (const vector3& Point, const matrix4& ObjectL2W, vector3& Extent, 
                                                        const bbox& FarClip, vector3& PointOnBox )
{
    // compute coordinates of point in box coordinate system
    vector3 Diff = Point - ObjectL2W.GetTranslation();
    vector3 BoxXAxis( 1.0f, 0.0f, 0.0f );
    vector3 BoxYAxis( 0.0f, 1.0f, 0.0f );
    vector3 BoxZAxis( 0.0f, 0.0f, 1.0f );
    
    ObjectL2W.GetColumns( BoxXAxis, BoxYAxis, BoxZAxis );
    
    f32 X =  Diff.Dot(BoxXAxis);
    f32 Y =  Diff.Dot(BoxYAxis);
    f32 Z =  Diff.Dot(BoxZAxis);

    vector3 Closest( X, Y, Z );
    vector3 Pos( Closest );

    // project test point onto box
    f32 SqrDistance = 0.0f;
    f32 Delta;

    if ( Closest.GetX() < -Extent.GetX() )
    {
        Delta          = Closest.GetX() + Extent.GetX();
        SqrDistance   += Delta*Delta;
        Closest.GetX() = -Extent.GetX();
    }
    else if ( Closest.GetX() > Extent.GetX() )
    {
        Delta          = Closest.GetX() - Extent.GetX();
        SqrDistance   += Delta*Delta;
        Closest.GetX() = Extent.GetX();
    }

    if ( Closest.GetY() < -Extent.GetY() )
    {
        Delta          = Closest.GetY() + Extent.GetY();
        SqrDistance   += Delta*Delta;
        Closest.GetY() = -Extent.GetY();
    }
    else if ( Closest.GetY() > Extent.GetY() )
    {
        Delta          = Closest.GetY() - Extent.GetY();
        SqrDistance   += Delta*Delta;
        Closest.GetY() = Extent.GetY();
    }

    if ( Closest.GetZ() < -Extent.GetZ() )
    {
        Delta          = Closest.GetZ() + Extent.GetZ();
        SqrDistance   += Delta*Delta;
        Closest.GetZ() = -Extent.GetZ();
    }
    else if ( Closest.GetZ() > Extent.GetZ() )
    {
        Delta          = Closest.GetZ() - Extent.GetZ();
        SqrDistance   += Delta*Delta;
        Closest.GetZ() = Extent.GetZ();
    }
    
    PointOnBox = Closest;
    PointOnBox = ObjectL2W * PointOnBox;

    if( Pos == Closest )
    {
        vector3 NearBoxCenter( 0.0f, 0.0f, 0.0f );
        if( NearBoxCenter == Pos )
            NearBoxCenter.GetZ() += 0.1f;

        Closest = NearBoxCenter;
    }
    
    // Get the vector form the player pos and the point on the box.
    vector3 Ray = Pos - Closest;

    if( Ray.LengthSquared() < 10.0f )
        Ray.Scale( 10000.0f );
    else
        Ray.Scale( 1000.0f );

    Ray += Closest;
    
    // Compute far plane intersection
    f32 FinalT;
    vector3 HitPoint;
    xbool Collision = FarClip.Intersect( FinalT, Ray, Closest );
    (void)Collision;

    HitPoint = Ray + FinalT*(Closest-Ray);

#ifdef sansari    
    // We better hit this if we are running logic.
    ASSERT( Collision );
#endif

    if( m_Debug )
    {
        TestPoint   = ObjectL2W * Closest;
        TestPoint2  = ObjectL2W * HitPoint;

        vector3 tmpRay = Point - (ObjectL2W*Closest);
        tmpRay.Scale( 1000.0f );
        tmpRay += Closest;
        TestPoint3  = tmpRay;
    }

    return (Closest - HitPoint).LengthSquared();
}

//=============================================================================

f32 sound_emitter::DistanceBetweenNearandFarClipAABox ( const bbox& NearClipBox, const bbox& FarClipBox, 
                                                        const vector3& ListnerPos, vector3& PointOnBox )
{
    vector3 Closest( 0.0f, 0.0f, 0.0f );

    ClosestPointToAABox( ListnerPos, NearClipBox, Closest );
    
    PointOnBox = Closest;
        
    if( ListnerPos == Closest )
    {
        vector3 NearBoxCenter = GetPosition();
        if( NearBoxCenter == ListnerPos )
            NearBoxCenter.GetZ() += 0.1f;

        Closest = NearBoxCenter;
    }

    // Get the vector form the player pos and the point on the box.
    vector3 Ray = ListnerPos - Closest;

    if( Ray.LengthSquared() < 10.0f )
        Ray.Scale( 10000.0f );
    else
        Ray.Scale( 1000.0f );

    Ray += Closest;


    f32 FinalT;
    vector3 HitPoint;
    xbool Collision = FarClipBox.Intersect( FinalT, Ray, Closest );
    (void)Collision;

    HitPoint = Ray + FinalT*(Closest-Ray);

    
    (void)Collision;

#ifdef sansari
    // We better hit this if we are running logic.
    ASSERT( Collision );
#endif

    if( m_Debug )
    {
        TestPoint   = Closest;
        TestPoint2  = HitPoint;
        
        TestPoint3  = Ray;
    }

    return (Closest - HitPoint).LengthSquared();
}

//=============================================================================

vector3 sound_emitter::GetPointOnEmitter( const vector3& ListnerPos, f32& DistanceToPoint )
{
	vector3 ClosestPoint( 0.0f, 0.0f, 0.0f );

    if( m_EmitterType == POINT )
	{

        if( m_EmitterShape == SHAPE_SPHERE )
        {            
                vector3 ClosestPoint( 0.0f, 0.0f, 0.0f );
                sphere NearSphere( GetPosition(), (m_AudioMgrNearClip*m_NearClip) );
            
                // This is much faster.
                ClosestPointToAABox( ListnerPos, NearSphere.GetBBox(), ClosestPoint );

                DistanceToPoint = (m_AudioMgrFarClip*m_FarClip) - (m_AudioMgrNearClip*m_NearClip);

                return ClosestPoint;
        }
        else
        {
            // We want to skip all the matrix multiplies for the AA BBox.
            if( GetL2W().GetRotation() == radian3( 0.0f, 0.0f, 0.0f ) )
            {
                // No rotation.
			    DistanceToPoint = DistanceBetweenNearandFarClipAABox( m_NearClipBox, m_FarClipBox, ListnerPos, 
                                    ClosestPoint );
                DistanceToPoint = x_sqrt( DistanceToPoint );

                return ClosestPoint;
            }
            else
            {                           
                matrix4 Scale;
                Scale.Zero();
                Scale.SetScale( vector3( m_XScale, m_YScale, m_ZScale ) );

                bbox Near( vector3(0.0f,0.0f,0.0f),(m_AudioMgrNearClip)*m_NearClip);
                bbox Far( vector3(0.0f,0.0f,0.0f),(m_AudioMgrFarClip)*m_FarClip);

                // Apply to scales to the box.
                Far.Max  = Scale * Far.Max;
                Far.Min  = Scale * Far.Min;

                Near.Max = Scale * Near.Max;
                Near.Min = Scale * Near.Min;
            
                // Since they are centered on the origin then we can just get the max values for the extent.
                vector3 Extent( Near.Max );

                // Get the distance from a Oriented box.
                DistanceToPoint = DistanceBetweenNearandFarClipOBox( ListnerPos, GetL2W(), Extent, Far, 
                                    ClosestPoint );
                DistanceToPoint = x_sqrt( DistanceToPoint );

                return ClosestPoint;     
            }
        }      
    }

    return ClosestPoint;
}

//=============================================================================

f32 sound_emitter::GetEmitterClipScale( f32 Distance )
{
    if( m_EmitterType == POINT )
	{
        f32 FarClip = g_AudioMgr.GetFarFalloff ( m_Label );

        return (Distance / (m_AudioMgrFarClip*FarClip)); 
    }
    else
    {
        ASSERTS( 0, "Only POINT emitter type should be calling this function\n" );
        return 0.0f;
    }
}

//=============================================================================
