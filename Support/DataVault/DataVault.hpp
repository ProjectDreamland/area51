//==============================================================================
//
// DataVault.hpp
//
//==============================================================================
#ifndef DATA_VAULT_HPP
#define DATA_VAULT_HPP
//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"

//#define DATA_VAULT_KEEP_NAMES
#define DATA_VAULT_NAME_LEN         31
#define MAX_DATA_BLOCK_COLLECTIONS  32

class data_vault;
class data_handle;
struct data_block;

//==============================================================================
//  DATA_TYPE
//==============================================================================

enum data_type
{
    DATA_TYPE_NONE                  = 0,
    DATA_TYPE_TWEAK                 = 1,
    DATA_TYPE_PAIN_PROFILE          = 4,
    DATA_TYPE_HEALTH_PROFILE        = 5,
    DATA_TYPE_PAIN_HEALTH_PROFILE   = 6,
    DATA_TYPE_MAX_TYPES,
};

//==============================================================================
//  DATA_ID
//==============================================================================
class data_id
{
public:
                        data_id         ( void );
                       ~data_id         ( void );

    void                Setup           ( const char* pDataDescriptorName, data_type Type );
    void                SetName         ( const char* pDataDescriptorName );
    void                SetType         ( data_type Type );
    data_type           GetType         ( void ) const;
    u32                 GetValue        ( void ) const;
    void                SetValue        ( u32 Value );
    xbool               IsValid         ( void ) const;
    xbool               operator ==     ( const data_id& ID ) const;
    xbool               operator !=     ( const data_id& ID ) const;
    const data_id&      operator =      ( const data_id& ID );

private:

    u32 m_Value;

    #ifdef DATA_VAULT_KEEP_NAMES
    public:
        const char*         GetName         ( void ) const;
        char m_Name[DATA_VAULT_NAME_LEN+1];
    #endif
};

//==============================================================================
//  DATA_HANDLE
//==============================================================================

class data_handle
{
public:
                        data_handle     ( void );
                       ~data_handle     ( void );

    void                Setup           ( const char* pDataDescriptorName, data_type Type );
    void                SetName         ( const char* pDataDescriptorName );
    void                SetType         ( data_type Type );
    data_type           GetType         ( void ) const;

    const data_block*   GetData         ( void ) const;
    xbool               IsValid         ( void ) const;
    
    const data_id&      GetDataID       ( void ) const;
    void                SetDataID       ( const data_id& ID );

    #ifdef DATA_VAULT_KEEP_NAMES
    const char*         GetName         ( void ) const;
    #endif

private:
    data_id             m_DataID;

    friend data_vault;
};

//==============================================================================
// DATA_BLOCK
//==============================================================================

struct data_block
{
                    data_block          ( void );
                   ~data_block          ( void );
    void            Setup               ( const char* pDataDescriptorName, data_type Type );
    void            SetName             ( const char* pDataDescriptorName );
    void            SetType             ( data_type Type );
    data_type       GetType             ( void ) const;
    const data_id&  GetDataID           ( void ) const;
    void            SetDataID           ( const data_id& ID );

    #ifdef DATA_VAULT_KEEP_NAMES
    const char*         GetName         ( void ) const;
    #endif

private:

    data_id         m_DataID;

    friend data_vault;
};

//==============================================================================
// DATA_VAULT
//==============================================================================

class data_vault
{
//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:
                        data_vault      ( void );
                        ~data_vault     ( void );

    void                Init            ( void );
    void                Kill            ( void );

    const data_block*   GetData         ( const data_handle& Handle );

    void                AddDataBlocks   ( const char*       pDataBlockCollectionName,
                                          const data_block* pBase, 
                                          s32               nBlocks, 
                                          s32               SizeOfBlock );

    data_block*         DelDataBlocks   ( const char*       pDataBlockCollectionName );

    void                SanityCheck     ( void );

//------------------------------------------------------------------------------
//  Security Functions
//------------------------------------------------------------------------------
// There are four tables of data blocks inside the data vault:
// iDBC     CollectionName      C++ Structure
//------------------------------------------------------------------------------
// 0        "TWDATA"            tweak_data_block
// 1        "P_PROFILE"         pain_profile
// 2        "H_PROFILE"         health_profile
// 3        "PH_PROFILE"        pain_health_profile
//------------------------------------------------------------------------------

    // If default parameters are used then entire hash table or DBC are checksummed
    u32                 ChecksumHashTable   ( s32 iFirstEntry=-1, s32 iLastEntry=-1 );
    u32                 ChecksumData        ( s32 iDBC, s32 iFirstBlock=-1, s32 iLastBlock=-1 );

    // Copies raw data for nBlocks into the DBC starting at a particular block
    void                WriteData           ( s32 iDBC, s32 iBlock, s32 nBlocks, byte* pData );

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------
private:
    void                Clear                   ( void );
    void                BuildHashTable          ( void );
    void                AddDataBlockToHashTable ( const data_block* pBlock, s32 iDBC, s32 iBlock );
    const data_block*   ResolveBlockPtr         ( s32 iDBC, s32 iBlock );

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------
    struct data_block_collection
    {
        #ifdef DATA_VAULT_KEEP_NAMES
        char            m_Name[DATA_VAULT_NAME_LEN+1];
        #endif
        u32             m_NameHash;
        byte*           m_pBase;
        s32             m_SizeOfBlock;
        s32             m_nBlocks;
        s32             m_TotalSize;
    };

    struct hash_entry
    {
        data_id         m_DataID;
        s16             m_iDBC;
        s16             m_iBlock;
    };

    data_block_collection       m_DataBlockCollection[MAX_DATA_BLOCK_COLLECTIONS];
    hash_entry*                 m_pHash;
    s32                         m_nHashEntries;
    s32                         m_nDataEntries;
};


//==============================================================================
// GLOBALS
//==============================================================================

extern data_vault g_DataVault;

//==============================================================================
// END
//==============================================================================
#endif 

