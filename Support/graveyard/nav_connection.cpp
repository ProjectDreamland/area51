///////////////////////////////////////////////////////////////////////////////
//
//  nav_connection.cpp
//
//
///////////////////////////////////////////////////////////////////////////////
#include "nav_connection.hpp"
#include "navnodemgr.hpp"
#include "e_draw.hpp"


    
nav_connection::nav_connection(void) : base_connection()
{
    
}


nav_connection::~nav_connection()
{

}

void nav_connection::Reset( void )
{
    base_connection::Reset();
    m_AIHints   =  HINT_NONE;      
}

void nav_connection::Update( void )
{
    base_connection::Update();
}
 

node_slot_id nav_connection::GetOtherEnd( node_slot_id thisEnd )
{
    return base_connection::GetOtherEnd( thisEnd );
}





void    nav_connection::OnEnumProp        ( prop_enum& List )
{
    
    base_connection::OnEnumProp( List );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AI Hints section
//
///////////////////////////////////////////////////////////////////////////////////////////////////

    List.AddHeader(     "BaseConnection\\NavConnection", "" );

    List.AddHeader(     "BaseConnection\\NavConnection\\AI_Hints",  
                        "AI_Hints represent the flags in the object to effect how AI's deal with the connection" 
                        );

    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\Dark",  
                        "This connection is an area that is designated as dark." 
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\Cover", 
                        "This connection is an area that is designated as having significant cover." 
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\Wall",  
                        "This connection has at least one end on a wall"
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\Ceiling",
                        "This connection has at least one end on a celing surface." 
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\Sneaky",
                        "This connection has been designated as sneaky.  This means that it is likely the inobvious path."
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\Jumper",
                        "This connection involves jumping."
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\OrientationChange",
                        "This connection involves an orientation change." 
                        );
    
    List.AddBool(       "BaseConnection\\NavConnection\\AI_Hints\\OneWay",
                        "This connection is a one way connection."
                        );
    List.AddBool(       "BaseConnection\\NavConnection\\Patrol Route",
                        "This connection is part of a patrol route."
                        );




}


xbool   nav_connection::OnProperty        ( prop_query&        I    )
{

    if( base_connection::OnProperty( I ) )
        return TRUE;

    xbool found = false;
    
   if( I.IsVar( "BaseConnection\\NavConnection\\Patrol Route" ) )
    {
        if(I.IsRead() )
        {
            I.SetVarBool( m_AIHints & HINT_PATROL_ROUTE  );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_AIHints |= HINT_PATROL_ROUTE;
            }
            else
            {
                m_AIHints &= ~HINT_PATROL_ROUTE;
            }

        }

        return TRUE;
    }




/*
    s32 count;
    for(count = 0; count < m_ConnectionCount; count++ )
    {
        char tempString[128];
        x_sprintf(tempString,"NavNode\\Connections\\Connection%2d",count );
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

    }

    if( I.IsVar( "NavNode\\Flags\\CoverSpot" ) )
    {
        found = TRUE;
        if(I.IsRead() )
        {
            I.SetVarBool( m_AIHints & FLAG_COVER_SPOT  );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_AIHints |= FLAG_COVER_SPOT;
            }
            else
            {
                m_AIHints &= ~FLAG_COVER_SPOT;
            }

        }
    }

    if( I.IsVar( "NavNode\\Flags\\HidingSpot" ) )
    {
        found = TRUE;
        if(I.IsRead() )
        {
            I.SetVarBool( m_AIHints & FLAG_HIDING_SPOT );
        }
        else
        {
            if( I.GetVarBool() )
            {
                m_AIHints |= FLAG_HIDING_SPOT;
            }
            else
            {
                m_AIHints &= ~FLAG_HIDING_SPOT;
            }

        }
    }
*/

    return found; 
}


void nav_connection::SetNodes( node_slot_id startNode, node_slot_id endNode )
{
    base_connection::SetNodes( startNode, endNode );
}




vector3 nav_connection::GetPosition( void )
{
    return base_connection::GetPosition();
}

f32  nav_connection::GetCost ( void )
{
    return base_connection::GetCost();
}


void nav_connection::FileIO ( fileio& File )
{
    

    File.Static( m_AIHints );

    base_connection::FileIO( File );
}

void nav_connection::Save( X_FILE *outFile )
{
    ASSERT(outFile);

    x_fwrite(&m_AIHints,        sizeof(s32),1, outFile );
    base_connection::Save( outFile );
}



void nav_connection::Load( X_FILE *inFile  )
{
    ASSERT(inFile);

    x_fread(&m_AIHints,         sizeof(s32),1, inFile );

    base_connection::Load( inFile );
}




s32 nav_connection::GetDataSize ( void ) 
{ 
    s32 totalSize = 0;
    totalSize += sizeof(s32);                               // m_AIHints
    totalSize += sizeof( slot_id ) *2;                      // node index
    totalSize += sizeof(f32);                               // m_Width
    

    return totalSize;
}

void nav_connection::AddData ( byte* thisBuffer)
{
    s32 index = 0;

    x_memcpy( &thisBuffer[index], &(m_AIHints),sizeof(s32) );
    index += sizeof(s32);

    x_memcpy( &thisBuffer[index], &(m_StartNode),sizeof(slot_id) );
    index += sizeof(slot_id);

    x_memcpy( &thisBuffer[index], &(m_EndNode),sizeof(slot_id) );
    index += sizeof(slot_id);

    x_memcpy( &thisBuffer[index], &(m_Width),sizeof(f32) );
    index += sizeof(f32);



}


void nav_connection::SetData ( byte* thisBuffer )
{
    s32 index = 0;

    Reset();

    m_AIHints = *(s32*)(&thisBuffer[index]);
    index += sizeof(s32);

    m_StartNode = *(slot_id*)(&thisBuffer[index]);
    index += sizeof(slot_id);

    m_EndNode = *(slot_id*)(&thisBuffer[index]);
    index += sizeof(slot_id);

    m_Width = *(f32*)(&thisBuffer[index]);
    index += sizeof(f32);

}


void nav_connection::OnRender    ( void )
{
    CONTEXT( "nav_connection::OnRender" );

    vector3 point1, point2;

    point1 = g_NavMgr.GetNode(m_StartNode)->GetPosition();
    point2 = g_NavMgr.GetNode(m_EndNode)->GetPosition();

    if( GetHints() & HINT_PATROL_ROUTE )
    {
        static int baseNum = 0;
        baseNum++;
        baseNum &= 0x000000FF;

        draw_Line(point1,point2,xcolor(baseNum,255,(baseNum+128)&0x000000FF ));
        point1 += GetPosition();
        point1 /= 2.0f;
        draw_Label(point1,xcolor(200,200,200),"Patrol Start");
    }
    else
    {
        draw_Line(point1,point2,xcolor(100,100,255) );

    }

}
