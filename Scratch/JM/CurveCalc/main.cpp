//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Render.hpp"

#include "navigation\\Nav_Map.hpp"
#include "navigation\\ng_node.hpp" 
#include "Characters/AStar.hpp"
#include "CurveCalcStructs.hpp"

static const s_Max_Edges = 20;

//==============================================================================
// MAIN
//==============================================================================

//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;

// Stuff for my pathfinding.
astar_path_finder Pathfinder;
xbool            g_bPathComputed = FALSE;
s32              g_PathList[20];
s32              g_NodeToTest = 8;
path_find_struct g_PathFindStruct;


//===========================================================================

#define VEC3F_NULL vector3( 0.f, 0.f, 0.f )

#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list

#ifdef TARGET_PC
    const char*     DataPath = "";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif
                    
static vector3      s_DesPoint(0,0,-400);
static radian       s_DesYaw = R_180 ;
static vector3      s_AimPoint(0,0,0);
static xbool        s_bRandDest = TRUE ;

vector3 NavMapPosition = VEC3F_NULL ;

vector3 g_vStartPoint( VEC3F_NULL );
vector3 g_vDestPoint( VEC3F_NULL );
vector3         g_StartP1 = VEC3F_NULL;
vector3         g_EndP0 = VEC3F_NULL;

xarray<vector3> g_PathPoints;


//==============================================================================

void AddNodesToGrid(  nav_node_slot_id thisNodeID,  u8 thisID )
{
    ng_node& tempNode = g_NavMap.GetNodeByIndex(thisNodeID);

    s32 count;
    for(count = 0; count < tempNode.GetConnectionCount(); count++ )
    {
        ng_connection& tempConnection = tempNode.GetConnectionByIndex(count);

        ng_node& otherNode = tempConnection.GetOtherNode( thisNodeID );
        
        if( otherNode.GetGridID() == 255 )
        {
            otherNode.SetGridID( thisID );
            AddNodesToGrid(tempConnection.GetOtherNodeID( thisNodeID ), thisID );
        }


    }
}

//==============================================================================

void SetGridIDs(void)
{
    nav_map::node_data* nodes = g_NavMap.GetNodeData(); 
    nav_map::connection_data* connections = g_NavMap.GetConnectionData();

    ASSERT(nodes);
    ASSERT(connections);

    s32 count;

    //  First set all the nodes to a bogus grid number
    for ( count = 0; count < g_NavMap.GetNodeCount(); count++ )
    {
        nodes[count].m_GridID = 255;
    }

    u8 currentGridNumber = 0;

    for ( count = 0; count < g_NavMap.GetNodeCount(); count++ )
    {
        if( nodes[count].m_GridID == 255 )
        {
            nodes[count].m_GridID = currentGridNumber ;
            AddNodesToGrid( count , currentGridNumber );
            currentGridNumber++;
            //  Just a sanity check
            ASSERT(currentGridNumber < 200);
        }

    }
    
}

//==============================================================================

void Initialize( void )
{
    eng_Init();
    render_Init() ;

	g_NavMap.Load( "C:\\GameData\\A51\\Release\\PS2\\FourZones.nmp" ) ;
    SetGridIDs() ;
    x_memset( g_PathList, -1, sizeof( s32 ) * 20 );

    ASSERT( g_NavMap.m_bIsLoaded ) ;

    // The fucking map is 50 million miles away.
    NavMapPosition = g_NavMap.GetNodeByIndex( 0 ).GetPosition() ;
    g_vStartPoint = NavMapPosition;
    g_vStartPoint+=vector3( -1000.f, 0.f, 1000.f );
    g_vDestPoint = g_vStartPoint;
    

    View.SetXFOV( R_90 );
    View.SetPosition( vector3(250, 250, 250) + NavMapPosition );
    View.LookAtPoint( vector3(  0,  0,  0) + NavMapPosition );
    View.SetZLimits ( 0.1f, 100000.0f );

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );

    g_RscMgr.Init();
    
    //g_RscMgr.SetRootDirectory("") ;
    g_RscMgr.SetRootDirectory( "C:\\GameData\\A51\\Release\\PC");
    //g_RscMgr.SetRootDirectory( "E:\\GameData\\A51\\Release\\PC");
    g_RscMgr.SetOnDemandLoading( TRUE );
}

//=========================================================================

void Shutdown( void )
{
    render_Kill() ;
}

//=========================================================================

xbool HandleInput( void )
{
    while( input_UpdateState() )
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 1.125f;
        f32    R = 0.005f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_KBD_W       ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_S       ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_A       ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_D       ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_R       ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_F       ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );       
        Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
        Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );
    }

    return( TRUE );
}

//==============================================================================

void Render( void )
{
    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();



		// XZ - Plane
        draw_Grid( vector3(  -50000,   0,    -50000) + NavMapPosition , 
                   vector3(100000,  0,    0) , 
                   vector3(  0,   0, 100000) , 
                   xcolor (  0,128,  0, 128), 500 );
/*
		// YZ - Plane
        draw_Grid( ( vector3(  0,   -50000,    -50000 ) + NavMapPosition ), 
                   ( vector3(  0,  100000,    0) ), 
                   ( vector3(  0,   0, 100000) ), 
                   xcolor (  128, 0,  0, 255), 100 );

		// XZ - Plane
        draw_Grid(  ( vector3(  -50000,   -50000, 0) + NavMapPosition ), 
                    ( vector3(  0,  100000,    0 ) ), 
                    ( vector3(  100000,   0, 0) ), 
                      xcolor (  0, 0,  128, 255), 100 );
*/
    }
    eng_End();

    //  Render Planes
/*
	eng_Begin( "ThreePlanes") ;
    {
	    vector3 vOffset =  NavMapPosition - View.GetPosition() ;
        vOffset.NormalizeAndScale( -.1f ) ;
	    
	    draw_Marker( NavMapPosition , XCOLOR_WHITE );
	    draw_Line( NavMapPosition + vOffset, NavMapPosition + vOffset + vector3( 50000.f, 0.f, 0.f ) ) ;
	    draw_Line( NavMapPosition + vOffset, NavMapPosition + vOffset + vector3( 0.f, 50000.f, 0.f ) ) ;
	    draw_Line( NavMapPosition + vOffset, NavMapPosition + vOffset + vector3( 0.f, 0.f, 50000.f ) ) ;
    
        eng_End();    
    }
*/
    eng_Begin( "NavMap" ) ;
    {
        g_NavMap.RenderExportedMap() ;
    }
    eng_End() ;

    eng_Begin( "Global point" );
    {
        draw_Sphere( g_vStartPoint, 12.f, XCOLOR_GREEN );
        draw_Sphere( g_vDestPoint, 10.f, XCOLOR_YELLOW );
    }
    eng_End();

    if ( g_bPathComputed )
    {
        eng_Begin( "Render Path" );
        {
            // Try rendering my new path.
            vector3 vP0, vP1;
            vP0 = g_vStartPoint;
            vP1 = g_StartP1;
            vector3 vOffset( 0.f, 10.f, 0.f );

            for ( s32 i = 0; i < 20; i++ )
            {
                // Draw a line from vP0 to vP1
                draw_Line( vP0 + vOffset, vP1 + vOffset, XCOLOR_BLUE );
                vP0 = vP1;

                if ( g_PathList[i] == -1 )
                    break;

                // Ask the map to find the next destination.,
//                g_NavMap.FindNextDestination( g_PathList, sizeof( g_PathList ) / sizeof( s32 ), i, vP0, g_vDestPoint );


                // Now setup vP0 and vP1 for next time.
                vP1 = g_NavMap.GetNodeByIndex( g_PathList[i] ).GetPosition();
            }

            //Let's finish rendering the path.
            vP1 = g_EndP0;
            draw_Line( vP0 + vOffset, vP1 + vOffset, XCOLOR_BLUE );
            vP0 = vP1;
            vP1 = g_vDestPoint;
            draw_Line( vP0 + vOffset, vP1 + vOffset, XCOLOR_BLUE );
        }
        eng_End();

        g_PathFindStruct.RenderEdgePaths();


    }

}

//==============================================================================

void Advance( f32 Seconds )
{       
    if( input_IsPressed( INPUT_KBD_SPACE ) )
    {
        eng_Begin( "To Nearest Connection" );
        {
            for ( s32 count = 0; count < g_NavMap.GetConnectionCount(); count++ )
            {
                // Draw a line to each node's shortest point.
                vector3 vClosestPoint = g_NavMap.GetNearestPointOnConnection( count, g_vStartPoint );
                draw_Line( g_vStartPoint, vClosestPoint, XCOLOR_GREEN );
            }
        }
        eng_End();
    }

    if( input_IsPressed( INPUT_KBD_C ) )
    {
        
        for ( s32 i = 0; i < g_NavMap.GetConnectionCount(); i++ )
        {
            if ( i % 2 == 1 )
            {
                vector3 vCorners[4];
                g_NavMap.GetConnectionByIndex( i ).GetCorners( vCorners );

                eng_Begin( "Corners" );
                for ( s32 j = 0; j < 4; j++ )
                {
                    draw_Label( vCorners[j], XCOLOR_WHITE, "%i", j );
                }
                eng_End();
            }
        }
    }

    if( input_IsPressed( INPUT_KBD_T ) )
    {
/*
        g_NodeToTest = x_irand( 0, g_NavMap.GetNodeCount() -1 );

        while ( g_NavMap.GetNodeByIndex(g_NodeToTest).GetConnectionCount() < 2 )
        {
            g_NodeToTest = x_irand( 0, g_NavMap.GetNodeCount() -1 );
        }
*/
    }

    if( input_IsPressed( INPUT_KBD_G ) )
    {
        s32 i = 0;
        // Let's see my connection intersectin points.
        ASSERT( g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionCount() > 1 );
/*
        for ( s32 i = 1; i < g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionCount(); i++ )
        {
            g_NavMap.GetAngleBetweenConnections( g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionIDByIndex(i-1), g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionIDByIndex(i) );
        }

        return;
*/
        for ( i = 1; i < g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionCount(); i++ )
        {
            g_NavMap.GetClosestEdgeIntersectionPoint( g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionIDByIndex(i-1), g_NavMap.GetNodeByIndex( g_NodeToTest ).GetConnectionIDByIndex(i), g_vStartPoint );
        }
/*
        for ( i = 0; i < g_NavMap.GetNodeByIndex( g_NodeToTest).GetConnectionCount(); i++ )
        {
            vector3 vCorners[4];
            g_NavMap.GetConnectionByIndex( i ).GetCorners( vCorners );

            eng_Begin( "Corners" );
            for ( s32 j = 0; j < 4; j++ )
            {
                draw_Label( vCorners[j], XCOLOR_WHITE, "%i", j );
            }
            eng_End();
        }
*/
    }


    if( input_WasPressed( INPUT_KBD_U      ) )
    {
        s_DesPoint.X = s_AimPoint.X + x_frand(-100, 100) ;
        s_DesPoint.Z = s_AimPoint.Z + x_frand(-100, 100) ;
    }        

    if( input_WasPressed( INPUT_KBD_Q ) )
        s_bRandDest ^= TRUE ;

    if( input_WasPressed( INPUT_KBD_NUMPAD1 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.X -= 50.f;
            g_vStartPoint.Z -= 50.f;

        }
        else
        {
            g_vDestPoint.X -= 50.f;
            g_vDestPoint.Z -= 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD2 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.Z -= 50.f;

        }
        else
        {
            g_vDestPoint.Z -= 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD3 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.X += 50.f;
            g_vStartPoint.Z -= 50.f;

        }
        else
        {
            g_vDestPoint.X += 50.f;
            g_vDestPoint.Z -= 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD4 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.X -= 50.f;

        }
        else
        {
            g_vDestPoint.X -= 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD5 ) )
    {
        g_vStartPoint = NavMapPosition;
        g_vStartPoint+=vector3( -1000.f, 0.f, 1000.f );
        g_vDestPoint = g_vStartPoint;
     }

    if( input_WasPressed( INPUT_KBD_NUMPAD6 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.X += 50.f;

        }
        else
        {
            g_vDestPoint.X += 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD7 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.X += 50.f;
            g_vStartPoint.Z += 50.f;

        }
        else
        {
            g_vDestPoint.X += 50.f;
            g_vDestPoint.Z += 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD8 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.Z += 50.f;

        }
        else
        {
            g_vDestPoint.Z += 50.f;
        }        
    }

    if( input_WasPressed( INPUT_KBD_NUMPAD9 ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.X -= 50.f;
            g_vStartPoint.Z += 50.f;

        }
        else
        {
            g_vDestPoint.X -= 50.f;
            g_vDestPoint.Z += 50.f;
        }        
    }
    
    if (input_WasPressed( INPUT_KBD_Z ) )
    {
        if ( input_IsPressed( INPUT_KBD_LSHIFT ) )
        {
            g_vStartPoint.Z += 50.f;
        }
        else
        {
            g_vStartPoint.Z -= 50.f;
        }
    }


    if( input_WasPressed( INPUT_KBD_P ) )
    {
        // The new way to request a path is as follows:

        // 1. Find the closest edges to the start and end positions.  In game, we would use the connection_zone_mgr.  
        // Here, I can't b/c there is no spatial dbase.

        // Find the start connection.
        connection_slot_id StartConnectionSlot = g_NavMap.GetNearestConnectionNoZoneCheck( g_vStartPoint );
        ng_connection& StartConnection = g_NavMap.GetConnectionByIndex( StartConnectionSlot );

        // Find the end connection.
        connection_slot_id EndConnectionSlot = g_NavMap.GetNearestConnectionNoZoneCheck( g_vDestPoint );
        ng_connection& EndConnection = g_NavMap.GetConnectionByIndex( EndConnectionSlot );

        // Let's set the point on the first connection that I'm going to.
        g_StartP1 = g_NavMap.GetNearestPointOnConnection( StartConnectionSlot, g_vStartPoint );
        g_EndP0 = g_NavMap.GetNearestPointOnConnection( EndConnectionSlot, g_vDestPoint );


        // If the connections are the same, don't worry about pathfinding because we're there
        if ( StartConnectionSlot == EndConnectionSlot )
        {
            g_bPathComputed = TRUE;
            g_NavMap.CreateEdgeList( g_vStartPoint, g_vDestPoint, g_PathList, 20, g_PathFindStruct, 20, StartConnection, StartConnectionSlot, EndConnection, EndConnectionSlot );
            return;
        }

        // 2.  Pick one of the nodes on the connection and generate a path from it.

        // Find the start node
        ng_node& SourceNode = StartConnection.GetStartNode();

        // Find the end node.
        ng_node& DestNode = EndConnection.GetStartNode();     

        x_memset( g_PathList, -1, sizeof( s32 ) * 20 );
        g_bPathComputed = Pathfinder.GeneratePath( &SourceNode, &DestNode, NULL, g_PathList, 20 );
        s32 i = 0;
        // 3.  If the other potential start node is on the path, We have chosen incorrectly and the path needs to be 'shifted' by one.
        if ( StartConnection.GetEndNodeID() == g_PathList[1] )
        {
            for ( i = 1; i < 20; i++ )
            {
                // Shift it all down one slot.
                g_PathList[i-1] = g_PathList[i];
            }
            g_PathList[19] = -1;
        }

        // 4.  If the other potential end node is on the path, then we don't want to use the end node that the pathfinder found.
        for ( i = 19; i > 0; i-- )
        {
            if ( g_PathList[i] > 0 )
            {
                if ( EndConnection.GetEndNodeID() == g_PathList[i -1] )
                {
                    g_PathList[i] = -1;
                }
                break;
            }
        }

        // Path is ready.  Let's initialize the edges.
        if ( g_bPathComputed )
        {
            g_NavMap.CreateEdgeList( g_vStartPoint, g_vDestPoint, g_PathList, 20, g_PathFindStruct, 20, StartConnection, StartConnectionSlot, EndConnection, EndConnectionSlot );
        }
    }

}

//===========================================================================


//==============================================================================

void AppMain( s32, char** )
{
    Initialize();
    xtimer Timer;
   
    while( TRUE )
    {
        if( !HandleInput() )
            break;

        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );
        eng_ActivateView    ( 0 );

        Advance( Timer.TripSec() );

        Render();

        if( 0 )
        {
            irect Rect( 0,0, 128,128 );

            View2.SetPosition( s_AimPoint + vector3( 0,100,0) );//s_HazmatLoco.GetPosition() + vector3( 0, 100, 300 ) );
            View2.LookAtPoint( vector3( 0, 100,   0 ) );
            View2.SetViewport( Rect.l, Rect.t, Rect.r, Rect.b );
            eng_SetView         ( View2, 0 );
            eng_ActivateView    ( 0 );

            eng_Begin( "Clear");
            draw_GouraudRect( Rect,XCOLOR_BLACK,XCOLOR_BLACK,XCOLOR_BLACK,XCOLOR_BLACK, FALSE );
            draw_ClearZBuffer( Rect );
            eng_End();

            Render();
        }

        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

