#ifndef __NETVARS_HPP
#define __NETVARS_HPP
#include "x_types.hpp"
#include "x_threads.hpp"
#include "netdefs.hpp"
//
// This contains all the variables and structures used on the EE side of the audio
// driver. There should be no equivalents on the IOP of the same fields.
// All common structs and defines should be in audiodefs.hpp
//

typedef struct s_monitored_port
{
    struct s_monitored_port *m_pNext;       // Ptr to next monitored port
    u16                     m_Port;         // Port number
    u16                     m_Flags;        // Flags for this port
    u16                     m_Status;       // Status
    u32                     m_Address;      // Interface address this port is bound to
    xmesgq*                 m_pqPending;    // List of pending read requests on this port
} monitored_port;

typedef struct s_net_vars
{
    interface_info  m_Info[MAX_NET_DEVICES];// Information about the current interface
    s32             m_InterfaceCount;
    s32             m_PacketsSent;          // # packets sent
    s32             m_BytesSent;            // # bytes sent
    s32             m_PacketsReceived;      // # packets received
    s32             m_BytesReceived;        // # bytes received
    s32             m_NAddressesBound;      // # ports opened
    xmesgq*         m_pqSendPending;        // Queue for send requests
    xmesgq*         m_pqAvailable;          // Queue for available packet blocks (shared between send/receive)
    xthread*        m_pSendThread;          // id of update thread
    xthread*        m_pRecvThread;          // id of kicker that will force update thread to run
    xbool           m_BindBusy;             // Set if we're trying to bind a port (packets will be dropped when set)
    monitored_port  *m_pPortList;           // List of ports opened
    net_sendrecv_request *m_pRequests;      // Ptr to base memory block allocated for requests
} net_vars;

extern net_vars *g_Net;


#endif // NETVARS_HPP
