///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SoundNav_editor.hpp
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "soundnav_editor.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "objects\nav_node_place_holder.hpp"
#include "objects\nav_connection_place_holder.hpp"
#include "Sound\sound_connection.hpp"
#include "navigation\NavNodeMgr.hpp"

//=========================================================================
// GLOBALS
//=========================================================================
soundnav_editor*  soundnav_editor::s_This = NULL;

//=========================================================================

xbool soundnav_editor::IsObjectNavNodePlaceHolder( guid aGuid )
{
    if(aGuid == 0)
        return false;
    if( g_ObjMgr.GetObjectByGuid(aGuid)->GetType() == object::TYPE_NAV_NODE_PLACE_HOLDER )
    {
        return true;
    }
    
    return false;
}

//=========================================================================

xbool soundnav_editor::NavNodePlaceHolderCTRLSelected(guid aGuid)
{
    object *tempObject = g_ObjMgr.GetObjectByGuid(aGuid);
    if ( ( tempObject ) && (tempObject->IsKindOf(nav_node_place_holder::GetRTTI())) )   // SB - Crash FIX!!
    {
        if(m_FirstSelectedObject == SLOT_NULL)
        {
            m_FirstSelectedObject = tempObject->GetSlotID();
        }
        else if( m_FirstSelectedObject == tempObject->GetSlotID() )
        {
            // The clicked the same node again...
            m_FirstSelectedObject = SLOT_NULL;
            return false;
        }
        else 
        {
            // if we have one and they selected

//            node_slot_id anID = nav_mgr::GetNavMgr()->GetNewConnection();
            guid tempGuid = g_ObjMgr.CreateObject( nav_connection_place_holder::GetObjectType() );
            object *tempNNCPH = g_ObjMgr.GetObjectByGuid( tempGuid );
            nav_connection_place_holder &aNNCPH = nav_connection_place_holder::GetSafeType( *tempNNCPH);

            node_slot_id anID = g_NavMgr.GetNewConnection();
            
            aNNCPH.SetConnection( anID );

            g_NavMgr.GetConnection(anID)->SetPlaceHolder(aNNCPH.GetGuid());


            
            object *tempNNPH = g_ObjMgr.GetObjectBySlot(m_FirstSelectedObject );
            nav_node_place_holder &aNavNode1 = nav_node_place_holder::GetSafeType(*tempNNPH);

            object *tempObject2 = g_ObjMgr.GetObjectBySlot(tempObject->GetSlotID() );
            nav_node_place_holder &aNavNode2 = nav_node_place_holder::GetSafeType(*tempObject2);

            node_slot_id connectedNavNode = aNNCPH.GetConnection();

            base_connection* aNC = g_NavMgr.GetConnection(connectedNavNode);
            aNC->SetNodes(aNavNode1.GetNode() , aNavNode2.GetNode() );

            base_node* aNN1 = g_NavMgr.GetNode(aNavNode1.GetNode() );
            base_node* aNN2 = g_NavMgr.GetNode(aNavNode2.GetNode() );

            aNN1->AddConnection(connectedNavNode);
            aNN2->AddConnection(connectedNavNode);
            
            aNNCPH.OnMove(aNC->GetPosition() );
            
            m_FirstSelectedObject = SLOT_NULL;

            return false;
        }
    }

    return true;
}

//=========================================================================

void  soundnav_editor::Render(void)
{

}

//=========================================================================

void soundnav_editor::ResetNavNodePlaceHolder(void)
{
    m_FirstSelectedObject = SLOT_NULL;
}

//=========================================================================

guid  soundnav_editor::CreateSoundNode(void)
{
    guid aGuid = g_ObjMgr.CreateObject( nav_node_place_holder::GetObjectType() );

    node_slot_id anID = g_NavMgr.GetNewNode();
    g_NavMgr.GetNode(anID)->SetPlaceHolder(aGuid );

    object *tempObject = g_ObjMgr.GetObjectByGuid( aGuid );
    nav_node_place_holder &aNNPH = nav_node_place_holder::GetSafeType(*tempObject);

    aNNPH.SetNode(anID);

    return aGuid;
}

//=========================================================================

void soundnav_editor::LoadNavMap( const char* fileName )
{
//    X_FILE* tempFile = x_fopen(fileName, "rb" );

//    fileio File;

    //nav_mgr *

//    File.Load(tempFile, *(nav_mgr::GetNavMgr() ) );
    
    g_NavMgr.LoadMap(fileName);

}

//=========================================================================

void soundnav_editor::SaveNavMap( const char* fileName )
{
    g_NavMgr.SaveMap(fileName);
} 

//=========================================================================

void soundnav_editor::SetNavTestStart( guid thisNode )
{
    g_NavMgr.SetTestStart(thisNode);
}

//=========================================================================

void soundnav_editor::SetNavTestEnd  ( guid thisNode )
{
    g_NavMgr.SetTestEnd(thisNode);
}

//=========================================================================

void soundnav_editor::CalcPath(void)
{
    g_NavMgr.TestPath();
}

//=========================================================================
