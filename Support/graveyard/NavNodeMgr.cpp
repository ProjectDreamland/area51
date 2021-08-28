///////////////////////////////////////////////////////////////////////////////////////////////////
// BaseNodeMgr.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "NavNodeMgr.hpp"
#include "navigation\nav_plan.hpp"
#include "objects\nav_node_place_holder.hpp"
#include "objects\nav_connection_place_holder.hpp"
navnode_mgr g_NavMgr;

//=========================================================================

navnode_mgr::navnode_mgr (void) : basenode_mgr()
{

    Init();
}

//=========================================================================

navnode_mgr::~navnode_mgr (void)
{

}

//=========================================================================

void navnode_mgr::Init ( void )
{
    m_HighestNode = 0;
    basenode_mgr::Init();
}

//=========================================================================

base_node* navnode_mgr::GetNode ( node_slot_id NodeIndex )
{
    if( (NodeIndex >= kMAX_CONNECTIONS) || (NodeIndex < 0) )
    {
        x_DebugMsg( "The NavNode Index is greater or less than than the size of the array." );
        return NULL;
    }

    
    return (base_node*)&m_NavNodes[ NodeIndex ];
}

//=========================================================================

base_node* navnode_mgr::GetNextNode ( void )
{
    m_CurrentNode++;

    if( m_CurrentNode >= kMAX_CONNECTIONS )
    {
        m_CurrentNode = kMAX_CONNECTIONS-1;
        return NULL;
    }

    return (base_node*)&m_NavNodes[ m_CurrentNode ];
}

//=========================================================================

base_node* navnode_mgr::GetPrevNode ( void )
{
    m_CurrentNode--;

    if( m_CurrentNode <= 0 )
    {
        m_CurrentNode = 0;
        return NULL;
    }

    return (base_node*)&m_NavNodes[ m_CurrentNode ];
}

//=========================================================================

base_connection* navnode_mgr::GetConnection ( node_slot_id ConnectionIndex )
{
    if( (ConnectionIndex >= kMAX_CONNECTIONS) || (ConnectionIndex < 0) )
        x_throw( "The NavConnection Index is greater or less than than the size of the array." );

    return (base_connection*)&m_Connections[ ConnectionIndex ];
}

//=========================================================================

base_connection* navnode_mgr::GetNextConnection ( void )
{
    m_CurrentConnection++;

    if( m_CurrentConnection >= kMAX_CONNECTIONS )
    {
        m_CurrentConnection = kMAX_CONNECTIONS-1;
        return NULL;
    }

    return (base_connection*)&m_Connections[ m_CurrentConnection ];
}

//=========================================================================

base_connection* navnode_mgr::GetPrevConnection ( void )
{
    m_CurrentConnection--;

    if( m_CurrentConnection <= 0 )
    {
        m_CurrentConnection = 0;
        return NULL;
    }

    return (base_connection*)&m_Connections[ m_CurrentConnection ];
}

//=========================================================================

node_slot_id navnode_mgr::GetNewNode ( void )
{
    node_slot_id anId = GetFirstOpenNode();
    m_NavNodes[anId].Reset();
    
    m_NavNodes[anId].SetIsInUse();
    return anId;
}

//=========================================================================

node_slot_id navnode_mgr::GetNewConnection ( void )
{
    node_slot_id anId = GetFirstOpenConnection();
    m_Connections[anId ].Reset();

    return anId;
}

//=========================================================================

void navnode_mgr::DeleteNode ( slot_id  thisNavNode    )
{
    ASSERT(thisNavNode != SLOT_NULL);
    m_NavNodes[thisNavNode].Reset();
}

//=========================================================================

void navnode_mgr::DeleteConnection ( slot_id  thisConnection )
{
    ASSERT(thisConnection != SLOT_NULL );

    m_NavNodes[m_Connections[thisConnection].GetStartNode()].DeleteConnection(thisConnection);
    m_NavNodes[m_Connections[thisConnection].GetEndNode()].DeleteConnection(thisConnection);

    m_Connections[thisConnection].Reset();
}

//=========================================================================

void navnode_mgr::TestPath (void)
{
    if( m_TestStart != SLOT_NULL && m_TestEnd != SLOT_NULL )
    {
        nav_plan aPlan;
        aPlan.SetStartingPoint( m_TestStart);
        aPlan.SetDestination( GetNode( m_TestEnd )->GetPosition() );

        UpdatePlan( GetNode( m_TestEnd )->GetPosition() , (base_plan*)&aPlan );
    }
}

//=========================================================================
//  Comparison function for the quicksort call
//=========================================================================
s32 NodeCompareXFN( const void* pA, const void* pB )
{
    node_slot_id &firstNode  = *(node_slot_id*)pA;
    node_slot_id &secondNode = *(node_slot_id*)pB;
    
    ASSERT(firstNode  >= 0 && firstNode  <= kMAX_NAV_NODES );
    ASSERT(secondNode >= 0 && secondNode <= kMAX_NAV_NODES );

    ASSERT( g_NavMgr.GetNode( firstNode  )->IsInUse() );
    ASSERT( g_NavMgr.GetNode( secondNode )->IsInUse() );

    f32 X1 = g_NavMgr.GetNode(firstNode )->GetPosition().X;
    f32 X2 = g_NavMgr.GetNode(secondNode)->GetPosition().X;

    if ( X1 > X2 )
    {
        return ( 1 );
    }
    else if ( X1 < X2 )
    {
        return ( -1 );
    }
    else
    {
        return ( 0 );
    }
    
}

//=========================================================================
//  Comparison function for the quicksort call
//=========================================================================
s32 NodeCompareYFN( const void* pA, const void* pB )
{
    node_slot_id &firstNode  = *(node_slot_id*)pA;
    node_slot_id &secondNode = *(node_slot_id*)pB;
    
    ASSERT(firstNode  >= 0 && firstNode  <= kMAX_NAV_NODES );
    ASSERT(secondNode >= 0 && secondNode <= kMAX_NAV_NODES );

    ASSERT( g_NavMgr.GetNode( firstNode  )->IsInUse() );
    ASSERT( g_NavMgr.GetNode( secondNode )->IsInUse() );

    f32 Y1 = g_NavMgr.GetNode(firstNode )->GetPosition().Y;
    f32 Y2 = g_NavMgr.GetNode(secondNode)->GetPosition().Y;

    if ( Y1 > Y2 )
    {
        return ( 1 );
    }
    else if ( Y1 < Y2 )
    {
        return ( -1 );
    }
    else
    {
        return ( 0 );
    }
    
}


//=========================================================================
void navnode_mgr::ResortNodes ( void )
{ 
    node_slot_id nodeCount;
    node_slot_id highestNode = 0;

    s32 sortedListSlot = 0;
    //  First task is to reset them all to first guess values
    for(nodeCount = 0; nodeCount < kMAX_NAV_NODES; nodeCount++)
    {
        if(m_NavNodes[nodeCount].IsInUse() )
        {
            m_SortedList[sortedListSlot] = nodeCount;
            sortedListSlot++;
        }
    }

    m_HighestNode = sortedListSlot ;
    if( highestNode == 0 )
    {
        return;
    }

    //  Next we go through the list n^2 times

    x_qsort(    &(m_SortedList[0]), 
                highestNode,
                sizeof(node_slot_id),
                NodeCompareXFN          );

      

}


//=========================================================================

void  navnode_mgr::LoadMap (const char* fileName )
{

    X_FILE* tempFile = x_fopen(fileName, "rb" );

    if (tempFile)
    {
        // write out the version number first
        s32 versionNumber;
        x_fread( &versionNumber,sizeof(s32),1, tempFile );
        if( versionNumber == 100 )
        {
    
            s32 connectionCount;
            x_fread( &connectionCount, sizeof(s32),1,tempFile );
            if( connectionCount != kMAX_CONNECTIONS )
            {
                x_throw("Different number of nodes in the map file and the map loader");
            }

            s32 nodeCount;

            x_fread( &nodeCount, sizeof(s32),1,tempFile );
            if( nodeCount != kMAX_NAV_NODES )
            {
                x_throw("Different number of nodes in the map file and the map loader");
            }

            s32 totalNodeDataSize =0;
            s32 nodeDataSize;
            nodeDataSize = m_NavNodes[0].GetDataSize( );

            totalNodeDataSize = nodeDataSize * nodeCount;

            byte* tempBuffer = (byte*)x_malloc( totalNodeDataSize );
            ASSERT(tempBuffer);
            
            s32 bytesRead;
            bytesRead = x_fread(tempBuffer,nodeDataSize,nodeCount,tempFile );

            ASSERT(bytesRead == nodeCount );

            s32 count;
            for(count = 0; count < nodeCount; count++ )
            {
                m_NavNodes[count].SetData(&(tempBuffer[count*nodeDataSize] ) );
                m_NavNodes[count].SetSlotID( count );
            }

            x_free( tempBuffer );



            s32 totalConnectionDataSize =0;
            s32 connectionDataSize;
            connectionDataSize = m_Connections[0].GetDataSize( );

            totalConnectionDataSize = connectionDataSize * connectionCount;

            tempBuffer = (byte*)x_malloc( totalConnectionDataSize );
            ASSERT(tempBuffer);
            
//            bytesRead;
            bytesRead = x_fread(tempBuffer,connectionDataSize,connectionCount,tempFile );

            ASSERT(bytesRead == connectionCount );

            for(count = 0; count < nodeCount; count++ )
            {
                m_Connections[count].SetData(&(tempBuffer[count*connectionDataSize] ) );

            }

            x_free( tempBuffer );

            base_node* pNode;

#ifdef WIN32             
            // Get the first node to check.
            SetStartNodeIndex( 0 );
            for( count = 0; count < kMAX_NAV_NODES ; count++ )
            {
                pNode = GetNode( count );

                if( pNode->IsInUse() )
                {
                

                    guid aGuid = g_ObjMgr.CreateObject(nav_node_place_holder::GetObjectType());

                    pNode->SetPlaceHolder(aGuid );

                    object* tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
                    nav_node_place_holder& aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

                    aNNPH.SetNode( count );        

                }

            }

            count = 0;

            base_connection* pCon;
            // Start from the first connection.
            SetStartConnectionIndex( 0 );
            for( count = 0; count < kMAX_CONNECTIONS ; count++ )
            {
                pCon = GetConnection( count );

                if( pCon->GetEndNode() != SLOT_NULL )
                {
                
                    guid node1, node2;

                    node1 = GetNode( pCon->GetStartNode() )->GetPlaceHolder();
                    node2 = GetNode( pCon->GetEndNode())->GetPlaceHolder();

                    AddGuidToConnection(node1,count);
                    AddGuidToConnection(node2,count);
                }


            }

            // Get the first node to check.
            SetStartNodeIndex( 0 );
            for( count = 0; count < kMAX_NAV_NODES ; count++ )
            {
                pNode = GetNode( count );
                if( pNode->IsInUse() )
                {
                
                    guid aGuid = pNode->GetPlaceHolder();

                    object *tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
                    nav_node_place_holder &aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

                    aNNPH.OnMove( pNode->GetPosition() );

                    pNode = GetNextNode();
                }
            } 

#endif//WIN32



        }
        else
        {

            x_fwrite( &kMAX_CONNECTIONS, sizeof(s32),1,tempFile );
            x_fwrite( &kMAX_NAV_NODES, sizeof(s32),1, tempFile );            


            // write out the nodes first
            s32 packedDataSize =0;
            s32 count;
            for( count = 0; count < kMAX_NAV_NODES ; count++)
            {
                if ( m_NavNodes[count].IsInUse() )
                {
                    packedDataSize += m_NavNodes[count].GetDataSize( );
                }
            }

            byte* tempBuffer = (byte*)x_malloc( packedDataSize );

            ASSERT(tempBuffer);

            s32 currentIndex = 0;
            for( count = 0; count < kMAX_NAV_NODES ; count++)
            {
                if ( m_NavNodes[count].IsInUse() )
                {
                    m_NavNodes[count].AddData(&tempBuffer[currentIndex] );
                    currentIndex += m_NavNodes[count].GetDataSize( );
                }
            }
        
            x_fwrite( tempBuffer, 1, packedDataSize, tempFile );
            x_free( tempBuffer );



            // now write out the connections
            packedDataSize = 0;
            for( count = 0; count < kMAX_CONNECTIONS; count++)
            {
                packedDataSize += m_Connections[count].GetDataSize( );
            }
            tempBuffer = (byte*)x_malloc( packedDataSize );
            ASSERT(tempBuffer);
            currentIndex = 0;
            for( count = 0; count < kMAX_CONNECTIONS ; count++)
            {
                m_Connections[count].AddData(&tempBuffer[currentIndex] );
                currentIndex += m_Connections[count].GetDataSize( );
            }
        
            x_fwrite( tempBuffer, 1, packedDataSize, tempFile );
            x_free( tempBuffer );

            x_fclose(tempFile);



        }

        ResortNodes();
    }

}


void  navnode_mgr::SaveMap (const char* fileName )
{
   
    X_FILE* tempFile = x_fopen(fileName, "wb" );

    if (tempFile)
    {
        // write out the version number first
        x_fwrite( &kNAV_MAP_VERSION_NUMBER,sizeof(s32),1, tempFile );
        x_fwrite( &kMAX_CONNECTIONS, sizeof(s32),1,tempFile );
        x_fwrite( &kMAX_NAV_NODES, sizeof(s32),1, tempFile );            


        // write out the nodes first
        s32 packedDataSize =0;
        s32 count;
        for( count = 0; count < kMAX_NAV_NODES ; count++)
        {
//            if ( m_NavNodes[count].IsInUse() )
            {
                packedDataSize += m_NavNodes[count].GetDataSize( );
            }
        }

        byte* tempBuffer = (byte*)x_malloc( packedDataSize );

        ASSERT(tempBuffer);

        s32 currentIndex = 0;
        for( count = 0; count < kMAX_NAV_NODES ; count++)
        {
//            if ( m_NavNodes[count].IsInUse() )
            {
                m_NavNodes[count].AddData(&tempBuffer[currentIndex] );
                currentIndex += m_NavNodes[count].GetDataSize( );
            }
        }
        
        x_fwrite( tempBuffer, 1, packedDataSize, tempFile );
        x_free( tempBuffer );



        // now write out the connections
        packedDataSize = 0;
        for( count = 0; count < kMAX_CONNECTIONS; count++)
        {
            packedDataSize += m_Connections[count].GetDataSize( );
        }
        tempBuffer = (byte*)x_malloc( packedDataSize );
        ASSERT(tempBuffer);
        currentIndex = 0;
        for( count = 0; count < kMAX_CONNECTIONS ; count++)
        {
            m_Connections[count].AddData(&tempBuffer[currentIndex] );
            currentIndex += m_Connections[count].GetDataSize( );
        }
        
        x_fwrite( tempBuffer, 1, packedDataSize, tempFile );
        x_free( tempBuffer );

        x_fclose(tempFile);
    }


    
}




//=================================================================================================

node_slot_id navnode_mgr::GetNearestPatrolNode ( vector3 thisPoint )
{



    f32 MinDistance = 999999999.0f;
    node_slot_id NearestSlot = SLOT_NULL;

    m_SearchCount++;

    
    //  SLOWEST POSSIBLE WAY TO DO THIS.  :P  Sorry, just for now.  Need to get a spatial
    //  Database in place for the real version or get these into the database.
    s32 count = 0;
    SetStartNodeIndex( 0 );
    for(count=0;count< kMAX_CONNECTIONS; count++)
    {
        if( m_Connections[count].GetStartNode() != SLOT_NULL && m_Connections[count].GetHints() & nav_connection::HINT_PATROL_ROUTE )
        {
            f32 thisLength = ( g_NavMgr.GetNode(m_Connections[count].GetStartNode())->GetPosition() - thisPoint).LengthSquared();
            if(thisLength < MinDistance )
            {
                MinDistance = thisLength;
                NearestSlot = count;
            }
        }
    }

    // Found?
    if (NearestSlot != SLOT_NULL)
        return ( m_Connections[NearestSlot].GetStartNode() );
    else
        return SLOT_NULL ;
}

//=================================================================================================

// Chooses new patrol node. CurrPatrolNode = node that npc is at, bForward = path search direction
node_slot_id navnode_mgr::GetNextPatrolNode( node_slot_id CurrPatrolNode, xbool bForward /*= TRUE*/ )
{
    // Create list of valid nodes since there maybe be multiple patrol connections
    node_slot_id    List[k_MAX_NODE_CONNECTIONS] ;
    s32             Count = 0 ;

    // Get current node
    base_node* pNode = GetNode(CurrPatrolNode) ;
    ASSERT(pNode) ;

    // Could take 2 passes if first direction attempt fails
    for (s32 Pass = 0 ; Pass < 2 ; Pass++)
    {
        // Check all connections
        for (s32 i = 0 ; i < pNode->GetConnectionCount() ; i++)
        {
            // Get connection
            node_slot_id    Connection  = pNode->GetConnectionByIndex(i) ;
            nav_connection* pConnection = (nav_connection*)GetConnection(Connection) ;
            ASSERT(pConnection) ;

            // Make sure we don't overrun local array!
            ASSERT(Count < k_MAX_NODE_CONNECTIONS) ; 

            // Is this a patrol connection?
            if (pConnection->GetHints() & nav_connection::HINT_PATROL_ROUTE)
            {
                // Choose forward node?
                if (bForward)
                {
                    // Try end node (forward direction)
                    if (CurrPatrolNode != pConnection->GetEndNode())
                        List[Count++] = pConnection->GetEndNode() ;
                }
                else
                {
                    // Try start node (reverse direction)
                    if (CurrPatrolNode != pConnection->GetStartNode())
                        List[Count++] = pConnection->GetStartNode() ;
                }
            }
        }

        // If list has entries, randomly pick one
        if (Count)
            return List[x_irand(0, Count-1)] ;

        // No nodes found so toggle direction for 2nd pass
        bForward ^= TRUE ;
    }

    // Failed to find a patrol node so just return current node
    return CurrPatrolNode ;
}

//=================================================================================================


node_slot_id navnode_mgr::LocateNearestNode ( vector3&  thisPoint )
{
    x_try;

    f32 MinDistance = 999999999.0f;
    node_slot_id nearestSlot = SLOT_NULL;
    const s32 kSlotsToTry = 30;
    const s32 kStepSize = 10;

    //  First we find the nearest slot in the sorted array +/- stepSize
    s32 count;
    s32 indexToNearestSortedSlot = 0;
    f32 nearestDistance = 999999999.0f;
    for( count = 0; count < m_HighestNode; count += kStepSize )
    {
        //   This should give us the distance on the X-axis to point
        f32 distanceX = x_abs( m_NavNodes[m_SortedList[count]].GetPosition().X - thisPoint.X );

        //  if it's greater than nearest distance then it is increasing
        if( distanceX > nearestDistance )
        {
            break;
        }
        else
        {
            nearestDistance = distanceX ;
            indexToNearestSortedSlot = count;
        }

    }

   
    if( nearestDistance > 5000.0f )
    {
        x_DebugMsg("Nearest node is more than 5000.0f units away!  Probable cause, no nav map but AI's are present" );
        return SLOT_NULL;
    }
    
    nearestDistance = 9999999999.0f;
    s32 slotToTry;

    // Now we start with this point and step up and down the list 
    count = 0;
    // HANDLE the first case of zero
    if( indexToNearestSortedSlot >=0 && indexToNearestSortedSlot <= m_HighestNode )
    {
        f32 thisDistance = (m_NavNodes[m_SortedList[indexToNearestSortedSlot]].GetPosition() - thisPoint).LengthSquared();
        if( thisDistance < nearestDistance )
        {
            nearestSlot = m_SortedList[indexToNearestSortedSlot];
            nearestDistance = thisDistance;
        }
    }

    // now check all the other cases up and down from this point
    for( count = 1; count < (kSlotsToTry+ kStepSize)/2; count++ )
    {
        slotToTry = indexToNearestSortedSlot + count;
        if( slotToTry >=0 && slotToTry <= m_HighestNode )
        {
            f32 thisDistance = (m_NavNodes[m_SortedList[slotToTry]].GetPosition() - thisPoint).LengthSquared();
            if( thisDistance < nearestDistance )
            {
                nearestSlot = m_SortedList[slotToTry];
                nearestDistance = thisDistance;
            }
        }

        slotToTry = indexToNearestSortedSlot - count;
        if( slotToTry >=0 && slotToTry <= m_HighestNode )
        {
            f32 thisDistance = (m_NavNodes[m_SortedList[slotToTry]].GetPosition() - thisPoint).LengthSquared();
            if( thisDistance < nearestDistance )
            {
                nearestSlot = m_SortedList[slotToTry];
                nearestDistance = thisDistance;
            }
        }
    }

    
    if( nearestSlot == SLOT_NULL )
    {
        x_DebugMsg("No nearest node found in nav search\n");
    }

    if( x_abs( m_NavNodes[nearestSlot].GetPosition().Y - thisPoint.Y )  > 200.0f  )
    {
        x_DebugMsg("Nearest nav node was likely on the level either above or below us.  Likely need more nav points in this area.\n");
    }

    if( nearestDistance < 250000.0f)
    {
        return nearestSlot;
    }
    
    m_SearchCount++;

    node_slot_id verifyNearestSlot = 0;

    //  SLOWEST POSSIBLE WAY TO DO THIS.  :P  Sorry, just for now.  Need to get a spatial
    //  Database in place for the real version or get these into the database.
    count = 0;
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
                verifyNearestSlot = count;
            }
        }
        
        pNode = GetNextNode();
        count++;
    }

//    ASSERT( nearestSlot == verifyNearestSlot );

    return verifyNearestSlot;
    
    x_catch_begin;

    return SLOT_NULL;

    x_catch_end;



}



