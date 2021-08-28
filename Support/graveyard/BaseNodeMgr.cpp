///////////////////////////////////////////////////////////////////////////////////////////////////
// BaseNodeMgr.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "BaseNodeMgr.hpp"
#include "navigation\base_plan.hpp"
#include "objects\nav_node_place_holder.hpp"
#include "objects\nav_connection_place_holder.hpp"

//=========================================================================

basenode_mgr::basenode_mgr (void) :
    m_HorizontalNodeSearchRange(2000.0f),
    m_VerticalNodeSearchRange(300.0f),
    m_SearchCount(0),
    m_NavConnectionModeEnabled(false),
    m_CheapestNode(SLOT_NULL),
    m_FirstNavNodeForConnection(0),
    m_FirstSelectedObject(SLOT_NULL),
    m_TestStart(SLOT_NULL),
    m_TestEnd(SLOT_NULL) 
{

}

//=========================================================================

basenode_mgr::~basenode_mgr (void)
{

}

//=========================================================================

base_node* basenode_mgr::GetNode ( node_slot_id NodeIndex )
{
    (void)NodeIndex;
    x_throw( "This function should never get called." );

    return NULL;
}

//=========================================================================

base_connection* basenode_mgr::GetConnection ( node_slot_id ConnectionIndex )
{
    (void)ConnectionIndex;
    x_throw( "This function should never get called." );

    return NULL;
}

//=========================================================================

xbool basenode_mgr::UpdatePlan ( vector3& thisPoint, base_plan* thisPlan )
{
    x_try;

    m_CheapestSolution = 99999999.0f;

    m_SearchCount++;

    node_slot_id nearestNode = thisPlan->GetStartingPoint();

    if( nearestNode == SLOT_NULL )
    {
        nearestNode = LocateNearestNode( thisPoint );
    }

    node_slot_id destinationNode = LocateNearestNode( thisPlan->GetDestination() );

    //  Handle the special case of the the starting node being the destination
    if( nearestNode == thisPlan->GetDestinationNode() )
    {
        thisPlan->Reset();

        thisPlan->AddPoint( SLOT_NULL , destinationNode );
        return true;
    }
    

    GetNode( nearestNode )->SetSearchInfo(SLOT_NULL,SLOT_NULL,m_SearchCount,m_SearchStepNumber,0.0f);
    
    m_CheapestNode = nearestNode;
    
    s32 tries = 200;

    while ( m_CheapestNode != destinationNode && tries )
    {
        node_slot_id tempID = m_CheapestNode;
        if( m_CheapestNode == SLOT_NULL )
        {
            x_DebugMsg("\n--No Valid Path found to destination.  Probable nav grid break.\n\n");
            return false;
        }

        m_CheapestNode = GetNode( m_CheapestNode )->GetNextInList();
        AddConnectedNodesToOpenList( tempID );
        tries--;
    } 

    node_slot_id tempID;
    tempID = m_CheapestNode;

    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );

    while( pNode )
    {
        pNode->SetIsInPath( FALSE );
        pNode = GetNextNode();
    }


    s32 count=0;
    while (tempID != SLOT_NULL)
    {
        if(GetNode( tempID )->GetPred() != SLOT_NULL)
            count++;

        GetNode( tempID )->SetIsInPath(true);
        tempID = GetNode( tempID )->GetPred();
    }

    tempID = m_CheapestNode;

    while((tempID != SLOT_NULL) && (count >= 0 ) )
    {
        thisPlan->AddPoint(GetNode( tempID )->GetConnectionUsedInSearch(), tempID, count);
        tempID = GetNode( tempID )->GetPred();
        count--;

    }

    if (tries > 0 )
    {
        thisPlan->SetCompletePath(true);
        return true;
    }
    else
    {
        thisPlan->SetCompletePath(false);
        return false;

    }

    x_catch_begin;

    thisPlan->SetCompletePath(false);
    return false;

    x_catch_end;
}

//=========================================================================

node_slot_id basenode_mgr::LocateNearestNode ( vector3&  thisPoint )
{
    x_try;

    f32 MinDistance = 999999999.0f;
    node_slot_id NearestSlot = SLOT_NULL;

    m_SearchCount++;

    
    //  SLOWEST POSSIBLE WAY TO DO THIS.  :P  Sorry, just for now.  Need to get a spatial
    //  Database in place for the real version or get these into the database.
    s32 count = 0;
    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );
    while( pNode )
    {
        if( pNode->IsInUse() )
        {
            f32 thisLength = ( pNode->GetPosition() - thisPoint).LengthSquared();
            if(thisLength < MinDistance )
            {
                MinDistance = thisLength;
                NearestSlot = count;
            }
        }
        
        pNode = GetNextNode();
        count++;
    }
    
    return NearestSlot;
    
    x_catch_begin;

    return SLOT_NULL;

    x_catch_end;
}

//=========================================================================

void basenode_mgr::LocateAllNodesInArea ( bbox& Area, xarray<base_node*>& Nodes, u32 flags )
{
    x_try;

    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );

    while( pNode )
    {
        if( pNode->IsInUse() )
        {
            if( flags & pNode->GetAttributes() )
            {            
                if( Area.Intersect( pNode->GetPosition(), 1 ) )
		        {
			        Nodes.Append( pNode );			
		        }
            }
        }
        pNode = GetNextNode();
    }

    x_catch_display;
}

//=========================================================================

void basenode_mgr::Init ( void )
{
    x_try;

    s32 count;
    for( count = 0; count < kMAX_STEPS; count++)
        m_CurrentPath[count] = SLOT_NULL;

    // Reset all the nodes.
    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );
    while( pNode )
    {
        pNode->Reset();
        pNode->SetSlotID( count );

        pNode = GetNextNode();
    }

    // Reset all the connections.
    SetStartConnectionIndex( 0 );
    base_connection* pCon = GetConnection( 0 );
    while( pCon )
    {
        pCon->Reset();

        pCon = GetNextConnection();
    }

    m_Destination.Zero();
    
    x_catch_display;
}

//=========================================================================

node_slot_id basenode_mgr::GetFirstOpenConnection ( void )
{
    s32 Index = 0;

    // Start from the first connection.
    SetStartConnectionIndex( 0 );
    base_connection* pCon = GetConnection( 0 );
    while( pCon )
    {
        if( pCon->GetStartNode() == SLOT_NULL ) 
            return Index;
        Index++;
        pCon = GetNextConnection();
    }

    return Index;
}

//=========================================================================

node_slot_id basenode_mgr::GetFirstOpenNode ( void )
{
    node_slot_id Index = 0;

    // Get the first node to check.
    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );
    while( pNode )
    {
        if( !(pNode->IsInUse() )  )
            return Index;

        Index++;
        pNode = GetNextNode();
    }

    return SLOT_NULL;
}

//=========================================================================

void basenode_mgr::InsertNodeIntoList ( node_slot_id thisNode, node_slot_id parent, node_slot_id connectionID  )
{
    x_try;

    //  Nodes are inserted in sorted order.  m_CheapestNode is the index to the current cheapest
    //  node in the list and is therefore the start of the list.  To inser the node we get it's
    //  cost and walk through the list until it's less than the next one or until the next index
    //  is SLOT_NULL

    //  if the cheapest node is SLOT_NULL then this is the first item to be entered.
    if ( m_CheapestNode == SLOT_NULL )
    {
        m_CheapestNode = thisNode;
        GetNode( thisNode )->SetSearchInfo( SLOT_NULL, parent,m_SearchCount, m_SearchStepNumber, GetNode( parent )->GetCostToNode() + GetConnection( connectionID )->GetCost() );
        return;

    }


    //  if we make it here then at least one item is already in the list.  We step through the list
    //  till we
    
    node_slot_id     m_HigherCostNode, 
                    nodeToInsertAfter;

    m_HigherCostNode = m_CheapestNode;
    nodeToInsertAfter = SLOT_NULL;

    f32 thisCost = GetNode( thisNode )->GetCostToNode();

    //  This while loop should walk through the list till it hits a node that is higher cost than
    //  itself or until it hits the end of the list.
    
    if(thisCost < GetNode( m_CheapestNode )->GetCostToNode() )
    {
        m_CheapestNode = thisNode;
    }

    while( m_HigherCostNode != SLOT_NULL &&  thisCost > GetNode( m_HigherCostNode )->GetCostToNode()  )
    {
        nodeToInsertAfter = m_HigherCostNode;
        m_HigherCostNode = GetNode( m_HigherCostNode )->GetNextInList();
    }

    //   if this is the new cheapest node...
    if(nodeToInsertAfter == SLOT_NULL )
    {

        m_CheapestNode = thisNode;
        GetNode( thisNode )->SetSearchInfo( m_HigherCostNode, parent,m_SearchCount, m_SearchStepNumber, GetNode( parent )->GetCostToNode() + GetConnection( connectionID )->GetCost() );

    }
    else //if(m_HigherCostNode != SLOT_NULL )
    {
        ASSERT(nodeToInsertAfter != SLOT_NULL );
        // to insert in the list we first make the previous point to this one
        GetNode( nodeToInsertAfter )->SetNextInList( thisNode );
        // then make this one point to the next one
        GetNode( thisNode )->SetSearchInfo( m_HigherCostNode, parent,m_SearchCount, m_SearchStepNumber, GetNode( parent )->GetCostToNode() + GetConnection( connectionID )->GetCost() );
    }

    x_catch_display;
}   

//=========================================================================

void basenode_mgr::FileIO ( fileio& File )
{
    // Get the first node to check.
    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );
    while( pNode )
    {
        pNode->FileIO( File );

        pNode = GetNextNode();
    }

    // Start from the first connection.
    SetStartConnectionIndex( 0 );
    base_connection* pCon = GetConnection( 0 );
    while( pCon )
    {
        pCon->FileIO( File );

        pCon = GetNextConnection();
    }
}

//=========================================================================

void basenode_mgr::LoadMap (const char* fileName )
{
    x_try;

    Init();
    X_FILE* tempFile = x_fopen(fileName, "rb" );

    s32 count = 0;

    if (tempFile)
    {

        // Start from the first connection.
        SetStartConnectionIndex( 0 );
        base_connection* pCon = GetConnection( 0 );
        while( pCon )
        {
            pCon->Load(tempFile);
            pCon = GetNextConnection();
        }

        // Get the first node to check.
        SetStartNodeIndex( 0 );
        base_node* pNode = GetNode( 0 );
        while( pNode )
        {
            pNode->Load(tempFile);
            pNode = GetNextNode();
        }

        x_fclose(tempFile);
        
        count = 0;

        // Get the first node to check.
        SetStartNodeIndex( 0 );
        pNode = GetNode( 0 );
        while( pNode )
        {
            if( !pNode->IsInUse() )
                break;
        
            guid aGuid = g_ObjMgr.CreateObject(nav_node_place_holder::GetObjectType());

            pNode->SetPlaceHolder(aGuid );

            object* tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
            nav_node_place_holder& aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

            aNNPH.SetNode( count );
        
            pNode = GetNextNode();
            count++;
        }

        count = 0;

        // Start from the first connection.
        SetStartConnectionIndex( 0 );
        pCon = GetConnection( 0 );
        while( pCon )
        {
            if( pCon->GetEndNode() == SLOT_NULL )
                break;

            guid node1, node2;

            node1 = GetNode( pCon->GetStartNode() )->GetPlaceHolder();
            node2 = GetNode( pCon->GetEndNode())->GetPlaceHolder();

            AddGuidToConnection(node1,count);
            AddGuidToConnection(node2,count);
            
            count++;
            pCon = GetNextConnection();
        }

        // Get the first node to check.
        SetStartNodeIndex( 0 );
        pNode = GetNode( 0 );
        while( pNode )
        {
            if( !pNode->IsInUse() )
                break;

            guid aGuid = pNode->GetPlaceHolder();

            object *tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
            nav_node_place_holder &aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

            aNNPH.OnMove( pNode->GetPosition() );

            pNode = GetNextNode();
        } 
    }

    x_catch_begin;

    // Delete all the place holders.

    // Get the first node.
    SetStartNodeIndex( 0 );
    base_node* pNode = GetNode( 0 );
    while( pNode )
    {
        if( !pNode->IsInUse() )
            break;
        g_ObjMgr.DestroyObject( pNode->GetPlaceHolder() );
    }

    // Start from the first connection.
    SetStartConnectionIndex( 0 );
    base_connection* pCon = GetConnection( 0 );
    while( pCon )
    {
        if( pCon->GetEndNode() == SLOT_NULL )
            break;

        g_ObjMgr.DestroyObject( pCon->GetPlaceHolder() );
    }

    Init();

    x_catch_end;
}

//=========================================================================

void basenode_mgr::SaveMap                 (const char* fileName )
{

    X_FILE* tempFile = x_fopen(fileName, "wb" );

    if (tempFile)
    {

        // Start from the first connection.
        SetStartConnectionIndex( 0 );
        base_connection* pCon = GetConnection( 0 );
        while( pCon )
        {
            pCon->Save(tempFile);
            pCon = GetNextConnection();

        }

        // Get the first node.
        SetStartNodeIndex( 0 );
        base_node* pNode = GetNode( 0 );
        while( pNode )
        {
            pNode->Save(tempFile);
            pNode = GetNextNode();
        }

        x_fclose(tempFile);
    }
}

//=========================================================================
/*
void basenode_mgr::StartConnectionCreation ( node_slot_id firstPoint )
{

}

//=========================================================================

void basenode_mgr::CompleteConnection      ( node_slot_id secondPoint )
{

}
*/
//=========================================================================

void basenode_mgr::AddConnectedNodesToOpenList( node_slot_id nearestNode )
{
    x_try;

    m_SearchStepNumber++;

    s32 count = 0;
    
    // Get the first node.
    
    for( count = 0; count < k_MAX_NODE_CONNECTIONS; count++ )
    {
        node_slot_id connectionID = GetNode( nearestNode )->GetConnectionByIndex( count );
        if(connectionID != SLOT_NULL )
        {
        
            node_slot_id nodeToAdd = GetConnection( connectionID )->GetOtherEnd(nearestNode);

            //  if it is not in this search 
            if( ( GetNode( nodeToAdd )->GetSearchNumber() != m_SearchCount) )
            {
                GetNode( nodeToAdd )->SetCost( GetNode( nearestNode )->GetCostToNode() + GetConnection( connectionID )->GetCost() );
                GetNode( nodeToAdd )->SetConnectionUsedInSearch( connectionID );
                InsertNodeIntoList( nodeToAdd, nearestNode, connectionID );
            }
            else if(  GetNode( nodeToAdd )->GetSearchStepNumber() == m_SearchStepNumber  )
            {
                //  if it is in the list already but it was from this same call to add nodes
                //  we need to check see which is cheaper and add the cheaper one
                //  FIXME!  
//                ASSERT(FALSE);
                x_DebugMsg("---Warning: 2 connects between the same 2 nodes.  Not fatal but not yet supported\n");
            
            }
            else
            {
                // else it is in the list and wasn't from this pass
//                ASSERT(FALSE);

            }
        }
    }

    x_catch_display;
}

//=========================================================================

void basenode_mgr::AddGuidToConnection ( guid aGuid, node_slot_id thisConnection)
{
    x_try;

    object* tempObject = g_ObjMgr.GetObjectByGuid(aGuid);
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
        }
        else 
        {
            // if we have one and they selected

            guid tempGuid = g_ObjMgr.CreateObject(nav_connection_place_holder::GetObjectType() );
            object* tempNNCPH = g_ObjMgr.GetObjectByGuid( tempGuid );
            nav_connection_place_holder &aNNCPH = nav_connection_place_holder::GetSafeType( *tempNNCPH);

            node_slot_id anID = thisConnection;
            
            aNNCPH.SetConnection( anID );

            GetConnection( anID )->SetPlaceHolder(aNNCPH.GetGuid());
            
            object* tempNNPH = g_ObjMgr.GetObjectBySlot( m_FirstSelectedObject );
            nav_node_place_holder& aNavNode1 = nav_node_place_holder::GetSafeType(*tempNNPH);

            object* tempObject2 = g_ObjMgr.GetObjectBySlot( tempObject->GetSlotID() );
            nav_node_place_holder& aNavNode2 = nav_node_place_holder::GetSafeType(*tempObject2);

            node_slot_id connectedNavNode = aNNCPH.GetConnection();

            base_connection* aNC = GetConnection( connectedNavNode );
            aNC->SetNodes( aNavNode1.GetNode() , aNavNode2.GetNode() );

            base_node* aNN1 = GetNode( aNavNode1.GetNode() );
            base_node* aNN2 = GetNode (aNavNode2.GetNode() );

            aNN1->AddConnection( connectedNavNode );
            aNN2->AddConnection( connectedNavNode );

            aNNCPH.OnMove( aNC->GetPosition() );
            
            m_FirstSelectedObject = SLOT_NULL;
        }
    }

    x_catch_display;
}

//=========================================================================

void basenode_mgr::SetTestStart ( guid thisNode )
{
    object *tempNNCPH = g_ObjMgr.GetObjectByGuid( thisNode);
    if(tempNNCPH && tempNNCPH->GetType() == object::TYPE_NAV_NODE_PLACE_HOLDER )
    {
        nav_node_place_holder &aNNPH = nav_node_place_holder ::GetSafeType( *tempNNCPH);

        m_TestStart = aNNPH.GetNode();
    }
}

//=========================================================================

void basenode_mgr::SetTestEnd ( guid thisNode )
{
    object *tempNNCPH = g_ObjMgr.GetObjectByGuid( thisNode);
    if(tempNNCPH && tempNNCPH->GetType() == object::TYPE_NAV_NODE_PLACE_HOLDER )
    {
        nav_node_place_holder &aNNPH = nav_node_place_holder ::GetSafeType( *tempNNCPH);

        m_TestEnd = aNNPH.GetNode();
    }
}

//=========================================================================
