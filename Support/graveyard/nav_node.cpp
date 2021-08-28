///////////////////////////////////////////////////////////////////////////////
//
//  nav_node.cpp
//
//
///////////////////////////////////////////////////////////////////////////////
#include "nav_node.hpp"
#include "Entropy.hpp"


//=========================================================================
// VARIABLES
//=========================================================================
const f32 renderRadius = 50.0f;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

nav_node::nav_node(void)  : base_node()
{

}

//=========================================================================

nav_node::~nav_node()
{
}


//=========================================================================

void nav_node::OnRender( void )
{
    CONTEXT( "nav_node::OnRender" );

//    draw_ClearL2W();
//    sphere aSphere;
//    aSphere.Set( m_Position, renderRadius );

//    draw_Sphere( m_Position, aSphere.R, xcolor(80,255,80) );

}

//=========================================================================

#ifndef X_RETAIL
void nav_node::OnDebugRender(void)
{
}
#endif // X_RETAIL

//=========================================================================

void nav_node::Reset( void )
{
    m_IsInPath = false;
    base_node::Reset();
//    x_DebugMsg("Reset Nav Node\n");

}

//=========================================================================

void  nav_node::Update ( void )
{
    base_node::Update();

}

//=========================================================================

void nav_node::SanityCheck( void )
{


    
}

//=========================================================================

void nav_node::OnEnumProp ( prop_enum& List )
{
    
    base_node::OnEnumProp( List );
    
    List.AddHeader("BaseNode\\NavNode",                  "");
    List.AddHeader("BaseNode\\NavNode\\Flags",           "List of flags for the Node");
    List.AddBool("BaseNode\\NavNode\\Flags\\CoverSpot",  "Node is designated as a spot with good cover" );
    List.AddBool("BaseNode\\NavNode\\Flags\\HidingSpot", "Node is designated as a good hiding spot");


}

//=========================================================================

xbool nav_node::OnProperty ( prop_query&        I    )
{

    xbool found = FALSE;

    if( base_node::OnProperty( I ) )
        return TRUE;

    if( I.IsVar( "BaseNode\\NavNode\\Flags\\CoverSpot" ) )
    {
        found = TRUE;
        if(I.IsRead() )
        {
            I.SetVarBool( m_Attributes & FLAG_COVER_SPOT  );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Attributes |= FLAG_COVER_SPOT;
            }
            else
            {
                m_Attributes &= ~FLAG_COVER_SPOT;
            }

        }
    }

    if( I.IsVar( "BaseNode\\NavNode\\Flags\\HidingSpot" ) )
    {
        found = TRUE;
        if(I.IsRead() )
        {
            I.SetVarBool( m_Attributes & FLAG_HIDING_SPOT );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_Attributes |= FLAG_HIDING_SPOT;
            }
            else
            {
                m_Attributes &= ~FLAG_HIDING_SPOT;
            }

        }
    }


    return found;


}

//=========================================================================

void nav_node::SetSearchInfo ( node_slot_id nextInSearch, node_slot_id predNode, s32 searchNumber, s32 searchStep, f32 costToNode )
{

    base_node::SetSearchInfo( nextInSearch, predNode, searchNumber, searchStep, costToNode );
    
}

//=========================================================================

void nav_node::FileIO( fileio& File )
{
    base_node::FileIO( File );
    File.Static( m_Attributes );
}

//=========================================================================

void nav_node::Save ( X_FILE *outFile )
{
    ASSERT(outFile);


    x_fwrite(&m_Attributes,        sizeof(s32),1,outFile);
    base_node::Save( outFile );
 
}

//=========================================================================

void nav_node::Load ( X_FILE *inFile  )
{
    ASSERT(inFile);

    x_fread(&m_Attributes,        sizeof(s32),     1,                       inFile);
    base_node::Load( inFile );
}


s32 nav_node::GetDataSize ( void ) 
{ 
    s32 totalSize = 0;
    totalSize += sizeof(xbool);                                 // Is in use?
    totalSize += sizeof(vector3) ;                              // It's position
    totalSize += k_MAX_NODE_CONNECTIONS*sizeof(node_slot_id) ;  // The connections
    totalSize += sizeof(u32);                                   // m_Attributes
    totalSize += sizeof( node_slot_id );                        // node index

    return totalSize;
}

void nav_node::AddData ( byte* thisBuffer)
{
    s32 index = 0;

    x_memcpy( &thisBuffer[index], &(m_InUse),sizeof(xbool) );
    index += sizeof(xbool);

    x_memcpy( &thisBuffer[index], &(m_Position.X),sizeof(f32) );
    index += sizeof(f32);

    x_memcpy( &thisBuffer[index], &(m_Position.Y),sizeof(f32) );
    index += sizeof(f32);

    x_memcpy( &thisBuffer[index], &(m_Position.Z),sizeof(f32) );
    index += sizeof(f32);


    x_memcpy( &thisBuffer[index], &(m_Connections),sizeof(node_slot_id)*k_MAX_NODE_CONNECTIONS );
    index += sizeof(node_slot_id)*k_MAX_NODE_CONNECTIONS;

    x_memcpy( &thisBuffer[index], &(m_Attributes),sizeof(u32) );
    index += sizeof(u32);


    x_memcpy( &thisBuffer[index], &(m_SlotID),sizeof(node_slot_id) );
    index += sizeof(node_slot_id);
    


}

void nav_node::SetData ( byte* thisBuffer )
{
    s32 index = 0;

    Reset();

    m_InUse = *(xbool*)(&thisBuffer[index]);
    index += sizeof(xbool);

    m_Position.X = *(f32*)(&thisBuffer[index]);
    index += sizeof(f32);

    m_Position.Y = *(f32*)(&thisBuffer[index]);
    index += sizeof(f32);

    m_Position.Z = *(f32*)(&thisBuffer[index]);
    index += sizeof(f32);

    m_ConnectionCount = 0;

    s32 count;
    for(count = 0; count < k_MAX_NODE_CONNECTIONS; count++ )
    {
        m_Connections[count] = *(node_slot_id*)(&thisBuffer[index]);
        index += sizeof(node_slot_id);
        if( m_Connections[count] != SLOT_NULL)
        {
            m_ConnectionCount++;

        }
    }

    m_Attributes = *(u32*)(&thisBuffer[index]);
    index += sizeof(u32);

    m_SlotID = *(node_slot_id*)(&thisBuffer[index]);
    index+= sizeof(node_slot_id);






}
