//==============================================================================
//
//  Turret.cpp
//
//==============================================================================
//
//  
//

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Turret.hpp"
#include "Objects\AlienOrb.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Characters\Character.hpp"
#include "CollisionMgr\PolyCache.hpp"

#include "AudioMgr\AudioMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "render\LightMgr.hpp"
#include "Objects\ProjectileBullett.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Objects\ProjectileAlienTurret.hpp"
#include "Objects\ProjectileEnergy.hpp"

#include "ProjectileMesonSeeker.hpp"

//#define TURRET_AI_LOGGING 

#ifdef TURRET_AI_LOGGING
#define DEBUG_LOG_MSG       LOG_MESSAGE
#else
#define DEBUG_LOG_MSG       debug_log_msg_fn   
#endif

inline 
void debug_log_msg_fn(...) {}


//void draw_Cylinder  ( const vector3& Center, f32 Radius, f32 Height, s32 nSteps, xcolor Color, xbool bCapped, const vector3& Up);
//==============================================================================
//  DEFINES
//==============================================================================

enum 
{
    TRACK_PITCH         = 0,
    TRACK_YAW           = 1,
};


static f32 k_MissFutureTime             = 0.55f;
static f32 k_TimeBetweenLOSChecks       = 2.0f;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
void Turret_Link( void ){}

static struct turret_desc : public object_desc
{
    turret_desc( void ) : object_desc( 
            object::TYPE_TURRET, 
            "Turret", 
            "AI",
            object::ATTR_COLLIDABLE       | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS | 
            object::ATTR_BLOCKS_RAGDOLL | 
            object::ATTR_BLOCKS_CHARACTER_LOS | 
            object::ATTR_BLOCKS_PLAYER_LOS | 
            object::ATTR_BLOCKS_SMALL_DEBRIS | 
            object::ATTR_RENDERABLE       |
            object::ATTR_NEEDS_LOGIC_TIME |
            object::ATTR_DAMAGEABLE |
            object::ATTR_SPACIAL_ENTRY,


            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_TARGETS_OBJS          |
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new turret; }

    //---------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

    //-------------------------------------------------------------------------

} s_turret_Desc;

//=========================================================================

const object_desc& turret::GetTypeDesc( void ) const
{
    return s_turret_Desc;
}

//=========================================================================

const object_desc& turret::GetObjectType( void )
{
    return s_turret_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

turret::turret( void ) :
    m_LeftBoundary ( NULL_GUID ),
    m_RightBoundary( NULL_GUID ),
    m_UpperBoundary( NULL_GUID ),
    m_LowerBoundary( NULL_GUID )
{
   m_ProjectileTemplateID   = -1;
    
    m_State                 = STATE_IDLE;
    m_PreChangeState        = STATE_UNDEFINED;
    m_TargetGuid            = 0;
    m_ShotCount             = 0;
    m_ActionTimer           = 0;
    m_ScanTimer             = x_frand(0,0.25f);
    m_Type                  = turret::TYPE_BULLET_LIGHT;

    m_RadiusSense           = 2000;
    m_RadiusFire            = 1800;
    m_RadiusMultiplier      = 1.0f;
    m_TimeBetweenScans      = 1.0f;
    m_TimeBetweenShots      = 0.25f;
    m_nShotsBetweenReloads  = 100;
    m_nMinShots             = 5;
    m_TimeCool              = 5.0f;
    m_Velocity              = 0.0f;
    m_PitchSpeed            = R_30;
    m_YawSpeed              = R_30;

    m_RestPitch             = R_0;
    m_RestYaw               = R_0;

    m_Pitch                 = m_RestPitch;
    m_Yaw                   = m_RestYaw;

    m_Health                = 240;
    m_MaxHealth             = m_Health;

    m_HitPercentage         = 20;
    m_EntryBone             = -1 ;
    m_StandBone             = -1 ;

    // If min and max constraints are ==, the constraint is ignored
    m_PitchUpLimit          = R_90;
    m_PitchDownLimit        = R_90;
    m_YawRightLimit         = R_180;
    m_YawLeftLimit          = R_180;
    
    m_FriendFaction = FACTION_WORKERS     | 
                      FACTION_BLACK_OPS   | 
                      FACTION_MILITARY;
    m_Faction       = FACTION_BLACK_OPS;

    m_bIsFunctional             = FALSE;         
    m_bHasAlert                 = FALSE;             
    m_bHasActiveIdle            = FALSE;
    m_bHasEngagedIdle           = FALSE;    
    m_bHasActivate              = FALSE;          
    m_bHasDeactivate            = FALSE;        
    m_bHasReload                = FALSE;    
    m_bHasRampUp                = FALSE;
    m_bHasRampDown              = FALSE;
    m_bHasActiveDeath           = FALSE;
    m_bHasInactiveDeath         = FALSE;
    m_bHasDestroyed             = FALSE;
    m_bYawLimited               = FALSE;
    m_bPitchLimited             = FALSE;

    m_bTurretActive             = FALSE;
    m_bIgnoresDamageable        = FALSE;

    m_bRequiesOrbPower          = FALSE;
    m_bOrbInside                = FALSE;

    m_ReservedBy                = NULL_GUID;
    m_OrbTemplateID             = -1;
    m_LaunchDelta.Set(0,100,0);

    m_bIndestructable           = FALSE;
    m_bInactiveIndestructable   = FALSE;

    m_TrackController[ TRACK_PITCH ].SetMixMode( anim_track_controller::MIX_ADDITIVE );
    m_TrackController[  TRACK_YAW  ].SetMixMode( anim_track_controller::MIX_ADDITIVE );

    m_FiringConeFOV         = R_2;

    m_LocalAimDir.Set(0,0,-1);
    m_LocalTargetDir.Set(0,0,-1);

    m_ActivateOnDestruction     = 0;
    m_ActivateOnActivation      = 0;
    m_ActivateOnDeactivation    = 0;
    m_ActivateOnTargetLoss      = 0;
    m_ActivateOnTargetAcquisition   = 0;
    m_ActivateOnFire            = 0;
    m_ActivateOnReload          = 0;
    m_ActivateOnScan            = 0;

    m_LOSCheckTimer             = 0;
    m_ObjSlotForLastLOSCheck    = SLOT_NULL;
    m_bLastLOSPassed            = FALSE;
    m_bPlayIdleCompletely       = FALSE;

    m_IdleCycle                 = -1;

    m_TrackingGuid              = 0;
    m_ObjectiveGuid             = 0;

//    m_CivilianAnimPackageFileName[0] = 0;
    ///m_SoldierAnimPackageFileName[0] = 0;
    //m_GreyAnimPackageFileName[0] = 0;
    m_EntryMarker               = 0;
    m_Gunner                    = 0;

    m_LastAimedTrackingStatus   = TRACK_NO_TARGET;


    m_bDestroySelfWhenDead      = FALSE;

    m_AimingSoundID             = -1;
    m_StopAimingSoundID         = -1;
    m_AimingSoundVoice          = -1;
    m_bAimStartLoopPlaying      = FALSE;
    m_TimeSinceLastAimCorrection = 0;
    m_IdleTimeUntilStopAim      = 1.0f;

    m_LogicalName               = -1;

    m_IsHidden                  = FALSE;
}

//=============================================================================

turret::~turret( void )
{
}

//=============================================================================

void turret::OnImport( text_in& TextIn )
{
    (void)TextIn;
}

//=============================================================================

void turret::OnMove( const vector3& NewPos )
{
    bbox BBox = GetBBox();
    object::OnMove( NewPos );
    m_AnimPlayer.SetL2W( GetL2W() );
    BBox += GetBBox();
    g_PolyCache.InvalidateCells( BBox, GetGuid() );
}

//=============================================================================

void turret::OnTransform( const matrix4& L2W )
{
    bbox BBox = GetBBox();
    object::OnTransform( L2W );
    m_AnimPlayer.SetL2W( GetL2W() );
    BBox += GetBBox();
    g_PolyCache.InvalidateCells( BBox, GetGuid() );
}

//=============================================================================

void turret::StopAimingSound( void )
{
    //
    // We didn't need to adjust the turret for a certain amount of time.
    // If it is still playing the aim loop, shut it down
    //
    if (m_bAimStartLoopPlaying)
    {
        // The loop is currently playing, we need to stop it and fire off the shutdown sound
        if (m_AimingSoundVoice != -1)
            g_AudioMgr.Release( m_AimingSoundVoice, 0 );

        m_AimingSoundVoice = -1;
        m_bAimStartLoopPlaying = FALSE;

        if (m_StopAimingSoundID != -1)
        {
            const char* pSoundName = g_StringMgr.GetString( m_StopAimingSoundID );
            if (pSoundName)
            {
                const matrix4& L2W = GetL2W();
                m_AimingSoundVoice = g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
            }  
        }            
    }
}

//=============================================================================

void turret::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "turret::OnAdvanceLogic" );

    // Update animation?
    if( !m_hAnimGroup.IsLoaded() )
        return;

    //
    // Consider updating polycache
    //
    {
        if( (m_AnimPlayer.GetFrame() != m_AnimPlayer.GetPrevFrame()) ||
            (m_AnimPlayer.GetCycle() != m_AnimPlayer.GetPrevCycle()) )
        {
            g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
        }
    }

    if( m_EntryBone == -1 )
    {    
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();    
        if( pAnimGroup )
        {
            m_EntryBone = pAnimGroup->GetBoneIndex("B_Attach_NPC_Start",  TRUE) ;
        }
    }
    object *markerObject = g_ObjMgr.GetObjectByGuid(m_EntryMarker);
    if( markerObject && m_EntryBone >= 0 )
    {
        vector3 EntryPos = m_AnimPlayer.GetBoneL2W(m_EntryBone,FALSE)->GetTranslation();
        markerObject->OnMove(EntryPos);
    }

    // Do we have enough anims to be able to function properly?
    if (!m_bIsFunctional)
        return;

    // Update last known target pos
    if (IsTargetValid())
    {        
        vector3 NewPos = GetObjectAimAt( m_TargetGuid );
        m_TargetVel = NewPos - m_TargetPos;
        m_TargetPos = NewPos;

        if (x_abs(DeltaTime) > 0.00001f)
            m_TargetVel.Scale( 1/DeltaTime );
        else
            m_TargetVel.Set(0,0,0);
    }

    m_AnimPlayer.Advance( DeltaTime );
    g_EventMgr.HandleSuperEvents( m_AnimPlayer, this );

    if (    (m_State != m_PreChangeState) 
         && (m_PreChangeState == STATE_UNDEFINED)  )
    {
        state New = m_State;
        m_State = m_PreChangeState;
        OnEnterState( New );
    }

    m_TimeSinceLastAimCorrection += DeltaTime;
    if (m_TimeSinceLastAimCorrection > m_IdleTimeUntilStopAim)
    {
        StopAimingSound();
    }

    m_PreChangeState = m_State;

    m_ActionTimer   += DeltaTime;
    m_ScanTimer     += DeltaTime;
    m_LOSCheckTimer += DeltaTime;

    switch( m_State )
    {
        //-------------------------------------------------
        case STATE_UNDEFINED:
            {
                OnEnterState( STATE_IDLE );
            }
            break;
            
        //-------------------------------------------------
        case STATE_IDLE:
            {
                if( LookForTarget() )
                {
                    // Target acquired.
                    DEBUG_LOG_MSG("Turret","Target acquired");                    
                    OnEnterState( STATE_PREACTIVATING );
                }  
            }
            break;

        //-------------------------------------------------

        case STATE_PREACTIVATING:
            {                
                if (!m_bPlayIdleCompletely)
                {
                    OnEnterState( STATE_ACTIVATING );
                }
                else
                {
                    s32 CurrentIdleCycle = m_AnimPlayer.GetCycle();
                    if (m_IdleCycle != CurrentIdleCycle)
                    {
                        OnEnterState( STATE_ACTIVATING );
                    }
                    m_IdleCycle = CurrentIdleCycle;
                }
            }
            break;

        //-------------------------------------------------
        case STATE_ACTIVATING:
            {
                if( (m_AnimPlayer.IsAtEnd()) || !m_bHasActivate)
                {
                    DEBUG_LOG_MSG("Turret","Activation complete");

                    m_bTurretActive = TRUE;

                    if( LookForTarget() )
                    {
                        DEBUG_LOG_MSG("Turret","Target still visible");
                        OnEnterState( STATE_AIMING );                         
                    }
                    else
                    {
                        DEBUG_LOG_MSG("Turret","Target lost");
                        OnEnterState( STATE_COOLING );
                    }
                }
            }
            break;

        //-------------------------------------------------
        case STATE_AIMING:
            {
                // If we are aiming at the tracking guid, we want to
                // try and refresh.
                if (m_TargetGuid == m_TrackingGuid)
                    LookForTarget();

                tracking_status Status = TrackTarget( DeltaTime );

                m_LastAimedTrackingStatus = Status;                

                s32 CurrentIdleCycle = m_AnimPlayer.GetCycle();

                if (Status == TRACK_NO_TARGET)
                {
                    DEBUG_LOG_MSG("Turret","Target lost");
                    OnEnterState( STATE_COOLING );
                }
                else if (Status == TRACK_OUTSIDE_FOF)
                {
                    LookForTarget();
                }
                else if (Status == TRACK_AIMING)
                {

                }
                else if (Status == TRACK_LOCKED)
                {   
                    TryToFireAtTarget();                    
                }

                m_IdleCycle = CurrentIdleCycle;
            }
            break;

        //-------------------------------------------------
        case STATE_RAMP_UP:
            {
                tracking_status Status = TrackTarget( DeltaTime );

                m_LastAimedTrackingStatus = Status;

                if ( m_AnimPlayer.IsAtEnd() || !m_bHasRampUp)
                {
                    if (Status == TRACK_NO_TARGET)
                    {
                        DEBUG_LOG_MSG("Turret","Target lost");
                        OnEnterState( STATE_RAMP_DOWN );
                    }
                    else
                    {
                        OnEnterState( STATE_FIRING );
                    }
                }                
            }
            break;


        //-------------------------------------------------
        case STATE_FIRING:
            {
                tracking_status Status = TrackTarget( DeltaTime );

                m_LastAimedTrackingStatus = Status;

                if (Status == TRACK_NO_TARGET)
                    LookForTarget();

                if ( m_AnimPlayer.IsAtEnd() )
                {
                    DEBUG_LOG_MSG("Turret","Turret firing complete");
                    if (m_ShotCount % m_nMinShots == 0)
                    {
                        OnEnterState( STATE_RAMP_DOWN );                        
                    }
                    else
                    {
                        // We haven't fired the required minimum number of shots
                        // Go around again
                        if ( m_ActionTimer > m_TimeBetweenShots )
                            OnEnterState( STATE_FIRING );
                    }
                }
            }
            break;

        //-------------------------------------------------
        case STATE_RAMP_DOWN:
            {
                tracking_status Status = TrackTarget( DeltaTime );

                m_LastAimedTrackingStatus = Status;

                if ( m_AnimPlayer.IsAtEnd() || !m_bHasRampDown)
                {
                    if (    (m_ShotCount >= m_nShotsBetweenReloads)
                         && (m_nShotsBetweenReloads > 0) )
                    {
                        // Time to reload
                        OnEnterState( STATE_RELOADING );                                        
                    }
                    else
                    {
                        OnEnterState( STATE_AIMING );                                        
                    }
                }
            }
            break;

        //-------------------------------------------------
        case STATE_RELOADING:
            {               
                if ( m_AnimPlayer.IsAtEnd() || !m_bHasReload)
                {
                    OnEnterState( STATE_AIMING );                                        
                }
            }
            break;



        //-------------------------------------------------
        case STATE_COOLING:
            {
                if (m_ActionTimer >= m_TimeCool)
                {
                    DEBUG_LOG_MSG("Turret","Turret cooldown complete");
                    OnEnterState( STATE_UNAIMING );
                }
                else
                {
                    if ( LookForTarget() )
                    {
                        DEBUG_LOG_MSG("Turret","Turret cooldown aborted");
                        OnEnterState( STATE_AIMING );
                    }
                }

            }
            break;

        //-------------------------------------------------
        case STATE_UNAIMING:
            {
                if ( LookForTarget() )
                {
                    DEBUG_LOG_MSG("Turret","Unaiming aborted");
                    OnEnterState( STATE_AIMING );
                }
                else 
                {
                    UpdateAim( DeltaTime, m_RestPitch, m_RestYaw );

                    if (IsAimedAt( m_RestPitch, m_RestYaw ))                    
                    {
                        DEBUG_LOG_MSG("Turret","Unaiming complete");
                        OnEnterState( STATE_PREDEACTIVATING );
                    }
                }
            }
            break;

        //-------------------------------------------------
        
        case STATE_PREDEACTIVATING:
            {
                if (!m_bPlayIdleCompletely)
                {
                    OnEnterState( STATE_DEACTIVATING );
                }
                else
                {
                    s32 CurrentIdleCycle = m_AnimPlayer.GetCycle();
                    if (m_IdleCycle != CurrentIdleCycle)
                    {
                        OnEnterState( STATE_DEACTIVATING );
                    }
                    m_IdleCycle = CurrentIdleCycle;
                }
            }
            break;
        //-------------------------------------------------
        case STATE_DEACTIVATING:
            {
                if (m_AnimPlayer.IsAtEnd() || !m_bHasDeactivate)
                {
                    DEBUG_LOG_MSG("Turret","Deactivation complete");
                    m_bTurretActive = FALSE;
                    OnEnterState( STATE_IDLE );
                }
            }
            break;

        //-------------------------------------------------
        case STATE_DESTROY_BEGIN:
            {
                if (     m_AnimPlayer.IsAtEnd() 
                     || (m_bTurretActive  && !m_bHasActiveDeath)
                     || (!m_bTurretActive && !m_bHasInactiveDeath) )
                {
                    DEBUG_LOG_MSG("Turret","Done destroying");
                    m_bTurretActive = FALSE;
                    m_Faction = FACTION_NONE;
                    OnEnterState( STATE_DESTROYED );
                }
            }
            break;

        //-------------------------------------------------
        case STATE_DESTROYED:
            {
                // We're stuck here permanently
            }
            break;
    }    

    f32 Weight = 0.0f;

    switch( m_State )
    {
    case STATE_AIMING:
    case STATE_RAMP_UP:
    case STATE_FIRING:
    case STATE_RAMP_DOWN:
    case STATE_RELOADING:
    case STATE_COOLING:
    case STATE_UNAIMING:
        Weight = 1.0f;
        break;
    case STATE_DESTROY_BEGIN:
        {
            f32 T = m_AnimPlayer.GetFrameParametric();
            Weight = 1.0f-T;
        }
        break;            
    }

    m_TrackController[ TRACK_PITCH ].SetWeight( Weight );
    m_TrackController[  TRACK_YAW  ].SetWeight( Weight );

}

//=============================================================================

void turret::OnEnterState( state NewState )
{
    m_ActionTimer = 0.0f;
  
    m_State = NewState;

    DEBUG_LOG_MSG("Turret","Entering state [%s]",GetStateName( NewState ));

    switch(NewState)
    {
        case STATE_IDLE:
            {
                ASSERT(    (m_PreChangeState == STATE_UNDEFINED)
                        || (m_PreChangeState == STATE_IDLE)
                        || (m_PreChangeState == STATE_DEACTIVATING) );
                m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );        
                
                m_IdleCycle = -1;   
            }
            break;

        case STATE_PREACTIVATING:
            {
                ASSERT( m_PreChangeState == STATE_IDLE );
                m_IdleCycle = m_AnimPlayer.GetCycle();
            }
            break;

        case STATE_ACTIVATING:
            {
                ASSERT(    (m_PreChangeState == STATE_PREACTIVATING ) );
                if (m_bHasActivate)
                    m_AnimPlayer.SetAnim( "ACTIVATE", FALSE );
                else
                    m_AnimPlayer.SetAnim( "ACTIVE_IDLE", FALSE );

                ActivateObject( m_ActivateOnActivation );                
            }
            break;

        case STATE_AIMING:
            {
                ASSERT(    (m_PreChangeState == STATE_ACTIVATING)
                        || (m_PreChangeState == STATE_COOLING)
                        || (m_PreChangeState == STATE_RAMP_DOWN)
                        || (m_PreChangeState == STATE_RELOADING)
                        || (m_PreChangeState == STATE_UNAIMING) );                               

                if (m_bHasActiveIdle)
                    m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                else
                    m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );

                m_IdleCycle = -1;   // SetAnim sets the cycle to zero
                                    // so this allows us to begin firing immediately
                                    // if the turret is already locked on target
            }
            break;

        case STATE_RAMP_UP:
            {
                ASSERT(    (m_PreChangeState == STATE_AIMING) );
                if (m_bHasRampUp)
                    m_AnimPlayer.SetAnim( "RAMP_UP", FALSE );
                else
                {
                    if (m_bHasActiveIdle)
                        m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                    else
                        m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );
                }
            }
            break;

        case STATE_FIRING:
            {
                ASSERT(    (m_PreChangeState == STATE_RAMP_UP)
                        || (m_PreChangeState == STATE_FIRING) );   
                if (m_bHasFire)
                {
                    m_AnimPlayer.SetAnim( "SHOOT", FALSE );

                    ActivateObject( m_ActivateOnFire );
                }
                m_ShotCount++;
            }
            break;

        case STATE_RAMP_DOWN:
            {
                ASSERT(    (m_PreChangeState == STATE_FIRING) 
                        || (m_PreChangeState == STATE_RAMP_UP) );
                if (m_bHasRampDown)
                    m_AnimPlayer.SetAnim( "RAMP_DOWN", FALSE );
                else
                {
                    if (m_bHasActiveIdle)
                        m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                    else
                        m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );
                }                           
            }
            break;

        case STATE_RELOADING:
            {
                ASSERT( (m_PreChangeState == STATE_RAMP_DOWN) ); 
                
                m_ShotCount = 0;

                if (m_bHasReload)
                {
                    m_AnimPlayer.SetAnim( "RELOAD", FALSE );

                    ActivateObject( m_ActivateOnReload );
                }
                else
                {
                    if (m_bHasActiveIdle)
                        m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                    else
                        m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );
                }
            }
            break;


        case STATE_COOLING:
            {
                SetTargetGuid( 0 );
                ASSERT(    (m_PreChangeState == STATE_AIMING)
                        || (m_PreChangeState == STATE_ACTIVATING) );                    
                if (m_bHasActiveIdle)
                    m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                else
                    m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );                                
            }
            break;

        case STATE_UNAIMING:
            {
                ASSERT(    (m_PreChangeState == STATE_COOLING) );                    
                if (m_bHasActiveIdle)
                    m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                else
                    m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );
            }
            break;

        case STATE_PREDEACTIVATING:
            {
                ASSERT( m_PreChangeState == STATE_UNAIMING );
                m_IdleCycle = m_AnimPlayer.GetCycle();
            }
            break;

        case STATE_DEACTIVATING:
            {
                ASSERT(    (m_PreChangeState == STATE_PREDEACTIVATING) );                    
                if (m_bHasDeactivate)
                    m_AnimPlayer.SetAnim( "DEACTIVATE", FALSE );
                ActivateObject( m_ActivateOnDeactivation );                
            }
            break;

        case STATE_DESTROY_BEGIN:
            {
                if (m_bTurretActive && m_bHasActiveDeath)
                    m_AnimPlayer.SetAnim( "ACTIVE_DEATH", FALSE );
                else if (!m_bTurretActive && m_bHasInactiveDeath)
                    m_AnimPlayer.SetAnim( "INACTIVE_DEATH", FALSE );                    
                else
                {
                    if (m_bHasActiveIdle)
                        m_AnimPlayer.SetAnim( "ACTIVE_IDLE", FALSE );
                    else
                        m_AnimPlayer.SetAnim( "INACTIVE_IDLE", FALSE );
                }

                ActivateObject( m_ActivateOnDestruction );                
            }
            break;
        case STATE_DESTROYED:
            {
                if (m_bHasDestroyed)
                    m_AnimPlayer.SetAnim( "DESTROYED", TRUE );
                else
                {
                    if (m_bHasActiveIdle)
                        m_AnimPlayer.SetAnim( "ACTIVE_IDLE", TRUE );
                    else
                        m_AnimPlayer.SetAnim( "INACTIVE_IDLE", TRUE );
                }    

                if (m_bRequiesOrbPower && m_bOrbInside)
                {
                    LaunchOrb();
                }

                if (m_bDestroySelfWhenDead)
                {
                    g_ObjMgr.DestroyObject( GetGuid() );
                }
            }
    }

    f32 Weight = 0.0f;

    switch( m_State )
    {
        case STATE_AIMING:
        case STATE_RAMP_UP:
        case STATE_FIRING:
        case STATE_RAMP_DOWN:
        case STATE_RELOADING:
        case STATE_COOLING:
        case STATE_UNAIMING:
            Weight = 1.0f;
            break;
        case STATE_DESTROY_BEGIN:
            {
                f32 T = m_AnimPlayer.GetFrameParametric();
                Weight = 1.0f-T;
            }
            break;            
    }

    m_TrackController[ TRACK_PITCH ].SetWeight( Weight );
    m_TrackController[  TRACK_YAW  ].SetWeight( Weight );


}

//=============================================================================

guid turret::GetEntryMarker()
{
    if( m_EntryBone == -1 )
    {    
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();    
        if( pAnimGroup )
        {
            m_EntryBone = pAnimGroup->GetBoneIndex("B_Attach_NPC_Start",  TRUE) ;
        }
    }

    object *markerObject = g_ObjMgr.GetObjectByGuid(m_EntryMarker);
    if( markerObject )
    {
        vector3 EntryPos = m_AnimPlayer.GetBoneL2W(m_EntryBone,FALSE)->GetTranslation();
        markerObject->OnMove(EntryPos);
    }    
    
    return m_EntryMarker;
}

//=============================================================================

const char* turret::GetAnimGroupName( object::type characterType )
{
    switch( characterType )
    {
    case object::TYPE_GRAY:
        return m_hGreyAnimGroup.GetName();//m_GreyAnimPackageFileName;
        break;
    case object::TYPE_FRIENDLY_SOLDIER:
    case object::TYPE_BLACK_OPPS:
    case object::TYPE_HAZMAT:
        return m_hSoldierAnimGroup.GetName();//m_SoldierAnimPackageFileName;
        break;
    case object::TYPE_FRIENDLY_SCIENTIST:
        return m_hCivilianAnimGroup.GetName();//_CivilianAnimPackageFileName;
        break;
    }
    return NULL;
}

//=============================================================================

vector3 turret::GetStandBonePosition()
{
    if( m_StandBone == -1 )
    {    
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();    
        if( pAnimGroup )
        {
            m_StandBone = pAnimGroup->GetBoneIndex("B_Attach_NPC",  TRUE) ;
        }
    }
    return m_AnimPlayer.GetBoneL2W(m_StandBone,FALSE)->GetTranslation();
}

//=============================================================================

void turret::ActivateObject( guid Guid )
{
    if (Guid != 0)
    {
        //activate this guid
        object* pObj = g_ObjMgr.GetObjectByGuid(Guid);
        if (pObj)
        {
            pObj->OnActivate(TRUE);
        }
    }   
}

//=============================================================================

#ifndef X_RETAIL
void turret::OnDebugRender( void )
{
#ifdef X_EDITOR
    matrix4     L2W = GetL2W();             // not const&, we need to modify it later
    f32 LocalYaw   = L2W.GetRotation().Yaw;
    f32 LocalPitch = -(L2W.GetRotation().Pitch);

    draw_ClearL2W();

    vector3 SensorPos;
    radian3 SensorRot;
    vector3 MyPos     = GetPosition();

    GetSensorInfo( SensorPos, SensorRot );

    // Sensor
    draw_Sphere( SensorPos, 20, XCOLOR_YELLOW );
    
    // Horizontal yaw limits
    vector3 Dir(0,0,-1);
    f32 Yaw = -m_YawLeftLimit + m_YawRightLimit;
    
    Yaw += LocalYaw;
    Dir.RotateY( Yaw );  
    draw_Arc( MyPos, (GetEffectiveRadiusSense()), Dir.GetYaw(), m_YawLeftLimit + m_YawRightLimit, XCOLOR_YELLOW );    
    
    /*
    // Vertical pitch limits
    Dir.Set(0,0,-1);
    f32 Pitch = -m_PitchUpLimit + m_PitchDownLimit;
    f32 LocalPitch = L2W.GetRotation().Pitch;
    Pitch += LocalPitch;
    Dir.RotateZ( Pitch );  
    matrix4 TempL2W;
    TempL2W.Identity();
    TempL2W.Rotate( radian3(0,0,R_90) );
    draw_SetL2W( TempL2W );  
    draw_Arc( MyPos, (m_RadiusSense * m_RadiusMultiplier), Dir.GetPitch(), m_PitchUpLimit + m_PitchDownLimit, XCOLOR_YELLOW );
    draw_ClearL2W();
    */

    if (m_TargetGuid != 0)
    {
        // Targeting
        vector3 TargetPos = GetTargetPos();
        draw_Sphere( TargetPos, 15, XCOLOR_RED );
        draw_Line( SensorPos, TargetPos, XCOLOR_RED );

        // Aiming
        vector3 Aim(0,0,300);
        Aim.Rotate( radian3( m_Pitch, m_Yaw + R_180, 0 ));
        matrix4 L2WNoRot = L2W;
        L2WNoRot.ClearTranslation();
        L2WNoRot.Transform( &Aim, &Aim, 1 );

        //Aim.Rotate( radian3( m_Pitch + LocalPitch, m_Yaw + R_180 + LocalYaw, 0 ) );
        draw_Line( SensorPos, SensorPos + Aim, XCOLOR_GREEN );

        

        #ifdef X_DEBUG
            xcolor C;
            C.Random();
            C.A = 255;
            vector3 Vel = m_LastFireTargetVel;
            Vel.NormalizeAndScale( 200 );
            draw_Line( m_LastFireBarrelPos, m_LastFireTargetPos, C );
            draw_Line( m_LastFireTargetPos, m_LastFireTargetPos+Vel, C );
        #endif

    }

    // Ranges
    draw_Cylinder( MyPos, (m_RadiusSense), 800.0f, 32, xcolor(64,64,190,128) );
    draw_Cylinder( MyPos, (m_RadiusFire), 800.0f, 32, xcolor(190,64,64,128) );

    
    // Activatable guids    
    object* pObj = NULL;

    if ((m_ActivateOnDestruction!=0) && (pObj = g_ObjMgr.GetObjectByGuid( m_ActivateOnDestruction )))        
    {
        draw_Line( SensorPos, pObj->GetPosition(), XCOLOR_AQUA );
        draw_BBox( pObj->GetBBox(), XCOLOR_AQUA );
        draw_Label( pObj->GetPosition(), XCOLOR_PURPLE, "Activate On Destruction" );
    }    

    if ((m_ActivateOnActivation!=0) && (pObj = g_ObjMgr.GetObjectByGuid( m_ActivateOnActivation )))        
    {
        draw_Line( SensorPos, pObj->GetPosition(), XCOLOR_AQUA );
        draw_BBox( pObj->GetBBox(), XCOLOR_AQUA );
        draw_Label( pObj->GetPosition(), XCOLOR_PURPLE, "Activate On Activation" );
    }    

    if ((m_ActivateOnDeactivation!=0) && (pObj = g_ObjMgr.GetObjectByGuid( m_ActivateOnDeactivation )))        
    {
        draw_Line( SensorPos, pObj->GetPosition(), XCOLOR_AQUA );
        draw_BBox( pObj->GetBBox(), XCOLOR_AQUA );
        draw_Label( pObj->GetPosition(), XCOLOR_PURPLE, "Activate On Deactivation" );
    }    

    if ((m_ActivateOnTargetLoss!=0) && (pObj = g_ObjMgr.GetObjectByGuid( m_ActivateOnTargetLoss )))        
    {
        draw_Line( SensorPos, pObj->GetPosition(), XCOLOR_AQUA );
        draw_BBox( pObj->GetBBox(), XCOLOR_AQUA );
        draw_Label( pObj->GetPosition(), XCOLOR_PURPLE, "Activate On Target Loss" );
    }    

    if ((m_ActivateOnTargetAcquisition!=0) && (pObj = g_ObjMgr.GetObjectByGuid( m_ActivateOnTargetAcquisition )))        
    {
        draw_Line( SensorPos, pObj->GetPosition(), XCOLOR_AQUA );
        draw_BBox( pObj->GetBBox(), XCOLOR_AQUA );
        draw_Label( pObj->GetPosition(), XCOLOR_PURPLE, "Activate On Target Acquisition" );
    }    

    draw_Label( GetPosition() + vector3(0,100,0), XCOLOR_WHITE, "Pitch: %5.3f(%5.3f deg)",m_Pitch,RAD_TO_DEG(m_Pitch));
    draw_Label( GetPosition() + vector3(0,150,0), XCOLOR_WHITE, "Yaw: %5.3f(%5.3f deg)",m_Yaw,RAD_TO_DEG(m_Yaw));
    draw_Label( GetPosition() + vector3(0,200,0), XCOLOR_WHITE, "State: %s, Cycle %d, IdleCycle %d",GetStateName( m_State ),m_AnimPlayer.GetCycle(),m_IdleCycle);

    if (m_bRequiesOrbPower)
    {        
        vector3 LaunchDelta = m_LaunchDelta;
        L2W.ClearTranslation();
        L2W.Transform( &LaunchDelta, &LaunchDelta, 1 );

        draw_Line( MyPos, MyPos + LaunchDelta, XCOLOR_YELLOW );
        draw_Sphere( MyPos + LaunchDelta, 20, XCOLOR_YELLOW );
        draw_Label( MyPos + LaunchDelta, XCOLOR_BLUE, "LAUNCH HERE" );
    }
#endif // X_EDITOR
}
#endif // X_RETAIL

//=============================================================================

void turret::OnRender( void )
{
    CONTEXT( "turret::OnRender" );

    // Early exit if this turret is hidden
    if( m_IsHidden )
        return;

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    
    if( pRigidGeom && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;

        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        m_Inst.Render( pBone, Flags | GetRenderMode() );
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }

#ifdef X_EDITOR
    //
    // manned turret stuff
    //
    object* pBoundaryObject;
    vector3 BoundaryPosition;

    if ( m_LeftBoundary && (pBoundaryObject = g_ObjMgr.GetObjectByGuid( m_LeftBoundary )) )
    {
        BoundaryPosition = pBoundaryObject->GetPosition();
        draw_Label( BoundaryPosition, XCOLOR_PURPLE, "Left" );
        draw_Line( GetPosition(), BoundaryPosition, XCOLOR_PURPLE );
    }

    if ( m_RightBoundary && (pBoundaryObject = g_ObjMgr.GetObjectByGuid( m_RightBoundary )) )
    {
        BoundaryPosition = pBoundaryObject->GetPosition();
        draw_Label( BoundaryPosition, XCOLOR_PURPLE, "Right" );
        draw_Line( GetPosition(), BoundaryPosition, XCOLOR_PURPLE );
    }

    if ( m_UpperBoundary && (pBoundaryObject = g_ObjMgr.GetObjectByGuid( m_UpperBoundary )) )
    {
        BoundaryPosition = pBoundaryObject->GetPosition();
        draw_Label( BoundaryPosition, XCOLOR_PURPLE, "Upper" );
        draw_Line( GetPosition(), BoundaryPosition, XCOLOR_PURPLE );
    }

    if ( m_LowerBoundary && (pBoundaryObject = g_ObjMgr.GetObjectByGuid( m_LowerBoundary )) )
    {
        BoundaryPosition = pBoundaryObject->GetPosition();
        draw_Label( BoundaryPosition, XCOLOR_PURPLE, "Lower" );
        draw_Line( GetPosition(), BoundaryPosition, XCOLOR_PURPLE );
    }

#endif // X_EDITOR
}

//=============================================================================

void turret::OnRenderTransparent( void )
{
    CONTEXT( "turret::OnRenderTransparent" );
    // Early exit if this turret is hidden
    if( m_IsHidden )
        return;

    play_surface::OnRenderTransparent();
}

//=============================================================================

void turret::OnColCheck ( void )
{
    CONTEXT("play_surface::OnColCheck");
    
    // Early exit if this turret is hidden
    if( m_IsHidden )
        return;

    // Compute the bone matrices, then let the play_surface code handle it.

    if( m_Inst.GetRigidGeom() && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;

        play_surface::DoColCheck( pBone );

    }
    else
    {
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplyAABBox( GetBBox() );    
        g_CollisionMgr.EndApply();    
    }
}

//=============================================================================

#ifndef X_RETAIL
void turret::OnColRender( xbool bRenderHigh )
{
    // Early exit if this turret is hidden
    if( m_IsHidden )
        return;

    // Compute the bone matrices, then let the play_surface code handle it.

    if( m_Inst.GetRigidGeom() && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;

        play_surface::DoColRender( pBone, bRenderHigh );
    }
}
#endif // X_RETAIL

//=============================================================================

const matrix4* turret::GetBoneL2Ws( void )
{
    return m_AnimPlayer.GetBoneL2Ws( TRUE );
}

//=============================================================================

void turret::OnEnumProp( prop_enum& List )
{
    play_surface::OnEnumProp( List );
    
    List.PropEnumExternal( "RenderInst\\Anim",           "Resource\0anim\0",       "Resource File", PROP_TYPE_MUST_ENUM );
    
    List.PropEnumHeader  ( "Turret", "This is the properties that are unique for the anim surface", 0 );

    List.PropEnumExternal( "Turret\\Audio Package",  "Resource\0audiopkg\0",   "The audio package associated with this anim surface.", 0 );    
    List.PropEnumFileName( "Turret\\Projectile Blueprint Path",
                      "Projectile object blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
                      "Resource for this item",
                      PROP_TYPE_MUST_ENUM );

    List.PropEnumString  ( "Turret\\Logical Name", "This name relates to entries in the pain and tweak tables.  Don't change it unless there is corresponding data in those tables", 0 );
    List.PropEnumBool    ( "Turret\\IsActive", "Tells whether an object is active or not. If you turn this off after loading the object will be not active. You can activate or deactivate an object any time.", PROP_TYPE_EXPOSE );    
    List.PropEnumFloat   ( "Turret\\Alert Distance", "How far can the turret sense targets", PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "Turret\\Firing Distance", "How far can the turret shoot", PROP_TYPE_EXPOSE );
    List.PropEnumBool    ( "Turret\\Ignore Damageable", "If set the turret will ignore all damageable objects when doing line of sight checks.", PROP_TYPE_EXPOSE );
   
    List.PropEnumFloat   ( "Turret\\Scan Time", "How many seconds between attempts to acquire a target", 0 );

    List.PropEnumFloat   ( "Turret\\Time Between Shots", "How many seconds between shots in a volley", 0 );
    
    List.PropEnumInt     ( "Turret\\Shots Between Reloads", "How many shots can be fired before reloading", 0 );
    List.PropEnumInt     ( "Turret\\Min Shots", "Min number of shots that must be fired once commiting to fire", 0 );

    List.PropEnumFloat   ( "Turret\\Hit Percentage", "How often the turret will really try to hit the target.  The rest will be misses.", PROP_TYPE_EXPOSE );

    List.PropEnumFloat   ( "Turret\\Cooling Time", "Number of seconds to way after losing a target before deactivating", 0 );

    List.PropEnumFloat   ( "Turret\\Health", "Amount of heal turret begins with.", PROP_TYPE_EXPOSE );   
    List.PropEnumBool    ( "Turret\\Indestructible", "Turret does not take damage.", PROP_TYPE_EXPOSE );
    List.PropEnumBool    ( "Turret\\Inactive Indestructible", "Turret does not take damage when it is inactive.  It DOES take damage when it is active and tracking.", PROP_TYPE_EXPOSE );

    List.PropEnumBool    ( "Turret\\Delete On Death", "Turret OBJECT itself is destroyed when the turret has been killed", 0 );

    List.PropEnumAngle   ( "Turret\\Pitch Speed", "Number of degrees the turret can pitch per second", 0 );
    List.PropEnumAngle   ( "Turret\\Yaw Speed", "Number of degrees the turret can yaw per second", 0 );

    List.PropEnumAngle   ( "Turret\\Pitch Up Limit", "Max angle the turret can rotate up to.  If 'Pitch Up Limit' and 'Pitch Down Limit' sum to 180 degrees, pitch limiting is turned off", 0 );
    List.PropEnumAngle   ( "Turret\\Pitch Down Limit", "Max angle the turret can rotate down to.  If 'Pitch Up Limit' and 'Pitch Down Limit' sum to 180 degrees, pitch limiting is turned off", 0 );

    List.PropEnumAngle   ( "Turret\\Yaw Right Limit", "Max angle the turret can rotate to the right.  If 'Yaw Right Limit' and 'Yaw Left Limit' sum to 360 degrees, yaw limiting is turned off", 0 );
    List.PropEnumAngle   ( "Turret\\Yaw Left Limit", "Max angle the turret can rotate to the left.  If 'Yaw Right Limit' and 'Yaw Left Limit' sum to 360 degrees, yaw limiting is turned off", 0 );    

    // Animation file
    List.PropEnumExternal( "Turret\\Civilian Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Turret\\Soldier Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Turret\\Grey Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM );
    List.PropEnumGuid    ( "Turret\\Entry Marker", "Marker that tells the NPCs where to go to mount the turret", 0 );
    List.PropEnumGuid    ( "Turret\\Controller", "Controller that we bind the gunner to.", 0 );

    // Orb
    List.PropEnumHeader  ( "Turret\\Orb Power", "Alien tech orb power", 0 );
    List.PropEnumBool    ( "Turret\\Orb Power\\Requires Orb Power", "Does the turret require an orb to be inside?", 0 );    
    List.PropEnumVector3 ( "Turret\\Orb Power\\Launch Vector", "When an orb leaves the turret, it will fly straight to the endpoint of this relative vector", 0 );

    List.PropEnumHeader  ( "Turret\\Scripting", "Turret Scripting Settings", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Target GUID", "What the turret should track and fire at", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY );

    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Destroy",        "Activate this guid on destruction of this turret.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Activation",     "Activate this guid when the turret powers up from it's rest state.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Deactivation",   "Activate this guid when the turret is powering down.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Target Acquired","Activate this guid when a target has been found.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Target Lost",    "Activate this guid when the current target has been lost.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Fire",           "Activate this guid when the turret fires.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Reload",         "Activate this guid when the turret reloads.", 0 );
    List.PropEnumGuid    ( "Turret\\Scripting\\Activate On Scan",           "Activate this guid when the turret scans for targets.", 0 );

    List.PropEnumGuid    ( "Turret\\Scripting\\Tracking Object",             "When no target is present, the turret will track this object.", PROP_TYPE_EXPOSE );
    List.PropEnumGuid    ( "Turret\\Scripting\\Objective Object",            "When set, the turret will track and fire at this object until it is destroyed.  The turret will not disengage from the objective even if it cannot fire on it.", PROP_TYPE_EXPOSE );
    
    List.PropEnumHeader  ( "Turret\\Animation", "Turret Animation Settings", 0 );
    List.PropEnumBool    ( "Turret\\Animation\\Play Complete Idle", "Idle animations must be played completely before switching to another anim.  This affects when the turret can come out of aiming in order to fire", 0 );

    List.PropEnumHeader  ( "Turret\\Audio", "Turret Audio Settings", 0 );
    List.PropEnumExternal( "Turret\\Audio\\Aiming",          "Sound\0soundexternal\0","The sound to play when the turret begins to aim", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Turret\\Audio\\Stop Aiming",     "Sound\0soundexternal\0","The sound to play when the turret stops aiming", PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "Turret\\Audio\\Time until aim stopped", "When the turret is not updating it's aim direction, it will wait this long before playing the Stop Aiming sound (if it had been aiming previously)", 0 );

    List.PropEnumHeader  ( "Turret\\Manned", "Settings for when this turret is manned by the player", 0 );
    List.PropEnumGuid    ( "Turret\\Manned\\Left Boundary", "Left aiming boundary", 0 );
    List.PropEnumGuid    ( "Turret\\Manned\\Right Boundary", "Right aiming boundary", 0 );
    List.PropEnumGuid    ( "Turret\\Manned\\Upper Boundary", "Upper aiming boundary", 0 );
    List.PropEnumGuid    ( "Turret\\Manned\\Lower Boundary", "Lower aiming boundary", 0 );

    List.PropEnumHeader("Factions", "General Faction Info", 0 );
    s32 ID = List.PushPath( "Factions\\" );
    factions_manager::OnEnumProp( List );
    List.PopPath( ID );

}

//=============================================================================

xbool turret::OnProperty( prop_query& I )
{
    if( play_surface::OnProperty( I ) )
        return( TRUE );
    
    s32 ID = I.PushPath( "Factions\\" );
    if ( factions_manager::OnProperty( I, m_Faction, m_FriendFaction ) )
    {
        return TRUE;
    }
    I.PopPath( ID );


    if( I.IsVar( "RenderInst\\Anim" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();
            if( pString[0] )
            {
                m_hAnimGroup.SetName( pString );
                anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
                
                if( pAnimGroup )
                {
                    // setup useful bones.
                    s32 iPitchAnim      = pAnimGroup->GetAnimIndex( "PITCH" );                    
                    s32 iPitchRefFrame  = 0;
                    s32 iYawRefFrame    = 0;

                    if (iPitchAnim != -1)
                    {
                        const anim_info& Info = pAnimGroup->GetAnimInfo( iPitchAnim );
                        s32 nFrames = Info.GetNFrames();
                        iPitchRefFrame = (nFrames-1) / 2;
                    }
                    
                    // FOR NOW, the yaw ref frame is always 0
                    // because the anim starts straight ahead and moves 360cw
                    //
                    // If we ever start somewhere else, then
                    // we will have to make sure the reference frame is
                    // the frame where the turret is facing along -z
                    iYawRefFrame = 0;

                    m_AnimPlayer.SetAnimGroup( m_hAnimGroup );
                    m_AnimPlayer.SetAnim( 0, TRUE );

                    m_AnimPlayer.SetTrackController( 0, &m_TrackController[ TRACK_PITCH ] );
                    m_AnimPlayer.SetTrackController( 1, &m_TrackController[  TRACK_YAW  ] );

                    m_TrackController[ TRACK_PITCH ].SetAnimGroup( m_hAnimGroup );
                    m_TrackController[ TRACK_PITCH ].SetAnim( "PITCH", TRUE );
                    m_TrackController[ TRACK_PITCH ].SetFrameParametric( 0 );
                    m_TrackController[ TRACK_PITCH ].SetWeight( 1 );
                    m_TrackController[ TRACK_PITCH ].SetMixMode( anim_track_controller::MIX_ADDITIVE );
                    m_TrackController[ TRACK_PITCH ].SetAdditveRefFrame( iPitchRefFrame );


                    m_TrackController[  TRACK_YAW  ].SetAnimGroup( m_hAnimGroup );                    
                    m_TrackController[  TRACK_YAW  ].SetAnim( "YAW",   TRUE );                    
                    m_TrackController[  TRACK_YAW  ].SetFrameParametric( 0 );
                    m_TrackController[  TRACK_YAW  ].SetWeight( 1 );
                    m_TrackController[  TRACK_YAW  ].SetMixMode( anim_track_controller::MIX_ADDITIVE );
                    m_TrackController[  TRACK_YAW  ].SetAdditveRefFrame( iYawRefFrame );

                    m_Yaw = m_RestYaw;
                    m_Pitch = m_RestPitch;

                    UpdateTrackControllers();

                    UpdateAnimList();
                }
            }
        }
        return( TRUE );
    }
    else
    if( I.IsVar( "Turret\\Audio Package" ) )
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
    else if( I.IsVar( "Turret\\Projectile Blueprint Path" ) )
    {
        if( I.IsRead() )
        {
            if( m_ProjectileTemplateID < 0 )
            {
                I.SetVarFileName("",256);
            }
            else
            {
                I.SetVarFileName( g_TemplateStringMgr.GetString( m_ProjectileTemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( I.GetVarFileName() ) > 0 )
            {
                m_ProjectileTemplateID = g_TemplateStringMgr.Add( I.GetVarFileName() );
            }
        }
        return TRUE;
    }
    else
    if( I.IsVar( "Turret\\IsActive" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( IsActive() );
        }
        else
        {
            OnActivate( I.GetVarBool() );
        }
        return TRUE;
    }
    if( I.VarFloat( "Turret\\Alert Distance", ( m_RadiusSense ) ) )
        return TRUE;
    else
    if( I.VarFloat( "Turret\\Firing Distance", ( m_RadiusFire ) ) )
        return TRUE;
    else
    if( I.IsVar( "Turret\\Ignore Damageable" ) )
    {
        xbool Temp = m_bIgnoresDamageable;

        if( I.IsRead() )
        {
            I.SetVarBool( Temp );
        }
        else
        {
            m_bIgnoresDamageable = I.GetVarBool();
        }
        return TRUE;
    }
    else
    if( I.VarFloat( "Turret\\Scan Time", m_TimeBetweenScans ) )
        return TRUE;
    else
    if( I.VarFloat( "Turret\\Time Between Shots", m_TimeBetweenShots ) )
        return TRUE;
    else
    if( I.VarInt( "Turret\\Shots Between Reloads", m_nShotsBetweenReloads ) )
        return TRUE;
    else
    if( I.VarInt( "Turret\\Min Shots", m_nMinShots ) )
        return TRUE;
    else
    if( I.VarFloat( "Turret\\Cooling Time", m_TimeCool ) )
        return TRUE;
    else
    if( I.VarAngle( "Turret\\Pitch Speed", m_PitchSpeed ) )
        return TRUE;
    else
    if( I.VarAngle( "Turret\\Yaw Speed", m_YawSpeed ) )
        return TRUE;
    // Animation
    else 
    if (I.IsVar( "Turret\\Civilian Cover Anim Package"))
    {
        if (I.IsRead())
        {        
            I.SetVarExternal( m_hCivilianAnimGroup.GetName(), RESOURCE_NAME_SIZE);
        }
        else
        {
            // Anim changed?
            if( I.GetVarExternal()[0] )
            {
                m_hCivilianAnimGroup.SetName( I.GetVarExternal() );
                //x_strcpy(m_CivilianAnimPackageFileName,I.GetVarExternal());
            }
        }
        return TRUE;
    }
    else 
    if (I.IsVar( "Turret\\Soldier Cover Anim Package"))
    {
        if (I.IsRead())
        {        
            I.SetVarExternal( m_hSoldierAnimGroup.GetName(), RESOURCE_NAME_SIZE);
        }
        else
        {
            // Anim changed?
            if( I.GetVarExternal()[0] )
            {
                m_hSoldierAnimGroup.SetName( I.GetVarExternal() );
                //x_strcpy(m_SoldierAnimPackageFileName,I.GetVarExternal());
            }
        }
        return TRUE;
    }
    else 
    if (I.IsVar( "Turret\\Grey Cover Anim Package"))
    {
        if (I.IsRead())
        {        
            I.SetVarExternal( m_hGreyAnimGroup.GetName(), RESOURCE_NAME_SIZE);
        }
        else
        {
            // Anim changed?
            if( I.GetVarExternal()[0] )
            {
                m_hGreyAnimGroup.SetName( I.GetVarExternal() );
                //x_strcpy(m_GreyAnimPackageFileName,I.GetVarExternal());
            }
        }
        return TRUE;
    }
    else
    if (I.VarGUID("Turret\\Entry Marker",m_EntryMarker))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Controller",m_Controller))
        return TRUE;
    else
    if( I.IsVar( "Turret\\Scripting\\Target GUID" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarGUID( m_TargetGuid );
        }
        else
        {
            SetTargetGuid( I.GetVarGUID() );
        }
        return TRUE;
    }
    else
    if( I.VarAngle( "Turret\\Pitch Up Limit", m_PitchUpLimit) )
    {
        m_PitchUpLimit = MIN(R_90,MAX(R_0,m_PitchUpLimit));
        
        if ((m_PitchDownLimit + m_PitchUpLimit) < (R_360 * 0.995f))
            m_bPitchLimited = TRUE;
        else 
            m_bPitchLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Turret\\Pitch Down Limit", m_PitchDownLimit ) )
    {
        m_PitchDownLimit = MIN(R_90,MAX(R_0,m_PitchDownLimit));

        if ((m_PitchDownLimit + m_PitchUpLimit) < (R_360 * 0.995f))
            m_bPitchLimited = TRUE;
        else 
            m_bPitchLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Turret\\Yaw Right Limit", m_YawRightLimit) )
    {
        if (m_YawRightLimit + m_YawLeftLimit > R_360)
            m_YawLeftLimit = R_360 - m_YawRightLimit;

        if ((m_YawLeftLimit + m_YawRightLimit) < (R_360 * 0.995f))
            m_bYawLimited = TRUE;
        else 
            m_bYawLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Turret\\Yaw Left Limit", m_YawLeftLimit ) )
    {
        if (m_YawRightLimit + m_YawLeftLimit > R_360)
            m_YawRightLimit = R_360 - m_YawLeftLimit;

        if ((m_YawLeftLimit + m_YawRightLimit) < (R_360 * 0.995f))
            m_bYawLimited = TRUE;
        else 
            m_bYawLimited = FALSE;
        return TRUE;
    }
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Destroy",m_ActivateOnDestruction))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Activation",m_ActivateOnActivation))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Deactivation",m_ActivateOnDeactivation))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Target Acquired",m_ActivateOnTargetAcquisition))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Target Lost",m_ActivateOnTargetLoss))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Fire",m_ActivateOnFire))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Reload",m_ActivateOnReload))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Activate On Scan",m_ActivateOnScan))
        return TRUE;
    else
    if (I.VarGUID("Turret\\Scripting\\Tracking Object",m_TrackingGuid))
        return TRUE;
    else
    if (I.IsVar("Turret\\Scripting\\Objective Object"))
    {
        if (I.IsRead())
        {
            I.SetVarGUID( m_ObjectiveGuid );            
        }
        else
        {
            guid NewGuid = I.GetVarGUID();

            // If we are setting the objective guid to NULL, and we are
            // currently targeting the objective, then clear the current target.
            if ((m_ObjectiveGuid == m_TargetGuid) && (NewGuid == NULL_GUID))
            {
                SetTargetGuid( NULL_GUID );
            }
            m_ObjectiveGuid = NewGuid;
        }
        return TRUE;
    }
    else
    if( I.VarFloat( "Turret\\Health", m_Health ) )
    {
        if (!I.IsRead())
            m_MaxHealth = m_Health;

        return TRUE;
    }
    if( I.VarFloat( "Turret\\Hit Percentage", m_HitPercentage ) )
    {
        m_HitPercentage = MIN(100,MAX(0,m_HitPercentage));
        return TRUE;
    }
    else
    if( I.IsVar( "Turret\\Indestructible" ) )
    {
        xbool Temp = m_bIndestructable;

        if( I.IsRead() )
        {
            I.SetVarBool( Temp );
        }
        else
        {
            m_bIndestructable = I.GetVarBool();
        }
        return TRUE;
    }
    if( I.IsVar( "Turret\\Inactive Indestructible" ) )
    {
        xbool Temp = m_bInactiveIndestructable;

        if( I.IsRead() )
        {
            I.SetVarBool( Temp );
        }
        else
        {
            m_bInactiveIndestructable = I.GetVarBool();
        }
        return TRUE;
    }
    else
    if( I.IsVar( "Turret\\Animation\\Play Complete Idle" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( m_bPlayIdleCompletely );
        }
        else
        {
            m_bPlayIdleCompletely = I.GetVarBool();
        }
        return TRUE;
    }
    if( I.IsVar( "Turret\\Orb Power\\Requires Orb Power" ) )
    {
        xbool Temp = m_bRequiesOrbPower;

        if( I.IsRead() )
        {
            I.SetVarBool( Temp );
        }
        else
        {
            m_bRequiesOrbPower = I.GetVarBool();
        }
        return TRUE;
    }
    else if (I.VarVector3("Turret\\Orb Power\\Launch Vector", m_LaunchDelta ))
    {
        return TRUE;
    }
    else if (I.IsVar("Turret\\Delete On Death" ))
    {
        xbool Temp = m_bDestroySelfWhenDead;
        if (I.IsRead())
        {
            I.SetVarBool( Temp );
        }
        else
        {
            m_bDestroySelfWhenDead = I.GetVarBool();
        }
        return TRUE;
    }
    else if( SMP_UTIL_IsAudioVar( I, "Turret\\Audio\\Aiming", m_hAudioPackage, m_AimingSoundID ) )
    {
        if( (m_AimingSoundID == -1) && (m_StopAimingSoundID == -1) )
            m_hAudioPackage.SetName( "" );

        return( TRUE );
    }
    else if( SMP_UTIL_IsAudioVar( I, "Turret\\Audio\\Stop Aiming", m_hAudioPackage, m_StopAimingSoundID ) )
    {
        if( (m_AimingSoundID == -1) && (m_StopAimingSoundID == -1) )
            m_hAudioPackage.SetName( "" );

        return( TRUE );
    }
    else
    if( I.VarFloat( "Turret\\Audio\\Time until aim stopped", m_IdleTimeUntilStopAim ) )
        return TRUE;    
    else if ( I.IsVar( "Turret\\Logical Name"))
    {
        if (I.IsRead())
        {
            if (m_LogicalName >= 0)
            {
                const char* String = g_StringMgr.GetString( m_LogicalName );
                I.SetVarString( String, x_strlen( String )+1 );
            }
            else
                I.SetVarString( "TURRET", 32 );                           
        }
        else
        {
            m_LogicalName = g_StringMgr.Add( I.GetVarString() );
        }
        return TRUE;
    }
    else if ( I.VarGUID( "Turret\\Manned\\Left Boundary", m_LeftBoundary ) )
            return TRUE;
    else if ( I.VarGUID( "Turret\\Manned\\Right Boundary", m_RightBoundary ) )
        return TRUE;
    else if ( I.VarGUID( "Turret\\Manned\\Upper Boundary", m_UpperBoundary ) )
        return TRUE;
    else if ( I.VarGUID( "Turret\\Manned\\Lower Boundary", m_LowerBoundary ) )
        return TRUE;


    return( FALSE );
}

//=========================================================================

void turret::UpdateAnimList( void )
{
    m_bIsFunctional             = FALSE;         
    m_bHasAlert                 = FALSE;             
    m_bHasActiveIdle            = FALSE;
    m_bHasEngagedIdle           = FALSE;    
    m_bHasActivate              = FALSE;          
    m_bHasDeactivate            = FALSE;        
    m_bHasReload                = FALSE;    
    m_bHasRampUp                = FALSE;
    m_bHasRampDown              = FALSE;
    m_bHasActiveDeath           = FALSE;
    m_bHasInactiveDeath         = FALSE;
    m_bHasDestroyed             = FALSE;

    if( !m_hAnimGroup.IsLoaded() )
        return;

    anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

    xbool bHasInactiveIdle  = (pAnimGroup->GetAnimIndex( "INACTIVE_IDLE" )==-1?FALSE:TRUE);                 
    xbool bHasPitch         = (pAnimGroup->GetAnimIndex( "PITCH"         )==-1?FALSE:TRUE);             
    xbool bHasYaw           = (pAnimGroup->GetAnimIndex( "YAW"           )==-1?FALSE:TRUE);             
    
    m_bHasFire                  = (pAnimGroup->GetAnimIndex( "SHOOT"            )==-1?FALSE:TRUE);             
    m_bHasAlert                 = (pAnimGroup->GetAnimIndex( "ALERT"            )==-1?FALSE:TRUE);             
    m_bHasActiveIdle            = (pAnimGroup->GetAnimIndex( "ACTIVE_IDLE"      )==-1?FALSE:TRUE);
    m_bHasEngagedIdle           = (pAnimGroup->GetAnimIndex( "ENGAGED_IDLE"     )==-1?FALSE:TRUE);        
    m_bHasActivate              = (pAnimGroup->GetAnimIndex( "ACTIVATE"         )==-1?FALSE:TRUE);          
    m_bHasDeactivate            = (pAnimGroup->GetAnimIndex( "DEACTIVATE"       )==-1?FALSE:TRUE);        
    m_bHasReload                = (pAnimGroup->GetAnimIndex( "RELOAD"           )==-1?FALSE:TRUE);           
    m_bHasRampUp                = (pAnimGroup->GetAnimIndex( "RAMP_UP"          )==-1?FALSE:TRUE);        
    m_bHasRampDown              = (pAnimGroup->GetAnimIndex( "RAMP_DOWN"        )==-1?FALSE:TRUE);           
    m_bHasActiveDeath           = (pAnimGroup->GetAnimIndex( "ACTIVE_DEATH"     )==-1?FALSE:TRUE);           
    m_bHasInactiveDeath         = (pAnimGroup->GetAnimIndex( "INACTIVE_DEATH"   )==-1?FALSE:TRUE);           
    m_bHasDestroyed             = (pAnimGroup->GetAnimIndex( "DESTROYED"        )==-1?FALSE:TRUE);           

    m_bIsFunctional =    bHasInactiveIdle 
                      && bHasPitch
                      && bHasYaw;
    
    
}

//=========================================================================

void turret::GetBoneL2W( s32 iBone, matrix4& L2W )
{
    const matrix4* pBone = GetBoneL2Ws() ;

    if( pBone )
    {
        L2W = pBone[iBone];
    }
    else
    {
        L2W.Identity();
    }
}

//=========================================================================

bbox turret::GetLocalBBox( void ) const
{
    if (m_hAnimGroup.GetPointer())
    {
        return m_hAnimGroup.GetPointer()->GetBBox();
    }
    else
    {
        return play_surface::GetLocalBBox();
    }
}

//=========================================================================

void turret::Fire( void )
{

    m_ShotCount++;
    
}

//=========================================================================

vector3 turret::GetObjectAimAt( guid Guid, actor::eOffsetPos OfsPos)
{
    if (Guid == 0)
        return vector3(0,0,0);

    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );

    if (NULL == pObj)
        return vector3(0,0,0);    
    
    vector3 TargetPos = pObj->GetBBox().GetCenter();

    if (pObj->IsKindOf( actor::GetRTTI() ) )
    {
        actor& Actor = actor::GetSafeType( *pObj );

        TargetPos = Actor.GetPositionWithOffset( OfsPos );
    }

    return TargetPos;
}

//=========================================================================

xbool turret::GetTargetSightYaw( guid Guid, radian& Yaw )
{
    if (Guid == 0)
        return FALSE;

    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );

    if (NULL == pObj)
        return FALSE;
    
    //vector3 TargetPos = pObj->GetPosition();

    if (pObj->IsKindOf( actor::GetRTTI() ) )
    {
        actor& Actor = actor::GetSafeType( *pObj );

        Yaw = Actor.GetSightYaw();

        return TRUE;
    }

    return FALSE;
}

//=========================================================================

f32 turret::DistToObjectAimAt( guid Guid )
{
    vector3     Target = GetObjectAimAt( Guid );
    vector3     SensorPos;
    radian3     SensorRot;

    GetSensorInfo( SensorPos, SensorRot );


    return (Target - SensorPos).Length();
}

//=========================================================================

xbool turret::CanSenseTarget( void )
{
    if ( DistToObjectAimAt( m_TargetGuid ) > (GetEffectiveRadiusSense()) )
        return FALSE;

    if ( !CheckLOS( m_TargetGuid ) )
        return FALSE;

    return TRUE;
}

//=========================================================================

xbool turret::CanFireAtTarget( void )
{
    if ( DistToObjectAimAt( m_TargetGuid ) > (GetEffectiveRadiusFire()) )
        return FALSE;

    //  LOS only matters when we are targeting something other than the objective guid
    if (m_TargetGuid != m_ObjectiveGuid)
    {   
//        slot_id SID = g_ObjMgr.GetSlotFromGuid( m_TargetGuid );
        {
            vector3 TargetPos = GetObjectAimAt( m_TargetGuid );
        
            if ( !CheckLOS( TargetPos, object::ATTR_BLOCKS_ALL_PROJECTILES ) )
                return FALSE;
        }
    }
    return TRUE;
}

//=========================================================================

xbool turret::IsTargetInRange( void )
{
    f32 Dist = DistToObjectAimAt( m_TargetGuid );

    if ( (Dist > (GetEffectiveRadiusFire())) && (Dist > (GetEffectiveRadiusSense())) )
        return FALSE;

    return TRUE;
}

//=========================================================================

xbool turret::CheckLOS( const vector3& Pt, u32 Attr )
{
    vector3 SensorPos;
    radian3 SensorRot;
    GetSensorInfo( SensorPos, SensorRot );

    u32 IgnoreFlags = object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING;
    if (m_bIgnoresDamageable)
    {
        IgnoreFlags |= object::ATTR_DAMAGEABLE;
    }    
    
    g_CollisionMgr.LineOfSightSetup( GetGuid(), SensorPos, Pt );
    g_CollisionMgr.AddToIgnoreList( GetGuid() );
    g_CollisionMgr.AddToIgnoreList( m_TargetGuid );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, Attr, (object::object_attr)(IgnoreFlags) );

	xbool bCollided = FALSE;
	if( g_CollisionMgr.m_nCollisions != 0 )
	{
		if ((g_CollisionMgr.m_Collisions[0].T > 0.01f) &&
			(g_CollisionMgr.m_Collisions[0].T < 0.99f))
		{
			bCollided = TRUE;
		}
	}

    //no collisions, move the object.
    if( !bCollided )
    {
        return TRUE;
    } 

    return FALSE;
}

//=========================================================================

xbool turret::CheckLOS( guid Guid, u32 Attr )
{
    slot_id SID = g_ObjMgr.GetSlotFromGuid( Guid );
    if ( SID == SLOT_NULL )
        return FALSE;

    if (m_LOSCheckTimer > k_TimeBetweenLOSChecks)
    {
        m_LOSCheckTimer          = 0;
        m_ObjSlotForLastLOSCheck = SLOT_NULL;
    }

    if ( SID == m_ObjSlotForLastLOSCheck )
        return m_bLastLOSPassed;

    vector3 TargetPos = GetObjectAimAt( Guid );

    m_bLastLOSPassed         = CheckLOS( TargetPos, Attr );    
    m_ObjSlotForLastLOSCheck = SID;

    return m_bLastLOSPassed;
}

//=========================================================================

void turret::UpdateAim( f32 DeltaTime, radian Pitch, radian Yaw )
{
    radian DeltaP, DeltaY;
    radian TurnP,  TurnY;
      
    DeltaP = x_MinAngleDiff( Pitch, m_Pitch );
    DeltaY = x_MinAngleDiff( Yaw,   m_Yaw   );
    TurnP  = m_PitchSpeed * DeltaTime;
    TurnY  = m_YawSpeed * DeltaTime;
    
    radian InitialPitch = m_Pitch;
    radian InitialYaw   = m_Yaw;

    if (m_bYawLimited)
    {
        if ((Yaw < (R_360-m_YawLeftLimit) ) &&
            (Yaw > m_YawRightLimit))
        {
            // Desired yaw is outside turret limits
            radian DeltaLeft  = x_MinAngleDiff( m_YawLeftLimit,  Yaw );
            radian DeltaRight = x_MinAngleDiff( R_360-m_YawRightLimit, Yaw );

            if (DeltaLeft < DeltaRight)
                DeltaY = TurnY;
            else
                DeltaY = -TurnY;
        }             
    }
/*
    if (m_bPitchLimited)
    {
        if ((Pitch < R_360-m_YawLeftLimit) ||
            (Pitch > m_YawRightLimit))
        {
            // Desired yaw is outside turret limits
            radian DeltaLeft  = x_MinAngleDiff( m_YawLeftLimit,  Yaw );
            radian DeltaRight = x_MinAngleDiff( m_YawRightLimit, Yaw );

            DeltaY = MIN( DeltaLeft, DeltaRight );
        }     
    }
  */  
    if( x_abs( DeltaP ) < TurnP )
    {
        m_Pitch = Pitch;
    }
    else
    {
        if( DeltaP > R_0 )  
            m_Pitch += TurnP;
        else                
            m_Pitch -= TurnP;
    }

    if( x_abs( DeltaY ) < TurnY )
    {
        m_Yaw = Yaw;     
    }
    else
    {
        if( DeltaY > R_0 )  
            m_Yaw += TurnY;
        else
            m_Yaw -= TurnY;
    }

    m_Yaw   = x_ModAngle( m_Yaw   );
    m_Pitch = x_ModAngle( m_Pitch );

    LimitPitchYaw( m_Pitch, m_Yaw );

    UpdateTrackControllers();


    // Are we locked on?
    m_LocalAimDir.Set(0,0,-1);
    m_LocalAimDir.Rotate( radian3( -m_Pitch, m_Yaw, 0 ));


    // Did we update?
    radian Diff = x_abs(m_Pitch - InitialPitch) + x_abs(m_Yaw - InitialYaw);

    if (Diff > 0.001f)
    {
        // We adjusted the turret.
        m_TimeSinceLastAimCorrection = 0;

        if (!m_bAimStartLoopPlaying)
        {
            // The loop is not currently being played, we need to start it up.

            if (m_AimingSoundVoice != -1)
                g_AudioMgr.Release( m_AimingSoundVoice, 0 );

            if (m_AimingSoundID != -1)
            {
                const char* pSoundName = g_StringMgr.GetString( m_AimingSoundID );
                if (pSoundName)
                {
                    const matrix4& L2W = GetL2W();
                    m_AimingSoundVoice = g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
                }            

                m_bAimStartLoopPlaying = TRUE;
            }
        }

        if ( m_AimingSoundVoice != -1 )
        {
            f32 DeltaTotal = x_abs(DeltaP) + x_abs(DeltaY);
            f32 T = (DeltaTotal - R_3) / R_20;

            T = MIN(1.0f,MAX(0.0f,T));
            
            // Pitch goes up as the turret moves
            f32 Pitch = 1.0f + T * 0.4f;
            f32 Volume = 1.0f + MIN(1.0f,T * 2);

            g_AudioMgr.SetPitch( m_AimingSoundVoice, Pitch );
            g_AudioMgr.SetVolume( m_AimingSoundVoice, Volume );

        }
    }
}
 
//=========================================================================

xbool turret::IsAimedAt( radian Pitch, radian Yaw )
{
    vector3 TargetDir(0,0,-1);
    TargetDir.Rotate( radian3( -Pitch, Yaw, 0 ));

    f32 FiringCone = m_LocalAimDir.Dot( TargetDir );
    if (FiringCone > 0.9999f)
        return TRUE;

    f32 Angle = x_acos( FiringCone );

    if (Angle <= m_FiringConeFOV)
        return TRUE;
    
    return FALSE;
}

//=========================================================================

xbool turret::LookForTarget( void )
{
    if( m_ScanTimer < m_TimeBetweenScans )
        return FALSE;

    if (!IsPowered())
        return FALSE;
 
    m_ScanTimer = 0;

    //
    //  Check to see if the objective (if we had one) is still valid
    //
    if (m_ObjectiveGuid != 0)
    {
        // We always go after the objective guid, if it is
        // valid.
        //
        // So... verify that it is indeed an object
        // 
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ObjectiveGuid );

        if (NULL == pObj)
        {
            SetObjectiveGuid(NULL_GUID);
        }
        else
        {
            ASSERT( pObj ); 
    /*
            // Make sure that it is active
            if (!pObj->IsActive())
            {
                SetObjectiveGuid(NULL);
            }
            else
*/
            {
                // If it's an actor derivitive, it has the extra
                // constraint that living is mandatory
                if (pObj->IsKindOf( actor::GetRTTI() ) )
                {
                    actor& Actor = actor::GetSafeType( *pObj );

                    if (!Actor.IsAlive())
                        SetObjectiveGuid(NULL_GUID);
                }
            }
        }
    }

    //
    //  If we passed through the cleansing phase above, and the objective
    //  is still valid, then we target it.
    //
    if (m_ObjectiveGuid != 0)
    {
        m_TargetGuid = m_ObjectiveGuid;
        return TRUE;
    }

    //
    //  Requirements for a valid target:
    //
    //      1.  Must be within sense radius
    //      2.  Must have LOS to target
    //

    if (m_TargetGuid == NULL_GUID)
    {
        ActivateObject( m_ActivateOnScan );
    }

    vector3 SensorPos;
    radian3 SensorRot;
    GetSensorInfo( SensorPos, SensorRot );


    // If we are tracking a real object
    if (    (m_TargetGuid != 0)
         && (m_TargetGuid != m_TrackingGuid) )
    {
        // Validate the target
        //
        //  It's ok to have a target that we can't shoot at because of
        //  turret yaw/pitch limits.
        //
        //  We on lose the target if it is out of range or hidden
        if (IsTargetInRange())
        {
            if ( CheckLOS( m_TargetGuid ) )
                return TRUE;
        }

        // The target we had is now invalid
        SetTargetGuid( 0 );
    }

    guid        BestGuidInFOF       = 0;
    guid        BestGuidOutsideFOF  = 0;

    f32         BestScoreInFOF      = -1e30f;
    f32         BestScoreOutsideFOF = 1e30f;
    s32         i;
        
    bbox TargetingBBox;
    TargetingBBox.Set( SensorPos, (GetEffectiveRadiusSense()) );

    const s32   kMAX_CONTACTS = 16;
    guid        GuidList[kMAX_CONTACTS];
    s32         GuidCount = 0;

    // Loop through active actors looking for a target
    actor* pActor = actor::m_pFirstActive;
    while( pActor && (GuidCount<kMAX_CONTACTS) )
    {
        if( (!( pActor->GetFaction() & m_FriendFaction )) && 
              (pActor->IsAlive()) && 
              !(pActor->GetFaction() & FACTION_NEUTRAL) )
        {
            f32 DistSquared = (pActor->GetPosition() - SensorPos).LengthSquared();
            if( DistSquared < (GetEffectiveRadiusSense())*(GetEffectiveRadiusSense()) )
            {
                GuidList[ GuidCount ]= pActor->GetGuid();
			    GuidCount++;                        
            }
        }

        pActor = pActor->m_pNextActive;
    }

    // Clean the list
    for (i=0;i<GuidCount;i++)
    {
        if ( CheckLOS( GuidList[i] ))
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( GuidList[i] );

            // Only process actors
            if (!pObj->IsKindOf( actor::GetRTTI() ))
                continue;

            actor& Actor = actor::GetSafeType( *pObj );
            vector3 Pt = Actor.GetPositionWithOffset( actor::OFFSET_EYES );
    
            f32 Score = GetAimScoreForPoint( Pt );
            if (Score >= 0)
            {
                if (Score > BestScoreInFOF)
                {
                    BestScoreInFOF = Score;
                    BestGuidInFOF  = GuidList[i];
                }
            }
            else
            {
                if (Score < BestScoreOutsideFOF)
                {
                    BestScoreOutsideFOF = Score;
                    BestGuidOutsideFOF  = GuidList[i];
                }
            }
        }
    }

    guid    NewTarget = 0;

    if ( 0 != BestGuidInFOF )
        NewTarget = BestGuidInFOF;   
    else
        NewTarget = BestGuidOutsideFOF;

    if ( 0 != NewTarget )
    {
        if (NewTarget != m_TargetGuid)
        {
            DEBUG_LOG_MSG("Turret","Look for target SUCCESSFUL");
            SetTargetGuid( NewTarget );
        }

        return TRUE;
    }	      

    // We found no target
    // If we have a tracking guid, use that.
    if ( 0 != m_TrackingGuid )
    {
        SetTargetGuid( m_TrackingGuid );
        return TRUE;
    }
    
    return FALSE;
}

//=========================================================================

f32 turret::GetAimScoreForPoint( const vector3& Pt )
{
    vector3     TargetPos = Pt;
    vector3     SensorPos;
    radian3     SensorRot;

    GetSensorInfo( SensorPos, SensorRot );

    matrix4 W2L = GetL2W();

    W2L.InvertRT();   

    TargetPos = W2L.Transform( TargetPos );
    SensorPos = W2L.Transform( SensorPos );

    vector3 Delta = TargetPos - SensorPos;

    radian  Yaw,Pitch;
    Delta.GetPitchYaw(Pitch,Yaw);
    Delta.Normalize();

    Pitch = x_ModAngle2( Pitch );
    Yaw   = x_ModAngle2( Yaw+R_180 );

    xbool bInYaw = FALSE;
    xbool bInPitch = FALSE;

    if ((Yaw < m_YawLeftLimit) && (Yaw > -m_YawRightLimit))
        bInYaw = TRUE;
    
    if ((Pitch < m_PitchDownLimit) && (Pitch > -m_PitchUpLimit))
        bInPitch = TRUE;

    f32 Score;

    if (bInPitch && bInYaw)
    {
        // Inside FOF
        // Score will be positive       
        f32 Dot = Delta.Dot( m_LocalAimDir );

        // Massage from [-1,1] to [0,1]
        Score = (Dot+1)/2;
    }
    else
    {
        // Outside FOF
        // Score will be negative
        f32 Dot = Delta.Dot( m_LocalAimDir );

        // Massage from [-1,1] to [-1,0]
        Score = (Dot-1)/2;
    }

    return Score;
}

//=========================================================================

void turret::SetTargetGuid( guid Guid )
{
    m_TargetGuid = Guid;

    m_TargetPos = GetObjectAimAt( m_TargetGuid );
    m_TargetVel.Set(0,0,0);

    if (m_TargetGuid == 0)
    {
        ActivateObject( m_ActivateOnTargetLoss );
    }
    else
    {
        ActivateObject( m_ActivateOnTargetAcquisition );
    }
}   

//=========================================================================

const char* turret::GetCurrentStateName( void )
{
    return GetStateName( m_State );
}

//=========================================================================

const char* turret::GetStateName( state State )
{
    switch( State )
    {
        default:
            return "UNKNOWN TURRET STATE";
        case STATE_UNDEFINED:
            return "STATE_UNDEFINED";
        case STATE_IDLE:
            return "STATE_IDLE";
        case STATE_ACTIVATING:
            return "STATE_ACTIVATING";
        case STATE_AIMING:
            return "STATE_AIMING";
        case STATE_RAMP_UP:
            return "STATE_RAMP_UP";
        case STATE_FIRING:
            return "STATE_FIRING";
        case STATE_RAMP_DOWN:
            return "STATE_RAMP_DOWN";
        case STATE_RELOADING:
            return "STATE_RELOADING";
        case STATE_COOLING:
            return "STATE_COOLING";
        case STATE_UNAIMING:
            return "STATE_UNAIMING";
        case STATE_DEACTIVATING:
            return "STATE_DEACTIVATING";
        case STATE_DESTROY_BEGIN:
            return "STATE_DESTROY_BEGIN";
        case STATE_DESTROYED:
            return "STATE_DESTROYED";
    }
}

//=========================================================================

xbool turret::IsTargetValid( void )
{
    if (m_TargetGuid == 0)
        return FALSE;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pObj)
        return FALSE;

    ASSERT( pObj ); 
    
    if (pObj->IsKindOf( actor::GetRTTI() ) )
    {
        actor& Actor = actor::GetSafeType( *pObj );

        return !!(Actor.GetParametricHealth() > 0);
    }
   
    return TRUE;
}

//=========================================================================

turret::tracking_status turret::TrackTarget( f32 DeltaTime )
{
    if (m_TargetGuid == 0)
        return TRACK_NO_TARGET;

    if (m_TargetGuid == m_ObjectiveGuid)
    {
        // Handle objective objects differently
        if ( DistToObjectAimAt( m_TargetGuid ) > (GetEffectiveRadiusFire()) )
        {
            return TRACK_NO_TARGET;
        }
    }
    else
    {
        
        if (!CanSenseTarget() && !CanFireAtTarget())
        {
            SetTargetGuid(0);
            return TRACK_NO_TARGET;
        }
    }

    if (!IsTargetValid())
    {
        SetTargetGuid(0);
        return TRACK_NO_TARGET;
    }

    // Make sure target is in FOF
    vector3 Pt = GetObjectAimAt( m_TargetGuid );
    if (GetAimScoreForPoint( Pt ) < 0)
    {
        SetTargetGuid(0);
        return TRACK_OUTSIDE_FOF;
    }

    vector3 TargetPos = GetTargetPos();
    vector3 SensorPos;
    radian3 SensorRot;
    GetSensorInfo( SensorPos, SensorRot );

    // We want to work with position and angles in local turret space
    matrix4 W2L = GetL2W();

    W2L.InvertRT();

    TargetPos = W2L.Transform( TargetPos );
    SensorPos = W2L.Transform( SensorPos );

    vector3 Delta = TargetPos - SensorPos;
    m_LocalTargetDir = Delta;
    m_LocalTargetDir.Normalize();

    radian  Yaw,Pitch;
    Delta.GetPitchYaw(Pitch,Yaw);

    // Look for out of range condition
    Delta.GetY() = 0;
    f32 Dist = Delta.Length();
    if (Dist > MAX( (GetEffectiveRadiusFire()), (GetEffectiveRadiusSense()) ))
        return TRACK_NO_TARGET;

    Yaw += R_180;

    // Limit pitch and yaw
    LimitPitchYaw( Pitch, Yaw );

    // Update the aimer
    UpdateAim( DeltaTime, Pitch, Yaw );
    
    // Check to see if we are aimed at the target
    m_LocalTargetDir.GetPitchYaw( Pitch, Yaw );
    Yaw += R_180;
    if (IsAimedAt( Pitch, Yaw ))                    
        return TRACK_LOCKED;

    return TRACK_AIMING;
}

//=========================================================================

void turret::LimitPitchYaw( radian& Pitch, radian& Yaw )
{
    radian Yaw2   = x_ModAngle2( Yaw );
    radian Pitch2 = x_ModAngle2( Pitch );

    if (m_bYawLimited)
    {
        if ((Yaw2 < -m_YawLeftLimit) ||
            (Yaw2 > m_YawRightLimit))
        {
            radian Mid = m_YawRightLimit + ((R_360-m_YawLeftLimit)-m_YawRightLimit) / 2.0f;

            if (Yaw < Mid)
                Yaw = m_YawLeftLimit;
            else 
                Yaw = -m_YawRightLimit;
        }
    }

    if (m_bPitchLimited)
    {
        if (Pitch2 < -m_PitchUpLimit)
        {
            Pitch = -m_PitchUpLimit;
        }
        else if (Pitch2 > m_PitchDownLimit)
        {
            Pitch = m_PitchDownLimit;
        }            
    }

    Yaw   = x_ModAngle( Yaw );
    Pitch = x_ModAngle( Pitch );
}

//=========================================================================

vector3 turret::GetTargetPos( void )
{
    return GetObjectAimAt( m_TargetGuid );    
}

//=========================================================================

void turret::UpdateTrackControllers()
{
    radian Pitch = x_ModAngle2( m_Pitch );
    f32 tP = ((Pitch + (PI/2)) / PI);
    f32 tY = 1.0f - (m_Yaw / (2*PI));

    m_TrackController[ TRACK_PITCH ].SetFrameParametric( tP );
    m_TrackController[  TRACK_YAW  ].SetFrameParametric( tY );
}

//=========================================================================

void turret::GetSensorInfo( vector3& Pos, radian3& Rot )
{
    if( !m_hAnimGroup.IsLoaded() )
        return;

    anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

    s32 iSensorBone = pAnimGroup->GetBoneIndex( "Turret_Sensor" );
    if (iSensorBone == -1)
    {
        // Just use the position of the turret
        Pos = GetPosition();
        Rot.Set(0,0,0);
        return;
    }

    const matrix4* pL2W = m_AnimPlayer.GetBoneL2W( iSensorBone, TRUE );
    if (NULL == pL2W)
    {
        Pos = GetPosition();
        return;
    }

    matrix4 L2W = *pL2W;

    L2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iSensorBone ) );

    Pos = L2W.GetTranslation();
    Rot = L2W.GetRotation();
}

//=========================================================================

void turret::OnEvent( const event& Event )
{
    switch( Event.Type )
    {
    case event::EVENT_WEAPON:
        {
            xbool bAimToHit = (x_frand(0,100) < m_HitPercentage)?TRUE:FALSE;

            const weapon_event& WeaponEvent = weapon_event::GetSafeType( Event );

            const matrix4& L2W = GetL2W();
            //f32 LocalYaw   = L2W.GetRotation().Yaw;
            //f32 LocalPitch = -(L2W.GetRotation().Pitch);

            vector3 AimDir(0,0,1);
            AimDir.Rotate( radian3( m_Pitch, m_Yaw + R_180, 0 ));
            matrix4 L2WNoRot = L2W;
            L2WNoRot.ClearTranslation();
            L2WNoRot.Transform( &AimDir, &AimDir, 1 );

            radian AimYaw,AimPitch;
            AimDir.GetPitchYaw( AimPitch, AimYaw );

            vector3 InitPos = WeaponEvent.Pos;
            radian3 InitRot( AimPitch, AimYaw, 0 );
            vector3 InitVelocity(0,0,1);

            vector3 AimPos = InitPos;

            f32 fProjSpeed = 50000;
                
            // if we have a valid target, adjust the aim
            if (IsTargetValid())
            {
                vector3 TargetPos = m_TargetPos;
                if (!bAimToHit)
                {
                    // Adjust the aim to be some small offset
                    // above the floor in front of the target
                    TargetPos = GetObjectAimAt( m_TargetGuid, actor::OFFSET_AIM_AT );
                    TargetPos.GetY() += x_frand(0,50);

                    // Displace the target pos forward by a small amount
                    // based on the target's sight direction.                    
                    vector3 MissDir = m_TargetVel;
                    f32     VelScale = m_TargetVel.Length();
                    radian  TargetYaw = R_0;

                    if (GetTargetSightYaw( m_TargetGuid, TargetYaw ))
                    {
                        MissDir.Set(0,0,VelScale);
                        MissDir.RotateY( TargetYaw );                        
                    }
                    
                    // Aim into the future
                    TargetPos += MissDir * k_MissFutureTime;

                    // Now, offset the position along the
                    // vector perpendicular to the ToTargetVector
                    vector3 ToTarget = TargetPos - InitPos;
                    ToTarget.GetY() = 0;
                    ToTarget.Normalize();
                    MissDir.Set(0,0,1);
                    MissDir.RotateY( TargetYaw );                                            

                    // This gets us a scale value for how much to
                    // aim along the perp (0,1);
                    f32 PerpMagnitude = x_abs(ToTarget.Dot(MissDir));

                    // Find out how wide the target is
                    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
                    ASSERT( NULL != pObj );
                    f32 Radius = pObj->GetBBox().GetRadius();

                    // Get the perp vector
                    vector3 Perp = ToTarget.Cross( vector3(0,1,0) );
                    Perp.Normalize();

                    //Scale the perp vector
                    // Get it in the range [-1,-0.75] or [0.75,1]
                    f32 MissThickness = 0.25f;
                    PerpMagnitude *= x_frand(-MissThickness,MissThickness);
                    if (PerpMagnitude<0)
                        PerpMagnitude -= (1-MissThickness);
                    else
                        PerpMagnitude += (1-MissThickness);
                    
                    PerpMagnitude *= Radius;

                    Perp.Scale(PerpMagnitude);
                    
                    TargetPos += Perp;
                }

                

                xbool Ret = CalculateLinearAimDirection( TargetPos,
                                                         m_TargetVel,
                                                         InitPos,
                                                         vector3(0,0,0),
                                                         0,
                                                         fProjSpeed,
                                                         10,
                                                         AimDir,
                                                         AimPos );
                
                if (Ret)
                {                    
                    // Verify that we can actually aim to this point
                    vector3     vToAimPos = AimPos - InitPos;
                    vector3     vAimDir(0,0,1);

                    vToAimPos.Normalize();
                    vAimDir.Rotate( InitRot );

                    f32 T = vAimDir.Dot( vToAimPos );

                    if (T > 0.9f)
                    {
                        AimDir.GetPitchYaw( InitRot.Pitch, InitRot.Yaw );
                        InitRot.Roll = 0;
                    }
                }
            }

            InitVelocity.Rotate( InitRot );
            

            const char* pTemplateString = g_TemplateStringMgr.GetString( m_ProjectileTemplateID );

            if( x_stristr( pTemplateString, "MasonCannon" ) )
            {
                guid ProjectileID = CREATE_NET_OBJECT( energy_projectile::GetObjectType(), netobj::TYPE_BBG_1ST );
                energy_projectile* pProjectile = (energy_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
                ASSERT( pProjectile );

                //make sure the bullet was created, make sure the firing bone was initialized.
                if ( pProjectile != NULL )
                {
                    pain_handle PainHandle(GetLogicalName());
                    tweak_handle SpeedTweak( xfs("%s_SPEED",GetLogicalName()) );

                    vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
                    Velocity.Rotate( InitRot );

                    pProjectile->Setup( GetGuid(),
                                        net_GetSlot(),
                                        InitPos,
                                        radian3(0.0f,0.0f,0.0f), //InitRot
                                        Velocity,
                                        GetZone1(),
                                        GetZone2(),
                                        0.0f,
                                        PainHandle );
                }
            }
            else
            {
                guid BulletID = g_TemplateMgr.CreateSingleTemplate( pTemplateString, InitPos, InitRot, GetZone1(), GetZone2() );
                object* pObject = g_ObjMgr.GetObjectByGuid( BulletID );
                ASSERT( pObject );

                //make sure the bullet was created, make sure the firing bone was initialized.
                if ( pObject != NULL )
                {
                    pain_handle PainHandle(GetLogicalName());
                    tweak_handle SpeedTweak( xfs("%s_SPEED",GetLogicalName()) );
                    base_projectile* pBullet = ( base_projectile* )pObject;
                    pBullet->Initialize( InitPos, InitRot, vector3(0,0,0), SpeedTweak.GetF32(), GetGuid(), PainHandle, bAimToHit );
                    pBullet->SetTarget( m_TargetGuid );
                }
            }


/*
            //create Bullet
            guid BulletID = g_ObjMgr.CreateObject( bullet_projectile::GetObjectType() );
            base_projectile* pBullet = (base_projectile*)g_ObjMgr.GetObjectByGuid( BulletID );
   
            f32 ShotDamage = m_ShotDamage;
            f32 ShotForce  = m_ShotForce;

            if (!bAimToHit)
            {
                ShotDamage = 0;
                ShotForce  = 0;
            }
            //make sure the bullet was created, make sure the firing bone was initialized.
            ASSERT( pBullet );    
            pBullet->Initialize( InitPos, 
                                 InitRot, 
                                 InitVelocity,                                  
                                 ShotDamage,
                                 ShotForce,
                                 fProjSpeed,
                                 GetGuid() );
                                 
*/
            //add a muzzle light where the bullet was fired from (will fade out really quickly)
            g_LightMgr.AddFadingLight( WeaponEvent.Pos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );
        /*
            InitRot.Set(0,0,0);
            radian Temp = R_0;
            debris_mgr::GetDebrisMgr()->CreateShell( WeaponEvent.Pos, 
                                                    InitRot + radian3( x_frand(-Temp,Temp), x_frand(-Temp,Temp), x_frand(-Temp,Temp)), 
                                                    "DEB_SMPshell.rigidgeom" );
        */

#ifdef X_DEBUG
            m_LastFireBarrelPos = InitPos;
            m_LastFireTargetPos = m_TargetPos;
            m_LastFireTargetVel = m_TargetVel;
#endif
        }

        break;


    case event::EVENT_GENERIC:
        {
            const generic_event& GenericEvent = generic_event::GetSafeType( Event );                

            if (x_stricmp( GenericEvent.GenericType, "DEATH" ) == 0)
            {
                rigid_inst& RInst = GetRigidInst();
                const geom* pGeom = RInst.GetRigidGeom();
                if( pGeom && (pGeom->m_nVirtualMeshes==2) )
                {
                    u32 VMeshMask = RInst.GetVMeshMask().VMeshMask;
                    VMeshMask ^= 0x3;   // swap the first two meshes
                    RInst.SetVMeshMask( VMeshMask );
                }
            }
        }
        break;
    }     
}

//=========================================================================

void turret::OnPain( const pain& Pain )
{
    // do not take damage if no sate or in a state that ignores pain
    if ( m_bIndestructable )
    {
        return;
    }
    if ( m_bInactiveIndestructable )
    {
        // bail if we are in certain states
        switch(m_State)
        {
        case STATE_IDLE:
        case STATE_PREACTIVATING:
        case STATE_ACTIVATING:
        case STATE_PREDEACTIVATING:
        case STATE_DEACTIVATING:        
        case STATE_DESTROY_BEGIN:
            return;
        }
    }

    // if we have already taken this pain, then throw it away
    if( Pain.GetAnimEventID() >= 0 )
    {
        if( Pain.GetAnimEventID() == m_LastAnimPainID )
        {
            return;
        }
        else
        {
            m_LastAnimPainID = Pain.GetAnimEventID();
        }
    }

    // do not take damage from friendly actors
    object *painSource = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
    if( painSource && painSource->IsKindOf(actor::GetRTTI()) )
    {
        actor &actorSource = actor::GetSafeType( *painSource );        

        if( actorSource.GetFaction() & m_FriendFaction )
        {
            return;
        }
    }

    // do not take damage if dead.
    if( m_Health <= 0.0f )
    {            
        return;
    }
    
    // all tests passed. We will take the damage.
    TakeDamage(Pain);
}

//=========================================================================

void turret::TakeDamage( const pain& Pain )
{
    vector3 Pos;
    radian3 Rot;
    GetSensorInfo( Pos, Rot );

    health_handle HealthHandle(GetLogicalName());
    Pain.ComputeDamageAndForce( HealthHandle, GetGuid(), Pos );

    f32 OrigHealth = m_Health;    
    f32 Damage = Pain.GetDamage();
    m_Health = MAX(0, m_Health - Damage);

    if( m_Health <= 0 )
    {
        OnEnterState( STATE_DESTROY_BEGIN );
    }
    else
    {
        s32 nHealthTiers = 4;
        if (m_MaxHealth != 0)
        {
            f32 OrigHealthT = OrigHealth / m_MaxHealth;
            s32 iOrigTier = (s32)x_ceil(OrigHealthT * nHealthTiers);

            f32 CurHealthT = m_Health / m_MaxHealth;
            s32 iCurTier = (s32)x_ceil(CurHealthT * nHealthTiers);

            if (iOrigTier != iCurTier)
            {
                // We have taken "major" damage.
                // play an FX object
                guid gObj = g_ObjMgr.CreateObject( particle_emitter::GetObjectType() );
                if (NULL_GUID != gObj)
                {
                    object_ptr<particle_emitter> pParticle( gObj );
                    if (pParticle.IsValid())
                    {
                        pParticle->InitParticleFromName(PRELOAD_FILE("spark_burst_001.fxo"));
                        radian3 Rot(0,0,0);
                        vector3 Dir = Pain.GetDirection();
                        //Dir *= -1;
                        Dir.GetPitchYaw(Rot.Pitch, Rot.Yaw);
                        f32 S = 3.5f;
                        pParticle->OnTransform( matrix4( vector3(S,S,S), Rot, Pain.GetPosition() ));  
                        pParticle->SetScale( S );
                        pParticle->SetLogicType( particle_emitter::PLAY_ONCE );
                    }
                }
            }
        }
    }
}

//=========================================================================

xbool turret::CalculateLinearAimDirection(   const vector3& TargetPos,
                                             const vector3& TargetVel,
                                             const vector3& SourcePos,
                                             const vector3& SourceVel,
                                             f32 VelInheritFactor,
                                             f32 LinearSpeed,
                                             f32 LifetimeS,
                                             vector3& AimDirection,
                                             vector3& AimPosition       )
{
    const vector3 EffTargetPos = TargetPos - SourcePos;
    const vector3 EffTargetVel = TargetVel - (SourceVel * VelInheritFactor);

    AimDirection.Zero();

    //
    // This is a straightforward law of cosines calculation.  
    // We're not being especially efficient here, but hey.
    //
    vector3 NormPos = EffTargetPos;
    vector3 NormVel = EffTargetVel;
    if( !NormPos.SafeNormalize() )  return( FALSE );
    NormVel.Normalize();

    const f32 A = EffTargetVel.LengthSquared() - (LinearSpeed*LinearSpeed);
    const f32 B = 2 * EffTargetPos.Length() * EffTargetVel.Length() * NormPos.Dot(NormVel);
    const f32 C = EffTargetPos.LengthSquared();

    const f32 det = (B * B) - (4 * A * C);

    if( IN_RANGE( -0.0001f, A, 0.0001f ) )
    {
        // Value A is 0 or at least damn close to 0.  The division for Sol1 a 
        // few lines below will either die or blow up.  So, bail out 
        // meainingfully.
        AimPosition  = TargetPos;
        AimDirection = (TargetPos - SourcePos);

        return( AimDirection.SafeNormalize() );
    }

    if( det < 0.0f ) 
    {
        // No solution is possible in the real numbers
        return FALSE;
    }

    const f32 Sol1 = (-B + x_sqrt(det)) / (2 * A);
    const f32 Sol2 = (-B - x_sqrt(det)) / (2 * A);

    f32 T;

    if( Sol2 > 0.0f )   T = Sol2;
    else                T = Sol1;
    
    if( T < 0.0f ) 
    {
        return FALSE;
    }

    // Once we know how long the projectile's path will take, it's
    // straightforward to find out where it should go...
    AimPosition  = TargetPos + (EffTargetVel * T);
    AimDirection = (EffTargetPos / (LinearSpeed * T)) + (EffTargetVel / LinearSpeed);

    return( AimDirection.SafeNormalize() && ((T * 0.001f) <= LifetimeS) );
}

//=========================================================================

//=============================================================================

void turret::EnumAttachPoints( xstring& String ) const
{
    String = "BaseObject~TurretAimer~"; 
    
    s32 i;
    
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {            
            s32 nBones = pGroup->GetNBones();    
        
            for (i=0;i<nBones;i++)
            {
                const anim_bone& Bone = pGroup->GetBone( i );

                String += Bone.Name;
                String += "~";
            }        
        }
    }


    for( i=0; String[i]; i++ )
    {
        if( String[i] == '~' )
            String[i] = 0;
    }
}

//=============================================================================

s32 turret::GetAttachPointIDByName( const char* pName ) const
{
    if (x_stricmp(pName,"BaseObject")==0)
        return 0;
    else
    if (x_stricmp(pName,"TurretAimer")==0)
        return 1;
    else
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {            
            s32 nBones = pGroup->GetNBones();    
            s32 i;
            for (i=0;i<nBones;i++)
            {
                const anim_bone& Bone = pGroup->GetBone( i );

                if (x_stricmp( pName, Bone.Name ) == 0)
                {
                    return i+2;
                }            
            }      
        }
    }


    return -1;
}

//=============================================================================

xstring turret::GetAttachPointNameByID( s32 iAttachPt ) const
{
    if (iAttachPt == 0)
        return xstring("BaseObject");
    else
    if (iAttachPt == 1)
        return xstring("TurretAimer");
    else
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {               
            // Decrement by one to bring it into the range [0,nbones)
            iAttachPt -= 2;
            s32 nBones = pGroup->GetNBones();
            if ( (iAttachPt >= 0) &&
                 (iAttachPt < nBones ))
            {
                const anim_bone& Bone = pGroup->GetBone( iAttachPt );

                return Bone.Name;
            }       
        }
    }

    return xstring("INVALID\0");
}

//=============================================================================

void turret::OnAttachedMove( s32             iAttachPt,
                                   const matrix4&  NewChildL2W )
{
    if (iAttachPt == 0)
    {                
        OnTransform( NewChildL2W );
    }
    else
    if (iAttachPt == 1)
    {                
        vector3 Pos;
        radian3 Rot;
        GetSensorInfo( Pos, Rot );
        
        matrix4 NewL2W = NewChildL2W;
        NewL2W.Translate( -Pos );
        OnTransform( NewL2W );
    }
    else
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {                 
            // Decrement by one to bring it into the range [0,nbones)
            iAttachPt -= 2;
            s32 nBones = pGroup->GetNBones();
            if ( (iAttachPt >= 0) &&
                 (iAttachPt < nBones ))
            {
                const matrix4* pTempL2W = m_AnimPlayer.GetBoneL2W( iAttachPt );
                if (pTempL2W)
                {
                    /*
                    matrix4 BoneL2W = *pTempL2W;
                    BoneL2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iAttachPt ) );

                    vector3 BonePos = BoneL2W.GetTranslation();
                    BonePos -= GetPosition();

                    matrix4 NewL2W = L2W;
                    NewL2W.Translate( -BonePos );

                    OnTransform( NewL2W );
                    */

                    matrix4 InvBoneL2W = *pTempL2W;
                    InvBoneL2W.InvertRT();
                    
                    matrix4 X = InvBoneL2W * GetL2W();

                    matrix4 NewL2W = NewChildL2W * X;

                    // SH: Added to restore order to the matrix.
                    //      First noticed with the turrets in the alamo level scaling
                    //      outside the spatial database boundaries.
                    NewL2W.Orthogonalize();
                    OnTransform( NewL2W );
                }
            }        
        }
    }
}



//=============================================================================

xbool turret::GetAttachPointData( s32      iAttachPt,
                                  matrix4& L2W,
                                  u32      Flags )
{
    if (iAttachPt == 0)
    {
        L2W = GetL2W();
        return TRUE;
    }
    else
    if (iAttachPt == 1)
    {               
        const matrix4& MyL2W = GetL2W();

        vector3 Pos;
        radian3 Rot;
        GetSensorInfo( Pos, Rot );

        vector3 Aim(0,0,20);
        Aim.Rotate( radian3( m_Pitch, m_Yaw + R_180, 0 ));
        matrix4 L2WNoRot = MyL2W;
        L2WNoRot.ClearTranslation();
        L2WNoRot.Transform( &Aim, &Aim, 1 );

        Pos += Aim;
        
        L2W.Identity();
        L2W.SetTranslation( Pos );

        radian Pitch,Yaw;
        Aim.GetPitchYaw(Pitch,Yaw);
        radian3 Rot2(Pitch,Yaw,0);
        L2W.SetRotation( Rot2 );

        return TRUE;
    }
    else
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {   
            // Decrement by one to bring it into the range [0,nbones)
            iAttachPt -= 2;
            s32 nBones = pGroup->GetNBones();
            if ( (iAttachPt >= 0) &&
                 (iAttachPt < nBones ))
            {
                const matrix4* pL2W = m_AnimPlayer.GetBoneL2W( iAttachPt );            
                if ( NULL != pL2W )
                    L2W = *pL2W;
                else
                    L2W.Identity();
 
                if (Flags & ATTACH_USE_WORLDSPACE)
                    L2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iAttachPt ) );
        
                return TRUE;
            }        
        }
    }

    return FALSE;
}

//=============================================================================

void turret::SetGunner( guid newGunner )
{
    m_Gunner = newGunner;
    if( m_Gunner == 0 )
    {
        u32 attrBits = GetAttrBits();
        attrBits &= ~ATTR_NEEDS_LOGIC_TIME;
        SetAttrBits(attrBits);
        StopAimingSound();
    }
    else
    {
        u32 attrBits = GetAttrBits();
        attrBits |= ATTR_NEEDS_LOGIC_TIME;
        SetAttrBits(attrBits);
    }
}

//=============================================================================

void turret::OnPolyCacheGather( void )
{
    RigidGeom_GatherToPolyCache( GetGuid(), 
                                 GetBBox(), 
                                 m_Inst.GetLODMask(U16_MAX), 
                                 GetBoneL2Ws(), 
                                 m_Inst.GetRigidGeom() );
}

//=============================================================================

void turret::SetRadiusMultiplier( f32 Mult )
{
    m_RadiusMultiplier = Mult;
}

//=============================================================================

f32 turret::GetRadiusMultiplier( void )
{
    return m_RadiusMultiplier;
}

//=============================================================================

void turret::SetObjectiveGuid( guid ObjectiveGuid )
{
    m_ObjectiveGuid = ObjectiveGuid;
}

//=============================================================================

xbool turret::OfferNewObjectiveGuid( guid NewGuid )           
{
    if ( NULL_GUID != m_ObjectiveGuid )
        return FALSE;

    SetObjectiveGuid( NewGuid );

    return TRUE;
}

//=============================================================================

xbool turret::OrbEnter( guid Orb )
{
    if (m_bOrbInside)
        return FALSE;

    if (NULL_GUID == m_ReservedBy)
        m_ReservedBy = Orb;
    if ((Orb != m_ReservedBy))
        return FALSE;

    m_bOrbInside = TRUE;
    return TRUE;
}

//=============================================================================

xbool turret::IsReserved( void )
{
    if (IsDestroyed())
        return TRUE;

    if (NULL_GUID == m_ReservedBy)
        return FALSE;

    return TRUE;
}

//=============================================================================

xbool turret::CanBeReservedByMe( guid Orb )
{
    if (!m_bRequiesOrbPower)
        return FALSE;
    if (IsDestroyed())
        return FALSE;
    if (NULL_GUID == m_ReservedBy)
        return TRUE;
    if (m_ReservedBy == Orb)
        return TRUE;
    return FALSE;
}

//=============================================================================

xbool turret::Reserve( guid Orb )
{
    if (!m_bRequiesOrbPower)
        return FALSE;

    if (Orb == m_ReservedBy)
        return TRUE;

    if (NULL_GUID != m_ReservedBy)
        return FALSE;

    m_ReservedBy = Orb;
    return TRUE;
}

//=============================================================================

xbool turret::Unreserve( guid Orb )
{
    if (!m_bRequiesOrbPower)
        return FALSE;

    if (m_ReservedBy == Orb)
    {
        m_ReservedBy = NULL_GUID;
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

xbool turret::IsPowered( void )
{
    if (m_bRequiesOrbPower)
    {
        if (m_bOrbInside)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
}

//=============================================================================

f32 turret::GetEffectiveRadiusFire( void )
{
    if (m_bRequiesOrbPower && !m_bOrbInside)
    {
        return 0;
    }
    return m_RadiusFire * m_RadiusMultiplier;
}

//=============================================================================

f32 turret::GetEffectiveRadiusSense( void )
{
    if (m_bRequiesOrbPower && !m_bOrbInside)
    {
        return 0;
    }
    return m_RadiusSense * m_RadiusMultiplier;
}

//=============================================================================

void turret::LaunchOrb( void )
{
    // We now destroy the orbs when we die.
   
    if (m_ReservedBy)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ReservedBy );
        if (pObj)
        {
            if (pObj->IsKindOf( alien_orb::GetRTTI()))
            {
                alien_orb& Orb = alien_orb::GetSafeType( *pObj );

                Orb.KillOrb();
            }
        }        
        m_ReservedBy = NULL_GUID;
    }    
}

//=============================================================================

void turret::TryToFireAtTarget( void )
{
    s32 CurrentIdleCycle = m_AnimPlayer.GetCycle();

    if (    CanFireAtTarget()                           
        && m_bHasFire                                  
        && ( m_ActionTimer > m_TimeBetweenShots )      
        && ( !m_bPlayIdleCompletely || ( m_IdleCycle != CurrentIdleCycle )) 
        && ( m_TargetGuid != m_TrackingGuid ) )       
    {
        OnEnterState( STATE_RAMP_UP );
    }
}
//=============================================================================

const char* turret::GetLogicalName( void )
{
    if( m_LogicalName >= 0 )
    {
        return g_StringMgr.GetString( m_LogicalName );
    }
    else
    {    
        return( "TURRET" );
    }
}

//=============================================================================

//=============================================================================


