#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include "x_files.hpp"

//#define MAKE_PROP_QUERY_INLINE
//#define MAKE_PROP_CONTAINER_INLINE
//#define MAKE_PROP_ENUM_INLINE

//=========================================================================
// ENUMS
//=========================================================================
//
// --- Types ----
//
// NULL      - This is on when a property doesn't have any data asociated with it
//             such a HEADER for instance.
// FLOAT     - The property is a regular floating point value.
// INT       - The property is a regular integer value.
// BOOL      - The property is a regular xbool value. (1/0)
// VECTOR2   - The property is a regular vector2 value.
// VECTOR3   - The property is a regular vector3 value.
// ROTATION  - The rotation is really a radian3. When display or expouse to the 
//             scripting system this property should be treaded as degrees. 
// ANGLE     - This property is really a radian. See above comment for the rest.
// BBOX      - The property is a regular bbox value.
// COLOR     - This is a regular xcolor.
// STRING    - This is a regular string which the maximun length should be 128.
// ENUM      - This is an enumation type. This contains a list of possible values.
//             This values are display as strings but set as integers. The list is 
//             descrive like this: "first Enum\0Second Enum\0...Last Enum\0"
//             boolean types are the most commonly use case of this type.
// BUTTON    - A button is represented with a button and a string inside the button.
//             To find which string need to use you need to query the interface.
// EXTERNAL  - This property is use to create more complex types of properties.
//             When doing an enumeration the first string is the type, after that it
//             can contain other strings use for that type. Example: "Resource\0geom\0skin\0"
//             Note that the property getter only gets a single string as well as the setter.
// FILENAME  - Basically is a string. But when enumerated in the enum parameter 0 will contain
//             the filter for the file type in win32 form. 
// 
// --- Flags ----
// 
// HEADER    - This flag is use to indicate that the property is to be display as a header.
//             Is there is not type associated with the header then there is nothing to edit.
// READ_ONLY - This flag indicates that the field can't be edited.
//
// MUST_ENUM - This flag tells the system that after writting the property it must re-enumerate
//             the list. Since big changes in the enumeration may had happen.
// DONT_SAVE - If this flag is set this property will not save on disk. There are also a few
//             properties that will also not save such: READ_ONLY, HEADER, BUTTON, EXTERNAL.
// DONT_SHOW - It tells the editor not to display this property.
// DONT_EXPORT - Will not export to console
// EXPOSE      - This is a variable that is exposed to the trigger get/set functionality
//
//=========================================================================
enum prop_type
{
    PROP_TYPE_NULL     ,  
    PROP_TYPE_FLOAT    ,  
    PROP_TYPE_INT      ,  
    PROP_TYPE_BOOL     ,
    PROP_TYPE_VECTOR2  ,
    PROP_TYPE_VECTOR3  ,  
    PROP_TYPE_ROTATION ,  
    PROP_TYPE_ANGLE    ,  
    PROP_TYPE_BBOX     ,  
    PROP_TYPE_GUID     ,  
    PROP_TYPE_COLOR    ,  
    PROP_TYPE_STRING   ,  
    PROP_TYPE_ENUM     ,  
    PROP_TYPE_BUTTON   ,  
    PROP_TYPE_EXTERNAL ,  
    PROP_TYPE_FILENAME ,  
//    PROP_TYPE_TRANSFORM  =  13, 

    PROP_TYPE_BASIC_MASK = 0xff,

    PROP_TYPE_HEADER            = (1<<19), 
    PROP_TYPE_READ_ONLY         = (1<<20), 
    PROP_TYPE_MUST_ENUM         = (1<<21),  
    PROP_TYPE_DONT_SAVE         = (1<<22), 
    PROP_TYPE_DONT_SHOW         = (1<<23), 
    PROP_TYPE_DONT_EXPORT       = (1<<24), 
    PROP_TYPE_EXPOSE            = (1<<25),   
    PROP_TYPE_DONT_SAVE_MEMCARD = (1<<26),  // This flag MUST be set if the property value is
                                            // dynamic and shouldn't be saved
    PROP_TYPE_DONT_COPY         = (1<<27),   
};

//=========================================================================
// PROPERTY INTERFACE
//=========================================================================
class text_in;
class text_out;
class prop_enum;
class prop_container;
class prop_query;

struct prop_interface
{
    virtual            ~prop_interface      ( void ) {}

    virtual void        OnEnumProp          ( prop_enum&                    List       ) = 0;
    virtual xbool       OnProperty          ( prop_query&                   I          ) = 0;

#if defined( X_EDITOR )
    virtual void        OnSave              ( text_out&                     TextOut    );
    virtual void        OnSave              ( const char*                   FileName   );
#endif // defined( X_EDITOR )

    virtual void        OnLoad              ( text_in&                      TextIn     );
    virtual void        OnLoad              ( const char*                   FileName   );
    virtual void        OnCopy              ( xarray<prop_container>&       Container  );
    virtual void        OnPaste             ( const xarray<prop_container>& Container  );
};

//=========================================================================
// prop_enum
//-------------------------------------------------------------------------
// This class is use to enumerate properties.
//=========================================================================
class prop_enum
{
public:

    class node
    {
    public:
                        node            ( void );
        void            Set             ( const char* pName, const char* pComment="", u32 Type = PROP_TYPE_NULL );

        const char*     GetName         ( void ) const;
        u32             GetType         ( void ) const;
        s32             GetEnumCount    ( void ) const;
        const char*     GetEnumType     ( s32 Index ) const;
        const char*     GetComment      ( void ) const;
        void            SetFlags        ( u32 Flags );

    protected:

        char        m_Name[128];
        //char        m_Enum[X_MAX_PATH];
        xstring     m_String;
        u32         m_Type;
        const char* m_pComment;

        friend class prop_enum;
    };
                    prop_enum       ( void );
    virtual        ~prop_enum       ( void ) {};

#ifdef X_EDITOR
#define USE_PROPERTY_HELP_STRINGS
#endif

#ifdef USE_PROPERTY_HELP_STRINGS

#define PropEnumHeader(     a, b, c     )   _PropEnumHeader     ( a, b, c)
#define PropEnumBool(       a, b, c     )   _PropEnumBool       ( a, b, c)
#define PropEnumInt(        a, b, c     )   _PropEnumInt        ( a, b, c)
#define PropEnumFloat(      a, b, c     )   _PropEnumFloat      ( a, b, c)
#define PropEnumVector2(    a, b, c     )   _PropEnumVector2    ( a, b, c)
#define PropEnumVector3(    a, b, c     )   _PropEnumVector3    ( a, b, c)
#define PropEnumRotation(   a, b, c     )   _PropEnumRotation   ( a, b, c)
#define PropEnumAngle(      a, b, c     )   _PropEnumAngle      ( a, b, c)
#define PropEnumBBox(       a, b, c     )   _PropEnumBBox       ( a, b, c)
#define PropEnumGuid(       a, b, c     )   _PropEnumGuid       ( a, b, c)
#define PropEnumColor(      a, b, c     )   _PropEnumColor      ( a, b, c)
#define PropEnumString(     a, b, c     )   _PropEnumString     ( a, b, c)
#define PropEnumEnum(       a, b, c, d  )   _PropEnumEnum       ( a, b, c, d )
#define PropEnumButton(     a, b, c     )   _PropEnumButton     ( a, b, c )
#define PropEnumFileName(   a, b, c, d  )   _PropEnumFileName   ( a, b, c, d )
#define PropEnumExternal(   a, b, c, d  )   _PropEnumExternal   ( a, b, c, d )

    virtual void            _PropEnumHeader       ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumBool         ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumInt          ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumFloat        ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumVector2      ( const char* pName, const char* pComment, u32 Flags );
    virtual void            _PropEnumVector3      ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumRotation     ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumAngle        ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumBBox         ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumGuid         ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumColor        ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumString       ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumEnum         ( const char* pName, const char* pEnum, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumButton       ( const char* pName, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumFileName     ( const char* pName, const char* pExt, const char* pComment, u32 Flags ); 
    virtual void            _PropEnumExternal     ( const char* pName, const char* TypeInfo, const char* pComment, u32 Flags ); 
#else

#define PropEnumHeader(     a, b, c     )   _PropEnumHeader     ( a, c)
#define PropEnumBool(       a, b, c     )   _PropEnumBool       ( a, c)
#define PropEnumInt(        a, b, c     )   _PropEnumInt        ( a, c)
#define PropEnumFloat(      a, b, c     )   _PropEnumFloat      ( a, c)
#define PropEnumVector2(    a, b, c     )   _PropEnumVector2    ( a, c)
#define PropEnumVector3(    a, b, c     )   _PropEnumVector3    ( a, c)
#define PropEnumRotation(   a, b, c     )   _PropEnumRotation   ( a, c)
#define PropEnumAngle(      a, b, c     )   _PropEnumAngle      ( a, c)
#define PropEnumBBox(       a, b, c     )   _PropEnumBBox       ( a, c)
#define PropEnumGuid(       a, b, c     )   _PropEnumGuid       ( a, c)
#define PropEnumColor(      a, b, c     )   _PropEnumColor      ( a, c)
#define PropEnumString(     a, b, c     )   _PropEnumString     ( a, c)
#define PropEnumEnum(       a, b, c, d  )   _PropEnumEnum       ( a, b, d )
#define PropEnumButton(     a, b, c     )   _PropEnumButton     ( a, c )
#define PropEnumFileName(   a, b, c, d  )   _PropEnumFileName   ( a, b, d )
#define PropEnumExternal(   a, b, c, d  )   _PropEnumExternal   ( a, b, d )

    virtual void            _PropEnumHeader       ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumBool         ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumInt          ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumFloat        ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumVector2      ( const char* pName, u32 Flags );
    virtual void            _PropEnumVector3      ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumRotation     ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumAngle        ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumBBox         ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumGuid         ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumColor        ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumString       ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumEnum         ( const char* pName, const char* pEnum, u32 Flags ); 
    virtual void            _PropEnumButton       ( const char* pName, u32 Flags ); 
    virtual void            _PropEnumFileName     ( const char* pName, const char* pExt, u32 Flags ); 
    virtual void            _PropEnumExternal     ( const char* pName, const char* TypeInfo, u32 Flags ); 

#endif

    s32             GetCount        ( void );
    node&           operator[]      ( s32 Index );    
    void            Clear           ( void );

    s32             PushPath        ( const char* pPath );
    void            PopPath         ( s32 iPath );
    void            SetCapacity     ( s32 Capacity ) { m_lList.SetCapacity( Capacity ); }

protected:

    const char*     GetRootPath     ( void );

protected:

    xarray<node> m_lList;
    char         m_RootPath[128];
    s32          m_iRootPath;

};

class prop_enum_counter : public prop_enum
{
public:
                            prop_enum_counter ( void ) { m_Count = 0; }
    virtual                ~prop_enum_counter ( void ) {} 
    s32                     GetCount        ( void ) { return m_Count; }

#ifdef USE_PROPERTY_HELP_STRINGS

    virtual void            _PropEnumHeader       ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumBool         ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumInt          ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumFloat        ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumVector2      ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; }
    virtual void            _PropEnumVector3      ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumRotation     ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumAngle        ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumBBox         ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumGuid         ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumColor        ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumString       ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; }
    virtual void            _PropEnumEnum         ( const char* pName, const char* pEnum, const char* pComment, u32 Flags ) { (void)pName; (void)pEnum; (void)pComment; (void)Flags; m_Count++; } 
    virtual void            _PropEnumButton       ( const char* pName, const char* pComment, u32 Flags ) { (void)pName; (void)pComment; (void)Flags; m_Count++; }
    virtual void            _PropEnumFileName     ( const char* pName, const char* pExt, const char* pComment, u32 Flags ) { (void)pName; (void)pExt; (void)pComment; (void)Flags; m_Count++; }
    virtual void            _PropEnumExternal     ( const char* pName, const char* TypeInfo, const char* pComment, u32 Flags ) { (void)pName; (void)TypeInfo; (void)pComment; (void)Flags; m_Count++; } 

#else

    virtual void            _PropEnumHeader       ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumBool         ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumInt          ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumFloat        ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumVector2      ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; }
    virtual void            _PropEnumVector3      ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumRotation     ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumAngle        ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumBBox         ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumGuid         ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumColor        ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; } 
    virtual void            _PropEnumString       ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; }
    virtual void            _PropEnumEnum         ( const char* pName, const char* pEnum, u32 Flags ) { (void)pName; (void)pEnum; (void)Flags; m_Count++; } 
    virtual void            _PropEnumButton       ( const char* pName, u32 Flags ) { (void)pName; (void)Flags; m_Count++; }
    virtual void            _PropEnumFileName     ( const char* pName, const char* pExt, u32 Flags ) { (void)pName; (void)pExt; (void)Flags; m_Count++; }
    virtual void            _PropEnumExternal     ( const char* pName, const char* TypeInfo, u32 Flags ) { (void)pName; (void)TypeInfo; (void)Flags; m_Count++; } 

#endif

protected:
    s32     m_Count;
};

//=========================================================================
// prop_container
//-------------------------------------------------------------------------
// This class is use to keep a copy of a property. This class is use mainly
// for editor porpuses. 
//=========================================================================
class prop_container
{
public:
                 prop_container   ( void );

    void         InitPropEnum     ( const prop_enum::node& Enum );
    void         InitFloat        ( const char* pName, const f32&     Data );
    void         InitInt          ( const char* pName, const s32&     Data );
    void         InitBool         ( const char* pName, const xbool&   Data );
    void         InitGUID         ( const char* pName, const guid&    Data );
    void         InitVector2      ( const char* pName, const vector2& Data );
    void         InitVector3      ( const char* pName, const vector3& Data );
    void         InitBBox         ( const char* pName, const bbox&    Data );
    void         InitAngle        ( const char* pName, const radian&  Data );
    void         InitRotation     ( const char* pName, const radian3& Data );
    void         InitColor        ( const char* pName, const xcolor&  Data );
    void         InitString       ( const char* pName, const char*    Data );
    void         InitEnum         ( const char* pName, const char*    Data );
    void         InitExternal     ( const char* pName, const char*    Data );
    void         InitFileName     ( const char* pName, const char*    Data );
    void         InitButton       ( const char* pName, const char*    Data );
    void         InitGeneric      ( const char* pName, prop_type Type, const void* pData );

    void         GetFloat         ( f32&     Data ) const;
    void         GetInt           ( s32&     Data ) const;
    void         GetBool          ( xbool&   Data ) const;
    void         GetGUID          ( guid&    Data ) const;
    void         GetVector2       ( vector2& Data ) const;
    void         GetVector3       ( vector3& Data ) const;
    void         GetBBox          ( bbox&    Data ) const;
    void         GetAngle         ( radian&  Data ) const;
    void         GetRotation      ( radian3& Data ) const;
    void         GetColor         ( xcolor&  Data ) const;
    void         GetString        ( char*    Data ) const;
    void         GetEnum          ( char*    Data ) const;
    void         GetExternal      ( char*    Data ) const;
    void         GetGeneric       ( void*    Data ) const;
    void         GetFileName      ( char*    Data ) const;
    void         GetButton        ( char*    Data ) const;

    void         SetFloat         ( const f32&        Data );
    void         SetInt           ( const s32&        Data );
    void         SetBool          ( const xbool&      Data );
    void         SetGUID          ( const guid&       Data );
    void         SetVector2       ( const vector2&    Data );
    void         SetVector3       ( const vector3&    Data );
    void         SetBBox          ( const bbox&       Data );
    void         SetAngle         ( const radian&     Data );
    void         SetRotation      ( const radian3&    Data );
    void         SetColor         ( const xcolor&     Data );
    void         SetString        ( const char*       pString );
    void         SetEnum          ( const char*       pString );
    void         SetExternal      ( const char*       pString );
    void         SetGeneric       ( const void*       Data );
    void         SetFileName      ( const char*       Data );
    void         SetButton        ( const char*       Data );

    const char*  GetName          ( void ) const;
    u32          GetType          ( void ) const;
    u32          GetTypeFlags     ( void ) const;
    s32          GetDataSize      ( void ) const;
    void*        GetRawData       ( void ) const;
    void*        GetRawData       ( void );
    void         SetName          ( const char* pName );

protected:

    char    m_Name[128];
    u32     m_Type;
    char    m_Data[256] PS2_ALIGNMENT( 16 );
};

//=========================================================================
// prop_query
//-------------------------------------------------------------------------
// This class is use to read/write properties for an object class.
//=========================================================================
class prop_query
{
public:
//=========================================================================
// Queries
//=========================================================================

                   prop_query      ( void );
    prop_type      GetQueryType    ( void ) const;

    prop_query&    RQuery          ( prop_container& Container );
    prop_query&    RQuery          ( const char* pName, prop_container& Container );
    prop_query&    RQueryFloat     ( const char* pName,       f32&     Data ); 
    prop_query&    RQueryInt       ( const char* pName,       s32&     Data ); 
    prop_query&    RQueryBool      ( const char* pName,       xbool&   Data ); 
    prop_query&    RQueryGUID      ( const char* pName,       guid&    Data ); 
    prop_query&    RQueryVector2   ( const char* pName,       vector2& Data );
    prop_query&    RQueryVector3   ( const char* pName,       vector3& Data ); 
    prop_query&    RQueryBBox      ( const char* pName,       bbox&    Data ); 
    prop_query&    RQueryAngle     ( const char* pName,       radian&  Data ); 
    prop_query&    RQueryRotation  ( const char* pName,       radian3& Data ); 
    prop_query&    RQueryColor     ( const char* pName,       xcolor&  Data );
    prop_query&    RQueryString    ( const char* pName,       char*    Data );
    prop_query&    RQueryEnum      ( const char* pName,       char*    Data );
    prop_query&    RQueryExternal  ( const char* pName,       char*    Data );
    prop_query&    RQueryFileName  ( const char* pName,       char*    Data );
    prop_query&    RQueryButton    ( const char* pName,       char*    Data );

    prop_query&    WQuery          ( const prop_container& Container );
    prop_query&    WQueryFloat     ( const char* pName, const f32&     Data ); 
    prop_query&    WQueryInt       ( const char* pName, const s32&     Data ); 
    prop_query&    WQueryBool      ( const char* pName, const xbool&   Data ); 
    prop_query&    WQueryGUID      ( const char* pName, const guid&    Data ); 
    prop_query&    WQueryVector2   ( const char* pName, const vector2& Data );
    prop_query&    WQueryVector3   ( const char* pName, const vector3& Data ); 
    prop_query&    WQueryBBox      ( const char* pName, const bbox&    Data ); 
    prop_query&    WQueryAngle     ( const char* pName, const radian&  Data ); 
    prop_query&    WQueryRotation  ( const char* pName, const radian3& Data ); 
    prop_query&    WQueryColor     ( const char* pName, const xcolor&  Data ); 
    prop_query&    WQueryString    ( const char* pName, const char*   pData ); 
    prop_query&    WQueryEnum      ( const char* pName, const char*   pData );
    prop_query&    WQueryExternal  ( const char* pName, const char*   pData );
    prop_query&    WQueryFileName  ( const char* pName, const char*   pData );
    prop_query&    WQueryButton    ( const char* pName, const char*   pData );

//=========================================================================

    template< class T > inline
    void GenericQuery( xbool bRead, prop_type Type, const char* pName, T& Data )
    {
        ParseString( pName );
        m_bRead       = bRead;
        m_Type        = (u32)Type;
        m_pData       = (void*)&Data;
        m_DataSize    = sizeof(T);
        m_RootPath[0] = 0;
        m_RootLength  = 0;
    }

//=========================================================================
// Properties
//=========================================================================

    s32             PushPath    ( const char* pRoot );
    void            PopPath     ( s32 iPath );

    // Generic Getters and setters
    xbool       VarFloat        ( const char* pPropName, f32&     Data, f32 MinVal=-F32_MAX, f32 MaxVal=F32_MAX );
    xbool       VarBool         ( const char* pPropName, xbool&   Data );
    xbool       VarInt          ( const char* pPropName, s32&     Data, s32 MinVal=S32_MIN, s32 MaxVal=S32_MAX );
    xbool       VarGUID         ( const char* pPropName, guid&    Data );
    xbool       VarVector2      ( const char* pPropName, vector2& Data );
    xbool       VarVector3      ( const char* pPropName, vector3& Data );
    xbool       VarBBox         ( const char* pPropName, bbox&    Data );
    xbool       VarAngle        ( const char* pPropName, radian&  Data, radian MinVal=-F32_MAX, radian MaxVal=F32_MAX );
    xbool       VarRotation     ( const char* pPropName, radian3& Data );
    xbool       VarColor        ( const char* pPropName, xcolor&  Data );
    xbool       VarString       ( const char* pPropName, char*    Data, s32 MaxStrLen );
    xbool       VarEnum         ( const char* pPropName, char*    Data );
    xbool       VarExternal     ( const char* pPropName, char*    Data, s32 MaxStrLen );
    xbool       VarFileName     ( const char* pPropName, char*    Data, s32 MaxStrLen );
    xbool       VarButton       ( const char* pPropName, char*    Data );

    // state
    xbool       IsVar           ( const char* pString );

    // Does Path contain pPath?
    xbool       IsSimilarPath   ( const char* pPath );

    // Does Path begin with pPath?
    xbool       IsBasePath      ( const char* pPath );

    xbool       IsEditor        ( void );
    xbool       IsRead          ( void );
    s32         GetIndex        ( s32 Number );
    s32         GetIndexCount   ( void ) {return m_nIndices;}

    const char* GetName         ( void ) { return m_PropName; } 

    // Getters
    f32         GetVarFloat     ( f32 MinVal=-F32_MAX, f32 MaxVal=F32_MAX );
    s32         GetVarInt       ( s32 MinVal=S32_MIN, s32 MaxVal=S32_MAX );
    xbool       GetVarBool      ( void );
    guid        GetVarGUID      ( void );
    vector2     GetVarVector2   ( void );
    vector3     GetVarVector3   ( void );
    bbox        GetVarBBox      ( void );
    radian      GetVarAngle     ( radian MinVal=-F32_MAX, radian MaxVal=F32_MAX );
    radian3     GetVarRotation  ( void );
    xcolor      GetVarColor     ( void );
    const char* GetVarFileName  ( void );
    const char* GetVarString    ( void );
    const char* GetVarEnum      ( void );
    const char* GetVarExternal  ( void );

    // Setters
    void        SetVarFloat     ( f32            Data );
    void        SetVarInt       ( s32            Data );
    void        SetVarBool      ( xbool          Data );
    void        SetVarGUID      ( guid           Data );
    void        SetVarVector2   ( const vector2& Data );
    void        SetVarVector3   ( const vector3& Data );
    void        SetVarBBox      ( const bbox&    Data );
    void        SetVarAngle     ( radian         Data );
    void        SetVarRotation  ( const radian3& Data );
    void        SetVarColor     ( xcolor         Data );
    void        SetVarFileName  ( const char*    Data, s32 MaxStrLen );
    void        SetVarString    ( const char*    Data, s32 MaxStrLen );
    void        SetVarEnum      ( const char*    Data );
    void        SetVarExternal  ( const char*    Data, s32 MaxStrLen );
    void        SetVarButton    ( const char*    Data );

    // Get ranges
    f32         GetMinFloat       ( void ) const { return m_MinFloat; }
    f32         GetMaxFloat       ( void ) const { return m_MaxFloat; }

    s32         GetMinInt         ( void ) const { return m_MinInt; }
    s32         GetMaxInt         ( void ) const { return m_MaxInt; }

    s32         GetMaxStringLength( void ) const { return m_MaxStringLength; }

//=========================================================================
// PROTECTED
//=========================================================================
protected:

    prop_type ClearType( u32        Type ) const;
    prop_type ClearType( prop_type  Type ) const;
    
    void RStringQuery( prop_type Type, const char* pName, char* pData )
    {
        ParseString( pName );
        m_bRead       = TRUE;
        m_Type        = (u32)Type;
        m_DataSize    = 0;
        m_pData       = pData; 
        m_RootPath[0] = 0;
        m_RootLength  = 0;
    }

    void WStringQuery( prop_type Type, const char* pName, const char* pData )
    {
        ParseString( pName );
        m_RootPath[0] = 0;
        m_RootLength  = 0;
        m_bRead       = FALSE;
        m_Type        = (u32)Type;
        m_DataSize    = 0;
        m_pData       = (void*)pData;
        m_RootPath[0] = 0;
        m_RootLength  = 0;
    }

    template< class T > inline
    void GenericVar( prop_type Type, T& Data )
    {
        ASSERT( ClearType(Type) == GetQueryType() ); 
        ASSERT( m_DataSize == sizeof(T) );
        (void)Type;
        if( m_bRead ) *((T*)m_pData) = Data;
        else Data           = *((T*)m_pData);
    }

    template< class T > inline
    void GenericVar( prop_type Type, T& Data, T Min, T Max )
    {
        GenericVar( Type, Data );
        Data = MAX( Data, Min );
        Data = MIN( Data, Max );
    }

    template< class T > inline
    void GetGenericVar( prop_type Type, T& Data ) const
    {
        ASSERT( ClearType(Type) == GetQueryType() ); 
        ASSERT( m_DataSize == sizeof(T) );
        ASSERT( m_bRead == FALSE );
        (void)Type;
        Data           = *((T*)m_pData);
    }

    template< class T > inline
    void GetGenericVar( prop_type Type, T& Data, T Min, T Max ) const
    {
        GetGenericVar( Type, Data );
        Data = MAX( Data, Min );
        Data = MIN( Data, Max );
    }

    template< class T > inline
    void SetGenericVar( prop_type Type, const T& Data )
    {
        ASSERT( ClearType(Type) == GetQueryType() ); 
        ASSERT( m_DataSize == sizeof(T) );
        ASSERT( m_bRead ); 
        (void)Type;
        *((T*)m_pData) = Data;
    }

    inline void GenericVarString( char* pData, s32 MaxStrLen )
    { 
        ASSERT( m_Type & PROP_TYPE_STRING );
        
        if( m_bRead )
        {
            x_strsavecpy( ((char*)m_pData), pData, MaxStrLen );
        }
        else                       
        {
            x_strsavecpy( pData, ((char*)m_pData), MaxStrLen );
        }

        // Set an indicater on how long a string can be
        m_MaxStringLength = MaxStrLen;
    }

    void ParseString( const char* pString );

    xbool   m_bRead;
    s32     m_PropNameLen;
    char    m_PropName[128];
    char    m_RootPath[128];
    s32     m_RootLength;

    s32     m_nIndices;
    s32     m_Index[16];
    u32     m_Type;
    s32     m_DataSize;
    void*   m_pData;

    s32     m_MaxStringLength;
    s32     m_MaxInt;
    s32     m_MinInt;
    f32     m_MinFloat;
    f32     m_MaxFloat;
};

//=========================================================================
// GLOBAL VARIABLES
//=========================================================================

extern prop_query g_PropQuery;
extern prop_enum  g_PropEnum;

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

//=========================================================================
//=========================================================================
//=========================================================================
#ifdef MAKE_PROP_ENUM_INLINE
//=========================================================================
//=========================================================================
//=========================================================================

inline const char* prop_enum::node::GetName       ( void ) const      { return m_Name; }
inline u32         prop_enum::node::GetType       ( void ) const      { return m_Type; }
inline const char* prop_enum::node::GetComment    ( void ) const      { return m_pComment; }
inline void        prop_enum::node::SetFlags      ( u32 Flags )       { m_Type |= Flags; }

inline prop_enum::node::node( void )
{
    //m_Enum[0] = 0;
    m_Name[0] = 0;
}

inline s32 prop_enum::PushPath( const char* pPath )
{
    ASSERT( pPath );
    s32 i;
    s32 Old = m_iRootPath;

    for( i=0; (m_RootPath[m_iRootPath+i] = pPath[i]); i++ )
    {
        ASSERT( i<256);
    }    

    m_iRootPath += i;
    return Old;
}

inline void prop_enum::PopPath( s32 iPath )
{
    m_iRootPath = iPath;
    m_RootPath[m_iRootPath]=0;
}


inline const char* prop_enum::GetRootPath( void )
{
    return m_RootPath;
}

inline prop_enum::prop_enum       ( void )
{
    m_RootPath[0]=0;
    m_iRootPath=0;
}

// ******
// RELEASE BUILD COMPILER ERROR
// 11/6/02 - Removed conditional statement in the following lines as they were
// causing the compiler to barf.
//

#ifdef USE_PROPERTY_HELP_STRINGS

inline void prop_enum::_PropEnumBool    ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_BOOL    ); }
inline void prop_enum::_PropEnumInt     ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_INT     ); }
inline void prop_enum::_PropEnumFloat   ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_FLOAT   ); }
inline void prop_enum::_PropEnumVector2 ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_VECTOR2 ); }
inline void prop_enum::_PropEnumVector3 ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_VECTOR3 ); }
inline void prop_enum::_PropEnumRotation( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_ROTATION); }
inline void prop_enum::_PropEnumAngle   ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_ANGLE   ); }
inline void prop_enum::_PropEnumBBox    ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_BBOX    ); }
inline void prop_enum::_PropEnumGuid    ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_GUID    ); }
inline void prop_enum::_PropEnumColor   ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_COLOR   ); }
inline void prop_enum::_PropEnumString  ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_STRING  ); }
inline void prop_enum::_PropEnumButton  ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_BUTTON  ); }
inline void prop_enum::_PropEnumHeader  ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_HEADER  ); }

#else

inline void prop_enum::_PropEnumBool    ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_BOOL    ); }
inline void prop_enum::_PropEnumInt     ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_INT     ); }
inline void prop_enum::_PropEnumFloat   ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_FLOAT   ); }
inline void prop_enum::_PropEnumVector2 ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_VECTOR2 ); }
inline void prop_enum::_PropEnumVector3 ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_VECTOR3 ); }
inline void prop_enum::_PropEnumRotation( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_ROTATION); }
inline void prop_enum::_PropEnumAngle   ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_ANGLE   ); }
inline void prop_enum::_PropEnumBBox    ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_BBOX    ); }
inline void prop_enum::_PropEnumGuid    ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_GUID    ); }
inline void prop_enum::_PropEnumColor   ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_COLOR   ); }
inline void prop_enum::_PropEnumString  ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_STRING  ); }
inline void prop_enum::_PropEnumButton  ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_BUTTON  ); }
inline void prop_enum::_PropEnumHeader  ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_HEADER  ); }

#endif

inline s32              prop_enum::GetCount  ( void )       { return m_lList.GetCount(); }
inline void             prop_enum::Clear     ( void )       {        m_lList.Clear(); }
inline prop_enum::node& prop_enum::operator[]( s32 Index )  { return m_lList[Index]; }


inline s32 prop_enum::node::GetEnumCount( void ) const
{
    s32 nEnums = 0;
    for( s32 i=0; m_String[i]; i++ )
    {
        for( ; m_String[i]; i++ );
        nEnums++;
    }
    return nEnums;
}

inline const char* prop_enum::node::GetEnumType( s32 Index ) const
{
    s32 nEnums = 0;
    for( s32 i=0; m_String[i]; i++ )
    {
        if( nEnums == Index ) return &m_String[i];
        for( ; m_String[i]; i++ );
        nEnums++;
    }
    return NULL;
}

#ifdef USE_PROPERTY_HELP_STRINGS
inline void prop_enum::_PropEnumEnum    ( const char* pName, const char* pEnum, const char* pComment, u32 Flags )
#else
inline void prop_enum::_PropEnumEnum    ( const char* pName, const char* pEnum, u32 Flags )
#endif
{
    ASSERT( pEnum );

    node&   Node = m_lList.Append();

#ifdef USE_PROPERTY_HELP_STRINGS
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_ENUM );
#else
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_ENUM );
#endif

    s32 Len = 0;
    while( (pEnum[Len] != 0) || (pEnum[Len+1] != 0) )
        Len++;
    Node.m_String.SetLength( Len+1 );
    x_memcpy( &Node.m_String[0], pEnum, Len+2 );
}

#ifdef USE_PROPERTY_HELP_STRINGS
inline void prop_enum::_PropEnumFileName( const char* pName, const char* pExt, const char* pComment, u32 Flags )
#else
inline void prop_enum::_PropEnumFileName( const char* pName, const char* pExt, u32 Flags )
#endif
{
    node& Node = m_lList.Append();

#ifdef USE_PROPERTY_HELP_STRINGS
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_FILENAME );
#else
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_FILENAME );
#endif

    ASSERT( pExt );
    ASSERT( x_strlen(pExt) < X_MAX_PATH );
    Node.m_String = pExt;
}

#ifdef USE_PROPERTY_HELP_STRINGS
inline void prop_enum::_PropEnumExternal( const char* pName, const char* TypeInfo, const char* pComment, u32 Flags )
#else
inline void prop_enum::_PropEnumExternal( const char* pName, const char* TypeInfo, u32 Flags )
#endif
{
    ASSERT( TypeInfo );

    node&   Node = m_lList.Append();

#ifdef USE_PROPERTY_HELP_STRINGS
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_EXTERNAL );
#else
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_EXTERNAL );
#endif

    s32 Len = 0;
    while( (TypeInfo[Len] != 0) || (TypeInfo[Len+1] != 0) )
        Len++;
    Node.m_String.SetLength( Len+1 );
    x_memcpy( &Node.m_String[0], TypeInfo, Len+2 );
}

//=========================================================================

inline 
void prop_enum::node::Set( const char* pString, const char* pComment, u32 aFlags )
{
    ASSERT( pString );
    ASSERT( x_strlen( pString ) < 128 );
    x_strcpy( m_Name, pString );
    m_Type     = aFlags;
    m_pComment = pComment;
}

//=========================================================================
//=========================================================================
//=========================================================================
#endif// MAKE_PROP_ENUM_INLINE
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================
//=========================================================================
//=========================================================================
#ifdef MAKE_PROP_CONTAINER_INLINE
//=========================================================================
//=========================================================================
//=========================================================================

inline prop_container::prop_container   ( void ) { m_Name[0]=0; m_Type=0; m_Data[0]=0x0; }

inline void prop_container::InitPropEnum( const prop_enum::node& EnumNode )
{
    x_strcpy( m_Name, EnumNode.GetName() );
    m_Type   = EnumNode.GetType();
};

inline void prop_container::InitFloat   ( const char* pName, const f32&      Data ) { InitGeneric( pName, PROP_TYPE_FLOAT,    (const void*)&Data ); }
inline void prop_container::InitBool    ( const char* pName, const xbool&    Data ) { InitGeneric( pName, PROP_TYPE_BOOL,     (const void*)&Data ); }
inline void prop_container::InitInt     ( const char* pName, const s32&      Data ) { InitGeneric( pName, PROP_TYPE_INT,      (const void*)&Data ); }
inline void prop_container::InitGUID    ( const char* pName, const guid&     Data ) { InitGeneric( pName, PROP_TYPE_GUID,     (const void*)&Data ); }
inline void prop_container::InitVector2 ( const char* pName, const vector2&  Data ) { InitGeneric( pName, PROP_TYPE_VECTOR2,  (const void*)&Data ); }
inline void prop_container::InitVector3 ( const char* pName, const vector3&  Data ) { InitGeneric( pName, PROP_TYPE_VECTOR3,  (const void*)&Data ); }
inline void prop_container::InitBBox    ( const char* pName, const bbox&     Data ) { InitGeneric( pName, PROP_TYPE_BBOX,     (const void*)&Data ); }
inline void prop_container::InitAngle   ( const char* pName, const radian&   Data ) { InitGeneric( pName, PROP_TYPE_ANGLE,    (const void*)&Data ); }
inline void prop_container::InitRotation( const char* pName, const radian3&  Data ) { InitGeneric( pName, PROP_TYPE_ROTATION, (const void*)&Data ); }
inline void prop_container::InitColor   ( const char* pName, const xcolor&   Data ) { InitGeneric( pName, PROP_TYPE_COLOR,    (const void*)&Data ); }
inline void prop_container::InitString  ( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_STRING,   (const void*)Data );  }
inline void prop_container::InitEnum    ( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_ENUM,     (const void*)Data );  }
inline void prop_container::InitExternal( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_EXTERNAL, (const void*)Data );  }

inline void prop_container::InitFileName( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_FILENAME, (const void*)Data );  }
inline void prop_container::InitButton  ( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_BUTTON,   (const void*)Data );  }

inline void prop_container::GetFloat    ( f32&        Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetInt      ( s32&        Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetGUID     ( guid&       Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetBool     ( xbool&      Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetVector2  ( vector2&    Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetVector3  ( vector3&    Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetBBox     ( bbox&       Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetAngle    ( radian&     Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetRotation ( radian3&    Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetColor    ( xcolor&     Data ) const { GetGeneric( (void*)&Data ); }
inline void prop_container::GetString   ( char*       Data ) const { GetGeneric( (void*) Data ); }
inline void prop_container::GetEnum     ( char*       Data ) const { GetGeneric( (void*) Data ); }
inline void prop_container::GetExternal ( char*       Data ) const { GetGeneric( (void*) Data ); }
inline void prop_container::GetFileName ( char*       Data ) const { GetGeneric( (void*) Data ); }
inline void prop_container::GetButton   ( char*       Data ) const { GetGeneric( (void*) Data ); }

inline void prop_container::SetFloat    ( const f32&        Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetInt      ( const s32&        Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetGUID     ( const guid&       Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetBool     ( const xbool&      Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetVector2  ( const vector2&    Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetVector3  ( const vector3&    Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetBBox     ( const bbox&       Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetAngle    ( const radian&     Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetRotation ( const radian3&    Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetColor    ( const xcolor&     Data ) { SetGeneric( (const void*)&Data ); }
inline void prop_container::SetString   ( const char*       Data ) { SetGeneric( (const void*) Data ); }
inline void prop_container::SetEnum     ( const char*       Data ) { SetGeneric( (const void*) Data ); }
inline void prop_container::SetExternal ( const char*       Data ) { SetGeneric( (const void*) Data ); }
inline void prop_container::SetFileName ( const char*       Data ) { SetGeneric( (const void*) Data ); }
inline void prop_container::SetButton   ( const char*       Data ) { SetGeneric( (const void*) Data ); }

inline const char* prop_container::GetName     ( void ) const { return m_Name; }
inline u32         prop_container::GetType     ( void ) const { return (m_Type  & PROP_TYPE_BASIC_MASK); }
inline u32         prop_container::GetTypeFlags( void ) const { return m_Type; }
inline void*       prop_container::GetRawData( void )       { return m_Data; }
inline void*       prop_container::GetRawData( void ) const { return (void*)m_Data; }
inline void        prop_container::SetName   ( const char* pName ) { ASSERT(pName); x_strcpy( m_Name, pName); }

//=========================================================================
inline
s32 prop_container::GetDataSize( void ) const
{
    switch( m_Type & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_FLOAT:       return sizeof(f32);
    case PROP_TYPE_VECTOR2:     return sizeof(vector2);
    case PROP_TYPE_VECTOR3:     return sizeof(vector3);
    case PROP_TYPE_INT:         return sizeof(s32);
    case PROP_TYPE_BOOL:        return sizeof(xbool);
    case PROP_TYPE_ROTATION:    return sizeof(radian3);
    case PROP_TYPE_ANGLE:       return sizeof(radian);
    case PROP_TYPE_BBOX:        return sizeof(bbox);
    case PROP_TYPE_GUID:        return sizeof(guid);
//    case PROP_TYPE_TRANSFORM:   return sizeof(matrix4);
    case PROP_TYPE_COLOR:       return sizeof(xcolor);
    case PROP_TYPE_FILENAME:    return x_strlen(m_Data);
    case PROP_TYPE_STRING:      return x_strlen(m_Data);
    case PROP_TYPE_ENUM:        return x_strlen(m_Data);
    case PROP_TYPE_BUTTON:      return x_strlen(m_Data);
    case PROP_TYPE_EXTERNAL:    return x_strlen(m_Data);
    default: 
        { 
            x_throw( "Internal error: Unkown property type" );
        }
    }

    return 0;
}

//=========================================================================
inline
void prop_container::InitGeneric( const char* pName, prop_type Type, const void* pData )
{
    // Validate property name
    for( s32 i=0; (m_Name[i] = pName[i]); i++ )
    {
        if( i >= 126 ) 
            x_throw( xfs( "The property name [%s] is too long max 127 chars", pName));
    }

    // copy the type
    m_Type = Type;
    SetGeneric( pData );
}

//=========================================================================
inline
void prop_container::SetGeneric( const void* pData )
{
    switch( m_Type & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_FLOAT:       x_memcpy( m_Data, pData, sizeof(f32) );         break;
    case PROP_TYPE_VECTOR2:     x_memcpy( m_Data, pData, sizeof(vector2) );     break;
    case PROP_TYPE_VECTOR3:     x_memcpy( m_Data, pData, sizeof(vector3) );     break;
    case PROP_TYPE_INT:         x_memcpy( m_Data, pData, sizeof(s32) );         break;
    case PROP_TYPE_BOOL:        x_memcpy( m_Data, pData, sizeof(xbool) );       break;
    case PROP_TYPE_ROTATION:    x_memcpy( m_Data, pData, sizeof(radian3) );     break;
    case PROP_TYPE_ANGLE:       x_memcpy( m_Data, pData, sizeof(radian) );      break;
    case PROP_TYPE_BBOX:        x_memcpy( m_Data, pData, sizeof(bbox) );        break;
    case PROP_TYPE_GUID:        x_memcpy( m_Data, pData, sizeof(guid) );        break;
//    case PROP_TYPE_TRANSFORM:   x_memcpy( m_Data, pData, sizeof(matrix4) );     break;
    case PROP_TYPE_COLOR:       x_memcpy( m_Data, pData, sizeof(xcolor) );      break;
    case PROP_TYPE_FILENAME:    x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_STRING:      x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_ENUM:        x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_EXTERNAL:    x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_BUTTON:      x_strcpy( m_Data, (const char*)pData );         break;
    default:
        x_throw( "Internal error: Unkown property type" );
    }
}

//=========================================================================
inline
void prop_container::GetGeneric( void* pData ) const
{
    switch( m_Type & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_FLOAT:       x_memcpy( pData, m_Data, sizeof(f32) );         break;
    case PROP_TYPE_VECTOR2:     x_memcpy( pData, m_Data, sizeof(vector2) );     break;
    case PROP_TYPE_VECTOR3:     x_memcpy( pData, m_Data, sizeof(vector3) );     break;
    case PROP_TYPE_INT:         x_memcpy( pData, m_Data, sizeof(s32) );         break;
    case PROP_TYPE_BOOL:        x_memcpy( pData, m_Data, sizeof(xbool) );       break;
    case PROP_TYPE_ROTATION:    x_memcpy( pData, m_Data, sizeof(radian3) );     break;
    case PROP_TYPE_ANGLE:       x_memcpy( pData, m_Data, sizeof(radian) );      break;
    case PROP_TYPE_BBOX:        x_memcpy( pData, m_Data, sizeof(bbox) );        break;
    case PROP_TYPE_GUID:        x_memcpy( pData, m_Data, sizeof(guid) );        break;
//    case PROP_TYPE_TRANSFORM:   x_memcpy( pData, m_Data, sizeof(matrix4) );     break;
    case PROP_TYPE_COLOR:       x_memcpy( pData, m_Data, sizeof(xcolor) );      break;
    case PROP_TYPE_FILENAME:    x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_STRING:      x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_ENUM:        x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_EXTERNAL:    x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_BUTTON:      x_strcpy( (char*)pData, m_Data ) ;              break;
    default:
        x_throw( "Internal error: Unkown property type" );
    }
}
//=========================================================================
//=========================================================================
//=========================================================================
#endif //MAKE_PROP_CONTAINER_INLINE
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================
//=========================================================================
//=========================================================================
#ifdef MAKE_PROP_QUERY_INLINE
//=========================================================================
//=========================================================================
//=========================================================================
inline             prop_query::prop_query    ( void ) { m_pData = NULL; m_RootPath[0]=0; m_RootLength=0;}

inline prop_type   prop_query::GetQueryType  ( void )            const { return ClearType(m_Type); }
inline prop_type   prop_query::ClearType     ( u32        Type ) const { return (prop_type)(Type&PROP_TYPE_BASIC_MASK); }
inline prop_type   prop_query::ClearType     ( prop_type  Type ) const { return (prop_type)(((u32)Type)&PROP_TYPE_BASIC_MASK); }

inline prop_query& prop_query::RQueryFloat   ( const char* pName, f32&        Data ) { GenericQuery( TRUE, PROP_TYPE_FLOAT,    pName, Data ); return *this; }
inline prop_query& prop_query::RQueryInt     ( const char* pName, s32&        Data ) { GenericQuery( TRUE, PROP_TYPE_INT,      pName, Data ); return *this; }
inline prop_query& prop_query::RQueryBool    ( const char* pName, xbool&      Data ) { GenericQuery( TRUE, PROP_TYPE_BOOL,     pName, Data ); return *this; }
inline prop_query& prop_query::RQueryGUID    ( const char* pName, guid&       Data ) { GenericQuery( TRUE, PROP_TYPE_GUID,     pName, Data ); return *this; }
inline prop_query& prop_query::RQueryVector2 ( const char* pName, vector2&    Data ) { GenericQuery( TRUE, PROP_TYPE_VECTOR2,  pName, Data ); return *this; }
inline prop_query& prop_query::RQueryVector3 ( const char* pName, vector3&    Data ) { GenericQuery( TRUE, PROP_TYPE_VECTOR3,  pName, Data ); return *this; }
inline prop_query& prop_query::RQueryBBox    ( const char* pName, bbox&       Data ) { GenericQuery( TRUE, PROP_TYPE_BBOX,     pName, Data ); return *this; }
inline prop_query& prop_query::RQueryAngle   ( const char* pName, radian&     Data ) { GenericQuery( TRUE, PROP_TYPE_ANGLE,    pName, Data ); return *this; }
inline prop_query& prop_query::RQueryRotation( const char* pName, radian3&    Data ) { GenericQuery( TRUE, PROP_TYPE_ROTATION, pName, Data ); return *this; }
inline prop_query& prop_query::RQueryColor   ( const char* pName, xcolor&     Data ) { GenericQuery( TRUE, PROP_TYPE_COLOR,    pName, Data ); return *this; }
inline prop_query& prop_query::RQueryString  ( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_STRING,   pName, Data ); return *this; }
inline prop_query& prop_query::RQueryEnum    ( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_ENUM,     pName, Data ); return *this; }
inline prop_query& prop_query::RQueryExternal( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_EXTERNAL, pName, Data ); return *this; }
inline prop_query& prop_query::RQueryFileName( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_FILENAME, pName, Data ); return *this; }
inline prop_query& prop_query::RQueryButton  ( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_BUTTON,   pName, Data ); return *this; }

inline prop_query& prop_query::WQueryFloat   ( const char* pName, const f32&        Data ) { GenericQuery( FALSE, PROP_TYPE_FLOAT,    pName, Data ); return *this; }
inline prop_query& prop_query::WQueryInt     ( const char* pName, const s32&        Data ) { GenericQuery( FALSE, PROP_TYPE_INT,      pName, Data ); return *this; }
inline prop_query& prop_query::WQueryBool    ( const char* pName, const xbool&      Data ) { GenericQuery( FALSE, PROP_TYPE_BOOL,     pName, Data ); return *this; }
inline prop_query& prop_query::WQueryGUID    ( const char* pName, const guid&       Data ) { GenericQuery( FALSE, PROP_TYPE_GUID,     pName, Data ); return *this; }
inline prop_query& prop_query::WQueryVector2 ( const char* pName, const vector2&    Data ) { GenericQuery( FALSE, PROP_TYPE_VECTOR2,  pName, Data ); return *this; }
inline prop_query& prop_query::WQueryVector3 ( const char* pName, const vector3&    Data ) { GenericQuery( FALSE, PROP_TYPE_VECTOR3,  pName, Data ); return *this; }
inline prop_query& prop_query::WQueryBBox    ( const char* pName, const bbox&       Data ) { GenericQuery( FALSE, PROP_TYPE_BBOX,     pName, Data ); return *this; }
inline prop_query& prop_query::WQueryAngle   ( const char* pName, const radian&     Data ) { GenericQuery( FALSE, PROP_TYPE_ANGLE,    pName, Data ); return *this; }
inline prop_query& prop_query::WQueryRotation( const char* pName, const radian3&    Data ) { GenericQuery( FALSE, PROP_TYPE_ROTATION, pName, Data ); return *this; }
inline prop_query& prop_query::WQueryColor   ( const char* pName, const xcolor&     Data ) { GenericQuery( FALSE, PROP_TYPE_COLOR,    pName, Data ); return *this; }
inline prop_query& prop_query::WQueryString  ( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_STRING,   pName, Data ); return *this; }
inline prop_query& prop_query::WQueryEnum    ( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_ENUM,     pName, Data ); return *this; }
inline prop_query& prop_query::WQueryExternal( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_EXTERNAL, pName, Data ); return *this; }
inline prop_query& prop_query::WQueryFileName( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_FILENAME, pName, Data ); return *this; }
inline prop_query& prop_query::WQueryButton  ( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_BUTTON,   pName, Data ); return *this; }

inline xbool prop_query::VarFloat     ( const char* pPropName, f32&     Data, f32    Min, f32    Max ){ if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_FLOAT, Data, Min, Max ); m_MinFloat = Min; m_MaxFloat = Max; return TRUE; }
inline xbool prop_query::VarInt       ( const char* pPropName, s32&     Data, s32    Min, s32    Max ){ if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_INT,   Data, Min, Max ); m_MinInt   = Min; m_MaxInt   = Max; return TRUE; }
inline xbool prop_query::VarAngle     ( const char* pPropName, radian&  Data, radian Min, radian Max ){ if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_ANGLE, Data, Min, Max ); m_MinFloat = Min; m_MaxFloat = Max; return TRUE; }
inline xbool prop_query::VarBool      ( const char* pPropName, xbool&   Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_BOOL,  Data, 0,   1   ); return TRUE; }
inline xbool prop_query::VarGUID      ( const char* pPropName, guid&    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_GUID,     Data ); return TRUE; }
inline xbool prop_query::VarVector2   ( const char* pPropName, vector2& Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_VECTOR2,  Data ); return TRUE; }
inline xbool prop_query::VarVector3   ( const char* pPropName, vector3& Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_VECTOR3,  Data ); return TRUE; }
inline xbool prop_query::VarBBox      ( const char* pPropName, bbox&    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_BBOX,     Data ); return TRUE; }
inline xbool prop_query::VarRotation  ( const char* pPropName, radian3& Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_ROTATION, Data ); return TRUE; }
inline xbool prop_query::VarColor     ( const char* pPropName, xcolor&  Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_COLOR,    Data ); return TRUE; }
inline xbool prop_query::VarString    ( const char* pPropName, char*    Data, s32 MaxStrLen )         { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, MaxStrLen ); return TRUE; }
inline xbool prop_query::VarEnum      ( const char* pPropName, char*    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, 256 );       return TRUE; }
inline xbool prop_query::VarExternal  ( const char* pPropName, char*    Data, s32 MaxStrLen )         { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, MaxStrLen ); return TRUE; }
inline xbool prop_query::VarFileName  ( const char* pPropName, char*    Data, s32 MaxStrLen )         { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, MaxStrLen ); return TRUE; }
inline xbool prop_query::VarButton    ( const char* pPropName, char*    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, 256 );       return TRUE; }

inline f32     prop_query::GetVarFloat   ( f32    MinVal, f32    MaxVal ) { f32     Data; GetGenericVar( PROP_TYPE_FLOAT,    Data, MinVal, MaxVal ); m_MinFloat = MinVal; m_MaxFloat = MaxVal; return Data; }
inline s32     prop_query::GetVarInt     ( s32    MinVal, s32    MaxVal ) { s32     Data; GetGenericVar( PROP_TYPE_INT,      Data, MinVal, MaxVal ); m_MinInt   = MinVal; m_MaxInt   = MaxVal; return Data; }
inline radian  prop_query::GetVarAngle   ( radian MinVal, radian MaxVal ) { radian  Data; GetGenericVar( PROP_TYPE_ANGLE,    Data, MinVal, MaxVal ); m_MinFloat = MinVal; m_MaxFloat = MaxVal; return Data; }
inline xbool   prop_query::GetVarBool    ( void )                         { xbool   Data; GetGenericVar( PROP_TYPE_BOOL,     Data ); return Data; }
inline guid    prop_query::GetVarGUID    ( void )                         { guid    Data; GetGenericVar( PROP_TYPE_GUID,     Data ); return Data; }
inline vector2 prop_query::GetVarVector2 ( void )                         { vector2 Data; GetGenericVar( PROP_TYPE_VECTOR2,  Data ); return Data; }
inline vector3 prop_query::GetVarVector3 ( void )                         { vector3 Data; GetGenericVar( PROP_TYPE_VECTOR3,  Data ); return Data; }
inline bbox    prop_query::GetVarBBox    ( void )                         { bbox    Data; GetGenericVar( PROP_TYPE_BBOX,     Data ); return Data; }
inline radian3 prop_query::GetVarRotation( void )                         { radian3 Data; GetGenericVar( PROP_TYPE_ROTATION, Data ); return Data; }
inline xcolor  prop_query::GetVarColor   ( void )                         { xcolor  Data; GetGenericVar( PROP_TYPE_COLOR,    Data ); return Data; }
inline const char* prop_query::GetVarFileName( void ) { ASSERT( PROP_TYPE_FILENAME == GetQueryType() ); return (char*)m_pData; }
inline const char* prop_query::GetVarString  ( void ) { ASSERT( PROP_TYPE_STRING   == GetQueryType() ); return (char*)m_pData; }
inline const char* prop_query::GetVarEnum    ( void ) { ASSERT( PROP_TYPE_ENUM     == GetQueryType() ); return (char*)m_pData; }
inline const char* prop_query::GetVarExternal( void ) { ASSERT( PROP_TYPE_EXTERNAL == GetQueryType() ); return (char*)m_pData; }

inline void prop_query::SetVarFloat   ( f32            Data ) { SetGenericVar( PROP_TYPE_FLOAT,    Data ); }
inline void prop_query::SetVarInt     ( s32            Data ) { SetGenericVar( PROP_TYPE_INT,      Data ); }
inline void prop_query::SetVarBool    ( xbool          Data ) { SetGenericVar( PROP_TYPE_BOOL,     Data ); }
inline void prop_query::SetVarGUID    ( guid           Data ) { SetGenericVar( PROP_TYPE_GUID,     Data ); }
inline void prop_query::SetVarVector2 ( const vector2& Data ) { SetGenericVar( PROP_TYPE_VECTOR2,  Data ); }
inline void prop_query::SetVarVector3 ( const vector3& Data ) { SetGenericVar( PROP_TYPE_VECTOR3,  Data ); }
inline void prop_query::SetVarBBox    ( const bbox&    Data ) { SetGenericVar( PROP_TYPE_BBOX,     Data ); }
inline void prop_query::SetVarAngle   ( radian         Data ) { SetGenericVar( PROP_TYPE_ANGLE,    Data ); }
inline void prop_query::SetVarRotation( const radian3& Data ) { SetGenericVar( PROP_TYPE_ROTATION, Data ); }
inline void prop_query::SetVarColor   ( xcolor         Data ) { SetGenericVar( PROP_TYPE_COLOR,    Data ); }
inline void prop_query::SetVarFileName( const char*   pData, s32 MaxStrLen ) { ASSERT( PROP_TYPE_FILENAME == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, MaxStrLen ); m_MaxStringLength = MaxStrLen; }
inline void prop_query::SetVarString  ( const char*   pData, s32 MaxStrLen ) { ASSERT( PROP_TYPE_STRING   == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, MaxStrLen ); m_MaxStringLength = MaxStrLen; }
inline void prop_query::SetVarButton  ( const char*   pData )                { ASSERT( PROP_TYPE_BUTTON   == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, 256       ); m_MaxStringLength = 256;       }
inline void prop_query::SetVarEnum    ( const char*   pData )                { ASSERT( PROP_TYPE_ENUM     == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, 256       ); m_MaxStringLength = 256;       }
inline void prop_query::SetVarExternal( const char*   pData, s32 MaxStrLen ) { ASSERT( PROP_TYPE_EXTERNAL == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, MaxStrLen ); m_MaxStringLength = MaxStrLen; }

//=========================================================================

inline s32 prop_query::PushPath( const char* pRootPath )
{
    s32 i;
    s32 OldID = m_RootLength;

    for( i=0; pRootPath[i]; i++ )
    {
        // Can we push it or not?
        // TODO: We can only push a path if this property also has that path
        //       so for right now we will fail to push it but restoring the length.
        //       In reality should return a -1 or something like that.
        if( m_PropName[m_RootLength+i] != pRootPath[i] )
        {
            m_RootLength = OldID;
            return OldID;
        }

        m_RootPath[m_RootLength+i]=pRootPath[i];
    }

    ASSERT( i<128 );
    m_RootLength += i;
    m_RootPath[m_RootLength]=0;
    
    return OldID;
}

//=========================================================================

inline void prop_query::PopPath( s32 iPath )
{
    m_RootLength = iPath;
    m_RootPath[m_RootLength]=0;
}

//=========================================================================
inline
xbool prop_query::IsVar( const char* pString )
{
    // Super fast string compare
    for( s32 i=m_PropNameLen; i>=m_RootLength; --i )
    {
        if( pString[i-m_RootLength] != m_PropName[i] ) return FALSE;
    }

    return TRUE;
}

//=========================================================================
inline
xbool prop_query::IsSimilarPath( const char* pPath )
{
    return x_stristr( &m_PropName[m_RootLength], pPath ) != NULL;
}

//=========================================================================
inline
xbool prop_query::IsBasePath( const char* pPath )
{
    const char* pStr1 = &m_PropName[m_RootLength];
    const char* pStr2 = pPath;
    ASSERT( pStr1 && pStr2 );

    s32 C1, C2;

    do
    {
        C1 = (s32)(*(pStr1++));
        if( (C1 >= 'A') && (C1 <= 'Z') )
            C1 -= ('A' - 'a');

        C2 = (s32)(*(pStr2++));
        if( (C2 >= 'A') && (C2 <= 'Z') )
            C2 -= ('A' - 'a');

    } while( C1 && C2 && (C1 == C2) );

    // If we made it to the end of pStr2 then we are good.
    if( C2 == 0 )
        return TRUE;

    return FALSE;
}

//=========================================================================

inline
xbool prop_query::IsRead( void )
{
    return m_bRead;
}

//=========================================================================
inline
s32 prop_query::GetIndex( s32 Number )
{
    ASSERT( Number >= 0 );
    if( Number >= m_nIndices )
        return 0;
    return m_Index[Number];
}

//=========================================================================
inline
prop_query& prop_query::RQuery( prop_container& Container )
{
    ParseString( Container.GetName() );
    m_bRead       = TRUE;
    m_Type        = Container.GetType();
    m_DataSize    = Container.GetDataSize();
    m_pData       = Container.GetRawData();
    m_RootPath[0] = 0;
    m_RootLength  = 0;

    return *this;
}

//=========================================================================
inline
prop_query& prop_query::RQuery( const char* pName, prop_container& Container )
{
    Container.SetName( pName );
    return RQuery( Container );
}

//=========================================================================
inline
prop_query& prop_query::WQuery( const prop_container& Container )
{
    ParseString( Container.GetName() );
    m_bRead       = FALSE;
    m_Type        = Container.GetType();
    m_DataSize    = Container.GetDataSize();
    m_pData       = (void*)Container.GetRawData();
    m_RootPath[0] = 0;
    m_RootLength  = 0;

    return *this;
}
//=========================================================================
//=========================================================================
#endif //MAKE_PROP_QUERY_INLINE
//=========================================================================
//=========================================================================

//=========================================================================
// END
//=========================================================================
#endif
