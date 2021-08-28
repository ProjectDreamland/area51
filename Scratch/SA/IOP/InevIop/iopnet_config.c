#include "ioptypes.h"
#include "iopnet_config.h"
#include "iopnet.h"
#include "netcnf.h"
#include "iopmain.h"
#include "stdio.h"
#include <inet/inetctl.h>

static void
patchEnv(sceNetCnfEnv_t *env)
{
  /*
   *  enable auto-redial for upto 3 phone numbers.
   */
  if (env->root->pair_head->ifc->type  == sceNetCnf_IFC_TYPE_PPP &&
      env->root->pair_head->ifc->pppoe != 1)
  {
    int i, redial_count;
    for(i = redial_count = 0; i < sceNetCnf_MAX_PHONE_NUMBERS; i++) {
      if (env->root->pair_head->ifc->phone_numbers[i] == NULL)
        continue;
      switch(i){
      case 0: redial_count++; break;
      case 1: redial_count++; break;
      case 2: redial_count++; break;
      }
    }
    env->root->pair_head->ifc->redial_count = redial_count - 1;
  }

  /*
   *  allow Ethernet Combinations with one cnf type==ETH and 
   *  another cnf type==NIC to work seamlessly.
   */
  if (env->root->pair_head->ifc->type  != sceNetCnf_IFC_TYPE_PPP &&
      env->root->pair_head->ifc->pppoe != 1)
  {
    env->root->pair_head->ifc->type = env->root->pair_head->dev->type;
  }

}

void NetGetConfig(char *pPath,s32 type,net_config_list *pConfigList)
{
    static s32 ConfigCount,ProviderCount;
    static sceNetCnfList_t *pList;
	static sceNetCnfList_t *pProviderList;
    static s32 i,ret,j;
    static char TempPath[64];
    static char vendor[64],name[64];
	static char* pName;
	static s32 index,config;
	static char* pStr;

	iop_DebugMsg("NetGetConfig: %s, %d\n",pPath,type);

    // We need to dup the path since the out buffer occupies the same
    // space as the input, path, buffer
    strcpy(TempPath,pPath);
	type = 0;
    ConfigCount = sceNetCnfGetCount(pPath,0);
    if (ConfigCount<=0)
    {
        pConfigList->Count = 0;
        return;
    }

    if (ConfigCount > MAX_CONFIG_NAME_COUNT)
    {
        ConfigCount = MAX_CONFIG_NAME_COUNT;
    }

    pList = iop_Malloc(sizeof(sceNetCnfList_t) * ConfigCount);
    ASSERT(pList);
    sceNetCnfGetList(pPath,0,pList);

	ProviderCount = sceNetCnfGetCount(pPath,1);
	pProviderList = iop_Malloc(sizeof(sceNetCnfList_t) * ProviderCount);
	if (!pProviderList)
	{
		iop_Free(pList);
		return;
	}
	sceNetCnfGetList(pPath,1,pProviderList);

    pConfigList->Count = ConfigCount;

    for (i=0;i<ConfigCount;i++)
    {
        memset(&g_Net.m_LoadedEnvironment,0,sizeof(g_Net.m_LoadedEnvironment));
        //g_Net.m_LoadedEnvironment.f_no_decode = TRUE;
        g_Net.m_LoadedEnvironment.mem_base = g_Net.m_EnvironmentWorkspace;
        g_Net.m_LoadedEnvironment.mem_last = g_Net.m_LoadedEnvironment.mem_base + ENV_WORKSPACE_SIZE;
        g_Net.m_LoadedEnvironment.mem_ptr  = g_Net.m_LoadedEnvironment.mem_base;
        ret = sceNetCnfLoadEntry(TempPath,type,pList[i].usr_name,&g_Net.m_LoadedEnvironment);
		patchEnv(&g_Net.m_LoadedEnvironment);
        if (ret==sceNETCNF_MAGIC_ERROR)
        {
            pConfigList->Count = -2;
            break;
        }
        if ( ret<0 )
        {
            pConfigList->Count = -1;
            break;
        }
        // Go through the list of currently available interfaces and see if this loaded entry
        // has an associated interface available
        pConfigList->Available[i]=FALSE;
        for (j=0;j<g_Net.m_InterfaceCount;j++)
        {
            sceInetInterfaceControl(g_Net.m_DeviceIdTable[j],sceInetCC_GetVendorName,vendor,sizeof(vendor));
            sceInetInterfaceControl(g_Net.m_DeviceIdTable[j],sceInetCC_GetDeviceName,name,sizeof(name));

            if ( (strcmp(vendor,g_Net.m_LoadedEnvironment.ifc->vendor)==0) &&
                 (strcmp(name,g_Net.m_LoadedEnvironment.ifc->product)==0) &&
				 (g_Net.m_LoadedEnvironment.file_err == 0) )
            {
                pConfigList->Available[i]=TRUE;
                break;
            }
        }
		// From the usr_name field, extract the combination # only
		pStr = pList[i].usr_name+strlen(pList[i].usr_name)-1;
		index = 1;
		config=0;
		while ( (*pStr >='0') && (*pStr<='9') &&
			    (pStr >= pList[i].usr_name) )
		{
			config += (*pStr-'0')*index;
			index*=10;
			pStr--;
		}
		// Now find the name of the "ifc" in the provider list
		index=0;

		pName = "";
		for (index=0;index<ProviderCount;index++)
		{
			if (strcmp(pProviderList[index].sys_name,g_Net.m_LoadedEnvironment.root->pair_head->attach_ifc)==0)
			{
				pName = pProviderList[index].usr_name;
				break;
			}
		}

        sprintf(pConfigList->Name[i],"%d: %s (%s %s)",config,pName,vendor,name);
        ASSERT(strlen(pConfigList->Name[i]) < MAX_CONFIG_NAME_LENGTH);
    }

    iop_Free(pList);
}

s32 NetActivateConfig(s32 on)
{
    // Before we activate, or deactivate a configuration, we make sure
    // we flush the event queue since the process of doing either will
    // issue some new events.
	iop_DebugMsg("NetActivateConfig: %d\n",on);
    g_Net.m_LastAttachStatus = ATTACH_STATUS_IDLE;

    if (on)
    {
		//sceInetCtlSetAutoMode(1);
        sceInetCtlUpInterface(0);
    }
    else
    {
		//sceInetCtlSetAutoMode(0);
		iop_DebugMsg("NetActivateConfig:: sceInetCtlDownInterface(0)\n");
        sceInetCtlDownInterface(0);
#if 0
		for (i=0;i<g_Net.m_InterfaceCount;i++)
		{
			sceInetAddress address;

			memset(&address,0,sizeof(address);
            sceInetInterfaceControl(g_Net.m_DeviceIdTable[i],sceInetCC_SetAddress,&address,sizeof(address));

		}
#endif
    }
    return 0;
}

s32 NetGetAttachStatus(void)
{
    s32 status;

    status = g_Net.m_NewAttachStatus;
    if (status != -1)
    {
        iop_DebugMsg("Attach status change 0x%08x\n",status);
        g_Net.m_LastAttachStatus = status;
        g_Net.m_NewAttachStatus = -1;
    }
    
    return g_Net.m_LastAttachStatus;
}

s32 NetSetConfig(char *pPath,s32 ConfigIndex)
{
    s32 status,ConfigCount,error;
    sceNetCnfList_t *pList;
    xbool Available;
    s32 j;
    char vendor[64],name[64];

    g_Net.m_InterfaceCount=0;

	status = 0;

    if (ConfigIndex < 0)
    {
		//sceInetCtlSetAutoMode(0);
        sceInetCtlDownInterface(0);
		iop_DebugMsg("NetSetConfig:: sceInetCtlDownInterface(0)\n");
        status = 0;
    }
    else
    {
#if 0
		error = sceInetCtlSetAutoMode(1);
		if (error < 0)
		{
			iop_DebugMsg("NetSetConfig: sceInetCtlSetAutoMode returned an error %d\n",error);
		}
#endif
        ConfigCount = sceNetCnfGetCount(pPath,0);
        if (ConfigCount <= ConfigIndex)
        {
            return -1;
        }

        pList = iop_Malloc(sizeof(sceNetCnfList_t) * ConfigCount);
        if (!pList)
            return -1;
		sceNetCnfGetList(pPath,0,pList);

       
        memset(&g_Net.m_LoadedEnvironment,0,sizeof(g_Net.m_LoadedEnvironment));
        g_Net.m_LoadedEnvironment.mem_base = g_Net.m_EnvironmentWorkspace;
        g_Net.m_LoadedEnvironment.mem_last = g_Net.m_LoadedEnvironment.mem_base + ENV_WORKSPACE_SIZE;
        g_Net.m_LoadedEnvironment.mem_ptr  = g_Net.m_LoadedEnvironment.mem_base;
		iop_DebugMsg("NetSetConfig: Attempting to load configuration %d from %s\n",ConfigIndex,pPath);
        status = sceNetCnfLoadEntry(pPath,0,pList[ConfigIndex].usr_name,&g_Net.m_LoadedEnvironment);
		patchEnv(&g_Net.m_LoadedEnvironment);
        if (status == sceNETCNF_MAGIC_ERROR)
        {
            iop_Free(pList);
            return -2;
        }
        Available = FALSE;
        for (j=0;j<g_Net.m_InterfaceCount;j++)
        {
            sceInetInterfaceControl(g_Net.m_DeviceIdTable[j],sceInetCC_GetVendorName,vendor,sizeof(vendor));
            sceInetInterfaceControl(g_Net.m_DeviceIdTable[j],sceInetCC_GetDeviceName,name,sizeof(name));

            if ( (strcmp(vendor,g_Net.m_LoadedEnvironment.ifc->vendor)==0) &&
                 (strcmp(name,g_Net.m_LoadedEnvironment.ifc->product)==0) &&
				 (g_Net.m_LoadedEnvironment.file_err == 0) )
            {
                Available = TRUE;
				g_Net.m_LastLoadedDeviceId = g_Net.m_DeviceIdTable[j];
                break;
            }
        }

        if (Available)
        {
            error = sceInetCtlSetConfiguration(&g_Net.m_LoadedEnvironment);
			if (error < 0)
			{
				iop_DebugMsg("NetSetConfig: sceInetCtlSetConfiguration returned an error %d\n",error);
			}
#if 0
            error = sceInetCtlUpInterface(0);
			iop_DebugMsg("NetSetConfig:: sceInetCtlUpInterface(0)\n");
			if (error < 0)
			{
				iop_DebugMsg("NetSetConfig: sceInetCtlUpInterface returned an error %d\n",error);
			}
#endif
        }
        else
        {
			iop_DebugMsg("NetSetConfig: Unable to find configuration\n");
            status = -1;
        }
        iop_Free(pList);
    }
    //
    // We want to force an interface list refresh
    //
    g_Net.m_InterfaceCount=0;
    return status;
}
