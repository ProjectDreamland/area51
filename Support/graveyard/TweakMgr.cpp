//=============================================================================
//
//		TweakMgr.cpp
//
//          - tweak_mgr handles loading and querying for tweak values.
//      
//
//=============================================================================


#include "TweakMgr.hpp"
#include "Parsing/tokenizer.hpp"
//#include "LuaLib\LuaMgr.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "resourcemgr\resourcemgr.hpp"


tweak_mgr g_TweakMgr;



//=============================================================================
//
//		
//
//
//=============================================================================
tweak_mgr::tweak_mgr()
{

    int count;
    for(count=0; count< kHASH_SIZE; count++)
    {
        m_NodeList[count] = NULL;
    }


}



//=============================================================================
//
//		
//
//
//=============================================================================
tweak_mgr::~tweak_mgr()
{




}



//=============================================================================
//
//		
//
//
//=============================================================================
void tweak_mgr::LoadTweakFile( const char* FileName )
{
    X_FILE* aFile;

    
    aFile = x_fopen(FileName,"rt");
    if(aFile == NULL)
    {
       char tempName[128];
       x_sprintf ( tempName,"%s\\%s.csv",g_RscMgr.GetRootDirectory(), FileName );
       aFile = x_fopen ( tempName , "rb" );
       if(!aFile)
           return;
       ASSERT(aFile);
    }

    s32 fileLength = x_flength ( aFile );
    char *tempString = new char[fileLength];
    ASSERT(tempString);

    s32 bytesRead = x_fread( tempString, 1, fileLength, aFile );

//    ASSERT(bytesRead == fileLength);

    ParseTweakFile(tempString, bytesRead);

    

}


//=============================================================================
//
//		
//
//
//=============================================================================
void tweak_mgr::AddDataNode(data_node* newNode )
{
    ASSERT(newNode);

    s32 whichList;
    whichList = (s32)newNode->GetChecksum();
    whichList %= kHASH_SIZE;

    newNode->SetNext( m_NodeList[whichList] );
    m_NodeList[whichList] = newNode;


}



//=============================================================================
//
//		
//
//
//=============================================================================
void tweak_mgr::ParseTweakFile(char* inString, s32 length)
{
    ASSERT(inString);

    s32 lineStart=0,
        lineEnd = 0,
        currentPos = 0,
        nextDelimiter = 0;
    char tempString[256];

    while(lineEnd + 2 < length)
    {
        lineStart = currentPos = nextDelimiter = lineEnd;

        //  seek to either the end of the file or the next CR/LF
        while(lineEnd < length && inString[lineEnd] != 0x0A )
        {
            ++lineEnd;
        }

        //  Ok, now we got the start and the end of the current line
        //  time to find the first word
        
        while (  nextDelimiter < lineEnd && inString[nextDelimiter] != ',' )
        {
            nextDelimiter++;
        }
        
        x_strncpy(tempString,&(inString[lineStart]),nextDelimiter-lineStart);
        tempString[nextDelimiter-lineStart] = 0;
        currentPos = nextDelimiter =  nextDelimiter+1;
    
        data_node* DataNode;
        DataNode = this->GetDataByName(tempString);
        if( !DataNode )
        {
            DataNode = new data_node(tempString);
            AddDataNode(DataNode);
        }

        // just one variable per line allowed ATM
//        while ( currentPos < lineEnd )
        {
            while( nextDelimiter< lineEnd && inString[nextDelimiter] != ',')
            {
               nextDelimiter++;
            }
            
            x_strncpy(tempString,&(inString[currentPos]),nextDelimiter- currentPos);
            tempString[nextDelimiter- currentPos] = 0;

            s32 tempInt = x_atoi(tempString);
            if( (tempInt == 0 && !strcmp(tempString,"0")) || tempInt != 0  )
            {
                DataNode->SetData(tempInt);
            }
            else
            {
                float tempFloat = x_atof(tempString);
                if( (tempFloat == 0.0f && !x_strncmp(tempString,"0.0",3)) || tempFloat != 0.0f )
                {
                    DataNode->SetData(tempFloat);
                }
                else
                {
                    DataNode->SetData(tempString);
                }

            }

            currentPos = nextDelimiter+2;

        }


        
        // skip past the CR/LF characters
        lineEnd+=1;
    }





}


//=============================================================================
//
//		
//
//
//=============================================================================
data_node* tweak_mgr::GetDataByName( const char *pName )
{
    
    s32 i;
    u8 checksum;
    s32 length = x_strlen(pName);
    checksum = 0;
    for (i=0;i<length;i++)
    {
        checksum ^= pName[i];
    }

    ;

    data_node* pNode = m_NodeList[checksum % kHASH_SIZE];
    while ( pNode != NULL )
    {
        if(checksum == pNode->GetChecksum() )
        {
            if(!x_stricmp(pName,pNode->GetName() ) )
            {
                return pNode;

            }

        }
        pNode = pNode->GetNext();
    }

    return NULL;

}


//=============================================================================
//
//		
//
//
//=============================================================================
xbool tweak_mgr::GetFloat(char *pName, float &val, float defaultVal )
{
    data_node*  Node;
    data_node::Data aData;
    
    Node = GetDataByName(pName);

    if(Node)
    {
        data_node::DataType aDataType = Node->GetData(aData);
        if(aDataType == data_node::DATA_TYPE_FLOAT )
        {
            val = aData.m_FloatData;
            return true;
            
        }
        if(aDataType == data_node::DATA_TYPE_INT )
        {
            val = (float)aData.m_IntData;
            return true;
            
        }

        else if( aDataType == data_node::DATA_TYPE_FLOAT_PTR )
        {   
            val = *(aData.m_pFloatData);
            return true;

        }

    }
    val = defaultVal;
    return false;

}



//=============================================================================
//
//		
//
//
//=============================================================================
xbool tweak_mgr::GetInt(char *pName, s32 &val, s32 defaultVal )
{
    data_node*  Node;
    data_node::Data aData;
    
    Node = GetDataByName(pName);

    if(Node)
    {
        data_node::DataType aDataType = Node->GetData(aData);
        if(aDataType == data_node::DATA_TYPE_INT )
        {
            val =  aData.m_IntData;
            return true;
            
        }
        else if( aDataType == data_node::DATA_TYPE_INT_PTR )
        {   
            val =  *(aData.m_pIntData);
            return true;

        }

    }
    val = defaultVal;
    return false;

}


//=============================================================================
//
//		
//
//
//=============================================================================
xbool tweak_mgr::GetString(char *pName, const char* val, const char* defaultVal )
{
    data_node*  Node;
    data_node::Data aData;
    
    Node = GetDataByName(pName);

    if(Node)
    {
        data_node::DataType aDataType = Node->GetData(aData);
        if(aDataType == data_node::DATA_TYPE_STRING_PTR )
        {
            val = aData.m_pStringData;
            return true;
        }

    }
    val = defaultVal;
    return false;

}






/*


//    rhandle_base aTweakMgr("test.twk");
//    tweak_mgr* aScript = (tweak_mgr*)(aScriptHandle.GetPointer());

//    g_LuaMgr.ExecuteScript(aScript);
    int count = 1;
    int numberOnStack = lua_gettop(g_LuaMgr.m_LuaState);
    const char* tempString;
    for(;count <= numberOnStack ; count++)
    {
         tempString = lua_tostring(g_LuaMgr.m_LuaState,count);


    }


//    while( lua_type(g_LuaMgr.m_LuaState,count != LUA_TNONE)
//    {
        

//        count++;

//    }
//    tempString = lua_typename(g_LuaMgr.m_LuaState,count++);




    token_stream TokenStream;

    if(!TokenStream.OpenFile( FileName ) )
    {
        ASSERT(false);
        return;
    }
    char Name[32];
    data_node *tempNode = NULL;

//    TokenStream.

    while( ! TokenStream.IsEOF() )
    {
        Name[0] = 0;

        token_stream::type aType = TokenStream.Read();
        switch(aType)
        {
        case token_stream::type::TOKEN_STRING:
            {
                if(tempNode == NULL)
                {
                    tempNode = new data_node( TokenStream.ReadString() );
                }
                else
                {
                    tempNode->SetData( TokenStream.ReadString());
                    AddDataNode( tempNode );
                    tempNode = NULL;

                }
                
            }
            break;

        case token_stream::type::TOKEN_NUMBER:
            {
                ASSERT(tempNode);

                if(! TokenStream.IsFloat() )
                {
                    tempNode->SetData( TokenStream.ReadInt() );

                }
                else
                {
                    float tempFloat[3];
                    tempFloat[0] = TokenStream.ReadFloat();
//                    if( TokenStream.Read() == token_stream::type::TOKEN_NUMBER )
//                    {
//                        tempFloat[1] = TokenStream->ReadFloat();
//                        if( TokenStream.Read() == token_stream::type::TOKEN_NUMBER )
//                        {
//                            tempFloat[2] = TokenStream->ReadFloat();

//                        }
//                    }
                    tempNode->SetData(tempFloat[0]);
                    AddDataNode( tempNode );
                    tempNode = NULL;
                        
                }


            }
            break;
        case token_stream::type::TOKEN_DELIMITER:
            {
                x_DebugMsg( "Delimiter\n" );
            }
            break;

        default:
            {
                x_DebugMsg(" other\n");
//                ASSERT( false );
            }
            break;            

        }

    }

*/