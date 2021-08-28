
#include "x_files.hpp"
#include "Downloader.hpp"

#if defined( TARGET_PS2 )
#include "NetworkMgr/GameSpy/ghttp/ghttp.h"

GHTTPBool http_completed( GHTTPRequest request, GHTTPResult result, char * buffer, int bufferLen, void * param );
void http_progress( GHTTPRequest Request, GHTTPState State, const char* pBuffer, GHTTPByteCount BufferSize, GHTTPByteCount Received, GHTTPByteCount TotalSize, void* userdata );

#endif

//----------------------------------------------------------
//----------------------------------------------------------
xbool downloader::Init( const char* URL )
{
    m_Length    = 0;
    m_pData     = NULL;
    m_Status    = DL_STAT_BUSY;
    m_Progress  = 0.0f;

#if defined(TARGET_PS2)
    ghttpStartup();
    ghttpGetEx(     URL,            // URL
                    NULL,           // headers to post with url
                    NULL,           // Buffer
                    0,              // Buffer size
                    NULL,           // Additional Post data
                    GHTTPFalse,     // Throttle
                    GHTTPFalse,     // Blocking
                    http_progress,  // Progress callback
                    http_completed, // Completion callback 
                    this );
    return TRUE;
#endif

    m_Status = DL_STAT_NOT_FOUND;
    return FALSE;
}

//----------------------------------------------------------
//----------------------------------------------------------
void downloader::Kill( void )
{
    if( m_pData )
    {
        x_free( m_pData );
        m_pData = NULL;
    }

#if defined(TARGET_PS2)
    ghttpCleanup();
#endif
}

//----------------------------------------------------------
//----------------------------------------------------------
void downloader::Update( f32 )
{
#if defined(TARGET_PS2)
    ghttpThink();
#endif
}

//----------------------------------------------------------
//----------------------------------------------------------
void downloader::DownloadComplete( download_status Status, void* pData, s32 Length )
{
    if( m_pData )
    {
        x_free( m_pData );
        m_pData = NULL;
    }

    if( Status == DL_STAT_OK )
    {
        m_pData = (byte*)x_malloc( Length+1 );
        m_Length = Length;
        if( m_pData == NULL )
        {
            m_Length = 0;
            m_Status = DL_STAT_ERROR;
            return;
        }
        x_memcpy( m_pData, pData, Length );
        m_pData[Length] = 0x0;
    }

    m_Status = Status;
}

//----------------------------------------------------------
//----------------------------------------------------------
#if defined(TARGET_PS2)
GHTTPBool http_completed( GHTTPRequest request, GHTTPResult result, char * buffer, int bufferLen, void * param )
{
    downloader* pDownloader = (downloader*)param;
    (void)request;

    switch( result )
    {
    case GHTTPSuccess:                  pDownloader->DownloadComplete( DL_STAT_OK, buffer, bufferLen ); break;
    case GHTTPFileNotFound:             pDownloader->DownloadComplete( DL_STAT_NOT_FOUND );             break;
    case GHTTPHostLookupFailed:         pDownloader->DownloadComplete( DL_STAT_NOT_FOUND );             break;
    default:                            pDownloader->DownloadComplete( DL_STAT_ERROR, NULL, 0 );        break;
    }
    return GHTTPTrue;
}

void http_progress( GHTTPRequest Request, GHTTPState State, const char* pBuffer, GHTTPByteCount BufferSize, GHTTPByteCount Received, GHTTPByteCount TotalSize, void* userdata )
{
    downloader* pDownloader = (downloader*)userdata;
    f32 Progress;

    (void)Request;
    (void)State;
    (void)pBuffer;
    (void)BufferSize;

    if( TotalSize > 0 )
    {
        Progress = (f32)Received / (f32)TotalSize;
    }
    else
    {
        Progress = 0.0f;
    }
    pDownloader->SetProgress( Progress );

}
#endif

#if defined(TARGET_PS2) && defined(X_DEBUG)
const char* GetName( GHTTPResult Result )
{
    switch( Result )
    {
    case GHTTPSuccess:          return "GHTTPSuccess";
    case GHTTPOutOfMemory:      return "GHTTPOutOfMemory";
    case GHTTPBufferOverflow:   return "GHTTPBufferOverflow";
    case GHTTPParseURLFailed:   return "GHTTPParseURLFailed";
    case GHTTPHostLookupFailed: return "GHTTPHostLookupFailed";
    case GHTTPSocketFailed:     return "GHTTPSocketFailed";
    case GHTTPConnectFailed:    return "GHTTPConnectFailed";
    case GHTTPBadResponse:      return "GHTTPBadResponse";
    case GHTTPRequestRejected:  return "GHTTPRequestRejected";
    case GHTTPUnauthorized:     return "GHTTPUnauthorized";
    case GHTTPForbidden:        return "GHTTPForbidden";
    case GHTTPFileNotFound:     return "GHTTPFileNotFound";
    case GHTTPServerError:      return "GHTTPServerError";
    case GHTTPFileWriteFailed:  return "GHTTPFileWriteFailed";
    case GHTTPFileReadFailed:   return "GHTTPFileReadFailed";
    case GHTTPFileIncomplete:   return "GHTTPFileIncomplete";
    case GHTTPFileToBig:        return "GHTTPFileToBig";
    default:                    return "<unknown>";
    }
}
#endif