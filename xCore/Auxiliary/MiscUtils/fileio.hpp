
#ifndef FILEIO_HPP
#define FILEIO_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "mem_stream.hpp"
#include "x_files.hpp"

//=========================================================================
// FILE SYSTEM
//=========================================================================
class fileio
{
public:

    struct ref
    {
        s32     OffSet;         // Byte offset where the pointer lives
        s32     Count;          // Count of entries that this pointer is pointing to
        s32     PointingAT;     // What part of the file is this pointer pointing to
        u32     Flags;          // Flags for the miscelaneous stuff
    };

    struct resolve
    {
        s32     nPointers;
        ref*    pTable;
        byte*   pStatic;
        byte*   pDynamic;
    };

    template< class T >
    void Save( const char* pFileName, T& Object, xbool bEndianSwap )  
    {
        // Open the file to make sure we are okay
        X_FILE* Fp = x_fopen( pFileName, "wb" );
        if( Fp == NULL )
            x_throw( "Unable to save file");

        // Call the actual save
        Save( Fp, Object, bEndianSwap );

        x_fclose( Fp );
    }


    template< class T >
    void Save( X_FILE* Fp, T& Object, xbool bEndianSwap )  
    {
        ASSERT( Fp );
        m_Fp = Fp;

        // Allocate the writting structure
        m_pWrite = new writting;
        ASSERT( m_pWrite );

        // Initialize class members
        m_bStatic   = TRUE;
        m_ClassPos  = 0;
        m_pClass    = (byte*)((void*)&Object);
        m_ClassSize = sizeof(Object);

        // Save the initial class
        GetW().Preallocate( m_ClassSize, TRUE );

        // Start the saving 
        Object.FileIO( *this );

        // Save the file
        SaveFile();

        // clean up
        delete m_pWrite;
        m_pWrite = NULL;
        
        (void)bEndianSwap;
    }

    inline
    resolve* PreLoad( X_FILE* Fp )  
    {
        resolve*     pResolve = NULL;
        file         File;

        //
        // Check signature
        //
        s32 i;

        i = x_fread( &File, 1, sizeof(File), Fp );
/*
        x_DebugMsg("Handle:%d, File Length:%d, Requested:%d, Returned:%d\n",Fp, x_flength(Fp), sizeof(File), i);


        const byte* ptr = (byte*)&File;
        for (i = 0 ; i < 32 ; i+=4)
        {
            x_DebugMsg("%02x:%02x:%02x:%02x\n",*ptr++,*ptr++,*ptr++,*ptr++);
        }
*/
        if( File.Signature != 0x56656e49 )
            x_throw( "Unkown file" );
        
        //
        // Read all the static data
        //
        byte* pData    = new byte[ File.nStatic ];
        if( pData == NULL )
            x_throw( "Out of meory");
        x_fread( pData, 1, File.nStatic, Fp );

        //
        // Read all the dynamic data
        //
        byte* pDynamic = new byte[ File.nDynamic + sizeof(resolve) ];
        if( pDynamic == NULL )
        {
            delete[]pData;
            x_throw( "Out of meory");
        }
        x_fread( pDynamic, 1, File.nDynamic, Fp );

        //
        // Fillup the resolved table
        //
        pResolve            = (resolve*)&pDynamic[ File.nDynamic ];
        pResolve->nPointers = File.nTable;
        pResolve->pTable    = (ref*)&pData[ File.nStatic - (File.nTable * sizeof(ref)) ];
        pResolve->pStatic    = pData;
        pResolve->pDynamic  = pDynamic;

        return pResolve;
    }

    template< class T > inline
    void Resolved( resolve* pResolve, T*& pObject )  
    {
        s32 i;
        // Fix up all the pointers
        for( i=0; i<pResolve->nPointers; i++ )
        {
            const ref& Ref = pResolve->pTable[i];
            switch( Ref.Flags )
            {
            case 3:
                *((void**)&pResolve->pStatic[ Ref.OffSet ])    = &pResolve->pStatic[ Ref.PointingAT ];
                break;
            case 0:
                *((void**)&pResolve->pDynamic[ Ref.OffSet ])   = &pResolve->pDynamic[ Ref.PointingAT ];
                break;
            case 1:
                *((void**)&pResolve->pStatic[ Ref.OffSet ])    = &pResolve->pDynamic[ Ref.PointingAT ];
                break;
            };
        }

        // Initialize all
        pObject = xConstruct( (T*)pResolve->pStatic, *this);

        // Free the dynamic memory
        delete[]pResolve->pDynamic;
    }

    template< class T > inline
    void Load( X_FILE* Fp, T*& pObject )  
    {
        resolve* pResolve = PreLoad( Fp );
        Resolved( pResolve, pObject );
    }

    template< class T > inline
    void Load( const char* pFileName, T*& pObject )  
    {
        // Open the file
        X_FILE* Fp = x_fopen( pFileName, "rb" );
        if( Fp == NULL ) 
            x_throw( xfs("Unable to open file %s", pFileName) );

        // Load the object
        Load( Fp, pObject );

        // Close the file
        x_fclose( Fp );
        Fp = NULL;
    }

    template< class T >
    void Static( T& A )                     
    { 
        byte* pA = (byte*)((void*)&A);
        ASSERTS( IsLocalVariable( pA ), "The variable must be a member of the class" );

        // Make sure that we are at the right offset
        GetW().SeekPos( m_ClassPos + ComputeLocalOffset( pA ) );

        // IF YOU GET AN ERROR HERE WHILE LINKING CHANCES ARE YOU ARE TRYING TO PASS
        // A POINTER WHEN YOU SHOULD USE --- Static( pPtr, Count ) ---
        Handle( A );  
    }

    template< class T >
    void StaticEnum( T& A )          
    { 
        if( sizeof(A) == 1 ) Static( *((u8*)&A)  );
        if( sizeof(A) == 2 ) Static( *((u16*)&A) );
        if( sizeof(A) == 4 ) Static( *((u32*)&A) );
        if( sizeof(A) > 4  ) ASSERT( 0 );
    }

    template< class T >
    void Static( T& A, s32 Count )          { Array( A, Count, TRUE );  }

    template< class T >
    void Dynamic( T*& A, s32 Count = 1 )    { Array( A, Count, FALSE ); }

    
    //=====================================================================    
    // PRIVATE
    //=====================================================================    
protected:

    struct writting
    {
        mem_stream  Static;
        mem_stream  Table;
        mem_stream  Dynamic;
    };

    struct file
    {
        u32 Signature;
        s32 Version;
        s32 nStatic;
        s32 nTable;
        s32 nDynamic;
    };

    //=====================================================================    
    //=====================================================================
    void SaveFile( void )
    {
        file File;

        s32 Alignment = m_pWrite->Static.GetLength();
            Alignment = ALIGN_4( Alignment ) - Alignment;

        // Save the actual file
        File.Signature = 0x56656e49;
        File.Version   = 1;
        File.nStatic   = m_pWrite->Static.GetLength() + Alignment + m_pWrite->Table.GetLength();
        File.nDynamic  = m_pWrite->Dynamic.GetLength();
        File.nTable    = m_pWrite->Table.GetLength() / sizeof(ref);

        // Save all the static memory first
        x_fwrite( &File, 1, sizeof(File), m_Fp );
        m_pWrite->Static.Save ( m_Fp );
        
        // Write some padding
        for( s32 i=0; i<Alignment; i++ )
        {
            const char Pad='?';
            x_fwrite( &Pad, 1, 1, m_Fp );
        }
        
        // Now we can save the table
        m_pWrite->Table.Save  ( m_Fp );

        // Dynamic memory last
        m_pWrite->Dynamic.Save( m_Fp );
    }

    //=====================================================================    
    inline mem_stream& GetW( void ) const
    {
        ASSERT( m_pWrite );
        if( m_bStatic ) return m_pWrite->Static;
        return m_pWrite->Dynamic;
    }

    //=====================================================================    
    inline mem_stream& GetTable( void ) const
    {
        ASSERT( m_pWrite );
        return m_pWrite->Table;
    }
    //=====================================================================    
    inline xbool IsLocalVariable( byte* pRange )
    {
        return (pRange >= m_pClass) && (pRange < ( m_pClass + m_ClassSize ));
    }

    //=====================================================================    
    inline s32 ComputeLocalOffset( u8* pItem )
    {
        ASSERT( IsLocalVariable( pItem ) );
        return (s32)(pItem - m_pClass);
    }

    //=====================================================================    
    template< class T >
    void Handle( T& A )
    {
        // Reading or writting?
        // TODO: Add support for complex reading
        if( m_pWrite == NULL ) 
            return;

        // Copy most of the data
        fileio File(*this);

        File.m_ClassPos     = GetW().Tell();
        File.m_pClass       = (byte*)&A;
        File.m_ClassSize    = sizeof( A );
        A.FileIO( File );

        // Go the end of the structure 
        GetW().SeekPos( File.m_ClassPos + sizeof( A ));
    }

    //=====================================================================    
    void Handle( char& A ) { GetW().Write( &A, sizeof(char) );  }
    void Handle( s8&   A ) { GetW().Write( &A, sizeof(s8) );    }
    void Handle( s16&  A ) { GetW().Write( &A, sizeof(s16) );   }
    void Handle( s32&  A ) { GetW().Write( &A, sizeof(s32) );   }
    void Handle( s64&  A ) { GetW().Write( &A, sizeof(s64) );   }

    void Handle( u8&   A ) { GetW().Write( &A, sizeof(u8) );    }
    void Handle( u16&  A ) { GetW().Write( &A, sizeof(u16) );   }
    void Handle( u32&  A ) { GetW().Write( &A, sizeof(u32) );   }
    void Handle( u64&  A ) { GetW().Write( &A, sizeof(u64) );   }

    void Handle( f32&  A ) { GetW().Write( &A, sizeof(f32) );   }
    void Handle( f64&  A ) { GetW().Write( &A, sizeof(f64) );   }

    //=====================================================================    
    void Handle( vector3&    A ) { GetW().Write( &A, sizeof(vector3)    ); }
    void Handle( vector3p&   A ) { GetW().Write( &A, sizeof(vector3p)   ); }
    void Handle( bbox&       A ) { GetW().Write( &A, sizeof(bbox)       ); }
    void Handle( xcolor&     A ) { GetW().Write( &A, sizeof(xcolor)     ); }
    void Handle( vector2&    A ) { GetW().Write( &A, sizeof(vector2)    ); }
    void Handle( vector4&    A ) { GetW().Write( &A, sizeof(vector4)    ); }
    void Handle( quaternion& A ) { GetW().Write( &A, sizeof(quaternion) ); }

    //=====================================================================    
    template< class T >
    void Array( T& A, s32 Count, xbool bStatic )
    {
        ASSERT( Count >= 0 );

        byte* pA = (byte*)((void*)A);
        byte* pB = (byte*)((void*)&A);

        // Check whether is an pointer
        if( pA != pB )
        {
            HandlePtr( A, Count, bStatic );
            return;
        }

        ASSERTS( bStatic == TRUE, "All local arrays must be static" );

        // Make sure that we are at the right offset
        GetW().SeekPos( m_ClassPos + ComputeLocalOffset( pA ) );

        // Is an array of structures of items so we want to preallocate the space
        // Since it is an array we don't need to put the "* Count"
        GetW().Preallocate( sizeof(A) );

        // Loop throw all the items
        for( s32 i=0; i<Count; i++ )
        {
            Handle( A[i] );
        }
    }

    //=====================================================================    
    template< class T >
    void HandlePtr( T& A, s32 Count, xbool bStatic )
    {
        ref    Ref;
        byte* pA = (byte*)((void*)&A);
        ASSERTS( !((m_bStatic == FALSE) && (bStatic == TRUE)), "The parent of these structure is been save as dynamic. Not allow statics." );

        //   Parent |  S  |  D  | Child
        // +--------+-----------+-------
        // |      S |  S  |  D  |
        // +--------+-----+-----+
        // |      D |  ?  |  D  |
        // +--------+-----+-----+
        //

        //
        // If we don't have any elements then just write the pointer raw
        //
        if( Count == 0 )
        {
            Static( *((s32*)(&A)) );
            return;
        }

        //
        // Handle the reference pointer
        //
        {    
            if( bStatic )
            {
                // Make sure we are at the end of the buffer before preallocating
                m_pWrite->Static.GotoEnd();

                m_pWrite->Static.Preallocate32( sizeof(*A) * Count );
                Ref.PointingAT = m_pWrite->Static.Tell();
            }
            else
            {
                // Make sure we are at the end of the buffer before preallocating
                m_pWrite->Dynamic.GotoEnd();

                m_pWrite->Dynamic.Preallocate32( sizeof(*A) * Count );
                Ref.PointingAT = m_pWrite->Dynamic.Tell();
            }

            Ref.OffSet     = m_ClassPos + ComputeLocalOffset( pA );
            Ref.Count      = Count;
            Ref.Flags      = ((bStatic==TRUE)<<1) | ((m_bStatic==TRUE)<<0);

            GetTable().Write( &Ref, sizeof(Ref) );
        }

        //
        // Loop throw all the items
        //

        // Upate the type
        xbool BackStatic = m_bStatic;
               m_bStatic = bStatic;

        // We better be at the write spot that we are pointing at 
        ASSERT( Ref.PointingAT == GetW().Tell() );

        // Now loop
        for( s32 i=0; i<Count; i++ )
        {
            Handle( A[i] );
        }

        // Restore the type
        m_bStatic = BackStatic;
    }

    xbool       m_bStatic;
    writting*   m_pWrite;
    X_FILE*     m_Fp;
    s32         m_ClassPos;
    byte*       m_pClass;
    s32         m_ClassSize;
};

//=========================================================================
// END
//========================================================================
#endif
