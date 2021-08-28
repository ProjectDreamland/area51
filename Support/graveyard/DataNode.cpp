#include "DataNode.hpp"

data_node::data_node( const char *Name  )
{
    ASSERT(Name);
    m_DataType = DATA_NONE_SET;
    m_Next     = NULL;
    ASSERT(x_strlen(Name));
    x_strcpy(m_pName, Name );

    s32 i;
    u8 checksum;
    s32 length = x_strlen(m_pName);
    checksum = 0;
    for (i=0;i<length;i++)
    {
        checksum ^= m_pName[i];
    }

    m_Checksum = checksum;

}


//=============================================================================
//
//		
//
//
//=============================================================================
data_node::~data_node           ( )
{

}


/*
//=============================================================================
//
//		
//
//
//=============================================================================
xbool data_node::Load           ( X_FILE* inFile )
{

}



//=============================================================================
//
//		
//
//
//=============================================================================
xbool data_node::Save           ( X_FILE* outFile)
{

}
*/


//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetData         ( const char* newData)
{
    ASSERT(newData);
    m_DataType = DATA_TYPE_STRING_PTR;
    char* tempString = new char[x_strlen(newData)+1];
    x_strcpy(tempString,newData);
    m_Data.m_pStringData = tempString;
    

}


//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetData         ( s32 newData)
{
    m_DataType = DATA_TYPE_INT;
    m_Data.m_IntData = newData;
}


//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetData         ( float newData)
{
    m_DataType = DATA_TYPE_FLOAT;
    m_Data.m_FloatData = newData;

}

//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetData         ( s32* newData)
{
    ASSERT(newData);
    m_DataType = DATA_TYPE_INT_PTR;
    m_Data.m_pIntData = newData;
}


//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetData         ( float* newData)
{
    ASSERT(newData);
    m_DataType = DATA_TYPE_FLOAT_PTR;
    m_Data.m_pFloatData = newData;

}


//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetData         ( Data data, DataType dataType)
{
    m_DataType = dataType;
    switch(dataType)
    {
    case DATA_TYPE_FLOAT:
        m_Data.m_FloatData = data.m_FloatData;
        break;
    case DATA_TYPE_INT:
        m_Data.m_IntData = data.m_IntData;
        break;
    case DATA_TYPE_STRING_PTR:
        m_Data.m_pStringData = data.m_pStringData;
        break;
    case DATA_TYPE_FLOAT_PTR:
        m_Data.m_pFloatData = data.m_pFloatData;
        break;
    case DATA_TYPE_INT_PTR:
        m_Data.m_pIntData = data.m_pIntData;
        break;
    default:
        ASSERT(false);

    }

}



//=============================================================================
//
//		
//
//
//=============================================================================
data_node::DataType data_node::GetData     ( Data&   data)
{
    data = m_Data;
    return m_DataType;
}



//=============================================================================
//
//		
//
//
//=============================================================================
guid data_node::GetGuid ( void )
{
    return m_AssociateGuid;
}


//=============================================================================
//
//		
//
//
//=============================================================================
void data_node::SetGuid ( guid    aGuid )
{
    m_AssociateGuid = aGuid;
}




//=============================================================================
//
//		
//
//
//=============================================================================
data_node*  data_node::GetNext( void )
{
    return m_Next;

}



//=============================================================================
//
//		
//
//
//=============================================================================
void        data_node::SetNext( data_node* aNode )
{
    m_Next = aNode;
}


//=============================================================================
//
//		
//
//
//=============================================================================
data_node::DataType  data_node::GetType( void )
{
    return m_DataType;
}





//=============================================================================
//
//		
//
//
//=============================================================================
u8  data_node::GetChecksum( void )
{
    return m_Checksum;
}



//=============================================================================
//
//		
//
//
//=============================================================================
const char*  data_node::GetName  ( void )
{
    return m_pName;

}
