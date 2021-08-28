//==============================================================================
//
//  dfs_Build.cpp
//
//==============================================================================

#include "dfs.hpp"
#include "dictionary.hpp"
#include "x_bytestream.hpp"
#include "x_plus.hpp"
#include "windows.h"

#if !defined(TARGET_PC)
  #error "dfs_Build.cpp is only valid for TARGET_PC"
#endif

#include <string.h>
#include <stdio.h>

//==============================================================================
//  DEBUG SWITCHES
//==============================================================================

#define VERIFY_NAME_SPLITTING

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  DATA
//==============================================================================

struct src_file 
{
    const char*     pPathName;                  // Full PathName string
    s32             PathNameLength;             // Length of PathName string
    s32             iPath;                      // Index of string in dictionary
    s32             iFile1;                     // Index of string in dictionary
    s32             iFile2;                     // Index of string in dictionary
    s32             iExt;                       // Index of string in dictionary
};

xarray<char*>       s_Scripts;                  // Array of scripts in memory
xarray<src_file>    s_SrcFiles;                 // Array of source files
xarray<u32>         s_FileOffsetTable;          // Array of file offsets
xarray<u32>         s_ChecksumIndex;            // Array of checksum indexes
u32                 s_DFSBufferSize = 32 * 1024;// Default buffer size is 32k
xarray<xstring>     s_SectorAligned;            // Sector aligned file extensions

//==============================================================================
//  Checksum Helper Class
//==============================================================================

extern "C" const u16 crc16Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

#define crc16ApplyByte( v, crc ) (u16)((crc << 8) ^  crc16Table[((crc >> 8) ^ (v)) & 255])

class checksummer
{
public:
    xarray<u16>     m_Checksums;
    u16             m_Checksum;
    s32             m_BytesProcessed;
    s32             m_BytesToNextSum;
    u32             m_ChunkSize;

    checksummer( )
    {
        m_Checksums.SetCapacity( 100000 );
        Init( 32768 );
    }

    void Init       ( u32 ChunkSize )
    {
        m_Checksums.Clear();
        m_Checksum = 0;
        m_BytesProcessed = 0;
        m_BytesToNextSum = ChunkSize;
        m_ChunkSize = ChunkSize;
    }

    void ApplyData  ( u8* pData, s32 Count )
    {
        while( Count > 0 )
        {
            s32 BytesToProcess = MIN( m_BytesToNextSum, Count );
            m_BytesProcessed += BytesToProcess;
            m_BytesToNextSum -= BytesToProcess;
            Count -= BytesToProcess;

            while( BytesToProcess-- > 0 )
            {
                m_Checksum = crc16ApplyByte( *pData, m_Checksum );
                pData++;
            }

            ASSERT( m_BytesToNextSum >= 0 );
            if( m_BytesToNextSum == 0 )
            {
                m_Checksums.Append( m_Checksum );
                m_Checksum = 0;
                m_BytesToNextSum = m_ChunkSize;
            }
        }
    }

    byte* GetData( void )
    {
        return (byte*)m_Checksums.GetPtr();
    }

    s32 GetDataSize( void )
    {
        return m_Checksums.GetCount() * sizeof(u16);
    }
};

checksummer g_Checksummer;

//==============================================================================
//  Helper Functions
//==============================================================================

static inline s32 TransformChar( s32 c )
{
    // Upper Case
    if( (c >= 'a') && (c <= 'z') )
        c += 'A' - 'a';

    // All slashes will be back-slashes
    if( c == '/' )
        c = '\\';

    // Return transformed character
    return c;
}

//==============================================================================
//  SetChunkSize
//==============================================================================

void dfs_SetChunkSize( u32 nBytes )
{
    s_DFSBufferSize = nBytes;
}

//==============================================================================
//  SectorAlign
//==============================================================================

void dfs_SectorAlign( const char* pExtension )
{
    s_SectorAligned.Append() = pExtension;
}

//==============================================================================
//  FreeScripts
//==============================================================================

void dfs_FreeScripts( void )
{
    // Loop through all allocated scripts
    for( s32 i=0 ; i<s_Scripts.GetCount() ; i++ )
    {
        // Free the memory
        x_free( s_Scripts[i] );
    }

    // Clear the scripts array
    s_Scripts.Clear();
}

//==============================================================================
//  ReadScripts
//==============================================================================

void dfs_ReadScripts( const xarray<xstring>& Scripts )
{
    // Prime s_Files for a lot of records
    s_SrcFiles.SetCapacity( 20000 );

    // Loop through scripts
    for( s32 i=0 ; i<Scripts.GetCount() ; i++ )
    {
        X_FILE* pFile = x_fopen( Scripts[i], "rb" );
        if( pFile )
        {
            // Get file length
            s32 Length = x_flength( pFile );

            // Allocate storage for the script
            char*& pScript = s_Scripts.Append();
            pScript = (char*)x_malloc( Length+1 );
            if( pScript )
            {
                // Read the script
                s32 BytesRead = x_fread( pScript, 1, Length, pFile );
                if( BytesRead == Length )
                {
                    // Terminate the data read
                    pScript[Length] = 0;

                    // Tokenize script file
                    char* pCursor       = pScript;

                    // Loop until end of script
                    while( *pCursor )
                    {
                        // Skip whitespace
                        while( x_isspace(*pCursor) )
                            pCursor++;

                        // Start of a new string
                        char* pStringStart = pCursor;

                        // Scan for end of string & transform string on the way
                        while( (*pCursor != 0x0d) && (*pCursor != 0) )
                        {
                            *pCursor++ = (char)TransformChar( *pCursor );
                        }

                        // Keep record of end of string
                        char* pStringEnd = pCursor;

                        // Advance cursor if not at end of file
                        if( *pCursor != 0 )
                            pCursor++;

                        // Remove any trailing whitespace
                        while( (pStringEnd > pStringStart) && x_isspace(*pStringEnd) )
                        {
                            *pStringEnd = 0;
                            pStringEnd--;
                        }

                        // Skip to next string if we got an empty string
                        if( pStringEnd == pStringStart )
                            continue;

                        // Add a new source file record
                        src_file& SrcFile = s_SrcFiles.Append();
                        SrcFile.pPathName = pStringStart;
                        SrcFile.PathNameLength = (s32)(pStringEnd-pStringStart)+1;
                    }
                }
                else
                {
                    // TODO: Error - read error
                    ASSERT( 0 );
                }
            }
            else
            {
                // TODO: Error - out of memory
                ASSERT( 0 );
            }

            // Close the file
            x_fclose( pFile );
        }
        else
        {
            // TODO: Error - couldn't open file
            ASSERT( 0 );
        }
    }
}

//==============================================================================
//  dfs_AreSourceFilesNewer - Check if source files are newer than the DFS
//==============================================================================

xbool GetFileTime( const char* pPathName, FILETIME& FileTime )
{
    BY_HANDLE_FILE_INFORMATION  FileInfo;
    xbool                       Found = FALSE;

    // Does the file exist?
    HANDLE hFile = CreateFile( pPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hFile != NULL )
    {
        // Get write time of file
        if( GetFileInformationByHandle( hFile, &FileInfo ) )
        {
            // Get time the DFS file was last written
            FileTime = FileInfo.ftLastWriteTime;
            Found = TRUE;
        }

        // Close the file
        CloseHandle( hFile );
    }

    // Return success code
    return Found;
}

xbool dfs_AreSourceFilesNewer( const xstring& PathNameDFS, xarray<src_file>& SrcFiles )
{
    xbool       FoundNewer = TRUE;
    s32         nCompared  = 0;
    FILETIME    TimeDFS;
    FILETIME    TimeSrc;

    //
    // Check if file listings match
    //
    {
        X_FILE* fp = x_fopen((const char*)PathNameDFS,"rb");
        if( !fp )
            return TRUE;

        s32 FSize = x_flength(fp);
        byte* pDFS = (byte*)x_malloc(FSize);
        if(!pDFS) 
            return TRUE;
        x_fread( pDFS, FSize, 1, fp );
        x_fclose(fp);

        dfs_header* pHeader = dfs_InitHeaderFromRawPtr( pDFS );
        if( !pHeader ) 
            return TRUE;

        if( pHeader->nFiles != SrcFiles.GetCount() )
            return TRUE;

        dfs_file*   pEntry  = pHeader->pFiles;
        for( s32 i=0 ; i<pHeader->nFiles ; i++, pEntry++ )
        {
            char OldName[256];
            x_sprintf( OldName, "%s%s%s%s", 
                pHeader->pStrings + (u32)pEntry->PathNameOffset,
                pHeader->pStrings + (u32)pEntry->FileNameOffset1,
                pHeader->pStrings + (u32)pEntry->FileNameOffset2,
                pHeader->pStrings + (u32)pEntry->ExtNameOffset);

            if( x_stricmp(OldName,(const char*)SrcFiles[i].pPathName) != 0 )
                return TRUE;
        }
    }

    // If DFS not found then return TRUE to force a build
    if( GetFileTime( PathNameDFS, TimeDFS ) )
    {
        // Reset the found newer flag
        FoundNewer = FALSE;

        // Get the most recent modified date from the SrcFiles
        for( s32 i=0 ; i<SrcFiles.GetCount() ; i++ )
        {

            if( GetFileTime( SrcFiles[i].pPathName, TimeSrc ) )
            {
                // Compare against DFS time
                nCompared++;
                if( CompareFileTime( &TimeDFS, &TimeSrc ) <= 0 )
                {
                    // The source file is newer than the DFS file
                    FoundNewer = TRUE;
                }
            }
        }

        // If no files were compared then set Newer
        if( nCompared == 0 )
            FoundNewer = TRUE;
    }

    // Return if a newer source file was found
    return FoundNewer;
}

//==============================================================================
//  FindCommonSubstring
//==============================================================================

void FindCommonSubstring( const char* pStr0, const char* pStr1, const char* pStr2, char* pSub1, char* pSub2 )
{
    s32         PrevCommonLen = 0;
    s32         CommonLen     = 0;
    const char* p1;
    const char* p2;

    // Determine PrevCommonLen between the 2 strings
    p1 = pStr1;
    p2 = pStr0;
    while( (*p1 != 0) && (*p2 != 0) && (*p1 == *p2) )
    {
        PrevCommonLen++;
        p1++;
        p2++;
    }

    // Back off if the last characters are numeric
    while( (PrevCommonLen > 0) && (x_isdigit(pStr1[PrevCommonLen-1])) )
        PrevCommonLen--;

    // Determine CommonLen between the 2 strings
    p1 = pStr1;
    p2 = pStr2;
    while( (*p1 != 0) && (*p2 != 0) && (*p1 == *p2) )
    {
        CommonLen++;
        p1++;
        p2++;
    }

    // Back off if the last characters are numeric
    while( (CommonLen > 0) && (x_isdigit(pStr1[CommonLen-1])) )
        CommonLen--;

    // Check if previous CommonLen is a better fit
    if( PrevCommonLen > CommonLen )
        CommonLen = PrevCommonLen;

    // Split the strings
    x_strncpy( pSub1, pStr1, CommonLen );
    pSub1[CommonLen] = 0;
    x_strcpy( pSub2, &pStr1[CommonLen] );

//    x_printf( "\"%s\" - \"%s\"\n", pSub1, pSub2 );
}

//==============================================================================
// IO Buffering
//==============================================================================

static byte*    s_pIOBuffer  = NULL;
static s32      s_DestOffset = 0;
static s32      s_IOBufferSize = 0;
static s32      s_IOOffset = 0;
static X_FILE*  s_pDestFile;

void io_Init( X_FILE* pDestFile, s32 IOBufferSize )
{
    ASSERT( pDestFile );
    s_pDestFile = pDestFile;

    s_pIOBuffer = (byte*)x_malloc( IOBufferSize );
    ASSERT( s_pIOBuffer );
    x_memset( s_pIOBuffer, 0, IOBufferSize );

    s_IOBufferSize = IOBufferSize;
    s_DestOffset = 0;
    s_IOOffset = 0;
}

void io_Flush( void )
{
    //x_DebugMsg("FLUSH!!!\n");

    g_Checksummer.ApplyData( s_pIOBuffer, s_IOOffset );

    x_fwrite( s_pIOBuffer, s_IOOffset, 1, s_pDestFile );
    s_IOOffset = 0;
}

void io_Kill( void )
{
    io_Flush();

    x_free( s_pIOBuffer );
    s_pIOBuffer     = NULL;
    s_IOBufferSize  = 0;
    s_DestOffset    = 0;
    s_IOOffset = 0;
}

s32 io_GetDestOffset( void )
{
    return s_DestOffset;
}

void io_Read( X_FILE* pSrcFile, s32 nBytes )
{
    while( nBytes > 0 )
    {
        s32 SizeRemainingInBuffer = s_IOBufferSize - s_IOOffset;
        s32 SizeToRead = MIN( SizeRemainingInBuffer, nBytes );

        // Do file read
        x_fread( s_pIOBuffer + s_IOOffset, SizeToRead, 1, pSrcFile );
        s_IOOffset += SizeToRead;
        s_DestOffset += SizeToRead;

        //x_DebugMsg("READ: %8d\n",SizeToRead);

        // Reduce total bytes to read
        nBytes -= SizeToRead;
        if( nBytes > 0 )
        {
            // We didn't fit so we must need to flush
            io_Flush();
        }
    }
}

void io_Pad( s32 nPadBytes )
{
    while( nPadBytes > 0 )
    {
        s32 SizeRemainingInBuffer = s_IOBufferSize - s_IOOffset;
        s32 SizeToPad = MIN( SizeRemainingInBuffer, nPadBytes );

        // Do file read
        x_memset( s_pIOBuffer + s_IOOffset, 0, SizeToPad );
        s_IOOffset += SizeToPad;
        s_DestOffset += SizeToPad;

        //x_DebugMsg("PAD:  %8d\n",nPadBytes);

        // Reduce total bytes to pad
        nPadBytes -= SizeToPad;
        if( nPadBytes > 0 )
        {
            // We didn't fit so we must need to flush
            io_Flush();
        }
    }
}


//==============================================================================
//  dfs_Build
//==============================================================================

void dfs_Build( const xstring&          PathName,
                const xarray<xstring>&  Scripts,
                xbool                   DoMake,
                u32                     SectorSize,
                u32                     SplitSize,
                u32                     ChunkSize,
                xbool                   bEnableCRC )
{
    // Initialize the checksummer
    g_Checksummer.Init( ChunkSize );

    // SplitSize must be a multiple of sector size
    ASSERT( SectorSize > 0 );
    ASSERT( (SectorSize & (SectorSize-1)) == 0 );
    ASSERT( (SplitSize % SectorSize) == 0 );

    // Data
    dictionary  Dictionary;                 // Dictionary of pathname strings
    xbytestream FileTable;                  // Bytestream to accumulate dfs_file structures for file table
    s32         nFilesOutput    = 0;        // Number of files output
    char        DataFileName[X_MAX_PATH];   // Name of data file
    s32         nDataFiles      = 0;        // Number of the current data file
    X_FILE*     pDataFile       = NULL;     // Pointer to data file
    u32         DataPosition    = 0;        // Current position in output data stream
    u32         FilePosition    = 0;
    xbool       bSectorAlign    = FALSE;
    xbool       bFirstTime      = TRUE;
    xstring     PathNameDFS;
    xbytestream Checksums;                  // Bytestream to accumulate checksums into

    // Build data file name
    char Drive[X_MAX_DRIVE];
    char Dir  [X_MAX_DIR];
    char FName[X_MAX_FNAME];
    x_splitpath( PathName, Drive, Dir, FName, NULL );
    x_strcpy( DataFileName, Drive );
    x_strcat( DataFileName, Dir );
    x_strcat( DataFileName, FName );

    // Generate the pathname to the DFS file
    PathNameDFS = xfs("%s.DFS", DataFileName);

    // Allocate Pad Buffer
//    byte* pPadBuffer = (byte*)x_malloc( SectorSize );
//    ASSERT( pPadBuffer );
//    x_memset( pPadBuffer, 0, SectorSize );

    // Allocate Buffer
//    byte* pBuffer = (byte*)x_malloc( s_DFSBufferSize );
//    if( pBuffer )
    {
        char Drive0[X_MAX_PATH];     // Drive (this is X_MAX_PATH because we combine Path into this buffer later)
        char Dir0  [X_MAX_PATH];     // Dir
        char FName0[X_MAX_FNAME];    // FName
        char Ext0  [X_MAX_EXT];      // Ext
        char Drive1[X_MAX_PATH];     // Drive (this is X_MAX_PATH because we combine Path into this buffer later)
        char Dir1  [X_MAX_PATH];     // Dir
        char FName1[X_MAX_FNAME];    // FName
        char Ext1  [X_MAX_EXT];      // Ext
        char Drive2[X_MAX_PATH];     // Drive (this is X_MAX_PATH because we combine Path into this buffer later)
        char Dir2  [X_MAX_PATH];     // Dir
        char FName2[X_MAX_FNAME];    // FName
        char Ext2  [X_MAX_EXT];      // Ext

        // Read scripts and generate file list
        dfs_ReadScripts( Scripts );

        x_printf( "Files = %d\n", s_SrcFiles.GetCount() );

        // Check if any of the source files are newer than the dfs file
        if( DoMake )
        {
            if( !dfs_AreSourceFilesNewer( PathNameDFS, s_SrcFiles ) )
            {
                x_printf( "Already built\n" );
                dfs_FreeScripts();
                return;
            }
        }

        // Loop through all file, this is a bit strange because it tries to compare the 2 adjacent strings
        // and find the largest commmon substring to decide how to break the filename into 2 parts
        for( s32 i=0 ; i<s_SrcFiles.GetCount() ; i++ )
        {
            //x_printf("%d percent completed.\015",(i*100)/s_SrcFiles.GetCount());

            // Get file record
            src_file& SrcFile0 = s_SrcFiles[MAX( i-1, 0 )];
            src_file& SrcFile1 = s_SrcFiles[i];
            src_file& SrcFile2 = s_SrcFiles[MIN( i+1, s_SrcFiles.GetCount()-1 )];

            // The following code will fail on zero length strings
            ASSERT( SrcFile1.PathNameLength > 0 );

            // Split path in components and combine Drive and Path
            x_splitpath( SrcFile0.pPathName, Drive0, Dir0, FName0, Ext0 );
            x_splitpath( SrcFile1.pPathName, Drive1, Dir1, FName1, Ext1 );
            x_splitpath( SrcFile2.pPathName, Drive2, Dir2, FName2, Ext2 );
            x_strcat( Drive1, Dir1 );

            // Search for common substring in FName0 and FName1 and FName2
            char Sub1[X_MAX_PATH];
            char Sub2[X_MAX_PATH];
            FindCommonSubstring( FName0, FName1, FName2, Sub1, Sub2 );

            // Add path substrings to dictionary
            SrcFile1.iPath  = Dictionary.Add( Drive1 );
            SrcFile1.iFile1 = Dictionary.Add( Sub1   );
            SrcFile1.iFile2 = Dictionary.Add( Sub2   );
            SrcFile1.iExt   = Dictionary.Add( Ext1   );

            // Check for sector alignment.
            bSectorAlign = FALSE;
            for( s32 j=0 ; j<s_SectorAligned.GetCount() ; j++ )
            {
                if( x_stricmp( Ext1, s_SectorAligned[j] ) == 0 )
                    bSectorAlign = TRUE;
            }

#ifdef VERIFY_NAME_SPLITTING
            {
                char Test[X_MAX_PATH];
                Test[0] = 0;

                // Rebuild the PathName
                x_strcat( Test, Dictionary.GetString( SrcFile1.iPath  ) );
                x_strcat( Test, Dictionary.GetString( SrcFile1.iFile1 ) );
                x_strcat( Test, Dictionary.GetString( SrcFile1.iFile2 ) );
                x_strcat( Test, Dictionary.GetString( SrcFile1.iExt   ) );

                // Compate against original name
                ASSERT( x_strcmp( Test, SrcFile1.pPathName ) == 0 );
            }
#endif

            // Open the file
            X_FILE* pFile = x_fopen( SrcFile1.pPathName, "rb" );
            if( pFile )
            {
                u32 BytesToPad;
                u32 TotalFileSize;
                u32 FileLength;

                // Get length of file
                FileLength = (u32)x_flength( pFile );

                // Need to pad?
                if( bSectorAlign && pDataFile )
                    BytesToPad = SectorSize - (io_GetDestOffset() & (SectorSize-1));
                else
                    BytesToPad = 0;

                // Calculate total size of the file.
                TotalFileSize = FileLength + BytesToPad;

                // To big? (or the first time???)
                if( !bFirstTime )
//                    FilePosition = x_ftell( pDataFile );
                    FilePosition = io_GetDestOffset();

                if( (FilePosition + TotalFileSize > SplitSize) || bFirstTime )
                {
                    // Only one first time.
                    bFirstTime = FALSE;

                    // Don't need to pad since its a new file.
                    BytesToPad = 0;

                    // Close previous data file
                    if( pDataFile )
                    {
                        s32 BytesToPad = ChunkSize - (io_GetDestOffset() & (ChunkSize-1));
                        io_Pad( BytesToPad );
                        DataPosition += BytesToPad;

                        s_FileOffsetTable.Append() = DataPosition;
                        io_Kill();
                        x_fclose( pDataFile );

                        s_ChecksumIndex.Append() = Checksums.GetLength() / sizeof(u16);
                        Checksums.Append( g_Checksummer.GetData(), g_Checksummer.GetDataSize() );
                        g_Checksummer.Init( ChunkSize );
                        
                        // Update data file number.
                        nDataFiles++;
                    }

                    // Create new data file
                    pDataFile = x_fopen( xfs("%s.%03d", DataFileName, nDataFiles), "wb" );
                    if( pDataFile == NULL )
                    {
                        // Error opening data file to write
                        ::MessageBox( NULL, xfs("Could not open file: %s.%03d\n",DataFileName, nDataFiles), "dfsTool Error", MB_ICONSTOP );
                        exit(10);
                    }
                    else
                    {
                        io_Init( pDataFile, s_DFSBufferSize );

                        // Set up write buffering with a large buffer size.
                        //setvbuf( (FILE *)pDataFile, NULL, _IOFBF, 64 * 1024*1024 );
                    }
                }

                // Pad the output file.
                if( BytesToPad )
                {
                    if( pDataFile )
                    {
                        io_Pad( BytesToPad );

                        // Write out the pad. 
                        //u32 nBytes = (u32)x_fwrite( pPadBuffer, 1, BytesToPad, pDataFile );
                        //ASSERT( nBytes == BytesToPad );

                        // Update position.
                        DataPosition += BytesToPad;
                    }
                }

                // Check that string table indices are valid
                ASSERT( (SrcFile1.iPath  >= 0) && (SrcFile1.iPath  < 65535) );
                ASSERT( (SrcFile1.iFile1 >= 0) && (SrcFile1.iFile1 < 65535) );
                ASSERT( (SrcFile1.iFile2 >= 0) && (SrcFile1.iFile2 < 65535) );
                ASSERT( (SrcFile1.iExt   >= 0) && (SrcFile1.iExt   < 65535) );

                // Add dfs_file record into bytestream
                dfs_file dfsFile;
                dfsFile.PathNameOffset  = (u32)Dictionary.GetOffset(SrcFile1.iPath);
                dfsFile.FileNameOffset1 = (u32)Dictionary.GetOffset(SrcFile1.iFile1);
                dfsFile.FileNameOffset2 = (u32)Dictionary.GetOffset(SrcFile1.iFile2);
                dfsFile.ExtNameOffset   = (u32)Dictionary.GetOffset(SrcFile1.iExt);
                dfsFile.DataOffset      = DataPosition;
                dfsFile.Length          = FileLength;
                FileTable.Append( (const byte*)&dfsFile, sizeof(dfsFile) );

                // Update number of files output
                nFilesOutput++;

                // Copy data to output stream
                u32 FileBytesLeft = FileLength;
                while( FileBytesLeft > 0 )
                {
                    // Get number of bytes to copy
                    u32 BytesToCopy = MIN( FileBytesLeft, s_DFSBufferSize );

                    // Do the copy
                    if( pDataFile )
                    {
                        //u32 nBytes;

                        io_Read( pFile, BytesToCopy );
                        //nBytes = (u32)x_fread ( pBuffer, 1, BytesToCopy, pFile );
                        //ASSERT( nBytes == BytesToCopy );
                        //nBytes = (u32)x_fwrite( pBuffer, 1, BytesToCopy, pDataFile );
                        //ASSERT( nBytes == BytesToCopy );

                        // Update counters
                        FileBytesLeft -= BytesToCopy;
                        DataPosition  += BytesToCopy;
                    }
                }

                // Close the file
                x_fclose( pFile );
            }
            else
            {
                // TODO: Error - couldn't open file
//                ASSERT( 0 );
            }
        }

        // Close data file
        if( pDataFile )
        {
            // Pad the data file to a sector boundry
            s32 BytesToPad = ChunkSize - (io_GetDestOffset() & (ChunkSize-1));
            io_Pad( BytesToPad );
            DataPosition += BytesToPad;

            s_FileOffsetTable.Append() = DataPosition;
            io_Kill();
            x_fclose( pDataFile );

            s_ChecksumIndex.Append() = Checksums.GetLength() / sizeof(u16);
            Checksums.Append( g_Checksummer.GetData(), g_Checksummer.GetDataSize() );
            g_Checksummer.Init( ChunkSize );

            // Update data file number
            nDataFiles++;
        }

        // TODO: Verbose output only
        x_printf( "Dictionary = %d : %d\n", Dictionary .GetCount(), Dictionary .GetSaveSize() );
        x_printf( "Data       = %d\n", DataPosition );
    }
//    else
//    {
//        // TODO: Error - out of memory
//        ASSERT( 0 );
//    }

    // Write the .DFS file
    checksummer Checksum;
    Checksum.Init( S32_MAX );

    X_FILE* pFile = x_fopen( PathNameDFS, "wb" );
    if( pFile != NULL )
    {
        // Write header
        dfs_header Header;
        x_memset( &Header, 0, sizeof(Header) );
        Header.Magic            = DFS_MAGIC;
        Header.Version          = DFS_VERSION;
        Header.Checksum         = 0;
        Header.SectorSize       = SectorSize;
        Header.SplitSize        = SplitSize;
        Header.nFiles           = nFilesOutput;
        Header.nSubFiles        = nDataFiles;
        Header.StringsLength    = Dictionary.GetSaveSize();
        Header.pSubFileTable    = 0; // Fixup later.
        Header.pFiles           = 0; // Fixup later.
        Header.pChecksums       = 0; // Fixup later.
        Header.pStrings         = 0; // Fixup later.

        xbytestream Data;

        Header.pSubFileTable = (dfs_subfile*)sizeof(Header);

        // Write out the sub file table.
        for( s32 j=0 ; j<s_FileOffsetTable.GetCount(); j++ )
        {
            u32 FileOffset = s_FileOffsetTable[j];
            Data.Append( (byte*)&FileOffset, sizeof(u32) );

            u32 ChecksumIndex = s_ChecksumIndex[j];
            Data.Append( (byte*)&ChecksumIndex, sizeof(u32) );
        }

        // Get offset of the file table
        Header.pFiles = (dfs_file*)(Data.GetLength() + sizeof(Header));

        // Write file table
        Data.Append( FileTable );

        // Only if it is enabled.
        if( bEnableCRC )
        {
            // Get offset of the checksum table.
            Header.pChecksums = (u16*)(Data.GetLength() + sizeof(Header));

            // Write checksum table
            Data.Append( Checksums );
        }

        // Get offset of the string table.
        Header.pStrings = (char*)(Data.GetLength() + sizeof(Header));

        // Write string table
        Dictionary.Save( Data );

        // Get offset of the checksums.
//        Header.pChecksums = (u16*)(Data.GetLength() + sizeof(Header));

        // Write checksums
//        Data.Append( Checksums );

        // Pad the data so that sizeof(Header) + Data.GetLength() is a multiple of 2k
        {
            char buffer[2048];
            x_memset( buffer, 0, 2048 );
            s32 nBytes = (sizeof(Header) + Data.GetLength()) % 2048;
            if( nBytes )
            {
                nBytes = 2048-nBytes;
                Data.Append( (byte*)buffer, nBytes );
            }
        }

        // Checksum the Header and Data
        Checksum.ApplyData( (u8*)&Header, sizeof(Header) );
        Checksum.ApplyData( (u8*)Data.GetBuffer(), Data.GetLength() );
        Header.Checksum = Checksum.m_Checksum;

        // Write the Header and the Data
        x_fwrite( &Header, sizeof(Header), 1, pFile );
        Data.SaveFile( pFile );

        // Close the file
        x_fclose( pFile );

        // These need to match.
        ASSERT( nDataFiles == s_FileOffsetTable.GetCount() );
    }
    else
    {
        // TODO: Error opening file to write
        ASSERT( 0 );
    }

    // Free any scripts allocated
    dfs_FreeScripts();
}

//==============================================================================
