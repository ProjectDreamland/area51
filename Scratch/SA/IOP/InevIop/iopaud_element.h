#ifndef __IOPAUD_ELEMENT_H
#define __IOPAUD_ELEMENT_H

#include "iopaud_defines.h"
#include "iopaud_host.h"
#include "iopaud_stream.h"

#define CFX_ELEMENTS_CONTAINED      (2)         // Count of internal elements in the cfx struct
#define ATTRIBUTES_PER_BLOCK        (64)

// Cfx structure is designed to be totally heirarchical. A cfx can consist of a series of
// children which, in their own right, can be a cfx or a cfxelement. The cfxelement contains
// the actual sample to be played.

// This is a mirror of what is stored in the data file

struct s_audio_voice;
struct s_voice_stream;
struct s_cfx;

// To save space, we combine all of the fields which should be relatively unchanged between
// cfx's and store them in a read-only area. Even when the cfx is instantiated, none of these
// values should be changing
typedef struct s_cfx_element_attributes
{
    s16         m_Type;                 // CFXTYPE_*
    s16         m_Priority;             // Priority 0..100
    s16         m_Volume;               // Volume (10000=1.0)
    s16         m_Pitch;                // Pitch (10000=1.0)
    s16         m_Count;                // Number of cfx's attached to this one
    s16         m_Falloff;              // Falloff rate (10000=1.0), 1.0 = farclip, 0.0 = nearclip
    s16         m_Pan;                  // Pan value (-10000 = full left, 10000 = full right)
    s16         m_ADSR1;
    s16         m_ADSR2;
    u16         m_SampleRate;
    s32         m_OwnerId;              // Owning cfx id. (==0 if we're a top level cfx)
    s32         m_MediaFile;
} cfx_element_attributes;

typedef struct s_cfx_attrib_list
{
    struct s_cfx_attrib_list *  m_pNext;
    s16                         m_InUse;
    cfx_element_attributes      m_Attributes[ATTRIBUTES_PER_BLOCK];
} cfx_attrib_list;

typedef struct s_cfx_element
{
    cfx_element_attributes *m_pAttributes;
    cfxstat     m_Status;
    s16         m_SurroundDelay;
    s16         m_ExpireDelay;
    s16         m_Delay;                // Delay in milliseconds
    s16         m_Flags;                // AUDFLAG_LOOPED, AUDFLAG_STEREO is the only valid flag
    struct s_audio_voice *m_pVoice;
    struct s_audio_voice *m_pSurroundVoice;
    struct s_voice_stream *m_pStream;
    struct s_cfx *m_pOwner;
    s32         m_Length;
    s32         m_SpuLocation;          // Address within SPU
    s32         m_MediaLocation;        // Offset within file on the media
    s32         m_HybridPosition;       // How much of the hybrid data have we consumed so far?
    s32         m_HybridLength;         // Length of the hybrid data block
    void       *m_pHybridBuffer;        // Buffer to store the start of the hybrid data block
} cfx_element;

// 
// This is the data format used within the actual data file. This will be expanded out
// to the above structure when cfxelement_Init is called.

typedef struct s_cfx_stored_element
{
    s16         m_Type;                 // CFXTYPE_*
    s16         m_Flags;                // AUDFLAG_LOOPED, AUDFLAG_STEREO is the only valid flag
    s16         m_Priority;             // Priority 0..100
    s16         m_Volume;               // Volume (10000=1.0)
    s16         m_Pitch;                // Pitch (10000=1.0)
    s16         m_Delay;                // Delay in milliseconds
    s16         m_Count;                // Number of cfx's attached to this one
    s16         m_Falloff;              // Falloff rate (10000=1.0), 1.0 = farclip, 0.0 = nearclip
    s16         m_Pan;                  // Pan value (-10000 = full left, 10000 = full right)
    s16         m_ADSR1;
    s16         m_ADSR2;
    u16         m_SampleRate;
    s32         m_OwnerId;              // Owning cfx id. (==0 if we're a top level cfx)
    s32         m_Length;
    s32         m_MediaLocation;        // Set to the cfx id of the target cfx if this is complex
} cfx_stored_element;

void    cfxelement_Init(cfx_element *pElement);
void    cfxelement_Kill(cfx_element *pElement);
xbool   cfxelement_Update(cfx_element *pElement,cfx_state *pParentState,s32 DeltaTime);

#endif 