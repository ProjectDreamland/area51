///////////////////////////////////////////////////////////////////////////////
//
//  MutantAI.cpp
//
//
///////////////////////////////////////////////////////////////////////////////
#include "MutantAI.hpp"
#include "objects\npc.hpp"





///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
mutant_ai::mutant_ai(slot_ID thisActor) :
    ai_state_machine(thisActor),
    m_BaseActivity(MUTANT_GUARD)
{


    m_State = STATE_IDLE;
    m_AnimName = "idle1";
    m_AwarenessRange = 2500.0f;
    m_AppliedDamageThisAttack = false;
    m_TimeToFlinch = false;

}



///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
mutant_ai::~mutant_ai()
{



}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void mutant_ai::OnAdvanceLogic(f32 TimeDelta)
{
    CONTEXT( "mutant_ai::OnAdvanceLogic" );

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
void mutant_ai::SetState(AI_state newState)
{

    if( newState == m_State )
        return;

    ai_state_machine::SetState(newState);

    m_TurnDirection     = STATE_DIRECTION_NONE;

    switch( m_State )
    {   
    case STATE_AWARE:
        {

            m_AnimName = "idle2"; //4
        }
        break;

    case STATE_IDLE:
        {
            m_AnimName = "idle2"; //1;
        }
        break;

    case STATE_DEAD:
        {
            if( m_KilledByGrenade )
            {
                m_AnimName = "explode5";
            }

            else if(x_rand() %2 )
            {
                m_AnimName = "death1";  //5;
            }
            else
            { 
                
                m_AnimName = "death2";  //5;
            }
        }
        break;

    case STATE_ATTACKING:
        {
            if( x_rand() % 5 )
            {
                object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
                ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );
                npc* tempnpc = (npc*)tempObject;

                tempnpc->LookAtPlayer();
                m_AnimName = "run";

            }
            else
            {
                m_AnimName = "idle3"; //4;
            }
        }
        break;

    case STATE_FLEE:
        {
//            m_AnimName = "run2";  //10;
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
void mutant_ai::AIStateAttack ( f32 DeltaTime )
{
    const f32   kDesiredDistanceForShooting = 300.0f;

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

    
    if ( CheckForDamageUpdate( currentAnim, tempnpc->GetAnimPlayer()->GetFrame(), distanceToPlayer ) )
    {
        tempnpc->Shoot();
    }



    if( tempnpc->GetAnimPlayer()->IsAtEnd( )  ) // || ( m_TimesShotSinceEvade && currentAnim != 12 && currentAnim != 10 && currentAnim != 11) ) 
    {
//        if ( tempnpc->GetAnimPlayer()->IsAtEnd( ) && ( currentAnim == 12 || currentAnim == 10|| currentAnim == 11 ) )
//        {
//            m_TimesShotSinceEvade = 0;
//        }

        switch(currentAnim)
        {

        case 15:
            {

                if( distanceToPlayer > 500.0f && distanceToPlayer < 600.0f && x_rand()%2 )
                {
                    tempnpc->LookAtPlayer();
                    m_AnimName = "attack2";
                    m_AppliedDamageThisAttack = false;
//                    tempnpc->Shoot();                    
                    

                }

                else if(distanceToPlayer > kDesiredDistanceForShooting * 1.3f )
                {
                    tempnpc->LookAtPlayer();
                    if(x_rand() %20 > 7 )
                    {
                        m_AnimName = "run";
                    }
                    else
                    {
                        m_AnimName = "jog";
                    }
                }
                else if(distanceToPlayer > kDesiredDistanceForShooting )
                {
  //                  if(x_rand() %20 > 5 )
                    {
                        m_AnimName = "walk";
                    }
//                    else
                    {
//                        m_AnimName = "walk";
                    }
                }
                else
                {
                    tempnpc->LookAtPlayer();
                    m_AnimName  = "attack3"; //0
                    m_AppliedDamageThisAttack = false;

                }

                
            }
            break;
/*        case 9:
            {
//                ASSERT(FALSE);
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
*/
        default:
            {
                if(playerVisible)
                {
                    
//                    if(  !m_TimesShotSinceEvade)
                    {
                        // Ok, decided it's time to shoot.  Let's check the distance
                                // and decide if we should 
                       if( distanceToPlayer > 500.0f && distanceToPlayer < 600.0f && x_rand()%2 )
                        {
                            tempnpc->LookAtPlayer();
                            m_AnimName = "attack2";
                            m_AppliedDamageThisAttack = false;

//                            tempnpc->Shoot();                    
                    

                        }

                        else if(distanceToPlayer > kDesiredDistanceForShooting * 1.3 )
                        {
                            tempnpc->LookAtPlayer();
        //                    if(x_rand() %20 > 7 )
                            {
                                m_AnimName = "run";
                            }
          //                  else
          //                  {
            //                    m_AnimName = "jog";
              //              }
                        }
                        else if(distanceToPlayer > kDesiredDistanceForShooting )
                        {
  //                          if(x_rand() %20 > 5 )
                            {
                                m_AnimName = "walk";
                            }
//                            else
                            {
//                                m_AnimName = "walk";
                            }
                        }
                        else
                        {
                            tempnpc->LookAtPlayer();
//                            tempnpc->Shoot();
                            m_AppliedDamageThisAttack = false;

                            if(x_rand()%2)
                                m_AnimName  = "attack1"; //0
                            else
                                m_AnimName  = "attack3";
                        }
                    }
/*                    else 
                    {


                        if( m_TimesShotSinceEvade > 0 )
                        {
                            vector3 newSpot;
                            f32 angleToPlayer ;
                            s32 tries = 5;

                            if( m_TimeToFlinch   )// || m_TimesShotSinceEvade > 40 )
                            {
                                m_TimeToFlinch = false;
                                m_AnimName = "idle1";

                            }
                            else if ( x_rand()%10  ) 
                            {
                                if(distanceToPlayer > kDesiredDistanceForShooting * 1.5 )
                                {
                                    tempnpc->LookAtPlayer();
                //                    if(x_rand() %20 > 7 )
                                    {
                                        m_AnimName = "run";
                                    }
                  //                  else
                  //                  {
                    //                    m_AnimName = "jog";
                      //              }
                                }
                                else if(distanceToPlayer > kDesiredDistanceForShooting )
                                {
    //                                if(x_rand() %20 > 5 )
                                    {
                                        m_AnimName = "jog";
                                    }
  //                                  else
                                    {
//                                        m_AnimName = "walk";
                                    }
                                }
                                else
                                {
                                    tempnpc->LookAtPlayer();
//                                    tempnpc->Shoot();
                                    m_AppliedDamageThisAttack = false;

                                    if(x_rand()%2)
                                        m_AnimName  = "attack1"; //0
                                    else
                                        m_AnimName  = "attack3";
                                }
                            }
                            else
                            {

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
                                    m_AnimName = "evade2";                                                      

                                }
                                else if (angleToPlayer < -1.2f && angleToPlayer > -1.7f)
                                {
                                    tempnpc->LookAtPoint(newSpot, angleToPlayer  );
                                    m_AnimName = "evade1";                                                      

                                }
                            }
                        }
                        else 
                        {
                            // else, let's just run around like a fool for no real reason
                            vector3 newSpot = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                            m_AnimName = "jog";    //10
                            tempnpc->LookAtPoint(newSpot);

                        }
                        m_TimesShotSinceEvade = 0;
                        
                    } */
                }
                else
                {
                        vector3 newSpot = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                        m_AnimName = "run";    //10
                        tempnpc->LookAtPoint(newSpot);

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
void mutant_ai::AIStateAware ( f32 DeltaTime )
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

    xbool  isAnimAtEnd = tempnpc->GetAnimPlayer()->IsAtEnd( );
    s32    currentAnim = tempnpc->GetAnimPlayer()->GetAnimIndex( );

    //  Time do do some logic checking for mode switching

    //  If he is out of our awareness range, drop into Idle mode
    if(  distanceToPlayer > m_AwarenessRange    )
    {
        SetState( STATE_IDLE );
        m_npcHeard = false;
        return;
    }

    //  If we spot him, go to Combat mode
    if(m_npcSpotted)
    {
        m_TurnDirection = STATE_DIRECTION_NONE;
        SetState( STATE_ATTACKING );
        return;
    }


    // if we aren't at the end of the anim, just return and wait for next anim
    if( !isAnimAtEnd )
    {
        return;
    }

    //  If we can hear him and our current anim is at the end, figure out which
    //  way to turn to get to him and set our direction
    if ( m_npcHeard )
    {
        m_TimeTurnStarted = currentTime;
        if( tempnpc->IsPlayerToLeft() )
           m_TurnDirection = STATE_LEFT;
        else
           m_TurnDirection = STATE_RIGHT;

    }

 
    
    switch ( currentAnim )
    {
    case 24: //turn1                     "C:\3dsmax4\AAAAREA51\MUTANT\MUTANT_idle_turnleft180.MATX"
        {
            tempnpc->LookAtPlayer();
            m_AnimName = "idle2";
            return;
        }
        break;
    case 25: //turn2                     "C:\3dsmax4\AAAAREA51\MUTANT\MUTANT_idle_turnleft90.MATX"
        {
            tempnpc->LookAtPlayer();
            m_AnimName = "idle2";
            return;

        }
        break;
    case 26://turn3                     "C:\3dsmax4\AAAAREA51\MUTANT\MUTANT_idle_turnright180.MATX"
        {
            tempnpc->LookAtPlayer();
            m_AnimName = "idle2";
            return;

        }
        break;
    case 27://turn4                     "C:\3dsmax4\AAAAREA51\MUTANT\MUTANT_idle_turnright90.MATX"
        {
            tempnpc->LookAtPlayer();
            m_AnimName = "idle2";
            return;

        }
        break;
/*    case 9: //idle2                     "C:\GameData\A51\Source\Prototype\Character\NPC Mutant\anims\MUTANT_idle_confused.MATX"
        {
            if(m_WanderInIdle && x_frand(0.0f, 100.0f )  < m_ChanceToWander )
            {
                tempnpc->LookAtPoint( tempnpc->PickARandomSpot() );
                m_AnimName = "walk";

            }
            else
            {
                m_AnimName = "idle2"

                
            }
        }
        break; */


    }

    f32 angleToPlayer = tempnpc->GetAngleToPlayer();
    if( angleToPlayer > PI/2.0f || angleToPlayer < -PI/2.0f )
    {
        if(m_TurnDirection == STATE_LEFT )
        {
            m_AnimName = "Turn1";
        }
        else
        {
            m_AnimName = "Turn3";
        }
        return;

    }
    else
    {
        if(m_TurnDirection == STATE_LEFT )
        {
            m_AnimName = "turn2";
        }
        else
        {
            m_AnimName = "turn4";
        }
    }

  

    //  Is the current animation done?  If so then time to pick a new animation




}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void mutant_ai::AIStateFlee ( f32 DeltaTime )
{
  (void)DeltaTime;

    /*
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
*/

}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void mutant_ai::AIStateIdle ( f32 DeltaTime )
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
                m_AnimName = "idle3";//= 1;
                
            }
            break;

        default:
            {
                const s32 kRepeatIdle = 20;
                if( x_rand() %100 < kRepeatIdle )
                {
                    m_AnimName = "idle3";//  = 1;
                }
                else if (x_rand() %100 < m_ChanceToWander  &&  m_WanderInIdle )
                {
                   vector3 aPoint = tempnpc->PickARandomSpot(m_RandomMoveDistance);
                   tempnpc->LookAtPoint(aPoint);
                   m_AnimName = "walk";// = 13;
                }
                else
                {
                    
                    m_AnimName = "idle2";//= 1;
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
void mutant_ai::AIStateDead ( f32 DeltaTime )
{
    object* tempObject = g_ObjMgr.GetObjectBySlot(m_Actor);
    ASSERT(tempObject && tempObject->GetType() == object::TYPE_NPC );

    npc* tempnpc = (npc*)tempObject;

    ASSERT( tempnpc->GetAnimPlayer() );



    s32 currentAnim = tempnpc->GetAnimPlayer()->GetAnimIndex();

    (void)DeltaTime;
    if( tempnpc->GetAnimPlayer()->IsAtEnd( ) )
    {
        if(currentAnim == 1 || currentAnim == 0 || currentAnim == 33 )
        {
            tempnpc->PauseAnim();
        }
        else if ( currentAnim == 31 )  // Just finished first part of the grenade death
        {
            m_ExplodeDirection.Normalize();
            m_ExplodeDirection.Y += 1.0f;
            m_ExplodeDirection.Scale(600);;
            tempnpc->SetVelocity(m_ExplodeDirection);
            m_AnimName = "explode6";
        }
    }
    
    if( currentAnim == 32 && tempnpc->GetVelocity().LengthSquared() < 0.01f)
    {
        m_AnimName = "explode7";
        
    }


}





void mutant_ai::GotShot( slot_ID thisSlot, s32 Damage )
{
    (void)thisSlot;
    m_TimesShotSinceEvade += Damage;

    if( Damage > 40.0f )
        m_TimeToFlinch = true;

}




xbool mutant_ai::CheckForDamageUpdate( s32 thisAnim, f32 thisFrame, f32 distanceToPlayer )
{
    
    if( distanceToPlayer > 450.0f )
    {
        return false;
    }

    switch( thisAnim )
    {
    case 6: // claw attack combo
        {
            if(!m_AppliedDamageThisAttack)
            {
            
                if( thisFrame > 6.0f &&  thisFrame < 10.0f )
                {
                    m_AppliedDamageThisAttack = true;
                    return true;
                }
                else if( thisFrame > 16.0f &&  thisFrame < 20.0f )
                {
                    m_AppliedDamageThisAttack = true;
                    return true;
                }

            }                
            else if( thisFrame > 11.0f &&  thisFrame < 15.0f )
            {
                m_AppliedDamageThisAttack = false;
                return false;
            }


        }
        break;
    case 7: // leap attack
        {
            if(!m_AppliedDamageThisAttack)
            {
            
                if( thisFrame > 22.0f &&  thisFrame < 26.0f )
                {
                    m_AppliedDamageThisAttack = true;
                    return true;
                }
            }

        }
        break;
    case 8:
        {
            if(!m_AppliedDamageThisAttack)
            {
            
                if( thisFrame > 6.0f &&  thisFrame < 9.0f )
                {
                    m_AppliedDamageThisAttack = true;
                    return true;
                }
            }


        }
        break;
          


    }

    return false;

}
