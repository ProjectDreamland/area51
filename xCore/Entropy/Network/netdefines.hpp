//==============================================================================
//
//  NETDEFINES.HPP
//
//==============================================================================

#ifndef NETDEFINES_HPP
#define NETDEFINES_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_time.hpp"
#include "x_plus.hpp"

#include "Network/netsocket.hpp"
#include "Network/netaddress.hpp"

//==============================================================================
//  DEFINITIONS
//==============================================================================

#define STARTING_PORT                       (8000)

#ifndef BAD_SOCKET
#define BAD_SOCKET                          (s32)(-1)
#endif

#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE                     (1024)
#endif

#define NET_FLAGS_BLOCKING                  (1<<0)
#define NET_FLAGS_TCP                       (1<<1)
#define NET_FLAGS_LISTEN                    (1<<2)
#define NET_FLAGS_BROADCAST                 (1<<3)
#define NET_FLAGS_CLOSED                    (1<<4)
#define NET_FLAGS_VDP                       (1<<5)

#define NET_HISTORY_SIZE                    10
#define NET_HISTORY_INTERVAL                1000

#define MAX_CONFIG_NAME_COUNT               8
#define MAX_CONFIG_NAME_LENGTH              128

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

enum net_error
{
        NET_ERR_OK = 0,
        NET_ERR_OTHER_SYSTEM = -128,
        NET_ERR_NO_HARDWARE,
        NET_ERR_NO_CONFIGURATION,
        NET_ERR_CARD_NOT_FORMATTED,
        NET_ERR_INVALID_CARD,
        NET_ERR_NO_CARD,
};

//==============================================================================
//  TYPES
//==============================================================================
struct interface_info
{
        s32         Address;
        s32         Netmask;
        s32         Broadcast;
        s32         Nameserver;
        xbool       IsAvailable;
        xbool       NeedsServicing;
};


struct net_config_list
{
    s32     Count;
    s8      Available[MAX_CONFIG_NAME_COUNT];
    char    Name[MAX_CONFIG_NAME_COUNT][MAX_CONFIG_NAME_LENGTH];
};

struct connect_status
{
        s32		        nRetries;                       // Number of retries attempted so far
        s32		        TimeoutRemaining;               // Amount of timeout remaining in seconds
        s32		        Status;                         // Various flags showing status of connection
        s32		        ConnectSpeed;                   // Speed at which we connected
	    char	        ErrorText[128];					// Error message
};


struct internet_settings
{
    s32             LatencyMs;
    s32             PercSendsLost;
    s32             PercReceivesLost;
    xbool           BlockSends;
    xbool           BlockReceives;
    xtimer          ReceiveDelay;
    xtimer          SendDelay;
    s32             PercPacketsDamaged;
    s32             PercPacketsSwapped;
    byte*           pPacketSwapBuffer;
    net_socket      PacketSwapCaller;
    net_address     PacketSwapDest;
    s32             PacketSwapSize;
    random          Random;
};

struct net_stats
{
    xtimer          ReceiveTime;
    xtimer          SendTime;
    u64             BytesReceived;
    u64             PacketsReceived;
    u64             BytesSent;
    u64             PacketsSent;
    u32             AddressesBound;
};

//==============================================================================
#endif // NETDEFINES_HPP
//==============================================================================
