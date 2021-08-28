#include "x_files.hpp"
#include "channelmanager.hpp"

//bphold:***#include "sifrpc.h"
//bphold:***#include "eekernel.h"


xbox_au_channel_manager g_ChannelManager;

//bphold:***sceSifClientData s_ChanClientData				PS2_ALIGNMENT(64);
//bphold:***union
//bphold:***{
//bphold:***	iop_channel	Channels[MAX_CHANNELS];
//bphold:***	u32		Longs[64];
//bphold:***} s_ChanBuffer PS2_ALIGNMENT(64);
//bphold:***
//bphold:***static s32 CallRpc(s32 Function)
//bphold:***{
//bphold:***	FlushCache(0);
//bphold:***    sceSifCallRpc (&s_ChanClientData,  
//bphold:***                    Function,0,
//bphold:***                    &s_ChanBuffer,sizeof(s_ChanBuffer),
//bphold:***                    &s_ChanBuffer,sizeof(s_ChanBuffer),
//bphold:***                    NULL,NULL);
//bphold:***	return s_ChanBuffer.Longs[0];
//bphold:***}

//-----------------------------------------------------------------------------
// This init call is done during initial startup of the iop manager. All it is
// supposed to do is initialize the rpc path. This will kick off a series of
// operations on the iop side
void xbox_au_channel_manager::Init(void)
{
//bphold:***	s32 result;
//bphold:***	xtimer t;
//bphold:***
//bphold:***	x_memset(m_Channels,0,sizeof(m_Channels));
//bphold:***	t.Start();
//bphold:***    while (1) 
//bphold:***    {
//bphold:***	    result = sceSifBindRpc (&s_ChanClientData, CHANNEL_RPC_DEV, 0);
//bphold:***        ASSERTS(result>=0,"error: sceSifBindRpc failed");
//bphold:***
//bphold:***	    if (s_ChanClientData.serve != 0) break;
//bphold:***
//bphold:***		if (t.ReadSec() > 2.0f)
//bphold:***		{
//bphold:***			ASSERTS(FALSE,"xbox_au_channel_manager::Init() - RPC call took too long, check init order\n");
//bphold:***		}
//bphold:***    }
}

//-----------------------------------------------------------------------------
// This init call actually sets up the channel manager. It is up to the application
// to specify how much ee side memory is to be donated to the iop audio layer.
void xbox_au_channel_manager::Init(s32 Length)
{
//bphold:***	s32 i;
//bphold:***	// Do the RPC call to initialize the channel manager with the size
//bphold:***	// of the amount of memory to use.
//bphold:***
//bphold:***	ASSERT(Length);
//bphold:***
//bphold:***	// We first bind the sifrpc call then
//bphold:***	// we send an init request
//bphold:***	m_LocalData = x_malloc(Length);
//bphold:***	ASSERT(m_LocalData);
//bphold:***	s_ChanBuffer.Longs[0]=(u32)m_LocalData;
//bphold:***	s_ChanBuffer.Longs[1]=Length;
//bphold:***	CallRpc(CHANCMD_INIT);
//bphold:***	for (i=0;i<MAX_CHANNELS;i++)
//bphold:***	{
//bphold:***		SetState(i,CHANSTAT_IDLE);
//bphold:***	}
}

//-----------------------------------------------------------------------------
// Make sure the channel manager shuts down gracefully.
void xbox_au_channel_manager::Kill(void)
{
//bphold:***	CallRpc(CHANCMD_KILL);
}

//-----------------------------------------------------------------------------
void xbox_au_channel_manager::Update(void)
{
//bphold:***	s32				i;
//bphold:***	iop_channel*	pData;
//bphold:***	iop_channel_state*	pChannel;
//bphold:***
//bphold:***	// Copy internal channel information to the update buffer then send the
//bphold:***	// update buffer to the iop. This is a synchronous call! Beware! Should
//bphold:***	// not take long anyway. We could make it asynchronous later.
//bphold:***	pData = s_ChanBuffer.Channels;
//bphold:***	pChannel = m_Channels;
//bphold:***	for (i=0;i<MAX_CHANNELS;i++)
//bphold:***	{
//bphold:***		pData->m_Pitch			= (s32)(pChannel->m_Pitch * 4096.0f);
//bphold:***		pData->m_LeftVolume		= (s32)((1.0f - pChannel->m_Pan)/2 * pChannel->m_Volume * 16383.0f);
//bphold:***		pData->m_RightVolume	= (s32)((pChannel->m_Pan+1.0f)/2 * pChannel->m_Volume * 16383.0f);
//bphold:***		pData->m_Priority		= pChannel->m_Priority;
//bphold:***		pData->m_Status			= pChannel->m_Status;
//bphold:***
//bphold:***		pData->m_pSample		= pChannel->m_pSample;
//bphold:***		pData->m_SampleRate		= pChannel->m_Rate;
//bphold:***		pData->m_Length			= pChannel->m_Length;
//bphold:***		pData++;
//bphold:***		pChannel++;
//bphold:***	}
//bphold:***	CallRpc(CHANCMD_UPDATE);
//bphold:***	// We do just need to copy status information but, for now, let's just copy the whole lot
//bphold:***	// In theory, this copy isn't even necessary but this layout is going to be preserved
//bphold:***	// since it will be needed when the async version of the update is coded
//bphold:***
//bphold:***	pData = s_ChanBuffer.Channels;
//bphold:***	pChannel = m_Channels;
//bphold:***	for (i=0;i<MAX_CHANNELS;i++)
//bphold:***	{
//bphold:***		pChannel->m_Status		= (iop_channel_status)pData->m_Status;
//bphold:***		pChannel->m_PlayPosition= pData->m_Length;
//bphold:***		pData++;
//bphold:***		pChannel++;
//bphold:***	}
}

