//=============================================================================
//
//  nav_connection2_anchor.cpp
//
//      - version of the nav_node used only in the editor.  Inherits from 
//          object
//
//=============================================================================
#include "stdafx.h"
#include "obj_mgr\obj_mgr.hpp"
#include "nav_connection2_anchor.hpp"
#include "worldeditor.hpp"
#include "transaction_layer_data.hpp"
#include "transaction_object_data.hpp"
#include "transaction_entry.hpp"
#include "Render\Editor\editor_icons.hpp"

#include "EditorAIView.h"
#include "EditorFrame.h"

#define INVALID_GUID        0

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
void NavConn2_Link( void ){}

//========================================================================

static struct nav_connection2_anchor_desc : public object_desc
{
    nav_connection2_anchor_desc( void ) : object_desc( 
        object::TYPE_NAV_CONNECTION2_ANCHOR, 
        "Navigation Connection v2 Anchor", 
        "AI",

            object::ATTR_SPACIAL_ENTRY              |
            object::ATTR_RENDERABLE                 |  
            object::ATTR_COLLISION_PERMEABLE        |
            object::ATTR_EDITOR_TEMP_OBJECT,

            FLAGS_EDITOR_TEMP   |
            FLAGS_NO_ALLOW_COPY   ) {}         
            

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new nav_connection2_anchor; }

    //-------------------------------------------------------------------------

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_NAV_NODE;
    }


} s_nav_connection2_anchor_desc;

//========================================================================

const object_desc&  nav_connection2_anchor::GetTypeDesc     ( void ) const
{
    return s_nav_connection2_anchor_desc;
}

//========================================================================

const object_desc&  nav_connection2_anchor::GetObjectType   ( void )
{
    return s_nav_connection2_anchor_desc;
}

//========================================================================

nav_connection2_anchor::nav_connection2_anchor ( void )
{
    m_Sphere.Set( vector3(0.0f,0.0f,0.0f) , 25.0f );
    Reset();
}

//========================================================================
nav_connection2_anchor::~nav_connection2_anchor ( )
{


}


//========================================================================
bbox   nav_connection2_anchor::GetLocalBBox ( void ) const
{
    return m_Sphere.GetBBox();

}

void    nav_connection2_anchor::OnInit( void)
{
    object::OnInit();
}


//========================================================================
void    nav_connection2_anchor::Reset  ( void )
{
    m_Connection = 0;
    m_Flags      = 0;
    m_FirstUpdate = true;
}

//========================================================================
void    nav_connection2_anchor::OnEnumProp ( prop_enum& List )
{
    List.PropEnumHeader(     "NavConnection Anchor",                    
                                "",
                                PROP_TYPE_DONT_SHOW);

    List.PropEnumGuid(       "NavConnection Anchor\\Connection",
                                "Connection guid",
                                PROP_TYPE_DONT_SHOW );    

    object::OnEnumProp(List);
}


//========================================================================
xbool   nav_connection2_anchor::OnProperty ( prop_query& I )
{
    if( object::OnProperty(I) )
    {
        return true;
    }  

    if(I.IsVar( "NavConnection Anchor\\Connection" ))
    {
        if(I.IsRead() )
        {
            I.SetVarGUID( m_Connection );                
        }
        else
        {
            // Update the internal array
            m_Connection = I.GetVarGUID();               
        }
        return true;    
    }
    
    return false;
}


//========================================================================
void    nav_connection2_anchor::OnMove  ( const vector3& NewPos )
{
    m_Flags |= FLAG_MOVING;

    object::OnMove( NewPos ); 

    object* tempObject;

    //  After we update our own position, we need to force all the connections
    //  to update their own positions

    //  Get the object and verify it is the correct type
    if (m_Connection != 0)
    {    
        tempObject = g_ObjMgr.GetObjectByGuid( m_Connection );
        
        if (tempObject)
        {      
            if( tempObject->IsKindOf( nav_connection2_editor::GetRTTI() ) )
            {
                nav_connection2_editor& tempConnection = nav_connection2_editor::GetSafeType( *tempObject );            
                tempConnection.SetAnchorsDirty();    
                g_WorldEditor.SetObjectsLayerAsDirty( m_Connection );
            }
            else
            {
                x_throw("Logic error!  Guid not of type nav_connection2_editor" );
            }        
        }
    }

    m_Flags &= (~FLAG_MOVING);
}

//========================================================================

xbool nav_connection2_anchor::HasConnection( guid Connection )
{
    //ValidateConnections();

    if( m_Connection == Connection )
        return TRUE;

    return FALSE;
}

//========================================================================

void   nav_connection2_anchor::SetConnection ( guid thisConnection )
{
    m_Connection = thisConnection;
    // SH-TODO: Should this even be used?
    // What if it was already attached? What happens to the original connection?
}


//========================================================================

#ifndef X_RETAIL
void    nav_connection2_anchor::OnDebugRender ( void )
{
    if ( GetAttrBits() & ATTR_EDITOR_PLACEMENT_OBJECT )
    {
        EditorIcon_Draw(EDITOR_ICON_NAV_NODE, GetL2W(), !!(GetAttrBits() & object::ATTR_EDITOR_SELECTED), xcolor(255, 255, 0, 100));
    }
}
#endif // X_RETAIL

//========================================================================

void nav_connection2_anchor::OnRender( void )
{
    CONTEXT( "nav_connection2_anchor::OnRender" );
}

//========================================================================

void    nav_connection2_anchor::OnLoad ( text_in& TextIn )
{
    object::OnLoad(TextIn );    
}

//========================================================================

void nav_connection2_anchor::OnKill( void )
{
    //ValidateConnections();

    m_Flags |= FLAG_DYING;
    /*
    object* tempObject = g_ObjMgr.GetObjectByGuid( m_Connection );
    if(tempObject)
    {
        if( tempObject->IsKindOf(nav_connection2_editor::GetRTTI() ) )
        {
            nav_connection2_editor& tempConn = nav_connection2_editor::GetSafeType( *tempObject );
            
            if ( !tempConn.IsDying() )
                g_WorldEditor.DeleteObject( m_Connection );                

            //  Note, deleting the connection will trigger the 
            //  deletion of the anchor on the other end, as part
            //  of the connection's OnKill function
        }
    }
    */
}


//========================================================================
guid nav_connection2_anchor::GetConnection ( void )
{
    return m_Connection;
}


/*
void  nav_connection2_anchor::ValidateConnections (void)
{   
    s32 count;
    for(count = 0; count < kMAX_CONNECTIONS_PER_NODE; count++ )
    {
        if(m_Connection[count] != 0 )
        {
            object* tempObject = g_ObjMgr.GetObjectByGuid( m_Connection[count] );

            if(tempObject == NULL )
            {
                m_Connection[count] =0;
                x_DebugMsg("Invalid connection in list!");

            }
        }
    }
}   
*/
