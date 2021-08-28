#include <kernel.h>
#include <sys/file.h>
#include <stdio.h>
#include "ioptypes.h"
#include "iopaud_stream.h"
#include "iopaud_cfx.h"
#include "iopaudio.h"
#include "iopmqueue.h"
#include "iopmain.h"
#include "iopaud_host.h"
#include "iopthreadpri.h"
#include "libcdvd.h"

//#define DUMP_VAG
//-----------------------------------------------------------------------------
// Internal only functions - should be static but wary of debugger problems....
//-----------------------------------------------------------------------------
xbool voicestream_UpdateReadahead(voice_stream *pStream);
xbool voicestream_FillBuffers(voice_stream *pStream);
s32 testfile;
#ifdef X_DEBUG
//#define STREAM_DEBUG
#endif
#define VAG_LOOP        (1<<1)
#define VAG_LOOP_START ((1<<2)|VAG_LOOP)
#define VAG_LOOP_END   ((1<<0)|VAG_LOOP)

#define BLOCK_COUNT(x)  ((((x)+4095)/4096) * 2)
enum stream_state_enum
{
    SS_WAITING,
    SS_SEEK,
    SS_READ,
    SS_SENDING,
};

enum stream_state_enum s_streaming_state;
stream_request *s_active_request;
voice_stream *s_pStreamList;


//-----------------------------------------------------------------------------
void        voicestream_PeriodicUpdate(void)
{
    stream_request *pRequest;
    s32             amount;
    s32             lsn;
    sceCdRMode      rmode;
    s32             status;

#ifdef STREAM_DEBUG
    s32 mintime,maxtime,time,totaltime;
    s32 count,i;

    count=10;
    mintime=0;
    maxtime=0;
    totaltime=0;
#endif

    rmode.trycount      = 10;
    rmode.spindlctrl    = SCECdSpinStm;
    rmode.datapattern   = SCECdSecS2048;
    while (1)
    {
        s_streaming_state = SS_WAITING;
        s_active_request = NULL;
        pRequest = (stream_request *)mq_Recv(&g_Audio.m_qPendingStreamRequests,MQ_BLOCK);
        ASSERT(pRequest);
        s_active_request = pRequest;
#ifdef STREAM_DEBUG
        time = iop_GetTime();
#endif
        if (pRequest->m_FileId < 0)
        {
            lsn = pRequest->m_FileId & ~(1<<31);
            lsn += pRequest->m_Offset / 2048;
            // We're using a cd streaming call
            s_streaming_state = SS_READ;
            status = sceCdRead(lsn,1,pRequest->m_pData,&rmode);
            if (status)
            {
                sceCdSync(0);
                status = sceCdGetError() == SCECdErNO;
            }

            if (!status)
            {
                ASSERT(pRequest->m_Length == 2048);
                memset(pRequest->m_pData,0,pRequest->m_Length);
                iop_DebugMsg("CD Read error occurred at lsn %d\n",lsn);
            }

        }
        else
        {
            s_streaming_state = SS_SEEK;
            lseek(pRequest->m_FileId,pRequest->m_Offset,0);
            s_streaming_state = SS_READ;
            amount = read(pRequest->m_FileId,pRequest->m_pData,pRequest->m_Length);
        }

#ifdef STREAM_DEBUG
        time = iop_GetTime()-time;

        if (time < mintime) mintime = time;
        if (time > maxtime) maxtime = time;
        totaltime+=time;

        count--;
        if (count<0)
        {
            count=50;
            printf("Streaming time, min=%d, max=%d, avg=%d\n",mintime,maxtime,totaltime/10);
            mintime = 100000;
            maxtime = 0;
            totaltime = 0;
        }
        for (i=0;i<AUD_MAX_STREAMS;i++)
        {
            if (s_pStreamList[i].m_Status == STREAMSTAT_IDLE)
            {
                ASSERT(mq_IsEmpty(&s_pStreamList[i].m_qReadyData));
                ASSERT(s_pStreamList[i].m_ReadaheadInProgress==0);
            }
        }
#endif
        ASSERT(pRequest->m_pqReply);
        ASSERT(pRequest->m_Length == 2048);
        s_streaming_state = SS_SENDING;
        mq_Send(pRequest->m_pqReply,pRequest,MQ_BLOCK);
    }
}

//-----------------------------------------------------------------------------
void        voicestream_Init(s32 nBuffers,s32 size)
{
    s32         i;
    voice_stream  *pStreams;

    ASSERT(size==AUD_STREAM_BUFFER_SIZE);
    pStreams = iop_Malloc(nBuffers * sizeof(voice_stream) );
    ASSERT(pStreams);
// Create our stream list
    mq_Create(&g_Audio.m_qFreeStreams,nBuffers,"qFreeStreams");
    memset(pStreams,0,sizeof(voice_stream)*nBuffers);
#ifdef STREAM_DEBUG
    s_pStreamList = pStreams;
#endif

    for (i=0;i<nBuffers;i++)
    {
        pStreams->m_SpuBuffer = spu_Alloc(size * 2,"SPU Buffer");
// Message queue for completed pre-read blocks
        mq_Create(&pStreams->m_qReadyData,AUD_STREAM_READAHEAD,"stream qReadyData");
        mq_Send(&g_Audio.m_qFreeStreams,pStreams,MQ_BLOCK);
        pStreams++;
    }
    g_Audio.m_pStreamBuffers = iop_Malloc(AUD_STREAM_BUFFER_SIZE * AUD_MAX_STREAMS * AUD_STREAM_READAHEAD);
    ASSERT(g_Audio.m_pStreamBuffers);
// Message queue for the currently available buffers
    mq_Create(&g_Audio.m_qAvailableStreamRequests,AUD_MAX_STREAMS * AUD_STREAM_READAHEAD,"qAvailableStreamRequests");
// Message queue for the currently outstanding read requests
    mq_Create(&g_Audio.m_qPendingStreamRequests,AUD_MAX_STREAMS * AUD_STREAM_READAHEAD,"qPendingStreamRequests");
    for (i=0;i<AUD_MAX_STREAMS * AUD_STREAM_READAHEAD;i++)
    {
        g_Audio.m_StreamRequests[i].m_pData = &g_Audio.m_pStreamBuffers[i*AUD_STREAM_BUFFER_SIZE];
        mq_Send(&g_Audio.m_qAvailableStreamRequests,&g_Audio.m_StreamRequests[i],MQ_BLOCK);
    }
    g_Audio.m_StreamThreadId = iop_CreateThread(voicestream_PeriodicUpdate,AUD_STREAM_PRIORITY,2048,"voicestream_PeriodicUpdate");

}

void voicestream_Kill(void)
{
    voice_stream *pStream;

    iop_DestroyThread(g_Audio.m_StreamThreadId);
    mq_Destroy(&g_Audio.m_qPendingStreamRequests);
    mq_Destroy(&g_Audio.m_qAvailableStreamRequests);
    iop_Free(g_Audio.m_pStreamBuffers);
    while(1)
    {
        pStream = mq_Recv(&g_Audio.m_qFreeStreams,MQ_NOBLOCK);
        if (pStream==NULL)
            break;
        mq_Destroy(&pStream->m_qReadyData);
    }
    iop_Free(s_pStreamList);
}

#ifdef DUMP_VAG
char vagheader[0x30]=
{
    0x56,0x41,0x47,0x70,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xA0,
    0x00,0x00,0x7D,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x73,0x31,0x31,0x5F,0x30,0x33,0x2F,0x4D,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

};
#endif
//-----------------------------------------------------------------------------
voice_stream *voicestream_Alloc(cfx_element *pElement)
{
    voice_stream *pStream;
    xbool        status;

#ifdef DUMP_VAG
    testfile = open("host0:raw.vag",O_WRONLY|O_CREAT);
    write(testfile,vagheader,0x30);
#endif
    pStream = mq_Recv(&g_Audio.m_qFreeStreams,MQ_NOBLOCK);
    if (!pStream) 
        return NULL;
    pStream->m_pOwner           = pElement;
    if (pElement->m_Flags & AUDFLAG_STEREO)
    {
        pStream->m_PendingReadahead = AUD_STREAM_READAHEAD * 2;
    }
    else
    {
        pStream->m_PendingReadahead = AUD_STREAM_READAHEAD;
    }
    pStream->m_BufferCount      = pStream->m_PendingReadahead;
    pStream->m_Status           = STREAMSTAT_STARTING;
    pStream->m_ReadaheadPosition= 0;
    pStream->m_FirstHalfFull    = 0;
    pStream->m_ActualPlayPosition = 0;
    pStream->m_LastOffset       = 0;
    pStream->m_PlayRemaining    = BLOCK_COUNT(pElement->m_Length);
    pStream->m_ReadaheadRemaining = pStream->m_PlayRemaining;
    pStream->m_ReadaheadInProgress = 0;
    
    if (pElement->m_Flags & AUDFLAG_STEREO)
    {
        // Reset the stereo flag so it doesn't try to allocate a sibling
        // next time round
        pElement->m_Flags &= ~AUDFLAG_STEREO;
        pStream->m_pSibling     = voicestream_Alloc(pElement);
        pElement->m_Flags |= AUDFLAG_STEREO;
        //
        // Set the flag again
        //
        if (!pStream->m_pSibling)
        {
            status = voicestream_Free(pStream);
            ASSERT(status);
            return NULL;
        }
    }
    else
    {
        pStream->m_pSibling     = NULL;
    }

    pStream->m_pVoice           = voice_Alloc(pElement);
    // If we can't get a voice, just bail now
    if (!pStream->m_pVoice)
    {
        if (pStream->m_pSibling)
        {
            status = voicestream_Free(pStream->m_pSibling);
            ASSERT(status);
        }

        status = voicestream_Free(pStream);
        ASSERT(status);
        return NULL;
    }
    voicestream_UpdateReadahead(pStream);
    return (pStream);
}

//-----------------------------------------------------------------------------
xbool        voicestream_Free(voice_stream *pStream)
{
    stream_request *pReq;
    xbool   status;

    ASSERT(pStream);
#ifdef DUMP_VAG
    close(testfile);
#endif
    if (pStream->m_pSibling)
    {
        ASSERT(pStream->m_pOwner->m_Flags & AUDFLAG_STEREO);
        if (!voicestream_Free(pStream->m_pSibling))
        {
            return FALSE;
        }
        else
        {
            pStream->m_pSibling = NULL;
        }
    }

    // Clear out any pending buffers for this stream
    while (pStream->m_ReadaheadInProgress)
    {
        pReq=mq_Recv(&pStream->m_qReadyData,MQ_NOBLOCK);
        if (pReq)
        {
            status = mq_Send(&g_Audio.m_qAvailableStreamRequests,pReq,MQ_NOBLOCK);
            ASSERT(status);
            pStream->m_ReadaheadInProgress--;
        }
        else
        {
            break;
        }
    }

    if (pStream->m_pVoice)
    {
        voice_Free(pStream->m_pVoice);
        pStream->m_pVoice = FALSE;
    }

    // if we were unable to clear out all of the pending read requests on this
    // turn, we leave the stream in a stopping state until the pending read 
    // requests have all been recovered
    if (pStream->m_ReadaheadInProgress)
    {
        pStream->m_Status = STREAMSTAT_STOPPING;
        return FALSE;
    }

    pStream->m_Status = STREAMSTAT_IDLE;
    mq_Send(&g_Audio.m_qFreeStreams,pStream,MQ_BLOCK);
    return TRUE;
}

//-----------------------------------------------------------------------------
xbool       voicestream_Update(voice_stream *pStream,cfx_state *pState,s32 DeltaTime)
{
    s32 status;
    xbool Stereo;
    cfx_state State;
    s32 offset,newoffset;

    State = *pState;

    Stereo = pStream->m_pOwner->m_Flags & AUDFLAG_STEREO;

    if (pStream->m_Status != STREAMSTAT_STARTING)
    {
        {
            if (Stereo)
            {
                ASSERT((pStream->m_PlayRemaining & 1)==0);
                State.m_Pan = -AUD_FIXED_POINT_1;
            }
            
            status = voice_Update(pStream->m_pVoice,&State,DeltaTime);
            // Voice should NEVER finish before the stream is finished
            ASSERT(!status);
        }

        if (pStream->m_pSibling)
        {
            ASSERT(Stereo);
            State.m_Pan = AUD_FIXED_POINT_1;
            status = voice_Update(pStream->m_pSibling->m_pVoice,&State,DeltaTime);
            ASSERT(!status);
        }
    }

    //
    // To make sure we terminate the stream properly, we need to make sure that
    // we keep track of the actual hardware voice position within the data.
    //
    newoffset = (pStream->m_pVoice->m_Hardware.Current - pStream->m_SpuBuffer); 
    // newoffset will be below 0 when we are initially starting the stream
    if (newoffset < 0 )
        newoffset = 0;

    // If we get below 0, this means the buffer wrapped.
    offset = newoffset - pStream->m_LastOffset;
    if (offset < 0)
    {
        offset+= AUD_STREAM_BUFFER_SIZE *2;
    }

    pStream->m_ActualPlayPosition += offset;
    if (pStream->m_LastOffset > newoffset)
    {
        pStream->m_pVoice->m_Hardware.Finished = TRUE;
    }
    else
    {
        pStream->m_pVoice->m_Hardware.Finished = FALSE;
    }
    pStream->m_LastOffset = newoffset;

    //
    // At the end of the update cycle, we go through to make sure we've enqueued all readahead
    // blocks.
    status = voicestream_UpdateReadahead(pStream);
    status = voicestream_FillBuffers(pStream);
    if (status)
    {
        pStream->m_Status = STREAMSTAT_STOPPING;
        return TRUE;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
void        voicestream_Rewind(voice_stream *pStream)
{
    pStream->m_ReadaheadPosition = 0;
    pStream->m_ReadaheadRemaining = BLOCK_COUNT(pStream->m_pOwner->m_Length);
}

//-----------------------------------------------------------------------------
xbool voicestream_UpdateReadahead(voice_stream *pStream)
{
    stream_request *pRequest;
    
    if ( (pStream->m_pOwner->m_pAttributes->m_Type == CFXTYPE_ELEMENT_HYBRID) ||
         (pStream->m_pOwner->m_pAttributes->m_Type == CFXTYPE_HOST_STREAM) )
    {
        return FALSE;
    }

    if (!pStream->m_ReadaheadRemaining)
    {
        ASSERT(pStream->m_pOwner);
        if (pStream->m_pOwner->m_Flags & AUDFLAG_LOOPED)
        {
            voicestream_Rewind(pStream);
        }
        else
        {
            return TRUE;        // The stream has finished REQUESTING to EOF.
        }
    }

    if ( (pStream->m_PendingReadahead >= (pStream->m_BufferCount/2) ) ||
         (pStream->m_ReadaheadRemaining < (pStream->m_BufferCount/2) ))
    {
        while ( pStream->m_PendingReadahead && pStream->m_ReadaheadRemaining)
        {
            pRequest = (stream_request *)mq_Recv(&g_Audio.m_qAvailableStreamRequests,MQ_NOBLOCK);
            ASSERT(pRequest);   // We *SHOULD* never run out of available requests
            if (!pRequest)      // But if we do on a release build, we can deal with that
                break;
            pRequest->m_FileId  = pStream->m_pOwner->m_pAttributes->m_MediaFile;
            pRequest->m_Offset  = pStream->m_pOwner->m_MediaLocation + pStream->m_ReadaheadPosition;
            pRequest->m_Length  = AUD_STREAM_BUFFER_SIZE;
            pRequest->m_pqReply = &pStream->m_qReadyData;
            mq_Send(&g_Audio.m_qPendingStreamRequests,pRequest,MQ_BLOCK);
            pStream->m_ReadaheadInProgress++;

            pStream->m_ReadaheadPosition += AUD_STREAM_BUFFER_SIZE;
            pStream->m_ReadaheadRemaining--;
            pStream->m_PendingReadahead--;
        }
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// Returns true if the buffer was updated,
// returns false if it was unable to do 
//-----------------------------------------------------------------------------
xbool voicestream_UpdateHybridBuffer(voice_stream *pStream,s32 BufferOffset,s32 remaining)
{
    xbool           Stereo;
    byte            *pData;

    ASSERT(pStream);
    ASSERT(pStream->m_pVoice);
    (void)remaining;

    Stereo = (pStream->m_pOwner->m_Flags & AUDFLAG_STEREO);

    ASSERT(!Stereo);

    pData = pStream->m_pOwner->m_pHybridBuffer + pStream->m_ReadaheadPosition;
    pStream->m_ReadaheadPosition+=AUD_STREAM_BUFFER_SIZE;

    if (BufferOffset)
    {
        pData[AUD_STREAM_BUFFER_SIZE-16+1] = VAG_LOOP_END;
        pData[1] = VAG_LOOP;
    }
    else
    {
        pData[1] = VAG_LOOP_START;
        pData[AUD_STREAM_BUFFER_SIZE-16+1] = VAG_LOOP;
    }

#ifdef DUMP_VAG
    write(testfile,pData,2048);
#endif
    spu_Transfer(pData,pStream->m_SpuBuffer+BufferOffset,AUD_STREAM_BUFFER_SIZE,SPUTRANS_WRITE);
    pStream->m_PlayRemaining--;
    //
    // Now, if we're just starting the stream, we make sure the buffers contain
    // valid data and then we actually start the voices playing. They should be
    // in perfect unison.
    //
    if (pStream->m_Status == STREAMSTAT_STARTING)
    {
        // just for now, let's transfer the same block in to the second half of the buffer space, This will force it
        // to loop back to the beginning of our buffer space. It really only comes in to play when we're debugging or
        // if, for some reason, the update cycle took way more than the amount of time for the hardware to consume the 
        // first of the streaming buffer.
        pData[1] = VAG_LOOP;
        pData[AUD_STREAM_BUFFER_SIZE-16+1] = VAG_LOOP_END;

        spu_Transfer(pData,pStream->m_SpuBuffer+AUD_STREAM_BUFFER_SIZE,AUD_STREAM_BUFFER_SIZE,SPUTRANS_WRITE);
        spu_InitVoice(pStream->m_pVoice->m_Id,pStream->m_SpuBuffer,pStream->m_pVoice->m_pOwner->m_pAttributes->m_ADSR1,pStream->m_pVoice->m_pOwner->m_pAttributes->m_ADSR2);
        spu_SetKey(pStream->m_pVoice->m_Id,1);

        pStream->m_Status = STREAMSTAT_PLAYING;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
// Returns true if the buffer was updated,
// returns false if it was unable to do 
//-----------------------------------------------------------------------------
xbool voicestream_UpdateBuffer(voice_stream *pStream,s32 BufferOffset,s32 remaining)
{
    stream_request *pRequest;
    stream_request *pSiblingRequest;
    xbool           Stereo;
    xbool           status;

    ASSERT(pStream);
    ASSERT(pStream->m_pVoice);

    Stereo = (pStream->m_pOwner->m_Flags & AUDFLAG_STEREO);

    if (!Stereo)
    {
        ASSERT(pStream->m_pSibling==NULL);
    }

    if ( (pStream->m_PlayRemaining==0) && (pStream->m_pVoice->m_Hardware.Finished) )
    {
        if (pStream->m_pOwner->m_Flags & AUDFLAG_LOOPED)
        {
            pStream->m_PlayRemaining = BLOCK_COUNT(pStream->m_pOwner->m_Length);
            pStream->m_Status = STREAMSTAT_STARTING;
            pStream->m_FirstHalfFull = FALSE;
        }
        else
        {
            return TRUE;
        }
    }

    // A hybrid stream, for now, will be totally RAM resident on the iop. We effectively do the same
    // thing as normal streaming from CD except that we just do the DMA transfer from RAM to SPU
    //
    if (pStream->m_pOwner->m_pAttributes->m_Type == CFXTYPE_ELEMENT_HYBRID)
    {
        return voicestream_UpdateHybridBuffer(pStream,BufferOffset,remaining);
    }
    //
    //
    pRequest = mq_Recv(&pStream->m_qReadyData,MQ_NOBLOCK);
    if (!pRequest)
    {
#ifdef X_DEBUG
//        printf("Block request delayed, offset=%d, remain=%d\n",pStream->m_PlayRemaining,remaining);
#endif
        return FALSE;
    }

    // The stereo channel has to be updated first just in case we cannot get
    // a sibling request which will allow us to push the data back in to the
    // buffer and return without any updates.
    if (Stereo)
    {
        ASSERT(pStream->m_pSibling);
        ASSERT(pStream->m_pSibling->m_pVoice);
        ASSERT(!pStream->m_pSibling->m_pSibling);

        pSiblingRequest = mq_Recv(&pStream->m_qReadyData,MQ_NOBLOCK);
        // If we can't get a stream block for the sibling, then let's just bail out and 
        // make sure the last block we got is at the head of the queue for next time round. 
        // This will make sure the stereo voices should stay aligned even if there is
        // a block delay.
        if (!pSiblingRequest)
        {
#ifdef X_DEBUG
            printf("Sibling block request delayed, offset=%d, remain=%d\n",pStream->m_PlayRemaining,remaining);
#endif
            status = mq_Send(&pStream->m_qReadyData,pRequest,MQ_NOBLOCK|MQ_JAM);
            ASSERT(status);
            return FALSE;
        }

        if (BufferOffset)
        {
            pSiblingRequest->m_pData[AUD_STREAM_BUFFER_SIZE-16+1] = VAG_LOOP_END;
        }
        else
        {
            pSiblingRequest->m_pData[1] = VAG_LOOP_START;
            pSiblingRequest->m_pData[1+16] = VAG_LOOP;
        }
        spu_Transfer(pSiblingRequest->m_pData,pStream->m_pSibling->m_SpuBuffer+BufferOffset,AUD_STREAM_BUFFER_SIZE,SPUTRANS_WRITE);
        pStream->m_PendingReadahead++;
        pStream->m_PlayRemaining--;
        pStream->m_ReadaheadInProgress--;
        mq_Send(&g_Audio.m_qAvailableStreamRequests,pSiblingRequest,MQ_BLOCK);
    }

    if (BufferOffset)
    {
        pRequest->m_pData[AUD_STREAM_BUFFER_SIZE-16+1] = VAG_LOOP_END;
    }
    else
    {
        pRequest->m_pData[1] = VAG_LOOP_START;
    }

#ifdef DUMP_VAG
    write(testfile,pRequest->m_pData,2048);
#endif
    spu_Transfer(pRequest->m_pData,pStream->m_SpuBuffer+BufferOffset,AUD_STREAM_BUFFER_SIZE,SPUTRANS_WRITE);
    pStream->m_PendingReadahead++;
    pStream->m_ReadaheadInProgress--;
    pStream->m_PlayRemaining--;
    //
    // Now, if we're just starting the stream, we make sure the buffers contain
    // valid data and then we actually start the voices playing. They should be
    // in perfect unison.
    //
    if (pStream->m_Status == STREAMSTAT_STARTING)
    {
        // just for now, let's transfer the same block in to the second half of the buffer space, This will force it
        // to loop back to the beginning of our buffer space. It really only comes in to play when we're debugging or
        // if, for some reason, the update cycle took way more than the amount of time for the hardware to consume the 
        // first of the streaming buffer.
        pRequest->m_pData[1] = VAG_LOOP;
        pRequest->m_pData[AUD_STREAM_BUFFER_SIZE-16+1] = VAG_LOOP_END;

        spu_Transfer(pRequest->m_pData,pStream->m_SpuBuffer+AUD_STREAM_BUFFER_SIZE,AUD_STREAM_BUFFER_SIZE,SPUTRANS_WRITE);
        spu_InitVoice(pStream->m_pVoice->m_Id,pStream->m_SpuBuffer,pStream->m_pOwner->m_pAttributes->m_ADSR1,pStream->m_pOwner->m_pAttributes->m_ADSR2);
        spu_SetKey(pStream->m_pVoice->m_Id,1);

        if (Stereo)
        {
            spu_Transfer(pRequest->m_pData,pStream->m_pSibling->m_SpuBuffer+AUD_STREAM_BUFFER_SIZE,AUD_STREAM_BUFFER_SIZE,SPUTRANS_WRITE);
            spu_InitVoice(pStream->m_pSibling->m_pVoice->m_Id,pStream->m_pSibling->m_SpuBuffer,pStream->m_pOwner->m_pAttributes->m_ADSR1,pStream->m_pOwner->m_pAttributes->m_ADSR2);
            spu_SetKey(pStream->m_pSibling->m_pVoice->m_Id,1);
        }
        pStream->m_Status = STREAMSTAT_PLAYING;
    }
    status = mq_Send(&g_Audio.m_qAvailableStreamRequests,pRequest,MQ_NOBLOCK);
    ASSERT(status);
    return TRUE;
}

//-----------------------------------------------------------------------------
xbool voicestream_FillBuffers(voice_stream *pStream)
{
    s32 offset;
    s32 status;

    ASSERT(pStream->m_pVoice);

    offset = pStream->m_pVoice->m_Hardware.Current - pStream->m_SpuBuffer;
    if (offset < 0)
    {
        offset = 0;
    }

    if ( pStream->m_PlayRemaining == 0)
    {
        if (pStream->m_pOwner->m_Flags & AUDFLAG_LOOPED)
        {
            pStream->m_Status = STREAMSTAT_STARTING;
            pStream->m_FirstHalfFull = FALSE;
            pStream->m_PlayRemaining = BLOCK_COUNT(pStream->m_pOwner->m_Length);
        }
        else
        {
            return pStream->m_pVoice->m_Hardware.Finished;
        }
    }

    if ( (offset >= AUD_STREAM_BUFFER_SIZE) || (pStream->m_Status == STREAMSTAT_STARTING) )
    {
        if (!pStream->m_FirstHalfFull)
        {
            status = voicestream_UpdateBuffer(pStream,0,(AUD_STREAM_BUFFER_SIZE * 2) - offset);
            if (status)
            {
                pStream->m_FirstHalfFull = TRUE;
            }
        }
    }
    else
    {
        if (pStream->m_FirstHalfFull)
        {
            status = voicestream_UpdateBuffer(pStream,AUD_STREAM_BUFFER_SIZE,AUD_STREAM_BUFFER_SIZE - offset);
            if (status)
            {
                pStream->m_FirstHalfFull = FALSE;
            }
        }
    }
    return FALSE;
}

