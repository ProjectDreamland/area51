//==============================================================================
//  
//  x_string.hpp
//  
//==============================================================================

#ifndef X_STRING_HPP
#define X_STRING_HPP

//==============================================================================
//  
//  This file provides two categories of strings:
//   - "Temporary formatted strings" via classes xfs and xvfs
//   - "C++ style strings" via class xstring
//   - "C++ style wide strings" via class xwstring
//  
//  Each category has its own comment section below.
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

//==============================================================================
//  TYPES
//==============================================================================
#define XSTRING_LOCAL_LENGTH 32
//==============================================================================
//  TEMPORARY FORMATTED STRINGS
//==============================================================================
//  
//  The classes xfs and xvfs provide for "efficient temporary formatted 
//  strings".  That is, they are used to create formatted strings that will only
//  be needed in an isolated scope (like a single function).  The run-time 
//  overhead of xfs/xvfs is very low.  When possible, xfs/xvfs should be used 
//  rather than a large local fixed size character buffer (which wastes stack 
//  space) or an xstring (which performs heap allocations).
//
//  Generally, xfs/xvfs should be instantiated anonymously.
//
//  Here are example uses of xfs:
//  
//      pFile = x_fopen( xfs( "%s\%s", pPath, pFileName ), "rt" );
//      ASSERTS( pFile, xfs( "Failed to open file: %s", pFileName ) );
//      
//  In the first line, an anonymous instance of xfs is used to create a complete
//  file specification from a path and a file name.  A temporary string is 
//  created, used for the x_fopen() function, and then destroyed.
//  
//  In the second line, xfs is used in a similar manner to provide a meaningful
//  message for the the ASSERTS verification.
//  
//  Class xvfs is very similar to xfs except that it uses x_va_args rather than
//  an elipses (...) to receive the variable argument list.
//
//  The xfs/xvfs classes provide only three public functions:
//      - A constructor which creates the string.
//      - A destructor.
//      - And a "const char*" cast operator.
//  
//  When using xfs/xvfs, you should only directly deal with the constructor.  
//  The destructor and cast should be automatic.  In fact, you do not even need
//  to use an explicit cast when an xfs/xvfs is used in a variable argument list
//  expecting a "const char*".
//  
//  All other functionality is denied or private.  In particular, the following
//  operations are NOT permitted:
//      - Default (or void) construction.
//      - Copy construction.
//      - Assignment.
//      - Increasing the length of the string.
//
//  The xfs and xvfs classes are thread safe.
//  
//==============================================================================

class xfs
{
public:
                xfs                     ( const char* pFormatString, ... );
               ~xfs                     ( void );
                operator const char*    ( void );
                
private:        
                xfs                     ( void );
                xfs                     ( const xfs& XFS );
    const xfs&  operator =              ( const xfs& XFS );
    char*       m_pString;
};

//==============================================================================

class xvfs
{
public:
                xvfs                    ( const char* pFormatString, x_va_list Args );
               ~xvfs                    ( void );
                operator const char*    ( void );
                
private:        
                xvfs                    ( void );
                xvfs                    ( const xvfs& xvfs );
    const xvfs& operator =              ( const xvfs& xvfs );
    char*       m_pString;
};

//==============================================================================
//  C++ STYLE STRING CLASS
//==============================================================================
//  
//  For the most part, there are no surprises in this class.  It has a good 
//  collection of functions with reasonable names.  And don't forget: this class 
//  silently uses the heap!  
// 
//  (There is also a "wide character" version of xstring, called xwstring.  It
//  implements only a subset of the functions from xstring, but they should be
//  sufficient.)
//  
//  Class xwstring is simply a wide character version of xstring.  Not all of 
//  the functions from xstring are implemented, though it should be sufficient.
//  
//  The xstring class has a "cast to const char*" function which is usually 
//  called implicitly.  Furthermore, xstring has been designed to work in place
//  of standard C style NULL terminated strings as transparently as possible.
//  
//  In fact, the xstring even works in cases where most other C++ style strings
//  fail.  Consider:
//
//      xstring     XString   = "ABC";  // Good old xstring!
//      string      CPPString = "ABC";  // C++ standard string.
//      s32         Integer   =  123;   // Just an integer.
//
//      x_printf( "(%s)(%d)", XString,   Integer );     // (ABC)(123)
//      x_printf( "(%s)(%d)", CPPString, Integer );     // Failure!
//
//  The first x_printf will work!  The second x_printf will fail!
//
//  Note that an xstring may contain embedded NULL (or '\0') characters.  For 
//  example:
//  
//      xstring S = "ABC";
//      S[1] = '\0';
//      ASSERT( S.GetLength() == 3 );   // No problem.  The length is 3.
//  
//  The xstring S contains 3 characters: 'A', '\0', and 'C'.
//  
//  Of course, when you use an xstring with embedded NULLs like a C style NULL 
//  terminated string, any characters beyond the first embedded NULL will not be
//  recognized.
//  
//  Many of the functions provide versions that accept either a single 
//  character, a "const char*" (C style NULL terminated string), or an xstring.
//  Notice the Insert function, for example.
//  
//  The relational operators ==, !=, <, >, <=, and >= all perform case sensitive 
//  comparisons.
//  
//==============================================================================
//  
//  xstring( s32 Reserve )  - Construct an empty string, but reserve enough
//                            storage for a string 'Reserve' characters long.
//                            
//  operator const char*    - Cast to "const char*" function.  Usually called
//                            implicitly.  Allows xstrings to be passed to 
//                            functions which take "const char*" parameters.
//                            For example:
//                            
//                              xstring FileName = "Example.txt"; 
//                              pFile = x_fopen( FileName, "rt" );
//                            
//  Format          - Like x_sprintf  into the xstring.
//  FormatV         - Like x_vsprintf into the xstring.
//  AddFormat       - Like appending x_sprintf  onto the xstring.
//  AddFormatV      - Like appending x_vsprintf onto the xstring.
//  
//  IndexToRowCol   - Imagine the contents of the string in a text editor where 
//                    each '\n' in the string starts a new line.  This function,
//                    given an index into the string, reports the row and column
//                    numbers in the imaginary editor.  The very first character
//                    is always at 1,1.  Values of -1,-1 are returned when the
//                    Index exceeds the size of the string.
//  
//  Find            - Search for given character/string/xstring within the 
//                    xstring.  Return index if found, -1 otherwise.  An 
//                    optional starting offset is available.
//
//  LoadFile        - Read the entire content of the named file into the 
//                    xstring.  The file is read as binary data.  The return
//                    value is TRUE on a successful operation.
//  
//  SaveFile        - Write the xstring to the named file.  The file is written
//                    in binary mode.  The return value is TRUE on a successful
//                    operation.
//  
//  Dump            - For debugging.  Better than using x_printf since it will
//                    handle strings with embedded NULLs.  All "nonprintable"
//                    characters are displayed as a period ('.').
//  
//  DumpHex         - For debugging.  Good old fashioned hexadecimal dump.
//  
//==============================================================================

class xwstring;

class xstring
{

public:
                    xstring         (       s32       Reserve   );
                    xstring         (       char      Character );
                    xstring         ( const char*     pString   );
                    xstring         ( const xstring&  String    );
                    xstring         ( const xwstring& String    );
                    xstring         ( void );
                   ~xstring         ( void );

                    operator const char*( void ) const;
        char&       operator []         ( s32 Index );
                             
        char        GetAt           ( s32 Index ) const;
        void        SetAt           ( s32 Index, char Character );
                                    
        s32         GetLength       ( void ) const;
        xbool       IsEmpty         ( void ) const;
                                    
        void        Clear           ( void );
        void        FreeExtra       ( void );
                                    
        xstring     Mid             ( s32 Index, s32 Count ) const;
        xstring     Left            ( s32 Count ) const;
        xstring     Right           ( s32 Count ) const;
                                    
        void        MakeUpper       ( void );
        void        MakeLower       ( void );
                                    
        void        Insert          ( s32 Index,       char     Character );
        void        Insert          ( s32 Index, const char*    pString   );
        void        Insert          ( s32 Index, const xstring& String    );
                                    
        void        Delete          ( s32 Index, s32 Count = 1 );
                                    
        s32         Format          ( const char* pFormat, ... );
        s32         FormatV         ( const char* pFormat, x_va_list ArgList );
        s32         AddFormat       ( const char* pFormat, ... );
        s32         AddFormatV      ( const char* pFormat, x_va_list ArgList );
                                    
        s32         Find            (       char      Character,  s32 StartIndex = 0 ) const;
        s32         Find            ( const char*     pSubString, s32 StartIndex = 0 ) const;
        s32         Find            ( const xstring&  SubString,  s32 StartIndex = 0 ) const;
                                    
        void        IndexToRowCol   ( s32 Index, s32& Row, s32& Col ) const;
                                    
        xbool       LoadFile        ( const char* pFileName );
        xbool       SaveFile        ( const char* pFileName ) const;
                                    
        void        Dump            ( xbool LineFeed = FALSE ) const;
        void        DumpHex         ( void ) const;
                                    
const   xstring&    operator =      (       char      Character );
const   xstring&    operator =      ( const char*     pString   );
const   xstring&    operator =      ( const xstring&  String    );
const   xstring&    operator =      ( const xwstring& String    );
                                    
const   xstring&    operator +=     (       char      Character );
const   xstring&    operator +=     ( const char*     pString   );
const   xstring&    operator +=     ( const xstring&  String    );
                                    
friend  xstring     operator +      ( const xstring&  String,          char      Character );
friend  xstring     operator +      ( const xstring&  String,    const char*     pString   );
friend  xstring     operator +      ( const xstring&  String1,   const xstring&  String2   );
friend  xstring     operator +      (       char      Character, const xstring&  String    );
friend  xstring     operator +      ( const char*     pString,   const xstring&  String    );
                                    
friend  xbool       operator ==     ( const xstring&  S1, const xstring&  S2 );
friend  xbool       operator !=     ( const xstring&  S1, const xstring&  S2 );
friend  xbool       operator <      ( const xstring&  S1, const xstring&  S2 );
friend  xbool       operator >      ( const xstring&  S1, const xstring&  S2 );
friend  xbool       operator <=     ( const xstring&  S1, const xstring&  S2 );
friend  xbool       operator >=     ( const xstring&  S1, const xstring&  S2 );
                                    
friend  xbool       operator ==     ( const xstring&  S1, const char*     S2 );
friend  xbool       operator !=     ( const xstring&  S1, const char*     S2 );
friend  xbool       operator <      ( const xstring&  S1, const char*     S2 );
friend  xbool       operator >      ( const xstring&  S1, const char*     S2 );
friend  xbool       operator <=     ( const xstring&  S1, const char*     S2 );
friend  xbool       operator >=     ( const xstring&  S1, const char*     S2 );
                                    
friend  xbool       operator ==     ( const char*     S1, const xstring&  S2 );
friend  xbool       operator !=     ( const char*     S1, const xstring&  S2 );
friend  xbool       operator <      ( const char*     S1, const xstring&  S2 );
friend  xbool       operator >      ( const char*     S1, const xstring&  S2 );
friend  xbool       operator <=     ( const char*     S1, const xstring&  S2 );
friend  xbool       operator >=     ( const char*     S1, const xstring&  S2 );
                                    
private:                            
        void        EnsureCapacity  ( s32 Capacity );

protected:
        char*       m_pData;
		char		m_LocalData[XSTRING_LOCAL_LENGTH];
};

//==============================================================================

class xwstring
{

public:
                    xwstring        (       s32       Reserve     );
                    xwstring        (       xwchar    WideChar    );
                    xwstring        ( const xwchar*   pWideString );
                    xwstring        ( const xwstring& WideString  );
                    xwstring        ( const char *pString         );
                    xwstring        ( void );
                   ~xwstring        ( void );

                    operator const xwchar*  ( void ) const;
        xwchar&     operator []             ( s32 Index );

        xwchar      GetAt           ( s32 Index ) const;
        void        SetAt           ( s32 Index, xwchar WideChar );

        s32         GetLength       ( void ) const;
        xbool       IsEmpty         ( void ) const;

        void        Clear           ( void );
        void        FreeExtra       ( void );

        s32         GetHashKey      ( s32 Index, s32 Count ) const;

        xwstring    Mid             ( s32 Index, s32 Count ) const;
        xwstring    Left            ( s32 Count ) const;
        xwstring    Right           ( s32 Count ) const;
/*
        void        MakeUpper       ( void );
        void        MakeLower       ( void );
*/
        void        Insert          ( s32 Index,       xwchar    WideChar );
        void        Insert          ( s32 Index, const xwchar*   pWideString );
        void        Insert          ( s32 Index, const xwstring& WideString  );

        void        Delete          ( s32 Index, s32 Count = 1 );
/*
        s32         Format          ( const char* pFormat, ... );
        s32         FormatV         ( const char* pFormat, x_va_list ArgList );
        s32         AddFormat       ( const char* pFormat, ... );
        s32         AddFormatV      ( const char* pFormat, x_va_list ArgList );
*/
        s32         Find            (       xwchar    WideChar,      s32 StartIndex = 0 ) const;
        s32         Find            ( const xwchar*   pWideSubStr,   s32 StartIndex = 0 ) const;
        s32         Find            ( const xwstring& WideSubString, s32 StartIndex = 0 ) const;
/*
        void        IndexToRowCol   ( s32 Index, s32& Row, s32& Col ) const;
*/
        xbool       LoadFile        ( const char* pFileName );
        xbool       SaveFile        ( const char* pFileName ) const;
/*
        void        Dump            ( xbool LineFeed = FALSE ) const;
*/
        void        DumpHex         ( void ) const;

const   xwstring&   operator =      (       xwchar     WideChar      );
const   xwstring&   operator =      ( const xwchar*    pWideString   );
const   xwstring&   operator =      ( const xwstring&  WideString    );
const   xwstring&   operator =      ( const xstring&   String        );
                                   
const   xwstring&   operator +=     (       xwchar     WideChar      );
const   xwstring&   operator +=     ( const xwchar*    pWideString   );
const   xwstring&   operator +=     ( const xwstring&  WideString    );
                                   
friend  xwstring    operator +      ( const xwstring&  WideString,        xwchar     WideChar    );
friend  xwstring    operator +      ( const xwstring&  WideString,  const xwchar*    pWideString );
friend  xwstring    operator +      ( const xwstring&  String1,     const xwstring&  String2     );
friend  xwstring    operator +      (       xwchar     WideChar,    const xwstring&  WideString  );
friend  xwstring    operator +      ( const xwchar*    pWideString, const xwstring&  WideString  );
                                    
friend  xbool       operator ==     ( const xwstring&  S1, const xwstring&  S2 );
friend  xbool       operator !=     ( const xwstring&  S1, const xwstring&  S2 );
friend  xbool       operator <      ( const xwstring&  S1, const xwstring&  S2 );
friend  xbool       operator >      ( const xwstring&  S1, const xwstring&  S2 );
friend  xbool       operator <=     ( const xwstring&  S1, const xwstring&  S2 );
friend  xbool       operator >=     ( const xwstring&  S1, const xwstring&  S2 );
                                    
friend  xbool       operator ==     ( const xwstring&  S1, const xwchar*    S2 );
friend  xbool       operator !=     ( const xwstring&  S1, const xwchar*    S2 );
friend  xbool       operator <      ( const xwstring&  S1, const xwchar*    S2 );
friend  xbool       operator >      ( const xwstring&  S1, const xwchar*    S2 );
friend  xbool       operator <=     ( const xwstring&  S1, const xwchar*    S2 );
friend  xbool       operator >=     ( const xwstring&  S1, const xwchar*    S2 );
                                    
friend  xbool       operator ==     ( const xwchar*    S1, const xwstring&  S2 );
friend  xbool       operator !=     ( const xwchar*    S1, const xwstring&  S2 );
friend  xbool       operator <      ( const xwchar*    S1, const xwstring&  S2 );
friend  xbool       operator >      ( const xwchar*    S1, const xwstring&  S2 );
friend  xbool       operator <=     ( const xwchar*    S1, const xwstring&  S2 );
friend  xbool       operator >=     ( const xwchar*    S1, const xwstring&  S2 );
                                    
private:                            
        void        EnsureCapacity  ( s32 Capacity );

protected:
        xwchar*     m_pData;
		xwchar		m_LocalData[XSTRING_LOCAL_LENGTH];
};

//==============================================================================

#include "Implementation/x_string_inline.hpp"

//==============================================================================
#endif // X_STRING_HPP
//==============================================================================
