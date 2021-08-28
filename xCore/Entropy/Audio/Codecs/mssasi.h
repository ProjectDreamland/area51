//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSASI.H: Audio Stream Interface RIB services                         ##
//##                                                                        ##
//##  Version 1.00 of 6-Apr-98: Initial                                     ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#ifndef AIL_ASI_VERSION

#define AIL_ASI_VERSION  1
#define AIL_ASI_REVISION 0

#ifndef RIB_H
#include "rib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IS_WINDOWS
#pragma pack(1)
#endif

#ifndef C8
#define C8 char
#endif

#ifndef SINGLE
#define SINGLE float
#endif

#ifndef DOUBLE
#define DOUBLE double
#endif

// -------------------------------------------------------------------------
// Handle to stream being managed by ASI codec
// -------------------------------------------------------------------------

typedef S32 HASISTREAM;

// -------------------------------------------------------------------------
// ASI result codes
// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
// Application-provided ASI callbacks
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------

typedef S32 (AILCALLBACK FAR * AILASIFETCHCB) (U32       user,            // User value passed to ASI_open_stream()
                                               void FAR *dest,            // Location to which stream data should be copied by app
                                               S32       bytes_requested, // # of bytes requested by ASI codec
                                               S32       offset);         // If not -1, application should seek to this point in stream

//############################################################################
//##                                                                        ##
//## Interface "ASI codec"                                                  ##
//##                                                                        ##
//############################################################################

// -------------------------------------------------------------------------
// Initialize ASI stream codec
//
// No other ASI functions may be called outside an ASI_startup() / 
// ASI_shutdown() pair, except for the standard RIB function 
// PROVIDER_query_attribute().  All provider attributes must be accessible
// without starting up the codec.
// -------------------------------------------------------------------------

typedef ASIRESULT (AILCALL FAR *ASI_STARTUP)(void);

// -------------------------------------------------------------------------
// Shut down ASI codec
// -------------------------------------------------------------------------

typedef ASIRESULT (AILCALL FAR *ASI_SHUTDOWN)(void);

// -------------------------------------------------------------------------
// Return codec error message, or NULL if no errors have occurred since
// last call
//
// The ASI error text state is global to all streams
// -------------------------------------------------------------------------

typedef C8 FAR *  (AILCALL FAR *ASI_ERROR)(void);

//############################################################################
//##                                                                        ##
//## Interface "ASI stream"                                                 ##
//##                                                                        ##
//############################################################################

// -------------------------------------------------------------------------
// Open a stream, returning handle to stream
// -------------------------------------------------------------------------

typedef HASISTREAM (AILCALL FAR *ASI_STREAM_OPEN) (U32           user,              // User value passed to fetch callback
                                                   AILASIFETCHCB fetch_CB,          // Source data fetch handler
                                                   U32           total_size);       // Total size for %-done calculations (0=unknown)

// -------------------------------------------------------------------------
// Translate data in stream, returning # of bytes actually decoded or encoded
// 
// Any number of bytes may be requested.  Requesting more data than is 
// available in the codec's internal buffer will cause the AILASIFETCHCB
// handler to be called to fetch more data from the stream.
// -------------------------------------------------------------------------

typedef S32  (AILCALL FAR *ASI_STREAM_PROCESS) (HASISTREAM  stream,              // Handle of stream
                                                void FAR   *buffer,              // Destination for processed data
                                                S32         buffer_size);        // # of bytes to return in buffer

// -------------------------------------------------------------------------
// Restart stream decoding process at new offset
//
// Relevant for decoders only
//
// Seek destination is given as offset in bytes from beginning of stream
//
// At next ASI_stream_process() call, decoder will seek to the closest possible 
// point in the stream which occurs at or after the specified position
//
// This function has no effect for decoders which do not support random 
// seeks on a given stream type
//
// Warning: some decoders may need to implement seeking by reparsing 
// the entire stream up to the specified offset, through multiple calls
// to the data-fetch callback.  This operation may be extremely 
// time-consuming on large files or slow network connections.
//
// A stream_offset value of -1 may be used to inform the decoder that the
// application has changed the input stream offset on its own, e.g. for a 
// double-buffering application where the ASI decoder is not accessing the
// stream directly.  ASI decoders should respond to this by flushing all 
// internal buffers and resynchronizing themselves to the data stream.
// -------------------------------------------------------------------------

typedef ASIRESULT (AILCALL FAR *ASI_STREAM_SEEK)    (HASISTREAM stream,          // Handle of stream
                                                     S32        stream_offset);  // New offset for decoding

// -------------------------------------------------------------------------
// Retrieve an ASI stream attribute or preference value by index
// -------------------------------------------------------------------------

typedef S32 (AILCALL FAR *ASI_STREAM_ATTRIBUTE) (HASISTREAM stream,     // Handle of stream
                                                 HATTRIB    attrib);    // RIB token for desired attribute

// -------------------------------------------------------------------------
// Set an ASI stream preference value by index
//
// Returns previous preference value
// -------------------------------------------------------------------------

typedef S32 (AILCALL FAR *ASI_STREAM_SET_PREFERENCE) (HASISTREAM stream,         // Handle of stream
                                                      HATTRIB    preference,     // RIB token for desired preference
                                                      S32        value);         // New value for preference

// -------------------------------------------------------------------------
// Close stream, freeing handle and all internally-allocated resources
// -------------------------------------------------------------------------

typedef ASIRESULT (AILCALL FAR *ASI_STREAM_CLOSE) (HASISTREAM stream);        // Handle of stream to close

#ifdef IS_WINDOWS
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif
