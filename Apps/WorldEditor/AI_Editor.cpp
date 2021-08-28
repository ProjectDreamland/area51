///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AI_editor.hpp
//
//      pass through class to shield the editorview from any game specific stuff
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ai_editor.hpp"
//#include "obj_mgr\obj_mgr.hpp"

#include "objects\player.hpp"
#include "navigation\nav_map.hpp"
#include "Navigation\ng_connection2.hpp"
//#include "Navigation\CoverNode.hpp"
#include "nav_connection2_editor.hpp"
#include "nav_connection2_anchor.hpp"
//#include "Characters\God.hpp"
#include "AI\AIMgr.hpp"
#include "Characters\God.hpp"


ai_editor*  ai_editor::s_This = NULL;

const f32 k_MaxDistanceBetweenNodes = 1000.0f;
const f32 k_VisionSphereRadiusCheck =   30.0f;

void AddConnectionsToGrid( nav_connection_slot_id thisNodeID,  u8 thisID, u16& iNext );


//=========================================================================

ai_editor::ai_editor(void) : m_FirstSelectedObject(SLOT_NULL) 
{ 
    s_This = this; 
    m_InConnectionMode = false;
    m_LastObjectSelected = 0;
    m_bTestPathValid = FALSE;
}

//=========================================================================

void  ai_editor::Render(void)
{
    if( eng_Begin("ai_editor::Render") )
    {
        if (g_AIMgr.GetShowNavSpine())
            g_NavMap.RenderNavigationSpine();

        RenderTestPath();

        eng_End();
    }
}

//=========================================================================

xbool  ai_editor::UpdateAI(void)
{
    return FALSE;

    /*
    xbool bUpdate = FALSE;

    //  To create the object within the editor we need to 
    //      CreateTemporaryObject
    //      Then PlaceTemporaryObject
    //      then ClearTemporaryObject
    //
    
    object* tempObject = g_ObjMgr.GetObjectByGuid(g_WorldEditor.GetGuidOfLastPlacedTemp());
    if(!tempObject || tempObject->GetType() != object::TYPE_NAV_NODE_EDITOR )
    {
        return FALSE;
    }

    return bUpdate;
    */
}

//=========================================================================

xbool ai_editor::DoesPlayerExist(void)
{

    slot_id anode_slot_id = g_ObjMgr.GetFirst(object::TYPE_PLAYER) ;
    if(anode_slot_id == SLOT_NULL )
    {
        return false;
    }
    else
    {
        return true;
    }
}

//=========================================================================

guid ai_editor::CreatePlayer(void)
{
    slot_id anode_slot_id = g_ObjMgr.GetFirst(object::TYPE_PLAYER) ;
    guid  aGuid;

    //
    //  Need to prevent the editor from trying to create a second player object.
    //  If they try to create a second player then we just give them the first one
    //  and the editor should select it.
    //  
    if( anode_slot_id ==  SLOT_NULL )
    {
        aGuid = g_ObjMgr.CreateObject( player::GetObjectType() );

    }
    else
    {
        aGuid = g_ObjMgr.GetObjectBySlot(anode_slot_id)->GetGuid();
    }


    return aGuid;
}

//=========================================================================

void ai_editor::LoadNavMap( const char* fileName )
{
    X_FILE* tempFile = x_fopen(xfs("%s.new",fileName), "rb" );
    if( tempFile == NULL )
    {
        return;
    }


    g_NavMap.Load( tempFile );
    x_fclose(tempFile);
    

}

//=========================================================================

void ai_editor::SaveNavMap( const char* fileName )
{    
    if(g_ObjMgr.GetNumInstances( object::TYPE_NAV_CONNECTION2_EDITOR ) == 0 )
    {
        return;
    }

    
    X_FILE* tempFile = x_fopen(xfs("%s.new",fileName), "wb" );

    CreateNavMap( );

    if(tempFile == NULL )
    {
        x_DebugMsg("\nNav Map could not be opened for writing!!!  Possibly read only??\n\n");
        return;        
    }

    g_NavMap.Save( tempFile );

    x_fclose(tempFile);
} 


//=========================================================================
//  CleanNavMap
//=========================================================================
void ai_editor::CleanNavMap()
{
    /*
    slot_id tempSlotId;
    tempSlotId = g_ObjMgr.GetFirst( object::TYPE_NAV_NODE_EDITOR );
    while( tempSlotId != SLOT_NULL )
    {
        object* tempNavNodeObject = g_ObjMgr.GetObjectBySlot( tempSlotId );        
        ASSERT( tempNavNodeObject );
        nav_node_editor& tempNavNode = nav_node_editor::GetSafeType(*tempNavNodeObject);
        
        //  Add the indexes used by 
        s32 connectionCount;
        for(connectionCount = 0; connectionCount <kMAX_CONNECTIONS_PER_NODE ; connectionCount++ )
        {   
            object* tempConnectionObject = g_ObjMgr.GetObjectByGuid( tempNavNode.GetConnection(connectionCount) );
            if( tempConnectionObject)
            {
                nav_connection_editor& tempConnection = nav_connection_editor::GetSafeType(* tempConnectionObject);
                if( !tempConnection.HasNode(tempNavNode.GetGuid()) )
                {
                    tempNavNode.RemoveConnection( tempNavNode.GetConnection(connectionCount) );
                    g_WorldEditor.MarkLayerDirty( g_WorldEditor.GetGlobalLayer() );
                }
            }
            else
            {
                tempNavNode.RemoveConnection( tempNavNode.GetConnection(connectionCount) );
                g_WorldEditor.MarkLayerDirty( g_WorldEditor.GetGlobalLayer() );
            }
        } 
        tempSlotId = g_ObjMgr.GetNext( tempSlotId );
    }


    // Check the nav connections.
    tempSlotId = g_ObjMgr.GetFirst( object::TYPE_NAV_CONNECTION_EDITOR );
    while(tempSlotId != SLOT_NULL )
    {
        object* tempObject = g_ObjMgr.GetObjectBySlot( tempSlotId );
        
        nav_connection_editor& tempConnection = nav_connection_editor::GetSafeType( *tempObject );

        //   now for the ugly part.  need to get the index of the nodes we are connecting too
        object* tempNodeObject = g_ObjMgr.GetObjectByGuid( tempConnection.GetStartNode() );
        if( !tempNodeObject )
        {
            tempSlotId = g_ObjMgr.GetNext( tempSlotId );
            g_ObjMgr.DestroyObjectEx(tempObject->GetGuid(), TRUE);
            continue;
        }

        nav_node_editor& StartNode = nav_node_editor::GetSafeType( *tempNodeObject );

        //  now to get the other end
        tempNodeObject = g_ObjMgr.GetObjectByGuid( tempConnection.GetEndNode() );
        if( !tempNodeObject )
        {
            tempSlotId = g_ObjMgr.GetNext( tempSlotId );
            g_ObjMgr.DestroyObjectEx(tempObject->GetGuid(), TRUE);
            continue;
        }

        nav_node_editor& EndNode = nav_node_editor::GetSafeType( *tempNodeObject );

        //
        //  Veryify the nodes have a reference to this connection
        //
        guid ConnectionGuid = tempObject->GetGuid();

        if (!StartNode.HasConnection( ConnectionGuid ))
        {
            x_DebugMsg( xfs("CleanNavMap: Connection %X:%X has been reconnected to start node %X:%X\n", 
                (u32)( ConnectionGuid >> 32),(u32)ConnectionGuid,
                (u32)( StartNode.GetGuid() >> 32),(u32)StartNode.GetGuid()));
            StartNode.AddConnection( ConnectionGuid );
        }

        if (!EndNode.HasConnection( ConnectionGuid ))
        {
            x_DebugMsg( xfs("CleanNavMap: Connection %X:%X has been reconnected to end node %X:%X\n", 
                (u32)( ConnectionGuid >> 32),(u32)ConnectionGuid,
                (u32)( EndNode.GetGuid() >> 32),(u32)EndNode.GetGuid()));
            EndNode.AddConnection( ConnectionGuid );
        }
        
        tempSlotId = g_ObjMgr.GetNext( tempSlotId );


        
    }
*/
    
    //TODO:SH - decide if we want to do this automatically, or hang it on a debug menu option
   // UpgradeNavObjects();
}


//=========================================================================
//
//  UpgradeNavObjects
//
//  Convert all nav_connection_editor objects to nav_connection2_editor
//  objects.  Destroy all old nav objects.
//
//=========================================================================
void ai_editor::UpgradeNavObjects( void )
{
    /*
    slot_id tempSlotId = g_ObjMgr.GetFirst( object::TYPE_NAV_CONNECTION_EDITOR );
    while(tempSlotId != SLOT_NULL )
    {
        object* pTempObject = g_ObjMgr.GetObjectBySlot( tempSlotId );
        
        nav_connection_editor& tempConnection = nav_connection_editor::GetSafeType( *pTempObject );

        guid newGuid = g_WorldEditor.CreateObject( "NavConnection2",
                                    nav_connection2_editor::GetObjectType().GetTypeName(), 
                                    tempConnection.GetPosition(),
                                    g_WorldEditor.GetLayerContainingObject( tempConnection.GetGuid()),
                                    "\\" );
        if (newGuid != 0)
        {       
            object* pNewObj = g_ObjMgr.GetObjectByGuid( newGuid );
            VERIFY( pNewObj );
            
            nav_connection2_editor& newConn = nav_connection2_editor::GetSafeType( *pNewObj );

            //
            //  Copy width and shared flags
            //
            newConn.SetWidth( tempConnection.GetWidth() );
            u32 Flags = tempConnection.GetFlags();            
            newConn.SetFlags( (newConn.GetFlags() & nav_connection2_editor::FLAG_MASK_PERSONAL) |
                              (Flags & nav_connection2_editor::FLAG_MASK_SHARED) );

            
            //
            //  Move one anchor to the start node position
            //
            {            
                nav_connection2_anchor* pA = newConn.GetAnchor( 0 );
                VERIFY( pA );
            
                guid NodeGuid = tempConnection.GetStartNode();
                object* pNodeObj = g_ObjMgr.GetObjectByGuid( NodeGuid );
                VERIFY( pNodeObj );
                nav_node_editor& Node = nav_node_editor::GetSafeType( *pNodeObj );

                pA->OnMove( Node.GetPosition() );
            }
            
            //
            //  Move the other anchor to the end node position
            //
            {            
                nav_connection2_anchor* pA = newConn.GetAnchor( 1 );
                VERIFY( pA );
            
                guid NodeGuid = tempConnection.GetEndNode();
                object* pNodeObj = g_ObjMgr.GetObjectByGuid( NodeGuid );
                VERIFY( pNodeObj );
                nav_node_editor& Node = nav_node_editor::GetSafeType( *pNodeObj );

                pA->OnMove( Node.GetPosition() );
            }
            
            //
            //  Destroy the original nav connection
            //
            g_ObjMgr.DestroyObject( pTempObject->GetGuid() );
        

            //...next...
            tempSlotId = g_ObjMgr.GetNext( tempSlotId );
        }
    }

    //
    //  Destroy all old navnodes
    //
    tempSlotId = g_ObjMgr.GetFirst( object::TYPE_NAV_NODE_EDITOR );
    while(tempSlotId != SLOT_NULL )
    {
        object* pTempObject = g_ObjMgr.GetObjectBySlot( tempSlotId );
        VERIFY( pTempObject );
        g_ObjMgr.DestroyObject( pTempObject->GetGuid() );
        tempSlotId = g_ObjMgr.GetNext( tempSlotId );
    }
*/
}



//=========================================================================
//
//  CreateNavMap
//
//      CreateNavMap creates a nav map from scratch from the current objects
//      in the editor.  Should get called any time the game logic starts or
//      the level is exported.  Once this has been called, it is safe to 
//      save out the nav_map file.
//
//=========================================================================
static const s32  NAVBUILD_MAX_OVERLAP_VERTS = 16;

struct navbuild_connection
{
    nav_connection2_editor*     pConnection;

    struct overlap_info
    {
        overlap_info()
        {
            iRemoteConnectionID = NULL_NAV_SLOT;
            iOverlapDataIndex   = 0;
        }
        s32 iRemoteConnectionID;
        s32 iOverlapDataIndex;
    };

    xarray<overlap_info>        OverlapInfo;     // Which connections do we overlap with
};

struct navbuild_overlapdata
{
    navbuild_overlapdata()
    {
        nVerts = 0;
        iFirstVertInNavMapData = 0;
        Centroid.Set(0,0,0);
        iConnection[0] = NULL_NAV_SLOT;
        iConnection[1] = NULL_NAV_SLOT;
        Flags          = 0;
    }
    vector3     Verts[ NAVBUILD_MAX_OVERLAP_VERTS ];
    s32         nVerts;
    s32         iFirstVertInNavMapData;
    vector3     Centroid;
    s32         iConnection[2];
    u16         Flags;
};  

void ai_editor::CreateNavMap( void )
{
    //==-------------------------------------------------------------
    //  Initial tests and setup
    //==-------------------------------------------------------------
    g_NavMap.Reset();
    CleanNavMap();

    s32 nConnections = g_ObjMgr.GetNumInstances( object::TYPE_NAV_CONNECTION2_EDITOR );

    if( nConnections <= 0 )
    {
        // No work to do!
        return;
    }

    //==-------------------------------------------------------------
    //  Fill an array with connection2 pointers
    //==-------------------------------------------------------------
    
    navbuild_connection* pConnections = new navbuild_connection[ nConnections ];
    ASSERT( pConnections );
    if (NULL == pConnections)
        return;

    xarray<navbuild_overlapdata>    OverlapData;        // Stores convex overlap hulls
    OverlapData.SetGrowAmount( 16 );
    
    slot_id tempSlotId = g_ObjMgr.GetFirst(object::TYPE_NAV_CONNECTION2_EDITOR );
    s32 iCount = 0;
    while(tempSlotId != SLOT_NULL )
    {
        ASSERT( iCount < nConnections ); 

        object* pTempObject = g_ObjMgr.GetObjectBySlot(tempSlotId);
        ASSERT( pTempObject->IsKindOf( nav_connection2_editor::GetRTTI() ));
        
        pConnections[ iCount ].pConnection = (nav_connection2_editor*)pTempObject;
        pConnections[ iCount ].OverlapInfo.SetGrowAmount( 4 );

        nav_connection2_editor& TempConnection = nav_connection2_editor::GetSafeType( *pTempObject );
        TempConnection.SetIndexInSavedList( iCount );
        TempConnection.SetAnchorsDirty();
        
        tempSlotId = g_ObjMgr.GetNext( tempSlotId );
        
        iCount++;
    }

    //==-------------------------------------------------------------
    //  Test connection against connection, looking for overlap
    //==-------------------------------------------------------------
    s32 iCurConnection;
    s32 iRemoteConnection;
    s32 nRequiredOverlapVerts = 0;

    vector3 OverlapVertBucket[ NAVBUILD_MAX_OVERLAP_VERTS ];
    s32     nOverlapVerts = 0;

    for ( iCurConnection = 0; iCurConnection < (nConnections-1); iCurConnection++ )
    {
        // iCurConnection is the base connection

        for ( iRemoteConnection = iCurConnection+1; iRemoteConnection < nConnections; iRemoteConnection++ )
        {
            // Test the base connection against the remote connection
            xbool bResult = TestConnectionsForOverlap( pConnections[iCurConnection].pConnection,
                                                       pConnections[iRemoteConnection].pConnection,
                                                       OverlapVertBucket,
                                                       16,
                                                       nOverlapVerts );

            if (bResult)
            {                
                nRequiredOverlapVerts += nOverlapVerts;                

                //
                //  Copy the overlap verts into a temp object and
                //  append it to the master xarray of overlapping regions
                //
                navbuild_overlapdata    Overlap;
                Overlap.nVerts = nOverlapVerts;
                s32 i;
                for (i=0;i<nOverlapVerts;i++)
                {
                    Overlap.Verts[ i ] = OverlapVertBucket[ i ];
                }
                for (i=nOverlapVerts;i<NAVBUILD_MAX_OVERLAP_VERTS;i++)
                {
                    Overlap.Verts[ i ].Set(0,0,0);
                }

                // Connection IDs are needed for testing the 
                // overlap verts later
                Overlap.iConnection[0] = iCurConnection;
                Overlap.iConnection[1] = iRemoteConnection;

                s32 iCurOverlap = OverlapData.GetCount();
                OverlapData.Append(Overlap);

                //
                //  Build the overlapping info required for each connection
                //
                navbuild_connection::overlap_info   Info;

                Info.iRemoteConnectionID = iRemoteConnection;
                Info.iOverlapDataIndex   = iCurOverlap;

                pConnections[ iCurConnection ].OverlapInfo.Append( Info );

                Info.iRemoteConnectionID = iCurConnection;

                pConnections[ iRemoteConnection ].OverlapInfo.Append( Info );
            }
        }
    }

    //==-------------------------------------------------------------
    //  We now have all the overlap information we need.
    //  We know what connections are overlapped with any other 
    //  connection, and we also have access to the overlap region
    //  verts
    //==-------------------------------------------------------------
    //
    //  These will be used to init the navmap
    //
    s32     nRequiredOverlaps       = OverlapData.GetCount();       // # of overlap structures
    s32     nRequiredOverlapInfos   = nRequiredOverlaps*2;          // Each overlap is referenced by only
                                                                    //      two connections.

    g_NavMap.Init( nConnections, nRequiredOverlaps, nRequiredOverlapInfos, nRequiredOverlapVerts );
    

    //==-------------------------------------------------------------
    //  Once the nav_map has allocated storage, we need to access
    //  it and fill it out with our temporary data
    //==-------------------------------------------------------------
    
    nav_map::overlap_data*                  pNavOverlapData      = g_NavMap.GetOverlapDataPtr();
    nav_map::connection2_data*              pNavConnectionData   = g_NavMap.GetConnectionDataPtr();
    nav_map::connection2_connectivity_data* pNavConnectivityData = g_NavMap.GetConnectivityDataPtr();
    nav_map::overlap_vert*                  pNavOverlapVertData  = g_NavMap.GetOverlapVertDataPtr();

    //==---------------------
    //  Fill up vertex data
    //==---------------------
    {
        s32 i;
        s32 iCur = 0;
        for (i=0;i<nRequiredOverlaps;i++)
        {
            navbuild_overlapdata&   D = OverlapData[i];

            D.iFirstVertInNavMapData = iCur;

            vector3 Sum(0,0,0);

            s32 j;
            s32 iFirstVert = iCur;

            for (j=0;j<D.nVerts;j++,iCur++)
            {
                ASSERT( iCur < nRequiredOverlapVerts );     // SAFETY

                pNavOverlapVertData[ iCur ].m_Pos = D.Verts[ j ];
                Sum += D.Verts[ j ];
            }

            Sum.Scale( 1.0f / D.nVerts );
            D.Centroid = Sum;

            // We need to test a point to see if each vert is "outside"
            // (meaning that it lies on the edge of connections, with invalid
            //  navigation space beyond it, and not inside of a connection)
            
            for (j=0;j<D.nVerts;j++)
            {
                vector3 Normal = D.Verts[j];
                vector3 TestPt = D.Verts[j];

                Normal -= D.Centroid;
                Normal.NormalizeAndScale( 2 );      // Test 2cm beyond

                TestPt += Normal;
                
                s32 c;
                f32 Dist = 2.0f;
                xbool bOutside = TRUE;
                for (c=0;c<nConnections;c++)
                {
                    // Inflate all connections, except for the 2 that
                    // caused this overlap
                    if (c==D.iConnection[0] || c==D.iConnection[1])
                        Dist = 0.0f;
                    else
                        Dist = 2.0f;
                    
                    xbool bInside = pConnections[ c ].pConnection->IsPointInConnection( TestPt, Dist );

                    if (bInside)
                    {
                        bOutside = FALSE;
                        break;
                    }
                }
                if (bOutside)
                {
                    pNavOverlapVertData[ iFirstVert + j ].m_Flags |= nav_map::overlap_vert::FLAG_OUTSIDE;
                }
            }
        }
    }

    //----------------------------
    //  Copy over the overlap data
    //----------------------------
    {
        s32 i;
        for (i=0;i<nRequiredOverlaps;i++)
        {
            pNavOverlapData[i].m_Center         = OverlapData[i].Centroid;
            pNavOverlapData[i].m_Flags          = OverlapData[i].Flags;
            pNavOverlapData[i].m_iConnection[0] = -1;
            pNavOverlapData[i].m_iConnection[1] = -1;
            pNavOverlapData[i].m_iFirstOverlapPt= OverlapData[i].iFirstVertInNavMapData;
            pNavOverlapData[i].m_nOverlapPts    = OverlapData[i].nVerts;
        }
    }



    //==---------------------
    //  Fill up connection 
    //  data and the 
    //  overlap data as we go
    //==---------------------
    {
        s32 i;
        //s32 iCurOverlap = 0;
        s32 iCurConnectivity = 0;

        for (i=0;i<nConnections;i++)
        {
            navbuild_connection&        SrcConn = pConnections[ i ];
            nav_map::connection2_data&  DstConn = pNavConnectionData[ i ];

            // Feed the connection guid to the nav map
            //g_NavMap.Set

            DstConn.m_Width = SrcConn.pConnection->GetWidth();
            DstConn.m_Flags = SrcConn.pConnection->GetFlags();           
            DstConn.m_nOverlaps = SrcConn.OverlapInfo.GetCount();
            DstConn.m_iFirstConnectivity = iCurConnectivity;
            DstConn.m_StartPt       = SrcConn.pConnection->GetAnchorPosition(0);
            DstConn.m_EndPt         = SrcConn.pConnection->GetAnchorPosition(1);
            DstConn.m_iGrid         = 255;           

            g_NavMap.AddConnectionGuid( SrcConn.pConnection->GetGuid(), i );

            // Itterate through the overlaps coming off of this connection.
            // -Fill out the nav maps connectivity structures
            // -Update the overlap data structures with the correct connection indices
            s32 j;
            for (j=0;j<SrcConn.OverlapInfo.GetCount();j++)
            {
                navbuild_connection::overlap_info& OverlapInfo = SrcConn.OverlapInfo[j];
                
                pNavConnectivityData[ iCurConnectivity ].m_iRemoteConnection = OverlapInfo.iRemoteConnectionID;
                pNavConnectivityData[ iCurConnectivity ].m_iOverlapData      = OverlapInfo.iOverlapDataIndex;

                pNavOverlapData[ OverlapInfo.iOverlapDataIndex ].m_iConnection[0] = i;
                pNavOverlapData[ OverlapInfo.iOverlapDataIndex ].m_iConnection[1] = OverlapInfo.iRemoteConnectionID;                                

                iCurConnectivity++;
            }            
        }
    }

    //==-------------------------------------------------------------
    //  Lock it down
    //==-------------------------------------------------------------
    g_NavMap.SetData();

    //==-------------------------------------------------------------
    //  Build the grid
    //==-------------------------------------------------------------
    SetGridIDs();

    //==-------------------------------------------------------------
    //  Build lookup boxes
    //==-------------------------------------------------------------
    g_NavMap.CompileLookupBoxes();

    //==-------------------------------------------------------------
    //  We're done, play nice and clean up...
    //==-------------------------------------------------------------
    delete[] pConnections;
}


//=========================================================================
//
//  TestConnectionsForOverlap
//
//  Test two connections pA, and pB, and see if they overlap.
//  If they do, compute the 2D convex hull of the overlapped region
//  (on the XZ plane).
//
//=========================================================================
xbool ai_editor::TestConnectionsForOverlap( nav_connection2_editor*  pA,                 // IN:  Connection 1
                                            nav_connection2_editor*  pB,                 // IN:  Connection 2
                                            vector3*                 pVertBucket,        // IN:  Storage for verts
                                            s32                      nMaxVerts,          // IN:  Storage bucket size
                                            s32&                     nOutputVerts    )   // OUT: # of verts written to bucket
{
    nOutputVerts = 0;

    if (NULL == pA)
        return FALSE;
    if (NULL == pB)
        return FALSE;
    
    const vector3* pAVerts = pA->GetRenderCorners(FALSE);        // See comment in nav_connection2_editor
    const vector3* pBVerts = pB->GetRenderCorners(FALSE);        // header about vertex winding


    //==-------------------------------------------------------------
    // First, try A against B
    //==-------------------------------------------------------------
    plane       Planes[6];         // Plane order is:  0=+Y
                                   //                  1=-Y
                                   //                  2=+X
                                   //                  3=-X
                                   //                  4=+Z
                                   //                  5=-Z
    
    Planes[0].Setup( pBVerts[1], pBVerts[0], pBVerts[4] );
    Planes[1].Setup( pBVerts[6], pBVerts[2], pBVerts[3] );
    Planes[2].Setup( pBVerts[4], pBVerts[0], pBVerts[2] );
    Planes[3].Setup( pBVerts[1], pBVerts[5], pBVerts[7] );
    Planes[4].Setup( pBVerts[5], pBVerts[4], pBVerts[6] );
    Planes[5].Setup( pBVerts[0], pBVerts[1], pBVerts[3] );
   
       
    s32 i;
    s32 iPlane;

    for (iPlane=0;iPlane<6;iPlane++)
    {
   
        s32 nFront = 0;
        for (i=0;i<8;i++)
        {
            if (Planes[iPlane].InFront( pAVerts[i] ))
                nFront++;
        }

        if (nFront == 8)        // If all 8 are in front of a plane, we don't overlap
            return FALSE;
    }


    //==-------------------------------------------------------------
    // Next, try B against A
    //==-------------------------------------------------------------    
    Planes[0].Setup( pAVerts[1], pAVerts[0], pAVerts[4] );
    Planes[1].Setup( pAVerts[6], pAVerts[2], pAVerts[3] );
    Planes[2].Setup( pAVerts[4], pAVerts[0], pAVerts[2] );
    Planes[3].Setup( pAVerts[1], pAVerts[5], pAVerts[7] );
    Planes[4].Setup( pAVerts[5], pAVerts[4], pAVerts[6] );
    Planes[5].Setup( pAVerts[0], pAVerts[1], pAVerts[3] );

    for (iPlane=0;iPlane<6;iPlane++)
    {
   
        s32 nFront = 0;
        for (i=0;i<8;i++)
        {
            if (Planes[iPlane].InFront( pBVerts[i] ))
                nFront++;
        }

        if (nFront == 8)        // If all 8 are in front of a plane, we don't overlap
            return FALSE;
    }
  
    //==------------------------------------------------------------- 
    //  If we get here, we have failed the trivial rejection tests
    //  Lets do something better...
    //
    //  -Planes[] still holds connection A's planes.
    //  -Take the verts of connection B, build quads from them
    //   and run them through what is essentially a sutherland-hodgeman
    //   clipper.  
    //  -If anything comes out the other side, for any of
    //   the quads, then the connections must overlap.
    //==------------------------------------------------------------- 
    
    vector3 Pool[2][16];
    s32     nVerts[2];
    s32     iQuadIndices[] = {  0,1,3,2, 
                                5,4,6,7,  
                                4,0,2,6,
                                1,5,7,3,
                                1,5,4,0,
                                6,2,3,7 };    

    s32     iSrc = 0;
    s32     iQuad;
    xbool   bOverlap = FALSE;

    for (i=0;i<6;i++)
        Planes[i].Negate();

    for (iQuad=0;iQuad<6;iQuad++)
    {
        iSrc = 0;

        Pool[iSrc][0] = pBVerts[iQuadIndices[iQuad*4+0]];
        Pool[iSrc][1] = pBVerts[iQuadIndices[iQuad*4+1]];
        Pool[iSrc][2] = pBVerts[iQuadIndices[iQuad*4+3]];
        Pool[iSrc][3] = pBVerts[iQuadIndices[iQuad*4+2]];

        nVerts[0] = 4;
        
        s32 i;
    
        for (i=0;i<6;i++)
        {
            nVerts[1-iSrc]=16;
            xbool Res = Planes[i].ClipNGon( Pool[1-iSrc], nVerts[1-iSrc], Pool[iSrc], nVerts[iSrc] );
            iSrc = 1-iSrc;
            if (nVerts[iSrc] == 0)
            {                
                break;
            }           
        }
        // Coming out, iSrc points to the clipped verts
        if (nVerts[iSrc] > 0)
        {
            bOverlap = TRUE;
            break;
        }
    }
    
    //==-------------------------------------------------------------    
    //  If we get here, we must be overlapping!
    //==-------------------------------------------------------------    
    nOutputVerts = ClipConnections( pA, pB, pVertBucket, nMaxVerts );


    if (nOutputVerts == 0)
        return FALSE;
    else
        bOverlap = TRUE;
    
    if (!bOverlap)
        return FALSE;

    return TRUE;
}

//=========================================================================

s32 ai_editor::ClipConnections( nav_connection2_editor* pA,  
                                nav_connection2_editor* pB,  
                                vector3* pVertBucket,  
                                s32 nMaxVerts )
{
    const vector3* pAVerts = pA->GetRenderCorners(FALSE);        // See comment in nav_connection2_editor
    const vector3* pBVerts = pB->GetRenderCorners(FALSE);        // header about vertex winding

    
    plane Planes[4];

    Planes[0].Setup( pBVerts[4], pBVerts[0], pBVerts[2] );
    Planes[1].Setup( pBVerts[1], pBVerts[5], pBVerts[7] );
    Planes[2].Setup( pBVerts[5], pBVerts[4], pBVerts[6] );
    Planes[3].Setup( pBVerts[0], pBVerts[1], pBVerts[3] );

    vector3 Pool[2][16];
    s32     nVerts[2];

    Pool[0][0] = pAVerts[1];
    Pool[0][1] = pAVerts[5];
    Pool[0][2] = pAVerts[4];
    Pool[0][3] = pAVerts[0];

    nVerts[0] = 4;
    
    // Clip the pool against the 4 planes
    
    s32 iSrc = 0;
    s32 i;
    for (i=0;i<4;i++)
        Planes[i].Negate();

    for (i=0;i<4;i++)
    {
        Planes[i].ClipNGon( Pool[1-iSrc], nVerts[1-iSrc], Pool[iSrc], nVerts[iSrc] );
        iSrc = 1-iSrc;
    }
    // Pool[iSrc] now has the clipped verts
    
    
    // Now, just figure out where the two planes intersect to determine the 
    // Y-coordinate of the overlap verts
    f32         AvgYA = 0;
    f32         AvgYB = 0;

    vector3     AnchorA0 = pA->GetAnchorPosition(0);
    vector3     AnchorA1 = pA->GetAnchorPosition(1);
    vector3     AnchorB0 = pB->GetAnchorPosition(0);
    vector3     AnchorB1 = pB->GetAnchorPosition(1);
    vector3     Displace(0,50,0);

    AnchorA0 += Displace;
    AnchorA1 += Displace;
    AnchorB0 += Displace;
    AnchorB1 += Displace;

    f32         MinY = MIN(MIN(AnchorB0.GetY(),AnchorB1.GetY()),MIN(AnchorA0.GetY(),AnchorA1.GetY()));
    f32         MaxY = MAX(MAX(AnchorB0.GetY(),AnchorB1.GetY()),MAX(AnchorA0.GetY(),AnchorA1.GetY()));

    Planes[0].Setup(AnchorA0,pA->GetPlane().Normal);
    Planes[1].Setup(AnchorB0,pB->GetPlane().Normal);
    
    for (i=0;i<nVerts[iSrc];i++)
    {
        f32 T=0;

        vector3 Start = Pool[iSrc][i];
        Start.GetY() = MaxY;
        vector3 End = Pool[iSrc][i];
        End.GetY() = MinY;

        // Intersect against plane A
        Planes[0].Intersect( T, Start, End );
        AvgYA += (MinY-MaxY)*T+MaxY;

        // Intersect against plane B
        Planes[1].Intersect( T, Start, End );
        AvgYB += (MinY-MaxY)*T+MaxY;
    }

    AvgYA /= nVerts[iSrc];
    AvgYB /= nVerts[iSrc];

    f32 AvgY = (AvgYA+AvgYB)/2;

    /*
    
    vector3     P1,P2;

    P1 = pA->GetAnchorPosition(0);
    P2 = pA->GetAnchorPosition(1);

    P1 += vector3(0,50,0);
    P2 += vector3(0,50,0);

    vector3     ConnectionARay = P2-P1;
    plane       BPlane = pB->GetPlane();

    f32 T = 0;
    BPlane.Intersect( T, P1, P2 );

    ConnectionARay.Scale( T );
    ConnectionARay += P1;

*/
    vector3 Centroid(0,0,0);
    for (i=0;i<nVerts[iSrc];i++)
    {
        pVertBucket[i] = Pool[iSrc][i];
        pVertBucket[i].GetY() = AvgY;//ConnectionARay.Y;

        Centroid += pVertBucket[i];
    }

    Centroid.Scale( 1.0f / nVerts[iSrc] );
    f32 ScaleAmt = g_AIMgr.GetOverlapConstructionScale();

    for (i=0;i<nVerts[iSrc];i++)
    {
        vector3 Ray = pVertBucket[i] - Centroid;
        Ray.Scale(ScaleAmt);
        pVertBucket[i] = Centroid+Ray;
    }



    return nVerts[iSrc];

 
/*
    // Get the clip edge values
    f32 fWidth = pA->GetWidth();
    f32 fLength = pA->GetLength();

    f32 ClipEdgeValues[ CLIP_MAX ];
    
    ClipEdgeValues[CLIP_LEFT]   = -fWidth;
    ClipEdgeValues[CLIP_TOP]    = fLength;
    ClipEdgeValues[CLIP_RIGHT]  = fWidth;
    ClipEdgeValues[CLIP_BOTTOM] = 0.f;

    // Lookup dest values.
    fWidth = pB->GetWidth();    
    fLength = pB->GetLength();    

    f32 DestEdgeValues[ CLIP_MAX ];
    DestEdgeValues[CLIP_LEFT]   = -fWidth;
    DestEdgeValues[CLIP_TOP]    = fLength;
    DestEdgeValues[CLIP_RIGHT]  = fWidth;
    DestEdgeValues[CLIP_BOTTOM] = 0.f;

    // Get the box to clip.
    vector3 vCorners[4];
    vCorners[0].Set( DestEdgeValues[CLIP_LEFT], 0.f, 0.f );
    vCorners[1].Set( DestEdgeValues[CLIP_RIGHT], 0.f, 0.f );
    vCorners[3].Set( DestEdgeValues[CLIP_LEFT], 0.f, DestEdgeValues[CLIP_TOP] );
    vCorners[2].Set( DestEdgeValues[CLIP_RIGHT], 0.f, DestEdgeValues[CLIP_TOP] );

    // Got to rotate into local space.
    radian AngleBetween = x_MinAngleDiff( pA->GetYaw(), pB->GetYaw() );
    radian AngleFromOriginal = x_MinAngleDiff( R_0, pA->GetYaw() );

    s32 i;
    for ( i = 0; i < 4; i++ )
    {
        vCorners[i].RotateY( -AngleBetween );
        vCorners[i] += vector3( 0.f, 0.f, ClipEdgeValues[CLIP_TOP] );
    }

    s32 nDestVerts = nMaxVerts;

    g_NavMap.ClipConnections( pVertBucket, vCorners, nDestVerts, 4, ClipEdgeValues );

    nav_connection2_anchor* pAnchor = pA->GetAnchor(0);
    ASSERT( pA );

    vector3 vBasePos = pAnchor->GetPosition();

    for ( i = 0; i < nDestVerts; i++ )
    {
        pVertBucket[i].RotateY( -AngleFromOriginal );
        pVertBucket[i] += vBasePos;
    }

    // Now, just figure out where the two planes intersect to determine the 
    // Y-coordinate of the overlap verts
    nav_connection2_anchor* pAnchor2 = pA->GetAnchor(1);
    ASSERT(pAnchor2);

    vector3     ConnectionARay = pAnchor2->GetPosition() - pAnchor->GetPosition();
    plane       BPlane = pB->GetPlane();

    f32 T = 0;
    BPlane.Intersect( T, pAnchor->GetPosition(), pAnchor2->GetPosition() );

    ConnectionARay.Scale( T );
    ConnectionARay += pAnchor->GetPosition();

    for ( i = 0; i < nDestVerts; i++ )
    {
        pVertBucket[i].Y = ConnectionARay.Y;        
    }

    //
    // Done
    //

    return nDestVerts;
    */
}

//=========================================================================

void ai_editor::SetNavTestStart( guid thisNode )
{
//    g_NavMgr.SetTestStart(thisNode);
}

//=========================================================================

void ai_editor::SetNavTestEnd  ( guid thisNode )
{
//    g_NavMgr.SetTestEnd(thisNode);
}

//=========================================================================

void ai_editor::CalcPath(void)
{
//    g_NavMgr.TestPath();
}

//=========================================================================
void ai_editor::CheckAllNavConnections( void )
{
/*
    object *currentObject;

    slot_id currentSlot;


    //   to check all nav connections, we walk through every nav_node_editor in
    //   the obj_mgr and check each of it's collisions to see if the ends can see each other.

    currentSlot = g_ObjMgr.GetFirst( object::TYPE_NAV_NODE_EDITOR );

    while( currentSlot != SLOT_NULL )
    {
        currentObject = g_ObjMgr.GetObjectBySlot( currentSlot );
        if(currentObject != NULL )
        {

            //  Verify we have the right type
            if(  !currentObject->IsKindOf( nav_node_editor::GetRTTI()))
            {
                x_throw("Logic error in ai_editor, tell Chris ");

            }
            else
            {   

                nav_node_editor& tempConnection = nav_node_editor::GetSafeType( *currentObject );



              
                //  Now that we have the right type for the node, walk the connections and make
                //  check LOS
                s32 count;
                for( count =0; count < tempConnection.GetConnectionCount(); count++ )
                {
                    object* tempConnectionObject = g_ObjMgr.GetObjectByGuid( tempConnection.GetConnection(count) );
                    if( tempConnectionObject && tempConnectionObject->IsKindOf( nav_connection_editor::GetRTTI()) )
                    {

                        //  Make sure we get the other end from the starting node
                        object* endNode;
                        nav_connection_editor& tempNavConnection = nav_connection_editor::GetSafeType( *tempConnectionObject );


                        // First check is going to be to make sure that both ends of the connection don't
                        // connect to the same node
                        if( tempNavConnection.GetStartNode() == tempNavConnection.GetEndNode() )
                        {
                            x_DebugMsg( xfs("Connection %d:%d is connected to it self!\n", (u32)( tempNavConnection.GetGuid() >> 32),(u32)tempNavConnection.GetGuid()  ) );
                            x_DebugMsg( "Please re-run Check all connections\n" );
                            g_WorldEditor.SelectObject( tempNavConnection.GetGuid(),FALSE );
                        }


                        if ( tempNavConnection.GetStartNode() == currentObject->GetGuid()  )
                        {
                            endNode = g_ObjMgr.GetObjectByGuid( tempNavConnection.GetEndNode() );
                        }
                        else if ( tempNavConnection.GetEndNode() == currentObject->GetGuid()  )
                        {
                            endNode = g_ObjMgr.GetObjectByGuid( tempNavConnection.GetStartNode() );
                        }
                        else
                        {
                            endNode = NULL;

                        }
                        if( endNode )
                        {
                            

                            //  
                            //  Ok, time to do some scary collision checking logic!
                            //  We don't have a system to check collision for an OABbox so instead
                            //  we take the 8 corners to the bbox and shoot rays to the 
                            //  other 7 corners.
                            //
                            vector3 *corners = tempNavConnection.GetRenderCorners();
                            tempNavConnection.SetClearLineOfSight( true );

                            s32 outerLoop, innerLoop;
                            for( outerLoop = 0; outerLoop < 7; outerLoop++)
                            {
                                //  small optimization, prevent it from checking from A to B and then B to A
                                for(innerLoop = outerLoop ; innerLoop < 8; innerLoop++ )
                                {

                                    //  If they are equal, don't check for collsion with itself
                                    if( outerLoop != innerLoop )
                                    {
                                    
                                        g_CollisionMgr.RaySetup(    tempConnection.GetGuid(),
                                                                    corners[outerLoop],
                                                                    corners[innerLoop] );


                                        g_CollisionMgr.SetMaxCollisions(4);

                                        g_CollisionMgr.CheckCollisions( object::TYPE_PLAY_SURFACE,  
                                                                        object::ATTR_COLLIDABLE  );



                                        // If we got a collision, set the the flag so it renders differently
                                        if( g_CollisionMgr.m_nCollisions > 0 )
                                        {
                                            // if we hit something, set the 
                                            tempNavConnection.SetClearLineOfSight( false );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } 
        }// if(currentObject != NULL )

        //  Get the next Nav Node and try it again
        currentSlot = g_ObjMgr.GetNext( currentSlot );

    
    }// while( currentSlot != SLOT_NULL )
   

*/


}




// ============================================================================
//  
//  
//  
// ============================================================================
void ai_editor::SetFlagsInSelectedObjects(u32 Flags, eSetFlagMode setFlagMode )
{
    xarray<guid> guidList;
    g_WorldEditor.GetSelectedList( guidList );

    s32 count;
    for( count = 0 ; count < guidList.GetCount() ; count++)
    {
        object* tempObject = g_ObjMgr.GetObjectByGuid( guidList[count] );
        if(tempObject)
        {
            g_WorldEditor.SetObjectsLayerAsDirty(tempObject->GetGuid() );
            if(tempObject->IsKindOf(nav_connection2_editor::GetRTTI() ) )
            {
                nav_connection2_editor& tempConnection = nav_connection2_editor::GetSafeType(*tempObject);
                switch (setFlagMode)
                {
                case k_NoChange:
                    {

                    }
                    break;
                case k_Toggle:
                    {
                        tempConnection.SetFlags( tempConnection.GetFlags() ^ Flags );
                    }
                    break;
                case k_AllTrue:
                    {
                        tempConnection.SetFlags( tempConnection.GetFlags() | Flags );

                    }
                    break;
                case k_AllFalse:
                    {
                        tempConnection.SetFlags( tempConnection.GetFlags() & ~Flags );

                    }
                    break;
                }
            }
        }
    }
}



// ============================================================================
//  
//  Finds all the connected nodes and marks them as a grid
//  
// ============================================================================
void ai_editor::SetGridIDs(void)
{
    nav_map::connection2_data* pConnData = g_NavMap.GetConnectionDataPtr();
    ASSERT( pConnData );

    s32 nConnections = g_NavMap.GetConnectionCount();

    s32 i;
    for (i=0;i<nConnections;i++)
    {
        pConnData[i].m_iGrid = 255;
    }

    u8  iCurGrid = 0;
    u16 iNext = 0;
    for (i=0;i<nConnections;i++)
    {
        if (pConnData[i].m_iGrid == 255)
        {
            iNext = NULL_NAV_SLOT;
            AddConnectionsToGrid( i, iCurGrid, iNext );
            iCurGrid++;
            VERIFYS( iCurGrid < 225, "Map has more than the 225 allowed navigation grids" );
        }
    }
}


// ============================================================================
//  
//  Recursively called for all connections from a node to flag with an ID
//  
// ============================================================================//
void AddConnectionsToGrid( nav_connection_slot_id iConn,  u8 iGrid, u16& iNext )
{
    nav_map::connection2_data& Conn = g_NavMap.GetConnectionData( iConn );

    ASSERT( Conn.m_iGrid == 255 );      // SAFETY

    Conn.m_iGrid = iGrid;
    Conn.m_iNextInGrid = iNext;

    iNext = iConn;

    // Each overlap/node has 2 connections
    // check each one, and if it exists, target 
    // each of it's overlaps/nodes also
    s32 i;
    for (i=0;i<Conn.m_nOverlaps;i++)
    {
        nav_map::connection2_connectivity_data& CD = g_NavMap.GetConnectivityData( Conn.m_iFirstConnectivity + i);
        nav_map::connection2_data& Conn = g_NavMap.GetConnectionData( CD.m_iRemoteConnection );

        if (Conn.m_iGrid == 255)
        {
            AddConnectionsToGrid( CD.m_iRemoteConnection, iGrid, iNext );
        }
    }    
}

// ============================================================================

void ai_editor::ClearTestPath( void )
{
    m_bTestPathValid = FALSE;
}

// ============================================================================

void ai_editor::ComputeTestPath( const vector3& MouseRayStart, const vector3& MouseRayEnd )
{
    // Get selected objects
    xarray<guid> SelectedList;
    g_WorldEditor.GetSelectedList( SelectedList );
    
    // Must have at least 1 or 2 objects selected
    if( ( SelectedList.GetCount() < 1 ) || ( SelectedList.GetCount() > 2 ) )
        return;
        
    // Compute object to move
    object* pMovingObject = NULL;
    vector3 Destination(0,0,0);
            
    // Use mouse as position?            
    if( SelectedList.GetCount() == 1 )        
    {
        // Lookup object
        pMovingObject = g_ObjMgr.GetObjectByGuid( SelectedList[0] );
        if( !pMovingObject )
            return;
            
        // Cast ray into screen to find destination position
        g_CollisionMgr.EditorSelectRay( MouseRayStart, MouseRayEnd, FALSE );
        if( g_CollisionMgr.m_nCollisions == 0 )
            return;
        
        // Use first collision as path end
        Destination = g_CollisionMgr.m_Collisions[0].Point;                    
    }
    else
    {
        // Use objects
        ASSERT( SelectedList.GetCount() == 2 );
        object* pObject1 = g_ObjMgr.GetObjectByGuid( SelectedList[0] );
        object* pObject2 = g_ObjMgr.GetObjectByGuid( SelectedList[1] );
        if( ( !pObject1 ) || ( !pObject2 ) )
            return;
    
        // Use actor based object as moving object
        if( pObject1->IsKindOf( actor::GetRTTI() ) )
        {
            pMovingObject = pObject1;
            Destination   = pObject2->GetPosition();
        }
        else
        {
            pMovingObject = pObject2;
            Destination   = pObject1->GetPosition();
        }
    }

    // Lookup moving object info
    ASSERT( pMovingObject );
    m_TestPathRadius = 0.5f * 100.0f;  // Default to 0.5 meters
    
    // If this is an actor, use the collision radius
    if( pMovingObject->IsKindOf( actor::GetRTTI() ) )
    {
        loco* pLoco = ((actor*)pMovingObject)->GetLocoPointer();
        if( pLoco )
            m_TestPathRadius = pLoco->m_Physics.GetColRadius();
    }
    
    // Create the nav map
    m_bTestPathValid = FALSE;
    CleanNavMap();
    CreateNavMap();

    // Finally compute the test path
    god* pGod = SMP_UTIL_Get_God();
    ASSERT( pGod );
    pGod->m_AStarPathFinder.ResetNumNodes() ;
    m_bTestPathValid = pGod->RequestPathWithEdges( pMovingObject, 
                                                  Destination, 
                                                  m_PathFindStruct, 
                                                  MAX_EDGES_IN_LIST, 
                                                  &m_PathingHints );
}

// ============================================================================

void ai_editor::RenderTestPath( void )
{
    // Nothing to render?
    if( !m_bTestPathValid )
        return;

    // Render the path
    m_PathFindStruct.RenderPath( m_TestPathRadius );
}

// ============================================================================
