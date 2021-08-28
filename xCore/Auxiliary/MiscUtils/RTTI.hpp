#ifndef RTTI_HPP
#define RTTI_HPP

#include "x_files.hpp"
//==============================================================================
// RTTI
//==============================================================================
// Using the RTTI
//------------------------------------------------------------------------------
// struct base
// {
//     CREATE_RTTI_BASE( base );
// };
// 
// struct pepe : public base
// {
//     CREATE_RTTI( pepe, base, base )
//     void Normal( void ){}
// };
// 
// struct super_pepe : public pepe
// {
//     CREATE_RTTI( super_pepe, pepe, base )
//     void Super( void ){}
// };
// 
// void main( void )
// {
//     super_pepe SP;         // The original class
//     base&      B = SP;     // Some generic base reference
// 
//     // We want to safely convert to pepe
//     pepe& P = pepe::GetSaveType( B );
// 
//     // cast the base class right
//     if( B.IsKindOf( super_pepe::GetRTTI() ) )
//     {
//         super_pepe& SP = super_pepe::GetSaveType( B );
//         SP.Super();
//     }
//     else if( B.IsKindOf( pepe::GetRTTI() ) )
//     {
//         pepe& P = pepe::GetSaveType( B );
//         P.Normal();
//     }
// }
//==============================================================================
#define CREATE_RTTI( TYPE, TYPE_PARENT, BASE_CLASS )
#define CREATE_RTTI_BASE( TYPE ) 

struct rtti
{
    xbool IsKindOf( const rtti& RTTI ) const;

    rtti( const char* pTypeName );
    rtti( const char* pTypeName, const rtti& RTTI );

    const rtti& Next( void )const
    {
        return *pNext;
    }

    const char* Type( void )const
    {
        return pType;
    }

private:

    const char* pType;      // This is not really need it, just useful in debugger.
    rtti* pNext;
};

//==============================================================================
// IMPLEMENTATION
//==============================================================================
#ifdef TARGET_PC
#pragma warning( push ) 
#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list
#endif
#undef CREATE_RTTI
#undef CREATE_RTTI_BASE

inline rtti::rtti( const char* pTypeName ): pType( pTypeName )
{
    pNext = this;
}

inline rtti::rtti( const char* pTypeName, const rtti& RTTI ) : pType(pTypeName)
{
    pNext = (rtti*)&RTTI;
}

inline xbool rtti::IsKindOf( const rtti& RTTI ) const 
{ 
    const rtti* p = this; 
    do
    { 
        if( p == &RTTI    ) return true;
        if( p == p->pNext ) break;
        p = p->pNext;

    } while( 1 );
    return false;
}

#define PRIVATE_RTTI_FUNCTION_SET( TYPE, BASE_CLASS )                 \
    virtual inline const rtti& GetObjectRTTI( void ) const            \
    {                                                                 \
        return TYPE::GetRTTI();                                       \
    }                                                                 \
    inline const xbool IsKindOf( const rtti& RTTI ) const             \
    {                                                                 \
        return GetObjectRTTI().IsKindOf( RTTI );                      \
    }                                                                 \
    static inline TYPE& GetSafeType( BASE_CLASS& Object )             \
    {                                                                 \
        ASSERT( Object.IsKindOf(TYPE::GetRTTI()) );                   \
        return *((TYPE*)&Object);                                     \
    }                                                                 \
    static inline const TYPE& GetSafeType( const BASE_CLASS& Object ) \
    {                                                                 \
        ASSERT( Object.IsKindOf(TYPE::GetRTTI()) );                   \
        return *((const TYPE*)&Object);                               \
    }

#define CREATE_RTTI( TYPE, TYPE_PARENT, BASE_CLASS )                  \
    static  inline const rtti& GetRTTI( void )                        \
    {                                                                 \
        static rtti s_RTTI( #TYPE, TYPE_PARENT::GetRTTI() );          \
        return s_RTTI;                                                \
    }                                                                 \
    PRIVATE_RTTI_FUNCTION_SET(TYPE,BASE_CLASS)

#define CREATE_RTTI_BASE( TYPE )                                      \
    static  inline const rtti& GetRTTI( void )                        \
    {                                                                 \
        static rtti s_RTTI( #TYPE );                                  \
        return s_RTTI;                                                \
    }                                                                 \
    PRIVATE_RTTI_FUNCTION_SET(TYPE,TYPE)

#ifdef TARGET_PC
#pragma warning( pop )
#endif
//==============================================================================
// END
//==============================================================================
#endif