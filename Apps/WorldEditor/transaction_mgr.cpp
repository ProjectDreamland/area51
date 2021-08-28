///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_mgr
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_mgr.hpp"

transaction_mgr*  transaction_mgr::s_This = NULL;

//=========================================================================

transaction_mgr::transaction_mgr()
{
    //singleton
    s_This = this;

    m_nMaxTransactions = 40;
    m_posCurrent = NULL;
    m_posCurrentTextDescription = NULL;
}

//=========================================================================

transaction_mgr::~transaction_mgr()
{
    ClearStack();

    //singleton
    s_This = NULL;
}

//=========================================================================

xbool transaction_mgr::CanUndo()
{
    if (m_posCurrent) //at valid position?
    {
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool transaction_mgr::CanRedo()
{
    DATAPOS pos = m_posCurrent;
    if (m_posCurrent) //valid position?
    {
        m_ptrlstTransactionStack.GetNext(pos);
        if (pos)
        {
            //we have something we can redo
            return TRUE;
        }
    }
    else
    {
        //must be at the head
        pos = m_ptrlstTransactionStack.GetHeadPosition();
        if (pos)
        {
            //we have something we can redo
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

s32 transaction_mgr::Undo(s32 nLevels)
{
    s32 nCount = 0;
    while (CanUndo() && (nCount<nLevels))
    {
        //get current entry
        transaction_entry* pEntry = (transaction_entry*)m_ptrlstTransactionStack.GetAt(m_posCurrent);
        ASSERT(pEntry);
        if (pEntry)
        {
            if (pEntry->Rollback())
            {
                nCount++;
                m_ptrlstTransactionStack.GetPrev(m_posCurrent); //move current position back an item
                x_DebugMsg("transaction_mgr::Undo::%s\n", pEntry->GetDescription());

                if (m_posCurrent)
                {
                    transaction_entry* pCurrentPos = (transaction_entry*)m_ptrlstTransactionStack.GetAt(m_posCurrent);
                    x_DebugMsg("transaction_mgr::Position::%s\n", pCurrentPos->GetDescription());
                }
                else
                {
                    x_DebugMsg("transaction_mgr::stack top\n");
                }
            }
            else
            {
                x_DebugMsg("FAILED transaction_mgr::Undo::%s\n", pEntry->GetDescription());
                return nCount;
            }
        }
    }

    return nCount;
}

//=========================================================================

s32 transaction_mgr::Redo(s32 nLevels)
{
    s32 nCount = 0;
    while (CanRedo() && (nCount<nLevels))
    {
        //get next entry
        DATAPOS pos = m_posCurrent;
        if (pos) //valid position?
        {
            m_ptrlstTransactionStack.GetNext(pos);
        }
        else
        {
            //must be at top, get head position
            pos = m_ptrlstTransactionStack.GetHeadPosition();
        }

        if (pos)
        {
            transaction_entry* pEntry = (transaction_entry*)m_ptrlstTransactionStack.GetAt(pos);
            ASSERT(pEntry);
            if (pEntry)
            {
                if (pEntry->Commit())
                {
                    nCount++;
                    m_posCurrent = pos; //move current position up an item
                    x_DebugMsg("transaction_mgr::Redo::%s\n", pEntry->GetDescription());

                    transaction_entry* pCurrentPos = (transaction_entry*)m_ptrlstTransactionStack.GetAt(m_posCurrent);
                    x_DebugMsg("transaction_mgr::Position::%s\n", pCurrentPos->GetDescription());
                }
                else
                {
                    x_DebugMsg("FAILED transaction_mgr::Redo::%s\n", pEntry->GetDescription());
                    return nCount;
                }
            }
        }
        else
        {
            return nCount;
        }
    }

    return nCount;
}

//=========================================================================

void transaction_mgr::AppendTransaction(transaction_entry* pEntry)
{
    //add a transaction following the current pos, if anything exists past this pos, remove it

    //clear everything after this position
    DATAPOS pos = m_posCurrent;

    if (!m_posCurrent)//is current position valid?
    {
        pos = m_ptrlstTransactionStack.GetHeadPosition();
    }

    if (pos) //valid position?
    {
        //only get next if current position is valid
        if (m_posCurrent)
            m_ptrlstTransactionStack.GetNext(pos);

        while (pos)
        {
            DATAPOS posToRemove = pos;
            m_ptrlstTransactionStack.GetNext(pos);

            //delete this positions data
            transaction_entry* pEntry = (transaction_entry*)m_ptrlstTransactionStack.GetAt(posToRemove);
            if (pEntry)
            {
                delete pEntry;
            }
            m_ptrlstTransactionStack.RemoveAt(posToRemove);
        }
    }

    m_posCurrent = m_ptrlstTransactionStack.AddTail(pEntry);

    //now limit the number of undo levels
    if (m_ptrlstTransactionStack.GetCount() > s32(m_nMaxTransactions))
    {
        DATAPOS pos = m_ptrlstTransactionStack.GetHeadPosition();
        if (pos == m_posCurrent)
        {
            m_posCurrent = NULL;
        }
        m_ptrlstTransactionStack.RemoveHead();
    }

    x_DebugMsg("transaction_mgr::Action::%s\n", pEntry->GetDescription());

    transaction_entry* pCurrentPos = (transaction_entry*)m_ptrlstTransactionStack.GetAt(m_posCurrent);
    x_DebugMsg("transaction_mgr::Position::%s\n", pCurrentPos->GetDescription());
}

//=========================================================================

void transaction_mgr::ClearStack()
{
    //clear position ptr
    m_posCurrent = NULL;

    //delete all data
    DATAPOS pos = m_ptrlstTransactionStack.GetHeadPosition();
    while (pos)
    {
        transaction_entry* pEntry = (transaction_entry*)m_ptrlstTransactionStack.GetAt(pos);
        if (pEntry)
        {
            delete pEntry;
        }
        m_ptrlstTransactionStack.GetNext(pos);
    }

    //clear ptrs
    m_ptrlstTransactionStack.RemoveAll();

    x_DebugMsg("transaction_mgr::undo stack cleared\n");
}

//=========================================================================

void transaction_mgr::SetFirstTransactionDescription()
{
    m_posCurrentTextDescription = m_ptrlstTransactionStack.GetHeadPosition();
}

//=========================================================================

xstring transaction_mgr::GetNextTransactionDescription(xbool& bIsCurrent)
{
    if (m_posCurrentTextDescription)
    {
        if (m_posCurrentTextDescription == m_posCurrent)
        {
            bIsCurrent = TRUE;
        }

        transaction_entry* pEntry = (transaction_entry*)m_ptrlstTransactionStack.GetNext(m_posCurrentTextDescription);
        if (pEntry)
        {
            return pEntry->GetDescription();
        }
    }

    return xstring("");
}
