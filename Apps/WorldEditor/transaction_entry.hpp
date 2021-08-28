///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_entry.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_entry_HPP
#define transaction_entry_HPP

#include "stdafx.h"
#include "ptr_list.hpp"
#include "transaction_data.hpp"

class transaction_entry
{
public:
    transaction_entry(const char* pText);
    ~transaction_entry();

    xbool Commit();
    xbool Rollback();

    xstring GetDescription() { return m_xstrDescription; }

    void AddCommitStep(transaction_data* pData);

    void ClearSet();

protected:

    xstring     m_xstrDescription; 
    ptr_list    m_ptrlstTransactionSet;
};

#endif//transaction_entry_HPP
 
