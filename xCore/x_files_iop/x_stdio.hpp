//==============================================================================
//
//  x_stdio.hpp
//
//==============================================================================

#ifndef X_STDIO_HPP
#define X_STDIO_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#define X_SEEK_SET  0
#define X_SEEK_CUR  1
#define X_SEEK_END  2

#define X_EOF      -1

//==============================================================================
//  TYPES
//==============================================================================

typedef u32 X_FILE;

//==============================================================================
//  STANDARD I/O FUNCTIONS
//==============================================================================
//
//  These functions have been designed to mimic their standard C library 
//  counterparts.  For information on a function, consult any C documentation.
//
//  x_fopen     - Attempt to open given file with given mode.
//  x_fclose    - Close given file.
//  x_fread     - Read from file into buffer based on size and count.
//  x_fwrite    - Write to file from buffer based on size and count.
//  x_fprintf   - Formatted write to file.
//  x_fflush    - Flush all pending I/O for file.
//  x_fseek     - Reposition logical cursor in file.
//  x_ftell     - Report cursor position in file.
//  x_feof      - Report if cursor is at or beyond end of file.
//  x_fgetc     - Read one character from file.
//  x_fputc     - Write one character to file.
//      
//  And, some extra functions...
//      
//  x_flength   - Return the length of the file.    
//      
//==============================================================================

X_FILE* x_fopen     ( const char* pFileName, const char* pMode );             
void    x_fclose    ( X_FILE* pFile );                                     
s32     x_fread     (       void* pBuffer, s32 Size, s32 Count, X_FILE* pFile );  
s32     x_fwrite    ( const void* pBuffer, s32 Size, s32 Count, X_FILE* pFile );  
s32     x_fprintf   ( X_FILE* pFile, const char* pFormatStr, ... );
s32     x_fflush    ( X_FILE* pFile );                                     
s32     x_fseek     ( X_FILE* pFile, s32 Offset, s32 Origin );             
s32     x_ftell     ( X_FILE* pFile );
s32     x_feof      ( X_FILE* pFile );
s32     x_fgetc     ( X_FILE* pFile );
s32     x_fputc     ( s32 C, X_FILE* pFile );
       
s32     x_flength   ( X_FILE* pFile );

//==============================================================================
//  FORMATTED STRING FUNCTIONS
//==============================================================================
//
//  x_printf    
//
//      Formatted print to "standard text output".  This is straight forward for
//      text mode programs and is handled by the x_files.  Graphical programs
//      must register a function to handle this operation.
//
//  x_printfxy
//
//      Like x_printf with the addition that the text will be printed at the
//      given character cell coordinates.  (0,0) represents the upper left most 
//      character available.  The x_files will provide versions of this function 
//      for each text mode target supported.  Graphical programs must register 
//      a function to handle this operation.
//      
//  x_sprintf   - Format print into given string.
//  x_vsprintf  - Format print into given string using x_va_list for args.
//
//==============================================================================
//
//  Also, check out file x_string.hpp.  It contains support for "temporary
//  formatted strings", and "C++ style dynamic strings".
//
//==============================================================================

s32     x_printf    (               const char* pFormatStr, ... );
s32     x_printfxy  ( s32 X, s32 Y, const char* pFormatStr, ... );
s32     x_sprintf   ( char* pStr,   const char* pFormatStr, ... );
s32     x_vsprintf  ( char* pStr,   const char* pFormatStr, x_va_list Args );

//==============================================================================
//  CUSTOM FILE I/O AND TEXT OUTPUT - EXPERTS ONLY
//==============================================================================

typedef X_FILE*   open_fn ( const char* pFileName, const char* pMode );
typedef void     close_fn ( X_FILE* pFile );
typedef s32       read_fn ( X_FILE* pFile,       byte* pBuffer, s32 Bytes );
typedef s32      write_fn ( X_FILE* pFile, const byte* pBuffer, s32 Bytes );
typedef s32       seek_fn ( X_FILE* pFile, s32 Offset, s32 Origin );
typedef s32       tell_fn ( X_FILE* pFile );
typedef s32      flush_fn ( X_FILE* pFile );
typedef xbool      eof_fn ( X_FILE* pFile );
typedef s32     length_fn ( X_FILE* pFile );

typedef void     print_fn ( const char* pString );
typedef void  print_at_fn ( const char* pString, s32 X, s32 Y );

//------------------------------------------------------------------------------

void    x_SetFileIOHooks    (     open_fn*  pOpen,
                                 close_fn*  pClose,
                                  read_fn*  pRead,
                                 write_fn*  pWrite,
                                  seek_fn*  pSeek,
                                  tell_fn*  pTell,
                                 flush_fn*  pFlush,
                                   eof_fn*  pEOF,
                                length_fn*  pLength  );

void    x_GetFileIOHooks    (     open_fn*  &pOpen,
                                 close_fn*  &pClose,
                                  read_fn*  &pRead,
                                 write_fn*  &pWrite,
                                  seek_fn*  &pSeek,
                                  tell_fn*  &pTell,
                                 flush_fn*  &pFlush,
                                   eof_fn*  &pEOF,
                                length_fn*  &pLength  );

void    x_SetPrintHook      (    print_fn*  pPrint   );
void    x_SetPrintAtHook    ( print_at_fn*  pPrintAt );

//==============================================================================
#endif // X_STDIO_HPP
//==============================================================================
