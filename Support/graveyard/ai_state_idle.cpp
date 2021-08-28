///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_idle.cpp
//
//      - implements an idle state for an actor
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_idle.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"
#include "objects\npc.hpp"
#include "Audio\audio_stream_mgr.hpp"

//=================================================================================================
ai_state_idle::ai_state_idle(brain* myBrain ) :
    ai_state(myBrain)
{

    OnInit();

}


//=================================================================================================
ai_state_idle::~ai_state_idle()
{



}



//=================================================================================================
void ai_state_idle::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_idle::OnAdvanceLogic" );

    //  tell locomotion system to idle
    //  Set locomotion look at to look at last new contact and Aim at last new hostile contact
    ai_state::OnAdvanceLogic(deltaTime);



    if( m_Brain->IsNewContactHostile() )
    {
        m_Brain->RequestStateChange( m_ExitStateAgro );
                

    }
    else
    {
        emotion_controller& emotions  = m_Brain->GetEmotions();
        
        if( emotions.GetEmotionLevel( emotion_controller::EMOTION_BOREDOM ) >= m_EmotionLevelToSwitchToBored )
        {
//            m_Brain->RequestStateChange( m_ExitStateBored );
        }
       
        else if(GetTimeInState() > m_MaxTimeInState )
        {
            m_Brain->RequestStateChange( m_ExitStateNormal );

        }



    }






}


//=================================================================================================
void ai_state_idle::OnDamaged(s32 thisDamage, vector3* thisDirection )
{
    (void)thisDamage;
    (void)thisDirection;
    m_Brain->RequestStateChange( m_ExitStateAgro );


}


//=================================================================================================
void ai_state_idle::OnEnterState( void )
{
    //  switch to animation stuff
    //  cancel movement?


}


//=================================================================================================
xbool ai_state_idle::OnAttemptExit( void )
{

    return true;
}


//=================================================================================================
void ai_state_idle::OnExitState( void )
{



}


    
//=================================================================================================
void ai_state_idle::OnInit( void )
{
    x_strcpy(m_ExitStateCurious,    "None");


}


    





///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

//=================================================================================================
void ai_state_idle::OnEnumProp( prop_enum&  List )
{
    ai_state::OnEnumProp( List );

 
 

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_CURIOUS",m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );

 



}


//=================================================================================================
xbool ai_state_idle::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);

    if(returnVal)
        return true;

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_CURIOUS",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateCurious );

        }
        else
        {
            x_strcpy(m_ExitStateCurious, I.GetVarEnum() );

        }
        return true;
    }




    return false;
}


