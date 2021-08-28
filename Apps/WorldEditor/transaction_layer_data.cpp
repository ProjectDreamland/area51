///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_layer_data
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_layer_data.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "WorldEditor.hpp"

//=========================================================================

transaction_layer_data::transaction_layer_data(TRANSACTION_LYR_DATA TransType,
                                                xstring xstrOldLayerName,
                                                xstring xstrNewLayerName)
{
    m_TransactionType   = TransType;
    m_LayerOld          = xstrOldLayerName;
    m_LayerNew          = xstrNewLayerName;
}

//=========================================================================

xbool transaction_layer_data::Commit()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case LYR_CREATE:
        bReturn = g_WorldEditor.AddLayer(m_LayerNew,TRUE);
        g_WorldEditor.MarkLayerLoaded(m_LayerNew, TRUE);
        break;
    case LYR_DELETE:
        bReturn = g_WorldEditor.RemoveLayer(m_LayerOld);
        break;
    case LYR_RENAME:
        bReturn = g_WorldEditor.RenameLayer(m_LayerOld, m_LayerNew);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_layer_data::Rollback()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case LYR_CREATE:
        bReturn = g_WorldEditor.RemoveLayer(m_LayerNew);
        break;
    case LYR_DELETE:
        bReturn = g_WorldEditor.AddLayer(m_LayerOld,TRUE);
        g_WorldEditor.MarkLayerLoaded(m_LayerOld, TRUE);
        break;
    case LYR_RENAME:
        bReturn = g_WorldEditor.RenameLayer(m_LayerNew, m_LayerOld);
        break;
    }

    return bReturn;
}

