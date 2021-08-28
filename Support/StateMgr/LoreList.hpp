
//==============================================================================
//  
//  LoreList.hpp
//  
//==============================================================================

#ifndef LORELIST_HPP
#define LORELIST_HPP

#include "x_files/x_types.hpp"

//=========================================================================
//  Defines
//=========================================================================

#define LORE_VERSION         1000
#define NUM_VAULTS           19
#define NUM_PER_VAULT        5
#define LORE_TABLE_SIZE      NUM_VAULTS * NUM_PER_VAULT

enum lore_type
{
    LORE_TYPE_VIDEO,
    LORE_TYPE_AUDIO,
    LORE_TYPE_STILL,
    LORE_TYPE_TEXT,
    LORE_TYPE_UNKNOWN,
};

//=========================================================================
//  Structs                                                                
//=========================================================================

struct lore_entry
{
    s32             LoreID;             // unique ID
    s32             MapID;              // the map ID that the lore is from 
    s32             NumItems;           // number of items in set
    lore_type       LoreType;           // lore type ID
    char            FileName[32];       // physical filename
    char            Clue[32];           // string ID of lore location clue
    char            ShortDesc[32];      // string ID of lore short description
    char            FullDesc[32] ;      // string ID of lore long description
    char            FullName[32] ;      // string ID of lore name for scanner
};

struct lore_vault
{
    s32             MapID;
    s32             LoreID[NUM_PER_VAULT];
};

//=========================================================================
//  Globals                                                                
//=========================================================================
class lore_list
{
public:
    void                Init                    ( void );
    void                Kill                    ( void );
    void                Clear                   ( void );
    void                Append                  ( const char* pLoreFile );
    const lore_entry*   Find                    ( s32 LoreID );
    const lore_type     GetType                 ( s32 LoreID );
    const char*         GetFileName             ( s32 LoreID );
    lore_entry*         GetByIndex              ( s32 Index );
    const xwchar*       GetLoreName             ( s32 Index );
    lore_vault*         GetVaultByMapID         ( s32 MapID, s32& Index );
    lore_vault*         GetVaultByLoreID        ( s32 LoreID, s32& Index, s32& LoreIndex );
    s32                 GetLoreIDByVault        ( lore_vault *pVault, s32 Index);

private:
    xbool               Append                  ( const lore_entry& Entry );
    xbool               AddToVault              ( const lore_entry& Entry );

    lore_entry          m_LoreList[LORE_TABLE_SIZE];
    lore_vault          m_LoreVault[NUM_VAULTS];   
};

//==============================================================================
//  functions
//==============================================================================
extern lore_list g_LoreList;
//==============================================================================
#endif // LORELIST_HPP
//==============================================================================
