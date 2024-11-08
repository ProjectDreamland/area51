//============================================================================
//
//  ng_connection2.cpp
//
//      Nav graph connection object.  Holds temp data and pointer to
//      connection data stored in the nav_map.
//
//
//============================================================================
#include "ng_connection2.hpp"
#include "ng_node2.hpp"


#include "../ZoneMgr/ZoneMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "../../MiscUtils/SimpleUtils.hpp"
#include "..\objects\object.hpp"
#include "../objects/Portal.hpp"

const f32   k_MaxYDiff = 200.0f;

//==============================================================================
ng_connection2::ng_connection2(void) :
    m_ConnectionData(NULL),
    m_Length(-1.0f)
{
    

}

//==============================================================================
ng_connection2::~ng_connection2()
{



}

//==============================================================================
//
//  Init
//
//      Sets up needed connection data
//  
//==============================================================================
void ng_connection2::Init ( nav_map::connection2_data* connectionData, 
                            nav_map *owner,
                            nav_connection_slot_id iSlot )
{
    ASSERT(owner);
    ASSERT(connectionData);
    m_Owner = owner;
    m_ConnectionData = connectionData;
    m_SlotID = iSlot;
}


//==============================================================================
//
//  Calculate Length
//
//      Gets called on connection creation to calc the length since it is used
//      heavily in the nav planning and it's somewhate expensive due to square
//      root call
//
//==============================================================================
void ng_connection2::CalculateLength ( void )
{
    m_Length = ( m_ConnectionData->m_StartPt - m_ConnectionData->m_EndPt ).Length();
}



//==============================================================================
//
//  GetDistanceToPoint
//
//      Basic distance between a line and a point formula
//
//==============================================================================
f32 ng_connection2::GetDistanceToPoint      ( const vector3& pointToCheck )
{
    vector3 startPoint, endPoint;
    startPoint = m_ConnectionData->m_StartPt;
    endPoint   = m_ConnectionData->m_EndPt;

    //  Formula I found on the web.  Probably far from optimal but it will work for
    //  now.  Replace it if it becomes a problem!

    vector3 v = endPoint - startPoint;
    vector3 w = pointToCheck - startPoint;

    f32 c1 = w.Dot(v);
    if ( c1 <= 0 )
        return ( ( pointToCheck - startPoint ).Length() );

    f32 c2 = v.Dot(v);
    if ( c2 <= c1 )
        return ( ( pointToCheck - endPoint ).Length() );

    f32 b = c1 / c2;
    vector3 Pb = startPoint + b * v;

    return ((pointToCheck - Pb).Length() );

}

//===========================================================================

xbool ng_connection2::IsPointInConnection( const vector3& pointToCheck, f32 bufferAmount )
{
    vector3 startNodePosition = m_ConnectionData->m_StartPt;
    vector3 endNodePosition = m_ConnectionData->m_EndPt;

    // do a quick y check, then ignore the y value.
    if( (pointToCheck.GetY() > startNodePosition.GetY() + k_MaxYDiff && pointToCheck.GetY() > endNodePosition.GetY() + k_MaxYDiff)
     || (pointToCheck.GetY() < startNodePosition.GetY() - k_MaxYDiff && pointToCheck.GetY() < endNodePosition.GetY() - k_MaxYDiff) )
    {    
        return FALSE;
    }

    startNodePosition.GetY() = 0;
    endNodePosition.GetY() = 0;

    vector3 Diff = vector3( pointToCheck.GetX(), 0.0f, pointToCheck.GetZ() ) - startNodePosition;
    vector3 Dir  = endNodePosition - startNodePosition;
    f32     T    = Diff.Dot( Dir );

    if( T > 0.0f )
    {
        f32 SqrLen = Dir.Dot( Dir );

        if ( T >= SqrLen )
        {
            return FALSE;
        }
        else
        {
            T    /= SqrLen;
            Diff -= T * Dir;
            return Diff.Length() <= GetWidth()+bufferAmount;
        }
    }
    return FALSE;
}

//===========================================================================

void ng_connection2::GetCorners( vector3* pCorners )
{
    // Get the line that runs down the center of this edge.
    vector3 Start = GetStartPosition();
    vector3 End   = GetEndPosition();
    
    vector3 vLineBetween = Start-End;

    // Cross with Y
    vector3 vY( 0.f, 1.f, 0.f );
    vector3 vPerpindicular = vLineBetween.Cross( vY );
    vPerpindicular.NormalizeAndScale( GetWidth() );

    // Find the corners.
    pCorners[0] = Start + vPerpindicular;
    pCorners[1] = End + vPerpindicular;
    pCorners[2] = End - vPerpindicular;
    pCorners[3] = Start - vPerpindicular;
}

//===========================================================================

xbool ng_connection2::ClipLine( vector3& LineStart, vector3& LineEnd )
{
    // Is start outside of connection?
    if( IsPointInConnection( LineStart, 0.0f ) == FALSE )
        return FALSE;
    
    // Get corners
    vector3 Corners[4];
    GetCorners( Corners );

    // Setup 2d line for intersection test
    vector3 Line2dStart = LineStart;
    vector3 Line2dEnd   = LineEnd;
    Line2dStart.GetY() = 0.0f;
    Line2dEnd.GetY() = 0.0f;
    
    // Flatten for 2d check
    Corners[0].GetY() = 0.0f;
    Corners[1].GetY() = 0.0f;
    Corners[2].GetY() = 0.0f;
    Corners[3].GetY() = 0.0f;
    
    // Check all connection edges
    for( s32 i = 0; i < 4; i++ )
    {
        // Get edge end pts
        const vector3& EdgeStart = Corners[ i ];
        const vector3& EdgeEnd   = Corners[ ( i == 3 ) ? 0 : ( i + 1 ) ];
        
        // Does 2d edge intersect 2d line?
        vector3 I;
        f32     TA,TB;
        if( x_IntersectLineSegLineSeg( EdgeStart, EdgeEnd, Line2dStart, Line2dEnd, I, TA, TB ) )
        {
            // Compute intersection pt in 3d
            LineStart += ( LineEnd - LineStart ) * TB;
            
            return TRUE;
        }
    }
    
    // No intersection found
    return FALSE;
}

//===========================================================================

#ifdef X_EDITOR

// We need to figure out what zones that this connection is in, and then increment the number of 
// connections in list by that much.
void ng_connection2::EditorGetZoneAndCount( xbool* pZoneArray, s32& nConnectionsInList )
{
    // First, figure out what zones that this guy is in.
    vector3 vStartPos = GetStartPosition();
    vector3 vEndPos = GetEndPosition();

    FindZonesSpanned( vStartPos, vEndPos, pZoneArray, nConnectionsInList );
}

//===========================================================================

void ng_connection2::FindZonesSpanned( vector3& vPosStart, vector3& vPosEnd, xbool* pZoneArray, s32& nConnectionsInList )
{
    u16 StartNodeZone = SMP_UTIL_GetZoneForPosition( vPosStart );
    u16 EndNodeZone = SMP_UTIL_GetZoneForPosition( vPosEnd );

    if ( StartNodeZone == EndNodeZone )
    {
        pZoneArray[StartNodeZone]   = TRUE;
        nConnectionsInList++;
        return;
    }

    // Otherwise, we have to figure out how many zones this thing spans.
    // Let's try firing a ray from start to end and see if I hit any portals.
    g_CollisionMgr.LineOfSightSetup( NULL, vPosStart, vPosEnd );
    g_CollisionMgr.CheckCollisions( object::TYPE_ZONE_PORTAL, object::ATTR_COLLISION_PERMEABLE );

    // just to be sure
//    ASSERT( g_CollisionMgr.m_nCollisions > 0 );

    // Bad data.  These are going into slot 255
    if ( g_CollisionMgr.m_nCollisions == 0 )
    {
        pZoneArray[255]   = TRUE;
        nConnectionsInList++;
        return;
    }

    // We've got the portals that this guy passes through.  Let's set the zones and stuff.
    for ( s32 i = 0; i < g_CollisionMgr.m_nCollisions; i++ )
    {
        object_ptr<zone_portal> pPortal( g_CollisionMgr.m_Collisions[i].ObjectHitGuid );
        
        ASSERT( pPortal.IsValid() );
        if ( !pPortal.IsValid() )
        {
            return;
        }

        pZoneArray[ pPortal.m_pObject->GetZone1() ] = TRUE;
        pZoneArray[ pPortal.m_pObject->GetZone2() ] = TRUE;
    }

    // Need to add 'number of portals' + 1 to the list.
    nConnectionsInList += ( g_CollisionMgr.m_nCollisions + 1 );
}

//===========================================================================

void ng_connection2::FindZonesSpanned( xarray<u16>* pZoneXArray, s32 nIndex )
{
    // First, figure out what zones that this guy is in.
    vector3 vStartPos = GetStartPosition();
    vector3 vEndPos = GetEndPosition();

    u16 StartNodeZone = SMP_UTIL_GetZoneForPosition( vStartPos );
    u16 EndNodeZone = SMP_UTIL_GetZoneForPosition( vEndPos );

    if ( StartNodeZone == EndNodeZone )
    {
        pZoneXArray[StartNodeZone].Append(nIndex);
        return;
    }

    // Otherwise, we have to figure out how many zones this thing spans.
    // Let's try firing a ray from start to end and see if I hit any portals.
    g_CollisionMgr.LineOfSightSetup( NULL, vStartPos, vEndPos );
    g_CollisionMgr.CheckCollisions( object::TYPE_ZONE_PORTAL, object::ATTR_COLLISION_PERMEABLE );


    if ( g_CollisionMgr.m_nCollisions == 0 )
    {
        pZoneXArray[255].Append(nIndex);
        return;
    }

    // We've got the portals that this guy passes through.  Let's set the zones and stuff.
    for ( s32 i = 0; i < g_CollisionMgr.m_nCollisions; i++ )
    {
        object_ptr<zone_portal> pPortal( g_CollisionMgr.m_Collisions[i].ObjectHitGuid );
        
        ASSERT( pPortal.IsValid() );
        if ( !pPortal.IsValid() )
        {
            return;
        }

        // There are now duplicates in my list...Sort it out later.
        pZoneXArray[ pPortal.m_pObject->GetZone1() ].Append( nIndex );
        pZoneXArray[ pPortal.m_pObject->GetZone2() ].Append( nIndex );
    }
}

#endif // X_EDITOR