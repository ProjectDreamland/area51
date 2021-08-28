//============================================================================
//
//  ng_node
//
//      Holds temp data and manages data from the nav_map
//
//
//
//============================================================================
#include "ng_node2.hpp"

ng_node2::ng_node2(void) :
    m_NodeData(NULL)
{

}


ng_node2::~ng_node2()
{


}
    

void ng_node2::Init( nav_map::overlap_data* nodeData, nav_map* owner, nav_node_slot_id iSlot )
{
    ASSERT(nodeData);
    ASSERT(owner);
    m_NodeData  = nodeData;
    m_Owner     = owner;
    m_SlotID    = iSlot;

}

xbool ng_node2::IsPointInside( const vector3& checkPoint )
{
    s32 c;
    f32 minVertY = GetVert(0).m_Pos.Y;
    f32 maxVertY = GetVert(0).m_Pos.Y;
    // quick Y test.
    for(c=1;c<GetVertCount();c++)
    {
        if( GetVert(c).m_Pos.Y < minVertY )
        {
            minVertY = GetVert(c).m_Pos.Y;
        }
        else if ( GetVert(c).m_Pos.Y > maxVertY )
        {
            maxVertY = GetVert(c).m_Pos.Y;
        }

    }

    if( checkPoint.GetY() > maxVertY + 200.0f || checkPoint.GetY() < minVertY - 200.0f )
    {
        return FALSE;
    }

    vector3 vTemp = checkPoint;
    vTemp.GetY()  = 0.0f;
    for(c=1;c<GetVertCount();c++)
    {
        vector3 firstHorizVert  = GetVert(c-1).m_Pos;
        vector3 secondHorizVert = GetVert(c).m_Pos;
        vector3 polyEdge = firstHorizVert - secondHorizVert;
        polyEdge = polyEdge.Cross( vector3(0.0f,1.0f,0.0f) );
        vector3 toEdge = checkPoint.GetClosestVToLSeg( firstHorizVert,secondHorizVert );
        if( polyEdge.Dot(toEdge) <= 0.0f )
            return FALSE;
    }
    return TRUE;
}