///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_bpref_data
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_bpref_data.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "WorldEditor.hpp"

//=========================================================================

transaction_bpref_data::transaction_bpref_data( TRANSACTION_BPREF_DATA TransType,
                                                guid BPGuid)
{
    m_TransactionType   = TransType;
    m_BPGuid            = BPGuid;
}

//=========================================================================

xbool transaction_bpref_data::Commit()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case BPREF_CREATE:
        //create the blueprint based on NewData
        bReturn = g_WorldEditor.AddBlueprintToLayer(m_BPRefNew, m_LayerNew, TRUE);
        break;
    case BPREF_DELETE:
        //delete the blueprint based on OldData
        bReturn = g_WorldEditor.RemoveBlueprintFromLayer(m_BPRefOld.Guid, m_LayerOld, TRUE);
        break;
    case BPREF_EDIT:
        bReturn = RestoreReferenceData(TRANSACTION_NEW_STATE);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_bpref_data::Rollback()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case BPREF_CREATE:
        //delete the blueprint based on NewData
        bReturn = g_WorldEditor.RemoveBlueprintFromLayer(m_BPRefNew.Guid, m_LayerNew, TRUE);
        break;
    case BPREF_DELETE:
        //create the blueprint based on OldData
        bReturn = g_WorldEditor.AddBlueprintToLayer(m_BPRefOld, m_LayerOld, TRUE);
        break;
    case BPREF_EDIT:
        bReturn = RestoreReferenceData(TRANSACTION_OLD_STATE);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_bpref_data::StoreReferenceData(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        VERIFY(g_WorldEditor.GetBlueprintByGuid( m_BPGuid, m_BPRefOld ));
        editor_layer& Layer = g_WorldEditor.FindBlueprintsLayer(m_BPGuid );
        if (!Layer.IsNull)
        {
            m_LayerOld = Layer.Name;
        }
        else return FALSE;
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        VERIFY(g_WorldEditor.GetBlueprintByGuid( m_BPGuid, m_BPRefNew ));
        editor_layer Layer = g_WorldEditor.FindBlueprintsLayer(m_BPGuid );
        if (!Layer.IsNull)
        {
            m_LayerNew = Layer.Name;
        }
        else return FALSE;     
    }
    else 
    { 
        ASSERT(FALSE); 
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool transaction_bpref_data::RestoreReferenceData(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        VERIFY(g_WorldEditor.RemoveBlueprintFromLayer(m_BPRefNew.Guid, m_LayerNew, TRUE));
        VERIFY(g_WorldEditor.AddBlueprintToLayer(m_BPRefOld, m_LayerOld, TRUE));
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        VERIFY(g_WorldEditor.RemoveBlueprintFromLayer(m_BPRefOld.Guid, m_LayerOld, TRUE));
        VERIFY(g_WorldEditor.AddBlueprintToLayer(m_BPRefNew, m_LayerNew, TRUE));
    }
    else 
    { 
        ASSERT(FALSE); 
        return FALSE;
    }

    return TRUE;
}
