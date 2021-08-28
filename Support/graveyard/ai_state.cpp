///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state.cpp
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ai_state.hpp"
#include "ai\Brain.hpp"
#include "objects\npc.hpp"
 
s32  ai_state::m_DummyNumber = 0;


//=================================================================================================
ai_state::ai_state(brain *myBrain) :
    m_TimeInState(0.0f),
    m_MinTimeInState(0.0f),
    m_TotalNumberOfCyclesInState(0),
    m_MaxTimeInState(999999.0f),
    m_Brain(myBrain),
    m_EmotionLevelToSwitchToBored( emotion_controller::EMOTION_LEVEL_ZERO ),
    m_LookAtPoint(0.0f, 0.0f, 0.0f ),
    m_LookAtPointWanderSpeed(25.0f),
    m_WanderingEyes(AI_EYE_WANDER_WANDER),
    m_WanderFrequency( 0.0f )
{
    m_DummyNumber++;
    m_CustomName[0] = 0;

#ifdef DEBUG
    m_TicksUsed = 0;
#endif//DEBUG

    OnInit();


}

//=================================================================================================
ai_state::~ai_state()
{


}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  OnAdvanceLogic - Normal advance logic call but only gets a call when this is the active state
//              in a brain.
//
//  OnEnterState - Called by brain when the state is entered.
//
//  OnAttemptExit - Called by brain to start a state exit.  Returns true if it is ready to exit, 
//              false if it needs more time.
//
//  OnExitState - Called by brain after OnAttemptExit returns true
//
//  OnInit - Called to reset the AI state to a base state.  Called by the constructor or to reset
//
//  GetType - returns the specific AI type  
//
//  GetName - returns the custom name for this AI
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//=================================================================================================
void ai_state::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state::OnAdvanceLogic" );

    (void)deltaTime;
    m_TimeInState += deltaTime; 
    m_TotalNumberOfCyclesInState++;
    UpdateLookAtObject(deltaTime);

}


//=================================================================================================
void ai_state::OnDamaged( s32 Damage, vector3* thisDirection )
{
    (void)Damage;
    (void)thisDirection;

}



//=================================================================================================
void ai_state:: OnEnterState( void )
{
    m_TimeInState = 0.0f;
    
}

//=================================================================================================
xbool ai_state::OnAttemptExit( void )
{
    if( m_TimeInState < m_MinTimeInState )
    {
        return false;

    }


    return true;

}

//=================================================================================================
void ai_state::OnExitState( void )
{



}

    
//=================================================================================================
void ai_state::OnInit( void )
{
    x_sprintf(m_CustomName,         "Temp State Name %2d", m_DummyNumber );
    x_strcpy(m_ExitStateNormal,     "None" );
    x_strcpy(m_ExitStateAgro,       "None" );
    x_strcpy(m_ExitStateBored,      "None" );

}

    

//=================================================================================================
const char* ai_state::GetName( void )
{
    return m_CustomName;


}


//=================================================================================================
void ai_state::SetName( const char* newName )
{
    x_strcpy(m_CustomName, newName);

}


//=================================================================================================
f32 ai_state::GetTimeInState( void )
{
    return m_TimeInState;

}



void ai_state::UpdateLookAtObject( f32 deltaTime )
{


    //  if we are more than 50 units from our final destination then default look at will be
    //  our next destination
//    if( m_Brain->GetDistanceToDestinationSquared() > 30000.0f )
    if( !m_Brain->GetOwner()->GetLocomotion()->IsAtDestination() )
    {
        m_Brain->SetLookAtDestination();
//        m_LookAtPoint.Zero();

    }
    else
    {
   
        //  base for an AI will be to look at the most recent sensor contact
        sensory_record* tempRecord = m_Brain->GetMostRecentContact();
        if(tempRecord )
        {
            m_Brain->SetLookAtObject(tempRecord->m_ContactObject );
            m_LookAtPoint.Zero();
        }
        else
        {
            //  else if no move to point and nothing to look at, pick a random target and move it
            //  every so often

            //  if X == Y, then odds are it was recently reset.  if not then it's supposed to be
            //  random anyways so pick a new spot
            if( m_WanderingEyes == AI_EYE_WANDER_NONE )
            {


            }
            else if (m_WanderingEyes == AI_EYE_WANDER_WANDER)
            {
            
                if( m_LookAtPoint.X == m_LookAtPoint.Z)
                {
                    m_LookAtPoint = m_Brain->GetOwner()->GetPosition();
                    m_LookAtPoint.Y += 100.0f;

                }

                m_LookAtPoint.X += deltaTime * x_frand(-m_LookAtPointWanderSpeed, m_LookAtPointWanderSpeed);
                m_LookAtPoint.Y += 0.1f * deltaTime * x_frand(-m_LookAtPointWanderSpeed, m_LookAtPointWanderSpeed);
                m_LookAtPoint.Z += deltaTime * x_frand(-m_LookAtPointWanderSpeed, m_LookAtPointWanderSpeed);

                m_Brain->SetLookAtPoint(m_LookAtPoint);
            }
            else
            {
                if( x_frand(0.0f,m_LookAtPointWanderSpeed+deltaTime ) > m_LookAtPointWanderSpeed )
                {
                    if( m_LookAtPoint.X == m_LookAtPoint.Z)
                    {
                        m_LookAtPoint = m_Brain->GetOwner()->GetPosition();
                        m_LookAtPoint.Y += 100.0f;

                    }
                    m_LookAtPoint.X += x_frand(-5000.0f, 5000.0f);
                    m_LookAtPoint.Y += x_frand(-50.0f, 50.0f);
                    m_LookAtPoint.Z += x_frand(-5000.0f, 5000.0f);

                    m_Brain->SetLookAtPoint(m_LookAtPoint);

                }
                else
                {
                    m_LookAtPoint.Zero();
                }

                
            }


        }
    }


}


guid ai_state::GetCurrentTarget( void )
{
    return 0;
}
















///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

//=================================================================================================
void ai_state::OnEnumProp( prop_enum&  List )
{
    List.AddHeader(xfs("AI State - %s",m_CustomName), "AI state represents a single unique state for an AI", PROP_TYPE_MUST_ENUM | PROP_TYPE_HEADER );
    List.AddButton(xfs("AI State - %s\\Delete State",m_CustomName) , "Delete this AI state", PROP_TYPE_MUST_ENUM );
    List.AddButton(xfs("AI State - %s\\Set As Base State",m_CustomName) , "Sets this as the base AI state", PROP_TYPE_MUST_ENUM );
    List.AddString(xfs("AI State - %s\\Name",m_CustomName),   "State's Custom Name", PROP_TYPE_MUST_ENUM  );
    List.AddString(xfs("AI State - %s\\AI Type",m_CustomName), "Type of AI, shows what class it uses", PROP_TYPE_READ_ONLY );
    List.AddInt(   xfs("AI State - %s\\AI Cycle Count",m_CustomName), "Shows how many frames this AI has been active", PROP_TYPE_READ_ONLY | PROP_TYPE_MUST_ENUM);
    List.AddFloat( xfs("AI State - %s\\Min Time In State",m_CustomName), "Minimum time in state" );
    List.AddFloat( xfs("AI State - %s\\Max Time In State",m_CustomName), "Maximum time in state" );

    List.AddEnum(  xfs("AI State - %s\\Eye wander Method",m_CustomName),"None\0Wander\0Random Target\0", "When bored, how fast does his look at point change" );
    List.AddFloat( xfs("AI State - %s\\Eye wander speed",m_CustomName), "When bored, how fast does his look at point change" );

    List.AddFloat( xfs("AI State - %s\\Wander Frequency",m_CustomName), "How often this state will wander.  0 is never, 1 is nonstop wandering" );
    


    List.AddHeader(xfs("AI State - %s\\Exit States",m_CustomName), "Exit States for this AI");


    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_NORMAL",m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );


    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_AGRO",m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_BORED", m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_BORED level needed", m_CustomName),
                    "EMOTION_LEVEL_ZERO\0EMOTION_LEVEL_MINIMAL\0EMOTION_LEVEL_MODERATE\0EMOTION_LEVEL_HIGH\0EMOTION_LEVEL_EXTREME\0EMOTION_LEVEL_IMPOSSIBLE\0",
                    "Level of boredom required for this AI to call the exit bored method",
                    PROP_TYPE_MUST_ENUM   );






}

//=================================================================================================
xbool ai_state::OnProperty( prop_query& I    )
{



    if( I.IsVar( xfs("AI State - %s\\Delete State",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton("Delete State" );
        }
        else
        {
            if( m_Brain )
            {
                m_Brain->SetStateToDelete(this);
                m_Brain->SetCurrentAIState(NULL);
            }
        }
        return TRUE;

    }    
    
    if( I.IsVar( xfs("AI State - %s\\Set As Base State",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton("Set As Base State" );
        }
        else
        {
            if( m_Brain )
            {
                m_Brain->SetCurrentAIState( this );
            }
        }
        return TRUE;

    }

    if( I.IsVar( xfs("AI State - %s\\Name",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( GetName(), 31 );
        }
        else
        {   
            x_strsavecpy(m_CustomName, I.GetVarString(), 31 );
        }


        return TRUE;
    }

    if( I.IsVar( xfs("AI State - %s\\AI Type",m_CustomName) ) )
    {
        if( I.IsRead() )    
        {
            I.SetVarString( GetTypeName(), 31 );
        }
        else
        {

        }
        
        return TRUE;
    }

    if( I.IsVar( xfs("AI State - %s\\AI Cycle Count",m_CustomName) ) )
    {
        if( I.IsRead() )    
        {
            I.SetVarInt(  m_TotalNumberOfCyclesInState );
        }
        else
        {

        }
        
        return TRUE;
    }

    if( I.IsVar(xfs("AI State - %s\\Min Time In State",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MinTimeInState);
        }
        else
        {   
            m_MinTimeInState = I.GetVarFloat();
        }


        return TRUE;
    }
    if( I.IsVar(xfs("AI State - %s\\Max Time In State",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MaxTimeInState);
        }
        else
        {   
            m_MaxTimeInState = I.GetVarFloat();
        }


        return TRUE;
    }


    if( I.IsVar(xfs("AI State - %s\\Eye wander speed",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_LookAtPointWanderSpeed);
        }
        else
        {   
            m_LookAtPointWanderSpeed = I.GetVarFloat();
        }


        return TRUE;
    }
 
    if( I.IsVar (  xfs("AI State - %s\\Eye wander Method",m_CustomName) ) ) 
    {
        if( I.IsRead() )
        {
            switch (m_WanderingEyes)
            {
                case AI_EYE_WANDER_NONE: I.SetVarEnum("None"); break;
                case AI_EYE_WANDER_WANDER: I.SetVarEnum("Wander"); break;
                case AI_EYE_WANDER_RANDOM: I.SetVarEnum("Random Target"); break;
                default: ASSERT(false);
            }

        }
        else
        {
            if( !x_strcmp("None", I.GetVarEnum() ) )
            {
                m_WanderingEyes = AI_EYE_WANDER_NONE;
            }
            else if( !x_strcmp("Wander", I.GetVarEnum() ) )
            {
                m_WanderingEyes = AI_EYE_WANDER_WANDER;

            }
            else if( !x_strcmp("Random Target", I.GetVarEnum() ) )
            {
                m_WanderingEyes = AI_EYE_WANDER_RANDOM;

            }
            else
            {
                ASSERT(false);
            }
        }
        return TRUE;

    }

    if( I.IsVar( xfs("AI State - %s\\Wander Frequency",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_WanderFrequency);
        }
        else
        {   
            m_WanderFrequency = I.GetVarFloat();
        }


        return TRUE;
    }





   if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_NORMAL",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateNormal );

        }
        else
        {
            x_strcpy(m_ExitStateNormal, I.GetVarEnum() );

        }
        return true;
    }

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_AGRO",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateAgro );

        }
        else
        {
            x_strcpy(m_ExitStateAgro, I.GetVarEnum() );

        }
        return true;
    }

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_BORED",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateBored );

        }
        else
        {
            x_strcpy(m_ExitStateBored, I.GetVarEnum() );

        }
        return true;
    }

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_BORED level needed", m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( emotion_controller::GetEmotionLevelString(m_EmotionLevelToSwitchToBored) );

        }
        else
        {
            m_EmotionLevelToSwitchToBored = emotion_controller::GetEmotionLevelFromString( I.GetVarEnum() );
        }
        return true;
    }






    return FALSE;
}


//=================================================================================================
s32 ai_state::GetCyclesInState( void )
{


    return m_TotalNumberOfCyclesInState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Performance
///////////////////////////////////////////////////////////////////////////////////////////////////

//=================================================================================================
void  ai_state::AddToTicksUsed(xtick timeUsed )
{

    m_TicksUsed += timeUsed;

}


//=================================================================================================
xtick ai_state::GetTicksUsed( void )
{
    return m_TicksUsed;
}
