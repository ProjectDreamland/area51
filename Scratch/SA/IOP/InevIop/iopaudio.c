#include "ioptypes.h"
#include "iopmain.h"
#include "iopaudio.h"
#include "iopaud_spu.h"
#include "iopaud_container.h"
#include "iopaud_cfx.h"
#include "iopaud_host.h"
#include "iopaud_hoststream.h"
#include "iopthreadpri.h"

#include "thread.h"
#ifdef X_DEBUG
//#define AUDIO_DEBUG
#endif
iop_audio_vars  g_Audio;
#define INITIAL_SURROUND_DELAY  1   // Number of update cycles before we kick in the surround channel

/*
#define SD_REV_MODE_OFF         0x80
#define SD_REV_MODE_ROOM        0x26c0
#define SD_REV_MODE_STUDIO_A    0x1f40
#define SD_REV_MODE_STUDIO_B    0x4840
#define SD_REV_MODE_STUDIO_C    0x6fe0
#define SD_REV_MODE_HALL Hall   0xade0
#define SD_REV_MODE_SPACE       0xf6c0
#define SD_REV_MODE_ECHO        0x18040
#define SD_REV_MODE_DELAY       0x18040
#define SD_REV_MODE_PIPE        0x3c00
*/

void audio_PeriodicUpdate(void);

const char *g_AudioLockOwner;
//-----------------------------------------------------------------------------
void audio_Lock(const char *locker)
{
    iop_EnterMutex(g_Audio.m_Mutex);
    g_AudioLockOwner = locker;
}

//-----------------------------------------------------------------------------
void audio_Unlock(void)
{
    g_AudioLockOwner=NULL;
    iop_ExitMutex(g_Audio.m_Mutex);
}


xbool audio_IsLocked(void)
{
	return (g_AudioLockOwner != NULL);
}

//-----------------------------------------------------------------------------
void    audio_DupState(cfx_state *pDest,cfx_state *pParent,cfx_state *pOurs)
{
    ASSERT(pDest);
    ASSERT(pParent);
    ASSERT(pOurs);

    pDest->m_Volume     = (pParent->m_Volume  * pOurs->m_Volume ) / AUD_FIXED_POINT_1;
    pDest->m_Pitch      = (pParent->m_Pitch   * pOurs->m_Pitch  ) / AUD_FIXED_POINT_1;
    pDest->m_RearVol    = (pParent->m_RearVol * pOurs->m_RearVol) / AUD_FIXED_POINT_1;
    pDest->m_Pan        = pParent->m_Pan;
}

//-----------------------------------------------------------------------------
void *audio_Init(audio_init *pInit)
{

    s32 memfree;

    memfree = iop_MemFree();
    iop_DebugMsg("Audio Init, %d bytes free before\n",iop_MemFree());
    memset(&g_Audio,0,sizeof(g_Audio));

    g_Audio.m_ChannelCount    = pInit->m_nChannels;
    g_Audio.m_TimerTick       = 0;
    g_Audio.m_Mutex           = iop_CreateMutex();
    g_Audio.m_UpdateThreadId  = iop_CreateThread(audio_PeriodicUpdate,AUD_UPDATE_PRIORITY,3072,"audio_PeriodicUpdate");

    //
    // We need to keep the initialization order from the bottom up
    //
    spu_Init();
    voice_Init();
    voicestream_Init(AUD_MAX_STREAMS,AUD_STREAM_BUFFER_SIZE);
    cfx_Init();
    container_Init();

    g_Audio.m_MasterState.m_Keyoff0 = 0;
    g_Audio.m_MasterState.m_Keyoff1 = 0;
    g_Audio.m_MasterState.m_Keyon0  = 0;
    g_Audio.m_MasterState.m_Keyon1  = 0;
    g_Audio.m_MasterState.m_State.m_LeftVolume = 0;
    g_Audio.m_MasterState.m_State.m_RightVolume = 0;
    iop_DebugMsg("audio_Init: %d bytes free after init, %d bytes used.\n",iop_MemFree(),memfree-iop_MemFree());
    audio_Unlock();
    return NULL;
}

void audio_Kill(void)
{
    audio_Lock("audio_Kill");
    container_Kill();
    cfx_Kill();
    voicestream_Kill();
    voice_Kill();
    spu_Kill();

    iop_DestroyThread(g_Audio.m_UpdateThreadId);
    iop_DestroyMutex(g_Audio.m_Mutex);
}

s32 audio_time,audio_max;
xbool DumpAudioList = FALSE;
//-----------------------------------------------------------------------------
void audio_PeriodicUpdate(void)
{
    s32 LastTime;
    s32 TimeNow;
    s32 DeltaTime;
    cfx *pThisCfx,*pNextCfx,*pFirstCfx;
    s32 status;
    s32 ProcessingTime;
    cfx_state State;
    s32 NumProcessed;

    audio_time = 0;
    audio_max  = 0;
    LastTime = iop_GetTime();
    while (1)
    {
        audio_Lock("Periodic Update");
        TimeNow = iop_GetTime();

        DeltaTime = TimeNow-LastTime;
        if (DeltaTime < 0)
        {
            BREAK;
        }

#ifdef AUDIO_DEBUG
        {
            static s32 i=1;
            static s32 tick=0;
            static s32 min,max;
            static s32 ProcessingAvg;

            i--;

            if (DeltaTime > max)
                max = DeltaTime;
            if (DeltaTime < min)
                min = DeltaTime;

            ProcessingAvg+=ProcessingTime;
            if (i<=0)
            {
                printf("[%d] Min=%d,max=%d,current=%d,processing=%d\n",tick,min,max,DeltaTime,ProcessingAvg/100);
                tick++;
                i=100;
                min = 10000;
                max = 0;
                ProcessingAvg=0;
            }
        }
#endif

        spu_ReadMaster(&g_Audio.m_CurrentMasterState);
        pThisCfx = g_Audio.m_pRootCfx;
        NumProcessed = 0;
        g_Audio.m_VoicesToKill = 0;
        g_Audio.m_LowestKillPriority = 256;
        if (DumpAudioList)
        {
            iop_DebugMsg("Audio List dump for tick %d\n",g_Audio.m_TimerTick);
        }
		pFirstCfx = pThisCfx;

        while (pThisCfx)
        {
            NumProcessed++;
            State = pThisCfx->m_State;

            status = cfx_Update(pThisCfx,&State,DeltaTime/10);
            // If the cfx expired on this update, then we need to unlink it from the
            // master list
            if (status)
            {
                cfx_RemoveFromUpdate(pThisCfx);
                pNextCfx = pThisCfx->m_pNext;
                cfx_Free(pThisCfx);
            }
            else
            {
                if (DumpAudioList)
                {
                    iop_DebugMsg("IOP Audio: id=0x%x,cfxid=%d, flags=%04x, count=%d\n",pThisCfx->m_Identifier,
                                                        pThisCfx->m_State.m_CfxId,
                                                        pThisCfx->m_Flags,
                                                        pThisCfx->m_Count);
                }
                pNextCfx = pThisCfx->m_pNext;
            }
            pThisCfx = pNextCfx;
			ASSERT(pThisCfx != pFirstCfx);
        }

        {
            static s32 i=10;
            static s32 max=0,min=10;

            i--;
            if (NumProcessed < min) min=NumProcessed;
            if (NumProcessed > max) max=NumProcessed;
            if (i<=0)
            {
                s32 memfree;
                i=100;
                memfree = QueryTotalFreeMemSize() - (6 *1048576);
                //printf("AudioMgr: Processs min=%d,max=%d; %d bytes free\n",min,max,memfree);
                min=1000;
                max=0;
            }
        }

        if (g_Audio.m_VoicesToKill)
        {
            voice_Cull(g_Audio.m_VoicesToKill);
        }
        g_Audio.m_TimerTick += DeltaTime;
        //
        // Now calculate the time for the next audio update taking in to
        // account any slew caused by this audio update cycle.
        //
        ProcessingTime = iop_GetTime() - TimeNow;
        audio_time = ProcessingTime;
        if (audio_time > audio_max)
        {
            audio_max = audio_time;
        }
        DeltaTime = 200 - (iop_GetTime()-LastTime);
        if ( (DeltaTime <=0) || (DeltaTime > 110) )
        {
            DeltaTime = 100;
        }

        spu_WriteMaster(&g_Audio.m_MasterState);

        LastTime = TimeNow;
        DumpAudioList = FALSE;
        audio_Unlock();
        DelayThread(DeltaTime*100);                 // Delay by 10ms
    }
}


//-----------------------------------------------------------------------------
void audio_ReadBuffer(void *pData,s32 length)
{
    memcpy(pData,g_Audio.m_pInBuffer,length);
    g_Audio.m_pInBuffer+=length;

}

//-----------------------------------------------------------------------------
void audio_WriteBuffer(void *pData,s32 length)
{
    //
    // Check for overflow and just wrap if we do.
    //
    memcpy(g_Audio.m_pOutBuffer,pData,length);
    g_Audio.m_pOutBuffer += length;
}

//-----------------------------------------------------------------------------
void audio_InitBuffer(byte *pBuffer)
{
    g_Audio.m_pInBuffer = pBuffer;
    g_Audio.m_pOutBuffer = pBuffer;
}
//-----------------------------------------------------------------------------
void audio_Update(u8 *pBuffer)
{
    s8 Command;
    cfx *pCfx;
    s32 done;
    s32 volume;

    cfx_state UpdatePacket;

    audio_InitBuffer(pBuffer);

    done = FALSE;
    while (!done)
    {
        audio_ReadBuffer(&Command,sizeof(s8));

        switch (Command)
        {
        case AUD_COMMAND_UPDATE_CFX:
            audio_ReadBuffer(&UpdatePacket,sizeof(cfx_state));
            if (UpdatePacket.m_CfxId != -1)
            {
                volume = container_GetVolume(UpdatePacket.m_CfxId & 0xff000000);
                UpdatePacket.m_Volume = (UpdatePacket.m_Volume * volume) / AUD_FIXED_POINT_1;
            }

            pCfx = cfx_Find(UpdatePacket.m_Id);
            if (!pCfx)
            {
				if (UpdatePacket.m_UniqueId > g_Audio.m_TopUniqueId)
				{
					g_Audio.m_TopUniqueId = UpdatePacket.m_UniqueId;
					g_Audio.m_TopCfxId = 0;
				}
                // An update was requested of a cfx that had already completed.
                // So we tell the host that it's completed again (it may have
                // missed it on the last run if the output buffer was full)
				if (UpdatePacket.m_UniqueId < g_Audio.m_TopUniqueId)
				{
                    audio_WriteBuffer(&UpdatePacket.m_Id,sizeof(s32));
                    break;
				}
                else if (UpdatePacket.m_Id <= g_Audio.m_TopCfxId)
                {
                    audio_WriteBuffer(&UpdatePacket.m_Id,sizeof(s32));
                    break;
                }
                else
                {
                    // Spawn new cfx if the update we're interested is for
                    // a cfx that isn't "alive" yet.
                    if ( (UpdatePacket.m_CfxId != -1) && (UpdatePacket.m_UniqueId >= g_Audio.m_TopUniqueId) )
                    {
                        audio_Lock("Create new cfx");
                        pCfx = cfx_Alloc(UpdatePacket.m_CfxId,&UpdatePacket);
                        if (pCfx)
                        {
                            pCfx->m_Identifier = UpdatePacket.m_Id;
                            ASSERT(UpdatePacket.m_Id > g_Audio.m_TopCfxId);
                            g_Audio.m_TopCfxId = UpdatePacket.m_Id;
                            pCfx->m_pNext = g_Audio.m_pRootCfx;
                            g_Audio.m_pRootCfx = pCfx;
                            if (UpdatePacket.m_UniqueId > g_Audio.m_TopUniqueId)
                            {
                                g_Audio.m_TopUniqueId = UpdatePacket.m_UniqueId;
                            }
                        }
                        audio_Unlock();
                    }
                }
            }
            else
            {
                if (UpdatePacket.m_CfxId==-1)
                {
                    pCfx->m_Status = CFXSTAT_STOPPING;
                }
                cfx_UpdateState(pCfx,&UpdatePacket);
            }
            break;
        case AUD_COMMAND_END:
            done = TRUE;
            break;

        case AUD_COMMAND_UPDATE_MASTER:
            audio_ReadBuffer(&g_Audio.m_MasterState.m_State,sizeof(g_Audio.m_MasterState.m_State));
            break;

        default:
            ASSERT(FALSE);
        }
    }

    // End of killed cfx id's passed back is ended with a 0. We will never
    // have a valid cfx id of 0
    done = 0;
    audio_WriteBuffer(&done,sizeof(s32));
}

//-----------------------------------------------------------------------------
void *audio_Dispatch(u32 Command,void *Data,s32 Size)
{
    s32 *pData;
    static s32 SimpleReturn;

    (void)Size;
    pData = (s32 *)Data;

    switch(Command)
    {
//-------------------------------------------------
    case AUDCMD_INIT:
        return audio_Init((audio_init *)pData);
        break;
//-------------------------------------------------
    case AUDCMD_KILL:
        audio_Kill();
        break;
//-------------------------------------------------
    case AUDCMD_LOADCONTAINER:
        return container_Load((char *)Data);
        break;
//-------------------------------------------------
    case AUDCMD_ISLOADCOMPLETE:
        return container_IsLoadComplete();
        break;
//-------------------------------------------------
    case AUDCMD_GETCONTAINERHEADER:
        return container_Header((container_hdr_request *)Data);
        break;
//-------------------------------------------------
    case AUDCMD_UNLOADCONTAINER:
        SimpleReturn = container_Unload(*(s32 *)Data);
        return &SimpleReturn;
        break;
//-------------------------------------------------
    case AUDCMD_CONTAINERVOLUME:
        container_SetVolume(pData[0],pData[1]);
        return &SimpleReturn;
        break;
//-------------------------------------------------
    case AUDCMD_UPDATE:
        audio_Update(Data);
        return Data;
        break;
//-------------------------------------------------
    case AUDCMD_STREAM_INIT:
        audio_Lock("hoststream_Init");
        hoststream_Init(*pData);
        audio_Unlock();
        break;
//-------------------------------------------------
    case AUDCMD_STREAM_KILL:
        audio_Lock("hoststream_Kill");
        hoststream_Kill();
        audio_Unlock();
        break;
//-------------------------------------------------
    case AUDCMD_STREAM_SEND_DATA_LEFT:
        audio_Lock("hoststream_Update left");
        SimpleReturn = hoststream_Update((u8 *)pData,Size,0);
        audio_Unlock();
        return &SimpleReturn;
        break;
//-------------------------------------------------
    case AUDCMD_STREAM_SEND_DATA_RIGHT:
        audio_Lock("hoststream_Update right");
        hoststream_Update((u8 *)pData,Size,1);
        audio_Unlock();
        return &SimpleReturn;
        break;
//-------------------------------------------------
    default:
        iop_DebugMsg("AudioDispatch: Invalid audio dispatch code 0x%08x\n",Command);
        break;
    }
    return Data;
}

