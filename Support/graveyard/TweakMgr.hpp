//=============================================================================
//
//		TweakMgr.hpp
//
//          - tweak_mgr handles loading and querying for tweak values.
//      
//
//=============================================================================

#ifndef __TWEAK_MGR_HPP__
#define __TWEAK_MGR_HPP__

#include "DataNode.hpp"
#include "Entropy.hpp"

const s32 kHASH_SIZE = 16;
const f32 kINVALID_FLOAT = -999.99f;
const s32 kINVALID_INT   = -99999;
class tweak_mgr
{
public:

    


public:
                                tweak_mgr();
                                ~tweak_mgr();

    void                        LoadTweakFile( X_FILE* aFile );
    void                        LoadTweakFile( const char* FileName );
    void                        ParseText(const char* tempString, s32 sizeRead );
    void                        AddDataNode(data_node* newNode );
    u8                          CalculateChecksum(byte *pBuffer,s32 length );
    void                        ParseTweakFile(char* inString, s32 length );
    data_node*                  GetDataByName( const char *name );
    xbool                       GetFloat( char *pName, float &val, float defaultVal = 0.0f );
    xbool                       GetInt  ( char *pName, s32 &val, s32 defaultVal = 0 );
    xbool                       GetString(char *pName, const char *val, const char *defaultVal = NULL );
                                


protected:
        
    data_node*  m_NodeList[ kHASH_SIZE ];


};

extern tweak_mgr g_TweakMgr;


#endif//__TWEAK_MGR_HPP__