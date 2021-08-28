///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_file_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_file_data_HPP
#define transaction_file_data_HPP

#include "stdafx.h"
#include "transaction_data.hpp"

class transaction_file_data : public transaction_data
{
public:

    enum TRANSACTION_FILE_DATA {
        TFILE_CREATE,
        TFILE_DELETE,
        TFILE_RENAME,
        TFILE_EDIT,
    };

    transaction_file_data(TRANSACTION_FILE_DATA TransType, xstring xstrPathOld, xstring xstrPathNew);
    ~transaction_file_data() {};

    virtual xbool Commit();
    virtual xbool Rollback();

    xbool StoreFileData(TRANSACTION_STATE state);
    xbool RestoreFileData(TRANSACTION_STATE state);

    static xbool DoesFileExist(xstring xstrPath);

protected:
    TRANSACTION_FILE_DATA   m_TransactionType;
    xstring                 m_FilePathOld;
    xstring                 m_FilePathNew;
    xstring                 m_TextStreamOld;
    xstring                 m_TextStreamNew;
};

#endif//transaction_file_data_HPP
 
