#include "AStar.hpp"
#include "Entropy.hpp"
#include "Obj_mgr/obj_mgr.hpp"

static const f32 k_DefaultScale = 1.0f;

//===============================================================================================================

astar_path_finder::astar_path_finder( void ) :
    m_CurrentNPCGuid( 0 ),
    m_GoalNodeID( NULL_NAV_SLOT ),
    m_PathIndex( -1 ),
    m_NumConnections( 0 )
{
}

//===============================================================================================================

xbool astar_path_finder::GeneratePath( ng_connection2*      pStartConnection, 
                                       ng_connection2*      pEndConnection,
                                       const vector3&       DestPoint, 
                                       guid                 NPCGuid, 
                                       const pathing_hints& Hints,              // IN
                                       s32*                 PathList, 
                                       s32                  PathCount,
                                       s32&                 nStepsInPath )    
{
    CONTEXT( "astar_path_finder::GeneratePath" );

    //clear the open list
    m_OpenList.Clear();

    if (NULL == pStartConnection)
        return FALSE;
   
    //initialize the goal node and NPC
    m_GoalNodeID = pStartConnection->GetSlotID();
    m_CurrentNPCGuid = NPCGuid;      

    object* pObject = g_ObjMgr.GetObjectByGuid( NPCGuid );
    ASSERTS(pObject,"astar_path_finder was not given a valid NPCGuid");

    vector3 StartPos = DestPoint;
    m_DestPoint = pObject->GetPosition();

    //set up the initial node, generate the path backwards the final node is actually the beginning node.
    AddToOpen( pEndConnection, StartPos, 
               NULL, StartPos, 
               pStartConnection, 
               -1, 1.0f );

    while( !m_OpenList.IsEmpty() )
    {
        //get a pointer to the node that we're analyzing
        s32                     CurIndex         = m_OpenList.Pop();
        astar_node&             CurrentAStarNode = m_NodeList[ CurIndex ];
        const ng_connection2*   pCurConnection   = CurrentAStarNode.m_pNavConnection;

        //if pCurrentNode is the goal, stop
        if ( CurrentAStarNode.m_pNavConnection->GetSlotID() == m_GoalNodeID )
        {
            m_PathIndex = CurIndex;
            //pass the path back to the NPC
            if ( PathList != NULL )
            {
                nStepsInPath = CreatePath( PathList , PathCount );
            }
            
            ResetNodeList();
            return TRUE;
        }

        // Get data required to process child connections:
        vector3 CurConCenterLine(0,0,0);
        xbool   bOneWay = pCurConnection->IsOneWay();
        
        if (bOneWay)
        {
            CurConCenterLine = pCurConnection->GetEndPosition() - pCurConnection->GetStartPosition();
        }


        //walk through the connections to the current node.
        for( u16 i = 0; i < CurrentAStarNode.m_pNavConnection->GetOverlapCount(); i++ )
        {
            nav_connection_slot_id  OtherID         = CurrentAStarNode.m_pNavConnection->GetOverlapRemoteConnectionID(i);
            ng_connection2&         OtherConnection = g_NavMap.GetConnectionByID( OtherID );
            
            //if the connected node is open or closed, don't do anything with it.
            //we will skip deactivated connections also.
            if( OtherConnection.IsClosed() || OtherConnection.IsOpen() || !OtherConnection.GetEnabled() )
            {
                continue;
            }
                        
            f32 Multiplier = CalculateMultiplierForConnection( OtherConnection, Hints );
            if (Multiplier == 0)
            {
                continue;
            }

            nav_node_slot_id        OtherNodeID     = CurrentAStarNode.m_pNavConnection->GetOverlapNodeID(i);
            ng_node2&               OtherNode       = g_NavMap.GetNodeByID( OtherNodeID );

            if (bOneWay)
            {
                // We dot the current connection's center line vector with
                // a vector from the current node's position to the other node's position.
                // If the dot product of the two vectors is less than or equal to zero,
                // then travelling to the other node would take us against the
                // direction of the nav connection.
                vector3 DeltaToOtherConnection = CurrentAStarNode.m_Position - OtherNode.GetPosition();
                f32     Dot = DeltaToOtherConnection.Dot( CurConCenterLine );
                if (Dot <= 0)
                    continue;
            }

            if (OtherConnection.IsOneWay() && (OtherID == m_GoalNodeID))
            {
                // Same as above
                vector3 DeltaFromCurConnection  = OtherNode.GetPosition() - m_DestPoint;
                vector3 OtherConCenterLine      = OtherConnection.GetEndPosition() - OtherConnection.GetStartPosition();
                f32     Dot = DeltaFromCurConnection.Dot( OtherConCenterLine );
                if( Dot <= 0 && !OtherNode.IsPointInside(m_DestPoint) )
                    continue;
            }

                        //otherwise, add the connected node to our list
            AddToOpen( &OtherConnection, 
                       OtherNode.GetPosition(),
                       CurrentAStarNode.m_pNavConnection,
                       CurrentAStarNode.m_Position, 
                       pStartConnection, 
                       CurIndex, 
                       Multiplier );
        }

        CurrentAStarNode.SetClosed();
    }

    ResetNodeList();

    return FALSE;
}

//===============================================================================================================

f32 astar_path_finder::CalculateMultiplierForConnection( const ng_connection2&     Conn, 
                                                         const pathing_hints&      Hints )
{
    u32 Flags = Conn.GetFlags();

    f32 Mult = 1.0f;

    // Setup default mult
    Mult = (2.0f - (Hints.Default / 128.0f)) * k_DefaultScale;

    //
    //  PROCESS JUMP FLAGS
    //
    if (Flags & ng_connection2::HINT_JUMP)
    {
        if (Hints.Jump == 0)
            return 0;

        f32 ThisMult = (2.0f - (Hints.Jump / 128.0f)) * k_DefaultScale;

        Mult = MIN(Mult,ThisMult);
    }

    //
    //  SMALL NPC
    //
    if (Flags & ng_connection2::HINT_SMALL_NPC)
    {
        if (Hints.SmallNPC == 0)
            return 0;

        f32 ThisMult = (2.0f - (Hints.SmallNPC / 128.0f)) * k_DefaultScale;

        Mult = MIN(Mult,ThisMult);
    }
    return Mult;
}

//===============================================================================================================
/*
xbool astar_path_finder::GenerateRunFrom(ng_node* pStartNode, vector3& RunPos, s32 Depth, s32* PathList, s32 PathCount )
{
    CONTEXT( "astar_path_finder::GenerateRunFrom" ) ;

    // Clear the open list.
    m_OpenList.Clear() ;
    ResetNodeList() ;

    // Push the initial node onto the run from list.
    AddToRunFrom( pStartNode ) ;

    for ( s32 i = 0; i < Depth; i++ )
    {
        s32 NodeCount = m_NumConnections ;

        // Add the connected nodes to the list.
        for ( s32 j = 0; j < NodeCount; j++ )
        {
            astar_node& CurrentAStarNode = m_NodeList[ j ] ;
            const u16 SlotID        = CurrentAStarNode.m_pNavNode->GetSlotID();
            ng_node&  ConnectedNode = CurrentAStarNode.m_pNavNode->GetConnectionByIndex(i).GetOtherNode( SlotID );

            //if the connected node is open or closed, don't do anything with it.
            if( ConnectedNode.IsClosed() || ConnectedNode.IsOpen() )
            {
                continue;
            }

            //otherwise, add the connected node to our list
            AddToRunFrom( &ConnectedNode );            
        }
    }

    // If there's only one node, we can't run from anything.
    if ( m_NumConnections == 1 )
    {
        return FALSE ;
    }

    // OK.  We are ready to analyze the list of nodes.
    u16 DestinationIndex = AnalyzeRunFromList( RunPos ) ;
}
*/
//===============================================================================================================

void astar_path_finder::AddToOpen( ng_connection2* pCurrentConnection,      // Current connection (travelling into this)
                                   const vector3&  CurrentPos,              // center of overlap used to get to current connection
                                   ng_connection2* pPrevConnection,         // Previous connection
                                   const vector3&  PrevPos,                 // center of overlap used to get into the previous connection
                                   ng_connection2* pEndConnection,          // Terminal connection.  When we get here we're done
                                   s32             ParentIndex,             
                                   f32             Multiplier )
{
    ASSERT( pCurrentConnection );
    ASSERT( m_NumConnections < MAX_NODES );
    
    m_NodeList[m_NumConnections].Initialize( pCurrentConnection,
                                             CurrentPos,
                                             pPrevConnection,
                                             PrevPos,
                                             pEndConnection, 
                                             ParentIndex, 
                                             m_DestPoint );
    m_NodeList[m_NumConnections].SetOpen();

    //push the initial node onto the queue
    m_OpenList.Push( m_NumConnections , m_NodeList[m_NumConnections].GetCostAndEstimate() * Multiplier );
    m_NumConnections++;
}

//===============================================================================================================
/*
void astar_path_finder::AddToRunFrom( ng_node* pCurrentNode )
{
    ASSERT( pCurrentNode );
    ASSERT( m_NumConnections < MAX_NODES );
    
    m_NodeList[m_NumConnections].SetOpen();

    //push the node onto the queue
    m_OpenList.Push( m_NumConnections , 0.f );
    m_NumConnections++;
}

//===============================================================================================================

u16 astar_path_finder::AnalyzeRunFromList( const vector3& RunPos )
{
    // Walk through the list and find the connected node that is furthest away from where we want to run from

}
*/
//===============================================================================================================

void astar_path_finder::ResetNodeList( void )
{
    //reset each node in our pool.
    for ( s32 i = 0 ; i < m_NumConnections ; i++ )
    {
        m_NodeList[i].ResetNode();
    }

    m_NumConnections = 0;
    m_PathIndex = -1;
}

//===============================================================================================================

void astar_path_finder::ResetNumNodes( void )
{
    m_NumConnections = 0;
    m_PathIndex = -1;
}

//===============================================================================================================

s32  astar_path_finder::CreatePath( s32* PathList, s32 PathCount )
{
    //we only create a path when one has been found.  ie:  m_PathIndex >=0
    s32 CurrentPathIndex = m_PathIndex;
    s32 i;
    for ( i = 0 ; i < PathCount ; i++ )
    {
        PathList[i] = -1;
    }

    ASSERT( CurrentPathIndex >= 0 );

    astar_node* pPathNode = &m_NodeList[ CurrentPathIndex ];

    s32 nSteps = 0;

    for ( i = 0 ; i < PathCount && CurrentPathIndex >= 0 ; i++ )
    {
        //store the slot ID in the path list
        PathList[i] = pPathNode->m_pNavConnection->GetSlotID();
        nSteps++;
        CurrentPathIndex = pPathNode->m_ParentIndex;
        
        if ( CurrentPathIndex >= 0 )
        {
            pPathNode = &m_NodeList[ CurrentPathIndex ];
        }
    }
    return nSteps;
}

#ifdef X_EDITOR

void astar_path_finder::RenderPath( void )
{
    /*
#ifdef TARGET_PC
    //walk through our pooled nodes and render spheres around their positions
    for ( s32 i = 0 ; i < m_NumConnections ; i++ )
    {
        vector3 vPos = m_NodeList[i].m_Position;
        xcolor  cColor;
        if ( m_NodeList[i].IsOpen() )
        {
            cColor = XCOLOR_RED;
        }
        else
        {
            cColor = XCOLOR_BLUE;
        }

        draw_Sphere( vPos , 50.f , cColor );
    }

    //now, if a path has been found, we draw a green line on the path
    if ( m_PathIndex >= 0 )
    {
        s32         CurrentIndex = m_PathIndex;
        astar_node* pPathNode = &m_NodeList[ CurrentIndex ];
        vector3 vOffset( 0.f , 10.f , 0.f );
        while ( pPathNode->m_pParentNode )
        {
            char buf[64];
            draw_Line( vOffset + pPathNode->m_pNavNode->GetPosition() , vOffset + pPathNode->m_pParentNode->GetPosition() , XCOLOR_GREEN );
            x_sprintf( buf , "NodeID: %u" , pPathNode->m_pNavNode->GetSlotID() );
            draw_Label( 1.5f * vOffset + pPathNode->m_pNavNode->GetPosition() , XCOLOR_WHITE , buf );
            CurrentIndex = pPathNode->m_ParentIndex;
            ASSERT( CurrentIndex >= 0 );
            pPathNode = &m_NodeList[ CurrentIndex ];
        }

        char buf[64];
        x_sprintf( buf , "GOAL NODE ID: %u" , pPathNode->m_pNavNode->GetSlotID() );
        draw_Label( 1.5f * vOffset + pPathNode->m_pNavNode->GetPosition() , XCOLOR_YELLOW , buf ); 
    }
#endif
*/
}

#endif // X_EDITOR