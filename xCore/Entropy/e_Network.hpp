//==============================================================================
//
//  E_NETWORK.HPP
//
//==============================================================================

#ifndef E_NETWORK_HPP
#define E_NETWORK_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_time.hpp"
#include "Network/netaddress.hpp"
#include "Network/netsocket.hpp"
#include "Network/netdefines.hpp"

extern internet_settings    ISettings;
extern net_stats            g_NetStats;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void    net_Init            ( void );
void    net_Kill            ( void );
xbool   net_IsInited        ( void );
                            
void    net_ClearStats      ( void );
void    net_GetStats        ( s64& PacketsSent,
                              s64& BytesSent, 
                              s64& PacketsReceived,
                              s64& BytesReceived,
                              s64& NAddressesBound,
                              f32& SendTime,
                              f32& ReceiveTime);
                            
void    net_GetInterfaceInfo( s32 ID, interface_info& Info );
void    net_ResolveIP       ( s32 IP, char* pStr );
s32     net_ResolveName     ( const char* pStr);
void    net_GetConnectStatus (connect_status &Status);
void    net_ActivateConfig  (xbool on);
s32     net_GetConfigList   (const char *pPath,net_config_list *pConfigList);
s32     net_SetConfiguration(const char *pPath,s32 configindex);
s32     net_GetAttachStatus (s32 &InterfaceId);
void    net_Encrypt         (byte *pBuffer,s32 &Length,s32 Seed);
xbool   net_Decrypt         (byte *pBuffer,s32 &Length,s32 Seed);
s32		net_GetSystemId		(void);
s32     net_GetVersionKey   (void);
void    net_SetVersionKey   (s32 Version);

void    net_GetHistory      (s32& SentPerSec, s32& ReceivedPerSec);
void    net_UpdateHistory   (s32 Sent,s32 Received);
void    net_BeginConfig     ( void );
void    net_EndConfig       ( void );


void    sys_net_Init     ( void );
void    sys_net_Kill     ( void );
//==============================================================================
#endif // E_NETWORK_HPP
//==============================================================================
