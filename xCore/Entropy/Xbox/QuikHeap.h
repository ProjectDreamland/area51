#ifndef _QUIKHEAP_H_
#define _QUIKHEAP_H_

#ifndef TARGET_XBOX
#   error This is not for this target platform. Check dependancy rules.
#else
#   include "x_files.hpp"
#   define D3DCOMPILE_PUREDEVICE 1
#   include<xtl.h>
#   include<xgraphics.h>
#endif

#define MAX_MEM_HEADERS 8192 // 128K

namespace heap
{
    ///////////////////////////////////////////////////////////////////////////

    void IncrementQHGuid( void );

    struct block
    {
        void*   m_pData;
        u32     m_Guid;
        u32     m_Size;

        #ifndef CONFIG_RETAIL
        const char* m_pResource;
        #endif
    };

    extern block g_pHeader[MAX_MEM_HEADERS];
    extern s32   g_iHeader;

    block* GetFreeHeader();

    ///////////////////////////////////////////////////////////////////////////

    enum alloc_direction
    {
        DIR_FORWARD = 1,
        DIR_REVERSE =-1
    };

    ///////////////////////////////////////////////////////////////////////////

    struct basic
    {
        //  -------------------------------------------------------------------

        basic( u8*& pBase,u32 Alignment,u32 Bytes,alloc_direction Direction = DIR_FORWARD );
    ~   basic( void );
        basic( void );

        //  -------------------------------------------------------------------

        u32   GetUsed    ( void )const{ return m_Allocated; }
        u32   GetFree    ( void )const{ return m_TotalSize-m_Allocated; }
        u32   GetBase    ( void )const{ return u32( m_pBase ); }
        u32   GetSize    ( void )const{ return m_TotalSize; }

        void* Alloc      ( const char* pResourceName,u32 Bytes );
        void* High       ( u32 Bytes ){ return(( u8* )m_pEnd)-Bytes; }
        void  Free       ( void* );

        //  -------------------------------------------------------------------

    private:

        s32 m_Direction;
        u32 m_Alignment;
        u32 m_Allocated;
        u32 m_TotalSize;

    public:

        u8* m_pBase;
        u8* m_pEnd;
        u8* m_pCur;
    };
}

#endif
