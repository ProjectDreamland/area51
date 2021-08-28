///////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////
#include "TrooperAI.hpp"
#include "objects\npc.hpp"


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
trooper_ai::trooper_ai(slot_ID thisActor) :
    ai_state_machine(thisActor)
{

    m_TimeTurnStarted = 0.0f;
    m_State = STATE_IDLE;
    m_AnimName = "idle1";
    m_TimesShotSinceEvade = 0;
}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
trooper_ai::~trooper_ai()
{



}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::OnAdvanceLogic(f32 TimeDelta)
{
    CONTEXT( "trooper_ai::OnAdvanceLogic" );

    ai_state_machine::OnAdvanceLogic(TimeDelta);
    

    
    switch( m_State )
    {
    case STATE_IDLE:
        {
            AIStateIdle( TimeDelta );
        }
        break;
    case STATE_ATTACKING:
        {
            AIStateAttack( TimeDelta );
            
        }
        break;
    case STATE_AWARE:
        {
            AIStateAware( TimeDelta );
        }
        break;
    case STATE_FLEE:
        {
            AIStateFlee( TimeDelta );
        }
        break;
    case STATE_DEAD:
        {
            AIStateDead( TimeDelta );
        }
        break;

    }





}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::SetState(AI_state newState)
{
    ai_state_machine::SetState(newState);

    m_TurnDirection     = STATE_DIRECTION_NONE;

    switch( m_State )
    {   
    case STATE_AWARE:
        {
            m_AnimName      = "idle3"; //4
        }
        break;

    case STATE_IDLE:
        {
            m_AnimName = "idle1"; //1;
        }
        break;

    case STATE_DEAD:
        {
            if(x_rand()%2)
                m_AnimName = "death1";  //5;
            else
                m_AnimName = "death4";
        }
        break;

    case STATE_ATTACKING:
        {
//            m_AnimName = "idle3"; //4;
        }
        break;

    case STATE_FLEE:
        {
            m_AnimName = "run2";  //10;
        }
        break;
    default:
        {
        }
        break;
    }



}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::AIStateAttack ( f32 DeltaTime )
{
    const f32   kDesiredDistanceForShooting = 1500.0f;

    (void)DeltaTime;
    object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );

    npc* tempnpc = (npc*)tempObject;

    ASSERT( tempnpc->GetAnimPlayer() );
    
    //
    //  First we check to see if we need to change states
    //
    f32 distanceToPlayer = tempnpc->GetDistanceToPlayer();
    f32 currentTime = x_TicksToMs( x_GetTime( ) );

    //  Time do do some logic checking for mode switching
    if( distanceToPlayer > m_AwarenessRange && ( currentTime -m_LastTimeHeard ) > 10.0f )
    {
        SetState ( STATE_AWARE );
        m_npcSpotted = false;
        return;

    }

    
    xbool playerVisible = tempnpc->IsPlayerVisible();

    //  Ok, if no state change is needed then we just update our animation

    //  Is the current animation done?  If so then time to pick a new animation
    s32 currentAnim = tempnpc->GetAnimPlayer()->GetAnimIndex();

    if( tempnpc->GetAnimPlayer()->IsAtEnd( ) || ( m_TimesShotSinceEvade && currentAnim != 20 && currentAnim != 23) ) 
    {
        if ( tempnpc->GetAnimPlayer()->IsAtEnd( ) && ( currentAnim == 20 || currentAnim == 23 ) )
        {
            m_TimesShotSinceEvade = 0;
        }

        switch(currentAnim)
        {
        case 4:
        case 0:
        case 8:
        case 1:
        case 10:
        case 20:
        case 23:
            {
                if(playerVisible)
                {
                    
                    if(  !m_TimesShotSinceEvade)
                    {
                        // Ok, decided it's time to shoot.  Let's check the distance
                        // and decide if we should 
                        if(distanceToPlayer > kDesiredDistanceForShooting )
                        {
                            tempnpc->LookAtPlayer();
                            m_AnimName = "jog2"; //7
                            tempnpc->Shoot();
                        }
                        else
                        {
                            tempnpc->LookAtPlayer();
                            tempnpc->Shoot();
                            m_AnimName  = "fire"; //0
                        }
                    }
                    else 
                    {


                        if( m_TimesShotSinceEvade > 0 )
                        {
                            vector3 newSpot;
                            f32 angleToPlayer ;
                            s32 tries = 5;
                            do 
                            {
                                newSpot = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                                angleToPlayer = tempnpc->GetAngleToPoint(newSpot);
                            } while( !((  ( angleToPlayer > 1.2f && angleToPlayer < 1.7f ) || 
                                        (angleToPlayer < -1.2f && angleToPlayer > -1.7f) ) 
                                        && tries-- ));
                        

                            if( angleToPlayer > 1.2f && angleToPlayer < 1.7f  )
                            {
                                tempnpc->LookAtPoint(newSpot, -angleToPlayer  );
                                m_AnimName = "evade4";                                                      

                            }
                            else if (angleToPlayer < -1.2f && angleToPlayer > -1.7f)
                            {
                                tempnpc->LookAtPoint(newSpot, angleToPlayer  );
                                m_AnimName = "evade1";                                                      

                            }
                        }
                        else
                        {
                            // else, let's just run around like a fool for no real reason
                            vector3 newSpot = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                            m_AnimName = "jog3";    //10
                            tempnpc->LookAtPoint(newSpot);

                        }
                        m_TimesShotSinceEvade = 0;
                        
                    }
                }
                else
                {
                        vector3 newSpot = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                        m_AnimName = "jog3";    //10
                        tempnpc->LookAtPoint(newSpot);

                }
                
            }
            break;
        case 7:
            {
               if(distanceToPlayer > kDesiredDistanceForShooting )
                {
                    tempnpc->LookAtPlayer();
                    m_AnimName = "jog2"; //7
               }
                else
                {
                    tempnpc->LookAtPlayer();
                    m_AnimName  = "fire"; //0
                }

                
            }
            break;
        case 9:
            {
                ASSERT(FALSE);
                const s32 kRepeatGun = 30;
                if( x_rand() %100 < kRepeatGun )
                {
                    m_AnimName  = "idle1"; //1;
                }
                else
                {
                    m_AnimName  = "idle1"; //1;
                }

            }
            break;

        default:
            {
                ASSERT(FALSE);
            }
            break;
        }


    }
    else
    {
        //  if current animation is not done and nothings going on then just
        //  return and keep playing the current animation
        return;

    }

}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::AIStateAware ( f32 DeltaTime )
{
     (void)DeltaTime;
    object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );

    npc* tempnpc = (npc*)tempObject;

    ASSERT( tempnpc->GetAnimPlayer() );


    //
    //  First we check to see if we need to change states
    //
    f32 distanceToPlayer        = tempnpc->GetDistanceToPlayer();
    f32 currentTime             = x_TicksToMs( x_GetTime( ) );
    f32 m_TimeInState    = currentTime - m_TimeInState;

    //  Time do do some logic checking for mode switching
    if( ( distanceToPlayer > m_AwarenessRange  )  ) // ||  
//            (currentTime - m_LastTimeHeard) > m_MinimumTimeInAware )
    {

        SetState( STATE_IDLE );
        m_npcHeard = false;
        return;
    }

    if(m_npcHeard)
    {
        m_TurnDirection = STATE_DIRECTION_NONE;

    }


    if( !m_npcSpotted  )
    {
        if( m_TurnDirection  == STATE_DIRECTION_NONE )
        {
            m_TimeTurnStarted = currentTime;
            if( tempnpc->IsPlayerToLeft() )
            {
            
                m_AnimName = "Turn_Left_90";
                m_TurnDirection = STATE_LEFT;
            }
            else
            {
                m_AnimName = "Turn_Left_90";
                m_TurnDirection = STATE_RIGHT;
            }


        }

        
        if( ( currentTime -m_TimeTurnStarted ) < m_TimeToScanForTarget )
        {
//            if(x_strncmp("Turn",m_AnimName,4 ) ) 
                m_AnimName = "idle3";//4;
            tempnpc->Rotate( m_ScanningTurnRate* DeltaTime * (m_TurnDirection==STATE_LEFT?1.0f:-1.0f) );
            return;
        }
        else if( !x_stricmp( "idle3", m_AnimName ) )
        {
            m_AnimName= "jog3"; // = 8;
            return;
        }
        else if (tempnpc->GetAnimPlayer()->IsAtEnd() )
        {
//            if(x_strncmp("Turn",m_AnimName,4 ) ) 
                m_AnimName = "idle3"; //4;
            m_TimeTurnStarted = currentTime;
            m_TimeToScanForTarget = m_TimeToScanForTarget + (x_rand()%1000) - 500;
            return;
            
        }
    }
    else
    {
        m_TurnDirection = STATE_DIRECTION_NONE;
        SetState( STATE_ATTACKING );
        return;
    }

    //  Ok, if no state change is needed then we just update our animation

    //  Is the current animation done?  If so then time to pick a new animation
    if( tempnpc->GetAnimPlayer()->IsAtEnd( ) )
    {
        s32 currentAnim = tempnpc->GetAnimPlayer()->GetAnimIndex( );
    

        //  Not real clear on this whole state....
        switch(currentAnim)
        {
        case 2:
        case 5:
            {
                if( x_rand() %100 < m_ChanceToShoot )
                {
                    m_AnimName = "idle3"; // = 4;
                }
                else
                {
                    // not sure what the desired 
                    tempnpc->LookAtPlayer();
                    m_AnimName = "jog3";//= 8;

                }

            }
            break;
        default:
            {
                ASSERT(FALSE);
            }
            break;

        }


    }
    else
    {
        //  if current animation is not done and nothings going on then just
        //  return and keep playing the current animation
        return;

    }



}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::AIStateFlee ( f32 DeltaTime )
{
  (void)DeltaTime;
   object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );

    npc* tempnpc = (npc*)tempObject;

    ASSERT( tempnpc->GetAnimPlayer() );



    //
    //  First we check to see if we need to change states
    //

    f32 distanceToPlayer = tempnpc->GetDistanceToPlayer();
//    f32 currentTime = x_TicksToMs( x_GetTime( ) );
//    f32 m_TimeInState = currentTime - m_LastStateChange;

    //  Time do do some logic checking for mode switching
    if( distanceToPlayer > m_AwarenessRange   )
    {
        SetState(STATE_ATTACKING);
        m_npcSpotted = true;;
        return;

    }


    //  Ok, if no state change is needed then we just update our animation

    //  Is the current animation done?  If so then time to pick a new animation
    if( tempnpc->GetAnimPlayer()->IsAtEnd( ) )
    {
//        s32 currentAnim = m_AnimPlayer.GetAnimIndex();
//        switch(currentAnim)
        {
//        case 7:
//        case 10:
            {
                tempnpc->LookAwayFromPlayer();
                m_AnimName = "run3";// = 10;
                
            }
//            break;

//        default:
//            {
//                ASSERT(FALSE);
//            }
//            break;

        }
        



    }
    else
    {
        //  if current animation is not done and nothings going on then just
        //  return and keep playing the current animation
        return;

    }


}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::AIStateIdle ( f32 DeltaTime )
{
    (void)DeltaTime;
    object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );

    npc* tempnpc = (npc*)tempObject;

    ASSERT( tempnpc->GetAnimPlayer() );

    //
    //  First we check to see if we need to change states
    //
    if(m_npcSpotted )//&& (x_TicksToMs( x_GetTime( ) ) - m_LastStateChange ) > m_MinimumTimeInIdle  )
    {
        SetState(STATE_ATTACKING );
        return;
    }
    

    

    if(     m_npcHeard)// && 
//            (x_TicksToMs( x_GetTime( ) ) - m_LastStateChange ) > m_MinimumTimeInIdle )
    {
        m_npcHeard  = false;
        SetState( STATE_AWARE );
        return;   
    }

    //  Ok, if no state change is needed then we just update our animation

    //  Is the current animation done?  If so then time to pick a new animation
    if( tempnpc->GetAnimPlayer()->IsAtEnd( ) )
    {
        s32 currentAnim = tempnpc->GetAnimPlayer()->GetAnimIndex();
        switch(currentAnim)
        {
        case 12:
            {
                m_AnimName = "idle1";//= 1;
                
            }
            break;
/*        case 4:
            {
                const s32 kRepeatGun = 30;
                if( x_rand() %100 < kRepeatGun )
                {
                    m_CurrentAnimSeq  = 4;
                }
                else
                {
                    m_CurrentAnimSeq = 1;
                }

            }
            break;
*/
        default:
            {
              const s32 kRepeatIdle = 20;
                if( x_rand() %100 < kRepeatIdle )
                {
                    m_AnimName = "idle1";//  = 1;
                }
                else if (x_rand() %100 < m_ChanceToWander  &&  m_WanderInIdle )
                {
                   vector3 aPoint = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                   tempnpc->LookAtPoint(aPoint);
                   m_AnimName = "walk2";// = 13;
                }
                else
                {
                    
                    m_AnimName = "idle1";//= 1;
                }

            }
            break;

        }





    }
    else
    {
        //  if current animation is not done and nothings going on then just
        //  return and keep playing the current animation
        return;

    }



}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void trooper_ai::AIStateDead ( f32 DeltaTime )
{
    object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );

    npc* tempnpc = (npc*)tempObject;

    ASSERT( tempnpc->GetAnimPlayer() );

    (void)DeltaTime;
    if( tempnpc->GetAnimPlayer()->IsAtEnd( ) )
    {
        s32 currentAnim = tempnpc->GetAnimPlayer()->GetAnimIndex();
        if(currentAnim == 5  || currentAnim == 16 )
        {
            tempnpc->PauseAnim();
        }
        else
        {
//            m_AnimName = "death1";//= 5;
        }
    }

}


void trooper_ai::GotShot( slot_ID thisSlot, s32 Damage )
{
    (void)thisSlot;
    m_TimesShotSinceEvade += Damage;

}