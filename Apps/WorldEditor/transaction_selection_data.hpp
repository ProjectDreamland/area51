///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_selection_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_selection_data_HPP
#define transaction_selection_data_HPP

#include "stdafx.h"
#include "transaction_data.hpp"

class transaction_selection_data : public transaction_data
{
public:

    transaction_selection_data();
    ~transaction_selection_data() {};

    virtual xbool   Commit();
    virtual xbool   Rollback();

protected:
    xbool           RestoreSelection();
    xarray<guid>    m_Selection;
};

#endif//transaction_selection_data_HPP
 
