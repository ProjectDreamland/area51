///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_entry
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_entry.hpp"

//=========================================================================

transaction_entry::transaction_entry(const char* pText)
{
    m_xstrDescription = pText;
}

//=========================================================================

transaction_entry::~transaction_entry()
{
    ClearSet();
}

//=========================================================================

xbool transaction_entry::Commit()
{
    DATAPOS pos = m_ptrlstTransactionSet.GetHeadPosition();
    while (pos)
    {
        transaction_data* pData = (transaction_data*)m_ptrlstTransactionSet.GetAt(pos);
        ASSERT(pData);
        if (pData)
        {
            if (!pData->Commit())
            {
                //COMMIT FAILED
                return FALSE;
            }
        }
        m_ptrlstTransactionSet.GetNext(pos);
    }

    return TRUE;
}

//=========================================================================

xbool transaction_entry::Rollback()
{
    DATAPOS pos = m_ptrlstTransactionSet.GetTailPosition();
    while (pos)
    {
        transaction_data* pData = (transaction_data*)m_ptrlstTransactionSet.GetAt(pos);
        ASSERT(pData);
        if (pData)
        {
            if (!pData->Rollback())
            {
                //Rollback FAILED
                return FALSE;
            }
        }
        m_ptrlstTransactionSet.GetPrev(pos);
    }

    return TRUE;
}

//=========================================================================

void transaction_entry::AddCommitStep(transaction_data* pData)
{
    m_ptrlstTransactionSet.AddTail(pData);
}

//=========================================================================

void transaction_entry::ClearSet()
{
    //delete all data
    DATAPOS pos = m_ptrlstTransactionSet.GetHeadPosition();
    while (pos)
    {
        transaction_data* pData = (transaction_data*)m_ptrlstTransactionSet.GetAt(pos);
        if (pData)
        {
            delete pData;
        }
        m_ptrlstTransactionSet.GetNext(pos);
    }

    //clear ptrs
    m_ptrlstTransactionSet.RemoveAll();
}
