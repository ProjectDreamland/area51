/******
nonport.h
GameSpy Common Code
  
Copyright 1999-2002 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
******/

#ifndef _NONPORT_H_
#define _NONPORT_H_

//*****
//*****
//***** THIS IS SO WE CAN REPLACE SOME OF THE GAMESPY GENERIC NETWORK FUNCTIONALITY
//*****
//*****
#include "EntropyAbstraction.h"

#ifndef GSI_MEM_ONLY

#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__mips64) && !defined(_WIN32)
    #define _MACOS
#endif

#ifdef __mips64
    #if !defined(SN_SYSTEMS) && !defined(EENET) && !defined(ENTROPY_NETWORK)
        #define EENET
    #endif
    #define _PS2
#endif

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    // Added by Saad Nader on 08-02-2004
    // support for winsock
    #ifdef __WINSOCK_2_0__
        #include <WINSOCK2.H>
    #else
        #include <winsock.h>
    #endif
    // end added
#else
#ifdef _MACOS
    #include <events.h>
    #include "mwinsock.h"
    #define GS_BIG_ENDIAN
#else
#ifdef _PS2
    #define GS_BIG_ENDIAN

#if defined(ENTROPY_NETWORK)
#else
    #ifdef EENET
        #include <libeenet.h>
        #include <eenetctl.h>
        #include <ifaddrs.h>
        #include <sys/socket.h>
        #include <sys/errno.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <net/if.h>
        #include <sys/select.h>
        #include <malloc.h>
    #endif
    #include <eekernel.h>
    #include <stdio.h>
    #include <malloc.h>
    #include <sifdev.h>
    #include <sifrpc.h>
    #include <sifcmd.h>
    #include <ilink.h>
    #include <ilsock.h>
    #include <ilsocksf.h>
    #ifdef SN_SYSTEMS
        // undefine socket defines from sys/types.h
        // This is to workaround sony now automatically including sys/types.h
        // and SNSystems having not produce a patch yet (they'll likely do the same since
        // the SNSystems fd_set is a slightly different size than the sys/types.h.
        #undef FD_CLR    
        #undef FD_ZERO
        #undef FD_SET    
        #undef FD_ISSET
        #undef FD_SETSIZE
        #undef fd_set
        #include "snskdefs.h"
        #include "sntypes.h"
        #include "snsocket.h"
        #include "sneeutil.h"
        #include "sntcutil.h"
    #endif // SN_SYSTEMS
    #ifdef INSOCK
        #include "libinsck.h"
        #include "libnet.h"
        #include "sys/errno.h"
        //#include "libmrpc.h"
    #endif // INSOCK
#endif
#else //UNIX
    #define UNDER_UNIX
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <ctype.h>
    #include <errno.h>
    #include <sys/time.h>
    #include <limits.h>
    //#include <sys/syslimits.h>
    #include <netinet/tcp.h>
#endif
#endif
#endif

#ifdef UNDER_CE
#include <platutil.h>
#endif

#if defined(ENTROPY_NETWORK) && defined(X_DEBUG)
#define assert(a) if( !(a) ) asm("break 1" );
#else
#if !defined(UNDER_CE)
#include <assert.h>
#else
#define assert(a)
#endif
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GSI_MEM_TRACK
    #if defined(ENTROPY_NETWORK)
        void* gsimalloc( size_t size );
        void  gsifree  ( void* ptr );
        void* gsirealloc( void* ptr, size_t size );
    #else
        #define gsimalloc malloc
        #define gsifree free
        #define gsirealloc realloc
        #define gsimemalign memalign
    #endif
#else
    void * gsimalloctrack(size_t size);
    void gsifreetrack(void * ptr);
    void * gsirealloctrack(void * ptr, size_t size);
    #define gsimalloc gsimalloctrack
    #define gsifree gsifreetrack
    #define gsirealloc gsirealloctrack
#endif

#ifndef GSI_MEM_ONLY
    
// Get rid of compiler warnings when parameters are never used
#if defined(__MWERKS__) || defined(WIN32)
    #define GSI_UNUSED(x) x
#else
#if defined(ENTROPY_NETWORK)
    #define GSI_UNUSED(x) (void)x
#else
    #define GSI_UNUSED(x)
#endif
#endif

#if defined(_INTEGRAL_MAX_BITS)
    #define GSI_MAX_INTEGRAL_BITS   _INTEGRAL_MAX_BITS
#else
    #define GSI_MAX_INTEGRAL_BITS   32
#endif


// common base type defines, please refer to ranges below when porting
typedef char              gsi_i8;
typedef unsigned char     gsi_u8;
typedef short             gsi_i16;
typedef unsigned short    gsi_u16;
typedef int               gsi_i32;
typedef unsigned int      gsi_u32;
typedef gsi_i32           goa_int32;  // 2003.Oct.04.JED - typename depreciated
typedef gsi_u32           goa_uint32; //  these types will be removed once all SDK's are updated

// Platform dependent types
#ifdef _PS2
    typedef signed long           gsi_i64;
    typedef unsigned long         gsi_u64;
    typedef unsigned int          gsi_time; // must be int (32bits), not long (64bits)
#elif defined(UNDER_UNIX)
    typedef long long             gsi_i64;
    typedef unsigned long long    gsi_u64;
    typedef unsigned int          gsi_time; // must be int (32bits), not long (64bits)
#else
    #if (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
    typedef __int64               gsi_i64;
    typedef unsigned __int64      gsi_u64;
    #endif
    typedef unsigned long         gsi_time;
#endif // 64 bit types


#ifndef GSI_UNICODE
    #define gsi_char  char
#else
    #define gsi_char  unsigned short
#endif // goa_char

// expected ranges for integer types
#define GSI_MIN_I8      (-127 - 1)
#define GSI_MAX_I8        127
#define GSI_MAX_U8        0xFF

#define GSI_MIN_I16     (-32767i16 - 1)
#define GSI_MAX_I16       32767i16
#define GSI_MAX_U16       0xffffui16

#define GSI_MIN_I32     (-2147483647 - 1)
#define GSI_MAX_I32       2147483647
#define GSI_MAX_U32       0xffffffffui32

#if (GSI_MAX_INTEGRAL_BITS >= 64)
#define GSI_MIN_I64     (-9223372036854775807 - 1)
#define GSI_MAX_I64       9223372036854775807
#define GSI_MAX_U64       0xffffffffffffffffui64
#endif

gsi_time current_time();
gsi_time current_time_hires();   // microseconds
void msleep(gsi_time msec);      // milliseconds

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Cross platform random number generator
void Util_RandSeed(unsigned long seed); // to seed it
int  Util_RandInt(int low, int high);   // retrieve a random int

// Base 64 encoding (printable characters)
void B64Encode(const char *input, char *output, int inlen, int useAlternateEncoding);
void B64Decode(char *input, char *output, int *len, int useAlternateEncoding);

void SocketStartUp();
void SocketShutDown();

#ifndef SOCKET_ERROR 
    #define SOCKET_ERROR (-1)
#endif

#ifndef INADDR_NONE
   #define INADDR_NONE 0xffffffff
#endif

#ifndef INVALID_SOCKET 
    #define INVALID_SOCKET (-1)
#endif

#if defined(_WIN32) && !defined(UNDER_CE)
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#else    
    char *_strlwr(char *string);
    char *_strupr(char *string);
#endif

#undef strdup
#define strdup goastrdup
#undef _strdup
#define _strdup goastrdup
char * goastrdup(const char *src);

#if defined(_MACOS) || defined(UNDER_CE)
    int strcasecmp(const char *string1, const char *string2);
    int strncasecmp(const char *string1, const char *string2, size_t count);
#endif

#ifdef SN_SYSTEMS
    #define IPPROTO_TCP PF_INET
    #define IPPROTO_UDP PF_INET
    #define FD_SETSIZE SN_MAX_SOCKETS
#endif

#if !defined(_MACOS) && !defined(_WIN32)
    #define SOCKET int
    
    #ifdef SN_SYSTEMS
        int GOAGetLastError(SOCKET s);
    #endif

    #ifdef EENET
        #define GOAGetLastError(s) sceEENetErrno
        #define closesocket        sceEENetClose
    #endif

    #ifdef INSOCK
        //#define NETBUFSIZE (sceLIBNET_BUFFERSIZE)
        #define NETBUFSIZE (32768)

        #define GOAGetLastError(s) sceInsockErrno  // not implemented
        #define closesocket(s)       sceInsockShutdown(s,SCE_INSOCK_SHUT_RDWR)
    #endif

    #ifdef UNDER_UNIX
        #define GOAGetLastError(s) errno
        #define closesocket        close //on unix
    #endif

    #define ioctlsocket ioctl

    #define WSAEWOULDBLOCK EWOULDBLOCK             
    #define WSAEINPROGRESS EINPROGRESS             
    #define WSAEALREADY EALREADY                
    #define WSAENOTSOCK ENOTSOCK                
    #define WSAEDESTADDRREQ EDESTADDRREQ            
    #define WSAEMSGSIZE EMSGSIZE                
    #define WSAEPROTOTYPE EPROTOTYPE              
    #define WSAENOPROTOOPT ENOPROTOOPT             
    #define WSAEPROTONOSUPPORT EPROTONOSUPPORT         
    #define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT         
    #define WSAEOPNOTSUPP EOPNOTSUPP              
    #define WSAEPFNOSUPPORT EPFNOSUPPORT            
    #define WSAEAFNOSUPPORT EAFNOSUPPORT            
    #define WSAEADDRINUSE EADDRINUSE              
    #define WSAEADDRNOTAVAIL EADDRNOTAVAIL           
    #define WSAENETDOWN ENETDOWN                
    #define WSAENETUNREACH ENETUNREACH             
    #define WSAENETRESET ENETRESET               
    #define WSAECONNABORTED ECONNABORTED            
    #define WSAECONNRESET ECONNRESET              
    #define WSAENOBUFS ENOBUFS                 
    #define WSAEISCONN EISCONN                 
    #define WSAENOTCONN ENOTCONN                
    #define WSAESHUTDOWN ESHUTDOWN               
    #define WSAETOOMANYREFS ETOOMANYREFS            
    #define WSAETIMEDOUT ETIMEDOUT               
    #define WSAECONNREFUSED ECONNREFUSED            
    #define WSAELOOP ELOOP                   
    #define WSAENAMETOOLONG ENAMETOOLONG            
    #define WSAEHOSTDOWN EHOSTDOWN               
    #define WSAEHOSTUNREACH EHOSTUNREACH            
    #define WSAENOTEMPTY ENOTEMPTY               
    #define WSAEPROCLIM EPROCLIM                
    #define WSAEUSERS EUSERS                  
    #define WSAEDQUOT EDQUOT                  
    #define WSAESTALE ESTALE                  
    #define WSAEREMOTE EREMOTE
    #define WSAEINVAL EINVAL
#else // !defined(_MACOS) && !defined(_WIN32)
    #define GOAGetLastError(s) WSAGetLastError()
#endif

#ifndef _WIN32
    typedef struct sockaddr SOCKADDR;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct in_addr IN_ADDR;
    typedef struct hostent HOSTENT;
    typedef struct timeval TIMEVAL;
#endif

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
    #define PATHCHAR '\\'
#else
#ifdef MACOS
    #define PATHCHAR ':'
#else
    #define PATHCHAR '/'
#endif
#endif

int SetSockBlocking(SOCKET sock, int isblocking);
int DisableNagle(SOCKET sock);
int SetReceiveBufferSize(SOCKET sock, int size);
int SetSendBufferSize(SOCKET sock, int size);
int GetReceiveBufferSize(SOCKET sock);
int GetSendBufferSize(SOCKET sock);
int CanReceiveOnSocket(SOCKET sock);
int CanSendOnSocket(SOCKET sock);
int GSISocketSelect(SOCKET theSocket, int* theReadFlag, int* theWriteFlag, int* theExceptFlag);

HOSTENT * getlocalhost(void);

int IsPrivateIP(IN_ADDR * addr);

// SN Systems doesn't support gethostbyaddr
#if defined(SN_SYSTEMS)
    #define gethostbyaddr(a,b,c)   NULL
#endif

// async way to resolve a hostname to an IP
typedef struct GSIResolveHostnameInfo * GSIResolveHostnameHandle;
#define GSI_STILL_RESOLVING_HOSTNAME   0
#define GSI_ERROR_RESOLVING_HOSTNAME   0xFFFFFFFF

// start resolving a hostname
// returns 0 on success, -1 on error
int GSIStartResolvingHostname(const char * hostname, GSIResolveHostnameHandle * handle);
// cancel a resolve in progress
void GSICancelResolvingHostname(GSIResolveHostnameHandle handle);
// returns GSI_STILL_RESOLVING if still resolving the hostname
// returns GSI_ERROR_RESOLVING if it was unable to resolve the hostname
// on success, returns the IP of the host in network byte order
unsigned int GSIGetResolvedIP(GSIResolveHostnameHandle handle);
#if defined(UNDER_CE)
//CE does not have the stdlib time() call
    time_t time(time_t *timer);
#else
    #include <time.h>
#endif

#ifndef UNDER_CE
    #include <ctype.h>
#else
    int isdigit(int c);
    int isxdigit(int c);
    int isalnum(int c);
    int isspace(int c);
#endif

#if defined(UNDER_CE) || defined(_PS2)
    #define NOFILE
#endif

#ifndef SOMAXCONN
    #define SOMAXCONN 5
#endif

typedef const char * (* GetUniqueIDFunction)();

extern GetUniqueIDFunction GOAGetUniqueID;

// Prototypes so the compiler won't warn
#ifdef _PS2
extern int wprintf(const wchar_t*,...);
#endif

#endif // #ifndef GSI_MEM_ONLY

#ifdef __cplusplus
}
#endif

#endif 
