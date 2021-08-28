#ifndef _PUSHMGR_HPP_
#define _PUSHMGR_HPP_

    #include "XboxMgr.hpp"

    //========================================================================

    #define MAX_MEMORY_SIZE u32(1048576.0f*0.25f+0.5f)
    #define MAX_RECORD_SIZE u32(1048576.0f*1.25f+0.5f)
	#define MAX_PREP_SIZE   524288
	#define PB_FLOOR_SIZE   2540

    //========================================================================
    //
    //  push_factory::handle Handle = g_PushMgr.CreateBuffer( ... );
    //  {   .
    //      .
    //  }   .
    //  delete Handle;
    //

    extern struct push_factory
    {
        static heap::basic m_Allocator[2];
		
        typedef class buffer : public IDirect3DPushBuffer8
        {
			friend push_factory;

            void  Kill( void );
            void  Init( void );

        public:

            buffer( void ) { Init(); }
        ~   buffer( void ) { Kill(); }

            void  Window( u32 Start,u32 Bytes );
            void  Set   ( const char* pResourceName,void* Data,u32 Bytes );
            void  Record( void );
			void  Stop  ( void );
			void  Run   ( void );

            u32   m_Length;
			void* m_Ptr;

            union
            {
                u32 Flags;
                struct
                {
                    u32 bOwnerData:1;
                    u32 bFence    :1;
                };
            };
        }
        * handle;

		const heap::basic& GetRecordingPool( void )const
		{
			return m_Allocator[1];
		}

		const heap::basic& GetGeneralPool( void )const
		{
			return m_Allocator[0];
		}

        handle  Create( void );
        void    Init  ( void );
        void    Kill  ( void );

        push_factory();
    ~   push_factory();

        void Flush( void );

        u32 m_RunCount;
    }
    g_PushFactory;

#endif
