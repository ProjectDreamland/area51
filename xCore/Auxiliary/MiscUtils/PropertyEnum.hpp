#ifndef PROPERTYENUM_HPP
#define PROPERTYENUM_HPP

//=============================================================================

#include "x_files.hpp"

//=========================================================================
// enum_pair : Templated pair class used within the enum_table..
//=========================================================================

extern const char*  k_EnumEndStringConst;
extern const s32    k_MaxEnumInTable;
extern char         g_EnumStringOut[255];

template< class T>
class enum_pair
{
public:
    
    enum_pair ( const char* pString, T EnumVal )
    {
        m_EnumVal=EnumVal;
        m_pString=pString;
    }

    T               m_EnumVal;
    const char*     m_pString;
};

//=========================================================================
// enum_table : Templated enumeration table useful within the property system..
//  NOTE: The enum_table expects a table of enum_pairs with the last pair 
//          be the terminating pair by convention with the String  g_EnumEndStringConst
//          and the EnumVal of the invalid value you wanted return on invalid results.
//=========================================================================

template< class T >
class enum_table
{
public:

    enum_table  ( enum_pair<T>* pTable );

    const char* BuildString         ( void ) const;
    
    xbool       GetValue            ( const char* pString, T& Value ) const;
    const char* GetString           ( T Value ) const;

    T&          operator[]          ( s32 Index ) ;
    T           GetValueFromIndex   ( s32 Index ) const ;
    const char* GetStringFromIndex  ( s32 Index ) const ;
    
    s32         GetCount            ( void ) const { return m_Count ; }
    xbool       IsEmpty             ( void ) const { return m_Count > 0; }

    xbool       DoesValueExist      ( T Value ) const;

    s32         GetIndex            ( const char* pString ) const;
    s32         GetIndex            ( T Value ) const;

private:

    enum_pair<T>*           m_pTable;
    s32                     m_Count;
};

//=============================================================================

template< class T >
enum_table<T>::enum_table  ( enum_pair<T>* pTable ) : m_pTable(NULL), m_Count(0)
{
    //Careful about the pTable, if it's allocated dynamically make sure its valid
    //for the lifetime of the enum_table...

    ASSERT( pTable );

    s32 i = 0;

    while(1)
    {
        if ( i > k_MaxEnumInTable )
        {
            x_throw( "Enumeration table has exceeded maximum table size limits.\n" );
        }

        if (x_strcmp(pTable[i].m_pString, k_EnumEndStringConst) == 0)
        {
            i++;
            break;
        }

        i++;
    }

    m_pTable    = pTable;
    m_Count     = i;
}

//=============================================================================

template< class T >
const char*  enum_table<T>::BuildString ( void ) const
{
    char*   pRawString      = g_EnumStringOut;
    s32     MaxStrLen       = 255;
    s32     CurrentIndex    = 0;
    
    ( void ) MaxStrLen;
    
    for (int i = 0; i < m_Count-1; i++)
    {
        if( CurrentIndex >= MaxStrLen )
        {
            x_throw( "enum_table::BuildString, tried to build enum string greater than buffer capacity.\n" );
        }
        
        x_strcpy( (char*) &pRawString[CurrentIndex], m_pTable[i].m_pString );
        
        CurrentIndex += x_strlen(m_pTable[i].m_pString);
        
        pRawString[CurrentIndex] = 0;
        
        CurrentIndex++;
    }
    
    if( CurrentIndex >= MaxStrLen )
    {
        x_throw( "enum_table::BuildString, tried to build enum string greater than buffer capacity.\n" );
    }
    
    
    pRawString[CurrentIndex] = 0;

    return pRawString;
}

//=============================================================================

template< class T >
xbool enum_table<T>::GetValue ( const char* pString, T& Value ) const
{
    s32 Index = GetIndex ( pString );
    
    if( Index >= 0 )
    {
        Value = m_pTable[Index].m_EnumVal;
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

template< class T >    
const char* enum_table<T>::GetString ( T Value ) const
{
    s32 Index = GetIndex ( Value );
    
    ASSERT (Index >= 0);

    if (Index < 0 )
        return NULL;

    return m_pTable[Index].m_pString;
}

//=============================================================================

template< class T >
T enum_table<T>::GetValueFromIndex ( s32 Index ) const 
{
    ASSERT( Index >= 0 && Index < m_Count );

    return m_pTable[Index].m_EnumVal;
}

//=============================================================================

template< class T >
T& enum_table<T>::operator[]( s32 Index ) 
{
    ASSERT( Index >= 0 && Index < m_Count );

    return m_pTable[Index].m_EnumVal;
}

//=============================================================================

template< class T >    
const char* enum_table<T>::GetStringFromIndex ( s32 Index ) const
{
    ASSERT( Index >= 0 && Index < m_Count );
    
    return m_pTable[Index].m_pString;
}

//=============================================================================

template< class T >
xbool enum_table<T>::DoesValueExist ( T Value ) const
{
    s32 Index = GetIndex ( Value );
    
    return Index >= 0;
}

//=============================================================================

template< class T >
s32 enum_table<T>::GetIndex ( const char* pString ) const
{
    for (int i = 0; i < m_Count-1; i++)
    {
        if (x_strcmp( pString, m_pTable[i].m_pString ) == 0)
            return i;
    }

    return -1;
}

//=============================================================================

template< class T >
s32 enum_table<T>::GetIndex ( T Value ) const
{
     for (int i = 0; i < m_Count-1; i++)
    {
        if ( Value == m_pTable[i].m_EnumVal )
            return i;
    }

    return -1;
}

//=========================================================================
// simple_string : Templated simple string class with some common funtionality
//=========================================================================

template< class T >
class enum_list
{
public:

    void        Add                 ( const char* pString, T Value );
    void        BuildString         ( char* pString ) const;
    
    T           GetValue            ( const char* pString ) const;
    const char* GetString           ( T Value ) const;
    
    T&          operator[]          ( s32 Index )       { return( m_lEnum[ Index ].Value   ); }
    T           GetValueFromIndex   ( s32 Index ) const { return( m_lEnum[ Index ].Value   ); }
    const char* GetStringFromIndex  ( s32 Index ) const { return( m_lEnum[ Index ].pString ); }
    
    s32         GetCount            ( void ) const { return( m_lEnum.GetCount() ); }
    xbool       IsInitialized       ( void ) const { return( m_lEnum.GetCount() > 0 ); }

    xbool       DoesValueExist      ( T Value ) const;

private:
    
    struct node
    {
        const char* pString;
        T           Value;
    };

    xarray<node>    m_lEnum;
};

//=============================================================================

template< class T >
void enum_list<T>::Add( const char* pString, T Value )
{
    node& Node   = m_lEnum.Append();
    Node.pString = pString;
    Node.Value   = Value;
}

//=============================================================================

template< class T >
void enum_list<T>::BuildString( char* pString ) const
{
    for( s32 i=0; i<m_lEnum.GetCount(); i++ )
    {
        node& Node = m_lEnum[i];
        
        s32 Len = x_strlen( Node.pString ) + 1;
        x_memcpy( pString,  Node.pString, Len );
        pString += Len;
    }
    
    pString[0] = '\0';
}

//=============================================================================

template< class T >
T enum_list<T>::GetValue( const char* pString ) const
{
    for( s32 i=0; i<m_lEnum.GetCount(); i++ )
    {
        node& Node = m_lEnum[i];
        
        if( x_strcmp( Node.pString, pString ) == 0 )
        {
            return( Node.Value );
        }
    }

    x_throw( xfs( "String [%s] could not be found in enum", pString ) );

    return m_lEnum[0].Value;
}

//=============================================================================

template< class T >
const char* enum_list<T>::GetString( T Value ) const
{
    for( s32 i=0; i<m_lEnum.GetCount(); i++ )
    {
        node& Node = m_lEnum[i];
        
        if( Node.Value == Value )
        {
            return( Node.pString );
        }
    }
    
    x_throw( xfs( "Value [%d] could not be found in enum", Value ) );
    return( NULL );
}

//=============================================================================

template< class T >
xbool enum_list<T>::DoesValueExist( T Value ) const
{
    for( s32 i=0; i<m_lEnum.GetCount(); i++ )
    {
        node& Node = m_lEnum[i];
        
        if( Node.Value == Value )
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

//=============================================================================

#endif
