//=============================================================================
//
//      DataNode.hpp		
//
//          - data_node encapsulates a single piece of data held by tweak_mgr
//
//
//=============================================================================
#ifndef __DATA_NODE_HPP__
#define __DATA_NODE_HPP__

#include "Obj_mgr\obj_mgr.hpp"


class data_node
{
public:

    enum    DataType
    {
        DATA_TYPE_FIRST =0,
        DATA_NONE_SET,
        DATA_TYPE_FLOAT,
        DATA_TYPE_INT,
        DATA_TYPE_STRING_PTR,
        DATA_TYPE_FLOAT_PTR,
        DATA_TYPE_INT_PTR,
        DATA_TYPE_LAST,
        DATA_TYPE_FORCE_32_BIT = 0xFFFFFFFF
    };

    union  Data 
    {
        char*           m_pStringData;
        float           m_FloatData;
        float*          m_pFloatData;
        s32             m_IntData;
        s32*            m_pIntData;
        
    };


public:
                        data_node ( const char *name                          );
                        ~data_node(                                     );

//    xbool               Load      ( X_FILE* inFile                      );
//    xbool               Save      ( X_FILE* outFile                     );

    void                SetData   ( const char*   newData                     );                        
    void                SetData   ( s32     newData                     );
    void                SetData   ( s32*    newData                     );
    void                SetData   ( float   newData                     );
    void                SetData   ( float*  newData                     );
    void                SetData   ( Data    data,   DataType dataType   );

    DataType            GetData   ( Data&   data                        );
    guid                GetGuid   ( void                                );
    void                SetGuid   ( guid    aGuid                       );
    
    const char*         GetName   ( void                                );
    u8                  GetChecksum( void                               );

    data_node*          GetNext   ( void                                );
    void                SetNext   ( data_node* aNode                    );
    DataType            GetType   ( void                                );

protected:

    char                m_pName[32];
    u8                  m_Checksum;
    Data                m_Data;
    DataType            m_DataType;
    guid                m_AssociateGuid;

    data_node*          m_Next;



};




#endif//__DATA_NODE_HPP__