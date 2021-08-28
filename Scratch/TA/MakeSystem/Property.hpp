
#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include "x_files.hpp"

enum prop_type
{
    PROP_TYPE_FLOAT      = (1<< 0),
    PROP_TYPE_VECTOR3    = (1<< 1),
    PROP_TYPE_INT        = (1<< 2),
    PROP_TYPE_ROTATION   = (1<< 3),
    PROP_TYPE_ANGLE      = (1<< 4), 
    PROP_TYPE_BBOX       = (1<< 5), 
    PROP_TYPE_TRANSFORM  = (1<< 6), 
    PROP_TYPE_COLOR      = (1<< 7), 
    PROP_TYPE_ENUM       = (1<< 8), 

    PROP_TYPE_END,        
    PROP_TYPE_BASIC      = (PROP_TYPE_END-2),

    PROP_TYPE_READ_ONLY  = (1<<20), 
    PROP_TYPE_BUTTON     = (1<<21), 
    PROP_TYPE_HEADER     = (1<<22), 
};

struct prop_enum
{
    char    String[128];
    u32     Flags;
    s32     Number;
    char    EnumList[32][128];

    void    Set( const char* pString, u32 Flags );
    void    Set( const char* pString, s32 Number, u32 Flags = 0 );
    void    Set( const char* pString, const char** EnumList, u32 Flags = 0 );
};

//=========================================================================
// CLASS
//=========================================================================
class prop_query
{
public:
//=========================================================================
// Queries
//=========================================================================

                prop_query      ( void ){};

    void        RQueryFloat     ( const char* pName,       f32&     Data ); 
    void        RQueryInt       ( const char* pName,       s32&     Data ); 
    void        RQueryVector3   ( const char* pName,       vector3& Data ); 
    void        RQueryAngle     ( const char* pName,       radian&  Data ); 
    void        RQueryRotation  ( const char* pName,       radian3& Data ); 

    void        WQueryFloat     ( const char* pName, const f32&     Data ); 
    void        WQueryInt       ( const char* pName, const s32&     Data ); 
    void        WQueryVector3   ( const char* pName, const vector3& Data ); 
    void        WQueryAngle     ( const char* pName, const radian&  Data ); 
    void        WQueryRotation  ( const char* pName, const radian3& Data ); 

//=========================================================================
// Properties
//=========================================================================

    // Generic Getters and setters
    void        VarFloat        ( f32&     Data );
    void        VarInt          ( s32&     Data );
    void        VarVector3      ( vector3& Data );
    void        VarAngle        ( radian&  Data );
    void        VarRotation     ( radian3& Data );

    // state
    xbool       IsVar           ( const char* pString );
    xbool       IsEditor        ( void );
    xbool       IsRead          ( void );
    s32         GetIndex        ( s32 Number );

    // Getters
    f32         GetVarFloat     ( void );
    s32         GetVarInt       ( void );
    vector3     GetVarVector3   ( void );
    radian      GetVarAngle     ( void );
    radian3     GetVarRotation  ( void );

//=========================================================================
// PROTECTED
//=========================================================================
protected:

    enum flags
    {
        FLAGS_READ   = (1<<20),
        FLAGS_EDITOR = (1<<21),
    };

    template< class T > inline
    void GenericQuery( xbool bRead, prop_type Type, const char* pName, T& Data )
    {
        ParseString( pName );
        m_Flags     = (u32)Type;
        m_Flags    |= (bRead?FLAGS_READ:0);
        m_pData     = (void*)&Data;
        m_DataSize  = sizeof(T);
    }

    template< class T > inline
    void GenericVar( prop_type Type, T& Data )
    {
        ASSERT( ((u32)Type&PROP_TYPE_BASIC) == ((u32)m_Flags&PROP_TYPE_BASIC) ); 
        ASSERT( m_DataSize == sizeof(T) );
        if( m_Flags & FLAGS_READ ) *((T*)m_pData) = Data;
        else                       Data          = *((T*)m_pData);
    }

    void ParseString( const char* pString );

    s32     m_StrLen;
    char    m_String[128];
    s32     m_nIndices;
    s32     m_Index[16];
    u32     m_Flags;
    s32     m_DataSize;
    void*   m_pData;
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

inline 
void prop_enum::Set( const char* pString, u32 aFlags )
{
    ASSERT( pString );
    ASSERT( x_strlen( pString ) < 128 );
    x_strcpy( String, pString );
    Flags  = aFlags;
    Number = 0;
}

//=========================================================================

inline 
void prop_enum::Set( const char* pString, s32 aNumber, u32 aFlags )
{
    ASSERT( pString );
    ASSERT( x_strlen( pString ) < 128 );
    x_strcpy( String, pString );
    Flags = aFlags | PROP_TYPE_HEADER;
    Number = aNumber;
}

//=========================================================================

inline
void prop_enum::Set( const char* pString, const char** pEnumList, u32 aFlags )
{
    Flags = aFlags | PROP_TYPE_ENUM;
    x_strcpy( String, pString );

    s32 i;
    for( i=Number=0; Number<32; Number++ )
    {
        if( pEnumList[Number] == NULL ) 
            break;

        x_strcpy( EnumList[Number], pEnumList[Number] ) ;
    }
}

//=========================================================================

inline void prop_query::RQueryFloat   ( const char* pName, f32&        Data ) { GenericQuery( TRUE, PROP_TYPE_FLOAT,    pName, Data ); }
inline void prop_query::RQueryInt     ( const char* pName, s32&        Data ) { GenericQuery( TRUE, PROP_TYPE_INT,      pName, Data ); }
inline void prop_query::RQueryVector3 ( const char* pName, vector3&    Data ) { GenericQuery( TRUE, PROP_TYPE_VECTOR3,  pName, Data ); }
inline void prop_query::RQueryAngle   ( const char* pName, radian&     Data ) { GenericQuery( TRUE, PROP_TYPE_ANGLE,    pName, Data ); }
inline void prop_query::RQueryRotation( const char* pName, radian3&    Data ) { GenericQuery( TRUE, PROP_TYPE_ROTATION, pName, Data ); }

inline void prop_query::WQueryFloat   ( const char* pName, const f32&        Data ) { GenericQuery( FALSE, PROP_TYPE_FLOAT,    pName, Data ); }
inline void prop_query::WQueryInt     ( const char* pName, const s32&        Data ) { GenericQuery( FALSE, PROP_TYPE_INT,      pName, Data ); }
inline void prop_query::WQueryVector3 ( const char* pName, const vector3&    Data ) { GenericQuery( FALSE, PROP_TYPE_VECTOR3,  pName, Data ); }
inline void prop_query::WQueryAngle   ( const char* pName, const radian&     Data ) { GenericQuery( FALSE, PROP_TYPE_ANGLE,    pName, Data ); }
inline void prop_query::WQueryRotation( const char* pName, const radian3&    Data ) { GenericQuery( FALSE, PROP_TYPE_ROTATION, pName, Data ); }

inline void prop_query::VarFloat   ( f32&     Data ){ GenericVar( PROP_TYPE_FLOAT,    Data ); }
inline void prop_query::VarInt     ( s32&     Data ){ GenericVar( PROP_TYPE_INT,      Data ); }
inline void prop_query::VarVector3 ( vector3& Data ){ GenericVar( PROP_TYPE_VECTOR3,  Data ); }
inline void prop_query::VarAngle   ( radian&  Data ){ GenericVar( PROP_TYPE_ANGLE,    Data ); }
inline void prop_query::VarRotation( radian3& Data ){ GenericVar( PROP_TYPE_ROTATION, Data ); }

inline f32     prop_query::GetVarFloat   ( void ) { f32     Data; GenericVar( PROP_TYPE_FLOAT,    Data ); return Data; }
inline s32     prop_query::GetVarInt     ( void ) { s32     Data; GenericVar( PROP_TYPE_INT,      Data ); return Data; }
inline vector3 prop_query::GetVarVector3 ( void ) { vector3 Data; GenericVar( PROP_TYPE_VECTOR3,  Data ); return Data; }
inline radian  prop_query::GetVarAngle   ( void ) { radian  Data; GenericVar( PROP_TYPE_ANGLE,    Data ); return Data; }
inline radian3 prop_query::GetVarRotation( void ) { radian3 Data; GenericVar( PROP_TYPE_ROTATION, Data ); return Data; }

//=========================================================================
inline
xbool prop_query::IsVar( const char* pString )
{
    // Super fast string compare
    for( s32 i=m_StrLen; i>=0; --i )
    {
        if( pString[i] != m_String[i] ) return FALSE;
    }

    return TRUE;
}

//=========================================================================
inline
xbool prop_query::IsEditor( void )
{
    return (m_Flags&FLAGS_EDITOR)!=0;
}

//=========================================================================

inline
xbool prop_query::IsRead( void )
{
    return (m_Flags&FLAGS_READ)!=0;
}

//=========================================================================
inline
s32 prop_query::GetIndex( s32 Number )
{
    ASSERT( Number < m_nIndices );
    ASSERT( Number >= 0 );
    return m_Index[Number];
}

//=========================================================================
// END
//=========================================================================
#endif
