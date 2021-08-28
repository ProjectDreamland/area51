//=========================================================================
//
//  Stage5.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Stage5.hpp"
#include "Objects\ProjectileBullett.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Audio\audio_stream_mgr.hpp"


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct stage5_desc : public object_desc
{
        stage5_desc( void ) : object_desc( 
            object::TYPE_STAGE5, 
            "NPC - Stage5", 
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_COLLIDABLE         | 
            object::ATTR_RENDERABLE         | 
            object::ATTR_SPACIAL_ENTRY      |
            object::ATTR_CHARACTER_OBJECT   |
            object::ATTR_DAMAGEABLE         |
            object::ATTR_LIVING,
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )   { }

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new stage5; } 

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "SkinGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_stage5_Desc;

//=============================================================================

const object_desc& stage5::GetTypeDesc( void ) const
{
    return s_stage5_Desc;
}

//=============================================================================

const object_desc& stage5::GetObjectType( void )
{
    return s_stage5_Desc;
}


//=========================================================================
// STAGE5_STATE CLASS
//=========================================================================

// Constructor
stage5_state::stage5_state( stage5& Stage5, states State ) :
    character_state(Stage5, State), 
    m_Base(Stage5) 
{
}

//=========================================================================
// STAGE5 IDLE STATE
//=========================================================================

stage5_idle_state::stage5_idle_state( stage5& Stage5, states State ) :
    stage5_state(Stage5, State)
{
}

//=========================================================================

void stage5_idle_state::OnInit( void )
{
    m_FidgetTime = 0 ;
}

//=========================================================================

void stage5_idle_state::OnEnter( void )
{
    // Put loco into idle
    m_Base.m_Loco.SetMoveStyle(loco::MOVE_STYLE_PROWL) ;
    m_Base.m_Loco.SetState(loco::STATE_IDLE) ;
}

//=========================================================================

void stage5_idle_state::OnAdvance( f32 DeltaTime )
{
    (void)DeltaTime ;

    // Stop moving and look forward
    m_Base.StopMovement() ;
    m_Base.LookForward() ;
    
    /*
    // Wait for anim to finish
    if (m_Base.m_Loco.GetState() != loco::STATE_PLAY_ANIM)
    {
        // Update fidget time
        m_FidgetTime += DeltaTime ;

        // Pick a new fidget anim?
        if (m_FidgetTime > 5)
        {
            // Reset timer
            m_FidgetTime -= x_frand(8.0f, 5.0f) ;

            // Play random fidget
            m_Base.m_Loco.PlayAnim((loco::anims)x_irand(loco::ANIM_FIDGET1, loco::ANIM_FIDGET2), 0.5f) ;
        }
    }
    */
}

//=========================================================================

void stage5_idle_state::OnThink( void )
{
    // Attack if we can see a target
    if (m_Base.UpdateTarget())
        m_Base.SetupState(STATE_ATTACK) ;
}

//=========================================================================
// STAGE5 PATROL STATE
//=========================================================================

stage5_patrol_state::stage5_patrol_state( stage5& Stage5, states State ) :
    stage5_state(Stage5, State)
{
}

//=========================================================================

void stage5_patrol_state::OnInit( void )
{
    m_FidgetTime = 0 ;
}

//=========================================================================

void stage5_patrol_state::OnEnter( void )
{
    // Get loco to move incase we are playing a fidget etc
    if (m_Base.m_Loco.GetState() != loco::STATE_MOVE)
        m_Base.m_Loco.SetState(loco::STATE_MOVE) ;

    // Walk
    m_Base.m_Loco.SetMoveStyle(loco::MOVE_STYLE_PROWL) ;
}

//=========================================================================

void stage5_patrol_state::OnAdvance( f32 DeltaTime )
{
    (void)DeltaTime ;

    /*
    // Wait for anim to finish
    if (m_Base.m_Loco.GetState() != loco::STATE_PLAY_ANIM)
    {
        // Update fidget time
        m_FidgetTime += DeltaTime ;

        // Pick a new fidget anim?
        if (m_FidgetTime > 5)
        {
            // Reset timer
            m_FidgetTime -= x_frand(8.0f, 5.0f) ;

            // Play random fidget
            m_Base.m_Loco.PlayAnim((loco::anims)x_irand(loco::ANIM_FIDGET1, loco::ANIM_FIDGET2), 0.5f) ;
        }
    }
    */
}

//=========================================================================

void stage5_patrol_state::OnThink( void )
{
    // Override normal with trigger info?
    if (m_Info.bValid)
    {
        // Allow target to be seen for a while...
        m_Base.m_TargetNotSeenTimer     = 0 ;
        m_Base.m_TargetNoSightConeTimer = 3 ;
        m_Base.UpdateTarget() ;

        // If path is complete, goto next state
        xbool bComplete = FALSE ;
        if (m_Base.m_TargetObjectGuid)
            bComplete = m_Base.PathFindMoveAndLook(m_Info.TargetPos, m_Base.GetTargetObjectPos()) ;
        else
            bComplete = m_Base.PathFindMoveAndLook(m_Info.TargetPos) ;
        
        if (bComplete)
        {
            m_Base.SetupState(m_Info.NextState) ;
            m_Base.m_pActiveState->OnThink() ;
        }

        return ;
    }

    // Call convenient function to update movement
    m_Base.Patrol(150) ;

    // Attack if we can see a target
    if (m_Base.UpdateTarget())
        m_Base.SetupState(STATE_ATTACK) ;
}

//=========================================================================

void stage5_patrol_state::OnRender( void )
{
    // Draw current nav node
    if (m_Base.GetCurrentNodeID() != SLOT_NULL)
    {
        // Lookup nav node info
        ng_node& NavNode = g_NavMap.GetNodeByIndex(m_Base.GetCurrentNodeID()) ;
        vector3& NavPos  = NavNode.GetPosition() ;
        
        // Draw
        draw_ClearL2W();
        draw_Marker( NavPos, xcolor(255,0,0,255)) ;
    }
}

//=========================================================================
// STAGE5 ATTACK STATE
//=========================================================================

stage5_attack_state::stage5_attack_state( stage5& Stage5, states State ) :
    stage5_state(Stage5, State)
{
}

//=========================================================================

void stage5_attack_state::OnEnter( void )
{
    // Get loco to move incase we are playing a fidget etc
    if (m_Base.m_Loco.GetState() != loco::STATE_MOVE)
        m_Base.m_Loco.SetState(loco::STATE_MOVE) ;

    // Leggit!
    m_Base.m_Loco.SetMoveStyle(loco::MOVE_STYLE_RUN) ;

    // Reset timers
    m_OffsetTimer = x_frand(0,2) ;
    m_ShootTimer  = x_frand(0,2) ;
    m_MeleeTimer  = x_frand(0,2) ;
    m_HowlTimer   = x_frand(0,2) ;
    m_Offset      = vector3(0,0,800) ;
    m_BulletTimer = 0 ;
    m_BulletSide  = x_irand(0,1) ;
    m_BulletCount = 4 ;

    // Allow target to be seen for a while...
    m_Base.m_TargetNotSeenTimer     = 0 ;
    m_Base.m_TargetNoSightConeTimer = 3 ;
}

//=========================================================================

void stage5_attack_state::OnAdvance( f32 DeltaTime )
{
    // Update attack timers when moving
    if (        (m_Base.m_Loco.GetState() == loco::STATE_IDLE)
            ||  (m_Base.m_Loco.GetState() == loco::STATE_MOVE) )
    {
        m_ShootTimer  += DeltaTime ;
        m_MeleeTimer  += DeltaTime ;
        m_HowlTimer   += DeltaTime ;
    }

    // Pick a new offset?
    m_OffsetTimer += DeltaTime ;
    if (m_OffsetTimer > 3)
    {
        // Update offset time
        m_OffsetTimer -= x_frand(3,5) ;

        // Calculate offset infront of target
        vector3 Front = m_Base.GetPosition() - m_Base.GetTargetObjectPos() ;
        Front.NormalizeAndScale(x_frand(800,1000)) ;
        
        // Calculate offset to side of target
        //radian Yaw = Front.GetYaw() ;
        //vector3 Side = vector3(x_frand(-400,400), 0,0) ;
        //Side.RotateY(Yaw) ;
        //m_Offset = Front + Side ;
    }

    // Shooting?
    if (m_Base.m_Loco.IsReadyToShoot())
    {
        // Update timer
        m_BulletTimer += DeltaTime ;

        // Time to fire?
        if (m_BulletTimer > 0.5f)
        {
            // Reset timer and toggle side
            m_BulletTimer -= 0.5f ;
            m_BulletSide ^= 1 ;

            // Out of bullets?
            if (m_BulletCount-- <= 0)
            {
                // Stop shooting
                m_Base.m_Loco.SetShooting(FALSE) ;

                // End the shoot attack
                m_Base.m_Loco.ExitShootState() ;
            }
            else
            {
                // Shoot!
                m_Base.m_Loco.SetShooting(TRUE) ;

                // Create bullet
		        guid BulletGuid = g_ObjMgr.CreateObject( bullet_projectile::GetObjectType() ) ;
		        bullet_projectile* pBullet = ( bullet_projectile* ) g_ObjMgr.GetObjectByGuid( BulletGuid ) ;
                ASSERT(pBullet) ;

                // Setup start position
                vector3 InitPos ;
                if (m_BulletSide)
                    InitPos = m_Base.m_Loco.GetLeftSpikePos() ;
                else
                    InitPos = m_Base.m_Loco.GetRightSpikePos() ;

                // Calculate pitch and yaw
                vector3 Delta       = m_Base.GetTargetObjectPos() - m_Base.GetPosition() ;
                
                radian  Pitch       = 0 ;
                radian  TargetPitch = Delta.GetPitch() ;
                radian  DeltaPitch  = x_MinAngleDiff(TargetPitch, Pitch) ;
                
                radian  Yaw         = m_Base.m_Loco.m_Player.GetFacingYaw() ;
                radian  TargetYaw   = Delta.GetYaw() ;
                radian  DeltaYaw    = x_MinAngleDiff(TargetYaw, Yaw) ;

                // Limit yaw
                if (DeltaYaw > R_25)
                    DeltaYaw = R_25 ;
                else
                if (DeltaYaw < -R_25)
                    DeltaYaw = -R_25 ;

                // Limit pitch
                if (DeltaPitch > R_15)
                    DeltaPitch = R_15 ;
                else
                if (DeltaPitch < -R_15)
                    DeltaPitch = -R_15 ;
                
                // Setup init rot
                radian3 InitRot ;
                InitRot.Pitch = x_ModAngle2(Pitch + DeltaPitch) ;
                InitRot.Yaw   = x_ModAngle2(Yaw   + DeltaYaw) ;
                InitRot.Roll  = 0 ;

                // Fire!
                pBullet->Initialize(InitPos,            // InitPos
                                    InitRot,            // InitRot
                                    vector3(0,0,0),     // InitVel
                                    10.0f,              // DamageAmount
                                    15.0f,              // ForceAmount
                                    10000.0f,           // Speed
                                    m_Base.GetGuid()) ; // OwnerGuid
            }
        }
    }
}

//=========================================================================

void stage5_attack_state::OnThink( void )
{
    // Override normal with trigger info?
    if (m_Info.bValid)
    {
        // Allow target to be seen for a while...
        m_Base.m_TargetNotSeenTimer     = 0 ;
        m_Base.m_TargetNoSightConeTimer = 3 ;
        m_Base.UpdateTarget() ;

        // If path is complete, goto next state
        xbool bComplete = FALSE ;
        if (m_Base.m_TargetObjectGuid)
            bComplete = m_Base.PathFindMoveAndLook(m_Info.TargetPos, m_Base.GetTargetObjectPos()) ;
        else
            bComplete = m_Base.PathFindMoveAndLook(m_Info.TargetPos) ;
        
        if (bComplete)
        {
            m_Base.SetupState(m_Info.NextState) ;
            m_Base.m_pActiveState->OnThink() ;
        }

        return ;
    }

    // Update target
    xbool bCanSeeTarget = m_Base.UpdateTarget() ;

    // No target anymore?
    if (!m_Base.m_TargetObjectGuid) 
    {
        // Go back to patrol
        m_Base.SetupState(STATE_PATROL) ;
        return ;
    }

    // Not seen for a while?
    if ((!bCanSeeTarget) && (m_Base.m_TargetNotSeenTimer > 5))
    {
        // Go back to patrol
        m_Base.SetupState(STATE_PATROL) ;
        return ;
    }

    // Choose a new offset it we can't get to the current one
    if (!m_Base.CanMoveToPos(m_Base.GetTargetObjectPos() + m_Offset))
        m_OffsetTimer = 100 ;

    // Go for target
    ASSERT(m_Base.m_TargetObjectGuid) ;
    m_Base.PathFindMoveAndLook(m_Base.m_TargetObjectGuid, m_Offset) ;

    // Possible attacking position?
    if (        (m_Base.m_TargetObjectGuid) &&
                (m_Base.m_Loco.GetState() != loco::STATE_PLAY_ANIM) &&
                (m_Base.m_Loco.GetState() != loco::STATE_SHOOT)     )
    {
        // Make sure sight cone is used!
        m_Base.m_TargetNoSightConeTimer = 0 ;

        // Get delta to object
        vector3 TargetPos = m_Base.GetTargetObjectPos() ;
        vector3 Delta     = TargetPos - m_Base.GetPosition() ;

        // Perform howl?
        if (        (m_Base.IsTargetInSightCone(TargetPos, m_Base.m_SightRadius, m_Base.m_SightFOV*0.9f))
                &&  (m_HowlTimer > 6) )
        {
            // Reset
            m_HowlTimer -= x_frand(6,8) ;

            // Howl!
            m_Base.m_Loco.PlayAnim(loco::ANIM_ATTACK_HOWL) ;
        }
        else
        // Perform melee?
        if (        (m_MeleeTimer > 3)
                &&  (Delta.LengthSquared() < (400.0f * 400.0f))
                &&  (m_Base.IsTargetInSightCone(TargetPos, m_Base.m_SightRadius, m_Base.m_SightFOV*0.25f)) )
        {
            // Reset
            m_MeleeTimer -= x_frand(3,4) ;

            // Melee!
            m_Base.m_Loco.PlayAnim(loco::ANIM_ATTACK_CLAW) ;
        }
        else
        // Shoot?
        if (        (m_Base.IsTargetInSightCone(TargetPos, m_Base.m_SightRadius, m_Base.m_SightFOV*0.4f))
                &&  (m_ShootTimer > 4) )
        {
            // Reset
            m_ShootTimer -= x_frand(4,6) ;

            // Go into shoot mode
            m_BulletCount = 4 ;
            m_BulletTimer = 0 ;
            m_Base.m_Loco.SetState(loco::STATE_SHOOT) ;
        }
    }
}

//=========================================================================

void stage5_attack_state::OnRender( void )
{
}


//=========================================================================
// STAGE5 FLEE STATE
//=========================================================================

stage5_flee_state::stage5_flee_state( stage5& Stage5, states State ) :
    stage5_state(Stage5, State)
{
}

//=========================================================================

void stage5_flee_state::OnEnter( void )
{
    // Get loco to move incase we are playing a fidget etc
    if (m_Base.m_Loco.GetState() != loco::STATE_MOVE)
        m_Base.m_Loco.SetState(loco::STATE_MOVE) ;

    // Leggit!
    m_Base.m_Loco.SetMoveStyle(loco::MOVE_STYLE_RUN) ;
}

//=========================================================================

void stage5_flee_state::OnAdvance( f32 DeltaTime )
{
    (void)DeltaTime ;
}

//=========================================================================

void stage5_flee_state::OnThink( void )
{
    // Override normal with trigger info?
    if (m_Info.bValid)
    {
        // Allow target to be seen for a while...
        m_Base.m_TargetNotSeenTimer     = 0 ;
        m_Base.m_TargetNoSightConeTimer = 3 ;
        m_Base.UpdateTarget() ;

        // If path is complete, goto next state
        xbool bComplete = FALSE ;
        if (m_Base.m_TargetObjectGuid)
            bComplete = m_Base.PathFindMoveAndLook(m_Info.TargetPos, m_Base.GetTargetObjectPos()) ;
        else
            bComplete = m_Base.PathFindMoveAndLook(m_Info.TargetPos) ;
        
        if (bComplete)
        {
            m_Base.SetupState(m_Info.NextState) ;
            m_Base.m_pActiveState->OnThink() ;
        }

        return ;
    }
}

//=========================================================================
// STAGE5 DEATH STATE
//=========================================================================

stage5_death_state::stage5_death_state( stage5& Stage5, states State ) :
    stage5_state(Stage5, State), 
        m_DeathCountdown(10.0f) //countdown time is default to 10 secs..
{
}

//=========================================================================

void stage5_death_state::OnEnter( void )
{
    // Trigger death anim?
    if ((m_Info.bValid) && (m_Info.Anim))
        m_Base.m_Loco.PlayDeathAnim(m_Info.Anim) ;
    else
        m_Base.m_Loco.PlayDeathAnim(loco::ANIM_DEATH) ;

    m_Base.SetAttrBits( m_Base.GetAttrBits() & ~object::ATTR_COLLIDABLE );

    //g_AudioManager.Play( "Stage5_DeathMoan", m_Base.GetPosition() ) ;
}

//=========================================================================

void stage5_death_state::OnAdvance( f32 DeltaTime ) 
{
    m_DeathCountdown -= DeltaTime;

    // Have to let the m_base kill himself since we are not in the object manager update loop
    if (m_DeathCountdown < 0.0f)
        m_Base.KillMe();
}

//=========================================================================
// STAGE5 CHARACTER
//=========================================================================

stage5::stage5() :
    character(),
    m_Idle   ( *this, STATE_IDLE    ),
    m_Patrol ( *this, STATE_PATROL  ),
    m_Attack ( *this, STATE_ATTACK  ),
    m_Flee   ( *this, STATE_FLEE    ),
    m_Death  ( *this, STATE_DEATH   )
{
    // Setup pointer to loco for base class to use
    m_pLoco             = &m_Loco ;
}

//=========================================================================

stage5::~stage5()
{
}

//=============================================================================

void stage5::OnInit( void )
{
    // Call base class
    character::OnInit() ;
}

//=============================================================================

void stage5::OnKill( void )
{
    // Call base class
    character::OnKill() ;
}

//=============================================================================

void stage5::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "stage5::OnAdvanceLogic" ) ;

    // Call base class which will call state advance
    character::OnAdvanceLogic(DeltaTime) ;
}

//=============================================================================

void stage5::OnRender( void )
{
    CONTEXT( "stage5::OnRender" ) ;

    // Call base class
    character::OnRender() ;
}

//=============================================================================

void stage5::OnColCheck( void )
{
    // Call base class
    character::OnColCheck();
}

//=========================================================================

xbool stage5::OnEvent( const anim_event& Event, const vector3& WorldPos )
{
    // Call base class
    if (character::OnEvent(Event, WorldPos))
        return TRUE ;

    // Setup pain from event type
    pain Pain ;
    Pain.AnimEvent = ANIM_EVENT_NULL ;
    f32  YDir = 0 ;

    // Old event?
    if (x_stricmp(Event.GetType(), "Old Event") == 0)
    {
        Pain.AnimEvent = (enum anim_events)Event.GetInt( anim_event::INT_IDX_OLD_TYPE ) ;

        switch(Event.GetInt( anim_event::INT_IDX_OLD_TYPE ))
        {
            case ANIM_EVENT_PAIN_HOWL:    // Howl attack
                // Setup pain to hurt player the closer they are
                Pain.Type           = pain::TYPE_GENERIC ;
                Pain.Center         = WorldPos ;
                Pain.Origin         = GetGuid() ;
                Pain.RadiusR0       = 0 ;
                Pain.RadiusR1       = 400 ;
                Pain.DamageR0       = 1 ;
                Pain.DamageR1       = 0 ;
                Pain.ForceR0        = 500 ;
                Pain.ForceR1        = 50 ;
                YDir = 10.0f ;
                break ;

            case ANIM_EVENT_PAIN_CLAW_FORWARD:      // Melee
            case ANIM_EVENT_PAIN_CLAW_LEFT_2_RIGHT:
            case ANIM_EVENT_PAIN_CLAW_RIGHT_2_LEFT:
                // Setup pain to give max damage anyware within the event
                Pain.Type      = pain::TYPE_CLAW ;
                Pain.Center    = WorldPos ;
                Pain.Origin    = GetGuid() ;
                Pain.RadiusR0  = Event.GetFloat( anim_event::FLOAT_IDX_RADIUS ) ;
                Pain.RadiusR1  = Event.GetFloat( anim_event::FLOAT_IDX_RADIUS ) ;
                Pain.DamageR0  = 1.0f ;
                Pain.DamageR1  = 1.0f ;
                Pain.ForceR0   = 250 ;
                Pain.ForceR1   = 150 ;
                YDir = 50 ; // Always force up
                break ;
        }
    }

    // Send pain?
    if (Pain.RadiusR1 > 0)
    {
        // Setup bbox to contain pain
        bbox BBox(WorldPos, Pain.RadiusR1) ;
        
        // Collect objects that can be hurt by this attack
        g_ObjMgr.SelectBBox( object::ATTR_ALL , BBox, object::TYPE_PLAYER ) ;
        slot_id ObjectID = g_ObjMgr.StartLoop();
        while( ObjectID != SLOT_NULL )
        {
            // Send pain (never to self)
            if (ObjectID != GetSlotID() )
            {
                // Lookup object
                object* pObject = g_ObjMgr.GetObjectBySlot(ObjectID) ;
                ASSERT(pObject) ;

                // Make pain point towards the object getting hit
                Pain.Direction   = pObject->GetBBox().GetCenter() - GetBBox().GetCenter() ;
                Pain.Direction.Y = YDir ;
                Pain.Direction.Normalize() ;

                Pain.PtOfImpact = pObject->GetBBox().GetCenter();

                // Hurt them
                pObject->OnPain(Pain) ;
            }
            ObjectID = g_ObjMgr.GetNextResult( ObjectID ) ;
        }
        g_ObjMgr.EndLoop();

        // Event handled
        return TRUE ;
    }

    // Event not handled
    return FALSE ;
}

//=========================================================================

void stage5::OnPain( const pain& Pain )
{
    if (m_pActiveState && m_pActiveState->IgnorePain(Pain))
		return;
	
    // Only take pain from the player for now...
    slot_id PlayerID = g_ObjMgr.GetFirst(object::TYPE_PLAYER) ;
    if (PlayerID != SLOT_NULL)
    {
        // Lookup object
        object* pObject = (object*)g_ObjMgr.GetObjectBySlot(PlayerID) ;
        if (!pObject)
            return ;

        // If not hit by the player, we are done
        if (pObject->GetGuid() != Pain.Origin)
            return ;
    }

    // Call base class
    character::OnPain(Pain) ;

    // Already dead?
    if (m_Health == 0)
        return ;

    // Pain hit cylinder?
    f32 Force ;
    f32 Damage ;
    if (!GetPainForceAndDamage(Pain, Force, Damage))
        return ;

    // Death?
    m_Health = MAX(0, m_Health - Damage) ;
    if (m_Health <= 0)
    {
        // Create the Blood Particle Effect only on Damage result..
        particle_emitter::CreateOnPainEffect( Pain, particle_emitter::BLOODY_MESS , particle_emitter::BLOODY_MESS );

        SetupState(STATE_DEATH) ;
        return ;
    }

    // Create the Blood Particle Effect only on Damage result..
    particle_emitter::CreateOnPainEffect( Pain, particle_emitter::BLOODY_BURST, particle_emitter::BLOODY_SPIKE_HIT );

    // Make stage5 be aware of who fired at him if it's the player
    m_TargetNotSeenTimer = 0 ;
    m_TargetObjectGuid   = Pain.Origin ;

    // Don't perform the sight cone check for a while
    m_TargetNoSightConeTimer = 3 ;

    // Play audio
//    g_AudioManager.Play( "Stage5_ImpactGrunt", GetPosition() );

    // Pain coming from a claw swipe?
    switch(Pain.AnimEvent)
    {
        case ANIM_EVENT_PAIN_CLAW_FORWARD:
        case ANIM_EVENT_PAIN_CLAW_LEFT_2_RIGHT:
        case ANIM_EVENT_PAIN_CLAW_RIGHT_2_LEFT:
        {
            // Slash!
            g_AudioManager.Play( "ClawSliceFlesh", Pain.Center ) ;

            // Play damage animation
            radian FaceYaw  = m_Loco.m_Player.GetFacingYaw() ;
            radian PainYaw  = Pain.Direction.GetYaw() ;
            radian DeltaYaw = x_MinAngleDiff(FaceYaw, PainYaw) ;
            if ((DeltaYaw >= R_45) && (DeltaYaw <= R_135))
                m_Loco.PlayAnim(loco::ANIM_DAMAGE_STEP_LEFT) ;    
            else
            if ((DeltaYaw <= -R_45) && (DeltaYaw >= -R_135))
                m_Loco.PlayAnim(loco::ANIM_DAMAGE_STEP_RIGHT) ;
            else
            if ((DeltaYaw >= -R_45) && (DeltaYaw <= R_45))
                m_Loco.PlayAnim(loco::ANIM_DAMAGE_STEP_BACK) ;
            else
                m_Loco.PlayAnim(loco::ANIM_DAMAGE_STEP_FORWARD) ;
        }
        break ;
    }
}


//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

void stage5::OnEnumProp ( prop_enum&    List )
{
    // Call base class
    character::OnEnumProp(List) ;

    // Header
    List.AddHeader(     "Stage5",  
                        "Stage5 NPC" 
                        ) ;
}

//=============================================================================

xbool stage5::OnProperty ( prop_query& I )
{
    // Call base class
    if (character::OnProperty(I))
        return TRUE ;

    // Not found
    return FALSE ;
}

//=============================================================================

