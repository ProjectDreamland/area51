#include "PushMgr.hpp"

#ifdef TARGET_XBOX
#   include "xbox_private.hpp"
#endif

#include "IndexMgr.hpp"

heap::basic push_factory::m_Allocator[];


//=============================================================================
//=============================================================================
//=============================================================================
//  XBOX Push Memory Manager
//=============================================================================
//=============================================================================
//=============================================================================

static u8 s_MD5Sum[16];

//=============================================================================

void push_factory::Init( void )
{
    m_RunCount = 0;
}

//=============================================================================

void push_factory::Kill( void )
{
}

//=============================================================================

void push_factory::Flush( void )
{
    if( m_RunCount )
        g_pd3dDevice->KickPushBuffer();
    m_RunCount = 0;
}

//=============================================================================

push_factory::handle push_factory::Create( void )
{
    handle  Handle = new buffer;
    ASSERT( Handle );
    return  Handle;
}

//=============================================================================

void push_factory::buffer::Window( u32 Start,u32 nBytes )
{
    ASSERTS(!( nBytes & 3 ),"Are you using PC data?");
    ASSERTS(!( Start  & 3 ),"Bad offset" );
    ASSERT( Start+nBytes <= m_Length );
    {
        xbool bSmallPB =     ( nBytes < PB_FLOOR_SIZE );
        XGSetPushBufferHeader( nBytes,bSmallPB,this,0 );
        void* pNewBase=((u8*)m_Ptr)+Start;
        Register( pNewBase );
    }
}

//=============================================================================

void push_factory::buffer::Set( const char* pResourceName,void* pPushData,u32 nBytes )
{
    ASSERTS(!( nBytes & 3 ),"Are you using PC data?");
    {
        xbool SmallPB = nBytes < PB_FLOOR_SIZE;
        XGSetPushBufferHeader( nBytes,SmallPB,this,0 );
        m_Length = nBytes;

        // Large push buffers run via interrupt ...............................

        if( !SmallPB )
        {
            m_Ptr = g_PushFactory.m_Allocator[1].Alloc( pResourceName,nBytes );
            x_memcpy( m_Ptr,pPushData,nBytes );
            Register( m_Ptr );
            bOwnerData = 1;
            return;
        }

        // Small buffers run in Cpu space .....................................

        Common |= D3DPUSHBUFFER_RUN_USING_CPU_COPY;
        m_Ptr = pPushData;
        Data = u32(m_Ptr);
        bOwnerData = 0;
    }
}

//=============================================================================

void push_factory::buffer::Init( void )
{
    m_Length = 0;
    m_Ptr    = NULL;
    Flags    = 0;
}

//=============================================================================

void push_factory::buffer::Kill( void )
{
    if( bOwnerData )
        g_PushFactory.m_Allocator[1].Free( m_Ptr );
    Init();
}

//=============================================================================

void push_factory::buffer::Record( void )
{
    u8* pData = g_PushFactory.m_Allocator[0].m_pCur;
    u32 Bytes = g_PushFactory.m_Allocator[0].GetFree();
    Bytes = (Bytes+4095)&~4095;

    // Don't allocate yet -- just point it to the beginning of
    // a future allocation (in Stop)

    XGSetPushBufferHeader( Bytes,FALSE,this,0 );
    Register( pData );

    m_Length = Bytes;
    m_Ptr    = pData;

    VERIFY( !g_pd3dDevice->BeginPushBuffer( this ));
}

//=============================================================================

void push_factory::buffer::Stop( void )
{
    HRESULT Error = g_pd3dDevice->EndPushBuffer( );
    switch( Error )
    {
        #ifndef CONFIG_RETAIL
        case D3DERR_BUFFERTOOSMALL:
            ASSERT(0);
            break;
        #endif

        case S_OK:
        {
            void* PushBuffer = g_PushFactory.m_Allocator[0].Alloc( "Recording",Size );
            XGSetPushBufferHeader( Size,(Size < PB_FLOOR_SIZE),this,0 );
            m_Length = Size;

            // Large push buffers run via interrupt ...............................

            Register( m_Ptr );
            bOwnerData = 0;
            break;
        }

        default:
            ASSERT(0);
    }
}

//=============================================================================

void push_factory::buffer::Run( void )
{
    g_pd3dDevice->RunPushBuffer( this,0 );
    g_PushFactory.m_RunCount++;
}



//=============================================================================

push_factory::push_factory( )
{
}

//=============================================================================

push_factory::~push_factory( )
{
}
