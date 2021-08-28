#ifndef __NETDEFS_HPP
#define __NETDEFS_HPP

#include "x_types.hpp"

#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 1024
#endif

#define IOP_BUFFER_SIZE (MAX_PACKET_SIZE + sizeof(net_sendrecv_header))

enum
{
        INEV_NET_DEVICE=0x4afb0000,
        INEV_NET_RECV_DEV,
        INEV_NET_SEND_DEV,
        INEV_NET_SEND_CMD,
        INEV_NET_RECV_CMD,
        INEV_NET_NEWIFC_CMD,
};

// Number of possible packets to send/receive in one go
#define MAX_PACKET_REQUESTS     160
// Number of packets to leave in a receive queue should we run out of packet buffers
#define PACKET_PURGE_THRESHOLD  (MAX_PACKET_REQUESTS - 2)

enum {
        NETCMD_BASE = 0x00020000,
        NETCMD_INIT,
        NETCMD_KILL,
        NETCMD_READCONFIG,              // Returns our own network config information
        NETCMD_BIND,                    // Binds a port
        NETCMD_UNBIND,                  // Unbind a port
        NETCMD_LISTEN,                  // Set listen state
        NETCMD_ACCEPT,                  // Try and accept a connection
        NETCMD_CONNECT,                 // Connect to a specific host
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

typedef struct s_net_init
{
    s32     nChannels;
} net_init;

struct net_sendrecv_header
{
        s32     Type;
        s32     Address;
        s32     Port;
        s32     LocalPort;
        s32     Socket;
        s32     Length;
        s32     Status;
        s32     Dummy[1];      // Force length of struct to 64 bytes
};

struct net_sendrecv_request
{
    net_sendrecv_header Header;
    u8                  Data[MAX_PACKET_SIZE];
};

struct net_connect_status
{
    s32     nRetries;
    s32     TimeoutRemaining;
    s32     TimeoutsRemaining;
    s32     Timeout;
    s32     Status;
    s32     ConnectSpeed;
	char	ErrorText[128];
};


enum
{
    CR_NET_SET_CONFIG,
    CR_NET_ACTIVATE_CONFIG,
};

#define MAX_NET_DEVICES 4
#define MAX_NET_NAME_LENGTH 128

#endif