///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_object_data
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_object_data.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "WorldEditor.hpp"

//=========================================================================

transaction_object_data::transaction_object_data(TRANSACTION_OBJ_DATA TransType,
                                                 guid           ObjectGuid,
                                                 const char*    pObjectType )
{
    m_TransactionType   = TransType;
    m_ObjectGuid        = ObjectGuid;
    m_pObjectType       = pObjectType;
    m_ObjectRefOld.Guid = ObjectGuid;
    m_ObjectRefOld.LayerPath = "\\";
    m_ObjectRefNew.Guid = ObjectGuid;
    m_ObjectRefNew.LayerPath = "\\";
}

//=========================================================================

xbool transaction_object_data::Commit()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case OBJ_CREATE:
        //create object
        g_ObjMgr.CreateObject( m_pObjectType, m_ObjectGuid);
        RestoreProperties(TRANSACTION_NEW_STATE);
        bReturn = TRUE;
        break;
    case OBJ_DELETE:
        //delete object
        g_ObjMgr.DestroyObjectEx(m_ObjectGuid, TRUE);
        bReturn = TRUE;
        break;
    case OBJ_EDIT:
        bReturn = RestoreProperties(TRANSACTION_NEW_STATE);
        break;
    case OBJ_LAYER_CHANGE:
        bReturn = RestoreLayer(TRANSACTION_NEW_STATE);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_object_data::Rollback()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case OBJ_CREATE:
        //delete object
        g_ObjMgr.DestroyObjectEx(m_ObjectGuid, TRUE);
        bReturn = TRUE;
        break;
    case OBJ_DELETE:
        //re-create object
        g_ObjMgr.CreateObject( m_pObjectType, m_ObjectGuid);
        bReturn = RestoreProperties(TRANSACTION_OLD_STATE);
        break;
    case OBJ_EDIT:
        bReturn = RestoreProperties(TRANSACTION_OLD_STATE);
        break;
    case OBJ_LAYER_CHANGE:
        bReturn = RestoreLayer(TRANSACTION_OLD_STATE);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_object_data::StoreProperties(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        m_ObjectPropsOld.Clear();
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        m_ObjectPropsNew.Clear();
    }
    else { ASSERT(FALSE); }

    object *pObject = g_ObjMgr.GetObjectByGuid(m_ObjectGuid);
    if (pObject)
    {
        if (state == TRANSACTION_OLD_STATE)
        {
            m_ObjectAttrsOld = (pObject->GetAttrBits() & ~object::ATTR_EDITOR_SELECTED);
            pObject->OnCopy(m_ObjectPropsOld);
        }
        else if (state == TRANSACTION_NEW_STATE)
        {
            m_ObjectAttrsNew = (pObject->GetAttrBits() & ~object::ATTR_EDITOR_SELECTED);
            pObject->OnCopy(m_ObjectPropsNew);
        }
        else { ASSERT(FALSE); }
    }
    else
    {
        ASSERT(FALSE);
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool transaction_object_data::RestoreProperties(TRANSACTION_STATE state)
{
    //destroy and recreate object
    g_ObjMgr.DestroyObjectEx(m_ObjectGuid, TRUE);
    g_ObjMgr.CreateObject( m_pObjectType, m_ObjectGuid);
    object *pObject = g_ObjMgr.GetObjectByGuid(m_ObjectGuid);
    if (pObject)
    {
        if (state == TRANSACTION_OLD_STATE)
        {
            pObject->OnPaste(m_ObjectPropsOld);
            pObject->SetAttrBits(m_ObjectAttrsOld);
        }
        else if (state == TRANSACTION_NEW_STATE)
        {
            pObject->OnPaste(m_ObjectPropsNew);
            pObject->SetAttrBits(m_ObjectAttrsNew);
        }
        else { ASSERT(FALSE); }
    }
    else
    {
        ASSERT(FALSE);
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool transaction_object_data::StoreLayer(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        editor_layer& LayerInfo = g_WorldEditor.FindObjectsLayer(m_ObjectGuid, TRUE);
        if (!LayerInfo.IsNull)
        {
            m_LayerOld = LayerInfo.Name;
            m_ObjectRefOld.LayerPath = g_WorldEditor.FindLayerPathForObject(m_ObjectGuid, LayerInfo);
            return TRUE;
        }
        return FALSE;
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        editor_layer& LayerInfo = g_WorldEditor.FindObjectsLayer(m_ObjectGuid, TRUE);
        if (!LayerInfo.IsNull)
        {
            m_LayerNew = LayerInfo.Name;
            m_ObjectRefNew.LayerPath = g_WorldEditor.FindLayerPathForObject(m_ObjectGuid, LayerInfo);
            return TRUE;
        }
        return FALSE;
    }
    else { ASSERT(FALSE); }

    return FALSE;
}

//=========================================================================

xbool transaction_object_data::RestoreLayer(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        if (!m_LayerNew.IsEmpty())
        {
            //remove from old layer if layer exists
            g_WorldEditor.RemoveObjectFromLayer(m_ObjectGuid, m_LayerNew);
        }
        if (!m_LayerOld.IsEmpty())
        {
            g_WorldEditor.AddObjectToLayer(m_ObjectRefOld, m_LayerOld, TRUE);
        }
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        if (!m_LayerOld.IsEmpty())
        {
            //remove from old layer if layer exists
            g_WorldEditor.RemoveObjectFromLayer(m_ObjectGuid, m_LayerOld);
        }
        if (!m_LayerNew.IsEmpty())
        {
            g_WorldEditor.AddObjectToLayer(m_ObjectRefNew, m_LayerNew, TRUE);
        }
    }
    else { ASSERT(FALSE); }

    return TRUE;
}
