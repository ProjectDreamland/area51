///////////////////////////////////////////////////////////////////////////////
//
//  Nav_mgr.cpp
//
//
///////////////////////////////////////////////////////////////////////////////
#include "navigation\nav_mgr.hpp"
#include "navigation\nav_plan.hpp"
#include "objects\nav_node_place_holder.hpp"
#include "objects\nav_connection_place_holder.hpp"




nav_mgr* nav_mgr::m_sThis = NULL;
//=============================================================================
//
//		
//
//
//=============================================================================
nav_mgr::nav_mgr(void) :
    m_HorizontalNodeSearchRange(2000.0f),
    m_VerticalNodeSearchRange(300.0f),
    m_SearchCount(0),
    m_NavConnectionModeEnabled(false),
    m_CheapestNode(SLOT_NULL),
    m_FirstNavNodeForConnection(0),
    m_FirstSelectedObject(SLOT_NULL),
    m_NavTestStart(SLOT_NULL),
    m_NavTestEnd(SLOT_NULL) 
{
    if ( m_sThis != NULL )
    {
        ASSERT(FALSE);    

    }

    Init();

}


//=============================================================================
//
//		
//
//
//=============================================================================
nav_mgr::~nav_mgr()
{


}



xbool nav_mgr::UpdateNavPlan( vector3& thisPoint, nav_plan& thisPlan )
{


    m_CheapestSolution = 99999999.0f;

    m_SearchCount++;

    slot_id nearestNode = thisPlan.GetStartingPoint();

    if( nearestNode == SLOT_NULL )
    {
        nearestNode = LocateNearestNode( thisPoint );
    }
//    thisPlan.SetDestination(thisPoint);
    slot_id destinationNode = LocateNearestNode( thisPlan.GetDestination() );

    //  Handle the special case of the the starting node being the destination
    if( nearestNode == thisPlan.GetDestinationNode() )
    {
        thisPlan.Reset();

        thisPlan.AddPoint( SLOT_NULL , destinationNode );
        return true;

    }
    

    m_NavNodes[nearestNode].SetSearchInfo(SLOT_NULL,SLOT_NULL,m_SearchCount,m_SearchStepNumber,0.0f);
    
    m_CheapestNode = nearestNode;
    
    s32 tries = 200;

    while ( m_CheapestNode != destinationNode && tries )
    {
        slot_id tempID = m_CheapestNode;
        m_CheapestNode = m_NavNodes[m_CheapestNode].GetNextInList();
        AddConnectedNodesToOpenList( tempID );
        tries--;

    } 

    slot_id tempID;
    s32 count=0;
    tempID = m_CheapestNode;

//    slot_id head;

    for( count =0; count < kMAX_CONNECTIONS; count++ )
    {
        m_NavNodes[count].SetIsInPath(false);

    }


    count = 0;
    while (tempID != SLOT_NULL)
    {
        if(m_NavNodes[tempID].GetPred() != SLOT_NULL)
            count++;

        m_NavNodes[tempID].SetIsInPath(true);
        tempID = m_NavNodes[tempID].GetPred();

    }

    tempID = m_CheapestNode;
    while(count >= 0 )
    {
        thisPlan.AddPoint(m_NavNodes[tempID].GetConnectionUsedInSearch(), tempID, count);
        tempID = m_NavNodes[tempID].GetPred();
        count--;

    }


    
    
//    while(head != -1)
//    {
//        char tempString[256];
//        x_sprintf( tempString,"Step:  %d   Node: %d   Cost: %f\n", count,head,m_NavNodes[head].GetCostToNode() );
        
//        head = m_NavNodes[head].GetNextInList();
//        count++;
//    }

//    thisPlan.AddPoint()

    if (tries > 0 )
    {
        thisPlan.SetCompletePath(true);
        return true;
    }
    else
    {
        thisPlan.SetCompletePath(false);
        return false;

    }


}






void  nav_mgr::AddConnectedNodesToOpenList( slot_id nearestNode )
{
    
    m_SearchStepNumber++;

    s32 count;
    for(count = 0; count < k_MAX_NODE_CONNECTIONS; count++ )
    {
        slot_id connectionID = m_NavNodes[nearestNode].GetConnectionByIndex( count );
        if(connectionID != SLOT_NULL )
        {
        
            slot_id nodeToAdd = m_Connections[connectionID].GetOtherEnd(nearestNode);

            //  if it is not in this search 
            if( ( m_NavNodes[nodeToAdd].GetSearchNumber() != m_SearchCount) )
            {
         
                m_NavNodes[nodeToAdd].SetCost( m_NavNodes[nearestNode].GetCostToNode() + m_Connections[connectionID].GetCost() );
                m_NavNodes[nodeToAdd].SetConnectionUsedInSearch( connectionID );
                InsertNodeIntoList( nodeToAdd, nearestNode, connectionID );

            }
            else if(  m_NavNodes[nodeToAdd].GetSearchStepNumber() == m_SearchStepNumber  )
            {
                //  if it is in the list already but it was from this same call to add nodes
                //  we need to check see which is cheaper and add the cheaper one
                //  FIXME!  
            
            }
            else
            {
                // else it is in the list and wasn't from this pass

            }

        }


    }



}


void nav_mgr::InsertNodeIntoList( slot_id thisNode, slot_id parent, slot_id connectionID )
{

//    slot_id currentID = m_CheapestNode; 


    //  Nodes are inserted in sorted order.  m_CheapestNode is the index to the current cheapest
    //  node in the list and is therefore the start of the list.  To inser the node we get it's
    //  cost and walk through the list until it's less than the next one or until the next index
    //  is SLOT_NULL

    //  if the cheapest node is SLOT_NULL then this is the first item to be entered.
    if ( m_CheapestNode == SLOT_NULL )
    {
        m_CheapestNode = thisNode;
        m_NavNodes[thisNode].SetSearchInfo( SLOT_NULL, parent,m_SearchCount, m_SearchStepNumber, m_NavNodes[parent].GetCostToNode() + m_Connections [ connectionID ].GetCost() );
        return;

    }


    //  if we make it here then at least one item is already in the list.  We step through the list
    //  till we
    
    slot_id     m_HigherCostNode, 
                nodeToInsertAfter;

    m_HigherCostNode = m_CheapestNode;
    nodeToInsertAfter = SLOT_NULL;

    f32 thisCost = m_NavNodes[thisNode].GetCostToNode();

    //  This while loop should walk through the list till it hits a node that is higher cost than
    //  itself or until it hits the end of the list.
    
    if(thisCost < m_NavNodes[m_CheapestNode].GetCostToNode() )
    {
        m_CheapestNode = thisNode;
    }

    while( m_HigherCostNode != SLOT_NULL &&  thisCost > m_NavNodes[m_HigherCostNode].GetCostToNode()  )
    {
        nodeToInsertAfter = m_HigherCostNode;
        m_HigherCostNode = m_NavNodes[m_HigherCostNode].GetNextInList();

    }

    //   if this is the new cheapest node...
    if(nodeToInsertAfter == SLOT_NULL )
    {

        m_CheapestNode = thisNode;
        m_NavNodes[thisNode].SetSearchInfo( m_HigherCostNode, parent,m_SearchCount, m_SearchStepNumber, m_NavNodes[parent].GetCostToNode() + m_Connections [ connectionID ].GetCost() );

    }
    else //if(m_HigherCostNode != SLOT_NULL )
    {
        ASSERT(nodeToInsertAfter != SLOT_NULL );
        // to insert in the list we first make the previous point to this one
        m_NavNodes[nodeToInsertAfter].SetNextInList( thisNode );
        // then make this one point to the next one
        m_NavNodes[thisNode].SetSearchInfo( m_HigherCostNode, parent,m_SearchCount, m_SearchStepNumber, m_NavNodes[parent].GetCostToNode() + m_Connections [ connectionID ].GetCost() );
        

    }



}



//=============================================================================
//
//		
//
//
//=============================================================================
nav_node& nav_mgr::GetNavNode(slot_id thisNavNode )
{
    ASSERT(thisNavNode != SLOT_NULL && thisNavNode < kMAX_NAV_NODES );
    return (m_NavNodes[thisNavNode] );

}




//=============================================================================
//
//		
//
//
//=============================================================================
nav_connection& nav_mgr::GetConnection( slot_id thisNavConnection )
{
    ASSERT(thisNavConnection != SLOT_NULL && thisNavConnection < kMAX_CONNECTIONS );
    return ( m_Connections[thisNavConnection] );

}





//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetNewNavNode( void )
{
    slot_id anId = GetFirstOpenNavNode();
    GetNavNode(anId ).Reset();
    
    m_NavNodes[anId].SetIsInUse();
    return anId;
}




//=============================================================================
//
//		
//
//
//=============================================================================
void nav_mgr::Init ( void )
{
    //  singleton class.  Need to keep track of the singleton pointer
    m_sThis = this;

    s32 count;
    for( count = 0; count < kMAX_STEPS; count++)
        m_CurrentPath[count] = SLOT_NULL;

    for(count = 0; count < kMAX_NAV_NODES; count++)
    {
        m_NavNodes[count].Reset();
        m_NavNodes[count].SetSlotID( count );
    }

    for(count = 0; count < kMAX_CONNECTIONS; count++)
    {
        m_Connections[count].Reset();
    }



    m_Destination.Zero();


}



slot_id nav_mgr::LocateNearestNode ( vector3& thisPoint )
{
    f32 MinDistance = 999999999.0f;
    slot_id NearestSlot = SLOT_NULL;

    m_SearchCount++;

    
    //  SLOWEST POSSIBLE WAY TO DO THIS.  :P  Sorry, just for now.  Need to get a spatial
    //  Database in place for the real version or get these into the database.

    s32 count;
    for( count = 0; count < kMAX_NAV_NODES; count++ )
    {
        if( m_NavNodes[count].IsInUse() )
        {
            f32 thisLength = ( m_NavNodes[count].GetPosition() - thisPoint).LengthSquared();
            if(thisLength < MinDistance )
            {
                MinDistance = thisLength;
                NearestSlot = count;
            }
        }

    }
    
    return NearestSlot;

}

//=============================================================================

void nav_mgr::LocateAllNodesInArea ( bbox& Area, xarray<base_node*>& Nodes, u32 flags  )
{
    s32 count;
    for( count = 0; count < kMAX_NAV_NODES; count++ )
    {
        if( m_NavNodes[count].IsInUse() )
        {
            if( flags == nav_node::FLAG_ALL || flags & m_NavNodes[count].GetAttributes() )
            {            
                if( Area.Intersect( &m_NavNodes[count].GetPosition(), 1 ) )
		        {
			        Nodes.Append( &m_NavNodes[count] );			
		        }
            }
        }
    }
}

//=============================================================================


//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetFirstOpenNavNode ( void )
{
    slot_id count;
    for(count=0 ; count<kMAX_NAV_NODES ; count++)
    {
        if( !(m_NavNodes[count].IsInUse() )  )
            return count;
    }

    return SLOT_NULL;
}


void nav_mgr::DeleteNavNode        ( slot_id  thisNavNode    )
{

    ASSERT(thisNavNode != SLOT_NULL);

    
    m_NavNodes[thisNavNode].Reset();

}



void nav_mgr::DeleteConnection     ( slot_id  thisConnection )
{
    ASSERT(thisConnection != SLOT_NULL );

    m_NavNodes[m_Connections[thisConnection].GetStartNode()].DeleteConnection(thisConnection);
    m_NavNodes[m_Connections[thisConnection].GetEndNode()].DeleteConnection(thisConnection);

    m_Connections[thisConnection].Reset();
    

}




//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetNewConnection( void )
{
    slot_id anId = GetFirstOpenConnection();
    GetConnection( anId ).Reset();
    

    return anId;
}






//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetFirstOpenConnection ( void )
{
    slot_id count;
    for(count=0 ; count<kMAX_CONNECTIONS ; count++)
    {
        if( m_Connections[count].GetStartNode() == SLOT_NULL ) 
            return count;

    }

    return SLOT_NULL;

}



//void nav_mgr::FileIO( fileio& File )
//{
    
//    File.Static( m_Connections, kMAX_CONNECTIONS    );
//    File.Static( m_NavNodes,    kMAX_CONNECTIONS    );
//    File.Static( m_SearchCount                      );


//}


//void nav_mgr::FileIO( fileio& File )
//{

 
//}


void nav_mgr::LoadNavMap(const char *fileName )
{
    Init();
    X_FILE* tempFile = x_fopen(fileName, "rb" );

    s32 count = 0;

    if (tempFile)
    {


        for( count =0; count< kMAX_CONNECTIONS;count++ )    
        {
            m_Connections[count].Load(tempFile);

        }

        for( count =0; count< kMAX_NAV_NODES;count++  )    
        {
            m_NavNodes[count].Load(tempFile);

        }

        x_fclose(tempFile);

        count = 0;
        while (count < kMAX_NAV_NODES  &&  m_NavNodes[count].IsInUse()  )
        {
            guid aGuid = g_ObjMgr.CreateObject(object::TYPE_NAV_NODE_PLACE_HOLDER);

            nav_mgr::GetNavMgr()->GetNavNode(count).SetPlaceHolder(aGuid );

            object *tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
            nav_node_place_holder &aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

            aNNPH.SetNode( count );

    //        aNNPH.OnMove(nav_mgr::GetNavMgr()->GetNavNode(count).GetPosition());
        
            count++;
        }

    
        count =0;
        while ( count < kMAX_CONNECTIONS && m_Connections[count].GetEndNode() != SLOT_NULL )
        {
        
            guid node1, node2;

            node1 = m_NavNodes[m_Connections[count].GetStartNode()].GetPlaceHolder();
            node2 = m_NavNodes[m_Connections[count].GetEndNode()].GetPlaceHolder();


            AddGuidToNavConnection(node1,count);
            AddGuidToNavConnection(node2,count);
            count++;

        }

        count =0;
        while ( m_NavNodes[count].IsInUse() )
        {
            guid aGuid = nav_mgr::GetNavMgr()->GetNavNode(count).GetPlaceHolder();

            object *tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
            nav_node_place_holder &aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

            aNNPH.OnMove( nav_mgr::GetNavMgr()->GetNavNode(count).GetPosition() );

            count++;

        } 

//        x_fclose(tempFile);
    //    m_hConnections.SetName( String );
    //    m_hConnections.GetPointer(); // Force to load now 
    }
}


void nav_mgr::SaveNavMap(const char* fileName )
{

    X_FILE* tempFile = x_fopen(fileName, "wb" );
//    ASSERT(tempFile);
    
//    fileio File;

//    File.Save(tempFile,*this,false);
    s32 count = 0;

    if (tempFile)
    {


        for( count =0; count< kMAX_CONNECTIONS;count++ )    
        {
            m_Connections[count].Save(tempFile);

        }

        for( count =0; count< kMAX_NAV_NODES;count++  )    
        {
            m_NavNodes[count].Save(tempFile);

        }

        x_fclose(tempFile);
    }
}




void nav_mgr::AddGuidToNavConnection( guid aGuid, slot_id thisConnection)
{

    object *tempObject = g_ObjMgr.GetObjectByGuid(aGuid);
    if ( ( tempObject ) && (tempObject->IsKindOf(nav_node_place_holder::GetRTTI())) )   // SB - Crash FIX!!
    {
        if(m_FirstSelectedObject == SLOT_NULL)
        {
            m_FirstSelectedObject = tempObject->GetSlotID();
        }
        else if( m_FirstSelectedObject == tempObject->GetSlotID() )
        {
            // The clicked the same node again...
            m_FirstSelectedObject = SLOT_NULL;
//            return false;
        }
        else 
        {
            // if we have one and they selected

//            slot_id anID = nav_mgr::GetNavMgr()->GetNewConnection();
            guid tempGuid = g_ObjMgr.CreateObject(object::TYPE_NAV_CONNECTION_PLACE_HOLDER );
            object *tempNNCPH = g_ObjMgr.GetObjectByGuid( tempGuid );
            nav_connection_place_holder &aNNCPH = nav_connection_place_holder::GetSafeType( *tempNNCPH);

            slot_id anID = thisConnection;
            
            aNNCPH.SetConnection( anID );

            nav_mgr::GetNavMgr()->GetConnection(anID).SetPlaceHolder(aNNCPH.GetGuid());


            
            object *tempNNPH = g_ObjMgr.GetObjectBySlot(m_FirstSelectedObject );
            nav_node_place_holder &aNavNode1 = nav_node_place_holder::GetSafeType(*tempNNPH);

            object *tempObject2 = g_ObjMgr.GetObjectBySlot(tempObject->GetSlotID() );
            nav_node_place_holder &aNavNode2 = nav_node_place_holder::GetSafeType(*tempObject2);



            slot_id connectedNavNode = aNNCPH.GetConnection();

            nav_connection &aNC = nav_mgr::GetNavMgr()->GetConnection(connectedNavNode);
            aNC.SetNodes(aNavNode1.GetNode() , aNavNode2.GetNode() );

            nav_node &aNN1 = nav_mgr::GetNavMgr()->GetNavNode(aNavNode1.GetNode() );
            nav_node &aNN2 = nav_mgr::GetNavMgr()->GetNavNode(aNavNode2.GetNode() );

            aNN1.AddConnection(connectedNavNode);
            aNN2.AddConnection(connectedNavNode);

            

            aNNCPH.OnMove(aNC.GetPosition() );
            
            m_FirstSelectedObject = SLOT_NULL;

//            return false;
        }


    }

//    return true;

}











void nav_mgr::SetNavTestStart( guid thisNode )
{
    object *tempNNCPH = g_ObjMgr.GetObjectByGuid( thisNode);
    if(tempNNCPH && tempNNCPH->GetType() == object::TYPE_NAV_NODE_PLACE_HOLDER )
    {
        nav_node_place_holder &aNNPH = nav_node_place_holder ::GetSafeType( *tempNNCPH);

        m_NavTestStart = aNNPH.GetNode();
    }

}


void nav_mgr::SetNavTestEnd  ( guid thisNode )
{
    object *tempNNCPH = g_ObjMgr.GetObjectByGuid( thisNode);
    if(tempNNCPH && tempNNCPH->GetType() == object::TYPE_NAV_NODE_PLACE_HOLDER )
    {
        nav_node_place_holder &aNNPH = nav_node_place_holder ::GetSafeType( *tempNNCPH);

        m_NavTestEnd = aNNPH.GetNode();

    }

}






void nav_mgr::TestNav(void)
{
    
    if( m_NavTestStart != SLOT_NULL && m_NavTestEnd != SLOT_NULL )
    {
        nav_plan aPlan;
        aPlan.SetStartingPoint( m_NavTestStart);
        aPlan.SetDestination(m_NavNodes[m_NavTestEnd].GetPosition() );

        UpdateNavPlan(m_NavNodes[m_NavTestEnd].GetPosition() ,aPlan );
    
    }


}





void nav_mgr::FileIO( fileio& File )
{

//    fileio File;
//    return( File.PreLoad( FP ) );

    s32 count = 0;
    for( count = 0; count < kMAX_NAV_NODES; count++ )
    {
        m_NavNodes[count].FileIO( File );
    }

    for( count = 0; count < kMAX_CONNECTIONS; count++ )
    {
        m_Connections[count].FileIO( File );
    }


}




void nav_mgr::SanityCheck(void)
{
    s32 count;
    for(count = 0; count < kMAX_NAV_NODES; count++ )
    {
        

    }


}















/**
 * Initialise the cost of each node to the maximum possible cost.
 */
/*
extern Node* source;
extern Node* target;

source->cost = 0.0f;

// could be almost any data-structure
vector<Node*> open;

open.insert( source );

while (!open.empty())
{
 // pick the next node as the one with the best estimate
    Node* current = FindBest( &open );

 // check if we've reached the target, and if so, return the result!
    if (current == target)
        return true;

 // scan all the neighbours of the current node
    for (Edge* e = current->edges.begin(); e != current->edges.end(); e++)
    {
     // determine the cost of this node for taking the current edge
        Node* neighbour = e->GetEndNode();
        const float cost = current->cost + e->length;
     // if it's worse, then we just skip it
        if (cost > neigbour->cost)
            continue;
                
     // remove this neighbour from the list
        open.remove( neigbour );

     // remember that the path via the current node is better
        neighbour->cost = cost;
        neighbour->parent = current;

     // and compute the estimated path length if this neighbour is used next
        neighbour->estimate = neighbour->cost 
                 + VectorLength( neighbour->pos - target->pos );

     // and allow the algorithm to scan this node when it becomes the best option
        open.insert( neighbour );
    }
}
*/



















/*





//=============================================================================
//
//		
//
//
//=============================================================================
xbool nav_mgr::UpdateNavPlan ( vector3& thisPoint, nav_plan& thisPlan )
{
    xbool   NotThereYet = true;
    s32     StepsSearched = 0;
    m_CheapestSolution = 99999999.0f;


    slot_id nearestNode = LocateNearestNode( thisPoint );
    slot_id destinationNode = LocateNearestNode( thisPlan.GetDestination() );


    if( nearestNode == thisPlan.GetDestination() )
    {
        thisPlan.AddPoint(SLOT_NULL, destinationNode );
        return true;

    }

    //  Ok, if the nearest node to us is not the nearest node to the destination then it's time
    //  to do some searching.

    //  First thing to do is add all the connection costs and nodes from the nearest node to get
    //  us started

    ResetSearchNodes();


    AddNodeToSearch( nearestNode );

    s32 MaxTries = 128;
    while( NotThereYet && MaxTries )
    {
        MaxTries--;
        s32 indexOfCurentCheapestNode = GetCheapestNode();
        if( m_PathNodes[indexOfCurentCheapestNode].m_Node == destinationNode )
        {
            UpdateNavPlan(thisPlan, );
        }

        
    }



    /*
    m_SearchCount++;

    //  First we check to make sure that the navPlan's final goal hasn't changed
    if ( thisPlan.GetDestination() != thisPoint )  
    {
        //  if it changed we need to start over
        thisPlan.Reset();
        thisPlan.SetDestination( thisPoint );
    }

    vector3 WorkingPoint;
    slot_id WorkingNode;
//            DestinationNode;

    //  Figure out what node to start at
    if( thisPlan.GetStartingPoint() != SLOT_NULL )
    {
        WorkingNode = LocateNearestNode( thisPoint );
    }
    else
    {
        WorkingNode = thisPlan.GetLastPoint();
    }
    //  At this point we should have a Working Node that is where we will be starting

    s32 StepCount = 0;
    
//    object* tempObject  = g_ObjMgr.GetObjectBySlot( WorkingNode );
//    nav_node CurrentNavNode = nav_node::GetSaveType( *tempObject );

    while ( NotThereYet && StepCount < k_MAX_SEARCH_NODES )
    {
        int connectionCount;

        for(connectionCount = 0; connectionCount< k_MAX_NODE_CONNECTIONS ; connectionCount++)
        {
//            if(CurrentNavNode.GetConnectionByIndex(connectionCount ) != SLOT_NULL )
            {
                

            }
        }
        


    }




/*    return false;

}



//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::LocateNearestNode ( vector3& thisPoint )
{
    f32 MinDistance = 999999999.0f;
    slot_id NearestSlot = SLOT_NULL;

    m_SearchCount++;

    
    //  SLOWEST POSSIBLE WAY TO DO THIS.  :P  Sorry, just for now.  Need to get a spatial
    //  Database in place for the real version or get these into the database.

    s32 count;
    for( count = 0; count < kMAX_NAV_NODES; count++ )
    {
        if( m_NavNodes[count].IsInUse() )
        {
            f32 thisLength = ( m_NavNodes[count].GetPosition() - thisPoint).LengthSquared();
            if(thisLength < MinDistance )
            {
                MinDistance = thisLength;
                NearestSlot = count;
            }
        }

    }
    
    return NearestSlot;



/*

    bbox volumeToCheck;

    volumeToCheck.Translate (   thisPoint );
    volumeToCheck.Inflate   (   m_HorizontalNodeSearchRange,
                                m_VerticalNodeSearchRange,
                                m_HorizontalNodeSearchRange );


    g_ObjMgr.SelectBBox(    object::ATTR_ALL, volumeToCheck, object::TYPE_NAVNODE );    
    

    
    slot_id anID = g_ObjMgr.StartLoop();

    while ( anID != SLOT_NULL )
    {
        f32 thisDistance = g_ObjMgr.GetObjectBySlot(anID)->GetPosition().Difference(thisPoint);
        if( thisDistance < MinDistance )
        {
            MinDistance = thisDistance;
            NearestSlot = anID;
        }
        anID = g_ObjMgr.GetNextResult( anID );

    }
    
    g_ObjMgr.EndLoop();

    //  if NearestSlot is neg 1 then we need to walk ALL nav nodes to find nearest
    if ( NearestSlot != SLOT_NULL )
    {
    
        anID = g_ObjMgr.GetFirst( object::type::TYPE_NAVNODE );
        if( anID == SLOT_NULL )
        {
//            e_throw("No Navnodes in obj_mgr");
            return SLOT_NULL;
        }

        while( anID != SLOT_NULL )
        {
            f32 thisDistance = g_ObjMgr.GetObjectBySlot(anID)->GetPosition().Difference(thisPoint);
            if( thisDistance < MinDistance )
            {
                MinDistance = thisDistance;
                NearestSlot = anID;
            }
            
            anID = g_ObjMgr.GetNext( anID );
        }

    }

    //  At this time we are guaranteed to have a slot and a distance unless there were no
    //  nav_node in which case it should throw an exception

    return anID;
*/
/*
}


//=============================================================================
//
//		
//
//
//=============================================================================
void nav_mgr::Init ( void )
{
    //  singleton class.  Need to keep track of the singleton pointer
    m_sThis = this;

    s32 count;
    for( count = 0; count < kMAX_STEPS; count++)
        m_CurrentPath[count] = SLOT_NULL;

    for(count = 0; count < kMAX_NAV_NODES; count++)
    {
        m_NavNodes[count].Reset();
    }

    for(count = 0; count < kMAX_CONNECTIONS; count++)
    {
        m_Connections[count].Reset();
    }



    m_Destination.Zero();


}



//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetFirstOpenConnection ( void )
{
    slot_id count;
    for(count=0 ; count<kMAX_CONNECTIONS ; count++)
    {
        if( m_Connections[count].GetStartNode() == SLOT_NULL ) 
            return count;

    }

    return SLOT_NULL;

}


//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetFirstOpenNavNode ( void )
{
    slot_id count;
    for(count=0 ; count<kMAX_NAV_NODES ; count++)
    {
        if( !(m_NavNodes[count].IsInUse() )  )
            return count;

    }

    return SLOT_NULL;

}


//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetNewNavNode( void )
{
    slot_id anId = GetFirstOpenNavNode();
    GetNavNode(anId ).Reset();
    
    m_NavNodes[anId].SetIsInUse();
    return anId;
}


//=============================================================================
//
//		
//
//
//=============================================================================
nav_node& nav_mgr::GetNavNode(slot_id thisNavNode )
{
    ASSERT(thisNavNode >=0 && thisNavNode < kMAX_NAV_NODES );
    return (m_NavNodes[thisNavNode] );

}


//=============================================================================
//
//		
//
//
//=============================================================================
nav_connection& nav_mgr::GetConnection( slot_id thisNavConnection )
{
    ASSERT(thisNavConnection >=0 && thisNavConnection < kMAX_CONNECTIONS );
    return ( m_Connections[thisNavConnection] );

}



//=============================================================================
//
//		
//
//
//=============================================================================
slot_id nav_mgr::GetNewConnection( void )
{
    slot_id anId = GetFirstOpenConnection();
    GetConnection( anId ).Reset();
    

    return anId;
}


//=============================================================================
//
//		
//
//
//=============================================================================
void nav_mgr::StartNavConnectionCreation ( slot_id firstPoint )
{
    m_FirstNavNodeForConnection = firstPoint;


}



//=============================================================================
//
//		
//
//
//=============================================================================
void nav_mgr::CompleteNavConnection( slot_id secondPoint )
{
    if ( m_FirstNavNodeForConnection != 0 )
    {
        slot_id anID = GetFirstOpenConnection();

//        GetConnection(anID).SetNodes(  m_FirstNavNodeForConnection, secondPoint );


    }   
    else
    {
        ASSERT(false);
    }

}



void nav_mgr::ResetSearchNodes(void)
{
    s32 count;
    for(count = 0; count < k_MAX_SEARCH_NODES; count++ )
    {
    
        m_PathNodes[count].m_Node          = SLOT_NULL;
        m_PathNodes[count].m_Connection    = SLOT_NULL;
        m_PathNodes[count].m_TotalCost     = -1.0f;
        m_PathNodes[count].m_SearchStep    = 0;
    }


}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AddNodeToSearch
//
//      Adds all the nodes to current list of nodes and costs
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void nav_mgr::AddNodeToSearch( slot_id nearestNode, f32 startingCost, slot_id destinationNode )
{
    nav_node &thisNavNode = GetNavNode(nearestNode);

    ASSERT( thisNavNode.IsInUse() );

    m_SearchStep++;

    s32 count;

    //  walk through the connections that are tied to this node
    for(count = 0; count <k_MAX_NODE_CONNECTIONS; count++)
    {
        //   if this connection slot is being used then investigate it further
        slot_id thisConnectionID = thisNavNode.GetConnectionByIndex( count );
        if( thisConnectionID != SLOT_NULL )
        {
           
            //  It's in use, not get the connection and the node that it attaches us too
            slot_id targetNodeID = thisConnection.GetOtherEnd(nearestNode) ;
            nav_connection &thisConnection  = GetConnection( thisConnectionID );
            nav_node       &targetNode      = GetNavNode( targetNodeID );

            
            // Make sure this node is in use.  if not, we got a logic error somewhere
            ASSERT(targetNode.IsInUse() );

            // Ok, so we got the connection and the node, now we make sure it's not already
            // in the search and if it is then we need to see if it was done in a previous step
            // of this very same AddNodeToSearch call.  This would be the case if there are two or
            // more connections between the same two nodes.  Need to check them both in case one
            // is a better fit for our current AI
            if( targetNode.GetSearchNumber() == m_SearchCount)
            {
                // if this is true then it was added earlier in this function call.  need to see
                // which path is better
                if(targetNode.GetSearchStepNumber() == m_SearchStep )
                {
                    f32 thisCost = thisConnection.GetCost() + startingCost;
                    s32 nodeAlreadyInList = GetIndexForNodeAlreadyInList( targetNodeID, m_SearchStep ) ;
                    if( thisCost < m_PathNodes[nodeAlreadyInList].m_TotalCost )
                    {
                        // so if taking this connection is cheaper then we need to replace the 
                        // data for the node
                        m_PathNodes[nodeAlreadyInList].m_Connection = thisConnectionID;
                        m_PathNodes[nodeAlreadyInList].m_Node       = targetNodeID;
                        m_PathNodes[nodeAlreadyInList].m_SearchStep = m_SearchStep;
                        m_PathNodes[nodeAlreadyInList].m_TotalCost  = thisCost;



                    }

                }

            }
            else
            {
                s32 firstOpenNode = GetFirstOpenSearchNode(m_SearchStep);
                m_PathNodes[firstOpenNode].m_Connection = thisConnectionID;
                m_PathNodes[firstOpenNode].m_Node       = targetNodeID;
                m_PathNodes[firstOpenNode].m_SearchStep = m_SearchStep;
                m_PathNodes[firstOpenNode].m_TotalCost  = thisConnection.GetCost() + startingCost;
           

            }

        }

    }
    
}
    



slot_id nav_mgr::GetIndexForNodeAlreadyInList(slot_id thisNode, s32 thisStep )
{
    s32 count; 
    for( count= 0; count < k_MAX_SEARCH_NODES; count++ )
    {
        if( m_PathNodes[count].m_Node == thisNode && m_PathNodes[count].m_SearchStep == thisStep )
        {
            return count;
        }

        
    }


    // should never get here if called correctly
    ASSERT(false);
    return 0;
}


s32 nav_mgr::GetFirstOpenSearchNode( void )
{
    s32 count; 
    for( count= 0; count < k_MAX_SEARCH_NODES; count++ )
    {
        if(m_PathNodes[count].m_Connection != SLOT_NULL )
        {
            return count;

        }


    }
    // search too deep!
    ASSERT( false );

}


s32 nav_mgr::GetCheapestNode(void)
{
    s32 count;
    f32 cheapestCost = 9999999999.0f;
    s32 cheapestIndex -1;
    for(count=0; count < k_MAX_SEARCH_NODES; count++)
    {
        if( m_PathNodes[count].m_Connection != -1 )
        {
            if( m_PathNodes[count].m_TotalCost < cheapestCost )
            {
                cheapestCost    = m_PathNodes[count].m_TotalCost;
                cheapestIndex   = count;


            }

        }
    }
    
    return cheapestIndex;


}






void nav_plan::UpdateNavPlan( nav_plan &thisPlan )
{
    s32 reverseList[64];
    s32 lastNodeAdded =0;

    //  First we have to reverse search from the best path for a list of nodes



    do
    {


    } while( m_NodePath[)



}
*/
