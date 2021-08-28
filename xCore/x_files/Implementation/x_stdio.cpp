//==============================================================================
//
//  x_stdio.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_STDIO_HPP
#include "..\x_stdio.hpp"
#endif

#ifndef X_STRING_HPP
#include "..\x_string.hpp"
#endif

#ifndef X_PLUS_HPP
#include "..\x_plus.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "..\x_debug.hpp"
#endif

#ifndef X_CONTEXT_HPP
#include "..\x_context.hpp"
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_PC
#include <stdio.h>
#include <windows.h>
#endif

#ifdef TARGET_XBOX
#include <xtl.h>
#endif

#ifdef TARGET_PS2
#include <sifdev.h>
#include <stdio.h>
#endif

#ifdef TARGET_GCN
#ifdef VENDOR_SN
  #include <libsn.h>
#endif
#include <dolphin.h>
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#if (defined(TARGET_PS2) || defined(TARGET_GCN) )
#define ENABLE_LOGGING
#endif

#ifdef TARGET_PC
#define USE_ANSI_IO_TEXT
#endif

//#define CAPTURE_FILE_NAMES

//==============================================================================
//  VARIABLES
//==============================================================================
                                    
static     open_fn*     s_pOpen     = NULL;
static    close_fn*     s_pClose    = NULL;
static     read_fn*     s_pRead     = NULL;
static    write_fn*     s_pWrite    = NULL;
static     seek_fn*     s_pSeek     = NULL;
static     tell_fn*     s_pTell     = NULL;
static    flush_fn*     s_pFlush    = NULL;
static      eof_fn*     s_pEOF      = NULL;
static   length_fn*     s_pLength   = NULL;

static    print_fn*     s_pPrint    = NULL;
static print_at_fn*     s_pPrintAt  = NULL;

//==============================================================================

#if defined(ENABLE_LOGGING)
xstring*    s_LoadMapLog    = NULL;
xbool       s_LoadMapEnabled= FALSE;
#endif

#ifdef CAPTURE_FILE_NAMES
#define NUM_FILE_NAME_ENTRIES 16
struct file_name_entry
{
    X_FILE* fp;
    char    FileName[256];
    char    Mode[8];
};
file_name_entry s_FileNameEntry[NUM_FILE_NAME_ENTRIES];
#endif

void AddToLoadMap(const char* pFilename);
//==============================================================================
#ifdef CAPTURE_FILE_NAMES

void AddFileNameEntry( X_FILE* fp, const char* pFileName, const char* pMode )
{
    if( fp )
    {
        s32 i;

        // Find an empty slot and add entry
        for( i=0; i<NUM_FILE_NAME_ENTRIES; i++ )
        if( s_FileNameEntry[i].fp == NULL )
            break;

        if( i<NUM_FILE_NAME_ENTRIES )
        {
            s_FileNameEntry[i].fp = fp;
            x_strncpy(s_FileNameEntry[i].FileName,pFileName,256);
            x_strncpy(s_FileNameEntry[i].Mode,pMode,8);
        }
    }
}

//==============================================================================

void DelFileNameEntry( X_FILE* fp )
{
    if( fp )
    {
        s32 i;

        // Find a match
        for( i=0; i<NUM_FILE_NAME_ENTRIES; i++ )
        if( s_FileNameEntry[i].fp == fp )
        {
            s_FileNameEntry[i].fp = NULL;
            s_FileNameEntry[i].FileName[0] = 0;
            s_FileNameEntry[i].Mode[0] = 0;
            break;
        }
    }
}

//==============================================================================

const file_name_entry* FindFileNameEntry( X_FILE* fp )
{
    if( fp )
    {
        s32 i;

        // Find a match
        for( i=0; i<NUM_FILE_NAME_ENTRIES; i++ )
        if( s_FileNameEntry[i].fp == fp )
            return &s_FileNameEntry[i];

        return NULL;
    }

    return NULL;
}
//==============================================================================

void DisplayFileNameEntries( void )
{
    s32 i;

    x_DebugMsg("FILES STILL OPEN!!!\n");

    // Find a match
    for( i=0; i<NUM_FILE_NAME_ENTRIES; i++ )
    if( s_FileNameEntry[i].fp != NULL )
    {
        x_DebugMsg("FILE: %s\n",s_FileNameEntry[i].FileName);
    }
}

#endif

//==============================================================================
//  CUSTOM FILE I/O AND TEXT OUT HOOK FUNCTIONS
//==============================================================================

void x_SetFileIOHooks(  open_fn*  pOpen,
                       close_fn*  pClose,
                        read_fn*  pRead,
                       write_fn*  pWrite,
                        seek_fn*  pSeek,
                        tell_fn*  pTell,
                       flush_fn*  pFlush,
                         eof_fn*  pEOF,
                      length_fn*  pLength )
{
    s_pOpen   = pOpen; 
    s_pClose  = pClose;
    s_pRead   = pRead; 
    s_pWrite  = pWrite;
    s_pSeek   = pSeek; 
    s_pTell   = pTell; 
    s_pFlush  = pFlush;
    s_pEOF    = pEOF;
    s_pLength = pLength;
}

//==============================================================================

void x_GetFileIOHooks    (     open_fn*  &pOpen,
                              close_fn*  &pClose,
                               read_fn*  &pRead,
                              write_fn*  &pWrite,
                               seek_fn*  &pSeek,
                               tell_fn*  &pTell,
                              flush_fn*  &pFlush,
                                eof_fn*  &pEOF,
                             length_fn*  &pLength  )
{
    pOpen   = s_pOpen; 
    pClose  = s_pClose;
    pRead   = s_pRead; 
    pWrite  = s_pWrite;
    pSeek   = s_pSeek; 
    pTell   = s_pTell; 
    pFlush  = s_pFlush;
    pEOF    = s_pEOF;
    pLength = s_pLength;
}

//==============================================================================

void x_SetPrintHook( print_fn* pPrint )
{
    s_pPrint = pPrint;
}

//==============================================================================

void x_SetPrintAtHook( print_at_fn* pPrintAt )
{
    s_pPrintAt = pPrintAt;
}

//==============================================================================
//  TEXT OUTPUT FUNCTIONS
//==============================================================================

s32 x_printf( const char* pFormatStr, ... )
{
    s32         NChars;
    x_va_list   Args;
    x_va_start( Args, pFormatStr );

    xvfs XVFS( pFormatStr, Args );
    NChars = *(((s32*)(const char*)XVFS)-1);
    
    if( s_pPrint )  
        s_pPrint( XVFS );

    return( NChars );
}

//==============================================================================

s32 x_printfxy( s32 X, s32 Y, const char* pFormatStr, ... )
{
    s32         NChars;
    x_va_list   Args;
    x_va_start( Args, pFormatStr );

    xvfs XVFS( pFormatStr, Args );
    NChars = *(((s32*)(const char*)XVFS)-1);

    if( s_pPrintAt )  
        s_pPrintAt( XVFS, X, Y );

    return( NChars );
}

//==============================================================================

s32 x_sprintf( char* pStr, const char* pFormatStr, ... )
{
    x_va_list   Args;
    x_va_start( Args, pFormatStr );

    return( x_vsprintf( pStr, pFormatStr, Args ) );
}

//==============================================================================
// Function x_vsprintf is implemented in file x_vsprintf.cpp.
//==============================================================================

//==============================================================================
//  STANDARD FILE I/O FUNCTIONS
//==============================================================================

X_FILE* x_fopen( const char* pFileName, const char* pMode ) 
{
    CONTEXT( "x_fopen" );

    X_FILE* fp;

    ASSERT( s_pOpen );
    fp = s_pOpen(pFileName,pMode);

    if (fp)
    {
        AddToLoadMap(pFileName);
    }

#ifdef CAPTURE_FILE_NAMES
    if( fp )
    {
        AddFileNameEntry(fp,pFileName,pMode);
    }
#endif

    return( fp );
}

//==============================================================================
void x_EnableLoadMap (xbool Flag)
{
    (void)Flag;
#if defined(ENABLE_LOGGING)
    s_LoadMapEnabled = Flag;
#endif
}

//==============================================================================
void x_DumpLoadMap(const char* pFilename)
{
    (void)pFilename;
#if defined(ENABLE_LOGGING)
    if (s_LoadMapEnabled)
    {
        if (pFilename)
            s_LoadMapLog->SaveFile( pFilename );
        s_LoadMapLog->Clear();
    }
#endif
}

//==============================================================================
void AddToLoadMap(const char* pFilename)
{
    (void)pFilename;
#if defined(ENABLE_LOGGING)
    if (s_LoadMapEnabled)
    {
        if (x_stristr(pFilename,".audiopkg"))
            return;
        if( !s_LoadMapLog )
            s_LoadMapLog = new xstring(128*1024);
        //*s_LoadMapLog += xfs( "[%2.02f] %s\n", (f32)x_GetTimeSec(),pFilename );
        *s_LoadMapLog += xfs( "%s\n", pFilename );
    }
#endif
}
//==============================================================================

void x_fclose( X_FILE* pFile ) 
{
    CONTEXT( "x_fclose" );

    if( pFile == NULL )
        return;

#ifdef CAPTURE_FILE_NAMES
    DelFileNameEntry(pFile);
#endif

    ASSERT( s_pClose );
    s_pClose( pFile );
}

//==============================================================================

s32 x_fread( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile ) 
{
    CONTEXT( "x_fread" );

    s32 Bytes;
    
    if( Size*Count == 0 )
        return 0;

    ASSERT( pBuffer );
    ASSERT( s_pRead );
    Bytes = s_pRead( pFile, (byte*)pBuffer, (Size * Count) );
    return( Bytes / Size );
}

//==============================================================================

s32 x_fwrite( const void* pBuffer, s32 Size, s32 Count, X_FILE* pFile ) 
{
    CONTEXT( "x_fwrite" );

    s32 Bytes;
    
    if( Count*Size == 0 )
        return 0;

    ASSERT( pBuffer );
    ASSERT( s_pWrite );
    Bytes = s_pWrite( pFile, (byte*)pBuffer, (Size * Count) );
    return( Bytes / Size );
}

//==============================================================================

s32 x_fprintf( X_FILE* pFile, const char* pFormatStr, ... )
{
    CONTEXT( "x_fprintf" );

    s32         NChars;
    s32         Bytes;
    x_va_list   Args;
    x_va_start( Args, pFormatStr );

    ASSERT( s_pWrite );

    xstring String;
    String.FormatV( pFormatStr, Args );

    #ifdef TARGET_PS2
    // Replace \n with \r\n.
    {           
        s32 Index = 0;
        while( (Index < String.GetLength() ) && 
               ((Index = String.Find( '\n', Index )) != -1) )
        {
            String.Insert( Index, "\r" );
            Index += 2;
        }
    }
    #endif

    NChars = String.GetLength();
    
    Bytes = s_pWrite( pFile, (const byte*)(const char*)String, NChars );
    ASSERT( Bytes == NChars );

    return( Bytes );
}

//==============================================================================

s32 x_fflush( X_FILE* pFile ) 
{
    CONTEXT( "x_fflush" );

    ASSERT( s_pFlush );
    return( s_pFlush( pFile ) );
}

//==============================================================================

s32 x_fseek( X_FILE* pFile, s32 Offset, s32 Origin ) 
{
    CONTEXT( "x_fseek" );

    ASSERT( IN_RANGE( 0, Origin, 2 ) );
    ASSERT( s_pSeek );
    return( s_pSeek( pFile, Offset, Origin ) );
}

//==============================================================================

s32 x_ftell( X_FILE* pFile ) 
{
    CONTEXT( "x_ftell" );

    ASSERT( s_pTell );
    return( s_pTell( pFile ) );
}

//==============================================================================

s32 x_feof( X_FILE* pFile ) 
{
    CONTEXT( "x_feof" );

    ASSERT( s_pEOF );
    return( s_pEOF( pFile ) );
}

//==============================================================================

s32 x_fgetc( X_FILE* pFile ) 
{
    CONTEXT( "x_fgetc" );

    char Char;
    s32  Result;
    s32  Bytes;

    Bytes = x_fread( &Char, 1, 1, pFile );
    if( Bytes == 0 )    Result = X_EOF;
    else                Result = Char;
    return( Result );
}

//==============================================================================

s32 x_fputc( s32 C, X_FILE* pFile ) 
{
    CONTEXT( "x_fputc" );

    char Char = (char)C;
    s32  Result;
    s32  Bytes;

    Bytes = x_fwrite( &Char, 1, 1, pFile );
    if( Bytes == 0 )    Result = X_EOF;
    else                Result = Char;
    return( Result );
}

//==============================================================================

s32 x_flength( X_FILE* pFile ) 
{
    CONTEXT( "x_flength" );

    ASSERT( s_pLength );
    return( s_pLength( pFile ) );
}

//==============================================================================
//  FILE I/O AND TEXT OUTPUT FUNCTIONS WRITTEN USING STANDARD ANSI C
//==============================================================================

//==============================================================================
#ifdef USE_ANSI_IO_TEXT
//==============================================================================

X_FILE* ansi_Open( const char* pFileName, const char* pMode )
{
    return( (X_FILE*)fopen( pFileName, pMode ) );
}

//==============================================================================

void ansi_Close( X_FILE* pFile )
{
    fclose( (FILE*)pFile );
}

//==============================================================================

s32 ansi_Tell( X_FILE* pFile );
xbool ansi_Length( X_FILE* pFile );

s32 ansi_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
/*
#ifdef X_ASSERT
    if( ansi_Tell(pFile)+Bytes > ansi_Length(pFile) )
    {
        #ifdef CAPTURE_FILE_NAMES
        const file_name_entry* pEntry = FindFileNameEntry( pFile );
        if( pEntry )
        {
            ASSERTS(FALSE,xfs("Read past end of file <%s>",pEntry->FileName));
        }
        #endif

        ASSERTS(FALSE,"Read past end of file");
    }
#endif
*/
    return( fread( pBuffer, 1, Bytes, (FILE*)pFile ) );
}

//==============================================================================

s32 ansi_Write( X_FILE* pFile, const byte* pBuffer, s32 Bytes )
{
    return( fwrite( pBuffer, 1, Bytes, (FILE*)pFile ) );
}

//==============================================================================

s32 ansi_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    return( fseek( (FILE*)pFile, Offset, Origin ) );
}

//==============================================================================

s32 ansi_Tell( X_FILE* pFile )
{
    return( ftell( (FILE*)pFile ) );
}

//==============================================================================

s32 ansi_Flush( X_FILE* pFile )
{
    return( fflush( (FILE*)pFile ) );
}

//==============================================================================

xbool ansi_EOF( X_FILE* pFile )
{
    return( feof((FILE*)pFile) ? 1 : 0 );
}

//==============================================================================

xbool ansi_Length( X_FILE* pFile )
{
    s32 Length;
    s32 Cursor;
    
    Cursor = ftell( (FILE*)pFile );   fseek( (FILE*)pFile,      0, SEEK_END );
    Length = ftell( (FILE*)pFile );   fseek( (FILE*)pFile, Cursor, SEEK_SET );

    return( Length );
}

//==============================================================================

void ansi_Print( const char* pString )
{
    printf( "%s", pString );
}

//==============================================================================

void ansi_PrintAt( const char* pString, s32 X, s32 Y )
{
#ifdef TARGET_PC
    HANDLE                      hConOut;
    CONSOLE_SCREEN_BUFFER_INFO  SBI;
    COORD                       Coord;

    hConOut = GetStdHandle( STD_OUTPUT_HANDLE );
    if( hConOut == INVALID_HANDLE_VALUE )
        return;

    Coord.X = (short)X;
    Coord.Y = (short)Y;

    GetConsoleScreenBufferInfo( hConOut, &SBI );
    SetConsoleCursorPosition  ( hConOut, Coord );
#endif

    printf( "%s", pString );

#ifdef TARGET_PC
    SetConsoleCursorPosition  ( hConOut, SBI.dwCursorPosition );
#endif
}

//==============================================================================
#endif // USE_ANSI_IO_TEXT
//==============================================================================








//==============================================================================
//  FILE I/O AND TEXT OUTPUT FUNCTIONS WRITTEN FOR XBOX
//==============================================================================

//==============================================================================
#ifdef TARGET_XBOX

#define IO_DEVICE_FILENAME_LIMIT 1024

//==============================================================================
static void CleanFilename( char* pClean, const char* pFilename )
{
    // Gotta fit.
    ASSERT( x_strlen(pFilename) < IO_DEVICE_FILENAME_LIMIT );

    // Move to end of string.
    pClean += x_strlen( pClean );

    // Now clean it.
    while( *pFilename )
    {
        if( (*pFilename == '\\') || (*pFilename == '/') )
        {
            *pClean++ = '\\';
            pFilename++;

            while( *pFilename && ((*pFilename == '\\') || (*pFilename == '/')) )
                pFilename++;
        }
        else
        {
            *pClean++ = *pFilename++;
        }
    }

    // Terminate it.
    *pClean = 0;
}
//==============================================================================

X_FILE* xbox_dev_Open( const char* pFileName, const char* pMode )
{
    HANDLE  Handle  = INVALID_HANDLE_VALUE;
    xbool   Read    = FALSE;
    xbool   Write   = FALSE;
    xbool   Append  = FALSE;

    // Pick through the Mode characters.
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        ++pMode;
    }
    
    ASSERT( Read || Write );

    // Sanitise filename
    char* SanitisedPath = (char*)pFileName;
    {
        static char CleanFile[IO_DEVICE_FILENAME_LIMIT];
        CleanFilename ( CleanFile, (char *)pFileName );
        SanitisedPath = CleanFile;
        strupr( CleanFile );
    }

    // Build parameter to CreateFile based on values found in Mode.
    
    if( Read  )
    {
        Handle = CreateFile( SanitisedPath, GENERIC_READ, FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    }
    else if( Write && !Append )
    {
        Handle = CreateFile( SanitisedPath, GENERIC_WRITE, FILE_SHARE_WRITE,
                             NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    }
    else if( Write && Append )
    {
        Handle = CreateFile( SanitisedPath, GENERIC_WRITE, FILE_SHARE_WRITE,
                             NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    }
    else if( Append)
    {
        Handle = CreateFile( SanitisedPath, GENERIC_WRITE, FILE_SHARE_WRITE,
                             NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    }

    // If no valid handle, return NULL.
    if( Handle == INVALID_HANDLE_VALUE )
    {
        #ifdef _DEBUG
            DWORD dwErr=GetLastError();
            (void)dwErr;
        #endif
        return NULL;
    }

    // Seek to end on an append
    if( Append )
    {
        SetFilePointer( Handle, 0, 0, FILE_END );
    }

    // Allocate an X_FILE handle
    HANDLE* pHandle = (HANDLE*)x_malloc( sizeof(HANDLE) );
    if( pHandle )
    {
        *pHandle = Handle;
    }
    else
    {
        CloseHandle( Handle );
    }
    return (X_FILE*)pHandle;
}

//==============================================================================

void xbox_dev_Close( X_FILE* pFile )
{
    CloseHandle( *(HANDLE*)pFile );
    x_free( pFile );
}

//==============================================================================

s32 xbox_dev_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    DWORD BytesRead;
    ReadFile( *(HANDLE*)pFile, pBuffer, Bytes, &BytesRead, NULL );
    return BytesRead;
}

//==============================================================================

s32 xbox_dev_Write( X_FILE* pFile, const byte* pBuffer, s32 Bytes )
{
    DWORD BytesWritten;
    WriteFile( *(HANDLE*)pFile, pBuffer, Bytes, &BytesWritten, NULL );
    return BytesWritten;
}

//==============================================================================

s32 xbox_dev_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    DWORD   Where = 0;
    DWORD   Result;
    
    switch( Origin )
    {
        case X_SEEK_SET: Where = FILE_BEGIN;   break;
        case X_SEEK_CUR: Where = FILE_CURRENT; break;
        case X_SEEK_END: Where = FILE_END;     break;
    }

    Result = SetFilePointer( *(HANDLE*)pFile, Offset, NULL, Where );

    if( Result != -1 )  return(  0 );
    else                return( -1 );
}

//==============================================================================

s32 xbox_dev_Tell( X_FILE* pFile )
{
    DWORD Position = SetFilePointer( *(HANDLE*)pFile, 0, NULL, FILE_CURRENT );
    return (s32)Position;
}

//==============================================================================

s32 xbox_dev_Flush( X_FILE* )
{
    return( 0 );
}

//==============================================================================

xbool xbox_dev_EOF( X_FILE* pFile )
{
    // We have to do this the hard way.
    //  (1)  Save the current position.
    //  (2)  Seek to the end of the file.
    //  (3)  Save the 'end' position.
    //  (4)  Seek back to the saved position.
    //  (5)  Return comparison between original position and end position.
    //
    // NOTE:  Since this implementation is clearly inefficient, loops
    //        such as "while( !x_feof(f) )" are discouraged.

    s32 Pos;
    s32 End;

    Pos = SetFilePointer( *(HANDLE*)pFile, 0, NULL, FILE_CURRENT );
    End = SetFilePointer( *(HANDLE*)pFile, 0, NULL, FILE_END     );
    SetFilePointer( *(HANDLE*)pFile, Pos, NULL, FILE_BEGIN );

    return( Pos >= End );
}

//==============================================================================

s32 xbox_dev_Length( X_FILE* pFile )
{
    return GetFileSize( *(HANDLE*)pFile, NULL );
}

//==============================================================================
//==============================================================================

void xbox_dev_Print( const char* pString )
{
    OutputDebugString( pString );
}

//==============================================================================

void xbox_dev_PrintAt( const char* pString, s32 X, s32 Y )
{
    X = Y   = 0;        // This is here to surpress a warning.
    pString = NULL;     // This is here to surpress a warning.
}

//==============================================================================
#endif // TARGET_XBOX
//==============================================================================







//==============================================================================
//  FILE I/O AND TEXT OUTPUT FUNCTIONS WRITTEN FOR PLAYSTATION 2
//==============================================================================

//==============================================================================
#if defined(TARGET_PS2) && defined(TARGET_DEV)
//==============================================================================

X_FILE* ps2_dev_Open( const char* pFileName, const char* pMode )
{
    s32   Handle;
    xbool Read    = FALSE;
    xbool Write   = FALSE;
    xbool Append  = FALSE;
    s32   OpenArg = 0;

    xfs FileName( "host0:%s", pFileName );

    // Pick through the Mode characters.
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        ++pMode;
    }
    
    ASSERT( Read || Write );

    // Build parameter to sceOpen based on values found in Mode.
    
    if( Read  )             OpenArg |= SCE_RDONLY;
    if( Write )             OpenArg |= SCE_WRONLY;
//    if( Write )             OpenArg |= SCE_CREAT;
    if( Append)             OpenArg |= SCE_APPEND;
    if( Write && !Append )  OpenArg |= SCE_TRUNC|SCE_CREAT;

    // For reference, here are the defines for the bit argument to sceOpen().
    //          
    //  #define SCE_RDONLY      0x0001
    //  #define SCE_WRONLY      0x0002
    //  #define SCE_RDWR        0x0003
    //  #define SCE_NBLOCK      0x0010  // Non-Blocking I/O 
    //  #define SCE_APPEND      0x0100  // append (writes guaranteed at the end) 
    //  #define SCE_CREAT       0x0200  // open with file create 
    //  #define SCE_TRUNC       0x0400  // open with truncation 
    //  #define SCE_NOBUF       0x4000  // no device buffer and console interrupt 
    //  #define SCE_NOWAIT      0x8000  // asyncronous i/o 

    Handle = sceOpen( const_cast<char*>( (const char*)FileName ), OpenArg );    

    // If no valid handle, return NULL.
    if( Handle < 0 )
    {
        // If we open a file for append and it didn't exist, let's open it
        // for create instead
        if (Append)
        {
            OpenArg &= ~SCE_APPEND;
            OpenArg |= SCE_TRUNC|SCE_CREAT;
            Handle = sceOpen( const_cast<char*>( (const char*)FileName ), OpenArg );    
            if (Handle < 0)
                return NULL;
        }
        else
        {
            return( NULL );
        }

    }

    if (Append)
    {
        sceLseek(Handle,0,SCE_SEEK_END);
    }
        
    // We may need to manually seek to the end of the file for append.
    // Check the behavior.    
    
    // NOTE:  Since 0 (zero) is a valid file handle with the Sony library, we
    // simply increment the handle value on the way out so the caller does not
    // confuse it with NULL.  All other functions below must reduce the handle
    // by 1 before using it.
    Handle++;

    return( (X_FILE*)Handle );
}

//==============================================================================

void ps2_dev_Close( X_FILE* pFile )
{
    sceClose( ((s32)pFile)-1 );
}

//==============================================================================

s32 ps2_dev_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    return( sceRead( ((s32)pFile)-1, pBuffer, Bytes ) );
}

//==============================================================================

s32 ps2_dev_Write( X_FILE* pFile, const byte* pBuffer, s32 Bytes )
{
    return( sceWrite( ((s32)pFile)-1, const_cast<byte*>( pBuffer ), Bytes ) );
}

//==============================================================================

s32 ps2_dev_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    s32 Where = 0;
    s32 Result;
    
    switch( Origin )
    {
        case X_SEEK_SET: Where = SCE_SEEK_SET; break;
        case X_SEEK_CUR: Where = SCE_SEEK_CUR; break;
        case X_SEEK_END: Where = SCE_SEEK_END; break;
    }

    Result = sceLseek( ((s32)pFile)-1, Offset, Where );

    if( Result != -1 )  return(  0 );
    else                return( -1 );
}

//==============================================================================

s32 ps2_dev_Tell( X_FILE* pFile )
{
    return( sceLseek( ((s32)pFile)-1, 0, SCE_SEEK_CUR ) );
}

//==============================================================================

s32 ps2_dev_Flush( X_FILE* )
{
    return( 0 );
}

//==============================================================================

xbool ps2_dev_EOF( X_FILE* pFile )
{
    // We have to do this the hard way.
    //  (1)  Save the current position.
    //  (2)  Seek to the end of the file.
    //  (3)  Save the 'end' position.
    //  (4)  Seek back to the saved position.
    //  (5)  Return comparison between original position and end position.
    //
    // NOTE:  Since this implementation is clearly inefficient, loops
    //        such as "while( !x_feof(f) )" are discouraged.

    s32 Pos;
    s32 End;
    s32 FH = ((s32)pFile) - 1;

    Pos = sceLseek( FH, 0, SCE_SEEK_CUR );   sceLseek( FH,   0, SCE_SEEK_END );
    End = sceLseek( FH, 0, SCE_SEEK_CUR );   sceLseek( FH, Pos, SCE_SEEK_SET );

    return( Pos == End );
}

//==============================================================================

s32 ps2_dev_Length( X_FILE* pFile )
{
    // We have to do this the hard way.
    //  (1)  Save the current position.
    //  (2)  Seek to the end of the file.
    //  (3)  Save the 'end' position.
    //  (4)  Seek back to the saved position.
    //  (5)  Return the end position.
    //

    s32 Pos;
    s32 End;
    s32 FH = ((s32)pFile) - 1;

    Pos = sceLseek( FH, 0, SCE_SEEK_CUR );   sceLseek( FH,   0, SCE_SEEK_END );
    End = sceLseek( FH, 0, SCE_SEEK_CUR );   sceLseek( FH, Pos, SCE_SEEK_SET );

    return( End );
}

//==============================================================================
//==============================================================================

void ps2_dev_Print( const char* pString )
{
    #ifdef _MSC_VER
    pString = NULL;     // This is here to surpress a warning.
    #else
    printf( pString );
    #endif
}

//==============================================================================

void ps2_dev_PrintAt( const char* pString, s32 X, s32 Y )
{
    X = Y   = 0;        // This is here to surpress a warning.
    pString = NULL;     // This is here to surpress a warning.
}

//==============================================================================
#endif // TARGET_PS2, TARGET_DEV
//==============================================================================





//==============================================================================
//  FILE I/O AND TEXT OUTPUT FUNCTIONS WRITTEN FOR GAMECUBE
//==============================================================================

//==============================================================================
#ifdef TARGET_GCN_DEV
//==============================================================================
#define FILE_CACHE_SIZE      8192

struct open_file
{
    char        Buffer[FILE_CACHE_SIZE];
    s32         Handle;
    s32         Length;
    s32         Position;               // Position we internally think we are at
    s32         HostPosition;           // Position that the host knows we are at
    s32         BufferBase;             // Base host position for this buffer entry
    s32         BufferValid;            // # bytes of valid data in the buffer
};

static xbool            Init = FALSE;

X_FILE* gcn_dev_Open( const char* pFileName, const char* pMode )
{
    s32     Handle;
    open_file *pFile;

    if (!Init)
    {
        PCinit();
        Init = TRUE;
    }

    xbool Read    = FALSE;
    xbool Write   = FALSE;
    xbool Append  = FALSE;

    // Pick through the Mode characters.
    while( *pMode )
    {
        if( (*pMode == 'r') || (*pMode == 'R') )  Read   = TRUE;
        if( (*pMode == 'w') || (*pMode == 'W') )  Write  = TRUE;
        if( (*pMode == 'a') || (*pMode == 'A') )  Append = TRUE;
        ++pMode;
    }

    s32 Flags = 0;

    if ( Read && !Write)  Flags = 0;
    if (!Read &&  Write)  Flags = 1;
    if ( Read &&  Write)  Flags = 2;
    
    //
    //  Open it
    //
    if (Flags == 1)
    {
        Handle = PCcreat((char*)pFileName,0);
    }
    else
    {
        Handle = PCopen( (char*)pFileName, Flags, 0 );
    }
    if (Handle == -1)
    {
        x_DebugMsg("gcn_dev_Open: File '%s' not found\n",pFileName);
        return NULL;        
    }

    pFile = (open_file*)x_malloc(sizeof(open_file));
    ASSERT(pFile);

    pFile->Handle = Handle;
    pFile->BufferBase = 0;
    pFile->BufferValid = 0;
    pFile->Position = 0;
    pFile->HostPosition = 0;
    pFile->Length = PClseek(Handle,0,X_SEEK_END);
    PClseek(Handle,0,X_SEEK_SET);

    return( (X_FILE*)pFile );  
}

//==============================================================================

void gcn_dev_Close( X_FILE* pFile )
{
    open_file* pOpenFile;
    ASSERT( pFile );

    pOpenFile = (open_file*)pFile;

    PCclose(pOpenFile->Handle);
    x_free(pOpenFile);
}

//==============================================================================
static s32 CacheHits=0;
static s32 CacheMisses=0;
s32 gcn_dev_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    open_file* pOpenFile;
    s32 BytesRead;
    s32 BufferLength;
    s32 BufferPos;

    ASSERT( pFile );
    pOpenFile = (open_file*)pFile;

    BytesRead = 0;

/*
    PClseek(pOpenFile->Handle,pOpenFile->Position,X_SEEK_SET);
    BytesRead = PCread(pOpenFile->Handle,(char*)pBuffer,Bytes);
    pOpenFile->Position+=BytesRead;
    return BytesRead;
*/
    if (Bytes > (pOpenFile->Length - pOpenFile->Position) )
    {
        ASSERTS(FALSE,"Reading past EOF");
        Bytes = pOpenFile->Length - pOpenFile->Position; 
    }

    while (Bytes)
    {
        BufferPos     = pOpenFile->Position - pOpenFile->BufferBase;
        // If the buffer does not contain the data we're looking for, let's
        // just load the data block
        if ( (BufferPos < 0) || (BufferPos >= pOpenFile->BufferValid) )
        {
            CacheMisses++;
            if (pOpenFile->Position != pOpenFile->HostPosition)
            {
                PClseek(pOpenFile->Handle,pOpenFile->Position,X_SEEK_SET);
            }
            pOpenFile->BufferBase   = pOpenFile->Position;
            pOpenFile->BufferValid  = PCread(pOpenFile->Handle,pOpenFile->Buffer,FILE_CACHE_SIZE);
            pOpenFile->HostPosition = pOpenFile->BufferBase+pOpenFile->BufferValid;
        }
        else
        {
            BufferLength = pOpenFile->BufferValid - BufferPos;
            if (BufferLength == 0)
                break;
            if (BufferLength > Bytes)
                BufferLength = Bytes;

            CacheHits++;
            x_memcpy(pBuffer,pOpenFile->Buffer+BufferPos,BufferLength);

            Bytes               -= BufferLength;
            pOpenFile->Position += BufferLength;
            pBuffer             += BufferLength;
            BytesRead           += BufferLength;
        }
    }
    return BytesRead;
}

//==============================================================================

s32 gcn_dev_Write( X_FILE* pFile, const byte* pBuffer, s32 Bytes )
{
    open_file* pOpenFile;
    ASSERT( pFile );

    pOpenFile= (open_file*)pFile;

    s32 Written = PCwrite( pOpenFile->Handle, (char*)pBuffer, Bytes );
    (void)Written;
    ASSERT( Written == Bytes );
    
    return Bytes;
}

//==============================================================================

s32 gcn_dev_Seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    open_file* pOpenFile;
    ASSERT( pFile );
 
    pOpenFile = (open_file*)pFile;

    switch( Origin )
    {
        case X_SEEK_SET: 
            break;
        case X_SEEK_CUR: 
            Offset += pOpenFile->Position;
            break;
        case X_SEEK_END: 
            Offset = pOpenFile->Length + Offset;
            break;
        default:
            ASSERTS(FALSE,"Unknown seek origin passed to x_fseek");
    }

    if (Offset > pOpenFile->Length)
        Offset = pOpenFile->Length;

    if (Offset < 0)
        Offset = 0;

    pOpenFile->Position = Offset;
    return Offset;
}

//==============================================================================

s32 gcn_dev_Tell( X_FILE* pFile )
{
    open_file *pOpenFile;
    ASSERT( pFile );

    pOpenFile = (open_file*)pFile;

    return pOpenFile->Position;
}

//==============================================================================

s32 gcn_dev_Flush( X_FILE* )
{
    return( 0 );
}

//==============================================================================

xbool gcn_dev_EOF( X_FILE* pFile )
{
    open_file* pOpenFile;
    ASSERT( pFile );

    pOpenFile = (open_file*)pFile;

    return pOpenFile->Position >= pOpenFile->Length;
 
}

//==============================================================================

s32 gcn_dev_Length( X_FILE* pFile )
{
    open_file* pOpenFile;
    ASSERT( pFile );

    pOpenFile = (open_file*)pFile;

    return pOpenFile->Length;
}

//==============================================================================
//==============================================================================

void gcn_dev_Print( const char* pString )
{
    #ifdef _MSC_VER
    pString = NULL;     // This is here to surpress a warning.
    #else
    x_printf( pString );
    #endif
}

//==============================================================================

void gcn_dev_PrintAt( const char* pString, s32 X, s32 Y )
{
    X = Y   = 0;        // This is here to surpress a warning.
    pString = NULL;     // This is here to surpress a warning.
}

//==============================================================================
#endif // TARGET_GCN_DEV
//==============================================================================


//==============================================================================
#ifdef TARGET_PC
//==============================================================================

xbool   x_GetFileTime( const char* pFileName, u64& FileTime )
{/*
    xbool Result;
    FILETIME FT;
    
    FILE* pF;
    pF = fopen(pFileName,"r");
    if( !pF )
        return FALSE;
    x_memset(&FT, 0, sizeof(FILETIME) );
    Result = GetFileTime(pF, NULL, NULL, &FT);
    fclose(pF);

    x_memcpy( &FileTime, &FT, sizeof(u64) );
    return Result;
    */

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile( pFileName, &FindFileData);
  
    if( hFind == INVALID_HANDLE_VALUE )
        return FALSE;

    x_memcpy( &FileTime, &FindFileData.ftLastWriteTime, sizeof(u64) );
    FindClose(hFind);
  
    return TRUE;
}

//==============================================================================
#endif
//==============================================================================


//==============================================================================
//  INITIALIZATION AND SHUT DOWN FUNCTIONS
//==============================================================================

void x_IOInit( void )
{
#if defined(X_DEBUG) || defined(TARGET_PC)
#ifdef USE_ANSI_IO_TEXT
    x_SetFileIOHooks( ansi_Open, 
                      ansi_Close, 
                      ansi_Read, 
                      ansi_Write, 
                      ansi_Seek, 
                      ansi_Tell, 
                      ansi_Flush, 
                      ansi_EOF,
                      ansi_Length  );
    x_SetPrintHook  ( ansi_Print   );
    x_SetPrintAtHook( ansi_PrintAt );
#endif // USE_ANSI_IO_TEXT

#if defined(TARGET_XBOX)
    x_SetFileIOHooks( xbox_dev_Open, 
                      xbox_dev_Close, 
                      xbox_dev_Read, 
                      xbox_dev_Write, 
                      xbox_dev_Seek, 
                      xbox_dev_Tell, 
                      xbox_dev_Flush, 
                      xbox_dev_EOF,
                      xbox_dev_Length  );
    x_SetPrintHook  ( xbox_dev_Print   );
    x_SetPrintAtHook( xbox_dev_PrintAt );
#endif // TARGET_XBOX


#if defined(TARGET_PS2)
    x_SetFileIOHooks( ps2_dev_Open, 
                      ps2_dev_Close, 
                      ps2_dev_Read, 
                      ps2_dev_Write, 
                      ps2_dev_Seek, 
                      ps2_dev_Tell, 
                      ps2_dev_Flush, 
                      ps2_dev_EOF,
                      ps2_dev_Length  );
    x_SetPrintHook  ( ps2_dev_Print   );
    x_SetPrintAtHook( ps2_dev_PrintAt );
#endif // TARGET_PS2


#if defined(TARGET_GCN)
    x_SetFileIOHooks( gcn_dev_Open, 
                      gcn_dev_Close, 
                      gcn_dev_Read, 
                      gcn_dev_Write, 
                      gcn_dev_Seek, 
                      gcn_dev_Tell, 
                      gcn_dev_Flush, 
                      gcn_dev_EOF,
                      gcn_dev_Length  );
    x_SetPrintHook  ( gcn_dev_Print   );
    x_SetPrintAtHook( gcn_dev_PrintAt );
#endif // TARGET_GCN

#endif // X_DEBUG

#ifdef CAPTURE_FILE_NAMES
    for( s32 i=0; i<NUM_FILE_NAME_ENTRIES; i++ )
    {
        s_FileNameEntry[i].fp = NULL;
        s_FileNameEntry[i].FileName[0] = 0;
    }
#endif
}

//==============================================================================

void x_IOKill( void )
{
    x_SetFileIOHooks( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    x_SetPrintHook  ( NULL );
    x_SetPrintAtHook( NULL );
}

//==============================================================================
