#include "iopaudio.h"
#include "iopmain.h"
#include "iopaud_element.h"
#include "iopaud_voice.h"
#include "iopaud_host.h"

//#define AUDIO_DEBUG

//-----------------------------------------------------------------------------
void voice_Init(void)
{
    s32 i;

    g_Audio.m_pVoices = (audio_voice *)iop_Malloc(sizeof(audio_voice) * AUD_MAX_PHYS_VOICES);
    ASSERT(g_Audio.m_pVoices);
    memset(g_Audio.m_pVoices,0,sizeof(audio_voice)* AUD_MAX_PHYS_VOICES);
    mq_Create(&g_Audio.m_qFreeVoices,AUD_MAX_PHYS_VOICES,"qFreeVoices");
    for (i=0;i<AUD_MAX_PHYS_VOICES;i++)
    {
        g_Audio.m_pVoices[i].m_Status = VSTAT_IDLE;
        g_Audio.m_pVoices[i].m_Id     = AUD_MAX_PHYS_VOICES-i-1;
        mq_Send(&g_Audio.m_qFreeVoices,&g_Audio.m_pVoices[i],MQ_BLOCK);
    }

    g_Audio.m_VoicesToKill = 0;
    g_Audio.m_VoicesInUse  = 0;
}

//-----------------------------------------------------------------------------
void voice_Kill(void)
{
    mq_Destroy(&g_Audio.m_qFreeVoices);
    iop_Free(g_Audio.m_pVoices);
}

//-----------------------------------------------------------------------------
audio_voice *voice_Alloc(cfx_element *pElement)
{
    audio_voice *pVoice;

    pVoice = (audio_voice *)mq_Recv(&g_Audio.m_qFreeVoices,MQ_NOBLOCK);
    if (!pVoice)
    {
        if (pElement->m_pAttributes->m_Priority < g_Audio.m_LowestKillPriority)
        {
            g_Audio.m_LowestKillPriority = pElement->m_pAttributes->m_Priority;
        }
        g_Audio.m_VoicesToKill++;
        return NULL;
    }

    pVoice->m_Status        = VSTAT_IDLE;
    pVoice->m_ExpireDelay   = 100;
    pVoice->m_pOwner        = pElement;
    pVoice->m_SpuCurrent    = pElement->m_SpuLocation;
    pVoice->m_pSibling      = NULL;
    pVoice->m_Age           = 0;
    //
    // A non streamed element should have it's voice started immediately.
    //
    if (pElement->m_pAttributes->m_Type == CFXTYPE_ELEMENT)
    {
        spu_InitVoice(pVoice->m_Id,pVoice->m_SpuCurrent,pVoice->m_pOwner->m_pAttributes->m_ADSR1,pVoice->m_pOwner->m_pAttributes->m_ADSR2);
        spu_SetKey(pVoice->m_Id,1);
        pVoice->m_Status = VSTAT_PLAYING;
    }
    // Start voice here from details in pElement
    return pVoice;
}

//-----------------------------------------------------------------------------
void voice_Free(audio_voice *pVoice)
{
    ASSERT(pVoice);
    ASSERT(pVoice->m_Status != VSTAT_FREE);
    //
    // Before we free it, we should make sure that it has been muted and stopped
    //
    spu_SetKey(pVoice->m_Id,0);
    pVoice->m_Status = VSTAT_FREE;
    pVoice->m_Hardware.Pitch = 0;
    pVoice->m_Hardware.LeftVolume = 0;
    pVoice->m_Hardware.RightVolume = 0;
    spu_WriteVoice(pVoice->m_Id,&pVoice->m_Hardware);
    if (pVoice->m_pSibling)
    {
        voice_Free(pVoice->m_pSibling);
    }
    mq_Send(&g_Audio.m_qFreeVoices,pVoice,MQ_BLOCK);
}

s32 qsort_compare(void *arg1,void *arg2)
{
    return (*(s32 *)arg1 - *(s32 *)arg2);
}
//-----------------------------------------------------------------------------

s32 voice_Cull(s32 NumberToKill)
{
    s32         NumberKilled;
    s32         SortKey[AUD_MAX_PHYS_VOICES];
    audio_voice *pVoices;
    s32         i,index;

    NumberKilled = 0;

    pVoices = g_Audio.m_pVoices;
    index = 0;
#ifdef X_DEBUG
    iop_DebugMsg("******************* Culling %d voices\n",NumberToKill);
#endif
    for (i=0;i<AUD_MAX_PHYS_VOICES;i++)
    {
        if ( (pVoices->m_Status == VSTAT_STOPPING) ||
             (pVoices->m_Status == VSTAT_FREE) ||
			 (pVoices->m_Status == VSTAT_IDLE) )
        {
            NumberToKill--;
            if (NumberToKill <= 0)
                return 0;
        }
        else
        {
            if ( (pVoices->m_pOwner->m_pAttributes->m_Type != CFXTYPE_ELEMENT_STREAM) &&
                 (pVoices->m_pOwner->m_pAttributes->m_Type != CFXTYPE_ELEMENT_HYBRID) &&
                 (pVoices->m_pOwner->m_pAttributes->m_Priority < g_Audio.m_LowestKillPriority) &&
                 ( (pVoices->m_pOwner->m_Flags & (AUDFLAG_LOOPED|AUDFLAG_HARDWARE_LOOP))==0 ) )
            {
                SortKey[index]  = pVoices->m_pOwner->m_pAttributes->m_Priority<<24;
                SortKey[index] |= (pVoices->m_pOwner->m_pOwner->m_State.m_Volume / (AUD_FIXED_POINT_1 / 128))<<16;
                SortKey[index] |= ((-pVoices->m_Age) & 0xff << 8);
                SortKey[index] |= i;
       
                index++;
            }
        }
        pVoices++;
    }
    // If index is still 0, this means we were unable to find any voices with higher priorities to kill
    //
    if (!index)
        return 0;
    iop_qsort(SortKey,index,sizeof(s32),qsort_compare);
    i=0;
    while (NumberToKill--)
    {
        audio_voice *pVoice;
//**NOTE: Need to put in code to make sure that we do not kill a voice
// that has a priority greater than the sample that's trying to start.
// If we end up consuming the entire array, then we could not delete
// any more voices.

        pVoice = &g_Audio.m_pVoices[SortKey[i] & 0xff];
#ifdef X_DEBUG
        iop_DebugMsg("Culled voice %d,%08x,%08x\n",pVoice->m_Id,SortKey[i],pVoice->m_pOwner->m_pOwner->m_Identifier);
#endif
        spu_SetKey(pVoice->m_Id,0);
        pVoice->m_Status = VSTAT_STOPPING;
        NumberKilled++;
        i++;
        if (i>=index)
            break;
    }
    return NumberKilled;

}

//-----------------------------------------------------------------------------
xbool       voice_Update(audio_voice *pVoice,cfx_state *pState,s32 DeltaTime)
{
    s32         offset;
    s32         PredictedOffset;
    spu_state   CurrentState;
    s32         leftvol,rightvol;

    spu_ReadVoice(pVoice->m_Id,&CurrentState);
    pVoice->m_Hardware.Current = CurrentState.Current;
    pVoice->m_Hardware.Envelope = CurrentState.Envelope;
    pVoice->m_Hardware.Finished = CurrentState.Finished;
    // This is the predicted audio sound position
    PredictedOffset =  ((pVoice->m_pOwner->m_pAttributes->m_SampleRate * pState->m_Pitch) / 100);
    PredictedOffset *= DeltaTime;
    pVoice->m_Age++;

    offset = pVoice->m_Hardware.Current - pVoice->m_SpuCurrent;
    if (pVoice->m_Status == VSTAT_STOPPING)
    {
        spu_SetKey(pVoice->m_Id,0);
        if (pVoice->m_Hardware.Envelope == 0)
        {
            pVoice->m_Status = VSTAT_IDLE;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    switch (pVoice->m_pOwner->m_pAttributes->m_Type)
    {
    case CFXTYPE_ELEMENT:
        offset = (u32)CurrentState.Current - (u32)pVoice->m_pOwner->m_SpuLocation;
        if (CurrentState.Finished)
        {
            if (pVoice->m_pOwner->m_Flags & AUDFLAG_LOOPED)
            {
                // Restart the sound here, this is simulated
                pVoice->m_SpuCurrent = pVoice->m_pOwner->m_SpuLocation;
                spu_InitVoice(pVoice->m_Id,pVoice->m_SpuCurrent,pVoice->m_pOwner->m_pAttributes->m_ADSR1,pVoice->m_pOwner->m_pAttributes->m_ADSR2);
                spu_SetKey(pVoice->m_Id,1);
            }
            else
            {
                if ((pVoice->m_pOwner->m_Flags & AUDFLAG_HARDWARE_LOOP)==0)
                {
                    spu_SetKey(pVoice->m_Id,0);
                    // Kill the sound here
                    return TRUE;
                }
            }
        }
        break;
    case CFXTYPE_ELEMENT_HYBRID:
    case CFXTYPE_ELEMENT_STREAM:
    case CFXTYPE_HOST_STREAM:
        break;
    default:
        ASSERT(FALSE);
    }

    pVoice->m_Hardware.Pitch = (pState->m_Pitch * 4096)/AUD_FIXED_POINT_1;
    pVoice->m_Hardware.Pitch = (pVoice->m_Hardware.Pitch * pVoice->m_pOwner->m_pAttributes->m_SampleRate) / SPU_BASE_SAMPLE_RATE;

    leftvol = pVoice->m_Hardware.LeftVolume;
    rightvol = pVoice->m_Hardware.RightVolume;

    pVoice->m_Hardware.RightVolume = (pState->m_Volume * 16383)/AUD_FIXED_POINT_1;
    pVoice->m_Hardware.LeftVolume = (pState->m_Volume * 16383)/AUD_FIXED_POINT_1;

    if (pState->m_Pan < 0)
    {
        pVoice->m_Hardware.RightVolume = (pVoice->m_Hardware.RightVolume * (AUD_FIXED_POINT_1 + pState->m_Pan)) / AUD_FIXED_POINT_1;
    }
    else if (pState->m_Pan > 0)
    {
        pVoice->m_Hardware.LeftVolume = (pVoice->m_Hardware.LeftVolume * (AUD_FIXED_POINT_1 - pState->m_Pan)) / AUD_FIXED_POINT_1;
    }

    if (pVoice->m_pSibling)
    {
        spu_ReadVoice(pVoice->m_Id,&CurrentState);
        pVoice->m_pSibling->m_Hardware.Current = CurrentState.Current;
        pVoice->m_pSibling->m_Hardware.Envelope = CurrentState.Envelope;
        pVoice->m_pSibling->m_Hardware.Finished = CurrentState.Finished;
        if (pState->m_RearVol==0)
        {
            if (pVoice->m_pSibling->m_Hardware.Envelope==0)
            {
#ifdef AUDIO_DEBUG
                iop_DebugMsg("Surround voice killed\n");
#endif
                voice_Free(pVoice->m_pSibling);
                pVoice->m_pSibling = NULL;
            }
            else
            {
                spu_SetKey(pVoice->m_pSibling->m_Id,0);
            }
        }
        else
        {
            s32 vol;

            vol = (pState->m_RearVol * 16383)/AUD_FIXED_POINT_1;
            if (vol > 16383)
            {
                vol = 16383;
            }
            pVoice->m_pSibling->m_Hardware.LeftVolume = vol;
            pVoice->m_pSibling->m_Hardware.RightVolume = (-vol & 0x7fff);
            pVoice->m_pSibling->m_Hardware.Pitch = pVoice->m_Hardware.Pitch;
            spu_WriteVoice(pVoice->m_pSibling->m_Id,&pVoice->m_pSibling->m_Hardware);
        }
    }
    else
    {
        if (pState->m_RearVol)
        {
            s32 current;

            current = CurrentState.Current - 128;
            if (current >= pVoice->m_pOwner->m_SpuLocation)
            {
                pVoice->m_pSibling = voice_Alloc(pVoice->m_pOwner);
                if (pVoice->m_pSibling)
                {
                    s32 vol;

                    vol = (pState->m_RearVol * 16383)/AUD_FIXED_POINT_1;
                    if (vol > 16383)
                    {
                        vol = 16383;
                    }
                    pVoice->m_SpuCurrent = pVoice->m_pOwner->m_SpuLocation;
                    spu_InitVoice(pVoice->m_pSibling->m_Id,pVoice->m_pOwner->m_SpuLocation,pVoice->m_pOwner->m_pAttributes->m_ADSR1,pVoice->m_pOwner->m_pAttributes->m_ADSR2);
                    pVoice->m_pSibling->m_Hardware.Pitch = pVoice->m_Hardware.Pitch;
                    pVoice->m_pSibling->m_Hardware.LeftVolume = vol;
                    pVoice->m_pSibling->m_Hardware.RightVolume = (-vol & 0x7fff);
                    spu_WriteVoice(pVoice->m_pSibling->m_Id,&pVoice->m_pSibling->m_Hardware);
                    spu_SetKey(pVoice->m_pSibling->m_Id,1);
#ifdef AUDIO_DEBUG
                    iop_DebugMsg("Surround voice activated\n");
#endif
                }
            }
        }
    }

    if (pVoice->m_Hardware.LeftVolume > 16383)
        pVoice->m_Hardware.LeftVolume = 16383;
    if (pVoice->m_Hardware.RightVolume > 16383)
        pVoice->m_Hardware.RightVolume = 16383;
    if (pVoice->m_Hardware.LeftVolume < 0)
        pVoice->m_Hardware.LeftVolume = 0;
    if (pVoice->m_Hardware.RightVolume < 0)
        pVoice->m_Hardware.RightVolume = 0;



#if 0
#ifdef X_DEBUG
    {
        static s32 i=10;

        i--;
        if (i<0)
        {
            spu_state spustate;
            iop_DebugMsg("Pan=%d, Left vol=%d, right vol=%d",pState->m_Pan,pVoice->m_Hardware.LeftVolume,pVoice->m_Hardware.RightVolume);
            if (pVoice->m_pSibling)
            {
                spu_ReadVoice(pVoice->m_pSibling->m_Id,&spustate);
                iop_DebugMsg(", Rear Volume =%d, delta pos=%d\n",pVoice->m_pSibling->m_Hardware.LeftVolume,CurrentState.Current-spustate.Current);
            }
            else
            {
                iop_DebugMsg("\n");
            }
            i=10;
        }
    }
#endif
#endif
    spu_WriteVoice(pVoice->m_Id,&pVoice->m_Hardware);

    return FALSE;
}


