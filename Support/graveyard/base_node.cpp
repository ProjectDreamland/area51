

#include "x_color.hpp"
#include "Entropy.hpp"
#include "base_node.hpp"


//=========================================================================

base_node::base_node(void)
{
    Reset();
}

//=========================================================================

base_node::~base_node()
{



}

//=========================================================================

void base_node::OnRender ( void )
{
    CONTEXT( "base_node::OnRender" );

    draw_ClearL2W();
    draw_Sphere( m_Position, 50.0f , xcolor(100,200,100 ) );
}

//=========================================================================

#ifndef X_RETAIL
void base_node::OnDebugRender ( void )
{
}
#endif // X_RETAIL

//=========================================================================
 
void base_node::AddConnection ( node_slot_id newConnection )
{
    if(m_ConnectionCount >= k_MAX_NODE_CONNECTIONS  )
    {
        x_DebugMsg("---Error:  Currently only %d slots allowed per node\n", k_MAX_NODE_CONNECTIONS  );
        return;
    }
    
    s32 FirstOpenSlot = SLOT_NULL;
    s32 count;
    for( count =0; count < k_MAX_NODE_CONNECTIONS ; count++ )
    {
        // Does this connection already exist.
        if( m_Connections[count] == newConnection )
            return;

        if( (m_Connections[count] == SLOT_NULL) && (FirstOpenSlot == SLOT_NULL) )
        {
            FirstOpenSlot = count;
        }
    }
    
    m_ConnectionCount++;

    m_Connections[FirstOpenSlot] = newConnection;
    return;
}

//=========================================================================

void base_node::DeleteConnection ( node_slot_id thisConnection )
{
    xbool connectionFound = false;
    s32 count;
    for( count =0; count < k_MAX_NODE_CONNECTIONS ; count++ )
    {
        if(connectionFound)
        {
            m_Connections[count -1] = m_Connections[count];
        }
        else if( m_Connections[count] == thisConnection )
        {
            m_Connections[count] = SLOT_NULL;
            m_ConnectionCount--;
            connectionFound = true;
        }

    }
    if(connectionFound)
    {
        m_Connections[k_MAX_NODE_CONNECTIONS-1] = SLOT_NULL;
        return;
    }

    x_DebugMsg("----Error:  Attempted to delete invalid connection from a nav node\n");
}

//=========================================================================

node_slot_id base_node::GetConnectionNearestPoint ( vector3 thisPoint )
{
    (void)thisPoint;
    ASSERT(false);
//    return nav_mgr::GetNavMgr()->LocateNearestNode( thisPoint );
    return SLOT_NULL;
}

//=========================================================================

node_slot_id base_node::GetConnectionByIndex ( s32  thisConnection ) 
{ 

    ASSERT(thisConnection< k_MAX_NODE_CONNECTIONS );

    return m_Connections[thisConnection];

}

//=========================================================================

s32 base_node::GetConnectionCount( void )
{
    return m_ConnectionCount;

}




//=========================================================================

void base_node::Reset( void )
{
    m_IsInPath = false;

    m_ConnectionCount =0;
//    node_slot_id     m_Connections[k_MAX_NODE_CONNECTIONS ];

    int count;
    for(count =0; count < k_MAX_NODE_CONNECTIONS; count++)
    {
        m_Connections[count] =SLOT_NULL;
    }
    
    m_InUse = FALSE;

}


//=========================================================================

void  base_node::Update ( void )
{

}

//=========================================================================

void base_node::OnEnumProp ( prop_enum& List )
{
    List.AddHeader("BaseNode",                    "BaseNode represents a navigation point" );
    List.AddInt("BaseNode\\NodeID",   "Node ID", PROP_TYPE_READ_ONLY );
    List.AddInt("BaseNode\\ConnectionCount",   "Number of connections to this node", PROP_TYPE_READ_ONLY );
    List.AddHeader("BaseNode\\Connections",       "List of connections to this node");
    
    s32 count;
    for(count=0;count< m_ConnectionCount; count++ )
    {
        char tempString[128];
        x_sprintf(tempString,"BaseNode\\Connections\\Connection%2d",count );
        List.AddInt(tempString, "ID for connections that are attached to this node", PROP_TYPE_READ_ONLY );            
    }
}

//=========================================================================

xbool base_node::OnProperty ( prop_query&        I    )
{

    if( I.VarInt("BaseNode\\NodeID", m_SlotID ) )
        return TRUE;
/*    if( I.IsVar(  )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            I.SetVarInt( m_SlotID );
        }
        else
        {
            ASSERT( 0 );
        }
    }

*/

    if( I.VarInt("BaseNode\\ConnectionCount", m_ConnectionCount ) )
        return TRUE;
/*
    if( I.IsVar( "BaseNode\\ConnectionCount" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            I.SetVarInt( m_ConnectionCount );
        }
        else
        {
            ASSERT( 0 );
        }
    }
*/
    s32 count;
    for(count = 0; count < m_ConnectionCount; count++ )
    {
        char tempString[128];
        x_sprintf(tempString,"BaseNode\\Connections\\Connection%2d",count );

        if( I.VarInt(tempString, m_Connections[count] ) )
            return TRUE;
/*
        if( I.IsVar( tempString ) )
        {
            found = TRUE;
            if( I.IsRead() )
            {
                I.SetVarInt( m_Connections[count] );
            }
            else
            {
                ASSERT(0);
            }

        }
*/
    }

    return FALSE;
}

//=========================================================================

void base_node::SetSearchInfo ( node_slot_id nextInSearch, node_slot_id predNode, s32 searchNumber, s32 searchStep, f32 costToNode )
{
    m_NextInSearch      = nextInSearch;
    m_PredNode          = predNode;
    m_SearchNumber      = searchNumber;
    m_SearchStepNumber  = searchStep;
    m_CostToNode        = costToNode;
}

//=========================================================================

void base_node::FileIO( fileio& File )
{

    File.Static( m_Position );
    File.Static( m_ConnectionCount );
//    File.Static( m_Connections );
//    File.Static( m_InUse );
    /*
    ASSERT(outFile);

    x_fwrite(&m_Position,       sizeof(vector3),1,outFile);
    x_fwrite(&m_ConnectionCount,sizeof(s32),1,outFile);
    x_fwrite(&m_Connections,    sizeof(s16),k_MAX_NODE_CONNECTIONS,outFile);
    x_fwrite(&m_InUse,          sizeof(s32),1 ,outFile);
    */
}

void base_node::Save ( X_FILE *outFile )
{
    x_fwrite(&m_Position,       sizeof(vector3),1,outFile);
    x_fwrite(&m_ConnectionCount,sizeof(s32),1,outFile);
    x_fwrite(&m_Connections,    sizeof(s16),k_MAX_NODE_CONNECTIONS,outFile);
    x_fwrite(&m_InUse,          sizeof(s32),1 ,outFile);

}

void base_node::Load ( X_FILE *inFile  )
{
    x_fread(&m_Position,       sizeof(vector3),1,inFile);
    x_fread(&m_ConnectionCount,sizeof(s32),1,inFile);
    x_fread(&m_Connections,    sizeof(s16),k_MAX_NODE_CONNECTIONS,inFile);
    x_fread(&m_InUse,          sizeof(s32),1 ,inFile);

}
