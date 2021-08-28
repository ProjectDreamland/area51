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

//------------------------------------------------------------------------------

#ifdef TARGET_PC
#include <stdio.h>
#include <windows.h>
#endif

#ifdef TARGET_PS2_DEV
#include <sifdev.h>
#include <stdio.h>
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#if( defined(TARGET_PS2_DEV) && defined(X_DEBUG) && defined(bwatson) )    // 1 Creates x_fopen_Log string
#define ENABLE_LOGGING  1
#else
#define ENABLE_LOGGING  0
#endif

#ifdef TARGET_PC
#define USE_ANSI_IO_TEXT
#endif

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

xstring* x_fopen_log = NULL;

#if ENABLE_LOGGING
xbool    x_logging_enabled = TRUE;
#else
xbool    x_logging_enabled = FALSE;
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
    X_FILE *pHandle;

    ASSERT( s_pOpen );
    pHandle = s_pOpen (pFileName,pMode);

#if ENABLE_LOGGING
    // CJG: Logging for building CD Filesystem FILES.DAT.
    // BW 8/3/01 - Log file only if the open succeeds 
    if (pHandle && x_logging_enabled)
    {
        if( !x_fopen_log )
            x_fopen_log = new xstring(262144);
		
        *x_fopen_log += xfs( "%s\n", pFileName );
		// Just to make sure it doesn't die on a devkit, if the log file
		// gets to be too long, just clear it. We shouldn't really do this
		// as we'll lose files in the file list but a 2 meg log file is 
		// pretty damn huge.
		if (x_fopen_log->GetLength() > 2*1048576)
		{
			x_fopen_log->Clear();
		}
    }
#endif


    return pHandle;
}

//==============================================================================

void x_fclose( X_FILE* pFile ) 
{
    ASSERT( s_pClose );
    s_pClose( pFile );
}

//==============================================================================

s32 x_fread( void* pBuffer, s32 Size, s32 Count, X_FILE* pFile ) 
{
    s32 Bytes;
    ASSERT( pBuffer );
    ASSERT( Size  >  0 );
    ASSERT( Count >= 0 );
    ASSERT( s_pRead );
    Bytes = s_pRead( pFile, (byte*)pBuffer, (Size * Count) );
    return( Bytes / Size );
}

//==============================================================================

s32 x_fwrite( const void* pBuffer, s32 Size, s32 Count, X_FILE* pFile ) 
{
    s32 Bytes;
    ASSERT( pBuffer );
    ASSERT( Size  >  0 );
    ASSERT( Count >= 0 );
    ASSERT( s_pWrite );
    Bytes = s_pWrite( pFile, (byte*)pBuffer, (Size * Count) );
    return( Bytes / Size );
}

//==============================================================================

s32 x_fprintf( X_FILE* pFile, const char* pFormatStr, ... )
{
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
    ASSERT( s_pFlush );
    return( s_pFlush( pFile ) );
}

//==============================================================================

s32 x_fseek( X_FILE* pFile, s32 Offset, s32 Origin ) 
{
    ASSERT( IN_RANGE( 0, Origin, 2 ) );
    ASSERT( s_pSeek );
    return( s_pSeek( pFile, Offset, Origin ) );
}

//==============================================================================

s32 x_ftell( X_FILE* pFile ) 
{
    ASSERT( s_pTell );
    return( s_pTell( pFile ) );
}

//==============================================================================

s32 x_feof( X_FILE* pFile ) 
{
    ASSERT( s_pEOF );
    return( s_pEOF( pFile ) );
}

//==============================================================================

s32 x_fgetc( X_FILE* pFile ) 
{
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

s32 ansi_Read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
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
//  FILE I/O AND TEXT OUTPUT FUNCTIONS WRITTEN FOR PLAYSTATION 2
//==============================================================================

//==============================================================================
#ifdef TARGET_PS2_DEV
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
#endif // TARGET_PS2_DEV
//==============================================================================

//==============================================================================
//  INITIALIZATION AND SHUT DOWN FUNCTIONS
//==============================================================================

void x_IOInit( void )
{
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

#ifdef TARGET_PS2_DEV
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
#endif // TARGET_PS2_DEV
}

//==============================================================================

void x_IOKill( void )
{
    x_SetFileIOHooks( NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    x_SetPrintHook  ( NULL );
    x_SetPrintAtHook( NULL );
}

//==============================================================================
