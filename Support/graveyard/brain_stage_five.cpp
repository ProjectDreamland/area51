///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  brain_stage_five.cpp
//
//      - brain specialization for a Stage 5 mutant
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//=================================================================================================
// INCLUDES
//=================================================================================================

#include "brain_stage_five.hpp"

#include "navigation\NavNodeMgr.hpp"

#include "..\objects\object.hpp"
#include "..\objects\npc.hpp"
#include "..\objects\player.hpp"

#include "objects\ParticleEmiter.hpp"

//=================================================================================================
// TWEAKABLE DEFINES
//=================================================================================================

static const f32  ATTACK_DIST    = 200.0f ;
static const f32  ATTACK_FACE    = 20.0f ;
static const f32  PAIN_PARTICLE_DISPLACE_AMT = 20.0f;

//=================================================================================================
// FUNCTIONS
//=================================================================================================

brain_stage_five::brain_stage_five( npc* newOwner) :
    brain(newOwner)
{
}

//=================================================================================================

brain_stage_five::~brain_stage_five()
{
}

//=================================================================================================

void brain_stage_five::Init(void)
{
    // Call base class
    brain::Init();

    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Patrol vars
    m_PatrolNode = g_NavMgr.GetNearestPatrolNode( pOwner->GetPosition() );  // Start node

    // Target vars
    m_TargetObject = SLOT_NULL ;            // Object to attack
    m_TargetPos = pOwner->GetPosition() ;   // Target position to run to
    m_LookAtPos.Zero() ;                    // Position to look at
    m_FacingTarget = FALSE ;                // TRUE if can shoot/hit target
    m_TargetMemoryTime = 0 ;                // How long enemy remembers target for    

    // Alert vars
    m_AlertPos.Zero()  ;                    // Pos to go for when alerted
    m_AlertTime = 0 ;                       // Time to be alerted for

    // Shoot vars
    m_ShootInterval = 5 ;                   // Time before next shooting is allowed
    m_ShootTime     = 0 ;                   // Time before next fire
    m_ShotCount     = 2 ;                   // Total shots to fire
    m_ShotInterval  = 0.2f ;                // Interval between shots
    m_ShotTime      = 0 ;                   // Time before next fire
    m_ShotRemain    = 0 ;                   // Shots remaining
    m_ShotSide      = 0 ;                   // Current side shooting from
  
    // Leash vars
    m_LeashRadius = 100*15 ;                // Limit from starting position
    m_LeashPos = pOwner->GetPosition() ;    // Start position

    // Sight vars
    m_SightRadius = 100*15 ;                // Sight distance
    m_SightFOV    = DEG_TO_RAD(80) ;        // FOV

    // Put into start state
    m_State     = STATE_PATROL ;
    m_StateTime = 0 ;
    m_TimeSinceLastUpdateTarget = 0;


    // Give full health
    pOwner->SetLife(100) ;
}

//=================================================================================================

// Draw XZ circle
void draw_Circle( const vector3& C, f32 R, const vector3& N, xcolor Color = XCOLOR_WHITE )
{
    f32 Sin, Cos ;

    s32    i          = 1 + (s32)(R * DEG_TO_RAD(360.0f) * 0.005f) ;
    radian Angle      = 0 ;
    radian DeltaAngle = DEG_TO_RAD(360) / i ;

    (void)N ;

    draw_Begin(DRAW_LINE_STRIPS) ;
    draw_Color(Color) ;
    while(i--)
    {
        x_sincos(Angle, Sin, Cos) ;

        draw_Vertex( C.X + (R * Sin),
                     C.Y,
                     C.Z + (R * Cos) ) ;

        Angle += DeltaAngle ;
    }

    draw_End() ;
}

//=================================================================================================

// Draw arc of XZ circle
void draw_Arc( const vector3& C, f32 R, radian Dir, radian FOV, xcolor Color = XCOLOR_WHITE )
{
    f32 Sin, Cos ;

    s32    i          = 1 + (s32)(R * FOV * 0.005f) ;
    radian Angle      = Dir - (FOV * 0.5f) ;
    radian DeltaAngle = FOV / i ;

    draw_Begin(DRAW_LINE_STRIPS) ;
    draw_Color(Color) ;
    draw_Vertex( C ) ;
    while(i--)
    {
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( C.X + (R * Sin),
                     C.Y,
                     C.Z + (R * Cos) ) ;

        Angle += DeltaAngle ;
    }
    draw_Vertex( C ) ;
    draw_End() ;
}

//=================================================================================================

// Draws brain suff
void brain_stage_five::OnRender ( void )
{
    CONTEXT( "brain_stage_five::OnRender" );

    // Call base class
    brain::OnRender() ;

    // Only draw on PC
    #ifdef TARGET_PC
        draw_ClearL2W() ;

        // Get loco
        base_loco* pLoco = (base_loco*)m_Owner->GetLocomotion() ;
        if (pLoco)
        {
            // Check events
            for (s32 i = 0; i < pLoco->m_Player.GetNEvents(); i++)
            {
                // Send this event?
                if (pLoco->m_Player.IsEventActive(i))
                {
                    // Lookup event and world position
                    const anim_event& Event  = pLoco->m_Player.GetEvent(i) ;
                    vector3           Pos    = pLoco->m_Player.GetEventPosition(i) ;

                    // Render the event sphere
                    draw_Sphere(Pos, Event.m_Radius, XCOLOR_RED) ;
                }
            }
        }

        // Draw leash
        draw_Circle(m_LeashPos, m_LeashRadius, vector3(0,1,0), XCOLOR_RED) ;

        // Draw sight
        draw_Arc( pLoco->GetPosition(), m_SightRadius, pLoco->GetFacingYaw(), m_SightFOV, XCOLOR_PURPLE ) ;
    #endif    
}

//=================================================================================================

void brain_stage_five::SetState( state State )
{
    // Already in this state?
    if (m_State == State)
        return ;

    // Exit the old state
    switch(m_State)
    {
        case STATE_PATROL:  PatrolExit  () ;    break ;
        case STATE_ALERT:   AlertExit   () ;    break ;
        case STATE_ATTACK:  AttackExit  () ;    break ;
        case STATE_SHOOT:   ShootExit   () ;    break ;
        case STATE_DEATH:   DeathExit   () ;    break ;
        default:
            ASSERT(0) ;
    }

    // Clear state time
    m_StateTime = 0 ;

    // Goto new state
    m_State = State ;

    // Enter the new state
    switch(m_State)
    {
        case STATE_PATROL:  PatrolEnter () ;    break ;
        case STATE_ALERT:   AlertEnter  () ;    break ;
        case STATE_ATTACK:  AttackEnter () ;    break ;
        case STATE_SHOOT:   ShootEnter  () ;    break ;
        case STATE_DEATH:   DeathEnter  () ;    break ;
        default:
            ASSERT(0) ;
    }
}

//=================================================================================================

// Advance the current state
void brain_stage_five::AdvanceState( f32 DeltaTime )
{
    // Update state time
    m_StateTime += DeltaTime ;

    // Update shoot time
    m_ShootTime = MAX(0, m_ShootTime - DeltaTime) ;

    // Which state?
    switch(m_State)
    {
        case STATE_PATROL:  PatrolAdvance   (DeltaTime) ;   break ;
        case STATE_ALERT:   AlertAdvance    (DeltaTime) ;   break ;
        case STATE_ATTACK:  AttackAdvance   (DeltaTime) ;   break ;
        case STATE_SHOOT:   ShootAdvance    (DeltaTime) ;   break ;
        case STATE_DEATH:   DeathAdvance    (DeltaTime) ;   break ;
        default:
            ASSERT(0) ;
    }
}

//=================================================================================================

// Takes care of all anim events
void brain_stage_five::ProcessAnimEvents( void )
{
    // Get loco
    base_loco* pLoco = (base_loco*)m_Owner->GetLocomotion() ;
    if (!pLoco)
        return ;

    // Check events
    for (s32 i = 0; i < pLoco->m_Player.GetNEvents(); i++)
    {
        // Send this event?
        if (pLoco->m_Player.IsEventActive(i))
        {
            // Lookup event and world position
            const anim_event& Event  = pLoco->m_Player.GetEvent(i) ;
            vector3           Pos    = pLoco->m_Player.GetEventPosition(i) ;

            // Setup pain from event type
            pain Pain ;
            Pain.AnimEvent = (enum anim_events)Event.m_Type ;
            f32  YDir = 0 ;
            switch(Event.m_Type)    // TO DO - Update defines and exporter
            {
                case ANIM_EVENT_PAIN_HOWL:    // Howl attack
                    // Setup pain to hurt player the closer they are
                    Pain.Type           = pain::TYPE_GENERIC ;
                    Pain.Center         = Pos ;
                    Pain.Origin         = m_Owner->GetGuid() ;
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
                    Pain.Center    = Pos ;
                    Pain.Origin    = m_Owner->GetGuid() ;
                    Pain.RadiusR0  = Event.m_Radius ;
                    Pain.RadiusR1  = Event.m_Radius ;
                    Pain.DamageR0  = 1.0f ;
                    Pain.DamageR1  = 1.0f ;
                    Pain.ForceR0   = 250 ;
                    Pain.ForceR1   = 150 ;
                    YDir = 50 ; // Always force up
                    break ;
            }

            // Send pain?
            if (Pain.RadiusR1 > 0)
            {
                // Setup bbox to contain pain
                bbox BBox(Pos, Pain.RadiusR1) ;
                
                // Collect objects that can be hurt by this attack
                g_ObjMgr.SelectBBox( object::ATTR_ALL , BBox, object::TYPE_PLAYER ) ;
                slot_id ObjectID = g_ObjMgr.StartLoop();
                while( ObjectID != SLOT_NULL )
                {
                    // Send pain (never to self)
                    if (ObjectID != m_Owner->GetSlotID() )
                    {
                        // Lookup object
                        object* pObject = g_ObjMgr.GetObjectBySlot(ObjectID) ;
                        ASSERT(pObject) ;

                        // Make pain point towards the object getting hit
                        Pain.Direction   = pObject->GetBBox().GetCenter() - m_Owner->GetBBox().GetCenter() ;
                        Pain.Direction.Y = YDir ;
                        Pain.Direction.Normalize() ;

                        // Hurt them
                        pObject->OnPain(Pain) ;
                    }
                    ObjectID = g_ObjMgr.GetNextResult( ObjectID ) ;
                }
                g_ObjMgr.EndLoop();
            }
        }
    }
}

//=================================================================================================

// Looks for targets
void brain_stage_five::UpdateTarget( f32 DeltaTime )
{
    m_TimeSinceLastUpdateTarget += DeltaTime;
    if( m_TimeSinceLastUpdateTarget < 0.75f )
    {
        return;
    }
    m_TimeSinceLastUpdateTarget = 0.0f;
    
    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    if (!pLoco)
        return ;

    // Clear target
    m_TargetObject = SLOT_NULL ;
    m_FacingTarget = FALSE ;

    // Setup detect bbox
    bbox DetectBBox;
    DetectBBox = m_Owner->GetBBox();
    DetectBBox.Inflate( m_SightRadius*4, m_SightRadius*4, m_SightRadius*4 ) ;

    // Search area
    g_ObjMgr.SelectBBox( object::ATTR_ALL , DetectBBox ,object::TYPE_PLAYER );
    slot_id ObjectID = g_ObjMgr.StartLoop();
    while( ObjectID != SLOT_NULL )
    {
        // Skip self
        if (ObjectID != pOwner->GetSlotID() )
        {
            // Lookup object
            object* pObject = g_ObjMgr.GetObjectBySlot(ObjectID) ;
            ASSERT(pObject) ;

            // Get delta and dist to object
            vector3 Delta = pObject->GetPosition() - pLoco->GetPosition() ;
            f32     Dist  = Delta.Length() ;

            // Update target memory if object can be seen - TO DO - A Ray cast
            if (        (Dist <= m_SightRadius)
                    &&  (pLoco->IsInForwardArc(Delta, RAD_TO_DEG(m_SightFOV))) )
                m_TargetMemoryTime = 3.0 ;

            // Can object be seen?
            if (m_TargetMemoryTime > 0)
            {
                // Keep it
                m_TargetObject = ObjectID ;
                break ;
            }
        }
        ObjectID = g_ObjMgr.GetNextResult( ObjectID ) ;
    }
    g_ObjMgr.EndLoop() ;

    // Is there an object to run after?
    if (m_TargetObject != SLOT_NULL)
    {
        //=============================================================================================
        // Attack object
        //=============================================================================================

        // Get object to attack. If none, exit
        object* pObject = g_ObjMgr.GetObjectBySlot(m_TargetObject) ;
        ASSERT(pObject) ;

        // Get dest pos infront of player
        vector3 ObjectPos = pObject->GetPosition() ;
        vector3 Delta     = ObjectPos - pLoco->GetPosition() ;
        Delta.NormalizeAndScale(ATTACK_DIST) ;
        m_TargetPos = ObjectPos - Delta ;

        // Look at object eye
        m_LookAtPos = ObjectPos ;
        m_LookAtPos.Y += 0.75f * pObject->GetBBox().GetSize().Y ;

        // Outside of leash range?
        f32 Dist = (m_TargetPos - m_LeashPos).Length() ;
        if (Dist > m_LeashRadius)
        {
            // Keep target on edge of leash radius
            Delta = ObjectPos - m_LeashPos ;
            Delta.Normalize() ;
            m_TargetPos = m_LeashPos + (m_LeashRadius * Delta) ;
        }
    }

    // Patrol?
    if (m_TargetObject == SLOT_NULL)
    {
        //=============================================================================================
        // Patrol
        //=============================================================================================

        // Keep trying to setup patrol node if one isn't setup yet
        if (m_PatrolNode == SLOT_NULL)
            m_PatrolNode = g_NavMgr.GetNearestPatrolNode( pOwner->GetPosition() );

        // No patrol node?
        if (m_PatrolNode == SLOT_NULL)
        {
            m_TargetPos = pLoco->GetPosition() ;
            m_LookAtPos = m_TargetPos ;
            return ;
        }

        // Keep going until we find a node within the leash radius
        f32 Dist = 0 ;
        node_slot_id StartPatrolNode = m_PatrolNode ;
        do
        {
            // Get dest node
            ASSERT(m_PatrolNode != SLOT_NULL) ;
            base_node* pNode = g_NavMgr.GetNode(m_PatrolNode) ;
            ASSERT(pNode) ;

            // Get dest pos
            m_TargetPos = pNode->GetPosition() ;

            // Calculate dist from leash pos
            Dist = (m_TargetPos - m_LeashPos).Length() ;

            // Arrive?
            if ( (Dist > m_LeashRadius) || (pLoco->IsAtLocation(m_TargetPos)) )
            {
                // Get next node
                m_PatrolNode = g_NavMgr.GetNextPatrolNode(m_PatrolNode, TRUE) ;

                // Get new node
                pNode = g_NavMgr.GetNode(m_PatrolNode) ;
                ASSERT(pNode) ;

                // Get current and dest pos
                m_TargetPos = pNode->GetPosition() ;

                // Calculate dist from leash pos
                Dist = (m_TargetPos - m_LeashPos).Length() ;
            }
        }
        while((Dist > m_LeashRadius) && (m_PatrolNode != StartPatrolNode)) ;
    }

    // Update target memory time
    m_TargetMemoryTime = MAX(0, m_TargetMemoryTime - DeltaTime) ;
}

//=================================================================================================

// Figures out how to get to target
void brain_stage_five::PathFind( f32 DeltaTime )
{

    base_loco* pLoco;
    {
        CONTEXT("Initialize stuff");
        // Get owner
        npc* pOwner = GetOwner() ;
        ASSERT(pOwner) ;

        // Get loco
        pLoco = (base_loco*)pOwner->GetLocomotion() ;
        ASSERT(pLoco) ;

        // Update loco
        pLoco->SetLookAt(m_LookAtPos) ;
        SetNewDestination(m_TargetPos) ;
    }
    {
        CONTEXT("UpdateNavPlan");
        // Keep the nav plan updated
        UpdateNavPlan();
    }
    {
        CONTEXT("UpdateMovement");
        // Move
        m_TimeSinceLastMove = 10 ; // Force update!
        UpdateMovement(DeltaTime) ;
    }
    // Facing the target?
    m_FacingTarget = FALSE ;
    if (m_TargetObject != SLOT_NULL)
    {
        CONTEXT("Cleanup");
        // Lookup object
        object* pObject = g_ObjMgr.GetObjectBySlot(m_TargetObject) ;
        ASSERT(pObject) ;

        // Get delta to object
        vector3 Delta  = pObject->GetPosition() - pLoco->GetPosition() ;
        m_FacingTarget = pLoco->IsInForwardArc(Delta, ATTACK_FACE) ;
    }
}

//=================================================================================================

// Called every game loop
void brain_stage_five::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "brain_stage_five::OnAdvanceLogic" );

    /*
    mutant_stage5_loco* pLoco = (mutant_stage5_loco*)m_Owner->GetLocomotion() ;
    ASSERT(pLoco) ;
    if (pLoco->GetCurrentState() == locomotion::STATE_IDLE)
        pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK) ;
    return ;
    */    
    {
        CONTEXT("UpdateTarget");
        // Update targets
        UpdateTarget(DeltaTime) ;
    }
    {
        CONTEXT("PathFind");
        // Figure out path to target
        PathFind(DeltaTime) ;
    }
    {
        CONTEXT("AdvanceState");
        // Advance state
        AdvanceState(DeltaTime) ;
    }

    {
        CONTEXT("ProcessAnimEvents");
        // Send all animation events
        ProcessAnimEvents() ;
    }
}
//=================================================================================================

// Getting hurt
void brain_stage_five::OnPain( const pain& Pain )
{
    // Already dead?
    if (m_State == STATE_DEATH)
        return ;

    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Lookup info about self
    vector3 Pos       = pLoco->GetPosition();
    f32     ColHeight = pLoco->m_Physics.GetColHeight();
    f32     ColRadius = pLoco->m_Physics.GetColRadius();
    f32     Health    = pOwner->GetHealth();

    // Cheap sphere v axis aligned cylinder collision

    // Below player?
    if ( (Pain.Center.Y + Pain.RadiusR1) < Pos.Y)
        return ;

    // Above player?
    if ( (Pain.Center.Y - Pain.RadiusR1) > (Pos.Y + ColHeight) )
        return ;

    // Get distance to axis aligned cylinder
    f32 Dist = SQR(Pos.X - Pain.Center.X) + SQR(Pos.Z - Pain.Center.Z) ;
    if (Dist > 0.00001f)
    {
        Dist = x_sqrt(Dist) ;
        Dist = MAX(0, Dist - ColRadius) ;
    }

    // Hit?
    if (Dist <= Pain.RadiusR1)
    {
        // Get T value from 0->1, where T=0 at R0, and T=1 at R1
        f32 T = 0 ;
        if (Dist > Pain.RadiusR0)
            T = (Dist - Pain.RadiusR0) / (Pain.RadiusR1 - Pain.RadiusR0) ;

        // Calculate damage and force
        f32 Force  = Pain.ForceR0  + (T * (Pain.ForceR1  - Pain.ForceR0 )) ;
        f32 Damage = Pain.DamageR0 + (T * (Pain.DamageR1 - Pain.DamageR0)) ;

        // Take that pain
        Health = MAX(0, Health - Damage) ;

        // Temp to make him turn around and see what's going on
        m_TargetMemoryTime = 5.0 ;
        
        // Create the particle blood..
        //if ( Health > 0 )
        {
            // Create the Blood Particle Effect only on Damage result..
                particle_emitter::CreateOnPainEffect(
                    Pain, PAIN_PARTICLE_DISPLACE_AMT);
        }

        // Die?
        if( (Health == 0) && (pOwner->IsAlive()) )
        {
            g_AudioManager.Play( "Stage5_DeathMoan", Pos );
            SetState(STATE_DEATH);
        }
        else
        {
            // If coming from a claw swipe - step back
            switch(Pain.AnimEvent)
            {
                case ANIM_EVENT_PAIN_CLAW_FORWARD:
                case ANIM_EVENT_PAIN_CLAW_LEFT_2_RIGHT:
                case ANIM_EVENT_PAIN_CLAW_RIGHT_2_LEFT:

                    g_AudioManager.Play( "ClawSliceFlesh", Pos );
                    pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK);
                    
                break;
            }

            g_AudioManager.Play( "Stage5_ImpactGrunt", Pos );
        }

        pOwner->SetLife((s32)Health);

        // Calculate impulse - TO DO - Use: Force/Mass 
        vector3 Impulse = Pain.Direction * Force ;
        
        // Apply impulse
        pLoco->m_Physics.AddVelocity(Impulse) ;
    }
}

//=================================================================================================
// STATE_PATROL
//=================================================================================================

void brain_stage_five::PatrolEnter( void )
{
    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Walk to nav node
    pLoco->SetMoveStyle(locomotion::STATE_WALK) ;

    // If no patrol node just idle
    if (m_PatrolNode == SLOT_NULL)
        pLoco->SetState( locomotion::STATE_IDLE ) ;
}

//=================================================================================================

void brain_stage_five::PatrolExit( void )
{
}

//=================================================================================================

// Patrol paths
void brain_stage_five::PatrolAdvance( f32 DeltaTime )
{
	(void)DeltaTime;

    // Goto attack?
    if (m_TargetObject != SLOT_NULL)
        SetState(STATE_ATTACK) ;

    // TO DO - Goto alert?
}

//=================================================================================================
// STATE_ALERT
//=================================================================================================

void brain_stage_five::AlertEnter( void )
{
}

//=================================================================================================

void brain_stage_five::AlertExit( void )
{
}

//=================================================================================================

// Patrol paths in an alerted state
void brain_stage_five::AlertAdvance ( f32 DeltaTime )
{
	(void)DeltaTime;
}

//=================================================================================================
// STATE_ATTACK
//=================================================================================================

void brain_stage_five::AttackEnter( void )
{
    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Run when attacking
    pLoco->SetMoveStyle(locomotion::STATE_RUN) ;
}

//=================================================================================================

void brain_stage_five::AttackExit( void )
{
}

//=================================================================================================

// Attack player
void brain_stage_five::AttackAdvance ( f32 DeltaTime )
{
	(void)DeltaTime;

    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // No target?
    if (m_TargetObject == SLOT_NULL)
    {
        SetState(STATE_PATROL) ;
        return ;
    }

    // Run when attacking
    pLoco->SetMoveStyle(locomotion::STATE_RUN) ;

    // Idling?
    if (pLoco->GetCurrentState() == locomotion::STATE_IDLE)
    {
        // At object?
        if (pLoco->IsAtLocation(m_TargetPos))
        {
            // Facing object?
            if (m_FacingTarget)
            {
                // Howl or swipe?
                if (x_irand(0,100) < 30)
                    pLoco->PlayAnimState(base_loco::AT_ATTACK_SPECIAL1) ;   // Howl
                else
                    pLoco->PlayAnimState(base_loco::AT_ATTACK_MELEE1) ;     // Swipe
            }
        }
    }
    else
    {
        // Shoot?
        if ( (m_FacingTarget) && (m_ShootTime == 0) )
            SetState(STATE_SHOOT) ;
    }
}

//=================================================================================================
// STATE_SHOOT
//=================================================================================================

void brain_stage_five::ShootEnter( void )
{
    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    mutant_stage5_loco* pLoco = (mutant_stage5_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Go into shoot pose
    pLoco->EnterAttackMode();

    // Init shot
    m_ShotRemain = m_ShotCount ;
    m_ShotTime   = 0 ;
}

//=================================================================================================

void brain_stage_five::ShootExit( void )
{
    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    mutant_stage5_loco* pLoco = (mutant_stage5_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Go back to idle
    pLoco->ExitAttackMode();

    // Set time before he can shoot again
    m_ShootTime = m_ShootInterval ;
}

//=================================================================================================

// Attack player
void brain_stage_five::ShootAdvance ( f32 DeltaTime )
{
    // Wait for a second before firing so beast can get into pose
    if (m_StateTime < 0.5f)
        return ;

    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    mutant_stage5_loco* pLoco = (mutant_stage5_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Fire?
    m_ShotTime -= DeltaTime ;
    if (m_ShotTime < 0)
    {
        // Set delay for next time
        m_ShotTime += m_ShotInterval ;

        // Anything left to fire?
        if (m_ShotRemain--)
        {
            // Start projectile
            guid ObjectGuid = g_ObjMgr.CreateObject( projectile::GetObjectType() ) ;
            ASSERT(ObjectGuid) ;
            projectile* pProj = (projectile*)g_ObjMgr.GetObjectByGuid(ObjectGuid) ;
            
            // Calculate position
            vector3 Pos ;
            if (m_ShotSide)
                Pos = pLoco->GetLeftSpikePos() ;
            else
                Pos = pLoco->GetRightSpikePos() ;
            
            // Toggle side for next time
            m_ShotSide ^= 1 ;

            // Calculate target yaw
            radian Yaw       = pLoco->GetFacingYaw() ;
            radian TargetYaw = (m_LookAtPos - Pos).GetYaw() ;
            radian DeltaYaw  = x_MinAngleDiff(TargetYaw, Yaw) ;

            // Limit fire direction
            if (DeltaYaw > DEG_TO_RAD(45))
                DeltaYaw = DEG_TO_RAD(45) ;
            else
            if (DeltaYaw < DEG_TO_RAD(-45))
                DeltaYaw = DEG_TO_RAD(-45) ;
                
            // Calculate velocity
            vector3 Vel(0,0,100*10) ;
            Vel.RotateY(Yaw + DeltaYaw) ;
            
            // Init
            pProj->OnMove(Pos) ;
            pProj->SetFiringPoint   (Pos) ;
            pProj->SetFiringVelocity(Vel) ;
            pProj->SetWhoFiredMe    (pOwner->GetGuid()) ;
            pProj->SetWeaponType    (projectile::WEAPON_TYPE_PISTOL) ;
            pProj->Fire() ;

            g_AudioManager.Play( "ClawSpikeLaunch", Pos );
        }
        else
        {
            // All shots fired - back to attack
            SetState(STATE_ATTACK) ;
        }
    }
}


//=================================================================================================
// STATE_DEATH
//=================================================================================================

void brain_stage_five::DeathEnter( void )
{
    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    mutant_stage5_loco* pLoco = (mutant_stage5_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    // Start death animation
    pLoco->PlayAnimState(base_loco::AT_DEATH) ;
}

//=================================================================================================

void brain_stage_five::DeathExit( void )
{
}

//=================================================================================================

void brain_stage_five::DeathAdvance( f32 DeltaTime )
{
    (void)DeltaTime ;

    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Bring back to life? (Someone must have set the health in the editor)
    if (pOwner->GetHealth() > 0)
        SetState(STATE_PATROL) ;
}

//=================================================================================================



///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

// Creates properties
void brain_stage_five::OnEnumProp( prop_enum&  List )
{
    List.AddHeader(     "BrainStage5",  
                        "BrainStage5 represents the AI for a stage5 NPC" 
                        );

    // m_LeashRadius
    List.AddFloat(      "BrainStage5\\Leash Radius",
                        "Maximum distance AI can be from start position" );
    
    // m_SightRadius
    List.AddFloat(      "BrainStage5\\Sight Radius",
                        "Maximum distance AI can see" );

    // m_SightFOV
    List.AddAngle(      "BrainStage5\\Sight FOV",
                        "Maximum field of view AI can see" );

    // m_ShootInterval
    List.AddFloat (     "BrainStage5\\Shoot Interval",
                        "Min time (secs) between shooting sprees" );
    

    // Call base class
    brain::OnEnumProp(List) ;
}

//=================================================================================================

// Reads/writes properties
xbool brain_stage_five::OnProperty( prop_query& I    )
{
    // m_LeashRadius
    if ( I.IsVar( "BrainStage5\\Leash Radius" ) )
    {
        if( I.IsRead() )
            I.SetVarFloat( m_LeashRadius );
        else
            m_LeashRadius = I.GetVarFloat();

        return TRUE;
    }

    // m_SightRadius
    if ( I.IsVar( "BrainStage5\\Sight Radius" ) )
    {
        if( I.IsRead() )
            I.SetVarFloat( m_SightRadius );
        else
            m_SightRadius = I.GetVarFloat();

        return TRUE;
    }

    // m_SightFOV
    if ( I.IsVar( "BrainStage5\\Sight FOV" ) )
    {
        if( I.IsRead() )
            I.SetVarAngle( m_SightFOV );
        else
            m_SightFOV = I.GetVarAngle() ;

        return TRUE;
    }

    // m_ShootInterval
    if ( I.IsVar( "BrainStage5\\Shoot Interval" ) )
    {
        if( I.IsRead() )
            I.SetVarFloat( m_ShootInterval );
        else
            m_ShootInterval = I.GetVarFloat();

        return TRUE;
    }

    // Call base class
    return brain::OnProperty(I) ;
}

//=================================================================================================
