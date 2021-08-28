#include "Character_Search_State.hpp"
#include "..\Character.hpp"
#include "Navigation\ng_node2.hpp"
//=========================================================================
// constants
//=========================================================================

const f32 k_MaxLookAroundTime   = 5.0f;
const f32 k_MinTimeIdling       = 1.5f;
const f32 k_MinTimeInPhase      = 1.0f;

//=========================================================================
// GRAY SEARCH STATE
//=========================================================================

character_search_state::character_search_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_TimeTillBored = 10.0f;
    m_MoveStyle = loco::MOVE_STYLE_PROWL;
}

//=========================================================================

character_search_state::~character_search_state()
{
}

//=========================================================================

void character_search_state::OnInit( void )
{
    character_state::OnInit();
}

//=========================================================================

void character_search_state::OnEnter( void )
{
    m_LookAroundTime = 0.0f;
    m_TimeAtLocationOfInterest = 0.0f;
    character_state::OnEnter();
}

//=========================================================================

s32 character_search_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_SEARCH_GOTO_INTEREST;
        break;
    case PHASE_SEARCH_GOTO_INTEREST:
        m_TimeAtLocationOfInterest += DeltaTime/2.0f;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_LookAroundTime = 0.0f;
            newPhase = PHASE_SEARCH_LOOK_AROUND;
        }
        else if( m_TimeInPhase > k_MinTimeInPhase && m_CharacterBase.GetSoundHeard() )
        {
            newPhase = PHASE_SEARCH_GOTO_INTEREST;
        }
        break;
    case PHASE_SEARCH_LOOK_AROUND:
//        m_LookAroundTime += DeltaTime;
        m_TimeAtLocationOfInterest += DeltaTime;
        if( m_LookAroundTime >= k_MaxLookAroundTime )
        {
            newPhase = PHASE_SEARCH_WALK_AROUND;
        }
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_SEARCH_IDLE_MOMENTARILY;
        }
        else if( m_TimeInPhase > k_MinTimeInPhase && m_CharacterBase.GetSoundHeard() )
        {
            newPhase = PHASE_SEARCH_GOTO_INTEREST;
        }
        break;
    case PHASE_SEARCH_IDLE_MOMENTARILY:
        m_TimeAtLocationOfInterest += DeltaTime;
        if( m_TimeInPhase > k_MinTimeIdling )
        {
            newPhase = PHASE_SEARCH_LOOK_AROUND;
        }
        else if( m_TimeInPhase > k_MinTimeInPhase && m_CharacterBase.GetSoundHeard() )
        {
            newPhase = PHASE_SEARCH_GOTO_INTEREST;
        }
        break;
    case PHASE_SEARCH_WALK_AROUND:
        m_TimeAtLocationOfInterest += DeltaTime;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_LookAroundTime = 0.0f;
            newPhase = PHASE_SEARCH_LOOK_AROUND;
        }
        else if( m_TimeInPhase > k_MinTimeInPhase && m_CharacterBase.GetSoundHeard() )
        {
            newPhase = PHASE_SEARCH_GOTO_INTEREST;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
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

void character_search_state::ChangePhase( s32 newPhase )
{
    f32 randomYaw;
    vector3 lookahead;
    switch( newPhase ) 
    {
    case PHASE_SEARCH_GOTO_INTEREST:
        m_TimeAtLocationOfInterest = 0.0f;
        m_CharacterBase.SetGotoLocationGoal( m_CharacterBase.GetLastLocationOfInterest() );
    	break;
    case PHASE_SEARCH_IDLE_MOMENTARILY:
        m_CharacterBase.SetIdleGoal();
    	break;
    case PHASE_SEARCH_LOOK_AROUND:
        // pick a random yaw and turn towards it.
        randomYaw = x_frand(-180,180);
        lookahead = vector3(100.0f,0.0f,0.0f);
        lookahead.RotateY(DEG_TO_RAD(randomYaw));
        lookahead += m_CharacterBase.GetLocoPointer()->GetEyeOffset() ;
        m_CharacterBase.SetTurnToLocationGoal( m_CharacterBase.GetPosition() + lookahead );
    	break;
    case PHASE_SEARCH_WALK_AROUND:
        m_CharacterBase.SetGotoLocationGoal( m_CharacterBase.GetClosestNode().GetPosition() );
    	break;
    case PHASE_SEARCH_SPOTTED_SURPRISE:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_SPOT_TARGET);
    	break;
    default:        
        if( newPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid New Phase " );
        }
    }
    character_state::ChangePhase( newPhase );
}

//=========================================================================

character_state::states character_search_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    if( m_CharacterBase.GetCurrentCover() &&
        m_CharacterBase.HasState(STATE_COVER))
    {
        return STATE_COVER;
    }
    else if( m_CharacterBase.GetAwarenessLevel() != character::AWARENESS_SEARCHING )
    {
        return m_CharacterBase.GetStateFromAwareness();
    }
    return STATE_NULL;
}

//=========================================================================

void character_search_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "SearchState",  "Different variables that effect the way that the character behaves when standing still.", 0 );
    List.PropEnumFloat( "SearchState\\TimeTillBored", "Amount of Time to stay in state with no new sounds heard.", 0 ) ;
}

//=========================================================================

xbool character_search_state::OnProperty ( prop_query& rPropQuery )
{
    if (rPropQuery.VarFloat("SearchState\\TimeTillBored", m_TimeTillBored))
        return TRUE;

    return FALSE ;
}

//=========================================================================

const char*character_search_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_SEARCH_GOTO_INTEREST:
        return "PHASE_SEARCH_GOTO_INTEREST";
    	break;
    case PHASE_SEARCH_LOOK_AROUND:
        return "PHASE_SEARCH_LOOK_AROUND";
    	break;
    case PHASE_SEARCH_IDLE_MOMENTARILY:
        return "PHASE_SEARCH_IDLE_MOMENTARILY";
    	break;
    case PHASE_SEARCH_WALK_AROUND:
        return "PHASE_SEARCH_WALK_AROUND";
    	break;
    case PHASE_SEARCH_SPOTTED_SURPRISE:
        return "PHASE_SEARCH_SPOTTED_SURPRISE";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
