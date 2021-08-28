
#include "ZoneMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Objects\Object.hpp"

#ifndef X_EDITOR
#include "NetworkMgr\GameMgr.hpp"
#endif

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

//=========================================================================
// VARIABLES
//=========================================================================

zone_mgr g_ZoneMgr;

#define FLOAT1_TO_U8BIT( n )                    ((u8)((n * 255.0f) + 0.5f))        
#define U8BIT_TO_FLOAT1( n )                    ((f32)n / 255.0f)        
#define MAX_FARCLIP_SCALE                       10.0f
#define MIN_ZONE_VALUE                          0.01f

//==============================================================================
// DEBUG
//==============================================================================

s32     s_ParentZone        = -1;
s32     s_ParentPortal      = -1;
X_FILE* pFile               = NULL;
xbool   s_PrintPortalWalk   = FALSE;

//==============================================================================
// SORTING
//==============================================================================
/*
static
s32 WeightSort( const void* pItem1, const void* pItem2 )
{
    zone_mgr::node* pNode1 = (zone_mgr::node*)pItem1;
    zone_mgr::node* pNode2 = (zone_mgr::node*)pItem2;

    f32 NearClip, FarClip;
    g_AudioManager.GetClipRange( NearClip, FarClip );
    
    // New nodes the volume weight.
    f32 Percent1 = MAX( 1.0f-(pNode1->Distance / (FarClip*MAX_FARCLIP_SCALE)), 0.0f );

    f32 Weight1  = pNode1->SndWeight * Percent1;

    // New nodes the volume weight.
    f32 Percent2 = MAX( 1.0f-(pNode2->Distance / (FarClip*MAX_FARCLIP_SCALE)), 0.0f );

    f32 Weight2  = pNode2->SndWeight * Percent2;

    // The lowest weight goes first.
    if( Weight1 > Weight2 )
        return -1;
    else if( Weight1 == Weight2 )
        return 0;
    else
        return 1;
}
*/
//-------------------------------------------------------------------------------

/*
static
s32 DistanceSort( const void* pItem1, const void* pItem2 )
{
    zone_mgr::node* pNode1 = (zone_mgr::node*)pItem1;
    zone_mgr::node* pNode2 = (zone_mgr::node*)pItem2;

    // The lowest distance goes first.
    if( pNode1->Distance < pNode2->Distance )
        return -1;
    else if( pNode1->Distance == pNode2->Distance )
        return 0;
    else
        return 1;
}
*/

//-------------------------------------------------------------------------------

/*
static
s32 ZoneIndexSort( const void* pItem1, const void* pItem2 )
{
    zone_mgr::node* pNode1 = (zone_mgr::node*)pItem1;
    zone_mgr::node* pNode2 = (zone_mgr::node*)pItem2;

    // Smallest zone index goes first.
    if( pNode1->iZone < pNode2->iZone )
        return -1;
    else if( pNode1->iZone == pNode2->iZone )
        return 0;
    else
        return 1;
}
*/

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

void zone_mgr::Reset( void )
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

    m_GuidLookup.Clear();
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
    (void)nZones;

    ASSERT( m_bAddMode == FALSE );
    ASSERT( m_pPortal  == NULL  );
    ASSERT( m_pZone    == NULL  );

    // Allocate zones and portals
    m_nPortals   = 0;
    m_MaxPortals = nPortals;
    m_pPortal    = new portal[ m_MaxPortals ];
    if( m_pPortal == NULL ) x_throw( "out of memory");

    m_nZones    = 256; // +1 because zone 0 actually is an empty zone
    m_pZone     = new zone[ m_nZones ];
    if( m_pZone == NULL ) x_throw( "out of memory");
    x_memset( m_pZone, 0, sizeof(zone) * m_nZones );

    // Initialize zone zero
    m_pZone[0].BBox.Clear();
    m_pZone[0].nPortals       = 0;
    m_pZone[0].iPortal2Portal = 0;

    // indicate that we are in adding mode
    m_bAddMode = TRUE;

    // Set the capacity for the guid lookup
    m_GuidLookup.SetCapacity( nPortals, FALSE );
}


//=========================================================================

void zone_mgr::AddPortal( guid Guid, const bbox& BBox, vector3* pEdges, s32 ZoneA, s32 ZoneB, f32 SoundOcclusion )
{
    ASSERT( m_bAddMode );    
    ASSERT( m_nPortals < m_MaxPortals );

    s32     Index  = m_nPortals;
    portal& Portal = m_pPortal[ m_nPortals++ ];

    Portal.Guid         = Guid;
    Portal.BBox         = BBox;
    Portal.iZone[0]     = ZoneA;
    Portal.iZone[1]     = ZoneB;
    Portal.Edges[0]     = pEdges[0];
    Portal.Edges[1]     = pEdges[1];
    Portal.Edges[2]     = pEdges[2];
    Portal.Edges[3]     = pEdges[3];
    Portal.Flags        = 0;
    Portal.Plane.Setup( pEdges[0], pEdges[1],  pEdges[3] );
    Portal.Occlusion    = SoundOcclusion;
    Portal.BaseOcclusion= SoundOcclusion;

    m_GuidLookup.Add( Guid, Index );
}

//=========================================================================

void zone_mgr::AddZone( const bbox& BBox, 
                              s32   ZoneID, 
                              f32   SndWeight, 
                              s32   MinPlayers,
                              s32   MaxPlayers,
                        const char* EnvMap, 
                        const char* FogName,
                              xbool QuickFog )
{
    ASSERT( m_bAddMode );    
    ASSERT( ZoneID < m_nZones );

    zone& Zone          = m_pZone[ ZoneID ];
    Zone.BBox           = BBox;
    Zone.nPortals       = 0;
    Zone.iPortal2Portal = 0;
    Zone.bStack         = FALSE;
    Zone.SndWeight      = SndWeight;

    Zone.MinPlayers     = MinPlayers;
    Zone.MaxPlayers     = MaxPlayers;

    x_strsavecpy( Zone.EnvMapName, EnvMap,  128 );
    x_strsavecpy( Zone.FogName,    FogName, 128 );
    Zone.QuickFog = QuickFog;

#ifndef X_EDITOR
    GameMgr.SetZoneLimits( ZoneID, MinPlayers, MaxPlayers );
#else
    (void)MinPlayers;
    (void)MaxPlayers;
#endif
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
    const zone&     Zone,
    frustum&        NewFrustum, 
    const frustum&  CurrentFrustum, 
    const vector3&  EyePosition, 
    const portal&   Portal ) const    
{
    s32     i;
    s32     Count[2];
    vector3 Edges[2][MAX_PLANES];

    //
    // TODO: Make sure that all the edges that make the frustrum have a larger length of 0
    // If not it will crash.
    //

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
        ASSERT( Count[iNew] < MAX_PLANES );

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
    NewFrustum.nPlanes = 0;
    for( i=0; i<NewFrustum.nEdges; i++ )
    {
        const vector3& V1    = EyePosition; 
        const vector3& V2    = Edges[iNew][i];
        const vector3& V3    = Edges[iNew][(i+1)%NewFrustum.nEdges];
        plane&         Plane = NewFrustum.Plane[ NewFrustum.nPlanes ];

        Plane.Normal = v3_Cross( V2-V1, V3-V1 );
        if( Plane.Normal.LengthSquared() < 0.00001f ) continue;

        Plane.Normal.Normalize();
        Plane.D = -Plane.Normal.Dot( V1 );

        // Okay ready for the next
        NewFrustum.nPlanes++;
    }

    if( NewFrustum.nPlanes < 3 )
        return FALSE;

    //
    // Now create the near plane
    // TODO: This could also be done using the plane of the portal.
    //       Then we will determine whether the eye is in the front or the back
    //       (It needs to be in the back) and we will flip the plane acordenly.
    //       This will be faster but I am too lazy right now.
    //
    NewFrustum.nPlanes++;
    NewFrustum.Plane[i++].Setup( NewFrustum.Edges[0], NewFrustum.Edges[1], NewFrustum.Edges[2] );

    //
    // Also Create the far plane.
    // 
    //

    // Compute the maximun distance of the far plane base on the bbox of the zone
    const f32*   pF     = (f32*)&Zone.BBox;
    f32 MaxDist = m_Far.Normal.GetX() * pF[m_FarMinIndex[0]] +
                  m_Far.Normal.GetY() * pF[m_FarMinIndex[1]] +
                  m_Far.Normal.GetZ() * pF[m_FarMinIndex[2]] +
                  m_Far.D;

    NewFrustum.nPlanes++;
    NewFrustum.Plane[i].Normal = m_Far.Normal;
    NewFrustum.Plane[i].D      = m_Far.D - MaxDist;
    i++;

    // make sure that something strange didn't happen.
    ASSERT( NewFrustum.nPlanes <= MAX_PLANES );

    return TRUE;
}

//=========================================================================

void zone_mgr::Search( f32* pVolumes, f32 Volume, s32 ZoneID, s32 Depth )
{
    zone& Zone = m_pZone[ ZoneID ];

    // Tell the system that we already tried this one.
    Zone.bStack = TRUE;

    // Set the volume.
    pVolumes[ZoneID] = MAX( pVolumes[ZoneID], Volume );

    // Only if we are not too deep...
    if( (Depth < 4) && (Volume > 0.01f) )
    {
        // Now lets go thru all its portals and see what we get
        for( s32 i=0; i<Zone.nPortals; i++ )
        {
            const s32 Index  = m_pZone2Portal[ Zone.iPortal2Portal+i ];
            portal&   Portal = m_pPortal[ Index ];

            // Find which is going to be our new zone.
            s32 iNewZone = ( Portal.iZone[0] == ZoneID ) ? Portal.iZone[1] : Portal.iZone[0];

            // Is the next zone already in the stack?
            if( m_pZone[iNewZone].bStack )
                continue;

            // If its really quiet, dont continue.
            if( Portal.Occlusion < 0.01f )
                continue;
        
            // Recurse.
            Search( pVolumes, Volume*Portal.Occlusion, iNewZone, Depth+1 );
        }
    }
    
    // Woot, done!
    Zone.bStack = FALSE;
}

//=========================================================================

void zone_mgr::UpdateEar( s32 EarID )
{
    f32     ZoneVolumes[MAX_ZONES];
    matrix4 W2V;
    vector3 Position;
    s32     ZoneID;
    f32     Volume;

    g_AudioMgr.GetEar( EarID, W2V, Position, ZoneID, Volume );

    if( (ZoneID >=0) && (ZoneID < ZONELESS) ) 
    {
        // Clear them all.
        for( s32 i=0 ; i<MAX_ZONES ; i++ )
        {
            ZoneVolumes[i] = 0.0f;
        }

        ZoneVolumes[ ZONELESS ] = 1.0f;
        ZoneVolumes[ ZoneID ]   = 1.0f;

        Search( ZoneVolumes, 1.0f, ZoneID, 0 );
        g_AudioMgr.UpdateEarZoneVolumes( EarID, ZoneVolumes );
    }
}

//=========================================================================

void zone_mgr::PortalWalk( frustum* pFrustum, const frustum& ParentFrustum )
{    
    zone& Zone = m_pZone[ ParentFrustum.iZone ];

    // Tell the system that we already when throw this zone
    Zone.bStack = TRUE;

    // Now lets go throw all its portals and see what we get
    for( s32 i=0; i<Zone.nPortals; i++ )
    {
        const s32     Index      = m_pZone2Portal[ Zone.iPortal2Portal+i ];
        portal&       Portal     = m_pPortal[ Index ];
        frustum&      NewFrustum = pFrustum[ m_nFrustums ];

        // Check whether this portal is disable
        if( (Portal.Flags & PFLAGS_DISABLE) == PFLAGS_DISABLE ) 
            continue;

        // 
        // +-------------------------+ If we are trying to solve a portal 
        // |           Zone1         | of a zone which we have already
        // |       +------------+    | visited we can skip it. In this example
        // |       | +--------+ |    | the view starts at zone1 then goes throw
        // |       | | Zone2  | |    | Portal1 (P1) amd that takes to an unvisited
        // | View  +-+        +-+    | Zone2 but this zone has a Portal going back
        // |  *<    | P1       | P2  | to Zone1 so this if statement tells the
        // |       +-+        +-+    | system not to go down this portal.
        // |       | |        | |    |
        // +-------+ |        | +----+
        //           +--------+  
        //
        if( m_pZone[Portal.iZone[0]].bStack && m_pZone[Portal.iZone[1]].bStack ) 
            continue;

        //
        // TODO: If zone1 has 2 portals and you can see portal2 throw portal1 
        //       portal1 should became an antiportal znd acluded portal2.
        //       this requieres may be a bit too much work. 
        //

        // Find which is going to be our new zone
        s32 iNewZone = ( Portal.iZone[0] == ParentFrustum.iZone )?Portal.iZone[1] : Portal.iZone[0];

        // Clip and compute the frustrum for the zone
        if( ComputeFrustum( m_pZone[iNewZone], NewFrustum, ParentFrustum, m_EyePosition, Portal ) )
        {
            // OKay here is the new frustum zone and portal
            NewFrustum.iPortal = Index;
            NewFrustum.iZone   = iNewZone;

            // Compute the bbox masks for this Frustum
            for( s32 j=0; j<NewFrustum.nPlanes;j++)
            {
                GetBBoxMaxNormalMasks( NewFrustum.Plane[j], NewFrustum.NormalMasks[j*2+0], NewFrustum.NormalMasks[j*2+1] );
            }

            // Okay we have sucessfully build a new Frustum
            m_nFrustums++;
            ASSERT( m_nFrustums < MAX_FRUSTUMS );

            // Okay lets walk down this new zone and see what we see.
            PortalWalk( pFrustum, NewFrustum );
        }
    }


    // OKay we pop this zone out
    Zone.bStack = FALSE;
}

//=========================================================================
// Note that under certain conditions one zone could have multiple frustrums
// Right now we are choose not to mergethe frustrums.
//=========================================================================
void zone_mgr::PortalWalk( const view& View, s32 iZone )
{
    CONTEXT( "zone_mgr::PortalWalk" );

    s32         i;

    // Nothing to do.
    if( iZone == 0 )
    {
        m_nFrustums = 0;
        return;
    }
        
    ASSERT( iZone < m_nZones );

    //
    // Clear the zone to frustrum look up
    //
    for( i=0; i<256;i++)
    {
        m_ZoneToFrustum[i] = -1;
    }

    // Save the eye position
    m_EyePosition        = View.GetPosition();

    //
    // Build the main frustum
    //
    m_nFrustums          = 1;
    m_Frustum[0].nPlanes = 5;
    m_Frustum[0].iZone   = iZone;
    m_Frustum[0].iPortal = -1;
    plane Near;
    View.GetViewPlanes( m_Frustum[0].Plane[0],    // Top
                        m_Frustum[0].Plane[1],    // Bottom
                        m_Frustum[0].Plane[2],    // Left
                        m_Frustum[0].Plane[3],    // Right
                        Near,                     // Near 
                        m_Frustum[0].Plane[4]);   // Far

    // Compute the bbox indices for this Frustum
    for( i=0; i<m_Frustum[0].nPlanes;i++)
    {
        GetBBoxMaxNormalMasks( m_Frustum[0].Plane[i], m_Frustum[0].NormalMasks[i*2+0], m_Frustum[0].NormalMasks[i*2+1] );
    }

    // Compute the max indices for the far plane
    s32 NearMinIndex[3]; 
    m_Far = m_Frustum[0].Plane[4];
    m_Far.GetBBoxIndices( m_FarMinIndex, NearMinIndex );

    // Compute the zone far plane base on the zone max bbox
    const f32*   pF     = (f32*)&m_pZone[iZone].BBox;    
    f32 MaxDist = m_Far.Normal.GetX() * pF[m_FarMinIndex[0]] +
                  m_Far.Normal.GetY() * pF[m_FarMinIndex[1]] +
                  m_Far.Normal.GetZ() * pF[m_FarMinIndex[2]] +
                  m_Far.D;

    m_Frustum[0].Plane[4].D -= MaxDist;

    //
    // Okay lets start the walk
    //
    PortalWalk( m_Frustum, m_Frustum[0] );

    //
    // Insert all the frustum into the lookup table
    //
    for( i=0; i<m_nFrustums; i++ )
    {
        m_Frustum[i].iNext = m_ZoneToFrustum[ m_Frustum[i].iZone ];        
        m_ZoneToFrustum[ m_Frustum[i].iZone ] = (s8)i;
    }

    //
    // TODO: here we may choose to merge frustrums that exits in the same zone.
    //       Some case this will work better some cases it will not. 
    //       I am not sure what is the way to go here.
    //
}

//=========================================================================

s32 zone_mgr::GetLastPortalWalkZone( void ) const
{
    // Last zone walked is tored here
    return m_Frustum[0].iZone ;
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

void zone_mgr::Render( void ) const
{
    s32 i;

    if( eng_Begin("ZoneMgrDebugRender") )
    {
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
            draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_CULL_NONE | DRAW_NO_ZWRITE );
            for( i=0; i<m_nFrustums; i++ )
            {
                const frustum& Frustum = m_Frustum[i];

                xcolor C( x_rand() );
                C.A = 64;
                draw_Color( C );
                for( s32 j=0; j<Frustum.nPlanes; j++ )
                {
                    s32     t;
                    s32     Count[2];
                    vector3 Points[2][MAX_PLANES];
                    vector3 V1, V2, V3;
                    const plane&  Plane = Frustum.Plane[j];

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
        // Render edges of frustrums 
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

        //
        // Render bbox for the zones
        //
        if( 1 )
        {
            for( i=0; i<m_nZones; i++ )
            {
                draw_BBox( m_pZone[i].BBox );
            }
        }

        //
        // Render bbox for the portals
        //
        if( 1 )
        {
            for( i=0; i<m_nPortals; i++ )
            {
                draw_BBox( m_pPortal[i].BBox );
            }
        }

        eng_End();
    }
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

#ifdef TARGET_PS2
// NOTE: Blech. I don't like putting this here, but we really don't have a
// good place for it. Using draw is not sufficient because drawing a big
// rectangle is VERY inefficient for the hardware. The PS2 likes long
// vertical strips for good fill-rate.
static
void ps2_FullScreenQuad( xcolor Color )
{
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );

    s32 ColumnWidth = 64;
    s32 nColumns    = XRes / ColumnWidth;
    Color.A = (Color.A==255) ? 128 : (Color.A>>1);

    gsreg_Begin( 2 + nColumns*2 );
    gsreg_Set( SCE_GS_PRIM, SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( Color.R, Color.G, Color.B, Color.A, 0x3f800000 ) );

    s32 X  = (2048-(VRAM_FRAME_BUFFER_WIDTH/2));
    s32 Y0 = (2048-(VRAM_FRAME_BUFFER_HEIGHT/2));
    s32 Y1 = Y0 + YRes;
    s32 i;
    for( i = 0; i < nColumns; i++ )
    {
        s32 X0 = X;
        s32 X1 = X0 + ColumnWidth;

        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4), (Y0 << 4), 0 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4), (Y1 << 4), 0 ) );

        X += ColumnWidth;
    }

    gsreg_End();
}
#endif

//=========================================================================

void zone_mgr::RenderMPZoneStates( void ) const
{
    #ifdef TARGET_XBOX
    ASSERT( eng_InBeginEnd() );

    extern void xbox_SetBackSurfaceWithZ( xbool Enable );

    xbox_SetBackSurfaceWithZ( TRUE );
    if( GameMgr.IsZoneColored( m_Frustum[0].iZone ) )
    {
        g_pd3dDevice->Clear( 0,0,D3DCLEAR_STENCIL,0,0.0f,1 );
    }
    else
    {
        g_pd3dDevice->Clear( 0,0,D3DCLEAR_STENCIL,0,0.0f,0 );
    }

    // Now loop through all the visible portals and determine if they need
    // to set or clear a stencil bit. Note that because the frustums are always
    // listed in front-to-back order, this should be perfectly safe to do and
    // *should* handle the case of looking through multiple portals.

    g_RenderState.Set( D3DRS_STENCILPASS,D3DSTENCILOP_REPLACE );
    g_RenderState.Set( D3DRS_STENCILZFAIL,D3DSTENCILOP_KEEP );
    g_RenderState.Set( D3DRS_STENCILFUNC,D3DCMP_ALWAYS );
    g_RenderState.Set( D3DRS_STENCILENABLE, TRUE );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE,0 );

    s32 Count = 0;
    DWORD StencilTest;
    for( s32 i=1; i<m_nFrustums; i++ )
    {
        xbool bZoneLocked = GameMgr.IsZoneColored( m_Frustum[i].iZone );
        if( bZoneLocked )
            g_RenderState.Set( D3DRS_STENCILREF,1 );
        else
            g_RenderState.Set( D3DRS_STENCILREF,0 );

        draw_ClearL2W();
        draw_Begin( DRAW_TRIANGLES, DRAW_NO_ZWRITE | DRAW_USE_ALPHA | DRAW_CULL_NONE | DRAW_KEEP_STATES );
        draw_Color( xcolor(0,0,0,0) );

        const portal& Portal = m_pPortal[m_Frustum[i].iPortal];
        draw_Vertex( Portal.Edges[0] );
        draw_Vertex( Portal.Edges[1] );
        draw_Vertex( Portal.Edges[2] );
        draw_Vertex( Portal.Edges[2] );
        draw_Vertex( Portal.Edges[3] );
        draw_Vertex( Portal.Edges[0] );

        draw_End();
    }

    g_RenderState.Set( D3DRS_STENCILZFAIL,D3DSTENCILOP_KEEP );
    g_RenderState.Set( D3DRS_STENCILPASS,D3DSTENCILOP_KEEP );
    g_RenderState.Set( D3DRS_STENCILFUNC,D3DCMP_NOTEQUAL );
    g_RenderState.Set( D3DRS_STENCILREF,0 );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE,
        D3DCOLORWRITEENABLE_RED   |
        D3DCOLORWRITEENABLE_GREEN |
        D3DCOLORWRITEENABLE_BLUE  );

    rect Rect;
    eng_GetView()->GetViewport(Rect);

    extern xbool m_SatCompensation;
    xbool OldSat=m_SatCompensation;
    m_SatCompensation = FALSE;

    draw_Rect( irect( s32(Rect.Min.X),s32(Rect.Min.Y),s32(Rect.Max.X),s32(Rect.Max.Y) ),
            GameMgr.GetZoneColor(),
            FALSE );

    g_RenderState.Set( D3DRS_STENCILENABLE,FALSE );
    xbox_SetBackSurfaceWithZ( FALSE );
    m_SatCompensation = OldSat;
    #endif

    ///////////////////////////////////////////////////////////////////////////

    #ifdef TARGET_PS2
    ASSERT( eng_InBeginEnd() );

    // set up our frame buffer mask so that we only mess with the alpha
    // channel for now, and we never want to update the zbuffer
    gsreg_Begin( 3 );
    gsreg_SetFBMASK( 0x00ffffff );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_End();

    // initialize the stencil buffer based on whether or not we're in a
    // zone that is turned off at the moment
    if( GameMgr.IsZoneColored( m_Frustum[0].iZone ) )
    {
        ps2_FullScreenQuad( xcolor(0,0,0,255) );
    }
    else
    {
        ps2_FullScreenQuad( xcolor(0,0,0,0) );
    }

    // re-enable the z-buffer test
    gsreg_Begin( 1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    gsreg_End();

    // Now loop through all the visible portals and determine if they need
    // to set or clear a stencil bit. Note that because the frustums are always
    // listed in front-to-back order, this should be perfectly safe to do and
    // *should* handle the case of looking through multiple portals.
    s32 i;
    for( i = 1; i < m_nFrustums; i++ )
    {
        xbool bZoneLocked = GameMgr.IsZoneColored( m_Frustum[i].iZone );
        const portal& Portal = m_pPortal[m_Frustum[i].iPortal];

        draw_ClearL2W();
        draw_Begin( DRAW_TRIANGLES, DRAW_NO_ZWRITE | DRAW_USE_ALPHA | DRAW_CULL_NONE );
        gsreg_Begin( 1 );
        gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_ZERO, C_ZERO, A_SRC, C_DST, 0 ) );
        gsreg_End();

        if( bZoneLocked )
        {
            draw_Color( xcolor(255,255,255,255) );
        }
        else
        {
            draw_Color( xcolor(255,255,255,0) );
        }
        draw_Vertex( Portal.Edges[0] );
        draw_Vertex( Portal.Edges[1] );
        draw_Vertex( Portal.Edges[2] );

        draw_Vertex( Portal.Edges[2] );
        draw_Vertex( Portal.Edges[3] );
        draw_Vertex( Portal.Edges[0] );
        draw_End();
    }

    // now render a big alpha-blended quad using a dest alpha test
    gsreg_Begin( 4 );
    gsreg_SetFBMASK( 0x00000000 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   TRUE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_SetAlphaBlend( ALPHA_BLEND_INTERP, 0 );
    gsreg_End();

    // render a big quad that fills the screen red (using the stencil buffer)
    ps2_FullScreenQuad( GameMgr.GetZoneColor() );

    // re-enable the z-buffer test
    gsreg_Begin( 1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    gsreg_End();
    #endif
}

//=============================================================================

zone_mgr::zone_id zone_mgr::FindZone( const vector3& Position ) const
{
    for( s32 i=1; i<m_nZones; i++ )
    {
        if( m_pZone[i].BBox.Intersect( Position ) )
            return (zone_id)i;
    }

    return (zone_id)0;
}

//=========================================================================

xbool zone_mgr::BBoxInView( const frustum& Frustum, const bbox& BBox ) const
{
    // Collect all the data
    const plane*   pPlane = Frustum.Plane;
    const vector3* pMask  = Frustum.NormalMasks;

    // Loop through planes looking for a trivial reject.
    for( s32 i=0; i<Frustum.nPlanes; i++ )
    {
        // Compute max dist along normal
        f32 MaxDist = pMask[0].Dot( BBox.Min ) +
                      pMask[1].Dot( BBox.Max ) +
                      pPlane->D;

        // If outside plane, we are culled.
        if( MaxDist < 0 )
        {
            return FALSE;
        }

        // Move to next plane
        pMask += 2;
        pPlane++;
    }

    return TRUE;
} 

//=========================================================================

void zone_mgr::GetBBoxMaxNormalMasks( const plane& Plane, vector3& Mask0, vector3& Mask1 ) const
{
    // normally, to check if a bbox is completely behind a plane, you will find
    // the bbox's max point along the plane normal, and it's usually done like this:
    //
    // BBoxMax.X = Plane.Normal.X>=0 ? BBox.Max.X : BBox.Min.X
    // BBoxMax.Y = Plane.Normal.Y>=0 ? BBox.Max.Y : BBox.Min.Y
    // BBoxMax.Z = Plane.Normal.Z>=0 ? BBox.Max.Z : BBox.Min.Z
    // MaxDist = Plane.Normal.Dot( BBoxMax ) + Plane.D
    // if( MaxDist < 0 )
    //     BBox is behind plane
    //
    // We will try to go with an optimized route that gets rid of the comparisons
    // at run-time, and replaces them with masks, so it looks like this:
    // MaxDist = Plane.Normal DOT (BBox.Min*Mask0) + Plane.Normal DOT (BBox.Max*Mask1) + D
    //         = (Plane.Normal*Mask0) DOT BBox.Min + (Plane.Normal*Mask1) DOT BBox.Max + D
    //
    // Hopefully this will speed up the calculation time since we'll be using the vector unit
    // to be dealing with the dot products.
    //
    // So we just want to pre-calculate (Plane.Normal*Mask0) and (Plane.Normal*Mask1)
    Mask0.Zero();
    Mask1.Zero();
    if( Plane.Normal.GetX() >= 0.0f )   Mask1.GetX() = Plane.Normal.GetX();
    else                                Mask0.GetX() = Plane.Normal.GetX();
    if( Plane.Normal.GetY() >= 0.0f )   Mask1.GetY() = Plane.Normal.GetY();
    else                                Mask0.GetY() = Plane.Normal.GetY();
    if( Plane.Normal.GetZ() >= 0.0f )   Mask1.GetZ() = Plane.Normal.GetZ();
    else                                Mask0.GetZ() = Plane.Normal.GetZ();
}

//=========================================================================

xbool zone_mgr::IsBBoxVisible( const bbox& BBox, zone_id Zone1, zone_id Zone2 ) const
{
    // make sure that we have Frustums to check with
    if( m_nFrustums == 0 )
        return TRUE;

	// Must be in the global zone
    if( Zone1 == 0 && Zone2 == 0  )
        return TRUE;

	// Must be insize a portal
    if( Zone1 && Zone2 )
	{
		// if neither zone is visible then you can't really see it
		if( m_ZoneToFrustum[Zone1] == -1 && m_ZoneToFrustum[Zone2] == -1 )
			return FALSE;

		// other wise you should always be able to see it
		return TRUE;
	}

    // Check the bbox with the first zone
    if( Zone1 )
    {        
        for( s8 I = m_ZoneToFrustum[Zone1]; I != -1; I = m_Frustum[I].iNext )
        {

            if( BBoxInView( m_Frustum[I], BBox ) )
                return TRUE;
        }
    }

    // Check bbox witht he second bbox
    if( Zone2 )
    {        
        for( s8 I = m_ZoneToFrustum[Zone2]; I != -1; I = m_Frustum[I].iNext )
        {
            if( BBoxInView( m_Frustum[I], BBox ) )
                return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

xbool zone_mgr::LineCrossPortal( const portal& Portal, const vector3& P0, const vector3& P1 ) const
{
    f32 t;
    xbool bFront = Portal.Plane.InFront( P0 );

    if( bFront == Portal.Plane.InFront( P1 ) )
        return FALSE;


    if( Portal.Plane.Intersect( t, P0, P1 ) == FALSE )
        return FALSE;


    if( (t < 0) || (t > 1.0f) )
        return FALSE;    
    

    vector3 Point = P0 + t * (P1-P0);

    for( s32 i=0; i<4; i++ )
    {
        vector3 Normal = Portal.Plane.Normal.Cross( Portal.Edges[(i+1)%4] - Portal.Edges[ i ] );

        if( Normal.Dot( Point - Portal.Edges[i] ) < -0.0001f )
        {
            return FALSE;
        }
    }

    return TRUE;
}

//=========================================================================

void zone_mgr::MoveTracker( tracker& Tracker, const vector3& NewPosition ) const
{
    if( m_nZones == 0 )
    {
        return;
    }

    if( NewPosition == vector3(0,0,0) )
        return;

    vector3 Distance;

    Distance = NewPosition - Tracker.LastPosition;
    if( Distance.LengthSquared() < (0.1f*0.1f) )
        return;

    ASSERT( Tracker.iCurrentZone < m_nZones );
    zone&	Zone			= m_pZone[ Tracker.iCurrentZone ];
    
	f32		ClosestPortal	= 100000000.0f;
	s32		ClosestIndex	= -1;
	s32		i;    
            
	bbox MoveBBox;
	MoveBBox.Clear();

    vector3 Temp = NewPosition + Tracker.BBox.Min;
	MoveBBox += Temp;

    Temp = NewPosition + Tracker.BBox.Max;
	MoveBBox += Temp;

    Temp = Tracker.LastPosition + Tracker.BBox.Min;
	MoveBBox += Temp;

    Temp = Tracker.LastPosition + Tracker.BBox.Max;
	MoveBBox += Temp;

    //
    // Check if we are near a portal
    //
    for( i=0; i<Zone.nPortals; i++ )
    {
		s32   Index;

        Index = m_pZone2Portal[ Zone.iPortal2Portal + i ];
        portal& Portal = m_pPortal[ Index ];

        if( Portal.BBox.Intersect( MoveBBox ) )
		{
			vector3 ClosestPoint;
			f32 Distance = NewPosition.ClosestPointToRectangle( 
							Portal.Edges[0],
							Portal.Edges[1] - Portal.Edges[0],
							Portal.Edges[3] - Portal.Edges[0],
							ClosestPoint );

			if( Distance < ClosestPortal )
			{
				ClosestPortal = Distance;
				ClosestIndex  = Index;
			}
		}
    }

    //
    // Okay we are close to a portal check whether we have penetrade the portal
    //
    if( ClosestIndex != -1 )
    {
        portal& Portal = m_pPortal[ ClosestIndex ];

        if( LineCrossPortal( Portal, Tracker.LastPosition, NewPosition ) )
        {
            // Okay we must be in the other zone.
            Tracker.iCurrentZone = (Portal.iZone[0] == Tracker.iCurrentZone)?Portal.iZone[1]:Portal.iZone[0];
        }
    }

    //
    // Update tracker new position
    //
    Tracker.LastPosition = NewPosition;

    //
    // Update the temporart zone
    //
    if( ClosestIndex != -1)
    {
		portal& Portal = m_pPortal[ ClosestIndex ];		
		Tracker.iTempZone = (Portal.iZone[0] == Tracker.iCurrentZone)?Portal.iZone[1]:Portal.iZone[0];
	}
	else
	{
		Tracker.iTempZone = 0;
	}
}

//=========================================================================

void zone_mgr::InitZoneTracking( object& Object, tracker& Tracker ) const
{
    Tracker.SetPosition( Object.GetPosition() );
    Tracker.SetMainZone( (u8)Object.GetZone1() );
    Tracker.SetZone2   ( (u8)Object.GetZone2() );
}

//=========================================================================

void zone_mgr::UpdateZoneTracking( object& Object, tracker& Tracker, const vector3& NewPosition ) const
{
    // Get current zones from object
    Tracker.SetMainZone( (u8)Object.GetZone1() );
    Tracker.SetZone2   ( (u8)Object.GetZone2() );
    
    // Update tracker zone info
    MoveTracker( Tracker, NewPosition ); 
    
    // Update object zones
    Object.SetZone1( Tracker.GetMainZone() );
    Object.SetZone2( Tracker.GetZone2() );
}

//=========================================================================

void zone_mgr::TurnOff( void )
{
    m_nFrustums = 0;
}

//=========================================================================
// The save is kind of hack right now
void zone_mgr::Save( const char* pFileName )
{
    X_FILE* FP;
    FP = x_fopen( pFileName, "wb" );
    if( FP == NULL )
        x_throw( xfs("Unable to save the zone file[%s]", pFileName ));

    s32 Version = ZONE_MANAGER_VERSION;
    x_fwrite( &Version,         1,  sizeof(s32),            FP );
    x_fwrite( &m_nPortals,      1,  sizeof(m_nPortals),     FP );
    x_fwrite( &m_nZones,        1,  sizeof(m_nZones),       FP );
    x_fwrite( &m_nZone2Portal,  1,  sizeof(m_nZone2Portal), FP );

    x_fwrite( m_pPortal,        1, m_nPortals      *  sizeof(portal), FP );
    x_fwrite( m_pZone,          1, m_nZones        *  sizeof(zone),   FP );
    x_fwrite( m_pZone2Portal,   1, m_nZone2Portal  *  sizeof(s32),    FP );

    x_fclose( FP );
}

//=========================================================================
// The load is kind of hack right now
void zone_mgr::Load( const char* pFileName )
{
    MEMORY_OWNER( "zone_mgr::Load" );
    X_FILE* FP = NULL;

    // Make sure we are ready to load
    Reset();

    // Start loading
    x_try;

    FP = x_fopen( pFileName, "rb" );
    if( FP == NULL )
        x_throw( xfs("Unable to load the zone file[%s]", pFileName ));

    s32 Version;
    x_fread( &Version,         1,  sizeof(s32),            FP );
    if ( Version != ZONE_MANAGER_VERSION )
        x_throw( xfs("Zone file[%s] is outdated. Please re-export the level", pFileName ) );
    x_fread( &m_nPortals,      1,  sizeof(m_nPortals),     FP );
    x_fread( &m_nZones,        1,  sizeof(m_nZones),       FP );
    x_fread( &m_nZone2Portal,  1,  sizeof(m_nZone2Portal), FP );

    m_pPortal       = new portal[ m_nPortals ];
    m_pZone         = new zone  [ m_nZones ];
    m_pZone2Portal  = new s32   [ m_nZone2Portal ];

    if( m_pPortal == NULL || m_pZone == NULL || m_pZone2Portal == NULL ) 
        x_throw( xfs("Out of memory while trying to load the zone file [%s]", pFileName ));

    x_fread( m_pPortal,        1, m_nPortals      *  sizeof(portal), FP );
    x_fread( m_pZone,          1, m_nZones        *  sizeof(zone),   FP );
    x_fread( m_pZone2Portal,   1, m_nZone2Portal  *  sizeof(s32),    FP );

    x_fclose( FP );

    // make sure to add the guid lookups
    m_GuidLookup.SetCapacity( m_nPortals, FALSE );
    for( s32 i=0; i<m_nPortals; i++ )
    {    
        m_GuidLookup.Add( m_pPortal[i].Guid, i );
    }

    // Deal with problems
    x_catch_begin;

    if( FP ) x_fclose( FP );

    Reset();

    x_catch_end_ret;

#ifndef X_EDITOR
    // Set up the zone limits in GameMgr.
    GameMgr.SetZoneLimits( 0, 0, 32 ); 
    for( s32 i = 1; i < m_nZones; i++ )
    {
        GameMgr.SetZoneLimits( i, m_pZone[ i ].MinPlayers, m_pZone[ i ].MaxPlayers ); 
    }
#endif
}

//=========================================================================

zone_mgr::portal& zone_mgr::GetPortal( guid Guid ) 
{
    s32 Index;

    ASSERT( m_pPortal );

    if( m_GuidLookup.Find( Guid, Index ) )
    {
        ASSERT( Index < m_nPortals );
        return m_pPortal[ Index ];
    }
    
    ASSERT( 0 );
    x_throw( "Unable to get portal" );
    return m_pPortal[0];
}

//=========================================================================

zone_mgr::portal& zone_mgr::GetPortal( s32 ZoneID, s32 PortalIndex )
{
    ASSERT( ZoneID      >= 0                        );
    ASSERT( ZoneID      <  256                      );
    ASSERT( PortalIndex <  m_pZone[ZoneID].nPortals );
    ASSERT( PortalIndex >= 0                        );

    const zone& Zone     = m_pZone[ZoneID];
    s32         iPortal  = m_pZone2Portal[ Zone.iPortal2Portal + PortalIndex ];

    return m_pPortal[ iPortal ];
}

//=========================================================================

const zone_mgr::zone& zone_mgr::GetZone( s32 ZoneID )
{
    ASSERT( ZoneID      >= 0                        );
    ASSERT( ZoneID      <  256                      );
    
    return m_pZone[ ZoneID ];    
}

//=========================================================================

void zone_mgr::TurnPortalOff( guid Guid ) 
{
    s32 Index;

    if( m_nPortals == 0 )
        return;

    ASSERT( m_pPortal );

    if( m_GuidLookup.Find( Guid, Index ) )
    {
        m_pPortal[ Index ].Flags |= PFLAGS_DISABLE;            
    }
}

//=========================================================================

void zone_mgr::TurnPortalOn( guid Guid ) 
{
    s32 Index;

    if( m_nPortals == 0 )
        return;

    ASSERT( m_pPortal );

    if( m_GuidLookup.Find( Guid, Index ) )
    {
        m_pPortal[ Index ].Flags &= ~PFLAGS_DISABLE;    
    }
}

//=========================================================================

xbool zone_mgr::IsPortalOn( guid Guid )
{
    s32 Index;

    if( m_nPortals == 0 )
        return FALSE;

    ASSERT( m_pPortal );

    if( m_GuidLookup.Find( Guid, Index ) )
    {
        if (!(m_pPortal[ Index ].Flags & PFLAGS_DISABLE))
            return TRUE;
    }

    return FALSE;
}

//=========================================================================

void zone_mgr::SetPortalOcclusion( guid Guid, f32 Occlusion )
{
    s32 Index;

    if( m_nPortals == 0 )
        return;

    ASSERT( m_pPortal );

    if( m_GuidLookup.Find( Guid, Index ) )
    {
        m_pPortal[ Index ].Occlusion = Occlusion;    
    }
}

//=========================================================================

xbool zone_mgr::IsAdjacentZone( s32 Zone1, s32 Zone2 )
{
    if (m_nZones > Zone1)
    {
        for (s32 i = 0; i < m_pZone[Zone1].nPortals; i++)
        {
            //for each portal
            s32 index = m_pZone[Zone1].iPortal2Portal + i;

            if ( (m_nZone2Portal > index) && (m_nPortals > m_pZone2Portal[index] ) )
            {
                if (m_pPortal[m_pZone2Portal[index]].iZone[0] == Zone2 ||
                    m_pPortal[m_pZone2Portal[index]].iZone[1] == Zone2)
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

//=========================================================================

void zone_mgr::SanityCheck( void )
{
}

//=============================================================================

zone_mgr::tracker::tracker( void )
{
    LastPosition.Zero();
    iTempZone = iCurrentZone = 0;
    BBox.Set( vector3(0,0,0), 100 );
}

//=============================================================================

