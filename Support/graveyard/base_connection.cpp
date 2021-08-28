///////////////////////////////////////////////////////////////////////////////
//
//  base_connection.cpp
//
//
///////////////////////////////////////////////////////////////////////////////
#include "base_connection.hpp"
#include "NavNodeMgr.hpp"

//=========================================================================

base_connection::base_connection(void)
{
    Reset();
}

//=========================================================================

base_connection::~base_connection()
{
   
}

//=========================================================================

void base_connection::Reset( void )
{
    m_StartNode =   SLOT_NULL;
    m_EndNode   =   SLOT_NULL;   
    m_Length    =   -1.0f;
    m_Width     =   -1.0f;    
//    x_DebugMsg("Reset Nav Connection\n");

}

//=========================================================================

void base_connection::Update( void )
{
   if( GetStartNode() != SLOT_NULL )
   {

   }
}

//========================================================================= 

node_slot_id base_connection::GetOtherEnd( node_slot_id thisEnd )
{
    if(thisEnd == m_StartNode )
        return m_EndNode;

    return m_StartNode;
}

//=========================================================================

void    base_connection::OnEnumProp        ( prop_enum& List )
{
    

    List.AddHeader(     "BaseConnection",  
                        "BaseConnection represents a connection between 2 baseNodes" 
                        );

    List.AddButton(     "BaseConnection\\StartingNode",  
                        "StartingNode is the node from which the connection originates.  baseConnections can be bi-directional but they still have a starting point and an end point", 
                        PROP_TYPE_MUST_ENUM |PROP_TYPE_READ_ONLY  );

    List.AddButton(     "BaseConnection\\EndNode",  
                        "EndNode is the node at which the connection ends.  baseConnections can be bi-directional but they still have a starting point and an end point", 
                        PROP_TYPE_MUST_ENUM | PROP_TYPE_READ_ONLY );

    List.AddButton(     "BaseConnection\\Switch Ends",  
                        "Switches the start and end node", 
                        PROP_TYPE_MUST_ENUM  );
    

    List.AddFloat(      "BaseConnection\\ConnectionWidth",  
                        "ConnectionWidth is the width of the connection path"
                         );

    List.AddFloat(      "BaseConnection\\ConnectionLength", 
                        "ConnectionLength is the calculated length of the connection path based on it's end nodes", 
                        PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    
}

//=========================================================================

xbool   base_connection::OnProperty        ( prop_query&        I    )
{

    xbool found = false;

    if( I.IsVar( "BaseConnection\\StartingNode" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            char tempName[16];
            x_sprintf(tempName,"Node: %d",GetStartNode() );
            I.SetVarButton( tempName );
        }
        else
        {
            ASSERT( 0 );
        }
    }

    else if( I.IsVar( "BaseConnection\\EndNode" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            char tempName[16];
            x_sprintf(tempName,"Node: %d",GetEndNode() );
            I.SetVarButton( tempName );
        }
        else
        {
            ASSERT( 0 );
        }

    }    
    else if( I.IsVar( "BaseConnection\\Switch Ends" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
            I.SetVarButton( "Switch Ends" );
        }
        else
        {
            slot_id tempID;
            tempID = m_StartNode;
            m_StartNode = m_EndNode;
            m_EndNode = tempID;
            
        }

    }
    else if( I.IsVar( "BaseConnection\\ConnectionWidth" ) )
    {
        found = TRUE;
        if( I.IsRead() )
        {
        
            I.SetVarFloat( GetWidth() );
        }
        else
        {
            m_Width = I.GetVarFloat();
        }
    }
    else if( I.IsVar( "BaseConnection\\ConnectionLength"  ) )
    {
    
        found = TRUE;
        if( I.IsRead() )
        {
        
            I.SetVarFloat( GetLength() );
        }
        else
        {
            ASSERT(0);
        }

    }
    return found; 
}

//=========================================================================

void base_connection::SetNodes( node_slot_id startNode, node_slot_id endNode )
{
    m_StartNode = startNode;
    
    m_EndNode   = endNode;
        
    base_node* tempObject1 = g_NavMgr.GetNode(startNode);
    base_node* tempObject2 = g_NavMgr.GetNode(endNode);

    m_Length = (tempObject1->GetPosition() -tempObject2->GetPosition()).Length();
}

//=========================================================================

vector3 base_connection::GetPosition( void )
{
    if( GetStartNode() == SLOT_NULL )
        return vector3( 0.0f,0.0f,0.0f );

    base_node* baseNode1 = g_NavMgr.GetNode( GetStartNode() );
    base_node* baseNode2 = g_NavMgr.GetNode( GetEndNode()   );

    vector3 aPoint = baseNode1->GetPosition() + baseNode2->GetPosition();
    aPoint = aPoint / 2.0f;


    return aPoint;
}

//=========================================================================

f32  base_connection::GetCost ( void )
{
    //
    //  Until we get an AI in place, need to just return the length
    //

    return m_Length;


}

//=========================================================================

void base_connection::FileIO ( fileio& File )
{
    File.Static( m_StartNode );
    File.Static( m_EndNode );
    File.Static( m_Length);
    File.Static( m_Width );
}

//=========================================================================

void base_connection::Save( X_FILE *outFile )
{
//    ASSERT(false);
    ASSERT(outFile);

    x_fwrite(&m_StartNode,      sizeof(s16),1, outFile );
    x_fwrite(&m_EndNode,        sizeof(s16),1, outFile );
    x_fwrite(&m_Length,         sizeof(f32),1, outFile );
    x_fwrite(&m_Width,          sizeof(f32),1, outFile );
}

//=========================================================================

void base_connection::Load( X_FILE *inFile  )
{
//    ASSERT(false);
    ASSERT(inFile);

    x_fread(&m_StartNode,       sizeof(s16),1, inFile );
    x_fread(&m_EndNode,         sizeof(s16),1, inFile );
    x_fread(&m_Length,          sizeof(f32),1, inFile );
    x_fread(&m_Width,           sizeof(f32),1, inFile );
}

//=========================================================================