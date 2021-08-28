//############################################################################
//##                                                                        ##
//##  RIB.H: RAD Interface Broker services                                  ##
//##                                                                        ##
//##  Version 1.00 of 21-May-98: Initial                                    ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#ifndef RIB_VERSION

#define RIB_VERSION      "1.1"
#define RIB_VERSION_DATE "21-Jan-01"

#define RIB_COPYRIGHT "Copyright (C) 1998-2001 RAD Game Tools, Inc."

#endif

#ifndef RIB_H
#define RIB_H

// IS_DOS for DOS
// IS_WINDOWS for Windows or Win32s
// IS_WIN32 for Win32s
// IS_WIN16 for Windows
// IS_32 for 32-bit DOS or Win32s
// IS_16 for 16-bit Windows
// IS_LE for little endian (PCs)
// IS_BE for big endian (Macs)
// IS_X86 for Intel
// IS_MAC for Mac
// IS_PPC for PPC Mac
// IS_68K for 68K Mac


#ifdef IS_DOS
#undef IS_DOS
#endif

#ifdef IS_WINDOWS
#undef IS_WINDOWS
#endif

#ifdef IS_WIN32
#undef IS_WIN32
#endif

#ifdef IS_WIN16
#undef IS_WIN16
#endif

#ifdef IS_32
#undef IS_32
#endif

#ifdef IS_16
#undef IS_16
#endif

#ifdef IS_LE
#undef IS_LE
#endif

#ifdef IS_BE
#undef IS_BE
#endif

#ifdef IS_X86
#undef IS_X86
#endif

#ifdef IS_MAC
#undef IS_MAC
#endif

#ifdef IS_PPC
#undef IS_PPC
#endif

#ifdef IS_68K
#undef IS_68K
#endif

#ifdef __DOS__
  #define IS_DOS
  #define IS_32
  #define IS_LE
  #define IS_X86
#else
  #ifdef _WIN32
    #define IS_WINDOWS
    #define IS_WIN32
    #define IS_32
    #define IS_LE
    #define IS_X86
  #else
    #ifdef WIN32
      #define IS_WINDOWS
      #define IS_WIN32
      #define IS_32
      #define IS_LE
      #define IS_X86
    #else
      #ifdef __NT__
        #define IS_WINDOWS
        #define IS_WIN32
        #define IS_32
        #define IS_LE
        #define IS_X86
      #else
        #ifdef __WIN32__
          #define IS_WINDOWS
          #define IS_WIN32
          #define IS_32
          #define IS_LE
          #define IS_X86
        #else
          #ifdef _WINDOWS
            #define IS_WINDOWS
            #define IS_WIN16
            #define IS_16
            #define IS_LE
            #define IS_X86
          #else
            #ifdef _WINDLL
              #define IS_WINDOWS
              #define IS_WIN16
              #define IS_16
              #define IS_LE
              #define IS_X86
            #else
              #ifdef WINDOWS
                #define IS_WINDOWS
                #define IS_WIN16
                #define IS_16
                #define IS_LE
                #define IS_X86
              #else
                #ifdef __WINDOWS__
                  #define IS_WINDOWS
                  #define IS_WIN16
                  #define IS_16
                  #define IS_LE
                  #define IS_X86
                #else
                  #ifdef _Windows
                    #define IS_WINDOWS
                    #define IS_WIN16
                    #define IS_16
                    #define IS_LE
                    #define IS_X86
                  #else
                    #if defined(macintosh) || defined(__powerc) || defined(powerc) || defined(__POWERPC__) || defined(__MC68K__)
                      #define IS_MAC
                      #define IS_32
                      #define IS_BE
                      #if defined(__powerc) || defined(powerc) || defined(__POWERPC__)
                        #define IS_PPC
                      #else
                        #if defined(__MC68K__)
                          #define IS_68K
                        #endif
                      #endif
                    #endif
                  #endif
                #endif
              #endif
            #endif
          #endif
        #endif
      #endif
    #endif
  #endif
#endif

#if (!defined(IS_LE) && !defined(IS_BE))
  #error RIB.H did not detect your platform.  Define __DOS__, _WINDOWS, WIN32, or macintosh.
#endif

#if defined(_PUSHPOP_SUPPORTED) || PRAGMA_STRUCT_PACKPUSH
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IS_DOS

#define RIBCALLBACK __pascal
#define RIBEXPORT cdecl
#define RIBDEC extern
#define RIBDEF
#define RIBCALL cdecl
#define FAR
#define HIWORD(ptr) (((U32)ptr)>>16)
#define LOWORD(ptr) ((U16)((U32)ptr))

#define RIBLIBCALLBACK __pascal

#else

#ifdef IS_MAC

#define RIBLIBCALLBACK //pascal

#ifdef BUILD_RIB
#define BUILDING_RIB
#endif

#ifdef BUILD_MSS
#define BUILDING_RIB
#endif

#define RIBEXPORT //pascal

#ifdef BUILDING_RIB
  #define RIBDEC __declspec(export)
  #define RIBDEF
#else
  #define RIBDEC extern
  #define RIBDEF
#endif

#define RIBDLLNAME "Miles Shared Library"

#define RIBCALL //pascal
#define RIBCALLBACK //pascal

#define FAR

#else

#define RIBLIBCALLBACK WINAPI

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <mmsystem.h>

//
// If compiling RIB DLL, use __declspec(dllexport) for both
// declarations and definitions
//
// If compiling RIB16 library or application, use "extern" in declarations
// and nothing in definitions
//

#ifdef BUILD_RIB
#define BUILDING_RIB
#endif

#ifdef BUILD_MSS
#define BUILDING_RIB
#endif

#ifdef IS_WIN32

  #define RIBEXPORT WINAPI

  #ifdef BUILDING_RIB
    #define RIBDEC __declspec(dllexport)
    #define RIBDEF __declspec(dllexport)
  #else

    #ifdef __BORLANDC__
      #define RIBDEC extern
    #else
      #define RIBDEC __declspec(dllimport)
    #endif

  #endif

  #define RIBDLLNAME "MSS32.DLL"

#else

  #define RIBEXPORT __export WINAPI

  #define RIBDEC  extern
  #define RIBDEF

  #define RIBDLLNAME "MSS16.DLL"

#endif

#define RIBCALL WINAPI
#define RIBCALLBACK RIBEXPORT

#endif

#endif

//
// General type definitions for portability
//

#ifndef NULL
#define NULL 0
#endif

#ifndef C8
#define C8 char
#endif

#ifndef F32
#define F32 float
#endif

#ifndef F64
#define F64 double
#endif

#ifndef U8
#define U8 unsigned char
#endif

#ifndef S8
#define S8 signed char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef S16
#define S16 signed short
#endif

#ifndef U32
#define U32 unsigned long
#endif

#ifndef S32
#define S32 signed long
#endif

#define ARY_CNT(x) (sizeof((x)) / sizeof((x)[0]))

// -------------------------------------------------------------------------
// RIB data types         
// -------------------------------------------------------------------------

typedef S32 RIBRESULT;

#define RIB_NOERR                    0   // Success -- no error
#define RIB_NOT_ALL_AVAILABLE        1   // One or more requested functions/attribs were not available
#define RIB_NOT_FOUND                2   // Resource not found      
#define RIB_OUT_OF_MEM               3   // Out of system RAM       

// -------------------------------------------------------------------------
// Handle to RIB interface provider
// -------------------------------------------------------------------------

typedef U32 HPROVIDER;

// -------------------------------------------------------------------------
// Handle representing token used to obtain attribute or preference 
// data from RIB provider
//
// These tokens are arbitrary, unique numeric values used by the
// interface provider to identify named attributes and preferences
// -------------------------------------------------------------------------

typedef U32 HATTRIB;

// -------------------------------------------------------------------------
// Handle representing an enumerated interface entry
//
// RIB_enumerate_interface() returns 1 if valid next entry found, else 
// 0 if end of list reached
// -------------------------------------------------------------------------

typedef U32 HINTENUM;
#define HINTENUM_FIRST 0

// -------------------------------------------------------------------------
// Handle representing an enumerated provider entry
//
// RIB_enumerate_providers() returns 1 if valid next entry found, else
// 0 if end of list reached
// -------------------------------------------------------------------------

typedef U32 HPROENUM;
#define HPROENUM_FIRST 0

// -------------------------------------------------------------------------
// Data types for RIB attributes and preferences
// -------------------------------------------------------------------------

typedef enum
{
   RIB_NONE = 0, // No type
   RIB_CUSTOM,   // Used for pointers to application-specific structures
   RIB_DEC,      // Used for 32-bit integer values to be reported in decimal
   RIB_HEX,      // Used for 32-bit integer values to be reported in hex
   RIB_FLOAT,    // Used for 32-bit single-precision FP values
   RIB_PERCENT,  // Used for 32-bit single-precision FP values to be reported as percentages
   RIB_BOOL,     // Used for Boolean-constrained integer values to be reported as TRUE or FALSE
   RIB_STRING    // Used for pointers to null-terminated ASCII strings
}
RIB_DATA_SUBTYPE;  

// -------------------------------------------------------------------------
// RIB_ENTRY_TYPE structure, used to register an interface or request one
// -------------------------------------------------------------------------

typedef enum
{
   RIB_FUNCTION = 0,  // Function
   RIB_ATTRIBUTE,     // Attribute: read-only data type used for status/info communication
   RIB_PREFERENCE     // Preference: read/write data type used to control behavior
}
RIB_ENTRY_TYPE;

// -------------------------------------------------------------------------
// RIB_INTERFACE_ENTRY, used to represent a function or data entry in an
// interface
// -------------------------------------------------------------------------

typedef struct
{
   RIB_ENTRY_TYPE   type;        // See list of RIB_ENTRY_TYPEs above
   char FAR        *entry_name;  // Name of desired function or attribute
   U32              token;       // Function pointer or attribute token
   RIB_DATA_SUBTYPE subtype;     // Data (attrib or preference) subtype
}
RIB_INTERFACE_ENTRY;

// -------------------------------------------------------------------------
// Standard RAD Interface Broker provider identification attributes
// -------------------------------------------------------------------------

#define PROVIDER_NAME    (-100)    // RIB_STRING name of decoder
#define PROVIDER_VERSION (-101)    // RIB_HEX BCD version number

// -------------------------------------------------------------------------
// Standard function to obtain provider attributes (see PROVIDER_ defines
// above)
//
// Each provider of a searchable interface must export this function
// -------------------------------------------------------------------------

typedef U32 (RIBCALL FAR *PROVIDER_QUERY_ATTRIBUTE) (HATTRIB index);

// -------------------------------------------------------------------------
// Macros to simplify interface registrations/requests for functions,
// attributes, and preferences
// 
// FN(entry_name): Request address of function entry_name to be placed in 
//                 variable of same name.  Variable must be in scope.
//
// REG_FN(entry_name): Used by a RIB provider to register (i.e., export)
//                     function entry_name.  
//
// AT(entry_name,ID): Request a token for the named attribute.  This token may
//                    later be used to request the current attribute value
//                    from the provider, either via the standard 
//                    PROVIDER_query_attribute() function or via a RIB-specific
//                    query function.
//
// REG_AT(entry_name,ID,subtype): Used by a RIB provider to register a named
//                                attribute (entry_name) with a numeric 
//                                token (ID) which may be used by the application
//                                to retrieve the value of that attribute.  The
//                                subtype is a value of type RIB_DATA_SUBTYPE 
//                                which allows generic attribute browsers and 
//                                the RIB_type_string() function to identify the
//                                type of enumerated attributes at runtime.
// PR(entry_name,ID)
// REG_PR(entry_name,ID,subtype): Preferences are treated as attributes (see above)
//                                whose values may be written as well as retrieved.
//                                Normally, the current preference values are readable 
//                                via the provider's attribute-query function, while a
//                                separate preference-write function is provided to
//                                allow the application to change the values.
// -------------------------------------------------------------------------

#define FN(entry_name)        { RIB_FUNCTION, #entry_name, (U32) &(entry_name), RIB_NONE }
#define REG_FN(entry_name)    { RIB_FUNCTION, #entry_name, (U32) &(entry_name), RIB_NONE }

#define AT(entry_name,ID)             { RIB_ATTRIBUTE, (entry_name), (U32) &(ID), RIB_NONE }
#define REG_AT(entry_name,ID,subtype) { RIB_ATTRIBUTE, (entry_name), (U32)  (ID), subtype  }

#define PR(entry_name,ID)             { RIB_PREFERENCE, (entry_name), (U32) &(ID), RIB_NONE }
#define REG_PR(entry_name,ID,subtype) { RIB_PREFERENCE, (entry_name), (U32)  (ID), subtype  }

// -------------------------------------------------------------------------
// RIB_register(x,y,z): Macro to conveniently register one or more interface entries in an
//                      array.  x=provider handle, y=name of interface with which the 
//                      entries are associated, z=address of RIB_INTERFACE_ENTRY declarations.
// 
// RIB_unregister(x,y,z): Unregister one or more arrayed interface entries.
//
// RIB_unregister_all(): Unregister all entries in all interfaces associated with a 
//                       given provider.
//
// RIB_free_libraries(): Unload all RIB libraries.  Should be preceded by RIB_unregister_all() for
//                       all loaded providers.
//
// RIB_request(x,y,z): Macro to conveniently request fixups for one or more interface entries
//                     in an array.  x=provider handle, y=name of interface containing 
//                     named entries, z=address of RIB_INTERFACE_ENTRY declarations.
// -------------------------------------------------------------------------

#define RIB_register(x,y,z)   RIB_register_interface  (HPROVIDER(x), y, ARY_CNT(z), z)
#define RIB_unregister(x,y,z) RIB_unregister_interface(HPROVIDER(x), y, ARY_CNT(z), z)
#define RIB_unregister_all(x) RIB_unregister_interface(HPROVIDER(x), NULL, 0, NULL)
#define RIB_free_libraries()  RIB_free_provider_library(HPROVIDER(NULL));
#define RIB_request(x,y,z)    RIB_request_interface   (x, y, ARY_CNT(z), z)

// ----------------------------------
// Standard RIB API prototypes
// ----------------------------------

// -------------------------------------------------------------------------
// RIB_alloc_provider_handle(module): Allocate a new HPROVIDER handle for a
// RIB provider, where module=the library's module handle.
//
// RIB_free_provider_handle(provider): Free an HPROVIDER handle allocated from
// the RIB_alloc_provider_handle() function (above).
//
// Most applications which only load RIB libraries do not need to call 
// either of these functions.  Their functionality is encapsulated by the
// RIB_load_provider_library() / RIB_free_provider_library() functions.
// -------------------------------------------------------------------------

RIBDEC HPROVIDER  RIBCALL RIB_alloc_provider_handle   (U32            module);
RIBDEC void       RIBCALL RIB_free_provider_handle    (HPROVIDER      provider);

// -------------------------------------------------------------------------
// RIB_load_provider_library(filename): Load a RIB provider DLL, returning its
// HPROVIDER handle or NULL on error.  Internally, this function combines a 
// call to LoadLibrary() with a call to RIB_alloc_provider_handle().  
//
// RIB_free_provider_library(provider): Free a previously-loaded provider DLL.
//
// Under DOS, RIB_load_provider_library() is equivalent to 
// calling RIB_alloc_provider_handle(0).
// -------------------------------------------------------------------------

RIBDEC HPROVIDER  RIBCALL RIB_load_provider_library   (C8 const FAR  *filename);
RIBDEC void       RIBCALL RIB_free_provider_library   (HPROVIDER      provider);

// -------------------------------------------------------------------------
// RIB_provider_library_handle(): Intended for use by the LibMain()-based
// interface registration procedure in a RIB provider library, this function
// returns the HPROVIDER handle last allocated by RIB_load_provider_library().
// It gives the provider library access to the HPROVIDER handle which will be
// used to access it.  This handle should be used by the library when registering
// its RIB interfaces.
// -------------------------------------------------------------------------

RIBDEC HPROVIDER  RIBCALL RIB_provider_library_handle (void);

// -------------------------------------------------------------------------
// RIB_register_interface(provider, interface_name, entry_count, list): Allows
// a RIB provider to register an interface -- i.e., a named collection of the specific 
// functions, attributes, and/or preferences it supports.  provider is normally the
// value obtained by calling RIB_provider_library_handle() from the library's
// LibMain() procedure.  interface_name is the name of the interface, entry_count
// is the number of interface entries being registered, and list is a pointer to an
// array of one or more RIB_INTERFACE_ENTRY structures representing the entries 
// to be registered.  All fields of each RIB_INTERFACE_ENTRY structure should be valid
// when calling RIB_register_interface().  For convenience, the macros REG_FN, REG_AT,
// and REG_PR can be used instead of RIB_register_interface() in many cases.
// 
// RIB_unregister_interface(provider,interface_name,entry_count,list) removes 
// the specified entries from the interface.  If list_name is NULL, all
// entries associated with the specified interface are unregistered.  If interface_name
// is NULL, all entries associated with all interfaces associated with the
// specified provider are unregistered.
// -------------------------------------------------------------------------


RIBDEC RIBRESULT  RIBCALL RIB_register_interface      (HPROVIDER                      provider,
                                                       C8 const FAR                  *interface_name,
                                                       S32                            entry_count,
                                                       const RIB_INTERFACE_ENTRY FAR *list);

RIBDEC RIBRESULT  RIBCALL RIB_unregister_interface    (HPROVIDER                      provider,
                                                       C8 const FAR                  *interface_name,
                                                       S32                            entry_count,
                                                       const RIB_INTERFACE_ENTRY FAR *list);

// -------------------------------------------------------------------------
// RIB_request_interface(provider, interface_name, entry_count, list): Allows
// a RIB application to request an interface -- i.e., a named collection of the specific
// functions, attributes, and/or preferences supported by a RIB provider.
//
// interface_name is the name of the requested interface, entry_count is the number of
// interface entries being requested, and list is a pointer to an array of one or more
// RIB_INTERFACE_ENTRY structures representing the entries to be requested.  The 'token' fields
// of the entry structures should consist of a pointer to the application variables which will
// receive the requested function pointers or attribute tokens.  The data subtype fields 
// do not need to be filled in when requesting interface entries, and may be set to RIB_NONE.
//
// For convenience, the macros FN, AT, and PR can be used instead of RIB_request_interface()
// whenever the entry names and their token variables share the same names, and the token
// variables are in scope.
// 
// If a requested interface entry is not supported by the specified provider,
// the entry structure's 'token' field will receive the value -1 for attribute and
// preference entries, or NULL for function entries.  If one or more entries are not
// found, the return value will be RIB_NOT_ALL_AVAILABLE.  Otherwise, RIB_NOERR will
// be returned if all requested entries were successfully resolved.
// -------------------------------------------------------------------------

RIBDEC RIBRESULT  RIBCALL RIB_request_interface       (HPROVIDER                provider,
                                                       C8 const FAR            *interface_name,
                                                       S32                      entry_count,
                                                       RIB_INTERFACE_ENTRY FAR *list);

// -------------------------------------------------------------------------
// RIB_request_interface_entry(provider, interface_name, entry_type, entry_name, token) provides
// a convenient way to request interface entries one at a time, without the need to declare an array
// of RIB_INTERFACE_ENTRY variables as with RIB_request_interface() (above).
//
// provider specifies the provider handle; interface_name is the name of the requested
// interface; entry_type specifies the type of the requested entry (RIB_FUNCTION,
// RIB_ATTRIBUTE, or RIB_PREFERENCE); entry_name is the name of the desired function,
// attribute, or preference; and token specifies the address of the variable to
// receive the attribute/preference token or function address.
//
// If a requested interface entry is not supported by the specified provider,
// the token' variable will receive the value -1 for attribute and preference
// entries, or NULL for function entries, and the value RIB_NOT_FOUND will be
// returned from the function.  Otherwise, RIB_NOERR will be returned if
// the requested entry was successfully resolved.
// -------------------------------------------------------------------------

RIBDEC RIBRESULT  RIBCALL RIB_request_interface_entry (HPROVIDER                provider,
                                                       C8 const FAR            *interface_name,
                                                       RIB_ENTRY_TYPE           entry_type,
                                                       C8 const FAR            *entry_name,
                                                       U32 FAR                 *token);

// -------------------------------------------------------------------------
// RIB_enumerate_interface(provider, interface_name, type, next, dest) allows a RIB application
// to determine the set of available RIB interface entries associated with a given provider and
// interface.
//
// provider specifies the provider handle; interface_name is the name of the interface to 
// be examined; type specifies the type of entry to return (attributes, preferences, or
// functions); next is a pointer to a variable of type HINTENUM which is initialized to NULL
// prior to the first call to RIB_enumerate_interface(), and subsequently used by the
// function to maintain the state of the enumeration operation; and dest is a pointer to
// a RIB_INTERFACE_ENTRY structure which receives information about the name, subtype, and 
// access token or function pointer for each qualified interface entry.
//
// A return value of 0 indicates that no (further) interface entries are available.
// -------------------------------------------------------------------------

RIBDEC S32        RIBCALL RIB_enumerate_interface     (HPROVIDER                provider,
                                                       C8 const FAR            *interface_name,
                                                       RIB_ENTRY_TYPE           type,
                                                       HINTENUM FAR            *next,
                                                       RIB_INTERFACE_ENTRY FAR *dest);

// -------------------------------------------------------------------------
// RIB_enumerate_providers(interface_name, next, dest) allows a RIB application
// to determine the set of available RIB providers of the specified interface.
//
// interface_name is the name of the specified interface; next is a pointer to a 
// variable of type HPROENUM which is initialized to NULL prior to the first call
// to RIB_enumerate_providers(), and subsequently used by the function to maintain the
// state of the enumeration operation; and dest is a pointer to a variable of type 
// HPROVIDER which receives, in turn, each qualified RIB provider handle.
//
// If interface_name is NULL, all available HPROVIDERs are enumerated, regardless of
// the interface(s) they support.
//
// A return value of 0 indicates that no (further) providers are available.
// -------------------------------------------------------------------------

RIBDEC S32        RIBCALL RIB_enumerate_providers     (C8 FAR                  *interface_name,
                                                       HPROENUM FAR            *next,
                                                       HPROVIDER FAR           *dest);

// -------------------------------------------------------------------------
// RIB_type_string(data, subtype): Returns a pointer to a string representing
// the value of a variable of a given RIB data subtype, in a format consistent with
// that subtype.  data represents the value to be converted to a string;
// subtype represents the data subtype which determines the string's format
// (RIB_DEC, RIB_HEX, RIB_FLOAT, etc.)
//
// For example, a variable of type RIB_PERCENT will be treated as a floating-
// point percentage, with its value returned in a string of the form "100.0%"
// (without the quotes).
// -------------------------------------------------------------------------

RIBDEC C8 FAR *   RIBCALL RIB_type_string             (U32                      data,
                                                       RIB_DATA_SUBTYPE         subtype);

// -------------------------------------------------------------------------
// RIB_find_file_provider(interface_name, attribute_name, file_suffix) provides a
// convenient way for the application to locate a RIB provider which exports a given
// attribute name matching a specified name.
//
// This function uses the PROVIDER_query_attribute() function exported by compliant
// RIB interfaces to help an application locate a RIB which is capable of processing
// a given data format.  The file_suffix string may be specified as either a full filename
// ("c:\foo\bar\test.xyz") or simply the suffix in question (".xyz" or "*.xyz").
// interface_name and attribute_name should be set by the application to correspond to
// the interface and attribute names required to process files of the specified type.
//
// RIB provider interfaces may be made compatible with this function by providing a
// PROVIDER_query_attribute() handler exporting the appropriate attribute and filespec
// string compatible with Win32 common file-open dialogs.  As an example, the
// function
//
// U32 AILEXPORT PROVIDER_query_attribute (HATTRIB index)
// {
//    switch ((ATTRIB) index)
//       {
//       case PROVIDER_NAME:    return (U32) "MSS MPEG Audio Decoder";
//       case PROVIDER_VERSION: return 0x100;
//
//       case IN_FTYPES:        return (U32) "MPEG Layer 3 audio files\0*.MP3\0MPEG Layer 2 audio files\0*.MP2\0MPEG Layer 1 audio files\0*.MP1\0";
//       case OUT_FTYPES:       return (U32) "Raw PCM files\0*.RAW\0";
//       }
//
//    return 0;
// }
//
// will allow its provider to be returned to an application which calls RIB_find_file_provider()
// with the name of the interface which exports the PROVIDER_query_attribute() function
// and the attribute name corresponding to the IN_FTYPES or OUT_FTYPES attributes.  (It is up
// to each provider author to decide what attribute and interface names to make available to
// applications which conduct provider searches with this function.)
// -------------------------------------------------------------------------


RIBDEC HPROVIDER  RIBCALL RIB_find_file_provider      (C8 const FAR                  *interface_name,
                                                       C8 const FAR                  *attribute_name,
                                                       C8 const FAR                  *file_suffix);

// -------------------------------------------------------------------------
// RIB_find_files_provider is exactly like RIB_find_file_providers, except
// it can search for two file types at once.
// -------------------------------------------------------------------------

RIBDEC HPROVIDER  RIBCALL RIB_find_files_provider     (C8 const FAR                  *interface_name,
                                                       C8 const FAR                  *attribute_name_1,
                                                       C8 const FAR                  *file_suffix_1,
                                                       C8 const FAR                  *attribute_name_2,
                                                       C8 const FAR                  *file_suffix_2);


// -------------------------------------------------------------------------
// RIB_find_file_dec_provider is exactly like RIB_find_file_providers, except
// it can search for one file type and one decimal value
// -------------------------------------------------------------------------

RIBDEC HPROVIDER  RIBCALL RIB_find_file_dec_provider  (C8 const FAR                  *interface_name,
                                                       C8 const FAR                  *attribute_name_1,
                                                       U32                            attribute_value_1,
                                                       C8 const FAR                  *attribute_name_2,
                                                       C8 const FAR                  *file_suffix_2);


// -------------------------------------------------------------------------
// RIB_find_provider will find the provider that contains an attribute
// name matching the specified attribute value.
// -------------------------------------------------------------------------

RIBDEC HPROVIDER  RIBCALL RIB_find_provider           (C8 const FAR                  *interface_name,
                                                       C8 const FAR                  *attribute_name,
                                                       C8 const FAR                  *attribute_value);

// -------------------------------------------------------------------------
// RIB_load_application_providers(filespec) accepts a wildcard file
// specification of the form ("*.xyz"), and loads all RIB provider
// library DLLs named with the suffix .xyz.  Internally, it performs a
// file search in the current working directory, calling the RIB_load_provider_library()
// function for each provider DLL it finds.
//
// This function has no effect under DOS.
// -------------------------------------------------------------------------

RIBDEC S32        RIBCALL RIB_load_application_providers 
                                                      (C8 const FAR *filespec);

// -------------------------------------------------------------------------
// RIB_set_provider_user/system_data(provider, index, value) and
// RIB_provider_user/system_data(provider, index) allow the application and
// system to associate one or more generic 32-bit data values with a 
// specified RIB provider handle.  
//
// provider represents the provider handle; index, which may range from 0 to
// 7, represents the "slot" in which the data value is to be stored or 
// retrieved; and value, in the case of the data write functions,
// represents the value to be written.  
//
// Applications should not use the RIB_set_provider_system_data() or 
// RIB_provider_system_data() functions.  These functions are reserved 
// for use by RAD system components only.
// -------------------------------------------------------------------------

RIBDEC  void      RIBCALL RIB_set_provider_user_data  (HPROVIDER provider,
                                                       U32       index,
                                                       S32       value);

RIBDEC  S32       RIBCALL RIB_provider_user_data      (HPROVIDER provider,
                                                       U32       index);

RIBDEC  void      RIBCALL RIB_set_provider_system_data 
                                                      (HPROVIDER provider,
                                                       U32       index,
                                                       S32       value);

RIBDEC  S32       RIBCALL RIB_provider_system_data    (HPROVIDER provider,
                                                       U32       index);

// -------------------------------------------------------------------------
// RIB_error(): Returns ASCII text string associated with last error
// condition
// -------------------------------------------------------------------------

RIBDEC C8 FAR *   RIBCALL RIB_error                   (void);


#ifdef __cplusplus
}
#endif

#if defined(_PUSHPOP_SUPPORTED) || PRAGMA_STRUCT_PACKPUSH
  #pragma pack(pop)
#else
  #pragma pack()
#endif
#endif

