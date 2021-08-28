
#include "ZoneMgr.hpp"

//=========================================================================
// VARIABLES
//=========================================================================

zone_mgr g_ZoneMgr;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

void zone_mgr::Clean( void )
{
    if( m_pPortal           ) delete []m_pPortal;
    if( m_pZone             ) delete []m_pZone;
    if( m_pZone2Portal      ) delete []m_pZone2Portal;
    if( m_pVisivilityBits   ) delete []m_pVisivilityBits;

    m_nPortals          = 0;
    m_pPortal           = NULL;
    m_nZones            = 0;
    m_pZone             = NULL;
    m_nZone2Portal      = 0;
    m_pZone2Portal      = NULL;
    m_pVisivilityBits   = NULL;
    m_bAddMode          = FALSE;
    m_MaxPortals        = 0;
    m_nFrustums         = 0;
}

//=========================================================================

zone_mgr::zone_mgr( void )
{
    m_nPortals          = 0;
    m_pPortal           = NULL;
    m_nZones            = 0;
    m_pZone             = NULL;
    m_nZone2Portal      = 0;
    m_pZone2Portal      = NULL;
    m_pVisivilityBits   = NULL;
    m_bAddMode          = FALSE;
    m_nFrustums         = 0;
}

//=========================================================================

zone_mgr::~zone_mgr( void )
{
    if( m_pPortal           ) delete []m_pPortal;
    if( m_pZone             ) delete []m_pZone;
    if( m_pZone2Portal      ) delete []m_pZone2Portal;
    if( m_pVisivilityBits   ) delete []m_pVisivilityBits;
}

//=========================================================================

void zone_mgr::AddStart( s32 nZones, s32 nPortals )
{
    ASSERT( m_bAddMode == FALSE );
    ASSERT( m_pPortal  == NULL  );
    ASSERT( m_pZone    == NULL  );

    // Allocate zones and portals
    m_nPortals   = 0;
    m_MaxPortals = nPortals;
    m_pPortal    = new portal[ m_MaxPortals ];
    if( m_pPortal == NULL ) x_throw( "out of memory");

    m_nZones    = nZones;
    m_pZone     = new zone[ m_nZones ];
    if( m_pZone == NULL ) x_throw( "out of memory");

    // indicate that we are in adding mode
    m_bAddMode = TRUE;
}

//=========================================================================

void zone_mgr::AddPortal( bbox& BBox, vector3* pEdges, s32 ZoneA, s32 ZoneB )
{
    ASSERT( m_bAddMode );    
    ASSERT( m_nPortals < m_MaxPortals );

    portal& Portal = m_pPortal[ m_nPortals++ ];

    Portal.BBox         = BBox;
    Portal.iZone[0]     = ZoneA;
    Portal.iZone[1]     = ZoneB;
    Portal.Edges[0]     = pEdges[0];
    Portal.Edges[1]     = pEdges[1];
    Portal.Edges[2]     = pEdges[2];
    Portal.Edges[3]     = pEdges[3];
    Portal.Flags        = 0;
    Portal.bStack       = FALSE;
    Portal.Plane.Setup( pEdges[0], pEdges[1],  pEdges[3] );
}

//=========================================================================

void zone_mgr::AddZone( bbox& BBox, s32 ZoneID )
{
    ASSERT( m_bAddMode );    
    ASSERT( ZoneID < m_nZones );

    zone& Zone          = m_pZone[ ZoneID ];
    Zone.BBox           = BBox;
    Zone.nPortals       = 0;
    Zone.iPortal2Portal = 0;
}

//=========================================================================

void zone_mgr::AddEnd( void )
{   
    s32 i, j;

    ASSERT( m_bAddMode );
    ASSERT( m_nPortals == m_MaxPortals );

    //
    // First lets find how many zones to portals 
    //
    m_nZone2Portal = 0;
    for( i=0; i<m_nZones; i++ )
    {
        zone& Zone = m_pZone[i];

        for( j=0; j<m_nPortals; j++ )
        {
            portal& Portal = m_pPortal[j];

            if( Portal.iZone[0] == i || Portal.iZone[1] == i )
            {
                m_nZone2Portal++;
                Zone.nPortals++;
            }
        }
    }

    //
    // Okay we are really to create the zone 2 portal mapping
    //

    // Allocate the zone to portal buffer
    m_pZone2Portal = new s32[ m_nZone2Portal ];
    if( m_pZone2Portal == NULL )
        x_throw( "out of memory" );

    // Set the zones indices
    s32 iCursor = 0;
    for( i=0; i<m_nZones; i++ )
    {
        zone& Zone = m_pZone[i];

        Zone.iPortal2Portal = iCursor;

        for( j=0; j<m_nPortals; j++ )
        {
            portal& Portal = m_pPortal[j];

            if( Portal.iZone[0] == i || Portal.iZone[1] == i )
            {
                m_pZone2Portal[ iCursor++ ] = j;
                ASSERT( iCursor <= m_nZone2Portal );
            }
        }

        ASSERT( Zone.nPortals == (iCursor-Zone.iPortal2Portal) );
    }
    ASSERT( iCursor == m_nZone2Portal );

    //
    // TODO: Compute the zone visibility
    // Given two portals try to see if you can see portal #3 This will tell whether
    // zone a can see zone b.
    //
}


//=========================================================================

xbool zone_mgr::ComputeFrustum( 
    frustum&        NewFrustum, 
    const frustum&  CurrentFrustum, 
    const vector3&  EyePosition, 
    const portal&   Portal ) const    
{
    s32     i;
    s32     Count[2];
    vector3 Edges[2][16];

    //
    // Get ready to start the process.
    // Make sure to that the portal always represets the near clip plane.
    // so if we are in the back of the plane we must reverse the edges
    //
    if( Portal.Plane.InBack( EyePosition ) )
    {
        Count[1] = 4;
        for( i=0; i<4; i++ )
        {
            Edges[1][i] = Portal.Edges[i];
        }
    }
    else
    {
        Count[1] = 4;
        for( i=0; i<4; i++ )
        {
            Edges[1][i] = Portal.Edges[3-i];
        }
    }

    //
    // Start clipping away
    //
    for( i=0; i<CurrentFrustum.nPlanes; i++ )
    {
        const s32 iNew = i&1;
        const s32 iOld = 1 - (i&1);

        CurrentFrustum.Plane[i].ClipNGon( Edges[iNew], Count[iNew], Edges[iOld], Count[iOld] );
        ASSERT( Count[iNew] < 16 );

        // Did we clip all away?
        if( Count[iNew] == 0 )
            return FALSE;
    }

    //
    // Copy the final edges 
    //
    const s32 iNew = 1-(i&1);

    NewFrustum.nEdges = Count[iNew];
    ASSERT( NewFrustum.nEdges >= 3 );
    for( i=0; i<NewFrustum.nEdges; i++)
    {
        NewFrustum.Edges[i] = Edges[iNew][i];
    }

    //
    // Compute the new planes
    //

    // Compute all the general planes
    NewFrustum.nPlanes = NewFrustum.nEdges;
    for( i=0; i<NewFrustum.nPlanes; i++ )
    {
        NewFrustum.Plane[i].Setup( EyePosition, Edges[iNew][i], Edges[iNew][(i+1)%NewFrustum.nEdges] );
    }

    //
    // Now create the near plane
    //
    NewFrustum.nPlanes++;
    NewFrustum.Plane[i++].Setup( NewFrustum.Edges[0], NewFrustum.Edges[1], NewFrustum.Edges[2] );

    //
    // Also copy the far plane
    //
    NewFrustum.nPlanes++;
    NewFrustum.Plane[i++] = CurrentFrustum.Plane[CurrentFrustum.nPlanes-1];

    // make sure that something strange didn't happen.
    ASSERT( NewFrustum.nPlanes <= 16 );

    return TRUE;
}

//=========================================================================

void zone_mgr::PortalWalk( frustum* pFrustum, const frustum& ParentFrustum )
{    
    zone& Zone = m_pZone[ ParentFrustum.iZone ];

    // Now lets go throw all its portals and see what we get
    for( s32 i=0; i<Zone.nPortals; i++ )
    {
        const s32     Index      = m_pZone2Portal[ Zone.iPortal2Portal+i ];
        portal&       Portal     = m_pPortal[ Index ];
        frustum&      NewFrustum = pFrustum[ m_nFrustums ];

        // Check whether this portal is disable
        if( (Portal.Flags & PFLAGS_DISABLE) == PFLAGS_DISABLE ) 
            continue;

        // We already when down this portal so skip it
        // This case happens if we endup in a previouly visited zone
        if( Portal.bStack )
            continue;

        // marks this portalk as been in the stack
        Portal.bStack = TRUE;

        // Clip and compute the frustrum for the zone
        if( ComputeFrustum( NewFrustum, ParentFrustum, m_EyePosition, Portal ) )
        {
            // OKay here is the new frustum zone
            NewFrustum.iZone = ( Portal.iZone[0] == ParentFrustum.iZone )?Portal.iZone[1] : Portal.iZone[0];

            // Okay we have sucessfully build a new Frustum
            m_nFrustums++;
            ASSERT( m_nFrustums < MAX_FRUSTUMS );

            // Okay lets walk down this new zone and see what we see.
            PortalWalk( pFrustum, NewFrustum );
        }

        // clear the portal stack marker
        Portal.bStack = FALSE;
    }
}

//=========================================================================
// Note that under certain conditions one zone could have multiple frustrums
// Right now we are choose not to mergethe frustrums.
//=========================================================================
void zone_mgr::PortalWalk( const view& View, s32 iZone )
{
    vector3     EyePosition;

    ASSERT( iZone >= 0 );
    ASSERT( iZone < m_nZones );

    //
    // Build the main frustum
    //
    m_EyePosition        = View.GetPosition();
    m_Frustum[0].nPlanes = 6;
    m_Frustum[0].iZone   = iZone;
    View.GetViewPlanes( m_Frustum[0].Plane[0],    // Top
                        m_Frustum[0].Plane[1],    // Bottom
                        m_Frustum[0].Plane[2],    // Left
                        m_Frustum[0].Plane[3],    // Right
                        m_Frustum[0].Plane[4],    // Near
                        m_Frustum[0].Plane[5]);   // Far

    //
    // Okay lets start the walk
    //
    m_nFrustums = 1;
    PortalWalk( m_Frustum, m_Frustum[0] );

    //
    // TODO: here we may choose to merge frustrums that exits in the same zone.
    //       Some case this will work better some cases it will not. 
    //       I am not sure what is the way to go here.
    //
}

//=========================================================================

void zone_mgr::Render( void )
{
    s32 i;

    //
    // Render the portals 
    //
    draw_ClearL2W();

    if( 1 )
    {
        draw_Begin( DRAW_LINES );
        for( i=0; i<m_nPortals; i++ )
        {
            portal& Portal = m_pPortal[i];

            draw_Color( xcolor(255,255,255,255) );

            draw_Vertex( Portal.Edges[0] );
            draw_Vertex( Portal.Edges[1] );

            draw_Vertex( Portal.Edges[1] );
            draw_Vertex( Portal.Edges[2] );

            draw_Vertex( Portal.Edges[2] );
            draw_Vertex( Portal.Edges[3] );

            draw_Vertex( Portal.Edges[3] );
            draw_Vertex( Portal.Edges[0] );
        }
        draw_End();
    }

    //
    // Render the frustrums 
    //
    if( 1 )
    {
        x_srand( 12312 );
        draw_Begin( DRAW_TRIANGLES );
        for( i=1; i<m_nFrustums; i++ )
        {
            frustum& Frustum = m_Frustum[i];

            for( s32 j=0; j<Frustum.nPlanes; j++ )
            {
                s32     t;
                s32     Count[2];
                vector3 Points[2][16];
                vector3 V1, V2, V3;
                plane&  Plane = Frustum.Plane[j];

                // Compute the ortho vectors
                Plane.GetOrthoVectors( V1, V2 );

                // Build a big ass face
                V3 = Plane.Normal * -Plane.D;
                V1.Normalize();
                V2.Normalize();
                Points[1][0] = V3 + V1*-100000.0f + V2*+100000.0f;
                Points[1][1] = V3 + V1*-100000.0f + V2*-100000.0f;
                Points[1][2] = V3 + V1*+100000.0f + V2*-100000.0f;
                Points[1][3] = V3 + V1*+100000.0f + V2*+100000.0f;                

                // Set the initial count
                Count[1] = 4;

                // Now lets clip it base on the other planes
                s32 o=0;
                for( t=0; t<Frustum.nPlanes; t++ )
                {
                    // Skip this plane
                    if( t == j )
                        continue;

                    // Clip with plane
                    Frustum.Plane[t].ClipNGon( Points[o&1], Count[o&1], Points[1-(o&1)], Count[1-(o&1)] );
                    o++;
                }
    
                // Set t to contain the final index
                t = 1-(o&1);

                // Now we are ready to render it
                draw_Color( xcolor( x_rand() ) );
                for( s32 k=0; k < (Count[t]-2); k++ )
                {
                    draw_Vertex( Points[t][0] );
                    draw_Vertex( Points[t][k+1] );
                    draw_Vertex( Points[t][k+2] );
                }
            }
        }
        draw_End();   
    }

    //
    // Render edges
    //
    if( 1 )
    {
        x_srand( 12312 );
        draw_Begin( DRAW_LINES );
        for( i=1; i<m_nFrustums; i++ )
        {
            const frustum& Frustum = m_Frustum[i];

            draw_Color( xcolor( x_rand(), x_rand(), x_rand(), 255 ) );

            for( s32 k=0; k<Frustum.nEdges; k++ )
            {
                draw_Vertex( Frustum.Edges[k] );
                draw_Vertex( Frustum.Edges[(k+1)%Frustum.nEdges] );
            }
        }
        draw_End();
    }

}
