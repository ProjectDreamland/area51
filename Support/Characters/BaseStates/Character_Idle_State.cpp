#include "Character_Idle_State.hpp"
#include "Character_Cover_State.hpp"
#include "..\Character.hpp"
#include "Navigation\ng_node2.hpp"
#include "navigation\coverNode.hpp"
#include "objects\player.hpp"

//=========================================================================

const f32 k_MinTimeBetweenLookats = 12.0f;

//=========================================================================
// IDLE STATE
//=========================================================================

character_idle_state::character_idle_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_MoveStyle = loco::MOVE_STYLE_WALK;
}

//=========================================================================

character_idle_state::~character_idle_state()
{
}

//=========================================================================

void character_idle_state::OnInit( void )
{
    m_AverageTimeTillWander = 5.0f;
    m_WanderLeashRadius = 1200.0f;
    character_state::OnInit();
}

//=========================================================================

void character_idle_state::OnEnter( void )
{
    if( m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_SEARCHING )
    {
        m_MoveStyle = loco::MOVE_STYLE_PROWL;
    }
    else
    {
        m_MoveStyle = loco::MOVE_STYLE_WALK;
    }
    m_TimeTillWander = x_frand(m_AverageTimeTillWander*0.5f,m_AverageTimeTillWander*1.5f);
    character_state::OnEnter();
}

//=========================================================================

s32 character_idle_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;
    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_IDLE_DEFAULT;
        break;
    case PHASE_IDLE_DEFAULT:
        {        
            if( m_AverageTimeTillWander > 0.0f && 
                m_WanderLeashRadius > 0.0f && 
                !m_CharacterBase.GetRootWhenIdle() )
            {        
                if( m_TimeInPhase >= m_TimeTillWander )
                {        
                    newPhase = PHASE_IDLE_WANDER;
                }
            }
            // put this behavior elsewhere.
/*            
            // if the player is nearby and in front of us, lookat him.
            player* pPlayer = SMP_UTIL_GetActivePlayer();
            if ( pPlayer && 
                g_ObjMgr.GetGameDeltaTime( m_LookatTime ) > k_MinTimeBetweenLookats )
            {
                m_LookatTime = g_ObjMgr.GetGameTime() + x_frand( 3.0f, 5.0f );    
                m_CharacterBase.SetHeadLookat( pPlayer->GetGuid(), x_frand( 3.0f, 5.0f ), 400.0f, R_60 );
            }*/
        }
        break;
    case PHASE_IDLE_WANDER:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_IDLE_DEFAULT;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase in Character Idle State" );
        }
    };

    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE &&
        !m_CharacterBase.GetRootWhenIdle() )
    {
        newPhase = basePhase;
    }

    return newPhase;
}

//=========================================================================

void character_idle_state::ChangePhase( s32 newPhase )
{
    f32 offsetLength;
    radian offsetRotation;
    vector3 offsetPosition;
    vector3 gotoPosition;
    nav_connection_slot_id destinationConnectionID;
    ng_connection2 destinationConnection;
    ng_node2 destinationNode;
    switch( newPhase ) 
    {

    case PHASE_IDLE_DEFAULT:
        m_CharacterBase.SetIdleGoal();
    	break;
    case PHASE_IDLE_WANDER:        
        // chose a random location within our wander radius.        
        offsetLength = x_frand(0,m_WanderLeashRadius);
        offsetRotation = x_frand(0,2.0f*PI);
        offsetPosition = vector3(0.0f,0.0f,offsetLength);
        offsetPosition.RotateY(offsetRotation);
        
        // figure out the position we want to go to.
        gotoPosition = m_CharacterBase.GetPosition() + offsetPosition;

        destinationConnectionID = NULL_NAV_SLOT;
        if( !g_NavMap.GetConnectionContainingPoint(destinationConnectionID,gotoPosition) ) 
        {
            // if our position is not in a connection, try to go to the 
            // nearest connection you can find.
            destinationConnectionID = g_NavMap.GetNearestConnection( gotoPosition );
            if( destinationConnectionID != NULL_NAV_SLOT )
            {
                gotoPosition = g_NavMap.GetNearestPointOnConnection( destinationConnectionID, gotoPosition );
            }
        }
        // go ahead and try to goto whatever position we are left with.
        m_CharacterBase.SetGotoLocationGoal( gotoPosition, loco::MOVE_STYLE_NULL, 100.0f );
        break;
    default:        
        if( newPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    }
    character_state::ChangePhase( newPhase );
}

//=========================================================================

character_state::states character_idle_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    // if we have sticky cover, go to it.
    object *followObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetFollowTargetGuid() );
    if( followObject && !m_CharacterBase.GetRootWhenIdle() )
    {
        return STATE_FOLLOW;
    }
    else if( m_CharacterBase.GetCurrentCover() &&
             m_CharacterBase.HasState(STATE_COVER) )
    {
        return STATE_COVER;
    }
    else if( m_CharacterBase.GetAwarenessLevel() != character::AWARENESS_NONE )
    {
        return m_CharacterBase.GetStateFromAwareness();
    }
    return STATE_NULL;
}

//=========================================================================

void character_idle_state::OnThink( void )
{
    character_state::OnThink();
}

//=========================================================================

void character_idle_state::OnBeingShotAt( object::type Type, guid ShooterID )
{
    character_state::OnBeingShotAt( Type, ShooterID );
}

//=========================================================================

void character_idle_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "IdleState",  "Different variables that effect the way that the character behaves when standing still.", 0 );

    List.PropEnumFloat( "IdleState\\WanderLeashRadius", "Threat lvl (1-10, 10 being highest) that will cause this character to alert and turn to a sound.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumFloat( "IdleState\\AverageTimeBetweenWanders", "Threat lvl (1-10, 10 being highest) that will cause this character to alert and turn to a sound.", PROP_TYPE_EXPOSE ) ;

    List.PropEnumEnum ( "IdleState\\Move Style", loco::GetMoveStyleEnum(), "What style of movement to use to get to location.", PROP_TYPE_MUST_ENUM | PROP_TYPE_EXPOSE );    
    character_state::OnEnumProp(List);
}

//=========================================================================

xbool character_idle_state::OnProperty ( prop_query& rPropQuery )
{
    if( rPropQuery.VarFloat("IdleState\\WanderLeashRadius", m_WanderLeashRadius) )
        return TRUE;
    if( rPropQuery.VarFloat("IdleState\\AverageTimeBetweenWanders", m_AverageTimeTillWander) )
        return TRUE;

    if ( rPropQuery.IsVar( "IdleState\\Move Style") )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( loco::GetMoveStyleName( m_MoveStyle ) ); 
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            m_MoveStyle = loco::GetMoveStyleByName  ( pString) ;
            return( TRUE );
        }
    }
    return character_state::OnProperty( rPropQuery );
}

//=========================================================================

const char*character_idle_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_IDLE_DEFAULT:
        return "PHASE_IDLE_DEFAULT";
    	break;
    case PHASE_IDLE_WANDER:
        return "PHASE_IDLE_DEFAULT";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
