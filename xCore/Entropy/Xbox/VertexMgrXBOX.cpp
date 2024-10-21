#include "VertexMgr.hpp"

#ifndef X_DEBUG
#define D3DCOMPILE_PUREDEVICE 1
#endif
#include<xtl.h>
#include<xgraphics.h>

#define MAX_VERTEX_SIZE u32( 1048576*11 )

//==============================================================================
//==============================================================================
//==============================================================================
//  XBOX Vertex Memory Manager
//==============================================================================
//==============================================================================
//==============================================================================

void vert_factory::Init( void )
{
    m_pOs = XPhysicalAlloc
    (
        MAX_VERTEX_SIZE,
        MAXULONG_PTR,
        16,
        PAGE_READWRITE // cannot allocate large amounts of write combined ram
    );
    ASSERT( m_pOs );
    x_memset( &m_Heap,0,sizeof( m_Heap ));
    m_Heap.quik_heap::quik_heap( m_pOs,MAX_VERTEX_SIZE );
    XPhysicalProtect
    (
        m_pOs,
        MAX_VERTEX_SIZE,
        PAGE_READWRITE|PAGE_WRITECOMBINE|PAGE_NOCACHE
    );
}

//=========================================================================

void vert_factory::Kill()
{
    ASSERT( m_pOs );
    XPhysicalFree( m_pOs );
    m_Heap.quik_heap::~quik_heap( );
    m_pOs=NULL;
}

//=========================================================================

vert_factory::handle vert_factory::Create( u32 nBytes,void*pRaw )
{
	CONTEXT( "vert_factory::Create" );
	//
    //  Allocates header
    //
    handle Handle = new buffer;
    ASSERT( Handle );

    XGSetVertexBufferHeader( 0,0,0,0,Handle,0 );
	ASSERT(( Handle->Common & D3DCOMMON_INTREFCOUNT_MASK ) != D3DCOMMON_INTREFCOUNT_MASK );
    //
    //  Allocate vertex buffer
    //
	u8* pVerts;
	{
		CONTEXT( "vert_factory::Create -- m_Heap.Alloc" );

		pVerts = (u8*)m_Heap.Alloc( nBytes );
	#ifdef bhapgood
		if( !pVerts )
		{
			x_DebugMsg( "\nOut of Vertex RAM: %d bytes requested\n", nBytes );
		BREAK
		}
	#else
		ASSERT( pVerts );
	#endif
	}
    //
    //  Register and return
    //
	{	CONTEXT( "Register RAM with D3D" );
	//  Handle->Data = DWORD( pVerts ) & 0xfffffff;
		Handle->Register( pVerts );
		Handle->m_Ptr = pVerts;
		Handle->m_Len = nBytes;
	}
    if( pRaw )
    {
		CONTEXT( "Copying raw verts to new vertex buffer" );

		Handle->Lock( );
        x_memcpy( pVerts,pRaw,nBytes );
        Handle->Unlock( );
    }
    //
    //  Return handle
    //
    return Handle;
}

//=========================================================================

void vert_factory::buffer::Init( void )
{
    m_Lock   = 0;
    m_Len    = 0;
    m_Ptr    = 0;
}

//=========================================================================

void vert_factory::buffer::Kill( void )
{
	BlockUntilNotBusy( );

	g_VertFactory.m_Heap.Free( m_Ptr );

	IDirect3DResource8::Common = 0;
	IDirect3DResource8::Lock = 0;
	IDirect3DResource8::Data = 0;
}

//=========================================================================

void* vert_factory::buffer::Lock( void )
{
    m_Lock ++;
    return m_Ptr;
}

//=========================================================================

void vert_factory::buffer::Unlock( void )
{
    ASSERT( m_Lock>0 );
    m_Lock--;
}
