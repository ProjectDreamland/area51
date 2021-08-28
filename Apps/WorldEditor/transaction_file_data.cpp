///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_file_data
//
//  THE REQUIRES WINDOWS FUNCTIONALITY FOR RENAME AND DELETE
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "transaction_file_data.hpp"
#include "..\WinControls\FileSearch.h" //including for DoesFileExist

//=========================================================================

transaction_file_data::transaction_file_data(TRANSACTION_FILE_DATA TransType, xstring xstrPathOld, xstring xstrPathNew)
{
    m_TransactionType   = TransType;
    m_FilePathOld       = xstrPathOld;
    m_FilePathNew       = xstrPathNew;
}

//=========================================================================

xbool transaction_file_data::Commit()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case TFILE_CREATE:
        bReturn = RestoreFileData(TRANSACTION_NEW_STATE);
        break;
    case TFILE_DELETE:
        bReturn = ::DeleteFile(m_FilePathOld);
        break;
    case TFILE_RENAME:
        bReturn = ::MoveFileEx(m_FilePathOld, m_FilePathNew, MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING);
        break;
    case TFILE_EDIT:
        bReturn = RestoreFileData(TRANSACTION_NEW_STATE);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_file_data::Rollback()
{
    xbool bReturn = FALSE;

    switch (m_TransactionType)
    {
    case TFILE_CREATE:
        bReturn = ::DeleteFile(m_FilePathNew);
        break;
    case TFILE_DELETE:
        bReturn = RestoreFileData(TRANSACTION_OLD_STATE);
        break;
    case TFILE_RENAME:
        bReturn = ::MoveFileEx(m_FilePathNew, m_FilePathOld, MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING);
        break;
    case TFILE_EDIT:
        bReturn = RestoreFileData(TRANSACTION_OLD_STATE);
        break;
    }

    return bReturn;
}

//=========================================================================

xbool transaction_file_data::StoreFileData(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        m_TextStreamOld.LoadFile(m_FilePathOld);
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        m_TextStreamNew.LoadFile(m_FilePathNew);
    }

    return TRUE;
}

//=========================================================================

xbool transaction_file_data::RestoreFileData(TRANSACTION_STATE state)
{
    if (state == TRANSACTION_OLD_STATE)
    {
        m_TextStreamOld.SaveFile(m_FilePathOld);
    }
    else if (state == TRANSACTION_NEW_STATE)
    {
        m_TextStreamNew.SaveFile(m_FilePathNew);
    }
    
    return TRUE;
}

//=========================================================================

xbool transaction_file_data::DoesFileExist(xstring xstrPath)
{
    return CFileSearch::DoesFileExist(xstrPath);
}
