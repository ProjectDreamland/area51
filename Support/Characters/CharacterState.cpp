// character_state : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "CharacterState.hpp"
#include "..\..\MiscUtils\SimpleUtils.hpp"
#include "God.hpp"
#include "Character.hpp"


const f32 k_MinTimeReturntoNav = 0.5f; 

//=========================================================================
// CHARACTER_STATE
//=========================================================================

character_state::character_state( character& Character, states State ) :
    m_MoveStyle( loco::MOVE_STYLE_NULL ),
    m_CharacterBase( Character )
{
    // Keep state
    m_State = State ;

    // Clear next pointer
    m_pNext = NULL ;

    // Add to empty list?
    if (Character.m_pStateList == NULL)
    {
        // Make head of list
        Character.m_pStateList = this ;
    }
    else
    {
        // Find tail of list
        character_state* pTail = Character.m_pStateList ;
        ASSERT(pTail) ;
        while(pTail->m_pNext)
            pTail = pTail->m_pNext ;

        // Add to tail
        pTail->m_pNext = this ;
    }

    m_EvadeDelay = 0.0f;
}

//=================================================================================================

void character_state::OnEnter()
{
    m_AllowStateSwitching = TRUE;
    m_TookPainThisTick  = FALSE;
    m_ShotAtThisTick    = FALSE;
    m_HitFriendlyThisTick = FALSE;
    m_HitByFriendlyThisTick = FALSE;

    m_CurrentPhase      = PHASE_NONE;
    m_LastPhase         = PHASE_NONE;
    m_TimeInState       = 0.0f;
    m_TimeInPhase       = 0.0f;
    m_WantsToEvade      = FALSE;

    m_EvadeDelay        = 0.0f;
    m_CharacterBase.GetLocoPointer()->SetAimerBlendSpeed( GetAimerBlendScale() );
}

//=================================================================================================

xbool character_state::OnPain( const pain& Pain )
{
    if( IgnorePain(Pain) )
    {    
        return FALSE;
    }
    else
    {
        m_TookPainThisTick = TRUE;
        return TRUE;
    }
}

//=================================================================================================

void character_state::OnBeingShotAt( object::type ProjectileType, guid ShooterID )
{
    (void)ProjectileType ;

    // if enemy set up as our target if closer than current target.
    factions sourceFaction = m_CharacterBase.GetFactionForGuid(ShooterID);
    if( sourceFaction != FACTION_NOT_SET && !m_CharacterBase.IsFriendlyFaction(sourceFaction) )
    {
/*        if ( m_CharacterBase.IsValidTarget(ShooterID) && (m_CharacterBase.GetTargetGuid() == NULL || m_CharacterBase.IsNewTargetCloser(ShooterID)) && !IgnoreAttacks() )
        {
            m_CharacterBase.SetTargetGuid(ShooterID);
        }*/
    }
    m_ShotAtThisTick = TRUE;
}

//=========================================================================

void character_state::OnHitByFriendly( guid ShooterID )
{
    (void)ShooterID;
    m_HitByFriendlyThisTick = TRUE;
}

//=========================================================================

void character_state::OnHitFriendly( guid FriendlyID )
{
    (void)FriendlyID;
    m_HitFriendlyThisTick = TRUE;
}

//=================================================================================================

xbool character_state::OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags )
{
    // Lookup loco
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    if( !pLoco )
        return FALSE;
    
    // Try play anim
    return pLoco->PlayAnim( AnimType, BlendTime, Flags );
}

//=================================================================================================

void character_state::OnThink( )
{
}

//=================================================================================================

void character_state::OnAdvance( f32 DeltaTime )
{
    m_TimeInState += DeltaTime;
    m_TimeInPhase += DeltaTime;

    // update our current phase, returns the next phase if desired
    s32 newPhase = UpdatePhase(DeltaTime);    
    if( newPhase != 0 )
    {
        
        loco::move_style usedMoveStyle = m_MoveStyle;
        // reset us to the default movement style at the beginning of each phase
        // this allows a phase to change the move style for it's particular goal
        if( usedMoveStyle != loco::MOVE_STYLE_NULL )
        {
            if( m_CharacterBase.GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_CROUCHAIM ||
                m_CharacterBase.GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_RUNAIM ||
                m_CharacterBase.GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_PROWL )
            {
                if( usedMoveStyle == loco::MOVE_STYLE_RUN )
                {   
                    usedMoveStyle = loco::MOVE_STYLE_RUNAIM;
                }
                else if( usedMoveStyle == loco::MOVE_STYLE_CROUCH )
                {   
                    usedMoveStyle = loco::MOVE_STYLE_CROUCHAIM;
                }
            }

            if( m_CharacterBase.GetLocoPointer()->GetMoveStyle() != usedMoveStyle )
            {            
                m_CharacterBase.GetLocoPointer()->SetMoveStyle(usedMoveStyle);
            }
        }
        // turn off aiming and wanting to fire by default, must turn it on in the phases that want to use it
        m_CharacterBase.SetWantsToAim( FALSE );
        m_CharacterBase.SetWantsToFirePrimary( FALSE );
        m_CharacterBase.SetWantsToFireSecondary( FALSE );
        m_CharacterBase.SetOverrideLookatMode( character::LOOKAT_NONE );
        m_CharacterBase.SetCrazyFire(FALSE);
        // Reset delta scale
        m_CharacterBase.GetLocoPointer()->SetDeltaPosScale(vector3(1,1,1)) ;

        if( m_CharacterBase.GetLogStateChanges() )
        {
            if( !m_CharacterBase.GetGoalCompleted() )
            {
                LOG_MESSAGE( "Character::PhaseChange","OldPhase %s: Time In Phase %f: New Phase %s: Completed = FALSE", GetPhaseName(), m_TimeInPhase,GetPhaseName(newPhase) );
            }
            else if( m_CharacterBase.GetGoalSucceeded() )
            {
                LOG_MESSAGE( "Character::PhaseChange","OldPhase %s: Time In Phase %f: New Phase %s: Completed = TRUE: Succeded = TRUE", GetPhaseName(), m_TimeInPhase,GetPhaseName(newPhase) );
            }
            else 
            {
                LOG_MESSAGE( "Character::PhaseChange","OldPhase %s: Time In Phase %f: New Phase %s: Completed = TRUE: Succeded = FALSE: Reason = %s", GetPhaseName(), m_TimeInPhase,GetPhaseName(newPhase),m_CharacterBase.GetGoalFailedReason() );
            }
        }
        ChangePhase( newPhase );        
    }
}

//=================================================================================================

s32 character_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;    
    m_AllowStateSwitching = TRUE;
    // if we are out of our connections, we want to go to them them!!!!
    switch( m_CurrentPhase )
    {
    case PHASE_GOTO_NEAREST_CONNECTION:
        m_CharacterBase.CheckShooting();
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.GetGoalSucceeded() )
            {            
                m_CurrentPhase = PHASE_NONE;
                m_LastPhase = PHASE_GOTO_NEAREST_CONNECTION;
                m_CharacterBase.RecalcPath();
            }
            else
            {
                return PHASE_GOTO_NEAREST_CONNECTION;
            }
        }
        else if( m_CharacterBase.GetInNavMap() )
        {
            m_CurrentPhase = PHASE_NONE;
            m_LastPhase = PHASE_GOTO_NEAREST_CONNECTION;
            m_CharacterBase.RecalcPath();
        }
        break;
    case PHASE_DODGE_GRENADE_LEFT:
        m_AllowStateSwitching = FALSE;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_CurrentPhase = PHASE_NONE;
            m_LastPhase = PHASE_DODGE_GRENADE_LEFT;
        }            
        break;
    case PHASE_DODGE_GRENADE_RIGHT:
        m_AllowStateSwitching = FALSE;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_CurrentPhase = PHASE_NONE;
            m_LastPhase = PHASE_DODGE_GRENADE_RIGHT;
        }            
        break;
    case PHASE_DODGE_GRENADE_RUN_AWAY:
        {
            object *grenadeObject = g_ObjMgr.GetObjectByGuid(m_EvadeFromGuid);
            if( !grenadeObject || m_CharacterBase.GetGoalCompleted() )
            {
                m_CurrentPhase = PHASE_NONE;
                m_LastPhase = PHASE_DODGE_GRENADE_RUN_AWAY;
            }
        }
        break;
    case PHASE_GOTO_LEASH:
        {
/*            nav_connection_slot_id tempConnection = NULL_NAV_SLOT;
            if( !g_NavMap.GetConnectionContainingPoint(tempConnection, m_CharacterBase.GetPosition()) )
            {
                character::goal_info goalInfo = m_CharacterBase.GetCurrentGoalInfo();
                if( g_NavMap.GetNearestConnection(m_CharacterBase.GetPosition()) != NULL_NAV_SLOT &&
                    goalInfo.m_GoalType != character::GOAL_PLAY_ANIMATION &&
                    goalInfo.m_GoalType != character::GOAL_PLAY_ANIMATION_SCALED_TO_TARGET &&
                    goalInfo.m_GoalType != character::GOAL_SAY_DIALOG )
                {
                    return PHASE_GOTO_NEAREST_CONNECTION;
                }
            }        
            else*/
            if( m_CharacterBase.GetGoalCompleted() )
            {
                if( m_CharacterBase.GetGoalSucceeded() )
                {            
                    m_CurrentPhase = PHASE_NONE;
                    m_LastPhase = PHASE_GOTO_LEASH;
                }
                else if (m_CharacterBase.GetGoalFailedReason() != character::FAILED_GOAL_OUTSIDE_LEASH)
                {
                    return PHASE_GOTO_LEASH;
                }
            }
        }
        break;
    case PHASE_PROJECTILE_ATTACHED:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_CurrentPhase = PHASE_NONE;
            m_LastPhase = PHASE_PROJECTILE_ATTACHED;
        }
        break;
    case PHASE_MESON_STUN:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_CurrentPhase = PHASE_NONE;
            m_LastPhase = PHASE_MESON_STUN;
        }
        break;
    default:
        if( m_WantsToEvade )
        {
            m_EvadeDelay -= DeltaTime;   
        }

        if( m_CharacterBase.GetStartStunAnim() )
        {
            newPhase = PHASE_MESON_STUN;
        }
        else if( m_CharacterBase.GetProjectileAttached() &&
                 m_CharacterBase.HasAnim(loco::ANIM_PROJECTILE_ATTACHED) &&
                 m_CurrentPhase != PHASE_PROJECTILE_ATTACHED )
        {
            newPhase = PHASE_PROJECTILE_ATTACHED;
        }
        else if( m_WantsToEvade && 
            m_EvadeDelay <= 0.0f )
        {
            m_WantsToEvade = FALSE;
            // we need to see if the grenade is to the left or right of us...
            // Time to run from the grenade.  Do some collision checks to see which way we want to go.
            vector3 vCurPos = m_CharacterBase.GetPosition();
            object *grenadeObject = g_ObjMgr.GetObjectByGuid(m_EvadeFromGuid);
            if( !grenadeObject )
            {
                return PHASE_NONE;
            }

            vector3 vToGrenade = grenadeObject->GetPosition() - vCurPos;
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            radian angleDiff = ABS(x_MinAngleDiff(vToGrenade.GetYaw(),ourYaw));


            // if the grenade is already far away, then ignore.
            if( vToGrenade.LengthSquared() >= 800.0f * 800.0f )
            {
                return PHASE_NONE;
            }

            if( angleDiff < R_45 || angleDiff > R_135 )
            {
                return PHASE_DODGE_GRENADE_RUN_AWAY;
            }

            vector3 forward(0.0f,ourYaw); 
            vector3 left = forward.Cross(vector3(0.0f,-1.0f,0.0f) );
            vector3 right = forward.Cross( vector3(0.0f,1.0f,0.0f) );
            if( right.Dot(vToGrenade) > left.Dot(vToGrenade) )
            {
                // evade left
                if( m_CharacterBase.HasAnim(loco::ANIM_GRENADE_EVADE_LEFT) )
                {                
                    return PHASE_DODGE_GRENADE_LEFT;
                }
                else
                {
                    return PHASE_DODGE_GRENADE_RUN_AWAY;
                }
            }
            else
            {
                // evade right
                if( m_CharacterBase.HasAnim(loco::ANIM_GRENADE_EVADE_RIGHT) )
                {                
                    return PHASE_DODGE_GRENADE_RIGHT;
                }
                else
                {
                    return PHASE_DODGE_GRENADE_RUN_AWAY;
                }
            }
        }
        if( m_CharacterBase.GetTimeOutofNavMap() >= k_MinTimeReturntoNav )
        {
            character::goal_info goalInfo = m_CharacterBase.GetCurrentGoalInfo();
            if( g_NavMap.GetNearestConnection(m_CharacterBase.GetPosition()) != NULL_NAV_SLOT &&
                goalInfo.m_GoalType != character::GOAL_PLAY_ANIMATION &&
                goalInfo.m_GoalType != character::GOAL_PLAY_ANIMATION_SCALED_TO_TARGET &&
                goalInfo.m_GoalType != character::GOAL_SAY_DIALOG )
            {
                return PHASE_GOTO_NEAREST_CONNECTION;
            }
        }
        if ((m_CharacterBase.GetLeashGuid()!= 0) 
            && (m_CharacterBase.GetLeashDistance()> 0)
            && (m_CharacterBase.GetGoalCompleted()))
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetLeashGuid() );
            if (pObj)
            {
                f32 Dist = (m_CharacterBase.GetPosition() - pObj->GetPosition()).Length();
                if (Dist > m_CharacterBase.GetLeashDistance() )
                {
                    return PHASE_GOTO_LEASH;
                }
            }
        }
        break;        
    }
    return newPhase;
}

//=================================================================================================

void character_state::ChangePhase( s32 newPhase )
{
    nav_connection_slot_id closestConnectionID;
    ng_connection2 closestConnection;
    vector3 closestLocation;
    vector3 toLocation;

    switch( newPhase ) 
    {
    case PHASE_GOTO_NEAREST_CONNECTION:
        closestConnectionID = g_NavMap.GetNearestConnection( m_CharacterBase.GetPosition() );
        closestConnection = g_NavMap.GetConnectionByID(closestConnectionID);
        closestLocation = g_NavMap.GetNearestPointOnConnection( closestConnectionID,m_CharacterBase.GetPosition() );
        toLocation = closestLocation - m_CharacterBase.GetPosition();
        // erase the Y value.
        toLocation.GetY() = 0.0f;

        // we want to try to move 1 meter inside the connection.
        if( closestConnection.GetWidth() < 200.0f )
        {
            toLocation.NormalizeAndScale( closestConnection.GetWidth()/2.0f );
        }
        else
        {        
            toLocation.NormalizeAndScale( 100.0f );
        }

        closestLocation += toLocation;
        m_CharacterBase.SetGotoLocationGoal( closestLocation, loco::MOVE_STYLE_NULL, 0.0f, TRUE );
    	break;
    case PHASE_DODGE_GRENADE_LEFT:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_GRENADE_EVADE_LEFT);
        m_CharacterBase.PlayDialog(character::DIALOG_GRENADE_SPOT);
        break;
    case PHASE_DODGE_GRENADE_RIGHT:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_GRENADE_EVADE_RIGHT);
        m_CharacterBase.PlayDialog(character::DIALOG_GRENADE_SPOT);
        break;
    case PHASE_DODGE_GRENADE_RUN_AWAY:
        m_CharacterBase.SetRetreatFromTargetGoal( m_EvadeFromGuid,vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,1000.0f,FALSE );
        m_CharacterBase.PlayDialog( character::DIALOG_GRENADE_SPOT );
        m_CharacterBase.SetOverrideLookatMode( character::LOOKAT_NAVIGATION );
        break;
    case PHASE_PROJECTILE_ATTACHED:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_PROJECTILE_ATTACHED);
        break;
    case PHASE_GOTO_LEASH:
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetLeashGuid() );
            if (NULL == pObj)
            {
                m_CharacterBase.SetIdleGoal();
            }
            else
            {
                m_CharacterBase.SetGotoLocationGoal( pObj->GetPosition(), loco::MOVE_STYLE_NULL, m_CharacterBase.GetLeashDistance() * 0.8f, FALSE );
            }
        }
        break;
    case PHASE_MESON_STUN:
        m_CharacterBase.SetPlayAnimationGoal( loco::ANIM_MESON_STUN, DEFAULT_BLEND_TIME, loco::ANIM_FLAG_PLAY_TYPE_TIMED, m_CharacterBase.GetStunTimer() );
        m_CharacterBase.StunAnimStarted();
        break;
    }

    // keep track of our last different phase.
    if( m_CurrentPhase != newPhase )
    {    
        m_LastPhase = m_CurrentPhase;
    }
    m_CurrentPhase = newPhase;
    m_TimeInPhase = 0.0f;
}

void character_state::PostUpdate()
{
    m_TookPainThisTick  = FALSE;
    m_ShotAtThisTick    = FALSE;
    m_HitFriendlyThisTick = FALSE;
    m_HitByFriendlyThisTick = FALSE;
}

/*
//=================================================================================================
// CONVERSATION_STATE
//=================================================================================================
conversation_state::conversation_state( character& Character, conversation_states State )
{
    // Keep state
    m_State = State ;

    // Clear next pointer
    m_pNext = NULL ;

    // Add to empty list?
    if (Character.m_pConvStateList == NULL)
    {
        // Make head of list
        Character.m_pConvStateList = this ;
    }
    else
    {
        // Find tail of list
        conversation_state* pTail = Character.m_pConvStateList ;
        ASSERT(pTail) ;
        while(pTail->m_pNext)
            pTail = pTail->m_pNext ;

        // Add to tail
        pTail->m_pNext = this ;
    }
}
*/
//=================================================================================================

#ifndef X_RETAIL
void character_state::OnDebugRender( void )
{
}
#endif // X_RETAIL

//=================================================================================================

const char* character_state::GetStateName( void )
{
#if defined(sbroumley) || defined(jfranklin)
    ASSERTS( 0, "Add your state to the info so it's easier to debug this stuff in the editor!" );
#endif
    
    return "NEED NAME";
}

//=================================================================================================

const char* character_state::GetPhaseName( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_GOTO_NEAREST_CONNECTION:
        return "PHASE_GOTO_NEAREST_CONNECTION";
    	break;
    case PHASE_DODGE_GRENADE_LEFT:
        return "PHASE_DODGE_GRENADE_LEFT";
    	break;
    case PHASE_DODGE_GRENADE_RIGHT:
        return "PHASE_DODGE_GRENADE_RIGHT";
    	break;
    case PHASE_DODGE_GRENADE_RUN_AWAY:
        return "PHASE_DODGE_GRENADE_RUN_AWAY";
        break;
    case PHASE_GOTO_LEASH:
        return "PHASE_GOTO_LEASH";
    case PHASE_MESON_STUN:
        return "PHASE_MESON_STUN";
    case PHASE_PROJECTILE_ATTACHED:
        return "PHASE_PROJECTILE_ATTACHED";
        break;
    }

#if defined(sbroumley) || defined(jfranklin)
    ASSERTS( 0, "Add your phase to the info so it's easier to debug this stuff in the editor!" );
#endif
    
    return "**PHASE NEEDS NAME**";
}

//=================================================================================================

void character_state::OnGrenadeAlert( alert_package& Package )
{
    // there is a grenade near us... 
    // we want to go to grenade evade!
    m_WantsToEvade = TRUE;
    m_EvadeDelay = x_frand( 0.0f,0.4f );
    m_EvadeFromGuid = Package.m_Cause;    
}

//=================================================================================================
// GRAVEYARD
//=================================================================================================
/*
void    character_state::OnPathComplete  ( void )
{
}
//=================================================================================================

xbool character_state::RequestNewPath  ( vector3& Pos, s32 NumNodes )
{
    //reset the path.
    for ( s32 i = 0; i < NUM_PATH_NODES; i++ )
    {
        m_PathList[i] = SLOT_NULL ;
    }

    m_NumNodesRequested = NumNodes ;

    // Make sure that we have the actual destination point
    m_Destination = Pos;

    // Make sure that we have a node
    nav_node_slot_id NearNode = g_NavMap.GetNearestNode( m_CharacterBase.GetPosition() ) ;

    
    god* pGod = SMP_UTIL_Get_God() ;
    ASSERT( pGod );
    m_bValidPath = pGod->RequestPath( NearNode, Pos, m_CharacterBase.GetGuid(), m_PathList, m_NumNodesRequested ) ;

    m_CurrentPathIndex = 0;


    if( m_bValidPath )
    {
        m_CharacterBase.MoveAndLookToNode( m_PathList[ m_CurrentPathIndex ] );
        ASSERT( m_PathList[ 0 ] == m_CharacterBase.GetCurrentNodeID() ) ;
    }
    else
    {
        m_CharacterBase.SetCurrentNodeID( SLOT_NULL ) ;
    }

    return m_bValidPath ;

}

//=================================================================================================

void    character_state::OnAdvancePath( void )
{
//    ASSERT( m_bValidPath ) ;
#ifdef TARGET_PC
    if (!m_bValidPath)
    {
        x_DebugMsg("ERROR: No valid path found for %s (%s)\n",
            m_CharacterBase.GetTypeDesc().GetTypeName(),
            guid_ToString(m_CharacterBase.GetGuid()));
    }
#endif

    if( m_CharacterBase.HasReachedMovePos() )
    {
        nav_node_slot_id NextNode = GetNextPathIndex() ;
        
        if( NextNode != SLOT_NULL )
        {
            m_CharacterBase.MoveAndLookToNode( NextNode );
        }
        else
        {
            m_CharacterBase.SetCurrentNodeID( SLOT_NULL ) ;
        }
    }

    // Reached destination.
    if( m_CharacterBase.GetCurrentNodeID() == SLOT_NULL || m_CurrentPathIndex > m_NumNodesRequested )
    {
        OnPathComplete() ;
    }    
}

//=========================================================================

nav_node_slot_id character_state::GetNextPathIndex( void )
{
    nav_node_slot_id RetSlot = SLOT_NULL;
    
    //if we have reached the end of the current path:
    if ( m_CurrentPathIndex >= m_NumNodesRequested )
    {
        //Calculate another path
        god* pGod = SMP_UTIL_Get_God() ;
        m_bValidPath = pGod->RequestPath( m_CharacterBase.GetCurrentNodeID() , m_Destination , m_CharacterBase.GetGuid() , m_PathList , NUM_PATH_NODES ) ;
        m_CurrentPathIndex = 0;

        //if the new path is valid, pass back the next node index.
        if ( m_bValidPath )
        {
            RetSlot = m_PathList[ m_CurrentPathIndex ];
        }
    }
    else
    //if there is still another valid path index in the list.
    if ( m_PathList[ m_CurrentPathIndex ] >= 0 )
    {
        //set the next node index.
        RetSlot =  m_PathList[ m_CurrentPathIndex ];
        m_CurrentPathIndex++;
    }    

    return RetSlot;
}
*/
