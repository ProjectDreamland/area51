///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_zone_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_zone_data_HPP
#define transaction_zone_data_HPP

#include "stdafx.h"
#include "transaction_data.hpp"
#include "Objects\Object.hpp"
#include "Auxiliary\MiscUtils\Property.hpp"

class transaction_zone_data : public transaction_data
{
public:

    enum TRANSACTION_ZONE_DATA {
        ZONE_CREATE,
        ZONE_DELETE,
        ZONE_RENAME,
    };

    transaction_zone_data(TRANSACTION_ZONE_DATA TransType,
                            xstring xstrOldZoneName,
                            xstring xstrNewZoneName,
                            xstring xstrLayer);
    ~transaction_zone_data() {};

    virtual xbool Commit();
    virtual xbool Rollback();

protected:
    TRANSACTION_ZONE_DATA   m_TransactionType;
    xstring                 m_ZoneOld;
    xstring                 m_ZoneNew;
    xstring                 m_Layer;
};

#endif//transaction_zone_data_HPP
 
