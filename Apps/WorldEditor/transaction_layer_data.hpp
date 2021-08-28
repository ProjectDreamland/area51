///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_layer_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_layer_data_HPP
#define transaction_layer_data_HPP

#include "stdafx.h"
#include "transaction_data.hpp"
#include "Objects\Object.hpp"
#include "Auxiliary\MiscUtils\Property.hpp"

class transaction_layer_data : public transaction_data
{
public:

    enum TRANSACTION_LYR_DATA {
        LYR_CREATE,
        LYR_DELETE,
        LYR_RENAME,
    };

    transaction_layer_data(TRANSACTION_LYR_DATA TransType,
                            xstring xstrOldLayerName,
                            xstring xstrNewLayerName);
    ~transaction_layer_data() {};

    virtual xbool Commit();
    virtual xbool Rollback();

protected:
    TRANSACTION_LYR_DATA    m_TransactionType;
    xstring                 m_LayerOld;
    xstring                 m_LayerNew;
};

#endif//transaction_layer_data_HPP
 
