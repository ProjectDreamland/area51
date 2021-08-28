///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_attack_aggressive.cpp
//
//      - implements an attack_aggressive state for an actor
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_State_attack.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"
#include "objects\npc.hpp"
#include "Audio\audio_stream_mgr.hpp"


char* ai_state_attack::m_FiringModeName[] =
{
    "SINGLE_SHOT_SLOW",
    "SINGLE_SHOT_MEDIUM",
    "SINGLE_SHOT_FAST",
    "THREE_SHOT_BURSTS",
    "NONSTOP"
};

ai_state_attack::ai_state_attack(brain* myBrain ):
    ai_state(myBrain),
    m_Target(0),
    m_MinTimeBetweenTargetSwitch(2.0f),
    m_TimeSinceTargetSwitch(5.0f),
    m_MinRangeToTarget(100.0f),
    m_MaxRangeToTarget(5000.0f),
    m_TimeSinceLastShot(0.0f),
    m_ShotsLeftToBurst(3),
    m_FiringMode( FIRING_MODE_SINGLE_SHOT_MEDIUM ),
    m_EmotionLevelToSwitchToFear(emotion_controller::EMOTION_LEVEL_ZERO ),
    m_EmotionLevelToSwitchToAngry(emotion_controller::EMOTION_LEVEL_ZERO ),
    m_MoveWhileFiring(1.0f),
    m_CombatMovement(false),
    m_MaxDistanceToRunForEvade(200.0f),
    m_FavorCover(0.0f),
    m_TimeToWatchTargetAfterLostLOS(1.0f),
    m_ChanceToMoveRelative(1.0f),
    m_MinTimeBetweenMoveChanges(1.0f),
    m_TimeSinceMoveChange(0.0f)

{



}


//=================================================================================================
ai_state_attack::~ai_state_attack()
{



}



//=================================================================================================
void ai_state_attack::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_attack::OnAdvanceLogic" );

    ai_state::OnAdvanceLogic(deltaTime);
    m_TimeSinceTargetSwitch += deltaTime;


    //  Re-evaluate our targets and decide if we need to switch
    if( m_TimeSinceTargetSwitch > m_MinTimeBetweenTargetSwitch )
    {
        EvaluateTargets();
        m_TimeSinceTargetSwitch = 0.0f;
    }
    UpdateRangeToTarget();
    
    emotion_controller& emotions  = m_Brain->GetEmotions();

    if( emotions.GetEmotionLevel( emotion_controller::EMOTION_BOREDOM ) >= m_EmotionLevelToSwitchToBored  )
    {
        m_Brain->RequestStateChange( m_ExitStateBored );
    }
    else if ( emotions.GetEmotionLevel( emotion_controller::EMOTION_ANGER ) >= m_EmotionLevelToSwitchToAngry  )
    {

        m_Brain->RequestStateChange( m_ExitStateAngry );
    }
    else if(GetTimeInState() > m_MaxTimeInState )
    {
        m_Brain->RequestStateChange( m_ExitStateNormal );

    }


    //  Update our destination
    UpdateDestination(deltaTime);

    UpdateWeaponFiring(deltaTime);

}



//=================================================================================================
void ai_state_attack::OnEnterState( void )
{
    ai_state::OnEnterState();

    // Turn the Idle off.
    if( g_AudioManager.IsPlaying( m_Brain->m_IdleVoiceID ) || g_AudioManager.IsStarting( m_Brain->m_IdleVoiceID ) )
        g_AudioManager.Stop( m_Brain->m_IdleVoiceID );


}


//=================================================================================================
xbool ai_state_attack::OnAttemptExit( void )
{


    return ai_state::OnAttemptExit();

}


//=================================================================================================
void ai_state_attack::OnExitState( void )
{
    m_CombatMovement = false;

    ai_state::OnExitState();
}



//=================================================================================================    
void ai_state_attack::OnInit( void )
{
    ai_state::OnInit();
    
    x_strcpy( m_ExitStateTargetLost, "None");
    x_strcpy( m_ExitStateScared, "None");
    x_strcpy( m_ExitStateAngry, "None");


}


//=================================================================================================
void ai_state_attack::EvaluateTargets( void )
{

    m_Target = m_Brain->EvaluateTargets();

    //  For attack states the default should be to look and aim at the same target
    m_Brain->SetAimAtObject( m_Target );
    m_Brain->SetLookAtObject( m_Target );
}


//=================================================================================================
//
//  Logic flow
//
//      if target is not visible
//          move to his last known spot
//      else if target is out of desired range
//          Move towards target
//      else if target is too close
//          Move away from target
//      else if
//
//
//
//
//
//
//
//
//
void ai_state_attack::UpdateDestination( f32 deltaTime )
{

    if(!m_CombatMovement && !m_Brain->IsNavPlanComplete() )
    {
        m_Brain->ClearDestination();
        m_CombatMovement = true;
    }
    else if( !m_Brain->IsNavPlanComplete() )
    {
        return;
    }

    if(m_TargetVisible)
    {
        m_TimeSinceTargetWasVisible = 0.0f;
 
    }
    
    if( !m_TargetVisible )
    {
        m_TimeSinceTargetWasVisible += deltaTime;
        if( m_Target!=0 && m_TimeSinceTargetWasVisible < m_TimeToWatchTargetAfterLostLOS )
        {

            vector3 chaseDestination;
            
            object* tempObject = g_ObjMgr.GetObjectByGuid(m_Target );
            ASSERT( tempObject );

            chaseDestination =  tempObject->GetPosition( );
//            if( chaseDestination.LengthSquared() == 0.0f )
//            {
//            }
//            else
            {
                m_Brain->SetNewDestination( chaseDestination );
                m_CombatMovement = true;
            }

        }
        else
        {
            m_Brain->RequestStateChange( m_ExitStateTargetLost );
        }


    }
    else if ( m_RangeToTarget > m_MaxRangeToTarget )
    {
        if( m_TargetVisible )
        {
            m_Brain->RequestStateChange( m_ExitStateTargetLost );
        }
//        else
        {
            AdvanceTowardsTarget();
            m_CombatMovement = true;

        }
        

    }
    else if (m_RangeToTarget < m_MinRangeToTarget)
    {
        PickANewCombatSpot( deltaTime );

    }
    else
    {

        m_TimeSinceMoveChange+= deltaTime;
    
        if( m_TimeSinceMoveChange < m_MinTimeBetweenMoveChanges)
        {
            return;
        }

        if ( x_frand(0.0f, m_MoveWhileFiring+deltaTime ) > m_MoveWhileFiring )
        {
            m_TimeSinceMoveChange = 0.0f;

            PickANewCombatSpot( deltaTime );
        }

    }

    
    
/*
    //  if we are too far away...
    if( m_RangeToTarget > m_MaxRangeToTarget )
    {
        // if we are both too far away and can't see him, time to switch states
        if( m_TargetVisible )
        {
            m_Brain->RequestStateChange( m_ExitStateTargetLost );
        }
        else
        {
            AdvanceTowardsTarget();
        }

    }
    //  else, if we are too close...
    else if( m_RangeToTarget < m_MinRangeToTarget )
    {
        //  hrmm... what to do if they are too close?
        //  for now keep on fighting and maybe find a new spot to move to
        PickANewCombatSpot(deltaTime);


    }
    //  else we are inside the desired range
    else
    { 
        //  if this is just left over movement from previous ai_state and there appears to be
        //  movement left to it then we clear the old movement
        if(!m_CombatMovement && !m_Brain->IsNavPlanComplete() )
        {
            m_Brain->ClearDestination();
        }

        if( m_Target!=0 && !m_TargetVisible )
        { 
            vector3 chaseDestination;

            chaseDestination = m_Brain->GetLastKnownPosition( m_Target );
            if( chaseDestination.LengthSquared() == 0.0f )
            {
                m_Brain->RequestStateChange( m_ExitStateTargetLost );
            }
            else
            {
                m_Brain->SetNewDestination( chaseDestination );
            }

        }

        
        //  else if our plan is complete
        else if( m_Brain->IsNavPlanComplete() )
        {
            if( x_frand(0.0f, m_MoveWhileFiring+deltaTime ) > m_MoveWhileFiring )
            {
                m_CombatMovement = true;
                PickANewCombatSpot(deltaTime);

            }

        }
        


    }
*/

}


//=================================================================================================
ai_state_attack::firing_mode ai_state_attack::GetFiringMode( void )
{
    return m_FiringMode;

}


//=================================================================================================
const char*  ai_state_attack::GetFiringModeString( firing_mode thisMode )
{
    ASSERT( thisMode < FIRING_MODE_LAST );

    return m_FiringModeName[thisMode];

}


//=================================================================================================
ai_state_attack::firing_mode ai_state_attack::GetFiringModeFromString( const char* thisMode )
{
    ASSERT( thisMode );
    u32 count;
    for( count = FIRING_MODE_SINGLE_SHOT_SLOW; count < FIRING_MODE_LAST; count++ )
    {
        if(!x_strcmp(thisMode, m_FiringModeName[count] ))
        {
            return (firing_mode)count;
        }
    }
    ASSERT(false);

    return FIRING_MODE_SINGLE_SHOT_SLOW;
}


//=================================================================================================
void ai_state_attack::SetFiringMode( firing_mode thisMode )
{
    ASSERT( thisMode < FIRING_MODE_LAST );

    m_FiringMode = thisMode;

}


//=================================================================================================
void ai_state_attack::SetFiringMode( const char* thisMode )
{
    ASSERT(thisMode);

    m_FiringMode = GetFiringModeFromString(thisMode);

}


//=================================================================================================
void ai_state_attack::UpdateRangeToTarget( void )
{

    // if we don't have a target return invalid distance
    if( m_Target == 0 )
    {
        m_RangeToTarget = -1.0f;
        return;
    }

    object* tempObject;
    tempObject = g_ObjMgr.GetObjectByGuid( m_Target );
    if(!tempObject)
    {
        m_RangeToTarget = -1.0f;
        return;
    }

    m_TargetVisible = m_Brain->IsObjectVisible(m_Target);
    m_RangeToTarget = (tempObject->GetPosition() - m_Brain->GetOwner()->GetPosition() ).Length();



}



//=================================================================================================
f32 ai_state_attack::GetRangeToTarget(void )
{
    return m_RangeToTarget;
}



//=================================================================================================
void ai_state_attack::AdvanceTowardsTarget(void)
{
    if(m_Target == 0 )
    {
        return;
    }
    object* tempObject = g_ObjMgr.GetObjectByGuid( m_Target );

    if(tempObject)
    {
        vector3 tempVector = tempObject->GetPosition();
        m_Brain->SetNewDestination(  tempVector );
    }
    else
    {
        //  should only get here if it has a guid but it is not valid
        ASSERT(false);

    }
    

}



//=================================================================================================
void    ai_state_attack::PickANewCombatSpot ( f32 deltaTime )
{
 
	
    vector3 pointToTry;
	(void)deltaTime;

    object* tempObject = NULL;
    if(m_Target != 0 )
    {
        tempObject = g_ObjMgr.GetObjectByGuid( m_Target );
        if( !tempObject )
        {
            m_Target = 0;
        }
    }
    if(tempObject == NULL )
    {
        return;
    }


    vector3 enemyPoint;
    enemyPoint = tempObject->GetBBox().GetCenter();

    if(m_FavorCover > 0.0  )//&&  ( x_frand(0.0f, 0.2f) < deltaTime  )   )
    {
        if(m_FavorCover > x_frand(0.0, 1.0f) )
        {
            vector3 thisSpot;
            
            if( m_Brain->FindCover(enemyPoint, thisSpot))
            {
                m_Brain->SetNewDestination( thisSpot );
                return;
            }
        }

    }

    base_loco::anim_type thisType = base_loco::AT_DEATH;

    s32 tries = 4;


    while (tries--)
    {
        pointToTry = m_Brain->GetOwner()->GetBBox().GetCenter();
        thisType = base_loco::AT_DEATH;
        if( x_frand( 0.0f, 1.0f ) < m_ChanceToMoveRelative || tries == 1 )
        {
            vector3 myPoint = m_Brain->GetOwner()->GetPosition();
            vector3 differenceAmount;
            differenceAmount = myPoint - enemyPoint;
            differenceAmount.Y = 0.0f;

            if(x_rand()&0x00000001)
            {
                differenceAmount.RotateY(PI*0.5f);
//                thisType   = base_loco::AT_EVADE_RIGHT;
//                m_Brain->GetOwner()->GetLocomotion()->PlayAnimState( base_loco::AT_EVADE_RIGHT );
            }
            else
            {
                differenceAmount.RotateY(PI * -0.5f);
                thisType   = base_loco::AT_EVADE_LEFT;
//                m_Brain->GetOwner()->GetLocomotion()->PlayAnimState( base_loco::AT_EVADE_LEFT );
            }
            differenceAmount.Normalize();

//            differenceAmount.Scale( 100.0f + x_frand( 0, m_MaxDistanceToRunForEvade ) );
            differenceAmount.Scale( 100.0f + x_frand( 0, 300.0f ) );
            pointToTry.X += differenceAmount.X;
            pointToTry.Z += differenceAmount.Z;
            
            
        }
        else
        {
        
            pointToTry.X += 100.0f + x_frand( -m_MaxDistanceToRunForEvade, m_MaxDistanceToRunForEvade );
            pointToTry.Z += 100.0f + x_frand( -m_MaxDistanceToRunForEvade, m_MaxDistanceToRunForEvade );
        }



        if( (pointToTry- enemyPoint).LengthSquared() > m_MinRangeToTarget *m_MinRangeToTarget ||
            (enemyPoint- pointToTry).LengthSquared()  > (enemyPoint-m_Brain->GetOwner()->GetPosition() ).LengthSquared() )
        {
            g_CollisionMgr.LineOfSightSetup( m_Brain->GetOwner()->GetGuid(), pointToTry, m_Brain->GetOwner()->GetBBox().GetCenter() );
            g_CollisionMgr.CheckCollisions();
        
            //  if zero collisions then we can see this thing
            if(      g_CollisionMgr.m_nCollisions  == 0 ||  
                   ( g_CollisionMgr.m_nCollisions  == 1 && g_CollisionMgr.m_Collisions[0].ObjectHitGuid == m_Brain->GetOwner()->GetGuid() ) )
            {
                vector3 pointToTryTop,
                        enemyPointTop;
                pointToTryTop = pointToTry;
                enemyPointTop = m_Brain->GetOwner()->GetPosition();
                pointToTryTop.Y += 180.0f;
                enemyPointTop.Y += 180.0f;
                g_CollisionMgr.CylinderSetup(    m_Brain->GetOwner()->GetGuid(),
                                                    m_Brain->GetOwner()->GetPosition(),
                                                    pointToTry,
                                                    enemyPointTop, 
                                                    pointToTryTop, 
                                                    50.0f );
                
                g_CollisionMgr.CheckCollisions();
                //  If we can see the point and get to the point, can we see our target from the point
                if( g_CollisionMgr.m_nCollisions == 0 )
                {

                    g_CollisionMgr.LineOfSightSetup( m_Brain->GetOwner()->GetGuid(), pointToTry, enemyPoint );
                    g_CollisionMgr.CheckCollisions();
                    
                    if(thisType == base_loco::AT_EVADE_RIGHT)
                    {
//                        m_Brain->GetOwner()->GetLocomotion()->PlayAnimState( base_loco::AT_EVADE_RIGHT );
                    }
                    else if (thisType == base_loco::AT_EVADE_LEFT)
                    {
//                        m_Brain->GetOwner()->GetLocomotion()->PlayAnimState( base_loco::AT_EVADE_LEFT );
                    }
                        
                    m_Brain->SetNewDestination( pointToTry );
                    return;
                }

            }
        
        }


    }



}









const f32 slowShootingTime      =  5.0f;
const f32 mediumShootingTime    =  3.0f;
const f32 fastShootingTime      =  1.5f;
const f32 burstShootingTime     =  6.0f;
const f32 nonstopShootingTime   =  0.0f;


//=================================================================================================
void ai_state_attack::UpdateWeaponFiring(f32 deltaTime)
{
    
    if(m_Brain->GetOwner()->GetLocomotion()->GetCurrentState() == locomotion::STATE_SPECIAL )
    {
        return;
    }

    m_TimeSinceLastShot += deltaTime;

    if(!m_TargetVisible)
    {
        return;
    }

    switch(m_FiringMode )
    {
        case FIRING_MODE_SINGLE_SHOT_SLOW:
            {
                if( m_TimeSinceLastShot > slowShootingTime )
                {
                    if( m_Brain->AttemptShot() )
                    {
                        m_TimeSinceLastShot = 0.0f ;
                    }
                }

            }
            break;
        case FIRING_MODE_SINGLE_SHOT_MEDIUM:
            {
                if( m_TimeSinceLastShot > mediumShootingTime)
                {
                    if( m_Brain->AttemptShot() )
                    {
                        m_TimeSinceLastShot = 0.0f ;
                    }
                }


            }
            break;
        case FIRING_MODE_SINGLE_SHOT_FAST:
            {
                if( m_TimeSinceLastShot > fastShootingTime)
                {
                    if( m_Brain->AttemptShot() )
                    {
                        m_TimeSinceLastShot = 0.0f ;
                    }
                }

            }
            break;
        case FIRING_MODE_THREE_SHOT_BURSTS:
            {
                if( m_TimeSinceLastShot > burstShootingTime )
                {
                    if( m_Brain->AttemptShot() )
                    {
                        m_ShotsLeftToBurst-- ;
                        if(!m_ShotsLeftToBurst )
                        {
                            m_TimeSinceLastShot= 0.0f;
                            m_ShotsLeftToBurst = 3;
                        }
                    }
                }

            }
            break;
        case FIRING_MODE_NONSTOP:
            {
                if( m_TimeSinceLastShot > nonstopShootingTime)
                {
                    if( m_Brain->AttemptShot() )
                    {
                        m_TimeSinceLastShot = 0.0f ;
                    }
                }

            }
            break;

        default:
            ASSERT(false);

    }
    


}





//=================================================================================================
xbool ai_state_attack::AttemptShot( void )
{
    if( m_Brain->GetOwner()->IsReadyToShoot() )
    {
        object* tempObject = g_ObjMgr.GetObjectByGuid( m_Brain->GetAimAtObject() );

        ASSERT(tempObject);
        ASSERT(m_Brain->GetOwner()->GetGuid() != 0 );
        
        g_CollisionMgr.LineOfSightSetup( m_Brain->GetOwner()->GetGuid(), m_Brain->GetOwner()->GetPosition() , tempObject->GetPosition() );
        g_CollisionMgr.CheckCollisions();

        //  if zero collisions then we can see this thing
        if( (g_CollisionMgr.m_nCollisions ) )
        {
            return false;

        }

    }


    if( m_Brain->AttemptShot() )
    {

        return TRUE;

    }

    return false;
}


//=================================================================================================
void ai_state_attack::UpdateLookAtObject( f32 deltaTime )
{
    // if we don't have a target or if we haven't seen our target lately then we call the 
    // base ai_state's UpdateLookAtObject function  else we look at our target
    if(m_Target == 0)
    {
        ai_state::UpdateLookAtObject(deltaTime);
    }
//    else if( m_Brain->GetTimeSinceObjectSeen(m_Target ) > m_TimeToWatchTargetAfterLostLOS )
//    {
//        ai_state::UpdateLookAtObject(deltaTime);

//    }
    else
    {
        m_Brain->SetLookAtObject(m_Target);
    }


}




guid ai_state_attack::GetCurrentTarget( void )
{
    return m_Target;
}











///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_attack::OnEnumProp( prop_enum&  List )
{
    ai_state::OnEnumProp( List );

    List.AddFloat(  xfs("AI State - %s\\Min Time Between Target Switch",m_CustomName), 
                    "Minimum time between a target switch" );

    List.AddFloat(  xfs("AI State - %s\\Time Since Target Switch",m_CustomName), 
                    "Time since target Switch",
                    PROP_TYPE_READ_ONLY );
    
    List.AddEnum(   xfs("AI State - %s\\Firing Mode", m_CustomName),
                    "SINGLE_SHOT_SLOW\0SINGLE_SHOT_MEDIUM\0SINGLE_SHOT_FAST\0THREE_SHOT_BURSTS\0NONSTOP\0",
                    "The firing mode for this AI state");

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_ANGRY", m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_TARGET_LOST", m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );
    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_SCARED", m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_SCARED level needed", m_CustomName),
                    "EMOTION_LEVEL_ZERO\0EMOTION_LEVEL_MINIMAL\0EMOTION_LEVEL_MODERATE\0EMOTION_LEVEL_HIGH\0EMOTION_LEVEL_EXTREME\0EMOTION_LEVEL_IMPOSSIBLE\0",
                    "Level of fear required for this AI to call the exit scared method",
                    PROP_TYPE_MUST_ENUM   );
   
    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_ANGRY level needed", m_CustomName),
                    "EMOTION_LEVEL_ZERO\0EMOTION_LEVEL_MINIMAL\0EMOTION_LEVEL_MODERATE\0EMOTION_LEVEL_HIGH\0EMOTION_LEVEL_EXTREME\0EMOTION_LEVEL_IMPOSSIBLE\0",
                    "Level of fear required for this AI to call the exit angry method",
                    PROP_TYPE_MUST_ENUM   );

    List.AddFloat(  xfs("AI State - %s\\Min Distance to Target",m_CustomName), 
                    "Minimum distance to the target for it to attack.  If closer than this, the AI will try to move further back or switch AI states"  );

    List.AddFloat(  xfs("AI State - %s\\Max Distance to Target",m_CustomName), 
                    "Max distance  to the target for it to attack.  If farther away than this, the AI will try to move closer or switch AI states." );

    List.AddFloat(  xfs("AI State - %s\\Move while firing",m_CustomName), 
                    "How much does the NPC move around while firing, 0 is always, higher is less frequently." );

    List.AddFloat(  xfs("AI State - %s\\Favor Cover",m_CustomName), 
                    "How much does he favor a spot with cover" );

    List.AddFloat(  xfs("AI State - %s\\Distance to Run for Evade",m_CustomName), 
                    "How far will he move at once in combat" );

    List.AddFloat(  xfs("AI State - %s\\Time to watch target after LOS lost",m_CustomName), 
                    "How long should we look towards a target after we lose our line of sight" );

    List.AddFloat(  xfs("AI State - %s\\Chance to move relative",m_CustomName), 
                    "Chance to move relative to target instead of random.  Zero never, 1.0 always " );
    
    


}


xbool ai_state_attack::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);

    if( I.IsVar( xfs("AI State - %s\\Min Time Between Target Switch",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MinTimeBetweenTargetSwitch);
        }
        else
        {   
            m_MinTimeBetweenTargetSwitch = I.GetVarFloat();
        }

        return TRUE;
    }
 

    if( I.IsVar( xfs("AI State - %s\\Time Since Target Switch",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_TimeSinceTargetSwitch);
        }
        else
        {   
            ASSERT(false);
        }

        return TRUE;
    }

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_TARGET_LOST",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateTargetLost );

        }
        else
        {
            x_strcpy(m_ExitStateTargetLost, I.GetVarEnum() );

        }
        return true;
    }

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_ANGRY",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateAngry );

        }
        else
        {
            x_strcpy(m_ExitStateAngry, I.GetVarEnum() );

        }
        return true;
    }


     if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_SCARED",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateScared );

        }
        else
        {
            x_strcpy(m_ExitStateScared, I.GetVarEnum() );

        }
        return true;
    }
 
    if( I.IsVar( xfs("AI State - %s\\Min Distance to Target",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MinRangeToTarget);
        }
        else
        {   
            m_MinRangeToTarget = I.GetVarFloat();
        }

        return TRUE;
    }

    if( I.IsVar( xfs("AI State - %s\\Max Distance to Target",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MaxRangeToTarget);
        }
        else
        {   
            m_MaxRangeToTarget = I.GetVarFloat();
        }

        return TRUE;
    }

    if( I.IsVar( xfs("AI State - %s\\Firing Mode", m_CustomName ) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarEnum( GetFiringModeString( m_FiringMode ) );

        }
        else
        {
            
            SetFiringMode( I.GetVarEnum() );

        }
        return true;
    }
    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_SCARED level needed", m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( emotion_controller::GetEmotionLevelString(m_EmotionLevelToSwitchToFear) );

        }
        else
        {
            m_EmotionLevelToSwitchToFear = emotion_controller::GetEmotionLevelFromString( I.GetVarEnum() );
        }
        return true;
    }

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_ANGRY level needed", m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( emotion_controller::GetEmotionLevelString(m_EmotionLevelToSwitchToAngry) );

        }
        else
        {
            m_EmotionLevelToSwitchToAngry = emotion_controller::GetEmotionLevelFromString( I.GetVarEnum() );
        }
        return true;
    }

    if( I.IsVar( xfs( "AI State - %s\\Move while firing", m_CustomName ) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MoveWhileFiring);
        }
        else
        {   
            m_MoveWhileFiring = I.GetVarFloat();
        }

        return TRUE;
    }


    if( I.IsVar( xfs("AI State - %s\\Favor Cover",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_FavorCover);
        }
        else
        {   
            m_FavorCover = I.GetVarFloat();
        }

        return TRUE;
    }    



    if( I.IsVar( xfs("AI State - %s\\Distance to Run for Evade",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MaxDistanceToRunForEvade);
        }
        else
        {   
            m_MaxDistanceToRunForEvade = I.GetVarFloat();
        }

        return TRUE;
    }    


    if( I.IsVar( xfs("AI State - %s\\Time to watch target after LOS lost",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_TimeToWatchTargetAfterLostLOS);
        }
        else
        {   
            m_TimeToWatchTargetAfterLostLOS = I.GetVarFloat();
        }

        return TRUE;
    }    

    if( I.IsVar( xfs("AI State - %s\\Chance to move relative",m_CustomName) ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_ChanceToMoveRelative);
        }
        else
        {   
            m_ChanceToMoveRelative = I.GetVarFloat();
        }

        return TRUE;
    }    



    return returnVal;

}



