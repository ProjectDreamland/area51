///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_data_HPP
#define transaction_data_HPP

#include "stdafx.h"

class transaction_data
{
public:

    enum TRANSACTION_STATE {
        TRANSACTION_OLD_STATE,
        TRANSACTION_NEW_STATE
    };

    transaction_data() {};
    ~transaction_data() {};

    virtual xbool Commit()=0;
    virtual xbool Rollback()=0;
};

#endif//transaction_data_HPP
 
