#include "QuikHeap.h"

#ifdef X_DEBUG
#include "TextureMgr.hpp"
#include "VertexMgr.hpp"
#include "PushMgr.hpp"
#endif

#ifndef TARGET_XBOX
#error "Xbox only"
#endif

#ifndef CONFIG_RETAIL
#include <XbDm.h>
#endif

using namespace heap;

//  ---------------------------------------------------------------------------

#define NODE_DELETED_FLAG 0x80000000
#define NODE_SCRATCH_FLAG 0x00000001

#define SWITCH_RECORD_NAMES 1
#define HEADER_GROUP_SIZE   32

static xbool s_TablesAreDirty = TRUE;
block heap::g_pHeader[MAX_MEM_HEADERS];
s32   heap::g_iHeader = -1;

static u32 s_AllocationGuid = 0;

//=============================================================================

void IncrementQHGuid( void )
{
    s_AllocationGuid++;
}

//  ---------------------------------------------------------------------------

static int CmpHeaders( const void* A,const void* B )
{
    block* BlockA = (block*)A;
    block* BlockB = (block*)B;

    if( u32(BlockA->m_pData) < u32(BlockB->m_pData) ) return-1;
    if( u32(BlockA->m_pData) > u32(BlockB->m_pData) ) return 1;

    return 0;
}

//=============================================================================

static void SortTables( int (__cdecl *compare )(const void *, const void *) )
{
    if( s_TablesAreDirty )
    {
        qsort( g_pHeader,MAX_MEM_HEADERS,sizeof(block),compare );
        s_TablesAreDirty = FALSE;
    }
}

//=============================================================================

void DeleteAllQHGuid( void )
{
    for( s32 i=0;i<MAX_MEM_HEADERS;i++ )
    {
        if( g_pHeader[i].m_Guid==s_AllocationGuid )
        {
            x_memset( &g_pHeader[i],0,sizeof(block) );
        }
    }
}

//=============================================================================

static void BlueScreenOfMemoryDeath( void )
{
#ifndef CONFIG_RETAIL
    D3DCOLOR Color = D3DCOLOR_RGBA( 0,0,127,0);
    D3D__pDevice->Clear( 0,0,D3DCLEAR_TARGET,Color,0.0f,0 );
    D3D__pDevice->Present( 0,0,0,0 );
__asm int 3
__asm int 3
#else
    // Lie about the problem
    extern void OnIOErrorCallback( void );
    OnIOErrorCallback();
#endif
}

//  ---------------------------------------------------------------------------

static block* GetNextUnusedHeader( void )
{
    ASSERT( g_iHeader >-1 );
    block* Result = g_pHeader;

    s32 i;
    for( i=0;i< MAX_MEM_HEADERS;i++ )
    {
        s32 Index = (i+g_iHeader)%MAX_MEM_HEADERS;
        if( Result->m_Size & NODE_DELETED_FLAG )
            // Use deleted header
            break;
        if( !Result->m_Size )
            // Use unallocated
            break;
        Result++;
    }

    if( i != MAX_MEM_HEADERS )
    {
        g_iHeader = i;
        return Result;
    }

    BlueScreenOfMemoryDeath();
    return NULL;
}

//  ---------------------------------------------------------------------------

basic::basic( void )
{
    x_memset( this,0,sizeof(basic) );
    if( g_iHeader==-1 )
    {
        x_memset( g_pHeader,0,sizeof(g_pHeader) );
        g_iHeader = 0;
    }
}

//  ---------------------------------------------------------------------------

basic::~basic( void )
{
}

//  ---------------------------------------------------------------------------

basic::basic( u8*& pBase,u32 Alignment,u32 Bytes,alloc_direction Direction )
{
    basic();

    m_Direction = s32( Direction );
    m_Alignment = ((Alignment+15) & ~15)-1;
    m_TotalSize = (Bytes+m_Alignment) & ~m_Alignment;

    // Allocate/protect memory space ******************************************

    u8* Result = pBase;
    if( !pBase )
    {
        Result = (u8*)XPhysicalAlloc(
            Bytes,
            MAXULONG_PTR,
            Alignment,
            PAGE_READWRITE
        );
        if( !Result )
        {
            D3D__pDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA( 0,0,127,0),0.0f,0 );
            D3D__pDevice->Present( 0,0,0,0 );
            static DWORD E = GetLastError( );
        __asm int 3
        __asm int 3
        }
    }
    else
    {
        pBase += m_TotalSize;
    }

    // Initialise the object **************************************************

    switch( Direction )
    {
        case DIR_FORWARD:
            m_pBase = ((u8*)Result);
            m_pEnd  = ((u8*)m_pBase) + m_TotalSize;
            m_pCur  = ((u8*)((u32(m_pBase) + m_Alignment) & ~m_Alignment));
            ASSERT( m_pEnd > m_pBase );
            break;

        case DIR_REVERSE:
            m_pBase = ((u8*)Result);
            m_pCur  = ((u8*)m_pBase);
            m_pEnd  = ((u8*)m_pBase) - m_TotalSize;
            ASSERT( m_pEnd < m_pBase );
            break;

        default:
            ASSERT(0);
            break;
    }
}

//  ---------------------------------------------------------------------------

extern void x_BeginAtomic( void );
extern void x_EndAtomic  ( void );

//=============================================================================

static const char* GetNameFor( basic* pThis )
{
#ifdef X_DEBUG
    if( pThis==&g_TextureFactory.GetGeneralPool() ) return "textures";
    if( pThis==&   g_VertFactory.GetGeneralPool() ) return "vertices";
    if( pThis==&g_TextureFactory.  GetTiledPool() ) return "tiled   ";
    if( pThis==&   g_PushFactory.GetGeneralPool() ) return "push    ";
#endif
    return "UNKNOWN";
}

//=============================================================================

void* basic::Alloc( const char* pResourceName,u32 Bytes )
{
    void* pResult = NULL;

    x_BeginAtomic();
    {
        //  -------------------------------------------------------------------
        //
        //  Allocate from free space
        //
        //  This works by casting a block object at the top of contiguous free
        //  space, then jumping to the given aligment following it. Adding
        //  the number of requested bytes of allocation frames it.
        //
        block* pNode;
        {
            // Determine high or low allocation ...........................

            s32 Leng = (Bytes + m_Alignment) & ~m_Alignment;
            u8* Ends = m_pCur + Leng * m_Direction;

            xbool bOk;
            {
                pNode = GetNextUnusedHeader();
                s_TablesAreDirty = TRUE;
                pNode->block::block();

                switch( m_Direction )
                {
                    case DIR_FORWARD:
                        bOk            = (Ends < m_pEnd );
                        pNode->m_pData = m_pCur;
                        pNode->m_Size  = Leng;
                        m_pCur         = Ends;
                        break;

                    case DIR_REVERSE:
                        bOk            = (Ends > m_pEnd );
                        pNode->m_pData = Ends;
                        pNode->m_Size  = Leng;
                        m_pCur         = Ends;
                        break;

                    default:
                        ASSERT(0);
                        break;
                }

                // Handle allocation failure ..................................

            #ifndef CONFIG_RETAIL
                pNode->m_pResource = NULL;
            #endif
                if( !bOk )
                {
                    SortTables( CmpHeaders );
                    // Ok, even in retail we die!
                    BlueScreenOfMemoryDeath();
                }

                // Update and track names .....................................

                m_Allocated  += pNode->m_Size;
                pNode->m_Guid = s_AllocationGuid;

                if( pResourceName )
                {
                #ifndef CONFIG_RETAIL
                    s32 Size = x_strlen(pResourceName)+10;
                    char* Str = (char*)DmAllocatePool( Size+1 );
                    x_sprintf( Str,"%08X: %s",s_AllocationGuid,pResourceName );
                    pNode->m_pResource = Str;
                    strlwr( Str );
                #endif
                }
            }
        }

        // Quick sanity check on totals ---------------------------------------

        pResult = pNode->m_pData;
    }
    x_EndAtomic();

    return pResult;
}

//=============================================================================

static block* GetHeader( u8* pData  )
{
    block* Result = NULL;
    {
        s32 high,i,low;
        // Binary search for requested data
        for( low=-1,high=MAX_MEM_HEADERS;high-low>1; )
        {
            i = (high+low)/2;
            if( pData <= g_pHeader[i].m_pData )
                high = i;
            else
                low  = i;
        }
        if( pData == g_pHeader[high].m_pData )
            Result = g_pHeader+high;
    }

    g_iHeader = s32(Result-g_pHeader);

    ASSERT( Result );
    return( Result );
}

//=============================================================================

static xbool IsFreeable( block* pHeader,u8* Ptr,s32 Direction )
{
    if( !(pHeader->m_Size & NODE_DELETED_FLAG) )
    {
        return FALSE;
    }

    u32 Size = pHeader->m_Size & ~NODE_DELETED_FLAG;
    s32 Reclaimed;

    switch( Direction )
    {
        case DIR_FORWARD:
            Reclaimed = Size;
            break;

        case DIR_REVERSE:
            Reclaimed = 0;
            break;

        default:
            ASSERT(0);
            break;
    }

    return( u32(pHeader->m_pData)+Reclaimed == u32(Ptr) );
}

//=============================================================================

void basic::Free( void* pBytes )
{
    x_BeginAtomic();
    {
        if( pBytes )
        {
            SortTables( CmpHeaders );
            block* pHeader = GetHeader( (u8*)pBytes );
            if( pHeader )
            {
                // Mark header for deletion (but don't do it yet)
                pHeader->m_Size |= NODE_DELETED_FLAG;

                // It's fine to assume contiguous headers.
                while( IsFreeable( pHeader,m_pCur,m_Direction ) )
                {
                    // Free up debug name
                    #ifndef CONFIG_RETAIL
                    if( pHeader->m_pResource )
                        DmFreePool( (PVOID)pHeader->m_pResource );
                    pHeader->m_pResource = NULL;
                    #endif

                    // Reclaim memory
                    u32 Size = pHeader->m_Size & ~NODE_DELETED_FLAG;
                    s32 Reclaimed = Size * m_Direction;

                    m_pCur      -= Reclaimed;
                    m_Allocated -= Reclaimed;
                    pHeader->m_pData = 0;
                    pHeader->m_Size  = 0;
                    pHeader--;

                    // Mark as dirty for next free session
                    s_TablesAreDirty = TRUE;
                }
            }
            else
            {
                BlueScreenOfMemoryDeath(); // Help, help!
            }
        }
    }
    x_EndAtomic();
}
