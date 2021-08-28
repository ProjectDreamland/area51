///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  transaction_object_data.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef transaction_object_data_HPP
#define transaction_object_data_HPP

#include "stdafx.h"
#include "transaction_data.hpp"
#include "Objects\Object.hpp"
#include "Auxiliary\MiscUtils\Property.hpp"
#include "worldeditor.hpp"

class transaction_object_data : public transaction_data
{
public:

    enum TRANSACTION_OBJ_DATA {
        OBJ_CREATE,
        OBJ_DELETE,
        OBJ_EDIT,
        OBJ_LAYER_CHANGE
    };

    transaction_object_data(TRANSACTION_OBJ_DATA TransType,
                            guid ObjectGuid,
                            const char* pObjectType);
    ~transaction_object_data() {};

    virtual xbool Commit();
    virtual xbool Rollback();

    xbool StoreProperties(TRANSACTION_STATE state);
    xbool StoreLayer(TRANSACTION_STATE state);

    xbool RestoreProperties(TRANSACTION_STATE state);
    xbool RestoreLayer(TRANSACTION_STATE state);

protected:
    TRANSACTION_OBJ_DATA    m_TransactionType;
    guid                    m_ObjectGuid;
    const char*             m_pObjectType;
    u32                     m_ObjectAttrsOld;
    u32                     m_ObjectAttrsNew;
    xarray<prop_container> 	m_ObjectPropsOld;
    xarray<prop_container> 	m_ObjectPropsNew;
    editor_object_ref       m_ObjectRefOld;
    editor_object_ref       m_ObjectRefNew;
    xstring                 m_LayerOld;
    xstring                 m_LayerNew;
};

#endif//transaction_object_data_HPP
 
