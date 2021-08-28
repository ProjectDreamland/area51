///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_selection_data
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_selection_data.hpp"
#include "WorldEditor.hpp"

//=========================================================================

transaction_selection_data::transaction_selection_data()
{
	//store selection state
    g_WorldEditor.GetSelectedList( m_Selection );
}

//=========================================================================

xbool transaction_selection_data::Commit()
{
    return RestoreSelection();
}

//=========================================================================

xbool transaction_selection_data::Rollback()
{
    return RestoreSelection();
}

//=========================================================================

xbool transaction_selection_data::RestoreSelection()
{
    g_WorldEditor.ClearSelectedObjectList();
    for (s32 i = 0; i < m_Selection.GetCount(); i++)
    {
        g_WorldEditor.SelectObject( m_Selection.GetAt(i), FALSE );
    }    
    return TRUE;
}
