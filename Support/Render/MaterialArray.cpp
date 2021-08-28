//=============================================================================
// Includes
//=============================================================================

#include "Render.hpp"

#define RENDER_PRIVATE
#include "MaterialArray.hpp"
#undef RENDER_PRIVATE

//=============================================================================
// Implementation
//=============================================================================

material_array::material_array( void ) :
    m_Sorted    ( FALSE ),
    m_Capacity  ( FALSE ),
    m_nNodes    ( 0     ),
    m_pNodes    ( NULL  ),
    m_pIndices  ( NULL  )
{
}

//=============================================================================

material_array::~material_array( void )
{
    Clear();
    if ( m_pNodes )
        x_free( m_pNodes );
}

//=============================================================================

static
s32 GetMaterialPriority( material_type Type )
{
    static const s8 kMatPriorities[Material_NumTypes] =
    {
        1,  // Material_Not_Used
        2,  // Material_Diff
        5,  // Material_Alpha
        2,  // Material_Diff_PerPixelEnv
        3,  // Material_Diff_PerPixelIllum
        5,  // Material_Alpha_PerPolyEnv
        4,  // Material_Alpha_PerPixelIllum
        4,  // Material_Alpha_PerPolyIllum
        6,  // Material_Distortion
        6,  // Material_Distortion_PerPolyEnv
    };

    return kMatPriorities[Type];
}

//=============================================================================

s32 NodeCompareFn( const void* pA, const void* pB )
{
    material_array::node_info* pNodeA = (material_array::node_info*)pA;
    material_array::node_info* pNodeB = (material_array::node_info*)pB;
    material* pMatA = &pNodeA->Item;
    material* pMatB = &pNodeB->Item;
    material_type TypeA = (material_type)pMatA->m_Type;
    material_type TypeB = (material_type)pMatB->m_Type;

    if ( GetMaterialPriority(TypeA)           > GetMaterialPriority(TypeB)           ) return  1;
    if ( GetMaterialPriority(TypeA)           < GetMaterialPriority(TypeB)           ) return -1;
    if ( pMatA->m_DiffuseMap.GetPointer()     > pMatB->m_DiffuseMap.GetPointer()     ) return  1;
    if ( pMatA->m_DiffuseMap.GetPointer()     < pMatB->m_DiffuseMap.GetPointer()     ) return -1;
    if ( pMatA->m_EnvironmentMap.GetPointer() > pMatB->m_EnvironmentMap.GetPointer() ) return  1;
    if ( pMatA->m_EnvironmentMap.GetPointer() < pMatB->m_EnvironmentMap.GetPointer() ) return -1;
    if ( pMatA->m_DetailMap.GetPointer()      > pMatB->m_DetailMap.GetPointer()      ) return  1;
    if ( pMatA->m_DetailMap.GetPointer()      < pMatB->m_DetailMap.GetPointer()      ) return -1;

    return 0;
}

//=============================================================================

void material_array::Sort( void )
{
    if ( m_Sorted )
        return;

    if ( m_nNodes )
    {
        // sort the materials
        x_qsort( m_pNodes, m_nNodes, sizeof(node_info), NodeCompareFn );
    
        // remap the material indices
        for ( s32 i = 0; i < m_nNodes; i++ )
        {
            ASSERT( (m_pNodes[i].Handle.Handle>=0) &&
                    (m_pNodes[i].Handle.Handle<m_Capacity) );
            m_pIndices[m_pNodes[i].Handle] = i;
        }
    }

    // sanity check
    if ( 0 )
    {
        for ( s32 iNode = 0; iNode < m_nNodes-1; iNode++ )
        {
            ASSERT( NodeCompareFn(&m_pNodes[iNode+1], &m_pNodes[iNode]) >= 0 );
        }
    }

    m_Sorted = TRUE;
}

//=============================================================================

void material_array::Clear( void )
{
    s32 i;

    // destruct all of the objects
    for ( i = 0; i < m_nNodes; i++ )
        ((destructor*)&m_pNodes[i].Item)->~destructor();

    // clear all of the handle indices
    for ( i = 0; i < m_Capacity; i++ )
        m_pIndices[i] = -1;

    m_nNodes = 0;
    m_Sorted = FALSE;
}

//=============================================================================

void material_array::GrowListBy( s32 nNodes )
{
    ASSERT( nNodes > 0 );

    // increase the capacity
    s32 NewCapacity = m_Capacity + nNodes;

    // allocate the new arrays
    node_info* pNewNodes = (node_info*)x_malloc( (sizeof(node_info) +
                                                  sizeof(s32)) * NewCapacity );
    ASSERT( pNewNodes );
    
    s32* pNewIndices   = (s32*)(pNewNodes + NewCapacity);
    ASSERT( pNewIndices );

    // copy all the previous nodes to the new arrays
    if ( m_nNodes )
    {
        ASSERT( m_pNodes );
        x_memcpy( pNewNodes, m_pNodes, sizeof(node_info)*m_nNodes );
    }
    if ( m_Capacity )
    {
        ASSERT( m_pIndices );
        x_memcpy( pNewIndices, m_pIndices, sizeof(s32)*m_Capacity );
    }

    // set the new indices to -1
    for ( s32 i = m_Capacity; i < NewCapacity; i++ )
    {
        pNewIndices[i] = -1;
    }

    // update the arrays
    if ( m_pNodes )
        x_free( m_pNodes );
    m_pNodes   = pNewNodes;
    m_pIndices = pNewIndices;
    m_Capacity = NewCapacity;
}

//=============================================================================

material& material_array::Add( xhandle& hHandle )
{
    // do we need to grow?
    if ( m_nNodes >= m_Capacity )
        GrowListBy( MAX( m_Capacity/2, 100 ) );

    // find the first available index
    s32 iHandle;
    for ( iHandle = 0; iHandle < m_Capacity; iHandle++ )
    {
        if ( m_pIndices[iHandle] == -1 )
            break;
    }
    ASSERT( iHandle != m_Capacity );

    // fill in the index information
    m_pIndices[iHandle] = m_nNodes;
    hHandle.Handle      = iHandle;

    // construct the node info
    xConstruct( &m_pNodes[m_nNodes].Item );
    m_pNodes[m_nNodes].Handle.Handle = iHandle;
    m_nNodes++;

    // we'll need to re-sort
    m_Sorted = FALSE;

    return m_pNodes[m_pIndices[iHandle]].Item;
}

//=============================================================================

void material_array::DeleteByHandle( xhandle hHandle )
{
    s32 Index = GetIndexByHandle(hHandle);
    ASSERT( (Index>=0) && (Index<m_nNodes) );

    // call destructor
    ((destructor*)&m_pNodes[Index].Item)->~destructor();

    // clear out the index
    m_pIndices[hHandle.Handle] = -1;

    // copy the last node into the deleted node
    if ( Index != (m_nNodes-1) )
    {
        m_pNodes[Index].Item   = m_pNodes[m_nNodes-1].Item;
        m_pNodes[Index].Handle = m_pNodes[m_nNodes-1].Handle;
        m_pIndices[m_pNodes[Index].Handle.Handle] = Index;
    }
    m_nNodes--;

    // flag that we'll need to sort again
    m_Sorted = FALSE;
}

//=============================================================================

xbool material_array::SanityCheck( void ) const
{
    s32 i;

    for ( i = 0; i < m_nNodes; i++ )
    {
        ASSERT( m_pIndices[m_pNodes[i].Handle.Handle] == i );
    }

    for ( i = 0; i < m_Capacity; i++ )
    {
        if ( m_pIndices[i] != -1 )
        {
            ASSERT( m_pNodes[m_pIndices[i]].Handle.Handle == i );
        }
    }

    return TRUE;
}

//=============================================================================

#ifndef X_RETAIL
void material_array::Dump( const char* pFilename )
{
    (void)pFilename;
    // DS 5/28/04 - this needs to be redone with the new material type
    // (we no longer use float params)
    /*
    X_FILE* fh = x_fopen( pFilename, "wt" );

    s32 i;
    for ( i = 0; i < GetCount(); i++ )
    {
        material& Mat = operator[]( i );

        switch( Mat.m_Type )
        {
        default:
            ASSERT( FALSE );
            break;
        case Material_Diff:
            x_fprintf( fh, "Material_Diff: " );
            x_fprintf( fh, "Diff(%s), ",         Mat.m_DiffuseMap.GetName() );
            x_fprintf( fh, "Detail(%s), ",       Mat.m_DetailMap.GetName() );
            x_fprintf( fh, "Punch(%d), ",        !!(Mat.m_Flags & geom::material::FLAG_IS_PUNCH_THRU) );
            x_fprintf( fh, "DetScale(%3.2f)\n",  Mat.m_DetailScale );
            break;
        case Material_Alpha:
            x_fprintf( fh, "Material_Alpha: " );
            x_fprintf( fh, "Diff(%s), ",         Mat.m_DiffuseMap.GetName() );
            x_fprintf( fh, "Detail(%s), ",       Mat.m_DetailMap.GetName() );
            x_fprintf( fh, "Punch(%d), ",        !!(Mat.m_Flags & geom::material::FLAG_IS_PUNCH_THRU) );
            x_fprintf( fh, "DetScale(%3.2f), ",  Mat.m_DetailScale );
            x_fprintf( fh, "BlendMode(%3.2f), ", Mat.m_Params[material::ENV_BLEND] );
            x_fprintf( fh, "ZFill(%3.3f)\n",     Mat.m_Params[material::FORCE_ZFILL] );
            break;
        case Material_Diff_PerPixelEnv:
            x_fprintf( fh, "Material_Diff_PerPixelEnv: " );
            x_fprintf( fh, "Diff(%s), ",         Mat.m_DiffuseMap.GetName() );
            x_fprintf( fh, "Env(%s), ",          Mat.m_EnvironmentMap.GetName() );
            x_fprintf( fh, "Detail(%s), ",       Mat.m_DetailMap.GetName() );
            x_fprintf( fh, "Punch(%d), ",        !!(Mat.m_Flags & geom::material::FLAG_IS_PUNCH_THRU) );
            x_fprintf( fh, "DetScale(%3.2f), ",  Mat.m_DetailScale );
            x_fprintf( fh, "Mapping(%3.2f), ",   Mat.m_Params[material::ENV_TYPE] );
            x_fprintf( fh, "BlendMode(%3.2f)\n", Mat.m_Params[material::ENV_BLEND] );
            break;
        case Material_Diff_PerPixelIllum:
            x_fprintf( fh, "Material_Diff_PerPixelIllum: " );
            x_fprintf( fh, "Diff(%s), ",         Mat.m_DiffuseMap.GetName() );
            x_fprintf( fh, "Detail(%s), ",       Mat.m_DetailMap.GetName() );
            x_fprintf( fh, "Punch(%d), ",        !!(Mat.m_Flags & geom::material::FLAG_IS_PUNCH_THRU) );
            x_fprintf( fh, "DetScale(%3.2f), ",  Mat.m_DetailScale );
            x_fprintf( fh, "DiffLight(%3.2f)\n", Mat.m_Params[material::USE_DIFFUSE] );
            break;
        case Material_Alpha_PerPolyEnv:
            x_fprintf( fh, "Material_Alpha_PerPolyEnv: " );
            x_fprintf( fh, "Diff(%s), ",         Mat.m_DiffuseMap.GetName() );
            x_fprintf( fh, "Env(%s), ",          Mat.m_EnvironmentMap.GetName() );
            x_fprintf( fh, "Detail(%s), ",       Mat.m_DetailMap.GetName() );
            x_fprintf( fh, "Punch(%d), ",        !!(Mat.m_Flags & geom::material::FLAG_IS_PUNCH_THRU) );
            x_fprintf( fh, "DetScale(%3.2f), ",  Mat.m_DetailScale );
            x_fprintf( fh, "Mapping(%3.2f), ",   Mat.m_Params[material::ENV_TYPE] );
            x_fprintf( fh, "BlendMode(%3.2f), ", Mat.m_Params[material::ENV_BLEND] );
            x_fprintf( fh, "Intensity(%3.2f), ", Mat.m_Params[material::FIXED_ALPHA] );
            x_fprintf( fh, "ZFill(%3.3f)\n",     Mat.m_Params[material::FORCE_ZFILL] );
            break;
        case Material_Alpha_PerPixelIllum:
            x_fprintf( fh, "Material_Alpha_PerPixelIllum: \n" );
            break;
        case Material_Alpha_PerPolyIllum:
            x_fprintf( fh, "Material_Alpha_PerPolyIllum: \n" );
            break;
        case Material_Distortion:
            x_fprintf( fh, "Material_Distortion\n: " );
            break;
        case Material_Distortion_PerPolyEnv:
            x_fprintf( fh, "Material_Distortion_PerPolyEnv: \n" );
            break;
        }
    }

    x_fclose( fh );
    */
}
#endif

//=============================================================================

void material_array::Update( f32 DeltaTime )
{
    for( s32 i = 0; i < GetCount(); i++ )
    {
        material&         Mat    = operator[](i);
        material::uvanim& UVAnim = Mat.m_UVAnim;
        if( UVAnim.nFrames <= 1 )
        {
            continue;
        }

        // Artists duplicate the first and last frame to be consistent
        // with the sketal animations (this is to make blending and
        // looped anims work). We really don't want to do this for
        // uv anims, so consider nFrames to be one smaller.
        s32 nFrames = UVAnim.nFrames - 1;

        // calculate the next frame
        UVAnim.CurrentFrame += DeltaTime * UVAnim.FPS * UVAnim.Dir;

        // determine the type of animation
        switch( UVAnim.Type )
        {
        case geom::material::uvanim::FIXED:
            UVAnim.Dir = 0;
            UVAnim.CurrentFrame = (f32)UVAnim.StartFrame;
            break;
        case geom::material::uvanim::LOOPED:
            UVAnim.Dir = 1;
            if( UVAnim.CurrentFrame >= (f32)nFrames )
            {
                UVAnim.CurrentFrame = (f32)UVAnim.StartFrame;
            }
            break;
        case geom::material::uvanim::PINGPONG:
            if( UVAnim.Dir > 0 )
            {
                UVAnim.Dir = 1;
                if( UVAnim.CurrentFrame >= (f32)nFrames )
                {
                    UVAnim.CurrentFrame = (f32)(nFrames -1);
                    UVAnim.Dir = -UVAnim.Dir;
                }
            }
            else
            {
                UVAnim.Dir = -1;
                if( UVAnim.CurrentFrame <= 0.0f )
                {
                    UVAnim.CurrentFrame = 0.0f;
                    UVAnim.Dir = -UVAnim.Dir;
                }
            }
            break;
        case geom::material::uvanim::ONESHOT:
            UVAnim.Dir = 1;
            if( UVAnim.CurrentFrame >= (f32)nFrames )
            {
                UVAnim.CurrentFrame = (f32)(nFrames - 1);
                UVAnim.Dir          = 0;
            }
            break;
        }

        UVAnim.iFrame = (s8)x_floor( UVAnim.CurrentFrame );
    }
}

//=============================================================================

// EOF