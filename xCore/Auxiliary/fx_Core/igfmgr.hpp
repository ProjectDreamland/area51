#ifndef __IGFMGR_HPP
#define __IGFMGR_HPP

#include "parsing\tokenizer.hpp"

//============================================================================
// IGF Manager
//
// Description:  IGF Manager is a class for loading, manipulating, and saving 
//               "Inevitable Generic Files".
//
//============================================================================


//============================================================================
// TYPEDEFS 
typedef u32                 hfield;                     // handle to a field, to access data from specific locations

//============================================================================
// DEFINES
#define BAD_HFIELD          (NULL)


//============================================================================
// Class definition
class igfmgr
{

public:
    // The supported atomic types
    enum atom
    {
        UNDEFINED,
        BOOL,
        U8,
        S8,
        U16,
        S16,
        U32,
        S32,
        U64,
        S64,
        F32,
        F64,
        STRING,
        COMMENT,
        // higher order types
        COLOR,
        VECTOR2,
        VECTOR3,
        RADIAN3,
        MATRIX4,
        QUATERNION,
        BYTE_ARRAY,                
        // for heirarchical structure
        GROUP,
        // not actually stored, only written to mark the end of a group
        // during save operations
        GROUP_END
    };

protected:
    // Variable-length data dictionary
    struct data_entry
    {
        u8*                     m_pData;                    // pointer to the data
        u32                     m_Length;
        data_entry*             m_pNext;
    };

    // used during save to save pointers as indices
    struct fixup_map
    {
        data_entry*             m_pEntry;
        u32                     m_RefCt;
        u32                     m_Index;
    };

    // .NUC Header
    struct header
    {
        char                    m_DocName[32];              // the name of the document
        char                    m_Creator[32];              // the name of the person who created the document
        f32                     m_Version;                  // the version number
    };                                                      
    
    // the structure of a field                     
    struct field                                            
    {                                                       
        atom                    m_Type;                     // the type of chunk
        data_entry*             m_pName;                    // the name of the chunk
        data_entry*             m_pComment;                 // a comment for the chunk (optional)
        void*                   m_pData;                    // pointer to the data (of type m_Type)
        field*                  m_pNext;                    // pointer to the next field
        field*                  m_pPrev;                    // pointer to the prev field
        xbool                   m_IsDataOwned;              // was the data allocated or is it simply referenced
                                                            // this determines whether or not we delete each field
                                                            // at destruct time  
        field()                 { x_memset(this, 0, sizeof(field)); }
    };                                                      

    // used for remembering where we are in the tree
    struct group_stack_entry
    {
        field*                  m_pGroup;
        field*                  m_pItem;
        field**                 m_ppHead;
    };
    xarray<group_stack_entry>   m_GroupStack;

                                                            
    data_entry*                 m_pDictionary;              // the dictionary for variable size data
    u32                         m_DictionarySize;           // I refuse to abbreviate, for obvious reasons
    fixup_map*                  m_pFixup;                   // used during save to save pointers as indices
    header                      m_Header;                   // the header
    field*                      m_pRoot;                    // the root of the data
    field*                      m_pCurGroup;
    field**                     m_ppHead;                   // pointer to the current head pointer
    field*                      m_pCursor;                  // the current cursor
    u8*                         m_pLoad;                    // when loading binary, points to start of entire data

protected:                                               
    
    data_entry* AddDataEntry    ( const void* pData,        // add a dictionary entry
                                  u32 Length           );   
    
    data_entry* FindDataEntry   ( const void* pData,        // find a data entry
                                  u32 Length           );   

    field*      AddField        ( const char* pName,        // protected function for adding fields
                                  const char* pComment,
                                  hfield InsertAfter = NULL );

    typedef     void            (igfmgr::*iterate_fn)       // iteration callback...write one recursive iteration function
                                ( field* pField,            // use it for whatever reason you need.
                                  u32    Data           );          

    void        IterateFields   ( iterate_fn IterateFn,
                                  field*    pField,
                                  u32       Data        );  // and this is the iteration function

    void        AddRefCt_NoComments
                                ( field*    pField, 
                                  u32       Data        );  // reference count callback, used with above function
    
    void        AddRefCt_WithComments
                                ( field*    pField,
                                  u32       Data        );  // reference count callback, used with above function

    void        DeleteField     ( field*    pField,
                                  u32       Data        );  // callback to destroy a field, used with above function
                                  
    fixup_map*  FindFixupEntry  ( data_entry* pEntry    );  // find a particular dictionary entry in the fixup map

    void        SaveFields_Binary
                                ( field*    pField, 
                                  u32       Data        );  // callback for writing each field

    void        SaveFields_Text_WithComments
                                ( field*    pField, 
                                  u32       Data        );  // callback for writing each field 
    
    void        SaveFields_Text_NoComments
                                ( field*    pField, 
                                  u32       Data        );  // callback for writing each field sans comments

    void        NullifyReferences
                                ( field*    pField,
                                  u32       Data        );  // callback to nullify references to specific data
                                                            // (used during destruction)

    xbool       SaveBinary      ( const char* pFileName,    // save a .NUC file in binary form, with/out comments
                                  xbool       StripComments = FALSE ) ;

    xbool       SaveText        ( const char* pFileName,    // save a .NUC file in text, with/out comments
                                  xbool       StripComments = FALSE ) ;

    xbool       LoadBinary      ( X_FILE* fp );
    field*      FixupGroup      ( field* pGroup         );  // loading and fixing up binary data
    data_entry* FixupDataEntry  ( u32 Index             );  // convert data_entry indices to pointers on load binary

    void        ParseGroup      ( token_stream& Stream  );             // recursive group parsing function for loading text ver.


public:                                                     
                                                           
                igfmgr         ( );                         // default constructor
                igfmgr         ( const char* pFileName );   // instantiate from file
                ~igfmgr        ( );                         // destructor

    void        SetHeaderInfo   ( const char* pDocName,     // fill out the header information
                                  const char* pAuthor,
                                  f32         Version   );

    void        Clear           ( );                        // destroys all contents
    xbool       Load            ( const char* pFileName );  // load a .NUC file
    xbool       Save            ( const char* pFileName,    // save a .NUC file in text or binary form, with/out comments
                                  xbool       IsBinary = FALSE,
                                  xbool       StripComments = FALSE ) ;
    
    //========================================================================
    // data mining (and prospecting)
    hfield      First           ( void );
    hfield      Next            ( void );
    hfield      Last            ( void );
    hfield      Find            ( const char* pName     );  // find a field by name within present scope

    atom        GetType         ( void                  ) const;  // gets type at cursor
    
    hfield      FullRewind      ( void );

    // retrieving actual values (these functions assert the hField is the requested type)
    xbool               GetBool         ( void ) const;
    u8                  GetU8           ( void ) const;
    s8                  GetS8           ( void ) const;
    u16                 GetU16          ( void ) const;
    s16                 GetS16          ( void ) const;
    u32                 GetU32          ( void ) const;
    s32                 GetS32          ( void ) const;
    u64                 GetU64          ( void ) const;
    s64                 GetS64          ( void ) const;
    f32                 GetF32          ( void ) const;
    f64                 GetF64          ( void ) const;
    const char*         GetString       ( void ) const;
    const char*         GetByteArray    ( u32& ByteCount ) const;
    const xcolor&       GetColor        ( void ) const;
    const vector2&      GetV2           ( void ) const;
    const vector3&      GetV3           ( void ) const;
    const radian3&      GetRad          ( void ) const;
    const matrix4&      GetM4           ( void ) const;
    const quaternion&   GetQuat         ( void ) const;

    hfield              GetGroup        ( void ) const;

    // combining finding and retrieving actual values
    xbool               GetBool         ( const char* pName );
    u8                  GetU8           ( const char* pName );
    s8                  GetS8           ( const char* pName );
    u16                 GetU16          ( const char* pName );
    s16                 GetS16          ( const char* pName );
    u32                 GetU32          ( const char* pName );
    s32                 GetS32          ( const char* pName );
    u64                 GetU64          ( const char* pName );
    s64                 GetS64          ( const char* pName );
    f32                 GetF32          ( const char* pName );
    f64                 GetF64          ( const char* pName );
    const char*         GetString       ( const char* pName );
    const char*         GetByteArray    ( const char* pName, u32& ByteCount );
    const xcolor&       GetColor        ( const char* pName );
    const vector2&      GetV2           ( const char* pName );
    const vector3&      GetV3           ( const char* pName );
    const radian3&      GetRad          ( const char* pName );
    const matrix4&      GetM4           ( const char* pName );
    const quaternion&   GetQuat         ( const char* pName );                  
    hfield              GetGroup        ( const char* pName );

    //========================================================================
    // Set the current group
    xbool       SetGroup        ( hfield Group );
    hfield      GetCurGroup     ( void );

    // Other functions for using the group stack (makes it easier on the app side)
    xbool       EnterGroup      ( hfield Group );
    hfield      ExitGroup       ( );


    //========================================================================
    // Creating data from scratch (add data after hField, return the new hField)
    hfield      AddBool         ( const char* pName, xbool Val,      const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddU8           ( const char* pName, u8  Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddS8           ( const char* pName, s8  Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddU16          ( const char* pName, u16 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddS16          ( const char* pName, s16 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddU32          ( const char* pName, u32 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddS32          ( const char* pName, s32 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddU64          ( const char* pName, u64 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddS64          ( const char* pName, s64 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddF32          ( const char* pName, f32 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddF64          ( const char* pName, f64 Val,        const char* pComment = NULL, hfield hField = NULL ); 
    hfield      AddColor        ( const char* pName, xcolor&  Val,   const char* pComment = NULL, hfield hField = NULL );
    hfield      AddV2           ( const char* pName, vector2& Val,   const char* pComment = NULL, hfield hField = NULL );
    hfield      AddV3           ( const char* pName, vector3& Val,   const char* pComment = NULL, hfield hField = NULL );
    hfield      AddR3           ( const char* pName, radian3& Val,   const char* pComment = NULL, hfield hField = NULL );
    hfield      AddM4           ( const char* pName, matrix4& Val,   const char* pComment = NULL, hfield hField = NULL );
    hfield      AddQuat         ( const char* pName, quaternion& Val,const char* pComment = NULL, hfield hField = NULL );                  

    hfield      AddString       ( const char* pName, const char* pStr, const char* pComment = NULL, hfield hField = NULL );
    hfield      AddByteArray    ( const char* pName, const u8* pBytes, u32 ByteCount, const char* pComment = NULL, hfield hField = NULL );
    hfield      AddComment      ( const char* pComment, hfield hField = NULL );
    hfield      AddGroup        ( const char* pName, const char* pComment = NULL, hfield hField = NULL  );


};


#endif

