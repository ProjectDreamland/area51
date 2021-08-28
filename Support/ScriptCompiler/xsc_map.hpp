//==============================================================================
//  
//  xsc_map.hpp
//  
//==============================================================================

#ifndef XSC_MAP_HPP
#define XSC_MAP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  XSC_MAP
//==============================================================================
//
//  A simple s32 to s32 map
//
//==============================================================================

class xsc_map
{
public:
    struct entry
    {
        s32 Key;
        s32 Value;
    };

private:
        void            EnsureCapacity  ( s32 Capacity );

public:
                        xsc_map         ( void );
                        xsc_map         ( const xsc_map& Map );
                       ~xsc_map         ( void );

        void            Clear           ( void );
        s32             GetCount        ( void ) const;

        void            Add             ( s32 Key, s32 Value );
        entry*          GetEntry        ( s32 Index );

        entry*          FindByKey       ( s32 Key );
        entry*          FindByValue     ( s32 Value );
        entry*          FindByPair      ( s32 Key, s32 Value );

        s32             ValueFromKey    ( s32 Key );
        s32             KeyFromValue    ( s32 Value );

const   xsc_map&        operator =      ( const xsc_map&    Map );
const   xsc_map&        operator +=     ( const entry&      Entry );

protected:
        entry*      m_pData;
        s32         m_Capacity;
        s32         m_Count;
};

//==============================================================================
#endif // X_MAP_HPP
//==============================================================================
