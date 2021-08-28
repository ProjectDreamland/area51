#include "Character_Follow_State.hpp"
#include "Character_Cover_State.hpp"
#include "..\Character.hpp"
#include "Navigation\ng_node2.hpp"

//=========================================================================
// GRAY FOLLOW STATE
//=========================================================================

character_follow_state::character_follow_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_MoveStyle = loco::MOVE_STYLE_PROWL;
}

//=========================================================================

character_follow_state::~character_follow_state()
{
}

//=========================================================================

void character_follow_state::OnInit( void )
{
    m_FollowDistance = 400.0f;
    character_state::OnInit();
}

//=========================================================================

void character_follow_state::OnEnter( void )
{
    m_SpottedSurpriseDone = FALSE;
    character_state::OnEnter();
}

//=========================================================================

s32 character_follow_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;

    vector3 toFollowTarget = m_CharacterBase.GetToTarget( m_CharacterBase.GetFollowTargetGuid() );

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_FOLLOW_IDLE;
        break;
    case PHASE_FOLLOW_IDLE:
        if( m_TimeInPhase > 1.0f )
        {        
            if( toFollowTarget.LengthSquared() > ( m_FollowDistance + 600.0f ) * ( m_FollowDistance + 600.0f) )
            {
                return PHASE_FOLLOW_RUN_TO_TARGET;
            }
            else if( toFollowTarget.LengthSquared() > ( m_FollowDistance + 200.0f) * ( m_FollowDistance + 200.0f) )
            {
                return PHASE_FOLLOW_WALK_TO_TARGET;
            }
        }
        break;
    case PHASE_FOLLOW_RUN_TO_TARGET:
        if( toFollowTarget.LengthSquared() < (m_FollowDistance + 200.0f) * (m_FollowDistance + 200.0f) || m_CharacterBase.GetGoalCompleted() )
        {
            return PHASE_FOLLOW_WALK_TO_TARGET;
        }
        break;
    case PHASE_FOLLOW_WALK_TO_TARGET:
        if( toFollowTarget.LengthSquared() < m_FollowDistance * m_FollowDistance || m_CharacterBase.GetGoalCompleted() )
        {
            return PHASE_FOLLOW_IDLE;
        }
        else if( toFollowTarget.LengthSquared() > ( m_FollowDistance + 600.0f ) * ( m_FollowDistance + 600.0f) )
        {
            return PHASE_FOLLOW_RUN_TO_TARGET;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase in Character Follow State" );
        }
    };

    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }

    return newPhase;
}

//=========================================================================

void character_follow_state::ChangePhase( s32 newPhase )
{
    ng_connection2 destinationConnection;
    ng_node2 destinationNode;
    switch( newPhase ) 
    {
    case PHASE_FOLLOW_WALK_TO_TARGET:
        m_CharacterBase.SetGotoTargetGoal( m_CharacterBase.GetFollowTargetGuid() );
        break;
    case PHASE_FOLLOW_RUN_TO_TARGET:
        m_CharacterBase.SetGotoTargetGoal( m_CharacterBase.GetFollowTargetGuid(), vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_RUN );
        break;
    case PHASE_FOLLOW_IDLE:
        m_CharacterBase.SetIdleGoal();
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

character_state::states character_follow_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    if( !m_CharacterBase.GetFollowTargetGuid() )
    {
        return STATE_IDLE;
    }
    else if ( m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_ALERT )
    {
        return m_CharacterBase.GetStateFromAwareness();
    }
    return STATE_NULL;
}

//=========================================================================

void character_follow_state::OnThink( void )
{
    character_state::OnThink();
}

//=========================================================================

void character_follow_state::OnBeingShotAt( object::type Type, guid ShooterID )
{
    m_SpottedSurpriseDone = TRUE;
    character_state::OnBeingShotAt( Type, ShooterID );
}

//=========================================================================

void character_follow_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "FollowState",  "Different variables that effect the way that the character behaves when standing still.", 0 );

    s32 iHeader = List.PushPath( "FollowState\\" );        
    List.PropEnumFloat( "Follow Radius", "How closely we follow.", PROP_TYPE_EXPOSE );
    m_ResponseList.OnEnumProp( List );
    List.PopPath( iHeader );        
    character_state::OnEnumProp(List);
}

//=========================================================================

xbool character_follow_state::OnProperty ( prop_query& rPropQuery )
{
    s32 iHeader = rPropQuery.PushPath( "FollowState\\" );        
    if (rPropQuery.VarFloat("Follow Radius", m_FollowDistance))
        return TRUE;
    if( m_ResponseList.OnProperty( rPropQuery ) )
    {
        rPropQuery.PopPath( iHeader );
        return TRUE;
    }
    rPropQuery.PopPath( iHeader );

    return character_state::OnProperty( rPropQuery );
}

//=========================================================================

const char*character_follow_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_FOLLOW_IDLE:
        return "PHASE_FOLLOW_IDLE";
    	break;
    case PHASE_FOLLOW_RUN_TO_TARGET:
        return "PHASE_FOLLOW_RUN_TO_TARGET";
        break;
    case PHASE_FOLLOW_WALK_TO_TARGET:
        return "PHASE_FOLLOW_WALK_TO_TARGET";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
