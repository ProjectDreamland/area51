///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_follow.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_follow.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"
#include "objects\npc.hpp"

ai_state_follow::ai_state_follow(brain* myBrain ):
    ai_state(myBrain),
    m_FollowTarget(0),
    m_FollowDistance(200.0f),
    m_FollowFriend(true)
{



}


ai_state_follow::~ai_state_follow()
{



}



void ai_state_follow::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_follow::OnAdvanceLogic" );

    ai_state::OnAdvanceLogic(deltaTime);

    if(  m_FollowTarget == 0 && m_FollowFriend )
    {
        m_FollowTarget = m_Brain->FindAFriend();
    }


    if( m_Brain->IsNewContactHostile() )
    {
        m_Brain->RequestStateChange( m_ExitStateAgro );
               

    }
    else if( m_FollowTarget != 0 && !m_FollowFriend)
    {
        object* tempObject = g_ObjMgr.GetObjectByGuid(m_FollowTarget);
        ASSERT(tempObject);

        if( ( tempObject->GetPosition() - m_Brain->GetOwner()->GetPosition() ).LengthSquared() > m_FollowDistance*m_FollowDistance )
        {
            vector3 tempVector;
            tempVector = tempObject->GetPosition();

            m_Brain->SetNewDestination(tempVector );


        }
        else
        {
            m_Brain->ClearDestination();
        }


    }




}



void ai_state_follow::OnEnterState( void )
{
    ai_state::OnEnterState();


}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_follow::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );

 

    List.AddBool(      xfs("AI State - %s\\Follow Friend",m_CustomName),
                        "Do we follow a friend or a guid" );
    List.AddGuid(      xfs("AI State - %s\\Follow this object",m_CustomName),
                        "Only used if Follow Friend is set to false" );
    List.AddFloat(   xfs("AI State - %s\\Follow Distance",m_CustomName),
                        "How far behind do we follow" );





}


xbool ai_state_follow::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);
    if(returnVal)
        return true;

    if( I.IsVar(  xfs("AI State - %s\\Follow Friend",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarBool( m_FollowFriend);

        }
        else
        {
            m_FollowFriend =  I.GetVarBool();

        }
        return true;
    }

    if( I.IsVar(  xfs("AI State - %s\\Follow this object",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarGUID( m_FollowTarget);

        }
        else
        {
            m_FollowTarget =  I.GetVarGUID();

        }
        return true;
    }

    if( I.IsVar(  xfs("AI State - %s\\Follow Distance",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_FollowDistance );
        }
        else
        {
            m_FollowDistance =  I.GetVarFloat( );
        }
        return true;

    }



    return returnVal;

}


