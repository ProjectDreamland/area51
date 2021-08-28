
#ifndef RTTI_HPP
#define RTTI_HPP

#include "x_files.hpp"

#pragma warning( disable : 4355 ) // warning C4355: 'this' : used in base member initializer list

//==============================================================================
// RTTI
//==============================================================================
struct rtti
{
    inline rtti( const char* pTypeName ) : pType(pTypeName), Next( *this ){}
    inline rtti( const char* pTypeName, const rtti& RTTI ) : pType(pTypeName), Next( RTTI ){}
    xbool rtti::IsTypeOf( const rtti& RTTI ) const 
    { 
        const rtti* p = this; 
        do
        { 
            if( p == &RTTI    ) return TRUE;
            if( p == &p->Next ) break;
            p = &p->Next;

        } while(1);
        return FALSE;
    }

    const char* pType;
    const rtti& Next;
};

#define CREATE_RTTI( TYPE, TYPE_PARENT, BASE_CLASS )               \
static  inline const rtti& GetRTTI( void )                         { static rtti s_RTTI( #TYPE, TYPE_PARENT::GetRTTI() ); return s_RTTI; } \
virtual inline const rtti& VGetRTTI( void ) const                  { return TYPE::GetRTTI(); } \
static inline TYPE&        GetSaveType( BASE_CLASS& Object )       { ASSERT( Object.VGetRTTI().IsTypeOf(TYPE::GetRTTI()) ); return *((TYPE*)&Object); }       \
static inline const TYPE&  GetSaveType( const BASE_CLASS& Object ) { ASSERT( Object.VGetRTTI().IsTypeOf(TYPE::GetRTTI()) ); return *((const TYPE*)&Object); } \

#define CREATE_RTTI_BASE( TYPE )                                   \
static  inline const rtti& GetRTTI( void )                         { static rtti s_RTTI( #TYPE ); return s_RTTI; } \
virtual inline const rtti& VGetRTTI( void ) const                  { return TYPE::GetRTTI(); } \
static inline TYPE&        GetSaveType( TYPE& Object )             { ASSERT( Object.VGetRTTI().IsTypeOf(TYPE::GetRTTI()) ); return *((TYPE*)&Object); }       \
static inline const TYPE&  GetSaveType( const TYPE& Object )       { ASSERT( Object.VGetRTTI().IsTypeOf(TYPE::GetRTTI()) ); return *((const TYPE*)&Object); } \

//==============================================================================
// END
//==============================================================================
#endif