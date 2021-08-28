#ifndef _VERTEXMGR_HPP_
#define _VERTEXMGR_HPP_

    #include "XboxMgr.hpp"

    #ifndef TARGET_XBOX
    # error TARGET_XBOX required
    # endif

    extern struct vert_factory
    {
        //==-------------------------------------------------------------------

        enum aliasing_style
        {
            ALIAS_FROM_SCRATCH,
            ALIAS_FROM_MAIN
        };

        //==-------------------------------------------------------------------

        typedef struct buffer : public IDirect3DVertexBuffer8
        {
            void  Set    ( u32 Stream,u32 Stride );
            void  Window ( u32 Start );
            void  Kill   ( void );
            void  Init   ( void );
            void  Unlock ( void );
            void* Lock   ( void );

            buffer       ( void );
        ~   buffer       ( void );

            u32 m_Len;
            u8* m_Ptr;

            union
            {
                u32 m_BitField;
                struct
                {
                    u32 m_bAlias:1;
                    u32 m_bLock :1;
                };
            };
        }
        * handle;

        //==-------------------------------------------------------------------

        handle  GetHandleFromPointer ( void*                 );
        handle  Create               ( const char* pResourceName,u32 nBytes,void*pRaw  );
        handle  Alias                ( u32 nBytes,void*pRaw, aliasing_style Aliases );
        void*   High                 ( u32 nBytes ){ return m_Allocator.High( nBytes ); }
        void    Init                 ( void       );
        void    Kill                 ( void       );

		const heap::basic& GetGeneralPool( void )const
		{
			return m_Allocator;
		}

        vert_factory ( void ){ }
    ~   vert_factory ( void ){ }

        //==-------------------------------------------------------------------

        static heap::basic m_Allocator;
    }
    g_VertFactory;

    //========================================================================

    inline vert_factory::buffer:: buffer( ) { Init(); }
    inline vert_factory::buffer::~buffer( ) { Kill(); }

#endif
