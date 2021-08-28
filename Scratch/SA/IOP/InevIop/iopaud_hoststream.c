#include "iopaudio.h"
#include "iopmain.h"
#include "iopaud_element.h"
#include "iopaud_voice.h"
#include "iopaud_hoststream.h"

static cfx *hs_Init(s32 index,s32 Pitch)
{
    cfx *pCfx;
    cfx_element *pElement;

    pCfx             = cfx_AllocFromPool();
    pElement = &g_Audio.m_StreamerElement[index];

    pCfx->m_pElements       = pElement;
    pCfx->m_Count           = 1;
	pCfx->m_ElementCount	= 1;
    pCfx->m_State.m_Volume  = AUD_FIXED_POINT_1;

    if (index)
        pCfx->m_State.m_Pan = AUD_FIXED_POINT_1;
    else
        pCfx->m_State.m_Pan = -AUD_FIXED_POINT_1;

    pCfx->m_State.m_Pitch   = AUD_FIXED_POINT_1;
    pCfx->m_State.m_UniqueId= -1;
    pCfx->m_State.m_RearVol = 0;

    pElement->m_Flags       = AUDFLAG_ALLOCATED;
    pElement->m_Length      = (1<<24);
    pElement->m_pAttributes = &g_Audio.m_StreamerAttributes[index];
    pElement->m_pAttributes->m_SampleRate   = 44100;
    pElement->m_pAttributes->m_Type         = CFXTYPE_HOST_STREAM;
    pElement->m_pAttributes->m_Pitch        = Pitch;
    pElement->m_pAttributes->m_Volume       = AUD_FIXED_POINT_1;
    pElement->m_pAttributes->m_Pan          = 0;

    pElement->m_pStream      = voicestream_Alloc(pElement);
    cfx_InsertForUpdate(pCfx);
    return pCfx;
}

void hs_Kill(s32 index)
{
    s32 status;
    cfx_RemoveFromUpdate(g_Audio.m_pStreamerCfx[index]);
    while (1)
    {
        status = voicestream_Free(g_Audio.m_StreamerElement[index].m_pStream);
        if (status)
            break;
        DelayThread(100);
    }
    // We need to set this to null so that cfx_Free won't attempt to release it
    // since it really cant! It's static!
    g_Audio.m_pStreamerCfx[index]->m_pElements = NULL;
    cfx_Free(g_Audio.m_pStreamerCfx[index]);
}

void hoststream_Init(s32 Pitch)
{
    // We hook in host streamed data in to the main cfx update list by
    // constructing a series of dummy cfx and cfx element structs and
    // adding in to the update list accordingly.
    g_Audio.m_pStreamerCfx[0] = hs_Init(0,Pitch);
    g_Audio.m_pStreamerCfx[1] = hs_Init(1,Pitch);

}

void hoststream_Kill(void)
{
    hs_Kill(0);
    hs_Kill(1);

}

s32 hoststream_Update(u8 *pData,s32 Length,s32 Index)
{
    stream_request *pStreamRequest;
    s32 SimpleReturn;
    cfx *pCfx;

    pStreamRequest = (stream_request *)mq_Recv(&g_Audio.m_qAvailableStreamRequests,MQ_NOBLOCK);
    if (pStreamRequest)
    {
        pCfx = g_Audio.m_pStreamerCfx[Index];
        ASSERT(pStreamRequest->m_pData);
        ASSERT(Length <= AUD_STREAM_BUFFER_SIZE);
        memcpy(pStreamRequest->m_pData,pData,Length);

        ASSERT(pCfx->m_pElements->m_pStream);
        SimpleReturn = mq_Send(&pCfx->m_pElements->m_pStream->m_qReadyData,pStreamRequest,MQ_NOBLOCK);
        if (!SimpleReturn)
        {
            mq_Send(&g_Audio.m_qAvailableStreamRequests,pStreamRequest,MQ_BLOCK);
        }
        else
        {
            pCfx->m_pElements->m_pStream->m_PendingReadahead++;
            pCfx->m_pElements->m_pStream->m_ReadaheadInProgress++;
        }
    }
    else
    {
        SimpleReturn = FALSE;
    }
    return SimpleReturn;
}