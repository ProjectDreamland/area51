#include "iopaud_cfx.h"
#include "iopaud_container.h"
#include "iopaudio.h"
#include "iopmain.h"
#include <kernel.h>
#include <sys/file.h>
#include <stdio.h>
#include "libcdvd.h"

#define ELEMENT_BUFFER_SIZE 128
void container_AsyncLoader(void);
static s32 s_ThreadId;
//-----------------------------------------------------------------------------
void container_Init(void)
{
    g_Audio.m_pContainers = NULL;
    mq_Create(&g_Audio.m_qLoadPending,1,"qLoadPending");
    s_ThreadId = iop_CreateThread(container_AsyncLoader,-1,2048,"container_AsyncLoader");
}


void container_Kill(void)
{
    iop_DestroyThread(s_ThreadId);
    mq_Destroy(&g_Audio.m_qLoadPending);
}


cfx_element_attributes *FindDuplicateAttribute(container    *pContainer,cfx_stored_element *pStoredElement)
{
    cfx_element_attributes  Attributes;
    cfx_attrib_list         *pList;
    cfx_element_attributes  *pEntry;
    s32                     i;

    Attributes.m_Type       = pStoredElement->m_Type;
    Attributes.m_Priority   = pStoredElement->m_Priority;
    Attributes.m_Volume     = pStoredElement->m_Volume;
    Attributes.m_Pan        = pStoredElement->m_Pan;
    Attributes.m_Pitch      = pStoredElement->m_Pitch;
    Attributes.m_Count      = pStoredElement->m_Count;
    Attributes.m_Falloff    = pStoredElement->m_Falloff;
    Attributes.m_ADSR1      = pStoredElement->m_ADSR1;
    Attributes.m_ADSR2      = pStoredElement->m_ADSR2;
    Attributes.m_SampleRate = pStoredElement->m_SampleRate;
    Attributes.m_OwnerId    = pStoredElement->m_OwnerId;
    Attributes.m_MediaFile  = pContainer->m_MediaFile;

    // We scan through our current list of attributes to see if we have any duplicates of the same data
    // First, we manicure what we have right now in to the form stored in the duplicates table.

    pList = pContainer->m_pAttributes;
    while (pList)
    {
        pEntry = pList->m_Attributes;
        for (i=0;i<pList->m_InUse;i++)
        {
            // If we do find an identical attrib entry, just return a pointer to that
            if (memcmp(pEntry,&Attributes,sizeof(Attributes))==0)
            {
                return pEntry;
            }
            pEntry++;
        }
        pList = pList->m_pNext;
    }
    // Here we did not find any dups, so let's create a new entry
    pList = pContainer->m_pAttributes;
    while (pList)
    {
        if (pList->m_InUse != ATTRIBUTES_PER_BLOCK)
        {
            pEntry = &pList->m_Attributes[pList->m_InUse];
            pList->m_InUse++;
            memcpy(pEntry,&Attributes,sizeof(Attributes));
            ASSERT((u8*)pEntry < ((u8*)pList+1800));
            return pEntry;
        }
        pList = pList->m_pNext;
    }
    // Here, we need to create a new block for duplicates
    pList = iop_Malloc(sizeof(cfx_attrib_list));
    ASSERT(pList);
    pList->m_pNext = pContainer->m_pAttributes;
    pList->m_InUse = 1;
    pList->m_pNext = NULL;
    pContainer->m_pAttributes = pList;
    memcpy(&pList->m_Attributes[0],&Attributes,sizeof(Attributes));
    return &pList->m_Attributes[0];
}

#if 0
void DumpAttributes(container *pContainer)
{
    cfx_attrib_list *pList;
    s32 i,index;

    cfx_element_attributes *pAttr;

    iop_DebugMsg("Attribute dump for %d\n",pContainer->m_Id);
    pList = pContainer->m_pAttributes;
    index = 0;
    while (pList)
    {
        pAttr = pList->m_Attributes;
        for (i=0;i<pList->m_InUse;i++)
        {
            iop_DebugMsg("Entry %d, Type:%d, Pri:%d, Vol:%d, Pitch:%d, Pan:%d, ADSR:%04x:%04x, Rate:%d\n",
                    index,pAttr->m_Type, pAttr->m_Priority,pAttr->m_Volume,pAttr->m_Pitch,pAttr->m_Pan,
                    pAttr->m_ADSR1,pAttr->m_ADSR2,pAttr->m_SampleRate);
            index++;
            pAttr++;
        }
        pList = pList->m_pNext;
    }
}
#endif

void container_AsyncLoader(void)
{

    container           *pContainer;
    container_header    Header;

    cfx_stored_element  *pStoredBuffer,*pStoredElement;
    cfx_element         *pElement;
    s32                 Handle;
    s32                 Length;
    s32                 spufree;
    s32                 i,Status;
    char                NameBuffer[64];
    s32                 CurrentMediaLocation;
    xbool               LoadingFromCD;
    sceCdlFILE          CDFileLocation;
    s32                 ContainerAllocations;
    cfx_element         *pOther;
    s32                 j;
    s32                 ElementsRemainingInBuffer;
    s32                 TotalElementCount;
    s32                 ElementFileIndex;

    while(1)
    {
        mq_Recv(&g_Audio.m_qLoadPending,MQ_BLOCK);

        iop_DebugMsg("AudioLoadContainer(%s)\n",g_Audio.m_LoadFilename);
        Handle = open(g_Audio.m_LoadFilename,O_RDONLY);
        ASSERT(Handle>=0);
        Length = lseek(Handle,0,SEEK_END);
        lseek(Handle,0,SEEK_SET);
        read(Handle,&Header,sizeof(container_header));
        ASSERT(memcmp(&Header.m_Signature,"inevaud",7)==0);
        ASSERT(Header.m_Version == CONTAINER_VERSION_NUMBER);
        ContainerAllocations = 0;

        if ( (g_Audio.m_LoadFilename[0]=='c') && (g_Audio.m_LoadFilename[1]=='d') &&
             (g_Audio.m_LoadFilename[2]=='r') && (g_Audio.m_LoadFilename[3]=='o') &&
             (g_Audio.m_LoadFilename[4]=='m') )
        {
            Status = sceCdSearchFile(&CDFileLocation,&g_Audio.m_LoadFilename[6]);
            LoadingFromCD = TRUE;
        }
        else
        {
            LoadingFromCD = FALSE;
        }

        //
        // Make sure we haven't loaded a container with the same ID yet. This would be BAD!
        //
        pContainer = g_Audio.m_pContainers;
        while (pContainer)
        {
            ASSERT(pContainer->m_Id != Header.m_ContainerID);
            pContainer= pContainer->m_pNext;
        }

        Length = sizeof(container)+Header.m_Count*sizeof(cfx_element);
        pContainer = iop_Malloc(Length);
        ContainerAllocations+=Length;
        ASSERT(pContainer);
        pStoredBuffer = iop_Malloc(sizeof(cfx_stored_element) * ELEMENT_BUFFER_SIZE);
        ASSERT(pStoredBuffer);

        if (LoadingFromCD)
            pContainer->m_MediaFile = (1<<31)|CDFileLocation.lsn;
        else
            pContainer->m_MediaFile = (s32)Handle;
        pContainer->m_Id            = Header.m_ContainerID<<24;
        pContainer->m_Count         = Header.m_Count;
        pContainer->m_MemoryBase    = spu_Memtop();
        pContainer->m_pAttributes   = NULL;
        pContainer->m_Volume        = AUD_FIXED_POINT_1;
    

        pElement = pContainer->m_Elements;
        pStoredElement = pStoredBuffer;

        spufree = spu_MemFree();
        CurrentMediaLocation = 0;
        // Although all of this stuff can be done in the loop below, this is being done
        // first so we can discard the pStoredBuffer block of memory to help alleviate
        // the problems caused by memory fragmentation. It *should* mean that all memory
        // allocations for the resident IOP Hybrid streams should be contiguous.
        TotalElementCount = Header.m_Count;
        ElementsRemainingInBuffer = 0;
        ElementFileIndex = lseek(Handle,0,SEEK_CUR);

        for (i=0;i<Header.m_Count;i++)
        {
            if (ElementsRemainingInBuffer==0)
            {
                s32 Count;
            
                Count = TotalElementCount;
                if (Count > 128)
                    Count = 128;
                lseek(Handle,ElementFileIndex,SEEK_SET);
                read(Handle,pStoredBuffer,ELEMENT_BUFFER_SIZE * sizeof(cfx_stored_element) );
                pStoredElement = pStoredBuffer;
                TotalElementCount-= Count;
                ElementFileIndex = lseek(Handle,0,SEEK_CUR);
                ElementsRemainingInBuffer = Count;
            }
            // All of the items loaded in the package need to be split into 2 sections. One, read only
            // and the other read-write. The read-only section should contain data that varies as little
            // as possible from sample to sample. This allows us to save significant amounts of space by
            // eliminating duplicates. All other either modified or unique fields should be stored in the
            // actual element structure.
            pElement->m_pAttributes     = FindDuplicateAttribute(pContainer,pStoredElement);

            pElement->m_MediaLocation   = pStoredElement->m_MediaLocation;
            pElement->m_HybridLength    = 0;
            pElement->m_HybridPosition  = 0;
            pElement->m_Status          = CFXSTAT_IDLE;
            pElement->m_pVoice          = NULL;
            pElement->m_pOwner          = NULL;
            pElement->m_Length          = pStoredElement->m_Length;
            pElement->m_Flags           = pStoredElement->m_Flags;
            pElement->m_Delay           = pStoredElement->m_Delay;
            pElement++;
            pStoredElement++;
            ElementsRemainingInBuffer--;
        }
        iop_Free(pStoredBuffer);

        // Parse elements, allocate memory as required (if hybrid mainly)
        pElement = pContainer->m_Elements;
        for (i=0;i<Header.m_Count;i++)
        {
            byte *pBuffer;
            s32 AlignedLength;

            switch (pElement->m_pAttributes->m_Type)
            {
            case CFXTYPE_ELEMENT:
                pElement->m_MediaLocation += Header.m_OffsetToSamples;
                AlignedLength = (pElement->m_Length + Header.m_Alignment - 1) & ~(Header.m_Alignment-1);
                //
                // Has this media point already been loaded? If so, find it
                //
                pOther = pContainer->m_Elements;
                if (pElement->m_MediaLocation < CurrentMediaLocation)
                {
                    for (j=0;j<i;j++)
                    {
                        if (pOther->m_MediaLocation == pElement->m_MediaLocation)
                        {
                            ASSERT(pElement->m_pAttributes->m_Type == pOther->m_pAttributes->m_Type);
                            break;
                        }
                        pOther++;
                    }
                    ASSERT(j!=i);
                    pElement->m_SpuLocation = pContainer->m_Elements[j].m_SpuLocation;
                }
                else
                {

                    pBuffer = iop_Malloc(AlignedLength);
                    ASSERT(pBuffer);
                    lseek(Handle,pElement->m_MediaLocation,SEEK_SET);
                    sprintf(NameBuffer,"CFXTYPE_ELEMENT(media=%d)",pElement->m_MediaLocation);
                    pElement->m_SpuLocation = spu_Alloc(AlignedLength,NameBuffer);
                    ASSERT(pElement->m_SpuLocation);
                    read(Handle,pBuffer,AlignedLength);
                    spu_Transfer(pBuffer,pElement->m_SpuLocation,pElement->m_Length,SPUTRANS_WRITE);
                    iop_Free(pBuffer);
                    CurrentMediaLocation = pElement->m_MediaLocation + pElement->m_Length;
                }
                break;
            case CFXTYPE_ELEMENT_STREAM:
                pElement->m_MediaLocation += Header.m_OffsetToSamples;
                break;
            case CFXTYPE_ELEMENT_HYBRID:
                pElement->m_MediaLocation += Header.m_OffsetToSamples;
                pOther = pContainer->m_Elements;
                for (j=0;j<i;j++)
                {
                    if (pElement->m_MediaLocation == pOther->m_MediaLocation)
                    {
                        ASSERT(pElement->m_pAttributes->m_Type == pOther->m_pAttributes->m_Type);
                        break;
                    }
                    pOther++;
                }
                pElement->m_HybridLength = (pElement->m_Length+4095)&~4095;

                if (j != i)
                {
                    pElement->m_pHybridBuffer = pOther->m_pHybridBuffer;
                }
                else
                {
                    pElement->m_pHybridBuffer = iop_Malloc(pElement->m_HybridLength);
                    ContainerAllocations += pElement->m_HybridLength;
                    ASSERT(pElement->m_pHybridBuffer);
                    memset(pElement->m_pHybridBuffer,0,pElement->m_HybridLength);
                    lseek(Handle,pElement->m_MediaLocation,SEEK_SET);
                    read(Handle,pElement->m_pHybridBuffer,pElement->m_Length);
                }
                break;
            case CFXTYPE_COMPLEX:
                break;
			default:
				ASSERT(FALSE);
            }

            pElement++;
        }

        pContainer->m_MemoryTop = spu_Memtop();

        // Although we should lock around this call since we're modifying a container list
        // that may be accessed by other threads, we're not bothered since we'll only ever
        // be adding to the start of the list.
        pContainer->m_pNext = g_Audio.m_pContainers;
        g_Audio.m_pContainers = pContainer;
        {
            cfx_attrib_list *pAttribs;
            pAttribs = pContainer->m_pAttributes;
            while (pAttribs)
            {
                ContainerAllocations += sizeof(cfx_attrib_list);
                pAttribs = pAttribs->m_pNext;
            }
        }

        Length = lseek(Handle,0,SEEK_END);
        iop_DebugMsg("Container '%s' id #%d loaded. \n",g_Audio.m_LoadFilename,Header.m_ContainerID);
        iop_DebugMsg("     %d bytes long, %d samples\n",Length,Header.m_Count);
        iop_DebugMsg("     %d bytes used on SPU, %d free\n",spufree-spu_MemFree(),spu_MemFree());
        iop_DebugMsg("     %d bytes used on IOP, %d free, %d largest.\n",ContainerAllocations,iop_MemFree(),iop_LargestFree());

    //    DumpAttributes(pContainer);
                
        if (LoadingFromCD)
        {
            close(Handle);
        }

        // We don't bother closing the file since it contains hybrid and streamed samples
        //    close(Handle);
        g_Audio.m_LoadReply.Id          = pContainer->m_Id;
        g_Audio.m_LoadReply.nCfxCount   = pContainer->m_Count;
        g_Audio.m_LoadReply.Status      = 0;
    }
}

//-----------------------------------------------------------------------------
container_reply *container_Load(char *pName)
{

    // Spawn a thread to actually perform the load operation and start
    // it loading
    strcpy(g_Audio.m_LoadFilename,pName);
    mq_Send(&g_Audio.m_qLoadPending,NULL,MQ_BLOCK);
    g_Audio.m_LoadReply.Status       = 1;
    return &g_Audio.m_LoadReply;
}

container_reply *container_IsLoadComplete(void)
{
    return &g_Audio.m_LoadReply;
}

//-----------------------------------------------------------------------------
s32 container_Unload(s32 containerid)
{
    container *pContainer,*pLastContainer;
    s32 i;
    cfx_element *pElement;
    cfx_attrib_list         *pList,*pNext;

    pContainer = g_Audio.m_pContainers;

    pLastContainer = NULL;

    while (pContainer)
    {
        if (pContainer->m_Id == containerid)
        {
            break;
        }
        pLastContainer = pContainer;
        pContainer = pContainer->m_pNext;
    }

    if (!pContainer)
        return 0;
    //
    // Check to make sure this was the last container to use any memory within the
    // spu. Containers must be unloaded in reverse order.
    //

    if (pContainer->m_MemoryTop != spu_Memtop() )
    {
        iop_DebugMsg("container_Unload: Attempt to unload a container that has data after it.\n");
        return 0;
    }
    // 
    // Remove the container from the list
    //
    if (pLastContainer)
    {
        pLastContainer->m_pNext = pContainer->m_pNext;
    }
    else
    {
        g_Audio.m_pContainers = pContainer->m_pNext;
    }
    //
    // Free any spu memory used by this container
    //
    spu_Free(pContainer->m_MemoryBase);

    //
    // Now release all other memory used by this container (iop buffers)
    //
    pElement = pContainer->m_Elements;

    for (i=0;i<pContainer->m_Count;i++)
    {
        if (pElement->m_pAttributes->m_Type == CFXTYPE_ELEMENT_HYBRID)
        {
            iop_Free(pElement->m_pHybridBuffer);
        }
        pElement++;
    }


    //
    // Free up any other open resources on this container
    //
    if ( (pContainer->m_MediaFile & (1<<31))==0 )
    {
        close(pContainer->m_MediaFile);
    }

    pList = pContainer->m_pAttributes;
    while (pList)
    {
        pNext = pList->m_pNext;
        iop_Free(pList);
        pList = pNext;
    }
    iop_Free(pContainer);
    return TRUE;
}

container *FindContainer(s32 ContainerId)
{
    container *pContainer;

    pContainer = g_Audio.m_pContainers;
    while (pContainer)
    {
        if (pContainer->m_Id == ContainerId)
            break;
        pContainer = pContainer->m_pNext;
    }
    return pContainer;
}
//-----------------------------------------------------------------------------
cfx_element *container_Find(s32 identifier)
{
    s32 ContainerId;
    container *pContainer;

    ContainerId = identifier & (255<<24);
    identifier = identifier & ((1<<24)-1);

    pContainer = FindContainer(ContainerId);
    ASSERT(pContainer);
    ASSERT(identifier < pContainer->m_Count);
    return &pContainer->m_Elements[identifier];

}

//-----------------------------------------------------------------------------
s32 container_GetVolume(s32 ContainerId)
{
    static s32 LastContainerId=-1;
    static container *pContainer=NULL;

    // ***SHORTCUT***, if we're looking for the same container as the
    // last time this routine was called, let's just pass back what we
    // had then.
    if (ContainerId != LastContainerId)
    {
        pContainer = FindContainer(ContainerId);
    }
    ASSERT(pContainer);
    LastContainerId = ContainerId;
    return pContainer->m_Volume;
}

//-----------------------------------------------------------------------------
void container_SetVolume(s32 ContainerId,s32 Volume)
{
    container *pContainer;

    pContainer = FindContainer(ContainerId);
    ASSERT(pContainer);
    pContainer->m_Volume = Volume;
}

//-----------------------------------------------------------------------------
cfx_state *container_Header(container_hdr_request *pRequest)
{
    container *pContainer;
    s32 count,i;
    cfx_element *pElement;
    cfx_info    *pOut;

    pContainer = g_Audio.m_pContainers;

    while (pContainer)
    {
        if (pContainer->m_Id == pRequest->Id)
            break;
        pContainer = pContainer->m_pNext;
    }
    ASSERT(pContainer);
    // We cannot use the pHeader block since it's just about to be
    // overwritten by the actual cfx_state structures for this
    // container.
    count = pRequest->Count;
    pElement = &pContainer->m_Elements[pRequest->Index];
    pOut = (cfx_info *)pRequest;

    for (i=0;i<count;i++)
    {
        ASSERT(pElement->m_pAttributes);
        pOut->m_Pan     = pElement->m_pAttributes->m_Pan;
        pOut->m_Pitch   = pElement->m_pAttributes->m_Pitch;
        pOut->m_Volume  = pElement->m_pAttributes->m_Volume;
        pOut->m_Falloff = pElement->m_pAttributes->m_Falloff;
        pOut++;
        pElement++;
    }

    return (cfx_state *)pRequest;
}


