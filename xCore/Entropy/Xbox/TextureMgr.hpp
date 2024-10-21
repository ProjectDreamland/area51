#ifndef _TEXTUREMGR_HPP_
#define _TEXTUREMGR_HPP_

    #include "XboxMgr.hpp"

    //========================================================================
    //
    //  texture_factory::handle Handle = g_TextureFactory.CreateBuffer( ... );
    //  {   .
    //      .
    //  }   .
    //  delete Handle;
    //
    extern struct texture_factory
    {
        // Allocators: 0=General
        //             1=Tiled
        //             2=Temp
        static heap::basic m_Allocator[3];

        u32 m_Pitch;

        //=====================================================================

        enum aliasing_style
        {
            ALIAS_FROM_SCRATCH,
            ALIAS_FROM_MAIN
        };

        //=====================================================================

        /* normal palette */

        class palette: public IDirect3DPalette8
        {
            void Kill( void );
            void Init( void );

        public:

            void* GetPtr ( void ){ return m_Ptr; }
            u32   GetSize( void ){ return m_Len; }

            palette( void ) { Init(); }
        ~   palette( void ) { Kill(); }

        private:

            friend texture_factory;

            u32 m_Len;
            u8* m_Ptr;
        };

        //=====================================================================

        /* normal texture */

        typedef class buffer : public IDirect3DTexture8
        {
            void  Kill( );
            void  Init( );

        public:

            void   * GetPtr        ( void ){ return m_Ptr   ; }
            palette* GetClut       ( void ){ return m_pClut ; }
            u32      GetSize       ( void ){ return m_Len   ; }
            xbool    isLinear      ( void ){ return m_Linear; }
            xbool    isAlias       ( void ){ return m_bAlias; }
            D3DTILE* GetTilePointer( void ){ return &m_Tile ; }
            s32      GetTileId     ( void ){ return m_iTile ; }
            void     Unlock        ( void );

            void     Register      ( xbitmap& );
            void*    Lock          ( u32 iMIP );

            buffer( ) { Init(); }
        ~   buffer( ) { Kill(); }

        private:

            friend texture_factory;

            u32      m_bAlias:1;
            u32      m_Linear:1;
            s32      m_iPool;
            palette* m_pClut;
            s32      m_iTile;
            D3DTILE  m_Tile;
            u32      m_Lock;
            u32      m_Len;
            u8*      m_Ptr;
        }
        * handle;

        //=====================================================================

        /* volume texture */

        typedef class volume_buffer : public IDirect3DVolumeTexture8
        {
            void  Kill( );
            void  Init( );

        public:

            void* GetPtr  ( ){ return m_Ptr;    }
            u32   GetSize ( ){ return m_Len;    }
            xbool isLinear( ){ return m_Linear; }
            xbool isAlias ( ){ return m_bAlias; }
            void  Register( xbitmap& );
            void  Unlock  ( );
            void* Lock    ( u32 iLevel );

            volume_buffer( ) { Init(); }
        ~   volume_buffer( ) { Kill(); }

        private:

            friend texture_factory;

            u32 m_bAlias:1;
            u32 m_Linear:1;
            s32 m_iPool;
            u32 m_Lock;
            u32 m_Len;
            u8* m_Ptr;
        }
        * volume_handle;

        //=====================================================================

        /* cubic texture */

        typedef class cube_buffer : public IDirect3DCubeTexture8
        {
            void  Kill( );
            void  Init( );

        public:

            void* GetPtr  ( ){ return m_Ptr;    }
            u32   GetSize ( ){ return m_Len;    }
            xbool isLinear( ){ return m_Linear; }
            xbool isAlias ( ){ return m_bAlias; }
            void* Lock    ( u32 Face,u32 iMIP );
            void  Unlock  ( );

            cube_buffer( ) { Init(); }
        ~   cube_buffer( ) { Kill(); }

        private:

            friend texture_factory;

            u32 m_bAlias:1;
            u32 m_Linear:1;
            u32 m_Tiled:1;
            s32 m_iPool;
            u32 m_Lock;
            u32 m_Len;
            u8* m_Ptr;
        }
        * cube_handle;

    public:

        //=====================================================================

    ~   texture_factory( void ){}
        texture_factory( void ){}

        void Init( void );
        void Kill( void );

        //=====================================================================

        void    GhostGeneralIntoTemp( xbool );

        handle  CreateTiledZ( const char* pResourceName,u32 Pitch,u32 Width,u32 Height,IDirect3DSurface8*& pSurface,D3DFORMAT Format,u32 TileIndex,bool bCompressed );
        handle  CreateTiled ( const char* pResourceName,u32 Pitch,u32 Width,u32 Height,IDirect3DSurface8*& pSurface,D3DFORMAT Format,u32 TileIndex );

        handle  CreateZ     ( const char* pResourceName,u32 Pitch,u32 Width,u32 Height,IDirect3DSurface8*& pSurface,D3DFORMAT Format,s32 Pool = kPOOL_GENERAL);
        handle  Create      ( const char* pResourceName,u32 Pitch,u32 Width,u32 Height,IDirect3DSurface8*& pSurface,D3DFORMAT Format,s32 Pool = kPOOL_GENERAL);
        handle  Create      ( const char* pResourceName,u32 Pitch,u32 Width,u32 Height,D3DFORMAT,s32 Pool = kPOOL_GENERAL );
        handle  Create      ( const char* pResourceName,xbitmap&,s32 Pool = kPOOL_GENERAL );

        //=====================================================================

        handle Alias( u32 Pitch,u32 Width,u32 Height,D3DFORMAT,void*,aliasing_style );
        handle Alias( handle Handle,void* Data,aliasing_style );

        //=====================================================================

        volume_handle  CreateVolume        ( const char* pResourceName,D3DFORMAT,u32 Bpp,u32 nMips,u32 W,u32 H,u32 D,s32 Pool = kPOOL_GENERAL );
          cube_handle  CreateCubeMap       ( const char* pResourceName,D3DFORMAT,u32,u32,u32,s32 Pool = kPOOL_GENERAL );
          cube_handle  CreateCubeMap       ( const char* pResourceName,D3DFORMAT,xbitmap*,s32 Pool = kPOOL_GENERAL );
          cube_handle  CreateTiledCubeMap  ( const char* pResourceName,D3DFORMAT,u32,u32,u32,u32 );
               handle  GetHandleFromPointer( void* );

        //=====================================================================

        // index 0: = normal texture memory
        // index 1: = tiled memory

	    const heap::basic& GetGeneralPool( void )const
	    {
		    return m_Allocator[kPOOL_GENERAL];
	    }

        const heap::basic& GetTiledPool( void )const
        {
            return m_Allocator[kPOOL_TILED];
        }

    private:

        handle Alloc( u32 Size,u32 Alignment,D3DFORMAT );
    }
    g_TextureFactory;

    //========================================================================

    //! Texture object.
    /** This structure defines an optimal texture switching system.
        */

    extern struct texture_mgr
    {
        //  -----------------------------------------------------------------
        //                                                          Accessors
        //  -----------------------------------------------------------------

        void Set( u32 iStage,IDirect3DBaseTexture8 * );
        void Set( u32 iStage,texture_factory::handle );

        void Clear( u32 iStage )
        {
            Set( iStage,(IDirect3DBaseTexture8*)NULL );
        }

        IDirect3DBaseTexture8* Get( u32 Index )
        {
            return m_Stage[Index];
        }

        //  -----------------------------------------------------------------
        //                                                 Private properties
        //  -----------------------------------------------------------------

    private:

        //! Texture stages
        /** This member contains a pointer to all texture stages
            which allows us to strip redundant texture changes
            and take advantage of SwitchTexture().
            */

    	IDirect3DBaseTexture8* m_Stage[ D3DTSS_MAXSTAGES ];
    }
    g_Texture;

#endif
