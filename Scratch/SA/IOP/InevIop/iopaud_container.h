#ifndef __IOPAUD_CONTAINER
#define __IOPAUD_CONTAINER

#include "ioptypes.h"
#include "iopaud_defines.h"
#include "iopaud_cfx.h"

#define CONTAINER_VERSION_NUMBER 5

typedef struct s_container
{
    struct s_container *m_pNext;
    s32             m_Id;
    s32             m_Count;            // Number of samples within this container
    s32             m_MediaFile;        // File ptr used for streaming
    s32             m_MemoryBase;       // Base address of spu memory used by this container
    s32             m_MemoryTop;        // End address of spu memory used by this container
    s32             m_Volume;           // Base volume for this container.
    cfx_attrib_list *m_pAttributes;
    cfx_element     m_Elements[0];      // This MUST be the last field within the struct since it's dynamically sized
} container;



typedef struct s_container_header
{
    char    m_Signature[7];             // 'inevaud'
    u8      m_Version;                  // See above CONTAINER_VERSION_NUMBER
    u8      m_ContainerID;
    s32     m_Count;                    // # samples
    s32     m_Length;                   // Length of container data
    s32     m_Alignment;                // # byte alignment (16)
    s32     m_OffsetToSamples;          // Actual base offset within file of the sample data
} container_header;

void                container_Init(void);
void                container_Kill(void);

container_reply*    container_Load(char *pName);
s32                 container_Unload(s32 ContainerId);
cfx_element*        container_Find(s32 ElementId);
cfx_state*          container_Header(container_hdr_request *pRequest);
void                container_SetVolume(s32 ContainerId,s32 Volume);
s32                 container_GetVolume(s32 identifier);
container_reply*    container_IsLoadComplete(void);

#endif