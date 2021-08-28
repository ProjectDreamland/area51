///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  brain.cpp
//
//      -the brain object manages an actors AI states, its virtual sensors, and its emotions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Brain.hpp"
#include "navigation\NavNodeMgr.hpp"
#include "ai_states\ai_state_idle.hpp"
#include "ai_states\ai_state_attack_passive.hpp"
#include "ai_states\ai_state_attack_normal.hpp"
#include "ai_states\ai_state_attack_aggressive.hpp"
#include "ai_states\ai_state_flee.hpp"
#include "ai_states\ai_state_curious.hpp"
#include "ai_states\ai_state_dead.hpp"
#include "ai_states\ai_state_move_to.hpp"
#include "ai_states\ai_state_follow.hpp"
#include "ai_states\ai_state_patrol.hpp"
#include "ai_states\ai_state_grabbed.hpp"

#include "..\objects\object.hpp"
#include "..\objects\npc.hpp"
#include "..\objects\player.hpp"

//=================================================================================================
brain::brain(npc* newOwner) :
    m_CurrentState(NULL),
    m_DesiredState(NULL),
    m_StateToDelete(NULL),
    m_TimeBetweenSensoryUpdates(1.0f),
    m_TimeSinceLastSensoryUpdate(0.0f),
    m_RecognitionMemoryLength(15.0f),
    m_MaxSearchRangeForCover(200.0f),
    m_Owner(newOwner),
    m_NewContactHostile(false),
    m_DestinationReached(true),
    m_LastNodeReached(true),
    m_LookAtDestination(true),
    m_MoveDisabled(false),
    m_LookAtObject(0),
    m_AimAtObject(0),
    m_TimeSinceLastMove(0.0f),
    m_MinimumDistanceToConsiderMovementActive(30.0f),
    m_TimeTillReThinkPath(0.6f),
    m_TimeTillCancelPath(1.0f),
    m_EyeLevel(130.0f),
    m_CoverLevel(60.0f),
    m_LookThisFarAhead(300.0f),
    m_BoneIndexForWeapon(-1)
{
    m_RecognitionRangeVisual = 1000.0f;
    m_AIName[0] = 0;
    m_StartingAIName[0] = 0;
    
    s32 count;
    for(count= 0; count < kMAX_WEAPONS; count++)
    {
        m_Weapons[ count ] = NULL;
    }
    
    m_SpecOpType        = x_irand( 1, 3 );
    m_AlertVoiceID      = 0;
    m_ManDownVoiceID    = 0;   
    m_SpotStage5VoiceID = 0;
    m_IdleVoiceID       = 0;      
}


//=================================================================================================
brain::~brain()
{



}
    
//=================================================================================================
void brain::Init(void)
{
    m_CurrentState = NULL;
    m_AIStates.Clear();
    m_Emotions.Init();

    s32 count;
    for( count = 0; count < MAX_SENSORY_RECORDS; count++)
    {
        
        m_Records[count].m_TimeOfContact  = 0.0f;
        m_Records[count].m_PointOfContact = vector3(0.0f, 0.0f, 0.0f);
        m_Records[count].m_TypeOfContact  = SENSORY_TYPE_LINE_OF_SIGHT;
        m_Records[count].m_ContactObject  = 0;
    }

    // small hack to make sure move to and current position are not the same
    vector3 aVector = m_Owner->GetPosition();
    aVector.X += 1.0f;

    SetNewDestination( aVector );

    static f32 startingTime = 0.0f;
    m_TimeSinceLastMove = startingTime;
    m_TimeSinceLastSensoryUpdate = startingTime + 0.3f;
    startingTime += 0.08f;


    x_strcpy( m_AIName, m_StartingAIName );

    
    m_CurrentState = GetStateByName( m_AIName );
    if(m_CurrentState)
    {
        m_CurrentState->OnEnterState();
    }



}


//=================================================================================================
void brain::SetNewDestination( vector3& newDestination )
{
//    m_LastPointDuringMove = m_Owner->GetPosition();
    m_NavPlan.SetDestination(newDestination);
    UpdateNavPlan();
}


//=================================================================================================
void brain::UpdateNavPlan( void )
{
    if(m_NavPlan.GetCompletePath() == false)
    {
        vector3 tempVec;
        tempVec  = m_Owner->GetEyePosition();
        g_NavMgr.UpdatePlan(tempVec , (base_plan*)&m_NavPlan );
        m_DestinationReached = false;
        m_LastNodeReached = false;
 
    }
    else
    {
//        m_DestinationReached = true;
//        m_LastNodeReached = true;
    }

  
   
}


//=================================================================================================
nav_plan& brain::GetNavPlan( void )
{
    return m_NavPlan;
}



//=================================================================================================
void brain::SetLookAtObject( guid thisGuid )
{
    m_LookAtDestination = false;
    m_LookAtObject = thisGuid;
        
}

//=================================================================================================
void brain::SetLookAtDestination(void)
{

    m_LookAtDestination = true;
    m_LookAtObject = 0;

}


//=================================================================================================
void brain::SetLookAtPoint(vector3 thisPoint)
{
    m_LookAtDestination = false;
    m_LookAtPoint=  thisPoint ;
    m_LookAtObject = 0;
    

}
 

//=================================================================================================
vector3 brain::GetCurrentLookAtVector( void )
{
    if( m_LookAtDestination )
    {
        vector3 thisPoint = GetPointOnPath( m_LookThisFarAhead );
        thisPoint.Y = GetEyeLevel();
    
        return thisPoint;

    }
    else if( m_LookAtObject == 0 )
    {
        return m_LookAtPoint;
    }
    else
    {       
        object* tempObject = g_ObjMgr.GetObjectByGuid(m_LookAtObject );
        if( tempObject )
        {
            if(tempObject->GetType() == object::TYPE_NPC )
            {
                npc &tempNPC = npc::GetSafeType(*tempObject);
                return tempNPC.GetEyePosition();
            }
            else if( tempObject->GetType() == object::TYPE_PLAYER )
            {
                player  &tempPlayer = player::GetSafeType(*tempObject);
                return tempPlayer.GetEyesPosition();
//                return tempObject->GetPosition();               

            }
            else 
            {
                return tempObject->GetPosition();

            }
            
        }
        else
        {
            //  Our look at object is Null!  Should never happen
//            ASSERT(FALSE);

            //  this is really just here to prevent warnings
            return m_LookAtPoint;

        }


    }
    
}

//=================================================================================================
guid brain::GetLookAtObject( void )
{
    return m_LookAtObject;
}




//=================================================================================================
void brain::SetAimAtObject( guid thisGuid )
{
    m_AimAtObject = thisGuid;

}



//=================================================================================================
guid brain::GetAimAtObject( void )
{

    return m_AimAtObject;

}


//=================================================================================================
void brain::SetCurrentAIState( ai_state* thisAIState )
{
    m_CurrentState = thisAIState;
    if (m_CurrentState)
        m_CurrentState->OnEnterState();
    m_NewContactHostile = false;    

}


//=================================================================================================
void brain::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "brain::OnAdvanceLogic" );

    if(!m_Owner->GetLocomotion())
        return;

    //   If someone has requested a state change
    if( m_DesiredState && m_DesiredState != m_CurrentState )
    {
        // Is there a current state?
        if (m_CurrentState)
        {
            //  See if the current state can exit
            if( m_CurrentState->OnAttemptExit() )
            {
                //   if it can exit then call the exit for the old one and the enter for the new one 
                //   and then make the desired the current and clear the desired
                m_CurrentState->OnExitState();
                m_DesiredState->OnEnterState();
                m_CurrentState = m_DesiredState;
                m_DesiredState = NULL;

            }
        }
        else
        {
            // Just switch
            m_DesiredState->OnEnterState();
            m_CurrentState = m_DesiredState;
        }
    }

    m_TimeSinceLastSensoryUpdate += deltaTime;
    UpdateSenses(deltaTime);

    //
    // Advance the Logic 
    //
    if( m_CurrentState )
    {
        m_CurrentState->OnAdvanceLogic( deltaTime );
    }

    UpdateWander( deltaTime );
    UpdateNavPlan();

//    if( m_LookAtDestination )
//    {
//        UpdateLookAtDestination();
//    }


    //  Time to update our look at object.  This should be the only place that the locomotion
    //  Look at is ever called!!!
    vector3 tempVector;
    tempVector = GetCurrentLookAtVector();

    m_Owner->GetLocomotion()->SetLookAt( tempVector );


    UpdateMovement( deltaTime );
    

}


//=================================================================================================
void brain::OnDamaged( s32 Damage, vector3* thisDirection )
{

    if( m_CurrentState )
    {
        m_CurrentState->OnDamaged(Damage, thisDirection );
    }

}




//=================================================================================================
void brain::UpdateSenses( f32 deltaTime )
{
    (void)deltaTime;
    ASSERT( m_Owner );

    //  We only update sensory info periodically.  Check the time since update and return if needed
    if( m_TimeBetweenSensoryUpdates > m_TimeSinceLastSensoryUpdate )
        return;

    //  Reset the time since we updated
    m_TimeSinceLastSensoryUpdate = 0.0f;
    
    f32 currentTime = (f32)x_GetTimeSec();

    s32 count;
    for (count =0; count < MAX_SENSORY_RECORDS; count++ )
    {
        if( m_Records[count].m_Enemy != 0 && m_Records[count].m_TimeOfContact + m_RecognitionMemoryLength < currentTime )
        {
            m_Records[count].m_ContactObject = 0;
            m_Records[count].m_Enemy = 0;
        }

    }

    


    // Updating Senses will be done in several steps

    //  We have to keep a temporary list of ids because object manager can't handle a search within 
    //  a search and we need to do a search for objects in range and then also a LOS check on those
    //  objects.
    const s32 kMAX_CONTACTS = 32;
    slot_id idList[kMAX_CONTACTS];
    s32 contactCount = 1;

    bbox aBbox;
    aBbox = m_Owner->GetBBox();
    aBbox.Inflate( m_RecognitionRangeVisual, m_RecognitionRangeVisual, m_RecognitionRangeVisual);

//    g_SpatialDBase.TraverseCells( aBbox );
/*
    g_ObjMgr.SelectBBox( object::ATTR_ALL , aBbox ,object::TYPE_PLAYER );
    
    while( aID != SLOT_NULL && contactCount < kMAX_CONTACTS )
    {
        if(aID != m_Owner->GetSlotID() )
        {
            idList[contactCount]= aID;
            ++contactCount;
        }
        aID = g_ObjMgr.GetNextResult( aID );

    }
    g_ObjMgr.EndLoop();
*/

    slot_id playerID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    
    if(playerID != SLOT_NULL )
    {
        object *tempPlayer = g_ObjMgr.GetObjectBySlot( playerID );
        if( tempPlayer )
        {   
            idList[contactCount]= playerID;
            ++contactCount;                                    
        }
    }

    slot_id aID;// = g_ObjMgr.StartLoop();

    g_ObjMgr.SelectBBox( object::ATTR_ALL , aBbox ,object::TYPE_NPC );
    
    aID = g_ObjMgr.StartLoop();
    while( aID != SLOT_NULL && contactCount < kMAX_CONTACTS )
    {
        if(aID != m_Owner->GetSlotID() )
        {
            idList[contactCount]= aID;
            ++contactCount;
        }
        aID = g_ObjMgr.GetNextResult( aID );

    }
    g_ObjMgr.EndLoop();


    while( --contactCount)
    {
        object* tempObject = g_ObjMgr.GetObjectBySlot(idList[contactCount] );
        if( tempObject->GetType() == object::TYPE_NPC || tempObject->GetType() == object::TYPE_PLAYER )
        {
            xbool isFriend = false;
            if(tempObject->GetType() == object::TYPE_NPC )
            {
                ASSERT(tempObject); 
                actor &tempActor = actor::GetSafeType( *tempObject );
                isFriend = tempActor.IsFriend( m_Owner ) ;

            }
            else
            {
                switch(m_Owner->GetFaction() )
                {
                case actor::FACTION_GOOD_GUYS:
                    isFriend = true;
                    break;
                case actor::FACTION_GREY:
                    isFriend = true;
                    break;
                case actor::FACTION_NEUTRAL:
                    isFriend = true;
                    break;
                default:
                    isFriend = false;
                    break;

                }
                

            }

            if( !isFriend )
            {
            

                vector3 fromHere = m_Owner->GetPosition();
            
    //            vector3 toHere = tempObject->GetPosition();
                vector3 toHere = tempObject->GetBBox().GetCenter();

                if((fromHere - toHere).LengthSquared() > m_RecognitionRangeVisual*m_RecognitionRangeVisual)
                {
                    return;
                }
                fromHere.Y = GetEyeLevel();
//                toHere.Y = GetEyeLevel();

                g_CollisionMgr.m_nMaxCollsions = 3;
                g_CollisionMgr.LineOfSightSetup( m_Owner->GetGuid(), fromHere, toHere );
                g_CollisionMgr.CheckCollisions();
                //PRINT_COLLISION_INFO( "brain::UpdateSenses",fromHere, toHere ); 
                g_CollisionMgr.m_nMaxCollsions = 8;


                
                
                s32 collisionCounter;
                s32 numberOfRealCollisions = 0;

                for( collisionCounter = 0; collisionCounter < g_CollisionMgr.m_nCollisions; collisionCounter++ )
                {
                    if ( g_CollisionMgr.m_Collisions[collisionCounter].ObjectHitGuid != tempObject->GetGuid() )
                    {
                        numberOfRealCollisions++;
                    }

                }


                //  if zero collisions then we can see this thing
                if( numberOfRealCollisions == 0 )
                {
                    AddSensorRecord( toHere,tempObject->GetGuid(), SENSORY_TYPE_LINE_OF_SIGHT );
                }
            }
        }

    }

}



//=================================================================================================
void brain::AddSensorRecord( vector3& thisPoint,guid thisGuid, type_of_contact thisType)
{
    ASSERT(g_ObjMgr.GetObjectByGuid(thisGuid ) );

    object* tempObject = g_ObjMgr.GetObjectByGuid(thisGuid);
    ASSERT(tempObject);
//    actor &tempActor = actor::GetSafeType( *tempObject );

    //  First step is see if this object already has a record in the list
    s32 count;

    for( count = 0; count <MAX_SENSORY_RECORDS; count++)
    {
        if( m_Records[count].m_ContactObject == thisGuid )
        {
            m_Records[count].m_TimeOfContact  = (f32)x_GetTimeSec(); 
            m_Records[count].m_PointOfContact = thisPoint;
            m_Records[count].m_TypeOfContact  = thisType;
            return;
        }

    }


    xbool isFriend = false;
    if(tempObject->GetType() == object::TYPE_NPC )
    {
        ASSERT(tempObject); 
        actor &tempActor = actor::GetSafeType( *tempObject );
        isFriend = tempActor.IsFriend( m_Owner ) ;

    }
    else
    {
        switch(m_Owner->GetFaction() )
        {
        case actor::FACTION_GOOD_GUYS:
            isFriend = true;
            break;
        case actor::FACTION_GREY:
            isFriend = true;
            break;
        case actor::FACTION_NEUTRAL:
            isFriend = true;
            break;
        default:
            isFriend = false;
            break;

        }
        

    }

    if( !isFriend )
    {
        m_NewContactHostile = true;
    }


    //  else if not already in the list, let's find the oldest record and replace it
    s32 whichSlot = 0;
    f32 oldestTime = m_Records[0].m_TimeOfContact ;
    for (count = 1; count < MAX_SENSORY_RECORDS; count++ )
    {
        if( m_Records[count].m_TimeOfContact < oldestTime )
        {
            whichSlot = count;
            oldestTime = m_Records[count].m_TimeOfContact ;
        }
    }

    m_Records[whichSlot].m_TimeOfContact  = (f32)x_GetTimeSec();
    m_Records[whichSlot].m_PointOfContact = thisPoint;
    m_Records[whichSlot].m_TypeOfContact  = thisType;
    m_Records[whichSlot].m_ContactObject  = thisGuid;
    m_Records[whichSlot].m_Enemy          = m_NewContactHostile;

   
    
    
}


//=================================================================================================

ai_state* brain::GetStateByName(const char* thisName )
{
    ASSERT( thisName );
    s32 count;
    for( count = 0; count < m_AIStates.GetCount(); count++ )
    {
        if(! x_strcmp(thisName, m_AIStates[count]->GetName( ) ) )
        {
            return m_AIStates[count];

        }

    }

    return NULL;

}


//=================================================================================================
void brain::RequestStateChange( ai_state* thisState )
{
    m_DesiredState = thisState;

}


//=================================================================================================
void brain::RequestStateChange( const char* thisState )
{
    ai_state* newState;
    newState = GetStateByName( thisState);
//    ASSERT(newState);

    m_DesiredState = newState;

}



//=================================================================================================
sensory_record* brain::GetMostRecentContact( xbool enemyOnly )
{
    s32 count;
    f32 mostRecentTime;
    s32 mostRecentIndex;
    
    mostRecentTime = m_Records[0].m_TimeOfContact;
    mostRecentIndex = -1;


    for (count =1; count < MAX_SENSORY_RECORDS ; count++ )
    {
        if( !enemyOnly || ( enemyOnly && m_Records[count].m_Enemy  ) )
        {
            if(m_Records[count].m_TimeOfContact > mostRecentTime )        
            {
                mostRecentTime = m_Records[count].m_TimeOfContact;
                mostRecentIndex = count;
            }
        }

    }

    if( mostRecentIndex == -1 )
    {
        return NULL;
    }


    return ( &(m_Records[mostRecentIndex]) );



}

//=================================================================================================
//
//  Logic Flor for UpdateMovement
//
//      if we can see our final destination
//          move towards our final destination
//      else if we can see our next next node + 1
//          move towards next node +1
//      else if we can see our next node
//          move towards next node
//      else
//          something went wrong and go into fall back mode
//
//
//
//
//=================================================================================================
xbool brain::UpdateMovement( f32 deltaTime )
{
    if(m_MoveDisabled)
    {
        return false;
    }

    m_TimeSinceLastMove += deltaTime;
    
    if(m_TimeSinceLastMove < 0.20f )
        return true;

    m_TimeSinceLastMove = 0.0f;

//    vector3 tempDestination;
 
    vector3 startBottom,  
            startTop, 
            endBottom, 
            endTop ;
    
    s32 m_PreviousMaxCollisions = g_CollisionMgr.m_nMaxCollsions;    
    g_CollisionMgr.m_nMaxCollsions = 3;
    //
    //  First we test to see if we can see our final destination.  If we can, head straight there
    //
    
    startBottom = m_Owner->GetLocomotion()->GetPosition();
    endBottom   = m_NavPlan.GetDestination();
    startTop    = startBottom;
    endTop      = endBottom;
    startBottom.Y += 30.0f;
    endBottom.Y += 30.0f;
    startTop.Y  += 180.0f;
    endTop.Y    += 180.0f;



    if((startBottom-endBottom).LengthSquared() < 2000.0f*2000.0f )
    {
    

        //  FIXME:  need to query for real height and radius


        g_CollisionMgr.CylinderSetup(   m_Owner->GetGuid(),
                                        startBottom,
                                        endBottom,
                                        startTop,
                                        endTop,
                                        30.0f   );
        g_CollisionMgr.SetMaxCollisions(3);

    //    g_CollisionMgr.LineOfSightSetup(    m_Owner->GetGuid(),  
    //                                        m_Owner->GetLocomotion()->GetPosition(), 
    //                                        m_NavPlan.GetDestination()  );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );

        //PRINT_COLLISION_INFO( "brain::UpdateMovement1",startTop, endTop );
        
    }
    else
    {
        g_CollisionMgr.m_nCollisions = 1;
    }
    //  if zero collisions then we can see this thing
    if( !(g_CollisionMgr.m_nCollisions ) )
    {
        m_Owner->GetLocomotion( )->SetMoveAt( m_NavPlan.GetDestination() );
        m_LastNodeReached = true;
    }
    else
    {
        //
        //  if we can't see the last point, can we see the point after our current one?
        //  If so, head straight there.
        //
        slot_id nextID = m_NavPlan.GetNextPoint(m_NavPlan.GetCurrentGoal() );
        if( nextID != SLOT_NULL)
        {

            //  if the point after next isn't NULL, check to see if we can see it and if we can,
            //  then just skip the next node
            if( m_NavPlan.GetNextPoint( nextID ) != SLOT_NULL )
            {
                base_node* tempNavNode = g_NavMgr.GetNode( nextID );

                endBottom   = tempNavNode->GetPosition();

                endTop      = endBottom;

                //  FIXME:  need to query for real height and radius
                endTop.Y    += 180.0f;
                endBottom.Y   +=  30.0f;

                g_CollisionMgr.CylinderSetup(   m_Owner->GetGuid(),
                                                startBottom,
                                                endBottom,
                                                startTop,
                                                endTop,
                                                30.0f   );

                g_CollisionMgr.SetMaxCollisions(3);


//                g_CollisionMgr.LineOfSightSetup(    m_Owner->GetGuid(),  
//                                                    m_Owner->GetBBox().GetCenter(), 
//                                                    tempNavNode.GetPosition()  );
                g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );

                //PRINT_COLLISION_INFO("brain::UpdateMovement2",startTop,endTop );


                //  if zero collisions then we can see this thing
                if( !(g_CollisionMgr.m_nCollisions ) )
                {
                    m_NavPlan.ReachedPoint( m_NavPlan.GetCurrentGoal() );
                    vector3 tempVector;
                    tempVector = m_NavPlan.GetCurrentGoalPoint();
                    m_Owner->GetLocomotion( )->SetMoveAt( tempVector );

                }


            }
        }
        
        if( m_NavPlan.GetNextPoint( nextID ) == SLOT_NULL || 
            (m_NavPlan.GetNextPoint( nextID ) != SLOT_NULL && g_CollisionMgr.m_nCollisions  ) )
        {
        
        
            endBottom   = m_NavPlan.GetCurrentGoalPoint();
            endTop      = endBottom;

            //  FIXME:  need to query for real height and radius
            endTop.Y    += 180.0f;
            endBottom.Y +=  30.0f;

            g_CollisionMgr.CylinderSetup(   m_Owner->GetGuid(),
                                            startBottom,
                                            endBottom,
                                            startTop,
                                            endTop,
                                            30.0f   );

            g_CollisionMgr.SetMaxCollisions(3);
    

//            g_CollisionMgr.LineOfSightSetup(    m_Owner->GetGuid(),  
//                                                m_Owner->GetBBox().GetCenter(), 
//                                                m_NavPlan.GetCurrentGoalPoint()  );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );

            //PRINT_COLLISION_INFO("brain::UpdateMovement3",startTop, endTop );

            vector3 tempVector = m_NavPlan.GetCurrentGoalPoint();

            //  if zero collisions then we can see this thing
            if( !(g_CollisionMgr.m_nCollisions ) )
            {
                m_Owner->GetLocomotion( )->SetMoveAt( tempVector );
            }
            else
            {
                // else we can't see our current goal!!! 
                
                m_Owner->GetLocomotion( )->SetMoveAt( tempVector );
//                return false;

            }
 
        }
    }

    g_CollisionMgr.SetMaxCollisions(m_PreviousMaxCollisions);

    if( !m_LastNodeReached )
    {
 
        if( m_Owner->GetLocomotion()->IsAtDestination() )
        {
            m_LastNodeReached = m_NavPlan.ReachedPoint( m_NavPlan.GetCurrentGoal() );
        }


    }
    if( m_LastNodeReached && !m_DestinationReached )
    {

        if( m_Owner->GetLocomotion()->IsAtDestination() )
        {
            m_DestinationReached = true;    
        }  
    }

    return true;


}




//=================================================================================================

// Draw XZ circle
void draw_Cylinder( const vector3& B, f32 R, f32 H, xcolor Color = XCOLOR_WHITE )
{
    f32 Sin, Cos ;
    s32 i ;
    radian Angle ;

    s32    Segs       = 1 + (s32)(R * DEG_TO_RAD(360.0f) * 0.015f) ;
    radian DeltaAngle = DEG_TO_RAD(360) / Segs ;

    draw_Begin(DRAW_LINES) ;
    draw_Color(Color) ;

    // Draw base
    i     = Segs ;
    Angle = 0 ;
    while(i--)
    {
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( B.X + (R * Sin),
                     B.Y,
                     B.Z + (R * Cos) ) ;
        Angle += DeltaAngle ;
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( B.X + (R * Sin),
                     B.Y,
                     B.Z + (R * Cos) ) ;
    }

    // Draw top
    i     = Segs ;
    Angle = 0 ;
    while(i--)
    {
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( B.X + (R * Sin),
                     B.Y + H,
                     B.Z + (R * Cos) ) ;
        Angle += DeltaAngle ;
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( B.X + (R * Sin),
                     B.Y + H,
                     B.Z + (R * Cos) ) ;
    }

    // Draw joining lines
    i     = Segs ;
    Angle = 0 ;
    while(i--)
    {
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( B.X + (R * Sin),
                     B.Y,
                     B.Z + (R * Cos) ) ;

        draw_Vertex( B.X + (R * Sin),
                     B.Y + H,
                     B.Z + (R * Cos) ) ;

        Angle += DeltaAngle ;
    }

    draw_End() ;
}



void brain::OnRender ( void )
{
    CONTEXT( "brain::OnRender" );

    // Only draw on PC
    #ifdef TARGET_PC
        draw_ClearL2W() ;

        // Get loco
        base_loco* pLoco = (base_loco*)m_Owner->GetLocomotion() ;
        if (pLoco)
        {
            draw_Cylinder( pLoco->GetPosition(), 
                           pLoco->m_Physics.GetColHeight(), 
                           pLoco->m_Physics.GetColRadius(),
                           XCOLOR_RED ) ;
        }

    #endif
}






//=================================================================================================
guid brain::EvaluateTargets( void )
{
    f32 distanceToNearestTarget= 999999999.0f;
    guid targetGuid= 0 ;

    s32 count;
    for( count = 0; count < MAX_SENSORY_RECORDS; count++ )
    {
        if( m_Records[count].m_ContactObject  )
        {
            if( m_Records[count].m_Enemy )
            {
//                if( IsObjectVisible( m_Records[count].m_ContactObject ) )
                {
                    object* tempObject = g_ObjMgr.GetObjectByGuid( m_Records[count].m_ContactObject );
                    ASSERT(tempObject);

                    vector3 fromHere    = m_Owner->GetEyePosition( );
                    vector3 toHere      = tempObject->GetPosition( );
                    f32 lengthBetweenPoints;
                    lengthBetweenPoints = (fromHere - toHere ).LengthSquared();
                    if( lengthBetweenPoints < distanceToNearestTarget  )
                    {
                        distanceToNearestTarget = lengthBetweenPoints;
                        targetGuid = m_Records[count].m_ContactObject;
                    }


                }

            }

        }

    }

    return targetGuid;
}





//=================================================================================================
xbool brain::IsObjectVisible( guid thisObject )
{

    object* tempObject = g_ObjMgr.GetObjectByGuid( thisObject );
    ASSERT(tempObject);
    {

        vector3 fromHere = m_Owner->GetEyePosition();
        vector3 toHere = tempObject->GetPosition();

        fromHere.Y = GetEyeLevel();
        toHere.Y = GetEyeLevel();

        g_CollisionMgr.LineOfSightSetup( m_Owner->GetGuid(), fromHere, toHere );
        g_CollisionMgr.CheckCollisions();

        //PRINT_COLLISION_INFO("brain::IsObjectVisible",fromHere,toHere );


        s32 collisionCounter;
        s32 numberOfRealCollisions = 0;

        for( collisionCounter = 0; collisionCounter < g_CollisionMgr.m_nCollisions; collisionCounter++ )
        {
            if ( g_CollisionMgr.m_Collisions[collisionCounter].ObjectHitGuid != tempObject->GetGuid() )
            {
                numberOfRealCollisions++;
            }

        }


        //  if zero collisions then we can see this thing
        if( numberOfRealCollisions == 0 )
        {
            return TRUE;

        }
    }

    return FALSE;

}





//=================================================================================================
//
//  FindCover
//
//      - Find cover will attempt to locate a point with some partial cover that still has
//        line of sight on the target position passed in
//
//=================================================================================================
xbool brain::FindCover( vector3& thisTarget, vector3& thisResult )
{

    bbox searchBbox;
    xarray<base_node*> nodes;

    searchBbox.Set( m_Owner->GetEyePosition(), m_Owner->GetEyePosition() );
    searchBbox.Inflate( m_MaxSearchRangeForCover,m_MaxSearchRangeForCover,m_MaxSearchRangeForCover  );

    g_NavMgr.LocateAllNodesInArea ( searchBbox, nodes, nav_node::FLAG_COVER_SPOT );


    
    //  Now we have all the points within range of this spot so we scan through and see if they 
    //  have LOS on the target
    s32 count;
    for (count = 0; count < nodes.GetCount(); count++)
    {

        vector3 theStart, theEnd;
        f32 eyeLevel, coverLevel;

        eyeLevel = 130.0f;
        coverLevel = 50.0f;

        theStart = thisTarget;
        theStart.Y = GetEyeLevel();
        theEnd   = nodes[count]->GetPosition();
        theEnd.Y = GetEyeLevel();

        g_CollisionMgr.LineOfSightSetup( m_Owner->GetGuid(), theStart, theEnd );
        g_CollisionMgr.CheckCollisions();

        //PRINT_COLLISION_INFO("brain::FindCover",theStart,theEnd);


        //  if zero collisions then we can see this thing
       
        if( g_CollisionMgr.m_nCollisions == 0 ||(g_CollisionMgr.m_nCollisions == 1 && g_CollisionMgr.m_Collisions[0].ObjectHitGuid == m_Owner->GetGuid() ))
        {
            theStart.Y = GetCoverLevel();
            theEnd.Y = GetCoverLevel();

            
            g_CollisionMgr.RaySetup( m_Owner->GetGuid(), theStart, theEnd );
            g_CollisionMgr.CheckCollisions();

            //PRINT_COLLISION_INFO("brain::FindCover",theStart, (theStart - theEnd) );

            if( g_CollisionMgr.m_nCollisions == 0 ||(g_CollisionMgr.m_nCollisions == 1 && g_CollisionMgr.m_Collisions[0].ObjectHitGuid == m_Owner->GetGuid() ))
            {
                // still visible at ground level so not a valid spot
                return FALSE;

            }
            else
            {
                                       // For now we're just going to return the first one we find
                // FIXME:  needs to check all spots for the best one and also evaluate if this spot
                // gives them cover based on where the target is
                thisResult = nodes[count]->GetPosition();
                return TRUE;

            }

        }

    }
    return FALSE;
}




//=================================================================================================
void brain::ClearDestination( void )
{
    m_NavPlan.Reset();
    m_DestinationReached = true;
    m_LastNodeReached = true;



}





//=================================================================================================
xbool brain::AttemptShot( void )
{
    if( m_Owner->IsReadyToShoot( ) )
    {
//        m_Owner->Shoot();
        if(m_CurrentState->GetCurrentTarget())
        {
            FireWeaponAt(m_CurrentState->GetCurrentTarget() );
            return true;

        }
    }
    
    return false;

}


//=================================================================================================
f32 brain::GetDistanceToDestinationSquared( void )
{
    f32 distance;

    distance = (m_NavPlan.GetDestination() - m_Owner->GetPosition() ).LengthSquared();

    return distance;
    
}





//=================================================================================================
void brain::UpdateWander( f32 deltaTime )
{
    if(m_CurrentState == NULL )
    {
        return;
    }

    //  The first pass at wander logic will have the AI grab the nearest node to it and then pick
    //  a node that is connected to it and set that as it's current destination

    //   If we are currently moving around then just return;
    if( !m_DestinationReached )
    {
        return;
    }

    // if Wander frequency is 0 then we never wander
    if( m_CurrentState->GetWanderFrequency() == 0.0f )
    {
        return;
    }

    if(x_frand(0.0f, m_CurrentState->GetWanderFrequency()+deltaTime) > m_CurrentState->GetWanderFrequency() )
    {
        PickANewDestination();

    }



}



//=================================================================================================
void brain::PickANewDestination( void )
{
    vector3 tempPosition = m_Owner->GetPosition();
    slot_id anID = g_NavMgr.LocateNearestNode( tempPosition );
    
    base_node* aNavNode = g_NavMgr.GetNode( anID );

    if(aNavNode == NULL)
    {
        return;
    }

    ASSERT( aNavNode->IsInUse() );

    s32 whichConnection;
    whichConnection = x_rand() % aNavNode->GetConnectionCount();
    
    slot_id aConnectionID;
    aConnectionID = aNavNode->GetConnectionByIndex(whichConnection);

    base_connection* aNavConnection = g_NavMgr.GetConnection( aConnectionID );

    slot_id adjacentNodeID;
    adjacentNodeID = aNavConnection->GetOtherEnd( anID );

    vector3 newDestination = g_NavMgr.GetNode(adjacentNodeID)->GetPosition();

    SetNewDestination( newDestination);

}




//=================================================================================================
vector3 brain::GetLastKnownPosition( guid thisObject )
{
    s32 count;
    for( count = 0; count < MAX_SENSORY_RECORDS; count++ )
    {
        if(m_Records[count].m_ContactObject == thisObject )
        {
            return m_Records[count].m_PointOfContact;
        }

    }

    vector3 tempVec;
    tempVec.Zero();
    return tempVec;

}


//=================================================================================================
guid brain::FindAFriend( void )
{
    s32 count;
    for( count = 0; count < MAX_SENSORY_RECORDS; count++ )
    {
        if(m_Records[count].m_ContactObject != 0)
        {
        
            if( !m_Records[count].m_Enemy )
            {
                if( IsObjectVisible(m_Records[count].m_ContactObject )  )
                {
                    return m_Records[count].m_ContactObject;
                }
                
            }

        }

    }

    return 0;
}



//=================================================================================================
f32 brain::GetEyeLevel(void)
{
    
    return (m_Owner->GetPosition().Y + m_EyeLevel );// - m_Owner->GetDuckAmount() );
 


}


//=================================================================================================
f32 brain::GetTimeSinceObjectSeen( guid thisGuid )
{
    s32 count;
    for( count =0; count < MAX_SENSORY_RECORDS; count++ )
    {
        if( m_Records[count].m_ContactObject == thisGuid )
        {
            return (((f32)x_GetTimeSec()) - m_Records[count].m_TimeOfContact );
        }

    }
    
    return 999999.0f;
}




//=================================================================================================
void brain::UpdateLookAtDestination( void )
{
    m_LookAtObject = 0;
    m_LookAtPoint = m_NavPlan.GetCurrentGoalPoint();
}

//=================================================================================================
f32 brain::GetCoverLevel(void)
{

    return (m_Owner->GetPosition().Y + m_CoverLevel);



}


//=================================================================================================

vector3 brain::GetPointOnPath( f32  thisFarAhead )
{
    vector3 myPosition,
            nextPointOnPath,
            pointAfterNextOnPath;

    f32 thisFarToNextPoint;
    myPosition          = m_Owner->GetPosition();
    nextPointOnPath     = m_NavPlan.GetCurrentGoalPoint();
    
    thisFarToNextPoint = ( myPosition - nextPointOnPath).Length();

    slot_id nextID = m_NavPlan.GetNextPoint(m_NavPlan.GetCurrentGoal() );
    

    //  if the next point is farther away then we want to look, just look at that point
    if( nextID == SLOT_NULL  || m_LastNodeReached )
    {

        if( g_NavMgr.GetNode(m_NavPlan.GetCurrentGoal())->GetSlotID() == m_NavPlan.GetDestinationNode() )
        {
            return m_NavPlan.GetDestination();
        }
        else
        {
            return m_NavPlan.GetDestination();

            //  Else let's special case our final destination and look at a point just beyond it
            vector3 lastNode = m_NavPlan.GetLastNodePoint();
            vector3 destination = m_NavPlan.GetDestination();
            lastNode.X = destination.X + (destination.X - lastNode.X) * 1.1f;
            lastNode.Y = destination.Y + (destination.Y - lastNode.Y) * 1.1f;
            lastNode.Z = destination.Z + (destination.Z - lastNode.Z) * 1.1f;
            return lastNode;

        }
        

    }
    else if( thisFarToNextPoint > thisFarAhead )
    {
        return nextPointOnPath;
    }

    //  if the point after next isn't NULL, check to see if we can see it and if we can,
    //  then just skip the next node
    else //if( nextID != SLOT_NULL)
    {

        base_node* tempNavNode = g_NavMgr.GetNode( nextID );
        pointAfterNextOnPath = tempNavNode->GetPosition();

        f32 thisFarToPointAfterNext = (pointAfterNextOnPath - nextPointOnPath).Length();

        // if our look at is farther than 2 nodes ahead then we just return the 2nd node
        if( thisFarToPointAfterNext + thisFarToNextPoint < thisFarAhead )
        {
            return pointAfterNextOnPath;

        }
        else
        {
            // else we interpolate a point
            f32 remainder = thisFarAhead - thisFarToNextPoint;
            f32 distanceBetweenTwoNodes = (nextPointOnPath - pointAfterNextOnPath).Length();

            f32 fractionOfTheWayThere = remainder/ distanceBetweenTwoNodes;

            nextPointOnPath.X += (pointAfterNextOnPath.X - nextPointOnPath.X ) * fractionOfTheWayThere;
            nextPointOnPath.Y += (pointAfterNextOnPath.Y - nextPointOnPath.Y ) * fractionOfTheWayThere;
            nextPointOnPath.Z += (pointAfterNextOnPath.Z - nextPointOnPath.Z ) * fractionOfTheWayThere;

            return nextPointOnPath;


        }



    }

    ASSERT(FALSE);
    return nextPointOnPath;




 
    

}


vector3 brain::GetNearestPatrolPoint( void )
{
    node_slot_id aSlotID = g_NavMgr.GetNearestPatrolNode( GetOwner( )->GetPosition( ) );

    base_node* thisNode = g_NavMgr.GetNode( aSlotID );

    ASSERT(thisNode);

    vector3 aVector = thisNode->GetPosition();

    return aVector;

}

node_slot_id brain::GetNearestPatrolNode( void )
{

    return g_NavMgr.GetNearestPatrolNode( GetOwner( )->GetPosition( ) );

}

void brain::GetNextPatrolPointSlot( node_slot_id &thisSlot, xbool &forward )
{
    base_node* startNode = g_NavMgr.GetNode(thisSlot);
    ASSERT(startNode);
    
    s32 count;
    for(count = 0; count < startNode->GetConnectionCount(); count++)
    {
        base_connection* tempConnection = g_NavMgr.GetConnection( startNode->GetConnectionByIndex(count) );
        ASSERT(tempConnection);

        if( ((nav_connection*)tempConnection)->GetHints() & nav_connection::HINT_PATROL_ROUTE )
        {
            if ( ( forward && ((nav_connection*)tempConnection)->GetStartNode() == thisSlot ) ||
                 ( !forward && ((nav_connection*)tempConnection)->GetEndNode() == thisSlot )    )
            {
                thisSlot = tempConnection->GetOtherEnd(thisSlot);
                return;
            }
            

        }

    }
    forward = !forward;
    
}



//=============================================================================
void brain::OnLoad                  ( void )
{
//    (void)TextIn;
    x_strcpy( m_AIName, m_StartingAIName );

    
    m_CurrentState = GetStateByName( m_AIName );
    if(m_CurrentState)
    {
        m_CurrentState->OnEnterState();
    }



}



//=============================================================================
void brain::SetCurrentFromStarting  ( void )
{
    
    x_strcpy( m_AIName, m_StartingAIName );
    m_CurrentState = GetStateByName( m_AIName );


}



//=============================================================================
void brain::SetStartingFromCurrent  ( void )
{
    x_strcpy( m_StartingAIName, m_AIName );
    
}


xbool brain::FireWeaponAt( guid thisTarget, s32 thisWeapon )
{
    
    if ( m_Weapons[thisWeapon] == NULL )
        return false;

    // if the bone hasn't been found yet, then time to search for the index
    if(m_BoneIndexForWeapon == -1 )    
    {
           m_BoneIndexForWeapon = m_Owner->GetLocomotion()->GetBoneIndexByName( "Bip01 R Hand");
    }
    vector3 handPosition = m_Owner->GetLocomotion()->GetBonePosition( m_BoneIndexForWeapon );
    
    object* tempObject = g_ObjMgr.GetObjectByGuid( thisTarget );
    ASSERT(tempObject);



    m_Weapons[thisWeapon]->FireWeapon(   handPosition, 
                                        vector3(0.0f, 0.0f, 0.0f),
                                        tempObject->GetBBox().GetCenter() );

    return true;

}








///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void brain::OnEnumProp( prop_enum&  List )
{
    List.AddHeader(     "Brain",  
                        "Brain represents the AI for an NPC" 
                        );

    List.AddString(     "Brain\\AI State",  
                        "Current AI State" ,
                        PROP_TYPE_MUST_ENUM     );

    List.AddString(     "Brain\\Starting AI State",  
                        "Starting AI State" ,
                        PROP_TYPE_MUST_ENUM     );

        

    List.AddEnum(       "Brain\\Add AI State",
                        "No State\0IDLE\0ATTACK_NORMAL\0CURIOUS\0DEAD\0FLEE\0MOVE_TO\0FOLLOW\0PATROL\0GRABBED\0",
                        "Adds a new AI state to the Brain",
                        PROP_TYPE_MUST_ENUM );

    
    List.AddFloat(      "Brain\\Visual Recognition Range",
                        "Distance at which AI can visually recognize objects" );

    List.AddFloat(      "Brain\\Recognition Memory Length",
                        "Recognition memory length in seconds" );


    List.AddFloat(      "Brain\\Audio Recognition Range",
                        "Distance at which AI can hear objects" );

    List.AddFloat(      "Brain\\Max Search Range For Cover",
                        "Max Search Range For Cover is the max distance the brain will look to find a cover spot" );
               
    List.AddVector3(    "Brain\\Current Destination",
                        "Current destination for the brain ",
                         PROP_TYPE_MUST_ENUM );

    List.AddFloat(      "Brain\\Minimum Distance To Consider Movement Active",
                        "Min distance for the AI to consider itself actively moving" );

    List.AddFloat(      "Brain\\Time Till ReThink Path",
                        "How long will it tolerate unproductive movement without rethinking it's path" );

    List.AddFloat(      "Brain\\Time Till Cancel Path",
                        "How long till it will just cancel it's path all together" );

    List.AddFloat(      "Brain\\Eye Level",
                        "How high about his feet does he consider Eye level" );

    List.AddFloat(      "Brain\\Coverl Level",
                        "How high about his feet does he consider partial cover level" );

    List.AddFloat(      "Brain\\Look ahead distance",
                        "When moving, how far ahead on the path does the head look" );


    s32 count; 
    for( count = 0; count < kMAX_AI_STATES; count++ )
    {
        List.AddString(  xfs("Brain\\AI_State %d", count), "dummy vars to create AI states" , PROP_TYPE_DONT_SHOW );
//        List.AddString(  xfs("Brain\\AIStateName %d", count), "dummy vars to create AI states" );//, PROP_TYPE_DONT_SHOW );
    }


    for( count = 0; count< m_AIStates.GetCount(); count++ )
    {

        m_AIStates[count]->OnEnumProp(List);

    }



}



xbool brain::OnProperty( prop_query& I    )
{

    xbool found = false;

//    found  =  object::OnProperty( I );


    if( I.IsVar( "Brain\\AI State" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            if( m_CurrentState )
            {
                x_strsavecpy( m_AIName, m_CurrentState->GetName(), 64 );

                I.SetVarString( m_AIName, 64 );
            }
            else
            {
             
                I.SetVarString("None", 64 );
            }
  
        }
        else
        {
            x_strsavecpy( m_AIName, I.GetVarString(), 64 );
            
        }

    }

    if( I.IsVar( "Brain\\Starting AI State" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            I.SetVarString( m_StartingAIName, 64 );
  
        }
        else
        {
            x_strsavecpy( m_StartingAIName, I.GetVarString(), 64 );
            
        }

    }
    if(!m_CurrentState)
    {
        m_CurrentState = GetStateByName( m_AIName );
    }



    if( I.IsVar( "Brain\\Visual Recognition Range" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_RecognitionRangeVisual );
        }
        else
        {
            m_RecognitionRangeVisual = I.GetVarFloat();
        }

        return TRUE;
    }
    if( I.IsVar( "Brain\\Recognition Memory Length" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_RecognitionMemoryLength );
        }
        else
        {
            m_RecognitionMemoryLength = I.GetVarFloat();
        }

        return TRUE;
    }





    if( I.IsVar( "Brain\\Audio Recognition Range" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_RecognitionRangeAudio );
        }
        else
        {
            m_RecognitionRangeAudio = I.GetVarFloat();
        }

        return TRUE;
    }

    if( I.IsVar( "Brain\\Max Search Range For Cover" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MaxSearchRangeForCover );
        }
        else
        {
            m_MaxSearchRangeForCover = I.GetVarFloat();
        }

        return TRUE;
    }



  if( I.IsVar( "Brain\\Minimum Distance To Consider Movement Active" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_MinimumDistanceToConsiderMovementActive );
        }
        else
        {
            m_MinimumDistanceToConsiderMovementActive = I.GetVarFloat();
        }

        return TRUE;
    }


  if( I.IsVar( "Brain\\Time Till ReThink Path" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_TimeTillReThinkPath );
        }
        else
        {
            m_TimeTillReThinkPath = I.GetVarFloat();
        }

        return TRUE;
    }


  if( I.IsVar( "Brain\\Eye Level" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_EyeLevel );
        }
        else
        {
            m_EyeLevel = I.GetVarFloat();
        }

        return TRUE;
    }


  if( I.IsVar( "Brain\\Coverl Level" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_CoverLevel );
        }
        else
        {
            m_CoverLevel = I.GetVarFloat();
        }

        return TRUE;
    }



  if( I.IsVar( "Brain\\Look ahead distance" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_LookThisFarAhead );
        }
        else
        {
            m_LookThisFarAhead = I.GetVarFloat();
        }

        return TRUE;
    }





  if( I.IsVar( "Brain\\Time Till Cancel Path" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_TimeTillCancelPath );
        }
        else
        {
            m_TimeTillCancelPath = I.GetVarFloat();
        }

        return TRUE;
    }







    if ( I.IsVar( "Brain\\Current Destination" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarVector3( m_NavPlan.GetDestination() );
        }
        else
        {
            vector3 tempVector ;
            tempVector = I.GetVarVector3();
            SetNewDestination( tempVector);
        }

        return TRUE;
    }


    s32 count; 
    for( count = 0; count < kMAX_AI_STATES; count++ )
    {
        if( I.IsVar(xfs("Brain\\AI_State %d", count) ) )
        {
            if( I.IsRead() )
            {
                if( count < m_AIStates.GetCount() )
                {
                
                    if(m_AIStates[count] != NULL )
                    {

                        I.SetVarString( xfs("Name: %s  Type: %s", m_AIStates[count]->GetName(),m_AIStates[count]->GetTypeName() ), 256 );
                    }
                    else
                    {
                        ASSERT(FALSE);
                    }
                }
                else
                {
                    I.SetVarString( "None", 256 );
                }

            }
            else
            {
                const char* tempString = I.GetVarString();
                if( tempString[0] != 0 && x_strcmp(tempString, "None" )   )
                {
                    char tempName[32];
                    char tempType[32];
                    const char* startOfType = x_strstr(tempString,"Type: ");
                    x_strncpy(tempName, &tempString[6], (startOfType - tempString ) );
                    tempName[startOfType - tempString - 8] = '\0';
                    x_strcpy ( tempType, &startOfType[6] );

                    ai_state *tempState = NULL;

                    if (!x_strcmp("IDLE", tempType ) )
                    {
                        tempState = new ai_state_idle(this);
        
                    }
                    else if (!x_strcmp("ATTACK_NORMAL", tempType ) )
                    {
                        tempState = new ai_state_attack_normal(this);

                    }
                    else if (!x_strcmp("ATTACK_PASSIVE", tempType ) )
                    {
                        tempState = new ai_state_attack_passive(this);

                    }
                    else if (!x_strcmp("ATTACK_AGGRESSIVE", tempType ) )
                    {
                        tempState = new ai_state_attack_aggressive(this);

                    }
                    else if (!x_strcmp("CURIOUS", tempType ) )
                    {
                        tempState = new ai_state_curious(this);

                    }
                    else if (!x_strcmp("FLEE", tempType ) )
                    {
                        tempState = new ai_state_flee(this);

                    }
                    else if (!x_strcmp("DEAD", tempType ) )
                    {
                        tempState = new ai_state_dead(this);

                    }
                    else if (!x_strcmp("MOVE_TO", tempType ) )
                    {
                        tempState = new ai_state_move_to(this);

                    }
                    else if (!x_strcmp("FOLLOW", tempType ) )
                    {
                        tempState = new ai_state_follow(this);

                    }
                    else if (!x_strcmp("PATROL", tempType ) )
                    {
                        tempState = new ai_state_patrol(this);

                    }
                    else if (!x_strcmp("GRABBED",tempType ) )
                    {
                        tempState = new ai_state_grabbed(this);
                    }

                    if( tempState )
                    {
                        tempState->SetName( tempName );
                        m_AIStates.Append( tempState );
                    }
                    else
                    {
                    
                    }

                }

            }

        }


        if( I.IsVar(xfs("Brain\\AIStateName %d", count) ) )
        {
            if( count < m_AIStates.GetCount() )
            {

                if( I.IsRead() )
                {
                    if(m_AIStates[count] != NULL )
                    {
                        I.SetVarString( m_AIStates[count]->GetName(), 256 );
                    }
                    else
                    {
                        I.SetVarString( "None", 256);
                    }

                }
                else
                {
                    if( m_AIStates[count] != NULL )
                    {
                        m_AIStates[count]->SetName(  I.GetVarString() );
                    }
                    else
                    {
                        ASSERT(false);
                    }


                }
            }

        }


    }



    if( I.IsVar("Brain\\Add AI State" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            I.SetVarEnum("No State");

        }
        else
        {
            ai_state* tempState = NULL;
            if(!x_strcmp("No State",I.GetVarEnum() ) )
            {

            }
            else if (!x_strcmp("IDLE", I.GetVarEnum() ) )
            {
                tempState = new ai_state_idle(this);
            
            }
            else if (!x_strcmp("ATTACK_NORMAL", I.GetVarEnum() ) )
            {
                tempState = new ai_state_attack_normal(this);

            }
            else if (!x_strcmp("ATTACK_PASSIVE", I.GetVarEnum() ) )
            {
                tempState = new ai_state_attack_passive(this);

            }
            else if (!x_strcmp("ATTACK_AGGRESSIVE", I.GetVarEnum() ) )
            {
                tempState = new ai_state_attack_aggressive(this);

            }
            else if (!x_strcmp("CURIOUS", I.GetVarEnum() ) )
            {
                tempState = new ai_state_curious(this);

            }
            else if (!x_strcmp("FLEE", I.GetVarEnum() ) )
            {
                tempState = new ai_state_flee(this);

            }
            else if (!x_strcmp("DEAD", I.GetVarEnum() ) )
            {
                tempState = new ai_state_dead(this);

            }
            else if (!x_strcmp("MOVE_TO", I.GetVarEnum() ) )
            {
                tempState = new ai_state_move_to(this);
            }
            else if (!x_strcmp("FOLLOW", I.GetVarEnum() ) )
            {
                tempState = new ai_state_follow(this);
            }
            else if (!x_strcmp("PATROL", I.GetVarEnum() ) )
            {
                tempState = new ai_state_patrol(this);

            }
            else if (!x_strcmp("GRABBED",I.GetVarEnum() ) )
            {
                tempState = new ai_state_grabbed(this);
            }
 

            if( tempState )
            {
                m_AIStates.Append( tempState );
            }

        }

    }


    m_StateToDelete = NULL;

    for( count = 0; count< m_AIStates.GetCount(); count++ )
    {
        if(m_AIStates[count]->OnProperty( I ) == TRUE )
        {
            if(m_StateToDelete != NULL)
            {
                delete m_StateToDelete;
                m_StateToDelete = NULL;
                m_AIStates.Delete( count );
            }
            return true;

        }

    }






    return found;


}


void  brain::SetStateToDelete( ai_state* thisState )
{
    m_StateToDelete = thisState;


}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  GetAIStateEnumText
//
//      - returns a formatted string to create a list of the available ai names
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
const char*  brain::GetAIStateEnumText( void )
{
    s32 count = 0;
    s32 position = 0;
    s32 length ;

    length = x_strlen( "Previous" );
    x_strncpy( &(m_StateEnumName[position]),"Previous",length);
    position += length;
    m_StateEnumName[position] = '\0';
    position++;


    for( count =0; count < m_AIStates.GetCount(); count++ )
    {

        length = x_strlen( (m_AIStates[count])->GetName() );
        x_strncpy( &(m_StateEnumName[position]),m_AIStates[count]->GetName(),length);
        position += length;
        m_StateEnumName[position] = '\0';
        position++;
        ASSERT(position < 512 ); 

    }
    m_StateEnumName[position] = '\0';

    return (m_StateEnumName);
    
}
