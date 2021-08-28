//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSDLS.H: DLS control library                                         ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Initial                                    ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#ifndef AIL_DLS_VERSION

#define AIL_DLS_VERSION  1
#define AIL_DLS_REVISION 0

#if defined(IS_DOS) || defined(IS_MAC)

#define FOURCC_LIST  mmioFOURCC('L','I','S','T')
#define FOURCC_RIFF  mmioFOURCC('R','I','F','F')

#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// low-level DLS stuff
//

#define DLS_NOERR                   0   // Success -- no error
#define DLS_NOT_ENABLED             1   // DLS device is not enabled
#define DLS_MEM_ERR                 2   // Memory error
#define DLS_ALREADY_OPENED          3   // DLS device is already opened
#define DLS_INVALID_PARAM           4   // Invalid parameters used
#define DLS_INVALID_HANDLE          5   // DLS handle is invalid
#define DLS_FILE_ERR                6   // File I/O failure
#define DLS_INTERNAL_ERR            7   // Internal error in DLS driver
#define DLS_OUT_OF_MEM              8   // Out of system RAM
#define DLS_FILE_NOT_LOADED         9   // DLS file has not been loaded
#define DLS_FILE_NOT_FOUND          10  // Specified DLS file not found
#define DLS_INVALID_FILE            11  // Invalid DLS file format or contents
#define DLS_ERR_EXISTING_PATCH      12  // Patch already exists
#define DLS_ERR_MIDI_MSG_Q_OVERFLOW 13  // Internal error in DLS driver
#define DLS_ERR_COLLECTION_INVALID  14  // Collection is not valid
#define DLS_ERR_NOT_IMPLEMENTED     15  // Feature not implemented
#define DLS_NOT_FOUND               16  // DLS device not found
#define DLS_NOT_INIT                17  // DLS not initialized
#define DLS_UNLOAD_ERR              18  // DLS collection was not unloaded correctly
#define DLS_CLOSE_ERR               19  // DLS device not closed correctly
#define DLS_SIZE_TOO_BIG            20  // Requested buffer size is too big

//
// Structure for the DLSGetInfo call
//
// Note: The application must fill in the dwSize value before 
// calling DLSGetInfo
//

typedef struct
{
   S32 dwSize;         // Size must be filled in by the application
   S32 dwVersion;      // Version of the driver
   S32 dwHwStatus;     // Status
   S32 dwMaxDlsMem;    // Maximum amount of memory available for DLS
   S32 dwCurrDlsMem;   // Current amount of memory available for downloading
   S32 dwLargestBuf;   // Size of the largest single buffer or file that
                       // can be downloaded
   S32 dwGMBankStatus; // Status of the GM bank
                       //   Bit 0 = 1 means GM bank is available.
                       //   Bit 0 = 0 means GM bank is not available.
                       //   Bit 2-31 are reserved
   S32 dwGMBankSize;   // Size of the buffer reserved for the GM bank
}
DLS_INFO_STRUC;

//
// Parameters for the dwFlag used in DLSOpen()
//

#define GM_BANK_NOT_REQUIRED 0x00000001


//
// Parameters for the dwFlag used in DLSUnloadAll()
//

#define DLS_REMOVE_ONLY_MINE        0x00000001
#define DLS_REMOVE_ALL              0x00000000

//
// Parameters for the dwFlag used in DLSLoadFile() and flags in AIL_DLS_load_*
//

#define DLS_OVERWRITE_EXISTING_PATCH 0x00000001

//
// Parameters for the dwAttribute used in DLSSetAttribute()
//

#define DLS_SET_DLS_BUFSIZE         0x00000001

//
// Structures for lpDlsParam used in DLSSetAttribute()
// (used if dwAttribute == DLS_SET_DLS_BUFSIZE)
//

typedef struct
{
   S32 dwSize;         // Size must be filled in by the caller
   S32 dwBufSize;      // New size in bytes for the DLS buffer
}
DLS_SET_DLS_BUFSIZE_PARAM;

//
// DLS return codes
//

// ----------------------------------
// Application-provided MSS callbacks
// ----------------------------------

//
// Called by DLS to return half-buffer (0 or 1) where play cursor
// currently resides
//

typedef S32    (AILCALLBACK FAR * AILDLSPCB) (U32 user);

//
// Called by DLS to lock partial buffer for writing, returning its address
//
// Size of locked region in bytes is specified at configuration time
//

typedef void FAR * (AILCALLBACK FAR * AILDLSLCB) (U32 user, S32 buffer_section);

//
// Called by DLS to release lock on partial buffer, indicating partial buffer
// has been filled with valid output data
//

typedef void   (AILCALLBACK FAR * AILDLSUCB) (U32 user, S32 buffer_section);


//
// MSS-specific DLS provider extensions
//
// If GetProcAddress() fails on any of these functions (Win32 version), the
// DLS provider does not require any configuration -- use DLSOpen() instead
//

DXDEC S32 AILCALL DLSMSSOpen      (S32 FAR   *lpdwHandle,
                                   S32        dwFlags,
                                   HMDIDRIVER MIDI_driver,        // XMIDI driver handle
                                   S32        output_format,      // DIG_F format for output
                                   S32        output_sample_rate, // Output samples/second
                                   S32        buffer_size,        // Size of each buffer in bytes
                                   U32        user,               // User value passed to buffer callbacks
                                   AILDLSPCB  buffer_poll_CB,     // Buffer polling handler
                                   AILDLSLCB  buffer_lock_CB,     // Buffer lock handler
                                   AILDLSUCB  buffer_unlock_CB);  // Buffer unlock handler

//
// Standard DLS API prototypes (patterned after S3 SDK)
//

DXDEC S32 AILCALL DLSOpen          (S32 FAR *lpdwHandle,
                                    S32      dwFlags);

DXDEC S32 AILCALL DLSClose         (S32      dwDLSHandle,
                                    S32      dwFlags);

DXDEC S32 AILCALL DLSLoadFile      (S32      dwDLSHandle,
                                    S32      dwFlags,
                                    S32  FAR *lpdwDownloadID,
                                    char const FAR*lpFileName);

#ifdef IS_MAC

DXDEC S32 AILCALL DLSFSSLoadFile   (S32      dwDLSHandle,
                                    S32      dwFlags,
                                    S32  FAR *lpdwDownloadID,
                                    FSSpec const FAR*lpFileName);

#endif

DXDEC S32 AILCALL DLSLoadMemFile   (S32      dwDLSHandle,
                                    S32      dwFlags,
                                    S32 FAR *lpdwDownloadID,
                                    void const FAR*lpMemPtr);

DXDEC S32 AILCALL DLSGetInfo       (DLS_INFO_STRUC FAR *lpDlsInfo);

DXDEC S32 AILCALL DLSCompactMemory (S32      dwDLSHandle);

DXDEC S32 AILCALL DLSUnloadAll     (S32      dwDLSHandle,
                                   S32      dwFlags);

DXDEC S32 AILCALL DLSMSSGetCPU     (S32      dwDLSHandle);

DXDEC S32 AILCALL DLSUnloadFile    (S32      dwDLSHandle,
                                    S32      dwDownloadID);

DXDEC S32 AILCALL DLSSetAttribute  (S32      dwDLSHandle,
                                    S32      dwAttribute,
                                    void FAR*lpDlsParam);

typedef S32 (AILCALL FAR *DLSMSSOPEN)(S32 FAR   *lpdwHandle,
                                      S32        dwFlags,
                                      HMDIDRIVER MIDI_driver,        // XMIDI driver handle
                                      S32        output_format,      // DIG_F format for output
                                      S32        output_sample_rate, // Output samples/second
                                      S32        buffer_size,        // Size of each buffer in bytes
                                      U32        user,               // User value passed to buffer callbacks
                                      AILDLSPCB  buffer_poll_CB,     // Buffer polling handler
                                      AILDLSLCB  buffer_lock_CB,     // Buffer lock handler
                                      AILDLSUCB  buffer_unlock_CB);  // Buffer unlock handler

typedef S32 (AILCALL FAR *DLSOPEN)          (S32 FAR *lpdwHandle,
                                             S32      dwFlags);

typedef S32 (AILCALL FAR *DLSCLOSE)         (S32      dwDLSHandle,
                                             S32      dwFlags);

typedef S32 (AILCALL FAR *DLSLOADFILE)      (S32      dwDLSHandle,
                                             S32      dwFlags,
                                             S32 FAR *lpdwDownloadID,
                                             char const FAR*lpFileName);

#ifdef IS_MAC

typedef S32 (AILCALL FAR *DLSFSSLOADFILE)   (S32      dwDLSHandle,
                                             S32      dwFlags,
                                             S32 FAR *lpdwDownloadID,
                                             FSSpec const FAR*lpFileName);

#endif

typedef S32 (AILCALL FAR *DLSLOADMEMFILE)     (S32      dwDLSHandle,
                                               S32      dwFlags,
                                               S32 FAR *lpdwDownloadID,
                                               void const FAR*lpMemPtr);

typedef S32 (AILCALL FAR *DLSGETINFO)           (DLS_INFO_STRUC FAR *lpDlsInfo);

typedef S32 (AILCALL FAR *DLSCOMPACTMEMORY)     (S32      dwDLSHandle);

typedef S32 (AILCALL FAR *DLSUNLOADALL)         (S32      dwDLSHandle,
                                                 S32      dwFlags);

typedef S32 (AILCALL FAR *DLSUNLOADFILE)        (S32      dwDLSHandle,
                                                 S32      dwDownloadID);

typedef S32 (AILCALL FAR *DLSSETATTRIBUTE)      (S32       dwDLSHandle,
                                                 S32       dwAttribute,
                                                 void FAR* lpDlsParam);



#ifdef __cplusplus
}
#endif

#endif
