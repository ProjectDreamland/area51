///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_mgr.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_mgr_HPP
#define transaction_mgr_HPP

#include "stdafx.h"
#include "ptr_list.hpp"
#include "transaction_entry.hpp"

class transaction_mgr
{
public:
    transaction_mgr(void);
    ~transaction_mgr();

    static transaction_mgr*  Transaction(void) { if(!s_This) new transaction_mgr; return s_This; }

    xbool CanUndo();
    xbool CanRedo();

    s32 Undo(s32 nLevels);
    s32 Redo(s32 nLevels);

    void AppendTransaction(transaction_entry* pEntry);
    void ClearStack();

    void SetMaxTransactions(u32 nMax) { m_nMaxTransactions = nMax; }

    void SetFirstTransactionDescription();
    xstring GetNextTransactionDescription(xbool& bIsCurrent);

protected:

    ptr_list    m_ptrlstTransactionStack;
    DATAPOS     m_posCurrent;
    DATAPOS     m_posCurrentTextDescription;

    u32         m_nMaxTransactions;

private:
    static transaction_mgr*  s_This;
};

extern transaction_mgr     g_TransActMgr;

#endif//transaction_mgr_HPP
 
