
//==============================================================================
//  
//  SecretList.hpp
//  
//==============================================================================

#ifndef SECRET_LIST_HPP
#define SECRET_LIST_HPP

#include "x_files/x_types.hpp"

//=========================================================================
//  Defines
//=========================================================================

#define SECRET_VERSION      1000
#define SECRET_TABLE_SIZE   21         // 21 video diaries and 0 cheats

enum secret_type
{
    SECRET_TYPE_VIDEO,
    SECRET_TYPE_CHEAT,
    SECRET_TYPE_UNKNOWN,
};

//=========================================================================
//  Structs                                                                
//=========================================================================

struct secret_entry
{
    s32             SecretID;           // secret ID
    secret_type     SecretType;         // secret type ID
    char            FileName[32];       // physical filename
    char            ShortDesc[32];      // string ID of secret short description
    char            FullDesc[32] ;      // string ID of secret long description
};

//=========================================================================
//  Globals                                                                
//=========================================================================
class secret_list
{
public:
    void                Init                    ( void );
    void                Kill                    ( void );
    void                Clear                   ( void );
    void                Append                  ( const char* pSecretFile );
    secret_entry*       GetByIndex              ( s32 Index );

private:
    xbool               Append                  ( const secret_entry& Entry );

    secret_entry        m_SecretList[SECRET_TABLE_SIZE];
};

//==============================================================================
//  functions
//==============================================================================
extern secret_list g_SecretList;
//==============================================================================
#endif // SECRET_LIST_HPP
//==============================================================================
