//==============================================================================
//
//  cdfsBuild.cpp
//
//==============================================================================
//
//  CD File System Builder
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "x_time.hpp"
//#include "x_string2.hpp"
//#include "x_array.hpp"

//#include "ps2_cdfs.hpp"

#include <stdio.h>

//==============================================================================
//  STRUCTURES
//==============================================================================

#define CDFS_VERSION    1                           // *** INCREMENT THIS EACH TIME STRUCT CHANGES ***
#define CDFS_MAGIC      'CDFS'                      // Magic Number

struct cdfsHeader
{
    u32         Magic;                              // Magic string CDFS
    s32         Version;                            // Version Number
    s32         SectorSize;                         // Sector size in bytes
    s32         RecommendedCacheSize;               // Recommended cache size
    s32         FirstSectorOffset;                  // Offset to first data sector in xcdfs file
    s32         TotalSectors;                       // Total sectors in filesystem
    s32         FileTableLength;                    // Length of FileTable in bytes
    s32         FileTableEntries;                   // Length of FileTable in entries
    s32         StringTableLength;                  // Length of StringTable in bytes
    s32         StringTableEntries;                 // Length of StringTable in entries
};

struct cdfsEntry
{
    s32         FileNameOffset;                     // Offset into StringTable
    s32         DirNameOffset;                      // Offset into StringTable
    s32         StartSector;                        // Start sector for file data
    s32         Length;                             // Length of file in bytes
};

struct FileTableEntry
{
    xstring PathName;
    s32     Length;

    s32     FileNameOffset;
    s32     DirNameOffset;
    s32     StartSector;
};

//==============================================================================
//  VARIABLES
//==============================================================================

#define FILE_BUFFER_SIZE    (16*1024*1024)
#define PAD_BUFFER_SIZE     (2048)

static s32                      ScriptIndex = 0;

static s32                      SectorSize;
static s32                      CacheSize;

static xarray<FileTableEntry*>  FileTable;

static xstring                  OutputPathName;

static xstring                  StringTable('\000') ;
static s32                      StringTableEntries;

static s32                      TotalDataSize = 0;
static s32                      TotalDataWritten = 0;
static u8                       FileBuffer[FILE_BUFFER_SIZE];
static  u8                      PadBuffer[PAD_BUFFER_SIZE];
//==============================================================================

xstring ReadLine( xstring& String )
{
    s32 StartIndex = ScriptIndex;
    xstring Line;

    while( (ScriptIndex < String.GetLength()) && (String[ScriptIndex] != '\n') )
    {
        ScriptIndex++;
    }
    Line = String.Mid( StartIndex, ScriptIndex-StartIndex );
    ScriptIndex++;

    // Strip Leading and Trailing whitespace
    s32 Index1 = 0;
    s32 Index2 = Line.GetLength()-1;
    while( x_isspace(Line[Index1]) && (Index1<Index2) ) Index1++;
    while( x_isspace(Line[Index2]) && (Index1<Index2) ) Index2--;
    Line = Line.Mid( Index1, Index2-Index1+1 );

    // Return Line
    return Line;
}

//==============================================================================

xbool ReadScript( const char* pScriptName )
{
    xbool   Success = FALSE;

    // Load script file
    xstring Script;
    if( Script.LoadFile( pScriptName ) )
    {
        // Read script header
//        OutputPathName = ReadLine( Script );
//        SectorSize     = x_atoi(ReadLine(Script));
//        CacheSize      = x_atoi(ReadLine(Script));
        OutputPathName = "FILES.DAT";
        SectorSize = 2048;
        CacheSize = 128*1024;

        // Loop through all files
        while( ScriptIndex < Script.GetLength() )
        {
            // Read File Entry and convert forward to backward slashes
            xstring PathName = ReadLine( Script );
            {
                s32 i;
                for( i=0 ; i<PathName.GetLength() ; i++ )
                {
                    if( PathName[i] == '/' ) PathName.SetAt(i, '\\');
                }
            }

            if( PathName.GetLength() > 0 )
            {
                // Open file
                X_FILE* pFile = x_fopen( PathName, "rb" );
                if( pFile )
                {
                    // Create New FileTableEntry
                    FileTableEntry* pEntry = new FileTableEntry; 
                    ASSERT( pEntry );
                    if( pEntry )
                    {
                        // Get File Length
                        x_fseek( pFile, 0, X_SEEK_END );
                        s32 Len = x_ftell( pFile );
                        x_fseek( pFile, 0, X_SEEK_SET );

                        // Setup Structure
                        pEntry->PathName = PathName;
                        pEntry->Length   = Len;

                        // Add Entry to Array
                        FileTable.Append() = pEntry;

                        // Display details
//                        x_printf( "FILE:  %10d bytes - \"%s\"\n", pEntry->Length, (char*)pEntry->PathName );
                    }

                    // Close file
                    x_fclose( pFile );
                }
                else
                {
                    // Display error on file open
                    x_printf( "ERROR: Can't open File \"%s\"\n", (const char*)PathName );
                }
            }
        }

        x_printf( "\n" );

        // Success!
        Success = TRUE;
    }
    else
    {
        // Failed to open script file
        x_printf( "ERROR: loading script file\n" );
    }

    return Success;
}

//==============================================================================

void SplitPathName( xstring PathName, xstring& Path, xstring& File )
{
    s32 Index = PathName.GetLength()-1;
    while( (Index >= 0) && (PathName[Index] != '\\') )
        Index--;

    if( Index >= 0 )
        Path = PathName.Left( Index );
    else
        Path = "";

    File = PathName.Right( PathName.GetLength() - (Index+1) );

//    x_printf( "'%s' - '%s':'%s'\n", (char*)PathName, (char*)Path, (char*)File );
}

//==============================================================================

s32 AddString( xstring String )
{
    s32 Index = 0;

    // Return Immediately if empty string
    if( String.GetLength() == 0 )
        return 0;

    // All string in table will be uppercase
    String.MakeUpper();

    // Search Table for string, if not found then append it
    while( Index < (StringTable.GetLength()-String.GetLength()) )
    {
        // Compare Strings
        if( (StringTable.Mid( Index, String.GetLength() ) == String ) &&
            ( StringTable[Index+String.GetLength()] == '\000' ) )
        {
            // Found it
//            x_printf( "Found: '%s'\n", ((char*)StringTable)+Index );
            return Index;
        }
        Index += x_strlen( ((const char*)StringTable)+Index ) + 1;
    }

    // String not found, so add it
    Index = StringTable.GetLength();
    StringTable.Insert( Index, String );
    StringTable.Insert( StringTable.GetLength(), '\000' );
    StringTableEntries++;
//    x_printf( "Added: '%s'\n", (char*)String );

    // Return index of string within table
    return Index;
}

//==============================================================================
//  main
//==============================================================================

int main( int argc, char** argv )
{
    s32 CurrentSector = 0;
    s32 i;
    s32 j;
    xtimer time;

    // Parse Args
    if( argc != 2 )
    {
        x_printf( "Usage: cdfsBuild <script>\n" );
        return 0;
    }

    // Read script and Create FileTable
    time.Reset();
    time.Start();
    x_printf("Reading script...");
    xbool Success = ReadScript( argv[1] );

    time.Stop();
    x_printf("Done in %2.2f seconds.\n",time.ReadSec());

    time.Reset();
    time.Start();
    x_printf("Building file system tables...");
    // Make a pass through the files setting up data to create cdfs
    for( i=0 ; i<FileTable.GetCount() ; i++ )
    {
        // Get pointer to file entry
        FileTableEntry* pFile = FileTable[i];
        ASSERT( pFile );

        // Setup Sector Offset for file
        pFile->StartSector = CurrentSector;
        CurrentSector += (pFile->Length+SectorSize-1) / SectorSize;
        TotalDataSize += pFile->Length;

        // Seperate Path and File & Add to Table
        xstring Path;
        xstring File;
        SplitPathName( pFile->PathName, Path, File );
        pFile->DirNameOffset  = AddString( Path );
        pFile->FileNameOffset = AddString( File );
    }

    time.Stop();

    x_printf("Done in %2.2f seconds.\n",time.ReadSec());

    // Open output file

    time.Reset();
    time.Start();
    X_FILE* pFile = x_fopen( OutputPathName, "wb" );
    if( pFile )
    {
        cdfsHeader  Header;
        s32         nBytes;
        s32         PadBytes;
        s32         HeaderLen         = sizeof(cdfsHeader);
        s32         FileTableLen      = FileTable.GetCount() * sizeof(cdfsEntry);
        s32         StringTableLen    = StringTable.GetLength() ;
        s32         FirstSectorOffset;
        s32         FirstSectorOffsetPadded;

        // Set up write buffering with a large buffer size.
        setvbuf((FILE *)pFile,NULL,_IOFBF,48 * 1048576);
        // Get First Sector Offset, rounded up to next sector
        FirstSectorOffset = HeaderLen + FileTableLen + StringTableLen;
        FirstSectorOffsetPadded = FirstSectorOffset;
        if( (FirstSectorOffset % SectorSize) > 0 )
            FirstSectorOffsetPadded += SectorSize - (FirstSectorOffset % SectorSize);

        // Build and write cdfs Header
        Header.Magic                = CDFS_MAGIC;
        Header.Version              = CDFS_VERSION;
        Header.SectorSize           = SectorSize;
        Header.RecommendedCacheSize = CacheSize;
        Header.FirstSectorOffset    = FirstSectorOffsetPadded;
        Header.TotalSectors         = CurrentSector;
        Header.FileTableLength      = FileTable.GetCount() * sizeof(cdfsEntry);
        Header.FileTableEntries     = FileTable.GetCount();
        Header.StringTableLength    = StringTable.GetLength();
        Header.StringTableEntries   = StringTableEntries;

/*
        // Dump cdfsHeader details
        x_printf( "CDFS File            = \"%s\"\n", (char*)OutputPathName );
        x_printf( "\n" );
        x_printf( "Magic                = 0x%08X\n", Header.Magic );
        x_printf( "Version              = %d\n", Header.Version );
        x_printf( "SectorSize           = %d\n", Header.SectorSize );
        x_printf( "RecommendedCacheSize = %d\n", Header.RecommendedCacheSize );
        x_printf( "FirstSectorOffset    = %d\n", Header.FirstSectorOffset );
        x_printf( "TotalSectors         = %d\n", Header.TotalSectors );
        x_printf( "FileTableLength      = %d\n", Header.FileTableLength );
        x_printf( "FileTableEntries     = %d\n", Header.FileTableEntries );
        x_printf( "StringTableLength    = %d\n", Header.StringTableLength );
        x_printf( "StringTableEntries   = %d\n", Header.StringTableEntries );
        x_printf( "\n" );
*/

        // Write cdfsHeader
        nBytes = x_fwrite( &Header, 1, sizeof(cdfsHeader), pFile );
        if( nBytes != sizeof(cdfsHeader) )
        {
            x_printf( "ERROR: writing Header\n" );
        }

        // Write File Table
        for( i=0 ; i<FileTable.GetCount() ; i++ )
        {
            cdfsEntry   Entry;

            // Build Entry
            Entry.DirNameOffset  = FileTable[i]->DirNameOffset;
            Entry.FileNameOffset = FileTable[i]->FileNameOffset;
            Entry.StartSector    = FileTable[i]->StartSector;
            Entry.Length         = FileTable[i]->Length;

            // Write Entry
            nBytes = x_fwrite( &Entry, 1, sizeof(cdfsEntry), pFile );
            if( nBytes != sizeof(cdfsEntry) )
            {
                x_printf( "ERROR: writing FileTable Entry\n" );
            }
        }

        // Write String Table
        nBytes = x_fwrite( (const char*)StringTable, 1, StringTable.GetLength(), pFile );
        if( nBytes != StringTable.GetLength() )
        {
            x_printf( "ERROR: writing StringTable\n" );
        }

        // Pad out File Data
        for( j=0 ; j<(FirstSectorOffsetPadded - FirstSectorOffset) ; j++ )
        {
            u8 Pad = 0;
            if( x_fwrite( &Pad, 1, 1, pFile ) != 1 )
            {
                x_printf( "ERROR: writing to file \"%s\"\n", OutputPathName );
            }
        }

        x_memset(PadBuffer,0,PAD_BUFFER_SIZE);
        // Write File Data
        for( i=0 ; i<FileTable.GetCount() ; i++ )
        {
//            x_printf( "%s - %08x\n", (const char*)FileTable[i]->PathName, (x_ftell(pFile)-0x1000)/0x800 );

            // Open File
            X_FILE* pSrcFile = x_fopen( FileTable[i]->PathName, "rb" );
            if( pSrcFile )
            {
                s32 BytesLeft = FileTable[i]->Length;

                // Copy File to output file
                do
                {
                    static s32  LastPercentageComplete = -1;
                    s32         PercentageComplete;
                    s32         BytesToCopy = MIN( BytesLeft, FILE_BUFFER_SIZE );

                    // Read Data
                    nBytes = x_fread( FileBuffer, 1, BytesToCopy, pSrcFile );
                    if( nBytes != BytesToCopy )
                    {
                        x_printf( "ERROR: reading from file \"%s\"\n", FileTable[i]->PathName );
                    }

                    // Write Data
                    nBytes = x_fwrite( FileBuffer, 1, BytesToCopy, pFile );
                    if( nBytes != BytesToCopy )
                    {
                        x_printf( "ERROR: writing to file \"%s\"\n", OutputPathName );
                    }

                    // Decrement Bytes left to copy
                    BytesLeft -= BytesToCopy;

                    // Increment TotalDataWritten
                    TotalDataWritten += BytesToCopy;

                    // Display % complete
                    PercentageComplete = (s32)(((f64)TotalDataWritten * 100) / TotalDataSize);
                    if( PercentageComplete != LastPercentageComplete )
                    {
                        LastPercentageComplete = PercentageComplete;
                        x_printf( "Building CDFS File - %3d%%\r", PercentageComplete );
                    }
                } while( BytesLeft > 0 );

                // Pad output to sector boundary
                PadBytes = ((FileTable[i]->Length % SectorSize) > 0) ? (SectorSize - (FileTable[i]->Length % SectorSize)) : 0;
                while (PadBytes)
                {
                    s32 Length;

                    Length = MIN(PadBytes,PAD_BUFFER_SIZE);

                    if( x_fwrite( PadBuffer, 1, Length, pFile ) != Length )
                    {
                        x_printf( "ERROR: writing to file \"%s\"\n", OutputPathName );
                    }
                    PadBytes -= Length;
                }

                // Close file
                x_fclose( pSrcFile );
            }
            else
            {
                x_printf( "ERROR: can't open \"%s\" for reading\n", FileTable[i]->PathName );
            }
        }

        // Close file
        x_fclose( pFile );

        // Print done
        x_printf( "\n" );
        x_printf( "\n" );
        time.Stop();
        x_printf( "Done in %2.2f seconds!\n",time.ReadSec() );
    }
    else
    {
        // Error opening file for output
        x_printf( "ERROR: opening \"%s\" for output\n", OutputPathName );
    }
}
