#include "ioptypes.h"
#include "iopaudio.h"
#include "iopmain.h"
#include "iopaud_cfx.h"
#include "iopaud_voice.h"
#include "iopaud_host.h"

//-----------------------------------------------------------------------------
cfx_pool    *cfxpool_FindOwner(cfx *pCfx)
{
    cfx_pool *pPool;

    pPool = g_Audio.m_pCfxPool;

    while (pPool)
    {
        if ( (pCfx>=pPool->m_CfxElements) &&
             (pCfx < &pPool->m_CfxElements[pPool->m_nEntries]) )
        {
            return pPool;
        }
        pPool = pPool->m_pNext;
		ASSERT(pPool != g_Audio.m_pCfxPool);
    }
    return NULL;
}

//-----------------------------------------------------------------------------
cfx_pool *cfxpool_Expand(s32 nEntries)
{
    cfx_pool    *pPool;
    cfx_pool    *pPrev;
    s32         i;

    pPool = (cfx_pool *)iop_Malloc(sizeof(cfx_pool) + sizeof(cfx) * nEntries);
    ASSERT(pPool);
    if (!pPool)
        return NULL;

    pPool->m_nEntries = nEntries;
    mq_Create(&pPool->m_qFree,nEntries,"cfxpool_qFree");

    pPrev = g_Audio.m_pCfxPool;
    if (pPrev)
    {
       while (pPrev->m_pNext)
       {
           pPrev = pPrev->m_pNext;
		   ASSERT(pPrev != g_Audio.m_pCfxPool);
       }
       pPool->m_pNext = pPrev->m_pNext;
       pPrev->m_pNext = pPool;
    }
    else
    {
       g_Audio.m_pCfxPool = pPool;
       pPool->m_pNext = NULL;
    }

    for (i=0;i<nEntries;i++)
    {
        cfx_Free(&pPool->m_CfxElements[i]);
    }
    return pPool;
}

//-----------------------------------------------------------------------------
void cfx_Init(void)
{
    s32 allocamount;

    allocamount = g_Audio.m_ChannelCount * sizeof(cfx) + sizeof(cfx_pool);
    
    g_Audio.m_pCfxPool = NULL;

    cfxpool_Expand(g_Audio.m_ChannelCount);
    
    g_Audio.m_TopCfxId        = 0;
}

//-----------------------------------------------------------------------------
void cfx_Kill(void)
{
    cfx_pool    *pPool,*pNext;

    pPool = g_Audio.m_pCfxPool;
    while (pPool)
    {
        pNext = pPool->m_pNext;
        mq_Destroy(&pPool->m_qFree);
        iop_Free(pPool);
        pPool = pNext;
		ASSERT(pPool != g_Audio.m_pCfxPool);
    }
    g_Audio.m_pCfxPool = NULL;
}

//-----------------------------------------------------------------------------
void cfx_Free(cfx *pCfx)
{
    cfx_pool *pPool;

    if (pCfx->m_pElements && (pCfx->m_pElements != pCfx->m_Elements))
    {
        iop_Free(pCfx->m_pElements);
    }

    ASSERT(pCfx);
    pPool = cfxpool_FindOwner(pCfx);
    mq_Send(&pPool->m_qFree,pCfx,MQ_BLOCK);
}

//-----------------------------------------------------------------------------
void        cfx_InsertForUpdate(cfx *pCfx)
{
    cfx *pLastCfx;

	ASSERT(audio_IsLocked());
    pCfx->m_pNext = NULL;
    pLastCfx = g_Audio.m_pRootCfx;
    if (!pLastCfx)
    {
        g_Audio.m_pRootCfx = pCfx;
        return;
    }
    // Find end of the master list
    while (pLastCfx->m_pNext)
    {
        pLastCfx = pLastCfx->m_pNext;
		ASSERT(pLastCfx != g_Audio.m_pRootCfx);
    }
    pLastCfx->m_pNext = pCfx;
}
//-----------------------------------------------------------------------------
void        cfx_RemoveFromUpdate(cfx *pCfx)
{
    cfx *pLastCfx,*pThisCfx;

    pLastCfx = NULL;

    pThisCfx = g_Audio.m_pRootCfx;
	ASSERT(audio_IsLocked());

    while (pThisCfx != pCfx)
    {
        pLastCfx = pThisCfx;
        pThisCfx = pThisCfx->m_pNext;
        ASSERT(pThisCfx);
		ASSERT(pThisCfx != g_Audio.m_pRootCfx);
    }

    if (pLastCfx)
    {
        pLastCfx->m_pNext = pCfx->m_pNext;
    }
    else
    {
        g_Audio.m_pRootCfx = pCfx->m_pNext;
    }
}

//-----------------------------------------------------------------------------
cfx *cfx_AllocFromPool(void)
{
    cfx_pool *pPool;
    cfx *pCfx;

    // Find a pool with some available space
    pPool = g_Audio.m_pCfxPool;

    while(1)
    {
        pCfx = mq_Recv(&pPool->m_qFree,MQ_NOBLOCK);
        if (pCfx)
            break;
        pPool = pPool->m_pNext;
        if (!pPool)
        {
            pPool = cfxpool_Expand(32);
            if (!pPool)
            {
                return NULL;
            }
            pCfx = mq_Recv(&pPool->m_qFree,MQ_NOBLOCK);
            ASSERT(pCfx);
            break;
        }
    }
    pCfx->m_pNext       = NULL;
    pCfx->m_Identifier  = 0;
    pCfx->m_Status      = CFXSTAT_STARTING;
    return pCfx;
}

//-----------------------------------------------------------------------------
cfx  *cfx_Alloc(s32 SampleId,cfx_state *pState)
{
    cfx *pCfx;
    cfx_element *pSample;
    cfx_element *pParent;
    s32 i;

    pSample = container_Find(SampleId);
    ASSERT(pSample);

    pCfx = cfx_AllocFromPool();
    ASSERT(pCfx);
    if (!pCfx)
        return NULL;
    pCfx->m_Count       = pSample->m_pAttributes->m_Count;
    pCfx->m_ElementCount= pSample->m_pAttributes->m_Count;

    pCfx->m_OriginalState.m_Pan = pSample->m_pAttributes->m_Pan;
    pCfx->m_OriginalState.m_Pitch = pSample->m_pAttributes->m_Pitch;
    pCfx->m_OriginalState.m_RearVol = pSample->m_pAttributes->m_Volume;
    pCfx->m_OriginalState.m_Volume = pSample->m_pAttributes->m_Volume;

    pCfx->m_OriginalState.m_CfxId = SampleId;

    pCfx->m_State       = pCfx->m_OriginalState;
    pCfx->m_Flags       = pSample->m_Flags;
    cfx_UpdateState(pCfx,pState);
    pCfx->m_State.m_RearVol = 0;

    if (pSample->m_pAttributes->m_Count > CFX_ELEMENTS_CONTAINED)
    {
        pCfx->m_pElements = iop_Malloc(pSample->m_pAttributes->m_Count * sizeof(cfx_element));
        ASSERT(pCfx->m_pElements);
    }
    else
    {
        pCfx->m_pElements   = pCfx->m_Elements;
    }

    if (pCfx->m_Count)
    {
        memcpy(pCfx->m_pElements,pSample+1,sizeof(cfx_element) * pCfx->m_Count);
    }
    else
    {
        // If we don't have any attached elements, we must not have a CFXTYPE_COMPLEX as the
        // starting sound effect. This would mean we've picked from a list of cfx bodies so
        // we fake it a little to make the system think we have a single element complex effect.
        // Allowing this "feature" will give us the facility to define one sound, with all it's
        // global properties and select from a list of enclosed elements. For example:
        // audio_Play(SFX_AMBIENT_WIND+1+x_irand(0,5));
        //
        pParent = container_Find(pSample->m_pAttributes->m_OwnerId);
        ASSERT(pParent);
        memcpy(pCfx->m_pElements,pSample,sizeof(cfx_element));
        pCfx->m_OriginalState.m_Pan = pParent->m_pAttributes->m_Pan;
        pCfx->m_OriginalState.m_Pitch = pParent->m_pAttributes->m_Pitch;
        pCfx->m_OriginalState.m_RearVol = pParent->m_pAttributes->m_Volume;
        pCfx->m_OriginalState.m_Volume = pParent->m_pAttributes->m_Volume;
        pCfx->m_Count = 1;
		pCfx->m_ElementCount = 1;
    }

    for (i=0;i<pCfx->m_Count;i++)
    {
        ASSERT(pCfx->m_pElements[i].m_pOwner==NULL);
        cfxelement_Init(&pCfx->m_pElements[i]);
        pCfx->m_pElements[i].m_pOwner = pCfx;
    }
    if (pCfx->m_Flags & AUDFLAG_CHANNELSAVER)
    {
        cfx *pCfxList;

        // If we are doing a channel save, we go through the master cfx list to see if
        // there is one the same as this. We will also have to include instance information
        // in the final version.

        pCfxList = g_Audio.m_pRootCfx;
        while (pCfxList)
        {
            if (pCfxList->m_OriginalState.m_CfxId == pCfx->m_OriginalState.m_CfxId)
            {

                if ( (pCfxList->m_Status == CFXSTAT_PLAYING) &&
                     (pCfxList->m_State.m_Volume < pCfx->m_State.m_Volume) )
                {
#ifdef X_DEBUG
                iop_DebugMsg("Channelsaver: Culled cfx #%d,cfxvol=%d,cullvol=%d\n",pCfxList->m_Identifier,pCfx->m_State.m_Volume,pCfxList->m_State.m_Volume);
#endif
                    pCfxList->m_Status = CFXSTAT_STOPPING;
                }
            }
            pCfxList = pCfxList->m_pNext;
			ASSERT(pCfxList != g_Audio.m_pRootCfx);
        }
    }
    return pCfx;
}
//-----------------------------------------------------------------------------
void  cfx_UpdateState(cfx *pCfx,cfx_state *pState)
{
    audio_DupState(&pCfx->m_State,&pCfx->m_OriginalState,pState);
    pCfx->m_State.m_Pan = pState->m_Pan;
}


//-----------------------------------------------------------------------------
xbool cfx_Update(cfx *pCfx,cfx_state *pParentState,s32 DeltaTime)
{
    cfx_element *pElement;
    xbool       status;
    s32         Count;
    cfx_state   State;
    s32         index;      // for test purposes

//    BREAK;

    pElement = pCfx->m_pElements;
    Count = pCfx->m_Count;
    index       = 0;

    if (pCfx->m_Status == CFXSTAT_STARTING)
    {
        pCfx->m_Status = CFXSTAT_PLAYING;
    }

    // This count is just the number of active, alive, elements.
    while ( Count )
    {
        ASSERT(index<pCfx->m_ElementCount);
        // Update all the elements except the expired ones
        if (pElement->m_Status != CFXSTAT_EXPIRED)
        {
            audio_DupState(&State,pParentState,&pCfx->m_State);
            if (pCfx->m_Status == CFXSTAT_STOPPING)
            {
                if (pElement->m_Status == CFXSTAT_PLAYING)
                {
                    pElement->m_Status = CFXSTAT_STOPPING;
                }

                if (pElement->m_Status == CFXSTAT_STARTING)
                {
                    pElement->m_Status = CFXSTAT_EXPIRED;
                }
            }

            status = cfxelement_Update(pElement,&State,DeltaTime);
            if (status)
            {
                pCfx->m_Count--;
            }
            Count--;
        }
        pElement++;
        index++;
    }

    if (pCfx->m_Count == 0)
    {
        pCfx->m_Status = CFXSTAT_EXPIRED;
    }
    return ( pCfx->m_Count == 0); 
}

//-----------------------------------------------------------------------------
cfx         *cfx_Find(s32 id)
{
    cfx *pCfx;
	cfx *pRoot;


    pCfx = g_Audio.m_pRootCfx;
	pRoot = pCfx;

    // Only cfx's at the top level will have anything useful to
    // be returned to the EE side. The identifiers used below this
    // level are just for internal tracking and debugging.
    while (pCfx)
    {
        if (pCfx->m_Identifier == id)
            return pCfx;
        pCfx = pCfx->m_pNext;
		ASSERT(pCfx != pRoot);
    }
    return NULL;
}
