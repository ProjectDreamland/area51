///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_zone_data
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_zone_data.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "WorldEditor.hpp"

//=========================================================================

transaction_zone_data::transaction_zone_data(TRANSACTION_ZONE_DATA TransType,
                                                xstring xstrOldZoneName,
                                                xstring xstrNewZoneName,
                                                xstring xstrLayer)
{
    m_TransactionType   = TransType;
    m_ZoneOld           = xstrOldZoneName;
    m_ZoneNew           = xstrNewZoneName;
    m_Layer             = xstrLayer;
}

//=========================================================================

xbool transaction_zone_data::Commit()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case ZONE_CREATE:
        bReturn = g_WorldEditor.CreateZone(m_ZoneNew,m_Layer);
        break;
    case ZONE_DELETE:
        bReturn = g_WorldEditor.DeleteZone(m_ZoneOld);
        break;
    case ZONE_RENAME:
        bReturn = g_WorldEditor.RenameZone(m_ZoneOld, m_ZoneNew);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_zone_data::Rollback()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case ZONE_CREATE:
        bReturn = g_WorldEditor.DeleteZone(m_ZoneNew);
        break;
    case ZONE_DELETE:
        bReturn = g_WorldEditor.CreateZone(m_ZoneOld,m_Layer);
        break;
    case ZONE_RENAME:
        bReturn = g_WorldEditor.RenameZone(m_ZoneNew, m_ZoneOld);
        break;
    }

    return bReturn;
}

