///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_bpref_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_bpref_data_HPP
#define transaction_bpref_data_HPP

#include "stdafx.h"
#include "transaction_data.hpp"
#include "WorldEditor.hpp"

class transaction_bpref_data : public transaction_data
{
public:

    enum TRANSACTION_BPREF_DATA {
        BPREF_CREATE,
        BPREF_DELETE,
        BPREF_EDIT
    };

    transaction_bpref_data( TRANSACTION_BPREF_DATA TransType,
                            guid BPGuid);
    ~transaction_bpref_data() {};

    virtual xbool Commit();
    virtual xbool Rollback();

    xbool StoreReferenceData(TRANSACTION_STATE state);
    xbool RestoreReferenceData(TRANSACTION_STATE state);

protected:
    TRANSACTION_BPREF_DATA  m_TransactionType;
    guid                    m_BPGuid;
    xstring                 m_LayerOld;
    xstring                 m_LayerNew;
    editor_blueprint_ref    m_BPRefOld;
    editor_blueprint_ref    m_BPRefNew;
};

#endif//transaction_bpref_data_HPP
 
