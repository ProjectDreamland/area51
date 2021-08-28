//==============================================================================
//  
//  xsc_errors.hpp
//  
//==============================================================================

#ifndef XSC_ERRORS_HPP
#define XSC_ERRORS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "xsc_tokenizer.hpp"

//==============================================================================
//  Defines
//==============================================================================

// Errors and Warnings
enum error_code
{
    err_start,
    err_internal,                           // Internal compiler error
    err_memory,                             // Out of memory
    err_syntax,                             // General syntax error
    err_end,

    warn_start,
    warn_syntax,                            // General syntax warning
    warn_end
};

//==============================================================================
//  xsc_errors
//==============================================================================

class xsc_errors
{
public:
    // Error structure
    struct err
    {
        s32                 Code;                   // Error Code
        const xsc_token*    pToken;                 // Token in source file
        s32                 Index;                  // Source Index if token is NULL
        xwstring            String;                 // Error string (EMPTY for default)
    };

//==============================================================================
//  Public Functions
//==============================================================================
public:
                        xsc_errors              ( const xwstring& Source );
                       ~xsc_errors              ( void );

    void                SetSourceName           ( const xstring&    SourceName );       // Set Source Name

    void                Error                   ( s32               Code,
                                                  s32               Index );            // Add error
    void                Error                   ( s32               Code,
                                                  s32               Index,
                                                  const char*       pString = NULL );   // Add error
    void                Error                   ( s32               Code,
                                                  const xsc_token*  pToken,
                                                  const xwchar*     pString = NULL );   // Add error
    void                Error                   ( s32               Code,
                                                  const xsc_token*  pToken,
                                                  const char*       pString );          // Add error
    void                Warning                 ( s32               Code,
                                                  s32               Index );            // Add warning
    void                Warning                 ( s32               Code,
                                                  const xsc_token*  pToken,
                                                  const xwchar*     pString = NULL );   // Add warning
    void                Warning                 ( s32               Code,
                                                  const xsc_token*  pToken,
                                                  const char*       pString );          // Add warning
    s32                 GetNumErrors            ( void ) const;                         // Get number of errors
    s32                 GetNumWarnings          ( void ) const;                         // Get number of warnings

    // Debugging
    xstring             Dump                    ( void ) const;                         // Dump

//==============================================================================
//  Protected Functions
//==============================================================================
protected:
    s32                 IndexToLine             ( s32 Index ) const;

//==============================================================================
//  Data
//==============================================================================

protected:
    const xwstring&     m_Source;                                           // Source code

    xstring             m_SourceName;                                       // Source name
    xarray<err>         m_Errors;                                           // Array of errors
    s32                 m_nErrors;                                          // Number of errors
    s32                 m_nWarnings;                                        // Number of warnings
};

//==============================================================================
#endif // XSC_ERRORS_HPP
//==============================================================================
