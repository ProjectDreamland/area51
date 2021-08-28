#ifndef __NET_COMMON_H
#define __NET_COMMON_H

#ifdef TARGET_PS2_IOP
#include "ioptypes.h"
#else
#include "x_types.hpp"
#endif

#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 1024
#endif

enum
{
    INEV_NET_RECV_DEV=0x4afb0000,
    INEV_NET_SEND_DEV,
    INEV_NET_SEND_CMD,
    INEV_NET_RECV_CMD,
    INEV_NET_NEWIFC_CMD,
};

// Number of possible packets to send/receive in one go
#define MAX_PACKET_REQUESTS     80
// Number of packets to leave in a receive queue should we run out of packet buffers
#define PACKET_PURGE_THRESHOLD  (MAX_PACKET_REQUESTS - 16)

enum {
    NETCMD_BASE = 0x00020000,
    NETCMD_INIT,
    NETCMD_KILL,
    NETCMD_READCONFIG,              // Returns our own network config information
    NETCMD_BIND,                    // Binds a port
    NETCMD_UNBIND,                  // Unbind a port
    NETCMD_SEND,                    // Send data (header only)
    NETCMD_RESOLVENAME,             // Resolve a string to IP
    NETCMD_RESOLVEIP,               // Resolve an IP to string
    NETCMD_GETCONNECTSTATUS,        // Get dialup status information (retries, connected etc)
    NETCMD_DISCONNECT,              // Bring an interface down
    NETCMD_GETCONFIGLIST,           // List of valid, bindable configurations
    NETCMD_SETCONFIGURATION,        // Set active bindable configuration (-1 to disable current)
    NETCMD_ACTIVATECONFIG,          // Activate or deactivate current configuration
    NETCMD_GET_ATTACH_STATUS,       // Return status of configuration request pending
    NETCMD_GET_GETCONFIG_STATUS,
    NETCMD_GET_SETCONFIG_STATUS,
    NETCMD_GET_ACTIVATE_CONFIG_STATUS,
	NETCMD_GET_SYSID,
};

#ifdef TARGET_PS2_IOP
enum {
        CONNECT_STATUS_IDLE = 0,
        CONNECT_STATUS_NOTPRESENT,
        CONNECT_STATUS_CONNECTED,
        CONNECT_STATUS_DISCONNECTED,
        CONNECT_STATUS_DIALING,
        CONNECT_STATUS_NEGOTIATING,
        CONNECT_STATUS_AUTHENTICATING,
        CONNECT_STATUS_RETRYING,
        CONNECT_STATUS_NODIALTONE,
        CONNECT_STATUS_TIMEDOUT,
};

enum {
    ATTACH_STATUS_IDLE  =0x0,       // This is not really a valid status but just used during init phase
    ATTACH_STATUS_ATTACHED,         // Interface has attached
    ATTACH_STATUS_DETACHED,         // Interface has detached
    ATTACH_STATUS_STARTED,          // Interface is started
    ATTACH_STATUS_STOPPED,          // Interface is stopped
    ATTACH_STATUS_ERROR,            // A general error occurred
    ATTACH_STATUS_NOCONFIG,         // No configuration available for this interface
    ATTACH_STATUS_CONFIGURED,       // Interface was successfully configured
    ATTACH_STATUS_UNKNOWN,          // An undefined event code was encountered
};

#endif


typedef struct s_net_init
{
    s32     nChannels;
} net_init;

typedef struct s_net_sendrecv_header
{
    s32     Type;
    u32     Address;
    u32     Port;
    u32     LocalPort;
    u32     Socket;
    s32     Length;
    s32     Status;
    s32     Dummy[1];      // Force length of struct to 64 bytes
} net_sendrecv_header;

typedef struct s_net_sendrecv_request
{
    net_sendrecv_header m_Header;
    u8                  m_Data[MAX_PACKET_SIZE];
} net_sendrecv_request;

typedef struct s_net_connect_status
{
    s32     nRetries;
    s32     TimeoutRemaining;
    s32     TimeoutsRemaining;
    s32     Timeout;
    s32     Status;
    s32     ConnectSpeed;
	char	ErrorText[128];
} net_connect_status;

#define MAX_NET_DEVICES 4
#define MAX_NAME_LENGTH 128

#endif