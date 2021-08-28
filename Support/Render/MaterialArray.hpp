#ifndef MATERIALARRAY_HPP
#define MATERIALARRAY_HPP

#include "render.hpp"

#ifndef RENDER_PRIVATE
#error "This file is for internal use by the rendering system. Please don't include it!"
#endif

//=============================================================================
// PRIVATE PRIVATE PRIVATE
//
// This class is VERY similar to a standard xharray, except that while the
// xharray has a direct mapping between the handle and the data, ours will
// use in indirect mapping through indices. This will be better for sorting,
// and allow us to still store xhandles for the normal render sort key. Also
// this guarantees that we'll be accessing materials linearly for rendering.
// This class should only be used by the rendering system and the game should
// go through the interface provided in Render.hpp
//=============================================================================


class material_array
{
public:
         material_array ( void );
        ~material_array ( void );

    void        Sort                ( void );
    s32         GetCount            ( void ) const;
    void        Clear               ( void );
    void        GrowListBy          ( s32       nNodes  );
    material&   Add                 ( xhandle&  hHandle );
    material&   operator[]          ( s32       Index   );
    material&   operator()          ( xhandle   hHandle );
    void        DeleteByHandle      ( xhandle   hHandle );
    s32         GetIndexByHandle    ( xhandle   hHandle );
    xhandle     GetHandleByIndex    ( s32       Index   );
    xbool       SanityCheck         ( void ) const;
    void        Update              ( f32       DeltaTime );
    
    #ifndef X_RETAIL
    void        Dump                ( const char* pFilename );
    #endif

protected:
    struct destructor                         
    {  
        material Item; 
        inline ~destructor() {} 
    };     

    struct node_info
    {
        material    Item;
        xhandle     Handle;
    };

    friend  s32 NodeCompareFn( const void* pA, const void* pB );

    xbool       m_Sorted;       // is this guy sorted?
    s32         m_Capacity;
    s32         m_nNodes;
    node_info*  m_pNodes;       // indexes have a direct mapping into this array
    s32*        m_pIndices;     // handles have a direct mapping into this array
};

//=============================================================================

inline s32 material_array::GetCount( void ) const
{
    return m_nNodes;
}

//=============================================================================

inline material& material_array::operator[]( s32 Index )
{
    ASSERT( (Index>=0) && (Index<m_nNodes) );
    return m_pNodes[Index].Item;
}

//=============================================================================

inline material& material_array::operator()( xhandle hHandle )
{
    ASSERT( (hHandle.Handle>=0) && (hHandle.Handle<m_Capacity) );
    return operator[](m_pIndices[hHandle.Handle]);
}

//=============================================================================

inline s32 material_array::GetIndexByHandle( xhandle hHandle )
{
    ASSERT( (hHandle.Handle>=0) && (hHandle.Handle<m_Capacity) );
    return m_pIndices[hHandle.Handle];
}

//=============================================================================

inline xhandle material_array::GetHandleByIndex( s32 Index )
{
    ASSERT( (Index>=0) && (Index<m_nNodes) );
    return m_pNodes[Index].Handle;
}

//=============================================================================

#endif // MATERIALARRAY_HPP