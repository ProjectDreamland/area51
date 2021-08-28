///////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////
#include "AIStateMachine.hpp"


s32 m_sRefCountForMusic = 0;
s32 ai_state_machine::m_AngryCritters = 0;

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
ai_state_machine::ai_state_machine(slot_ID thisActor) :
    m_Actor(thisActor),
    m_State(STATE_NONE),
    m_TimeInState(0.0f),
    m_AnimName(NULL),
    m_npcSpotted(false),
    m_npcHeard(false),
    m_LastTimeHeard(0.0f),
    m_AwarenessRange(2500.0f),
    m_AwarenessVelocity(500.0f),
    m_MoveToLastKnownLocation(false),
    m_TimeToScanForTarget(2000)
{
            
    m_StateNames[STATE_FIRST]       = "First State";
    m_StateNames[STATE_NONE]        = "None";
    m_StateNames[STATE_IDLE]        = "Idle";
    m_StateNames[STATE_ATTACKING]   = "Attacking";
    m_StateNames[STATE_AWARE]       = "Aware";
    m_StateNames[STATE_FLEE]        = "Flee";
    m_StateNames[STATE_DEAD]        = "Dead";


    m_ScanningTurnRate = 2.0f;
    m_ChanceToShoot = 70.0f;
    m_ChanceToWander = 30.0f;
    m_RandomMoveDistance = 200.0f;
    m_WanderInIdle = false;

    m_TimeTurnStarted           = 0.0f;
    m_TimesShotSinceEvade       = 0;
    m_DeactivationDistance      = 2500.0f;
    m_Paused                    = false;

    m_FramesBetweenVisCheck = 5;
    m_FrameCount= 0;
    m_KilledByGrenade = false;


}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
ai_state_machine::~ai_state_machine()
{



}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void ai_state_machine::OnAdvanceLogic(f32 TimeDelta)
{
    CONTEXT( "ai_state_machine::OnAdvanceLogic" );

//    return;

//    x_printfxy(1,3," %s ", m_AnimName );


    object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );
    npc* tempnpc = (npc*)tempObject;

    slot_ID playerSlot = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    object* tempPlayer = g_ObjMgr.GetObjectBySlot(playerSlot);
    ASSERT( tempPlayer );

    vector3 distance = tempPlayer->GetPosition() - tempnpc->GetPosition();

    m_TimeInState += TimeDelta;
    if( distance.LengthSquared() > m_DeactivationDistance*m_DeactivationDistance || m_Paused )
    {
        return;
    }


    if( (m_FrameCount++)%m_FramesBetweenVisCheck  == 0)
    {
        if( tempnpc->IsPlayerVisible() && tempnpc->IsPointInFOV(tempPlayer->GetPosition()) )
        {
            m_npcSpotted = true;
        }
        else
        {
            m_npcSpotted = false;
        }
    }

    if(     tempnpc->GetDistanceToPlayer() < m_AwarenessRange  &&
            ((actor*)tempPlayer)->GetVelocity().Length() > m_AwarenessVelocity )
    {
        m_LastKnownLocation = tempPlayer->GetPosition();
        m_MoveToLastKnownLocation = true;
        m_npcHeard = true;
        m_LastTimeHeard = x_TicksToMs( x_GetTime( ) );
    } 
    else
    {
        m_npcHeard = false;

    }

    s32 animNum = tempnpc->GetAnimPlayer()->GetAnimIndex(m_AnimName) ;
    ASSERT(animNum != -1 );
    tempnpc->SetCurrentAnimSeq( animNum);

}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void ai_state_machine::SetState(AI_state newState)
{
    m_State  = newState;
    m_TimeInState = 0.0f;
}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
const char* ai_state_machine::GetStateName( void )
{
    return m_StateNames[m_State];
}


ai_state_machine::AI_state  ai_state_machine::GetState( void ) 
{ 
    return m_State;  
}

xbool  ai_state_machine::IsAngry( void )
{
    switch( m_State )
    {
    case STATE_ATTACKING:
    case STATE_AWARE:
        return true;

    }

    return false;

}



/*


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
anim_handle::anim_handle(char_anim_player* thisAnimPlayer, char* thisAnimName, char* thisOurName )
{
    m_AnimPlayer = thisAnimPlayer;
    x_strcpy(m_OurName,thisOurName);
    m_NameInAnim = thisAnimName;

    m_IndexInAnim = m_AnimPlayer->GetAnimIndex(m_NameInAnim );
}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
anim_handle::~anim_handle()
{



}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
s32 anim_handle::GetIndexFromName(const char* thisName )
{


}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
const char* anim_handle::GetNameFromIndex( s32 thisIndex )
{



}


  */



