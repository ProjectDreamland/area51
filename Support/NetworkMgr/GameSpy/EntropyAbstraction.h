//=========================================================================
//
//  EntropyAbstraction.hpp
//
//=========================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//=========================================================================
//
//  Abstraction of inevitable Entropy networking code to GameSpy
//
//=========================================================================
#if !defined(ENTROPY_ABSTRACTION_H)
#define ENTROPY_ABSTRACTION_H

#include "x_types.hpp"

#ifdef TARGET_PS2

#define ENTROPY_NETWORK
#undef  _WIN32
#define _PS2

struct sockaddr_in
{
    s32 sin_family;
    u32 sin_port;
    struct
    {
        u32 s_addr;
    } sin_addr;
};

struct sockaddr
{
    s32 s_family;
    u32 s_port;
    u32 s_addr;
};

struct hostent
{
    char*   h_name;
    char**  h_aliases;
    u16     h_addrtype;
    u16     h_length;
    char**  h_addr_list;
};

struct in_addr
{
    u32 s_addr;
};


typedef unsigned char u_char;
typedef unsigned short u_short;

typedef s32 SOCKET;

#if defined(__cplusplus)
extern "C"
{
#endif
s32             abstract_SendTo         ( SOCKET s, const char* pBuffer, s32 Length, s32 Flags, struct sockaddr* To, s32 ToLength);
SOCKET          abstract_Socket         ( s32 af, s32 type, s32 protocol );
void            abstract_CloseSocket    ( SOCKET s );
s32             abstract_RecvFrom       ( SOCKET s, char* pBuffer, s32 Length, s32 Flags, struct sockaddr* From, s32* FromLength);
void            abstract_Sleep          ( s32 msec );
s32             abstract_SetBlocking    ( SOCKET s, xbool IsBlocking );
struct hostent* abstract_GetLocalHost   ( void );
u32             abstract_Resolve        ( const char* pHostname );
char*           abstract_ResolveIP      ( struct in_addr in );
struct hostent* abstract_GetHostByName  ( const char* pHostname );
s32             abstract_Bind           ( SOCKET s, const struct sockaddr* addr, s32 namelen );
s32             abstract_GetSockName    ( SOCKET s, struct sockaddr* name, s32* length );
s32             abstract_Recv           ( SOCKET s, char* pBuffer, s32 Length, s32 Flags );
s32             abstract_Send           ( SOCKET s, const char* pBuffer, s32 Length, s32 Flags );
s32             abstract_Connect        ( SOCKET s, const struct sockaddr* addr, s32 namelen );
s32             abstract_Listen         ( SOCKET s, s32 MaxConnect );
s32             abstract_Accept         ( SOCKET s, struct sockaddr* addr, s32 namelen );
s32             abstract_Shutdown       ( SOCKET s, s32 How );
xbool           abstract_CanReceive     ( SOCKET s );
xbool           abstract_CanSend        ( SOCKET s );
u32             abstract_GetTime        ( void );
void            abstract_Assert         ( const char* pMessage, const char* pFile, s32 Line );
u16             htons                   (u16 n);
u32             htonl                   (u32 n);
u16             ntohs                   (u16 n);
u32             ntohl                   (u32 n);
s32             GOAGetLastError         ( SOCKET s );
#if defined(__cplusplus)
}
#endif
#define AF_INET     (0)
#define SOCK_DGRAM  (0)
#define SOCK_STREAM (1)
#define IPPROTO_UDP (0)
#define IPPROTO_TCP (1)
#define INADDR_ANY  (0)

#define EWOULDBLOCK  (-1)
#define ECONNRESET   (-2)
#define EINPROGRESS  (-3)
#define ENOTCONN     (-4)
#define ETIMEDOUT    (-5)

#define sendto          abstract_SendTo
#define socket          abstract_Socket
#define closesocket     abstract_CloseSocket
#define recvfrom        abstract_RecvFrom
#define inet_addr       abstract_Resolve
#define inet_ntoa       abstract_ResolveIP
#define gethostbyname   abstract_GetHostByName
#define bind            abstract_Bind
#define getsockname     abstract_GetSockName
#define connect         abstract_Connect
#define send            abstract_Send
#define recv            abstract_Recv
#define shutdown        abstract_Shutdown
#define listen          abstract_Listen
#define accept          abstract_Accept

#if defined(assert)
#undef assert
#if defined(X_ASSERT)
#define assert(x) if( !(x) ) abstract_Assert( #x, __FILE__, __LINE__ );
#else
#define assert(x)
#endif
#endif

#define sizeof(x)       (s32)sizeof(x)

#endif // TARGET_PS2

#endif // ENTROPY_ABSTRACTION_H
