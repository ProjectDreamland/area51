#include "ioptypes.h"
#include "iopaud_cfx.h"
#include "iopaud_element.h"
#include "iopaud_voice.h"
#include "iopaudio.h"
#include "iopaud_host.h"
#include "iopmain.h"



//-----------------------------------------------------------------------------
void    cfxelement_Init(cfx_element *pElement)
{
    pElement->m_ExpireDelay     = 1000;                 // Let the voice stick around for 100ms if it can't get a channel
    pElement->m_SurroundDelay   = -1;
    pElement->m_HybridPosition  = 0;
    pElement->m_pVoice          = NULL;
    pElement->m_pStream         = NULL;
    pElement->m_Status          = CFXSTAT_IDLE;

    ASSERT(pElement->m_pAttributes);

    switch (pElement->m_pAttributes->m_Type)
    {
    case CFXTYPE_ELEMENT:
        break;
    case CFXTYPE_ELEMENT_STREAM:
        break;
    case CFXTYPE_ELEMENT_HYBRID:
        break;
	default:
		ASSERT(FALSE);
    }
}

//-----------------------------------------------------------------------------
void    cfxelement_Kill(cfx_element *pElement)
{
    ASSERT(pElement->m_pAttributes);
    switch (pElement->m_pAttributes->m_Type)
    {
    case CFXTYPE_ELEMENT_STREAM:
    case CFXTYPE_ELEMENT_HYBRID:
        break;
    case CFXTYPE_ELEMENT:
        break;
    default:
        ASSERT(FALSE);
    }
}

//-----------------------------------------------------------------------------
xbool   cfxelement_Update(cfx_element *pElement,cfx_state *pParentState,s32 DeltaTime)
{
    audio_voice *pVoice;
    voice_stream *pStream;
    cfx_state   State,OriginalState;
    xbool       status;
    cfx         *pCfx;

    // Update envelopes here.

    // Now update the delay time on this element to see if we
    // need to actually start the voice. It will be below 0 if
    // the voice had already been started.

    if (pElement->m_Status == CFXSTAT_IDLE)
    {
        pElement->m_Delay -= DeltaTime;
        if (pElement->m_Delay <= 0)
        {
            pElement->m_Status  = CFXSTAT_STARTING;
            pElement->m_Delay   = 0;
        }
        else
        {
            return FALSE;
        }
    }

    if (pElement->m_Status == CFXSTAT_STARTING)
    {
        ASSERT(pElement->m_pAttributes);
        switch (pElement->m_pAttributes->m_Type)
        {
//----------------------------------------
        case CFXTYPE_COMPLEX:
            // Here we need to spawn a new cfx from this element
            // and the cfx would be added to the end of the master cfx list
            if (pElement->m_MediaLocation == -1)
            {
                pElement->m_Status = CFXSTAT_EXPIRED;
            }
            else
            {
                pCfx = cfx_Alloc(pElement->m_MediaLocation,pParentState);
                if (pCfx)
                {
                    cfx_InsertForUpdate(pCfx);

                    pElement->m_Status = CFXSTAT_EXPIRED;
                }
            }
            break;
//----------------------------------------
        case CFXTYPE_ELEMENT:
            pVoice = voice_Alloc(pElement);
            if (pVoice)
            {
                pElement->m_Status  = CFXSTAT_PLAYING;
                pElement->m_pVoice  = pVoice;
                if (pElement->m_Flags & AUDFLAG_3D_POSITION)
                {
                    pElement->m_SurroundDelay = 100;
                }
            }
            else
            {
                pElement->m_ExpireDelay -= DeltaTime ;
                if (pElement->m_ExpireDelay<=0)
                {
                    pElement->m_Status = CFXSTAT_EXPIRED;
					iop_DebugMsg("Expired element id 0x%08x\n",pElement->m_pOwner->m_Identifier);
                }
                else
                {
                    g_Audio.m_VoicesToKill++;
                    pElement->m_Delay = 0;
                }
            }
            break;
//----------------------------------------
        case CFXTYPE_ELEMENT_STREAM:
        case CFXTYPE_ELEMENT_HYBRID:
            pStream = voicestream_Alloc(pElement);
            if (pStream)
            {
                pElement->m_Status = CFXSTAT_PLAYING;
                pElement->m_pStream = pStream;
            }
            else
            {
                pElement->m_ExpireDelay -= DeltaTime;
                if (pElement->m_ExpireDelay <= 0)
                {
                    pElement->m_Status = CFXSTAT_EXPIRED;
                }
            }
            break;
//----------------------------------------
        case CFXTYPE_HOST_STREAM:
            pElement->m_Status = CFXSTAT_PLAYING;
            break;
        default:
            ASSERT(FALSE);
        }
    }

    if (pElement->m_Status == CFXSTAT_PLAYING)
    {
        switch (pElement->m_pAttributes->m_Type)
        {
//----------------------------------------
        case CFXTYPE_COMPLEX:
            ASSERT(FALSE);
            break;
//----------------------------------------
        case CFXTYPE_ELEMENT:
            ASSERT(pElement->m_pStream==NULL);
            ASSERT(pElement->m_pVoice);

            pVoice = pElement->m_pVoice;
            ASSERT(pVoice);
            OriginalState.m_Pan = pElement->m_pAttributes->m_Pan;
            OriginalState.m_Pitch = pElement->m_pAttributes->m_Pitch;
            OriginalState.m_Volume = pElement->m_pAttributes->m_Volume;
            OriginalState.m_RearVol = pElement->m_pAttributes->m_Volume;
            audio_DupState(&State,pParentState,&OriginalState);
            if (pElement->m_pAttributes->m_Pan != 0)
            {
                State.m_Pan = pElement->m_pAttributes->m_Pan;
            }

            status = voice_Update(pVoice,&State,DeltaTime);
            if (status)
            {
                pElement->m_Status = CFXSTAT_STOPPING;
            }
            break;
//----------------------------------------
        case CFXTYPE_HOST_STREAM:
        case CFXTYPE_ELEMENT_STREAM:
        case CFXTYPE_ELEMENT_HYBRID:
            ASSERT(pElement->m_pStream);
            ASSERT(pElement->m_pVoice == NULL);
            OriginalState.m_Pan = pElement->m_pAttributes->m_Pan;
            OriginalState.m_Pitch = pElement->m_pAttributes->m_Pitch;
            OriginalState.m_Volume = pElement->m_pAttributes->m_Volume;
            OriginalState.m_RearVol = pElement->m_pAttributes->m_Volume;
            audio_DupState(&State,pParentState,&OriginalState);
            status = voicestream_Update(pElement->m_pStream,&State,DeltaTime);
            if (status)
            {
                pElement->m_Status = CFXSTAT_STOPPING;
            }
            break;
//----------------------------------------
        default:
            ASSERT(FALSE);
        }
    }

    if (pElement->m_Status == CFXSTAT_STOPPING)
    {
        switch (pElement->m_pAttributes->m_Type)
        {
        case CFXTYPE_ELEMENT_STREAM:
        case CFXTYPE_ELEMENT_HYBRID:
        case CFXTYPE_HOST_STREAM:
            ASSERT(pElement->m_pStream);
            if (voicestream_Free(pElement->m_pStream))
            {
                pElement->m_Status = CFXSTAT_EXPIRED;
            }
            break;
        case CFXTYPE_ELEMENT:
            ASSERT(pElement->m_pStream==NULL);
            ASSERT(pElement->m_pVoice);

            pVoice = pElement->m_pVoice;
            ASSERT(pVoice);
            pVoice->m_Status = VSTAT_STOPPING;
            OriginalState.m_Pan = pElement->m_pAttributes->m_Pan;
            OriginalState.m_Pitch = pElement->m_pAttributes->m_Pitch;
            OriginalState.m_Volume = pElement->m_pAttributes->m_Volume;
            OriginalState.m_RearVol = pElement->m_pAttributes->m_Volume;
            audio_DupState(&State,pParentState,&OriginalState);
            if (pElement->m_pAttributes->m_Pan != 0)
            {
                State.m_Pan = pElement->m_pAttributes->m_Pan;
            }

            status = voice_Update(pVoice,&State,DeltaTime);
            if (status)
            {
                voice_Free(pVoice);
                pElement->m_Status = CFXSTAT_EXPIRED;
            }
            break;
        default:
            ASSERT(FALSE);
        }
    }
    
    return (pElement->m_Status == CFXSTAT_EXPIRED);

}

