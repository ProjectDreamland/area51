#include "../auxiliary/bitmap/aux_bitmap.hpp"
#include "TextureMgr.hpp"
#include "errorlog.hpp"

extern fx_core::error_log g_ErrorLog;

namespace fx_core
{

texture_mgr* g_pTextureMgr = new texture_mgr;

//============================================================================
// Constructor
texture_mgr::texture_mgr()
{
    m_UseVRAM = TRUE;
    m_DontLoad = FALSE;
}

//============================================================================
// Destructor
texture_mgr::~texture_mgr()
{
    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        record* pRec = Itr.Item();
        delete pRec;
    }
}

//============================================================================
void texture_mgr::DontUseVRAM( void )
{
    m_UseVRAM = FALSE;
}

//============================================================================
void texture_mgr::DontLoad( void )
{
    m_DontLoad = TRUE;
}

//============================================================================
// Is the number a power of 2?  (Swiped algorithm from the web)
int ispow2(int x)
{
      return! ((~(~0U>>1)|x)&x -1) ;
}

//============================================================================

xbool ReadFileCreationTime( const char* pFileName, FILETIME& FileTime )
{
    xbool Success = FALSE;

    HANDLE hFile = CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
    if( hFile )
    {
        BY_HANDLE_FILE_INFORMATION Info;
        if( GetFileInformationByHandle( hFile, &Info ) )
        {
            FileTime = Info.ftCreationTime;
            Success = TRUE;
        }
        else
        {
            FileTime.dwHighDateTime = 0;
            FileTime.dwLowDateTime  = 0;
        }
        CloseHandle( hFile );
    }

    return Success;
}

//============================================================================
// Given a string, load and activate a bitmap
xbool texture_mgr::ActivateBitmap ( const char* pFileName )
{
    record* pRec = NULL;

    // check to see if the bitmap is already loaded
    pRec = Lookup( pFileName );

    if ( !pRec )
    {
        // New record
        pRec = new texture_mgr::record;
        pRec->UsedFlag = FALSE;
        pRec->IsLoaded = FALSE;
        pRec->ID       = -1;
        pRec->CreationTime.dwHighDateTime = 0;
        pRec->CreationTime.dwLowDateTime  = 0;

        // Just return success if TextureMgr is set not to load bitmaps
        if( m_DontLoad )
        {
            x_strcpy( pRec->Path, pFileName );
            m_Records.Append( pRec );
            return TRUE;
        }

        // if it is not, then load it!
        if( auxbmp_LoadNative( pRec->Bitmap, pFileName ) )
        {
            ReadFileCreationTime( pFileName, pRec->CreationTime );

            s32 Width = pRec->Bitmap.GetWidth();
            s32 Height = pRec->Bitmap.GetHeight();

            if ( !(ispow2(Width) && ispow2(Height)) )
            {
                // MessageBox( (HWND)NULL, "The specified bitmap is not power of 2", "Error", MB_ICONEXCLAMATION | MB_SYSTEMMODAL );
                // m_Records.Delete( m_Records.GetCount()-1 );
                delete pRec;
                return FALSE;
            }
            else
            {
                if( m_UseVRAM )
                {
                    pRec->ID = vram_Register( pRec->Bitmap );
                    vram_Activate( pRec->Bitmap );
                }
                x_strcpy( pRec->Path, pFileName );
                m_Records.Append( pRec );

                pRec->IsLoaded = TRUE;

                return TRUE;
            }
        }
        else
        {
            // MessageBox( (HWND)NULL, "Unable to load the bitmap. Providing default instead.", "Error", MB_ICONEXCLAMATION | MB_SYSTEMMODAL );
            u8 *pData = (u8*)x_malloc(64*64*4);
            
            x_memset( pData,        255,      64*16*4 );
            x_memset( pData + (64*16*4), 192, 64*16*4 );
            x_memset( pData + (64*32*4), 128, 64*16*4 );
            x_memset( pData + (64*48*4),  64, 64*16*4 );
            
            pRec->Bitmap.Setup( xbitmap::FMT_32_ARGB_8888, 64, 64, TRUE, pData );
            
            if ( x_strlen(pFileName) == 0 )
                x_strcpy( pRec->Path, "fx_default.xbmp" );
            else
                x_strcpy( pRec->Path, pFileName );

            if( m_UseVRAM )
            {
                pRec->ID = vram_Register( pRec->Bitmap );
                vram_Activate( pRec->Bitmap );
            }
            m_Records.Append( pRec );

            pRec->IsLoaded = TRUE;

            return TRUE;
        }
    }
    else
    {
        if( m_UseVRAM )
        {
            vram_Activate( pRec->ID );
        }
    }

    return TRUE;

}

//============================================================================
// Given a string, remove a bitmap
xbool texture_mgr::DeActivateBitmap ( const char* pFileName )
{
    record*             pRec = NULL;
    
    // Make sure we have a valid filename that is already in use first
    if( !x_strlen(pFileName)  )
        return FALSE;

    // remove the entry
    Remove( pFileName );

    return TRUE;
}
//============================================================================
// Given a string, see if it's loaded and if it is, return the ID
s32 texture_mgr::GetBitmapID( const char* pFileName )
{
    record* pRec;

    // check to see if the bitmap is already loaded
    pRec = Lookup( pFileName );

    if ( pRec )
        return pRec->ID;
    else
        return -1;
}

//============================================================================
// Given a string, see if it's loaded and if it is, return the ID
s32 texture_mgr::GetTextureIndex( const char* pFileName )
{
    record* pRec = NULL;
    s32 Index = 0;

    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        pRec = Itr.Item();

        if ( pRec->UsedFlag )
        {
            if ( x_stricmp(pRec->Path, pFileName) == 0 )
            {
                return Index;
            }
            
            Index++;
        }
    }

    return -1;
}


//============================================================================
// Export the bitmap data
void texture_mgr::MarkAsUsed( const char* pFileName )
{
    record* pRec;

    // check to see if the bitmap is already loaded
    pRec = Lookup( pFileName );
    if ( pRec )
        pRec->UsedFlag = TRUE;
    else
        ASSERT(FALSE);
}

//============================================================================
// Get the number of active bitmaps
s32 texture_mgr::GetBitmapCount( void )
{
    record* pRec = NULL;
    s32 Count = 0;
    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        if ( Itr.Item()->UsedFlag )
            Count++;
    }

    return Count;
}

//============================================================================
// Export the bitmap data

void texture_mgr::ExportList( const char* pFileName, s32 ExportTarget )
{
    record*   pRec = NULL;
    X_FILE*   FP   = x_fopen( pFileName, "ab" );
    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        pRec = Itr.Item();

        if ( pRec->UsedFlag && pRec->IsLoaded )
        {
            switch( ExportTarget )
            {
            case EXPORT_TARGET_PC:
                {
                    xbool Success = pRec->Bitmap.Save( FP );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FP) );
                }
                break;

            case EXPORT_TARGET_GCN:
                {
                    xbool Success = pRec->Bitmap.Save( FP );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FP) );
                }
                break;

            case EXPORT_TARGET_PS2:
                {
                    xbool Success = pRec->Bitmap.Save( FP );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FP) );
                }
                break;

            case EXPORT_TARGET_XBOX:
                {
                    xbool Success = pRec->Bitmap.Save( FP );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FP) );
                }
                break;
            }
        }
    }

    x_fclose( FP );
}

//============================================================================

void texture_mgr::ExportNames( xbytestream& Stream, s32 ExportTarget )
{
    char    FName   [ X_MAX_FNAME ];
    char    FNameA  [ X_MAX_FNAME ];

    record* pRec = NULL;

    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        pRec = Itr.Item();
        
        char* Key;
        Key = pRec->Path;

        if( pRec->UsedFlag )
        {
            x_splitpath( (LPCTSTR)Key, NULL, NULL, FName, NULL );

            switch( ExportTarget )
            {
            case EXPORT_TARGET_PC:
            case EXPORT_TARGET_XBOX:
            case EXPORT_TARGET_PS2:
                {
                    // Write the filename.
                    // Write a NULL name.

                    x_strcat( FName, ".xbmp" );
                    Stream.Append( (byte*)&FName, x_strlen(FName)+1 );
                    Stream.Append( 0x00 );
                }
                break;

            case EXPORT_TARGET_GCN:
                {
                    // If the bitmap has alpha
                    //      Write the filename[D].
                    //      Write the filename[A].
                    // Otherwise
                    //      Write the filename.
                    //      Write a NULL name.

                    if( pRec->Bitmap.HasAlphaBits() )
                    {
                        x_strcpy( FNameA, FName );
                        x_strcat( FName , "[D].xbmp" );
                        x_strcat( FNameA, "[A].xbmp" );

                        Stream.Append( (byte*)&FName , x_strlen(FName )+1 );
                        Stream.Append( (byte*)&FNameA, x_strlen(FNameA)+1 );
                    }
                    else
                    {
                        x_strcat( FName, ".xbmp" );
                        Stream.Append( (byte*)&FName, x_strlen(FName)+1 );
                        Stream.Append( 0x00 );
                    }

                }
                break;
            }
        }
    }
}

//============================================================================

void texture_mgr::ExportXBMPs( const char* pFileName, s32 ExportTarget )
{
    char    FullName[ X_MAX_PATH  ];
    char    Drive   [ X_MAX_DRIVE ];
    char    Dir     [ X_MAX_DIR   ];
    char    FName   [ X_MAX_FNAME ];

    x_splitpath( pFileName, Drive, Dir, NULL, NULL );

    record* pRec = NULL;
    xbitmap Diffuse;
    xbitmap Alpha;
    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        pRec = Itr.Item();
        
        char *Key = pRec->Path;

        if( pRec->UsedFlag && pRec->IsLoaded )
        {
            Diffuse = pRec->Bitmap;

            x_splitpath( (LPCTSTR)Key, NULL, NULL, FName, NULL );
            x_makepath( FullName, Drive, Dir, FName, ".xbmp" );

            // Check the output is not newer than the input bitmap
            FILETIME DestTime;
            if( ReadFileCreationTime( FullName, DestTime ) )
            {
                // If input is older than output then skip this bitmap
                if( CompareFileTime( &pRec->CreationTime, &DestTime ) < 0 )
                    continue;
            }

            switch( ExportTarget )
            {
                case EXPORT_TARGET_XBOX:
                {
                    auxbmp_Compress( Diffuse,pFileName,4 );
                    xbool Success = Diffuse.Save( FullName );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FullName) );
                }
                break;

                case EXPORT_TARGET_PC:
                {
                    //  alpha:   FMT_32_ARGB_8888 
                    // no alpha: FMT_32_URGB_8888

                    if( Diffuse.HasAlphaBits() )
                        Diffuse.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
                    else
                        Diffuse.ConvertFormat( xbitmap::FMT_32_URGB_8888 );

                    xbool Success = Diffuse.Save( FullName );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FullName) );
                }
                break;

                case EXPORT_TARGET_GCN:
                {
                    // Not supported any more
                    ASSERT( 0 );
/*
                    //  alpha:   FMT_DXT1 and FMT_DXT1
                    // no alpha: FMT_DXT1

                    if( Diffuse.HasAlphaBits() )
                    {
                        auxbmp_MakeDiffuseFromA( Alpha, Diffuse );
                        auxbmp_CompressDXT1( Diffuse );
                        auxbmp_CompressDXT1( Alpha   );
                        auxbmp_ConvertToGCN( Diffuse, TRUE );
                        auxbmp_ConvertToGCN( Alpha  , TRUE );

                        x_strcpy( FNameA, FName );
                        x_strcat( FName , "[D]" );
                        x_strcat( FNameA, "[A]" );

                        x_makepath( FullName, Drive, Dir, FName,  ".xbmp" );
                        xbool Success = Diffuse.Save( FullName );
                        if( !Success )
                            g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FullName) );

                        x_makepath( FullName, Drive, Dir, FNameA, ".xbmp" );
                        Success = Alpha.Save( FullName );
                        if( !Success )
                            g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FullName) );
                    }
                    else
                    {
                        auxbmp_CompressDXT1( Diffuse );
                        auxbmp_ConvertToGCN( Diffuse, TRUE );
                        x_makepath( FullName, Drive, Dir, FName, ".xbmp" );
                        xbool Success = Diffuse.Save( FullName );
                        if( !Success )
                            g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FullName) );
                    }
*/
                }
                break;

                case EXPORT_TARGET_PS2:
                {
                    //  alpha:   FMT_32_ARGB_8888 
                    // no alpha: FMT_32_URGB_8888

                    if( Diffuse.HasAlphaBits() )
                        Diffuse.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );
                    else
                        Diffuse.ConvertFormat( xbitmap::FMT_P8_UBGR_8888 );

                    Diffuse.PS2SwizzleClut();

                    xbool Success = Diffuse.Save( FullName );
                    if( !Success )
                        g_ErrorLog.Append( xfs("Error - Saving xbitmap \"%s\"", FullName) );
                }
                break;
            }
        }
    }
}

//============================================================================

texture_mgr::record* texture_mgr::Lookup( const char* pFileName )
{
    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        record* pRec = Itr.Item();

        if ( x_stricmp(pRec->Path, pFileName) == 0 )
        {
            return pRec;
        }
    }

    return NULL;
}

//============================================================================

void texture_mgr::Remove( const char* pFileName )
{

    DListIterator<record*> Itr = m_Records.GetIterator();

    for ( Itr.Start(); Itr.Valid(); Itr.Forth() )
    {
        record* pRec = Itr.Item();

        if ( x_stricmp(pRec->Path, pFileName) == 0 )
        {
            delete pRec;
            m_Records.Remove( Itr );
        }
    }
}

} // namespace fx_core
