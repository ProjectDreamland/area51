#ifdef TARGET_XBOX
#   include "xbox_private.hpp"
#endif

#include "VertexMgr.hpp"
#include "e_ScratchMem.hpp"

extern void SanityCheckTNV( void );

heap::basic vert_factory::m_Allocator;


//=============================================================================
//=============================================================================
//=============================================================================
//  XBOX Vertex Memory Manager
//=============================================================================
//=============================================================================
//=============================================================================



//=============================================================================

extern void TnvSanityCheck( void );

//=============================================================================

void vert_factory::Init( void )
{
}

//=============================================================================

void vert_factory::Kill( void )
{
}

//=============================================================================

vert_factory::handle vert_factory::GetHandleFromPointer( void* pObject )
{
    handle Handle = handle( pObject )-1;
    return Handle;
}

//=============================================================================

vert_factory::handle vert_factory::Alias( u32 nBytes,void* pData,aliasing_style AliasStyle )
{
    SanityCheckTNV();

    //  -----------------------------------------------------------------------
    //
    //  Create D3D header
    //
    handle Handle = NULL;
    switch( AliasStyle )
    {
        case ALIAS_FROM_SCRATCH:
            Handle = (handle)smem_BufferAlloc( sizeof(buffer) );
            break;

        case ALIAS_FROM_MAIN:
            Handle = new buffer;
            break;

        default:
            ASSERT(0);
            break;
    }
    Handle->buffer::buffer();
    XGSetVertexBufferHeader( 0,0,0,0,Handle,0 );

    ASSERT( pData );
	ASSERT(( Handle->Common & D3DCOMMON_INTREFCOUNT_MASK ) != D3DCOMMON_INTREFCOUNT_MASK );

    //  -----------------------------------------------------------------------
    //
    //  Register and return
    //
	Handle->Register( pData );
	Handle->m_Ptr   = ( u8* )pData;
	Handle->m_Len   = nBytes;
    Handle->m_bAlias= true;
    return Handle;
}

//=============================================================================

vert_factory::handle vert_factory::Create( const char* pResourceName,u32 nBytes,void*pRaw )
{
    ASSERT( nBytes );

    handle Handle = new buffer;
    Handle->buffer::buffer();
    XGSetVertexBufferHeader(
        0,      // Length
        0,      // Usage
        0,      // FVF
        0,      // Pool
        Handle, // pBuffer
        0       // Data
    );

    SanityCheckTNV();
    u8* Data = (u8*)m_Allocator.Alloc( pResourceName,nBytes );
    SanityCheckTNV();

	Handle->Register( Data );
	Handle->m_Len = nBytes;
	Handle->m_Ptr = Data;
    if( pRaw )
        x_memcpy( Data,pRaw,nBytes );

    return Handle;
}



//=============================================================================
//=============================================================================
//=============================================================================
//  Vertex buffers
//=============================================================================
//=============================================================================
//=============================================================================



//=============================================================================

void vert_factory::buffer::Init( void )
{
    SanityCheckTNV();

    m_BitField = 0;
    m_Len      = 0;
    m_Ptr      = 0;
}

//=============================================================================

void vert_factory::buffer::Kill( void )
{
    SanityCheckTNV();

    if( !m_bAlias )
        m_Allocator.Free( m_Ptr );

    IDirect3DResource8::Common = 0;
	IDirect3DResource8::Lock   = 0;
	IDirect3DResource8::Data   = 0;

    Init( );
}

//=============================================================================

void* vert_factory::buffer::Lock( void )
{
    SanityCheckTNV();

    m_bLock = true;
    return m_Ptr;
}

//=============================================================================

void vert_factory::buffer::Unlock( void )
{
    SanityCheckTNV();

    ASSERT( m_bLock );
    m_bLock = false;
}

//=============================================================================

void vert_factory::buffer::Window( u32 Start )
{
    SanityCheckTNV();

    XGSetVertexBufferHeader( 0,0,0,0,this,0 );
	ASSERT(( Common & D3DCOMMON_INTREFCOUNT_MASK ) != D3DCOMMON_INTREFCOUNT_MASK );
    void* pNewBase = ((u8*)m_Ptr)+Start;
	Register( pNewBase );
}

//=============================================================================

void vert_factory::buffer::Set( u32 Stream,u32 Stride )
{
    SanityCheckTNV();

    VERIFY( !g_pd3dDevice->SetStreamSource( Stream,this,Stride ));
}
