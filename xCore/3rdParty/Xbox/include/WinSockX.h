/* Winsock2.h -- definitions to be used with the WinSock 2 DLL and
 *               WinSock 2 applications.
 *
 * This header file corresponds to version 2.2.x of the WinSock API
 * specification.
 *
 * This file includes parts which are Copyright (c) 1982-1986 Regents
 * of the University of California.  All rights reserved.  The
 * Berkeley Software License Agreement specifies the terms and
 * conditions for redistribution.
 */

#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */


/*
 * Ensure structures are packed consistently.
 */

#ifndef _WIN64
#include <pshpack4.h>
#endif

/*
 * Default: include function prototypes, don't include function typedefs.
 */

#ifndef INCL_WINSOCK_API_PROTOTYPES
#define INCL_WINSOCK_API_PROTOTYPES 1
#endif

#ifndef INCL_WINSOCK_API_TYPEDEFS
#define INCL_WINSOCK_API_TYPEDEFS 0
#endif

/*
 * Pull in WINDOWS.H if necessary
 */
#ifndef _INC_WINDOWS
#include <windows.h>
#endif /* _INC_WINDOWS */

/*
 * Define the current Winsock version. To build an earlier Winsock version
 * application redefine this value prior to including Winsock2.h.
 */

#if !defined(MAKEWORD)
#define MAKEWORD(low,high) \
        ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
#endif

#ifndef WINSOCK_VERSION
#define WINSOCK_VERSION MAKEWORD(2,2)
#endif

/*
 * On XBox, the winsock is in a static library rather than a DLL
 */
#define WINSOCK_API_LINKAGE

/*
 * Establish DLL function linkage if supported by the current build
 * environment and not previously defined.
 */

#ifndef WINSOCK_API_LINKAGE
#ifdef DECLSPEC_IMPORT
#define WINSOCK_API_LINKAGE DECLSPEC_IMPORT
#else
#define WINSOCK_API_LINKAGE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;


/*
 * The new type to be used in all
 * instances which refer to sockets.
 */
typedef UINT_PTR        SOCKET;

/*
 * Select uses arrays of SOCKETs.  These macros manipulate such
 * arrays.  FD_SETSIZE may be defined by the user before including
 * this file, but the default here should be >= 64.
 *
 * CAVEAT IMPLEMENTOR and USER: THESE MACROS AND TYPES MUST BE
 * INCLUDED IN WINSOCK2.H EXACTLY AS SHOWN HERE.
 */
#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif /* FD_SETSIZE */

typedef struct fd_set {
        u_int fd_count;               /* how many are SET? */
        SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
} fd_set;

extern int PASCAL FAR __WSAFDIsSet(SOCKET, fd_set FAR *);

#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#define FD_SET(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == (fd)) { \
            break; \
        } \
    } \
    if (__i == ((fd_set FAR *)(set))->fd_count) { \
        if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) { \
            ((fd_set FAR *)(set))->fd_array[__i] = (fd); \
            ((fd_set FAR *)(set))->fd_count++; \
        } \
    } \
} while(0)

#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)

#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set))

/*
 * Structure used in select() call, taken from the BSD file sys/time.h.
 */
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
        ((tvp)->tv_sec cmp (uvp)->tv_sec || \
         (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 *
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONREAD    _IOR('f', 127, u_long) /* get # bytes to read */
#define FIONBIO     _IOW('f', 126, u_long) /* set/clear non-blocking i/o */




/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, taken from the BSD file netinet/in.h.
 */

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_IGMP            2               /* internet group management protocol */
#define IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_ND              77              /* UNOFFICIAL net disk proto */

#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256

/*
 * Port/socket numbers: network standard functions
 */
#define IPPORT_ECHO             7
#define IPPORT_DISCARD          9
#define IPPORT_SYSTAT           11
#define IPPORT_DAYTIME          13
#define IPPORT_NETSTAT          15
#define IPPORT_FTP              21
#define IPPORT_TELNET           23
#define IPPORT_SMTP             25
#define IPPORT_TIMESERVER       37
#define IPPORT_NAMESERVER       42
#define IPPORT_WHOIS            43
#define IPPORT_MTP              57

/*
 * Port/socket numbers: host specific functions
 */
#define IPPORT_TFTP             69
#define IPPORT_RJE              77
#define IPPORT_FINGER           79
#define IPPORT_TTYLINK          87
#define IPPORT_SUPDUP           95

/*
 * UNIX TCP sockets
 */
#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_CMDSERVER        514
#define IPPORT_EFSSERVER        520

/*
 * UNIX UDP sockets
 */
#define IPPORT_BIFFUDP          512
#define IPPORT_WHOSERVER        513
#define IPPORT_ROUTESERVER      520
                                        /* 520+1 also used */

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 */
#define IPPORT_RESERVED         1024


/*
 * Internet address (old style... should be updated)
 */
struct in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
#define s_addr  S_un.S_addr
                                /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2
                                /* host on imp */
#define s_net   S_un.S_un_b.s_b1
                                /* network */
#define s_imp   S_un.S_un_w.s_w2
                                /* imp */
#define s_impno S_un.S_un_b.s_b4
                                /* imp # */
#define s_lh    S_un.S_un_b.s_b3
                                /* logical host */
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define IN_CLASSD(i)            (((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET           0xf0000000       /* These ones aren't really */
#define IN_CLASSD_NSHIFT        28               /* net and host fields, but */
#define IN_CLASSD_HOST          0x0fffffff       /* routing needn't know.    */
#define IN_MULTICAST(i)         IN_CLASSD(i)

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

#define ADDR_ANY                INADDR_ANY

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef struct WSAData {
        WORD                    wVersion;
        WORD                    wHighVersion;
#ifdef _WIN64
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
#else
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
#endif
} WSADATA, FAR * LPWSADATA;

/*
 * Definitions related to sockets: types, address families, options,
 * taken from the BSD file sys/socket.h.
 */

/*
 * This is used instead of -1, since the
 * SOCKET type is unsigned.
 */
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

/*
 * The  following  may  be used in place of the address family, socket type, or
 * protocol  in  a  call  to WSASocket to indicate that the corresponding value
 * should  be taken from the supplied WSAPROTOCOL_INFO structure instead of the
 * parameter itself.
 */
#define FROM_PROTOCOL_INFO (-1)

/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */

/*
 * Option flags per-socket.
 */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_LINGER       0x0080          /* linger on close if data present */

#define SO_DONTLINGER   (int)(~SO_LINGER)
#define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR)) /* disallow local address reuse */

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_TYPE         0x1008          /* get socket type */


/*
 * TCP options.
 */
#define TCP_NODELAY     0x0001

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
/*
 * Although  AF_UNSPEC  is  defined for backwards compatibility, using
 * AF_UNSPEC for the "af" parameter when creating a socket is STRONGLY
 * DISCOURAGED.    The  interpretation  of  the  "protocol"  parameter
 * depends  on the actual address family chosen.  As environments grow
 * to  include  more  and  more  address families that use overlapping
 * protocol  values  there  is  more  and  more  chance of choosing an
 * undesired address family when AF_UNSPEC is used.
 */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */

#define AF_MAX          29

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
        u_short sa_family;              /* address family */
        char    sa_data[14];            /* up to 14 bytes of direct address */
};


/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_INET         AF_INET

#define PF_MAX          AF_MAX

/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        u_short l_onoff;                /* option on/off */
        u_short l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       0x7fffffff


#define MSG_PARTIAL     0x8000          /* partial send or recv for message xport */




/*
 * WinSock error codes are also defined in winerror.h
 * Hence the IFDEF.
 */
#ifndef WSABASEERR

/*
 * All Windows Sockets error constants are biased by WSABASEERR from
 * the "normal"
 */
#define WSABASEERR              10000

/*
 * Windows Sockets definitions of regular Microsoft C error constants
 */
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

/*
 * Windows Sockets definitions of regular Berkeley error constants
 */
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

/*
 * Extended Windows Sockets error constant definitions
 */
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)
#define WSAEDISCON              (WSABASEERR+101)
#define WSAENOMORE              (WSABASEERR+102)
#define WSAECANCELLED           (WSABASEERR+103)
#define WSAEINVALIDPROCTABLE    (WSABASEERR+104)
#define WSAEINVALIDPROVIDER     (WSABASEERR+105)
#define WSAEPROVIDERFAILEDINIT  (WSABASEERR+106)
#define WSASYSCALLFAILURE       (WSABASEERR+107)
#define WSASERVICE_NOT_FOUND    (WSABASEERR+108)
#define WSATYPE_NOT_FOUND       (WSABASEERR+109)
#define WSA_E_NO_MORE           (WSABASEERR+110)
#define WSA_E_CANCELLED         (WSABASEERR+111)
#define WSAEREFUSED             (WSABASEERR+112)

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver). Note that these errors are
 * retrieved via WSAGetLastError() and must therefore follow
 * the rules for avoiding clashes with error numbers from
 * specific implementations or language run-time systems.
 * For this reason the codes are based at WSABASEERR+1001.
 * Note also that [WSA]NO_ADDRESS is defined only for
 * compatibility purposes.
 */

/* Authoritative Answer: Host not found */
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)

/* Non-Authoritative: Host not found, or SERVERFAIL */
#define WSATRY_AGAIN            (WSABASEERR+1002)

/* Non-recoverable errors, FORMERR, REFUSED, NOTIMP */
#define WSANO_RECOVERY          (WSABASEERR+1003)

/* Valid name, no data record of requested type */
#define WSANO_DATA              (WSABASEERR+1004)


/*
 * WinSock error codes are also defined in winerror.h
 * Hence the IFDEF.
 */
#endif /* ifdef WSABASEERR */

/*
 * Compatibility macros.
 */

#define h_errno         WSAGetLastError()
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define TRY_AGAIN               WSATRY_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
#define NO_DATA                 WSANO_DATA
/* no address, look for MX record */
#define WSANO_ADDRESS           WSANO_DATA
#define NO_ADDRESS              WSANO_ADDRESS



/*
 * Windows Sockets errors redefined as regular Berkeley error constants.
 * These are commented out in Windows NT to avoid conflicts with errno.h.
 * Use the WSA constants instead.
 */
#if 0
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#endif

/*
 * WinSock 2 extension -- new error codes and type definition
 */

#ifdef _WIN32

#define WSAAPI                  FAR PASCAL
#define WSAEVENT                HANDLE
#define LPWSAEVENT              LPHANDLE
#define WSAOVERLAPPED           OVERLAPPED
typedef struct _OVERLAPPED *    LPWSAOVERLAPPED;

#define WSA_IO_PENDING          (ERROR_IO_PENDING)
#define WSA_IO_INCOMPLETE       (ERROR_IO_INCOMPLETE)
#define WSA_INVALID_HANDLE      (ERROR_INVALID_HANDLE)
#define WSA_INVALID_PARAMETER   (ERROR_INVALID_PARAMETER)
#define WSA_NOT_ENOUGH_MEMORY   (ERROR_NOT_ENOUGH_MEMORY)
#define WSA_OPERATION_ABORTED   (ERROR_OPERATION_ABORTED)

#define WSA_INVALID_EVENT       ((WSAEVENT)NULL)
#define WSA_MAXIMUM_WAIT_EVENTS (MAXIMUM_WAIT_OBJECTS)
#define WSA_WAIT_FAILED         (WAIT_FAILED)
#define WSA_WAIT_EVENT_0        (WAIT_OBJECT_0)
#define WSA_WAIT_IO_COMPLETION  (WAIT_IO_COMPLETION)
#define WSA_WAIT_TIMEOUT        (WAIT_TIMEOUT)
#define WSA_INFINITE            (INFINITE)

#else /* WIN16 */

#define WSAAPI                  FAR PASCAL
typedef DWORD                   WSAEVENT, FAR * LPWSAEVENT;

typedef struct _WSAOVERLAPPED {
    DWORD    Internal;
    DWORD    InternalHigh;
    DWORD    Offset;
    DWORD    OffsetHigh;
    WSAEVENT hEvent;
} WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;

#define WSA_IO_PENDING          (WSAEWOULDBLOCK)
#define WSA_IO_INCOMPLETE       (WSAEWOULDBLOCK)
#define WSA_INVALID_HANDLE      (WSAENOTSOCK)
#define WSA_INVALID_PARAMETER   (WSAEINVAL)
#define WSA_NOT_ENOUGH_MEMORY   (WSAENOBUFS)
#define WSA_OPERATION_ABORTED   (WSAEINTR)

#define WSA_INVALID_EVENT       ((WSAEVENT)NULL)
#define WSA_MAXIMUM_WAIT_EVENTS (MAXIMUM_WAIT_OBJECTS)
#define WSA_WAIT_FAILED         ((DWORD)-1L)
#define WSA_WAIT_EVENT_0        ((DWORD)0)
#define WSA_WAIT_TIMEOUT        ((DWORD)0x102L)
#define WSA_INFINITE            ((DWORD)-1L)

#endif  /* WIN32 */

/*
 * WinSock 2 extension -- WSABUF and QOS struct, include qos.h
 * to pull in FLOWSPEC and related definitions
 */

typedef struct _WSABUF {
    u_long      len;     /* the length of the buffer */
    char FAR *  buf;     /* the pointer to the buffer */
} WSABUF, FAR * LPWSABUF;

/*
 * WinSock 2 extension -- manifest constants for shutdown()
 */
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02




/*
 * Microsoft Windows Extended data types required for the functions to
 * convert   back  and  forth  between  binary  and  string  forms  of
 * addresses.
 */
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;


/* Socket function prototypes */

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
accept(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT int FAR * addrlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_ACCEPT)(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT int FAR * addrlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
bind(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_BIND)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
closesocket(
    IN SOCKET s
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_CLOSESOCKET)(
    IN SOCKET s
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
connect(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_CONNECT)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
ioctlsocket(
    IN SOCKET s,
    IN long cmd,
    IN OUT u_long FAR * argp
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_IOCTLSOCKET)(
    IN SOCKET s,
    IN long cmd,
    IN OUT u_long FAR * argp
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getpeername(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETPEERNAME)(
    IN SOCKET s,
    IN struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getsockname(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETSOCKNAME)(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getsockopt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT int FAR * optlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETSOCKOPT)(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT int FAR * optlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_long
WSAAPI
htonl(
    IN u_long hostlong
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_long
(WSAAPI * LPFN_HTONL)(
    IN u_long hostlong
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_short
WSAAPI
htons(
    IN u_short hostshort
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_short
(WSAAPI * LPFN_HTONS)(
    IN u_short hostshort
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
unsigned long
WSAAPI
inet_addr(
    IN const char FAR * cp
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
unsigned long
(WSAAPI * LPFN_INET_ADDR)(
    IN const char FAR * cp
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
listen(
    IN SOCKET s,
    IN int backlog
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_LISTEN)(
    IN SOCKET s,
    IN int backlog
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_long
WSAAPI
ntohl(
    IN u_long netlong
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_long
(WSAAPI * LPFN_NTOHL)(
    IN u_long netlong
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_short
WSAAPI
ntohs(
    IN u_short netshort
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_short
(WSAAPI * LPFN_NTOHS)(
    IN u_short netshort
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
recv(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_RECV)(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
recvfrom(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR * from,
    IN OUT int FAR * fromlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_RECVFROM)(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR * from,
    IN OUT int FAR * fromlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
select(
    IN int nfds,
    IN OUT fd_set FAR * readfds,
    IN OUT fd_set FAR * writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR * timeout
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SELECT)(
    IN int nfds,
    IN OUT fd_set FAR * readfds,
    IN OUT fd_set FAR * writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR * timeout
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
send(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SEND)(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
sendto(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR * to,
    IN int tolen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SENDTO)(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR * to,
    IN int tolen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
setsockopt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SETSOCKOPT)(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
shutdown(
    IN SOCKET s,
    IN int how
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SHUTDOWN)(
    IN SOCKET s,
    IN int how
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
socket(
    IN int af,
    IN int type,
    IN int protocol
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_SOCKET)(
    IN int af,
    IN int type,
    IN int protocol
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

/* Database function prototypes */



/* Microsoft Windows Extension function prototypes */

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAStartup(
    IN WORD wVersionRequested,
    OUT LPWSADATA lpWSAData
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASTARTUP)(
    IN WORD wVersionRequested,
    OUT LPWSADATA lpWSAData
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACleanup(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACLEANUP)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
void
WSAAPI
WSASetLastError(
    IN int iError
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
void
(WSAAPI * LPFN_WSASETLASTERROR)(
    IN int iError
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAGetLastError(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAGETLASTERROR)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


/*
 * WinSock 2 extensions -- data types for the condition function in
 * WSAAccept() and overlapped I/O completion routine.
 */

typedef
void
(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags
    );

/* WinSock 2 API new function prototypes */


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSACloseEvent(
    IN WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSACLOSEEVENT)(
    IN WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
WSAEVENT
WSAAPI
WSACreateEvent(
    void
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
WSAEVENT
(WSAAPI * LPFN_WSACREATEEVENT)(
    void
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAGetOverlappedResult(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    OUT LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAGETOVERLAPPEDRESULT)(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    OUT LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecv(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECV)(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecvFrom(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR * lpFrom,
    IN OUT LPINT lpFromlen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECVFROM)(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR * lpFrom,
    IN OUT LPINT lpFromlen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAResetEvent(
    IN WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSARESETEVENT)(
    IN WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASend(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASEND)(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASendTo(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr FAR * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASENDTO)(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr FAR * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif // INCL_WINSOCK_API_TYPEDEFS

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSASetEvent(
    IN WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSASETEVENT)(
    IN WSAEVENT hEvent
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
DWORD
WSAAPI
WSAWaitForMultipleEvents(
    IN DWORD cEvents,
    IN const WSAEVENT FAR * lphEvents,
    IN BOOL fWaitAll,
    IN DWORD dwTimeout,
    IN BOOL fAlertable
    );
#endif // INCL_WINSOCK_API_PROTOTYPES

#if INCL_WINSOCK_API_TYPEDEFS
typedef
DWORD
(WSAAPI * LPFN_WSAWAITFORMULTIPLEEVENTS)(
    IN DWORD cEvents,
    IN const WSAEVENT FAR * lphEvents,
    IN BOOL fWaitAll,
    IN DWORD dwTimeout,
    IN BOOL fAlertable
    );
#endif // INCL_WINSOCK_API_TYPEDEFS


/* Microsoft Windows Extended data types */
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger FAR *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr FAR *LPIN_ADDR;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set FAR *LPFD_SET;


typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval FAR *LPTIMEVAL;


// XBox Secure Network Library -----------------------------------------------------------

#include <pshpack1.h>

//
// XNetStartup is called to load the XBox Secure Network Library.  It takes an optional
// pointer to a parameter structure.  To initialize the library with the default set
// of parameters, simply pass NULL for this argument.  To initialize the library with
// a different set of parameters, place an XNetStartupParams on the stack, zero it out,
// set the cfgSizeOfStruct to sizeof(XNetStartupParams), set any of the parameters you
// want to configure (leaving the remaining ones zeroed), and pass a pointer to this
// structure to XNetStartup.
//
// By default the XBox Secure Network Library operates in secure mode, which means that
// communication to untrusted hosts (such as a PC) is disallowed.  However, the devkit
// version of the XBox Secure Network Library (xnet.lib and xnetd.lib) allow you to 
// bypass this security by specifying the XNET_STARTUP_BYPASS_SECURITY flag in the 
// cfgFlags parameter.  Here is an example of how to do this:
// 
//      XNetStartupParams xnsp;
//      memset(&xnsp, 0, sizeof(xnsp));
//      xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
//      xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
//      INT err = XNetStartup(&xnsp);
//
// Attempting to bypass security when linking with xnets.lib or xnetsd.lib does not
// return an error, it is simply ignored.  Attempts to send or receive packets from
// untrusted hosts will fail.
//

typedef struct {

    BYTE        cfgSizeOfStruct;

        // Must be set to sizeof(XNetStartupParams).  There is no default.

    BYTE        cfgFlags;

        // One or more of the following flags OR'd together:

        #define XNET_STARTUP_BYPASS_SECURITY            0x01
            // This devkit-only flag tells the XNet stack to allow insecure
            // communication to untrusted hosts (such as a PC).  This flag
            // is silently ignored by the secure versions of the library.

        #define XNET_STARTUP_BYPASS_DHCP                0x02
            // This devkit-only flag tells the XNet stack to skip searching for
            // for a DHCP server and use auto-ip only to acquire an IP address.
            // This will save several seconds when starting up if you know
            // that there is no DHCP server configured.  This flag is silently
            // ignored by the secure versions of the library.
            //
            // If you use XNET_STARTUP_BYPASS_DHCP, you will usually
            // want to specify XNET_STARTUP_ALLOW_AUTOIP as well. Otherwise
            // DHCP will be bypassed, but auto-ip won't be run, and as
            // result no IP address will be chosen.

        #define XNET_STARTUP_ALLOW_AUTOIP                0x04
            // This flag tells the XNet stack to use auto-ip to obtain an
            // IP address if DHCP fails or is bypassed.

        // The default is 0 (no flags specified).

    BYTE        cfgPrivatePoolSizeInPages;

        // Specifies the size of the pre-allocated private memory pool used by
        // XNet for the following situations:
        //
        //      - Responding to ARP/DHCP/ICMP messages
        //      - Responding to certain TCP control messages
        //      - Allocating incoming TCP connection request sockets
        //      - Buffering outgoing data until it is transmitted (UDP) or
        //        until it is acknowledged (TCP)
        //      - Buffering incoming data on a socket that does not have a
        //        sufficiently large overlapped read pending
        //
        // The reason for using a private pool instead of the normal system
        // pool is because we want to have completely deterministic memory 
        // behavior.  That is, all memory allocation occurs only when an API
        // is called.  No system memory allocation happens asynchronously in
        // response to an incoming packet.
        //
        // Note that this parameter is in units of pages (4096 bytes per page). 
        //
        // The default is 12 pages (48K).

    BYTE        cfgEnetReceiveQueueLength;
        
        // The length of the Ethernet receive queue in number of packets.  Each 
        // packet takes 2KB of physically contiguous memory.
        //
        // The default is 8 packets (16K).

    BYTE        cfgIpFragMaxSimultaneous;

        // The maximum number of IP datagrams that can be in the process of reassembly
        // at the same time.
        //
        // The default is 4 packets.

    BYTE        cfgIpFragMaxPacketDiv256;

        // The maximum size of an IP datagram (including header) that can be reassembled.
        // Be careful when setting this parameter to a large value as it opens up 
        // a potential denial-of-service attack by consuming large amounts of memory
        // in the fixed-size private pool.
        //
        // Note that this parameter is in units of 256-bytes each.
        //
        // The default is 8 units (2048 bytes).

    BYTE        cfgSockMaxSockets;

        // The maximum number of sockets that can be opened at once, including those 
        // sockets created as a result of incoming connection requests.  Remember
        // that a TCP socket may not be closed immediately after closesocket is
        // called depending on the linger options in place (by default a TCP socket
        // will linger).
        //
        // The default is 64 sockets.
        
    BYTE        cfgSockDefaultRecvBufsizeInK;

        // The default receive buffer size for a socket, in units of K (1024 bytes).
        //
        // The default is 16 units (16K).

    BYTE        cfgSockDefaultSendBufsizeInK;

        // The default send buffer size for a socket, in units of K (1024 bytes).
        //
        // The default is 16 units (16K).

    BYTE        cfgKeyRegMax;

        // The maximum number of XNKID / XNKEY pairs that can be registered at the 
        // same time by calling XNetRegisterKey.
        //
        // The default is 4 key pair registrations.

    BYTE        cfgSecRegMax;

        // The maximum number of security associations that can be registered at the
        // same time.  Security associations are created for each unique XNADDR / XNKID
        // pair passed to XNetXnAddrToInAddr.  Security associations are also implicitly
        // created for each secure host that establishes an incoming connection
        // with this host on a given registered XNKID.  Note that there will only be
        // one security association between a pair of hosts on a given XNKID no matter
        // how many sockets are actively communicating on that secure connection.
        //
        // The default is 32 security associations.

     BYTE       cfgQosDataLimitDiv4;

        // The maximum amount of Qos data, in units of DWORD (4 bytes), that can be supplied
        // to a call to XNetQosListen or returned in the result set of a call to XNetQosLookup.
        //
        // The default is 64 (256 bytes).

} XNetStartupParams;

typedef struct {
    IN_ADDR     ina;                            // IP address (zero if not static/DHCP)
    IN_ADDR     inaOnline;                      // Online IP address (zero if not online)
    WORD        wPortOnline;                    // Online port
    BYTE        abEnet[6];                      // Ethernet MAC address
    BYTE        abOnline[20];                   // Online identification
} XNADDR;

typedef struct {
    BYTE        ab[8];                          // xbox to xbox key identifier
} XNKID;

typedef XNADDR TSADDR;


#define XNET_XNKID_MASK                 0xF0    // Mask of flag bits in first byte of XNKID
#define XNET_XNKID_SYSTEM_LINK          0x00    // Peer to peer system link session
#define XNET_XNKID_ONLINE_PEER          0x80    // Peer to peer online session
#define XNET_XNKID_ONLINE_SERVER        0xC0    // Client to server online session
#define XNET_XNKID_ONLINE_TITLESERVER   0xE0    // Client to title server online session

#define XNetXnKidIsSystemLink(pxnkid)           (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_SYSTEM_LINK)
#define XNetXnKidIsOnlinePeer(pxnkid)           (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_ONLINE_PEER)
#define XNetXnKidIsOnlineServer(pxnkid)         (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_ONLINE_SERVER)
#define XNetXnKidIsOnlineTitleServer(pxnkid)    (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_ONLINE_TITLESERVER)

typedef struct {
    BYTE        ab[16];                         // xbox to xbox key exchange key
} XNKEY;

typedef struct {
    INT         iStatus;                        // WSAEINPROGRESS if pending; 0 if success; error if failed
    UINT        cina;                           // Count of IP addresses for the given host
    IN_ADDR     aina[8];                        // Vector of IP addresses for the given host
} XNDNS;

typedef struct {
    BYTE        bFlags;                         // See XNET_XNQOSINFO_* below
    BYTE        bReserved;                      // Reserved
    WORD        cProbesXmit;                    // Count of Qos probes transmitted
    WORD        cProbesRecv;                    // Count of Qos probes successfully received
    WORD        cbData;                         // Size of Qos data supplied by target (may be zero)
    BYTE *      pbData;                         // Qos data supplied by target (may be NULL)
    WORD        wRttMinInMsecs;                 // Minimum round-trip time in milliseconds
    WORD        wRttMedInMsecs;                 // Median round-trip time in milliseconds
    DWORD       dwUpBitsPerSec;                 // Upstream bandwidth in bits per second
    DWORD       dwDnBitsPerSec;                 // Downstream bandwidth in bits per second
} XNQOSINFO;

#define XNET_XNQOSINFO_COMPLETE         0x01    // Qos has finished processing this entry
#define XNET_XNQOSINFO_TARGET_CONTACTED 0x02    // Target host was successfully contacted
#define XNET_XNQOSINFO_TARGET_DISABLED  0x04    // Target host has disabled its Qos listener
#define XNET_XNQOSINFO_DATA_RECEIVED    0x08    // Target host supplied Qos data
#define XNET_XNQOSINFO_PARTIAL_COMPLETE 0x10    // Qos has unfinsihed estimates for this entry

typedef struct {
    UINT        cxnqos;                         // Count of items in axnqosinfo[] array
    UINT        cxnqosPending;                  // Count of items still pending
    XNQOSINFO   axnqosinfo[1];                  // Vector of Qos results
} XNQOS;


#include <poppack.h>

INT   WSAAPI XNetStartup(const XNetStartupParams * pxnsp);
INT   WSAAPI XNetCleanup();

INT   WSAAPI XNetRandom(BYTE * pb, UINT cb);

INT   WSAAPI XNetCreateKey(XNKID * pxnkid, XNKEY * pxnkey);
INT   WSAAPI XNetRegisterKey(const XNKID * pxnkid, const XNKEY * pxnkey);
INT   WSAAPI XNetUnregisterKey(const XNKID * pxnkid);

INT   WSAAPI XNetXnAddrToInAddr(const XNADDR * pxna, const XNKID * pxnkid, IN_ADDR * pina);
INT   WSAAPI XNetServerToInAddr(const IN_ADDR ina, DWORD dwServiceId, IN_ADDR * pina);
INT   WSAAPI XNetTsAddrToInAddr(const TSADDR * ptsa, DWORD dwServiceId, const XNKID * pxnkid, IN_ADDR * pina);
INT   WSAAPI XNetInAddrToXnAddr(const IN_ADDR ina, XNADDR * pxna, XNKID * pxnkid);
INT   WSAAPI XNetInAddrToServer(const IN_ADDR ina, IN_ADDR *pina);
INT   WSAAPI XNetInAddrToString(const IN_ADDR ina, char * pchBuf, INT cchBuf);
INT   WSAAPI XNetUnregisterInAddr(const IN_ADDR ina);
INT   WSAAPI XNetXnAddrToMachineId(const XNADDR *pxnaddr, ULONGLONG *pqwMachineId);

INT   WSAAPI XNetConnect(const IN_ADDR ina);
DWORD WSAAPI XNetGetConnectStatus(const IN_ADDR ina);

#define XNET_CONNECT_STATUS_IDLE            0x0000  // Connection not started; use XNetConnect or send packet
#define XNET_CONNECT_STATUS_PENDING         0x0001  // Connecting in progress; not complete yet
#define XNET_CONNECT_STATUS_CONNECTED       0x0002  // Connection is established
#define XNET_CONNECT_STATUS_LOST            0x0003  // Connection was lost

INT   WSAAPI XNetDnsLookup(const char * pszHost, WSAEVENT hEvent, XNDNS ** ppxndns);
INT   WSAAPI XNetDnsRelease(XNDNS * pxndns);

INT   WSAAPI XNetQosListen(const XNKID * pxnkid, const BYTE * pb, UINT cb, DWORD dwBitsPerSec, DWORD dwFlags);
INT   WSAAPI XNetQosLookup(UINT cxna, const XNADDR * apxna[], const XNKID * apxnkid[], const XNKEY * apxnkey[], UINT cina, const IN_ADDR aina[], const DWORD adwServiceId[], UINT cProbes, DWORD dwBitsPerSec, DWORD dwFlags, WSAEVENT hEvent, XNQOS ** ppxnqos);
INT   WSAAPI XNetQosServiceLookup(DWORD dwFlags, WSAEVENT hEvent, XNQOS ** ppxnqos);
INT   WSAAPI XNetQosRelease(XNQOS * pxnqos);

#define XNET_QOS_LISTEN_ENABLE              0x01    // Responds to queries on the given XNKID
#define XNET_QOS_LISTEN_DISABLE             0x02    // Rejects queries on the given XNKID
#define XNET_QOS_LISTEN_SET_DATA            0x04    // Sets the block of data to send back to queriers
#define XNET_QOS_LISTEN_SET_BITSPERSEC      0x08    // Sets max bandwidth that query reponses may consume
#define XNET_QOS_LISTEN_RELEASE             0x10    // Stops listening on given XNKID and releases memory
#define XNET_QOS_LOOKUP_RESERVED            0x00    // No flags defined yet for XNetQosLookup
#define XNET_QOS_SERVICE_LOOKUP_RESERVED    0x00    // No flags defined yet for XNetQosServiceLookup

DWORD WSAAPI XNetGetTitleXnAddr(XNADDR * pxna);
DWORD WSAAPI XNetGetDebugXnAddr(XNADDR * pxna);

#define XNET_GET_XNADDR_PENDING             0x0000  // Address acquisition is not yet complete
#define XNET_GET_XNADDR_NONE                0x0001  // XNet is uninitialized or no debugger found
#define XNET_GET_XNADDR_ETHERNET            0x0002  // Host has ethernet address (no IP address)
#define XNET_GET_XNADDR_STATIC              0x0004  // Host has staticically assigned IP address
#define XNET_GET_XNADDR_DHCP                0x0008  // Host has DHCP assigned IP address
#define XNET_GET_XNADDR_PPPOE               0x0010  // Host has PPPoE assigned IP address
#define XNET_GET_XNADDR_GATEWAY             0x0020  // Host has one or more gateways configured
#define XNET_GET_XNADDR_DNS                 0x0040  // Host has one or more DNS servers configured
#define XNET_GET_XNADDR_ONLINE              0x0080  // Host is currently connected to online service
#define XNET_GET_XNADDR_TROUBLESHOOT        0x8000  // Network configuration requires troubleshooting

DWORD WSAAPI XNetGetEthernetLinkStatus();

#define XNET_ETHERNET_LINK_ACTIVE           0x01    // Ethernet cable is connected and active
#define XNET_ETHERNET_LINK_100MBPS          0x02    // Ethernet link is set to 100 Mbps
#define XNET_ETHERNET_LINK_10MBPS           0x04    // Ethernet link is set to 10 Mbps
#define XNET_ETHERNET_LINK_FULL_DUPLEX      0x08    // Ethernet link is in full duplex mode
#define XNET_ETHERNET_LINK_HALF_DUPLEX      0x10    // Ethernet link is in half duplex mode

DWORD WSAAPI XNetGetBroadcastVersionStatus(BOOL fReset);

#define XNET_BROADCAST_VERSION_OLDER        0x0001  // Got broadcast packet(s) from incompatible older version of title
#define XNET_BROADCAST_VERSION_NEWER        0x0002  // Got broadcast packet(s) from incompatible newer version of title

//
// Since our socket handles are not file handles, apps can NOT call CancelIO API to cancel
// outstanding overlapped I/O requests. Apps must call WSACancelOverlappedIO function instead.
//

INT WSAAPI WSACancelOverlappedIO(SOCKET s);

//
// The following protocol can be passed to the socket() API to create a datagram socket that
// transports encrypted data and unencrypted voice in the same packet.
//

#define IPPROTO_VDP                         254



#ifdef __cplusplus
}
#endif

#ifndef _WIN64
#include <poppack.h>
#endif

#endif  /* _WINSOCK2API_ */

