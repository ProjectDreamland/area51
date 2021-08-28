#include "ioptypes.h"
#include "iopmain.h"
#include "iopnet.h"
#include "iopthreadpri.h"
#include "string.h"

#include <inet/inet.h>
#include <inet/inetctl.h>
#include <inet/netdev.h>
#include <inet/pppctl.h>

#define scePPPCC_GetAuthMessage _PPPCC(_PPPCC_READ,67)
//-----------------------------------------------------------------------------
net_connect_status *NetConnectStatus(void)
{

	char module_name[64];

	memset(&g_Net.m_ConnectStatus,0,sizeof(g_Net.m_ConnectStatus));

	sceInetInterfaceControl(g_Net.m_LastLoadedDeviceId, 
                              sceInetCC_GetModuleName,
                              module_name, 
                              sizeof(module_name));
	if (strncmp(module_name,"ppp",3)!=0)
	{
		return &g_Net.m_ConnectStatus;
	}

	//
	// Fill in status information for PPPoE devices here!
	//
	// Get the extended error string
	g_Net.m_ConnectStatus.ErrorText[0]=0x0;

	sceInetInterfaceControl(g_Net.m_LastLoadedDeviceId,
							scePPPCC_GetAuthMessage,
							g_Net.m_ConnectStatus.ErrorText,
							sizeof(g_Net.m_ConnectStatus.ErrorText));
	iop_DebugMsg("Error string %s\n",g_Net.m_ConnectStatus.ErrorText);

	if (g_Net.m_ConnectStatus.ErrorText[0]==0)
	{
		sceInetInterfaceControl(g_Net.m_LastLoadedDeviceId,
								scePPPCC_GetCurrentStatus,
								g_Net.m_ConnectStatus.ErrorText,
								sizeof(g_Net.m_ConnectStatus.ErrorText));
		iop_DebugMsg("Error string %s\n",g_Net.m_ConnectStatus.ErrorText);

		if ( (strcmp("AUTH Failed",g_Net.m_ConnectStatus.ErrorText)==0) ||
			 (strcmp("DISCONNECT Auth",g_Net.m_ConnectStatus.ErrorText)==0) )
		{
			strcpy(g_Net.m_ConnectStatus.ErrorText,"Authentication failed");
		}
		else if (strncmp("ABORT",g_Net.m_ConnectStatus.ErrorText,5)==0)
		{
			strcpy(g_Net.m_ConnectStatus.ErrorText,&g_Net.m_ConnectStatus.ErrorText[5]);
		}
		else
		{
			strcpy(g_Net.m_ConnectStatus.ErrorText,"Connect failed");
		}

	}

	return &g_Net.m_ConnectStatus;
}
