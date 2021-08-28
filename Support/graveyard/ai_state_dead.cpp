///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_curious.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_dead.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"
#include "objects\npc.hpp"
#include "objects\DeadBody.hpp"
#include "Audio\audio_stream_mgr.hpp"

ai_state_dead::ai_state_dead(brain* myBrain ):
    ai_state(myBrain)
{



}


ai_state_dead::~ai_state_dead()
{



}



void ai_state_dead::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_dead::OnAdvanceLogic" );

    (void)deltaTime;

	//when the death animation finishes, spawn a dead body.
	if ( m_Brain->GetOwner()->GetLocomotion()->m_Player.IsAtEnd() )
	{
		//create a new dead body, pass the death animation index.
        guid bodyGuid = g_ObjMgr.CreateObject( dead_body::GetObjectType().GetTypeName() );
		object* pDeadBody = g_ObjMgr.GetObjectByGuid( bodyGuid );
		s32 nAnimIndex = m_Brain->GetOwner()->GetLocomotion()->m_Player.GetAnimIndex();
		( ( dead_body* )pDeadBody )->InitializeDeadBody( m_Brain->GetOwner()->GetSkinInst().GetSkinGeomName() , m_Brain->GetOwner()->GetLocomotion()->m_Player , m_Brain->GetOwner()->GetLocomotion()->m_Physics , nAnimIndex );
		
		//set owner up for destruction
		m_Brain->GetOwner()->Disable();
	}

}


    
void ai_state_dead::OnEnterState( void )
{
    m_Brain->DisableMovement();
    m_Brain->GetOwner()->GetLocomotion()->PlayAnimState( base_loco::AT_DEATH );
    m_Brain->GetOwner()->GetLocomotion()->m_Player.SetCycle(0);
    
    if( g_AudioManager.IsPlaying( m_Brain->m_AlertVoiceID ) || g_AudioManager.IsStarting( m_Brain->m_AlertVoiceID ) )
        g_AudioManager.Release( m_Brain->m_AlertVoiceID, 0.0f );
    
    // Play the Spec Ops death audio.
    s8 DescName[32];
    x_sprintf( (char*)DescName, "SpecOps%d_Death", m_Brain->m_SpecOpType );
    g_AudioManager.Play( (const char*)DescName, m_Brain->GetOwner()->GetPosition() );

    slot_id tempSlot = g_ObjMgr.GetFirst( object::TYPE_NPC );
    object* tempObject = g_ObjMgr.GetObjectBySlot( tempSlot );

    object* closestObject = NULL;
    f32     closestDistance = 99999999.0f;
    while( tempSlot != SLOT_NULL && tempObject )
    {
        f32 thisDistance = ( tempObject->GetPosition() - m_Brain->GetOwner()->GetPosition() ).Length();
        if( (thisDistance < closestDistance) && (tempObject->GetGuid() != m_Brain->GetOwner()->GetGuid()) )
        {
            closestObject = tempObject;
            closestDistance = thisDistance;
        }
        tempSlot = g_ObjMgr.GetNext(tempSlot);

        if(tempSlot != SLOT_NULL )
        {
            tempObject = g_ObjMgr.GetObjectBySlot(tempSlot);
            if( tempObject->GetGuid() == m_Brain->GetOwner()->GetGuid() )
            {
                tempSlot = g_ObjMgr.GetNext(tempSlot);
            }
        }
    }

    if( closestObject )
    {
        if(closestDistance < 2500.0f )
        {
            if( closestObject->GetGuid() == m_Brain->GetOwner()->GetGuid()  )
                return;
            
            npc& rNpc = npc::GetSafeType( *closestObject );
            
            // Make sure that its not a stage 5.
            if( rNpc.GetCharacterType() == npc::NPC_TYPE_STAGE_5 )
                return;

            if( g_StreamCount >= 3 )
                return;

            // Let the other npc play the man down sound.
            s8 DescName[32];
            x_sprintf( (char*)DescName, "SpecOps%d_ManDies", x_irand( 1, 3 ) );
            m_Brain->m_ManDownVoiceID = g_AudioManager.Play( (const char*)DescName, closestObject->GetPosition() );

        }
    }


}


xbool ai_state_dead::OnAttemptExit( void )
{


    return false;
}


void ai_state_dead::OnExitState( void )
{



}


    
void ai_state_dead::OnInit( void )
{



}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_dead::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );





}


xbool ai_state_dead::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);
    if(returnVal)
        return true;

 


    return returnVal;

}


