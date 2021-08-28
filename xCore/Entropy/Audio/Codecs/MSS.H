//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSS.H: Miles Sound System main header file                            ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Initial, derived from AIL.H V3.02          ##
//##          1.01 of 19-Jun-95: Added various functions for V3.03 release  ##
//##          1.02 of 22-Nov-95: C++ typedef problem fixed, declspecs added ##
//##          1.03 of 15-Feb-96: Changes for 16 bit callbacks and multiple  ##
//##                             16 bit DLL loads (JKR)                     ##
//##          1.04 of  2-Nov-97: Changes made to handle DLS in future       ##
//##                             versions                                   ##
//##          1.05 of  1-Jan-98: Massive changes for version 4.0            ##
//##          1.06 of 17-Sep-98: Massive changes for version 5.0            ##
//##          1.07 of  2-Feb-99: Changes for new input API                  ##
//##          1.08 of  8-Feb-99: Changes for new filter helper functions    ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################
//##  Stripped down for Gamecube by CJ                                      ##
//############################################################################

#include "x_types.hpp"

//
// General type definitions for portability
//

#ifndef C8
#define C8 char
#endif

#ifndef U8
#define U8 u8
#endif

#ifndef S8
#define S8 s8
#endif

#ifndef U16
#define U16 u16
#endif

#ifndef S16
#define S16 s16
#endif

#ifndef U32
#define U32 u32
#endif

#ifndef S32
#define S32 s32
#endif

#ifndef F32
#define F32 f32
#endif

#ifndef F64
#define F64 f64
#endif

#define AILCALLBACK
#define AILCALL
#define AILEXPORT

//
// Handle representing token used to obtain attribute or preference
// data from RIB provider
//

typedef U32 HATTRIB;

// ----------------------------------
// Application-provided ASI callbacks
// ----------------------------------

//
// AILASIFETCHCB: Called by ASI to obtain data from stream source
//
// offset normally will be either 0 at the first call made by the codec
// or -1 to specify a continuous stream, except when ASI_stream_seek()
// is called to restart the stream codec at a new stream offset.  In this
// case, the application must execute the seek operation on the ASI codec's
// behalf.
//
// In response to this callback, the application should read the requested
// data and copy it to the specified destination buffer, returning the number
// of bytes copied (which can be less than bytes_requested if the end of
// the stream is reached).
//

typedef S32 (AILCALLBACK* AILASIFETCHCB) (U32   user,            // User value passed to ASI_open_stream()
                                          void* dest,            // Location to which stream data should be copied by app
                                          S32   bytes_requested, // # of bytes requested by ASI codec
                                          S32   offset);         // If not -1, application should seek to this point in stream

#define AIL_MAX_FILE_HEADER_SIZE 4096     // AIL_set_named_sample_file() requires at least 4K
                                          // of data or the entire file image, whichever is less,
                                          // to determine sample format

//
// Standard RAD Interface Broker provider identification attributes
//

#define PROVIDER_NAME    (-100)  // RIB_STRING name of decoder
#define PROVIDER_VERSION (-101)  // RIB_HEX BCD version number

typedef S32 ASIRESULT;

#define ASI_NOERR                   0   // Success -- no error
#define ASI_NOT_ENABLED             1   // ASI not enabled
#define ASI_ALREADY_STARTED         2   // ASI already started
#define ASI_INVALID_PARAM           3   // Invalid parameters used
#define ASI_INTERNAL_ERR            4   // Internal error in ASI driver
#define ASI_OUT_OF_MEM              5   // Out of system RAM
#define ASI_ERR_NOT_IMPLEMENTED     6   // Feature not implemented
#define ASI_NOT_FOUND               7   // ASI supported device not found
#define ASI_NOT_INIT                8   // ASI not initialized
#define ASI_CLOSE_ERR               9   // ASI not closed correctly

//
// Handle to stream being managed by ASI codec
//

typedef S32 HASISTREAM;
