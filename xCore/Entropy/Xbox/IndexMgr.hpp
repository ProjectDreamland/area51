#ifndef _INDEXMGR_HPP_
#define _INDEXMGR_HPP_

    #include "XboxMgr.hpp"

    //========================================================================

    struct index_factory : public singleton_t< index_factory >
    {
        //  ------------------------------------------------------------------
        //
        //  This class is used to track individual index buffers.
        //
        typedef class buffer : public IDirect3DIndexBuffer8
        {
            void  Kill( );
            void  Init( );

        public:

            void  Unlock( );
            void* Lock  ( );

            buffer( ) { Init(); }
        ~   buffer( ) { Kill(); }

            u32 m_Len;
            u32 m_Lock;
            u8* m_Ptr;
        }
        * handle;

        //  ------------------------------------------------------------------
        //
        //  This routine creates an index buffer.
        //
        handle Create( u32 nBytes, u32 Format, void* pBytes = NULL );

        //  ------------------------------------------------------------------
        //
        //  Construction
        //
        index_factory();
    ~   index_factory();
    };

    //========================================================================

    #ifndef g_IndexFactory
    #define g_IndexFactory (*singleton_t<index_factory>::me)
    #endif

#endif
