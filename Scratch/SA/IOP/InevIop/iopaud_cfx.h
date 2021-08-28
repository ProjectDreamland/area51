#ifndef __IOPAUD_CFX_H
#define __IOPAUD_CFX_H

#include "ioptypes.h"
#include "iopaud_defines.h"
#include "iopaud_host.h"
#include "iopaud_voice.h"
#include "iopaud_element.h"


//
// The actual volume/pitch will start at 0, increase at the attack rate until it
// reaches the sustain level. When the effect is killed or released from it's loop,
// the volume/pitch will decay at m_ReleaseRate until it hits or goes below 0.
//
typedef struct s_cfx_envelope
{
    s32         m_CurrentLevel;
    s32         m_AttackRate;
    s32         m_SustainLevel;
    s32         m_ReleaseRate;
} cfx_envelope;

//
// This is set up from cfx_header. All spawned effects have to be added in to the Children list
// This struct will not be freed until all children have expired and all cfx elements have been
// processed.
//
typedef struct s_cfx
{
    s32             m_Identifier;           // Unique ID assigned to this CFX (will be passed to EE for sound effect id)
    struct s_cfx   *m_pNext;                // Ptr to next cfx at this level (used in free list & at top level)
    s16             m_Count;                // Number of elements within this cfx remaining to be processed (or blocked due to previous element not completed)
    s16             m_ElementCount;         // Actual # that should be in the element
    cfxstat         m_Status;
    s16             m_Flags;                // Copy of flags in element that initiated this cfx
    cfx_state       m_State;                // Current cfx's state
    cfx_state       m_OriginalState;        // Original state pulled from the sample data
    cfx_element    *m_pElements;            // Ptr to the list of cfx elements (may NOT be the immediate following block!)
    cfx_element     m_Elements[CFX_ELEMENTS_CONTAINED];
} cfx;

typedef struct s_cfx_pool
{
    struct s_cfx_pool *m_pNext;         // Link to next additional chunk
    iop_message_queue   m_qFree;              // List of free entries
    s32             m_nEntries;
    cfx             m_CfxElements[1];
} cfx_pool;
void        cfx_Init(void);
void        cfx_Kill(void);

void        cfx_Free(cfx *pCfx);
cfx         *cfx_Alloc(s32 SampleId,cfx_state *pState);
cfx         *cfx_Find(s32 id);
// Returns false if this cfx is still alive, true if it is complete
xbool       cfx_Update(cfx *pCfx,cfx_state *pParentState,s32 DeltaTime);
void        cfx_UpdateState(cfx *pCfx,cfx_state *pState);
cfx         *cfx_AllocFromPool(void);
void        cfx_InsertForUpdate(cfx *pCfx);
void        cfx_RemoveFromUpdate(cfx *pCfx);

#endif // __IOPAUD_CFX_H