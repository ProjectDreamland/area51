#ifndef __IOPAUD_STREAM_H
#define __IOPAUD_STREAM_H

#define VOICE_STREAM_ALIGNMENT    16

#include "iopmqueue.h"
#include "iopaud_defines.h"
#include "iopaud_cfx.h"

struct s_audio_voice;

typedef struct s_voice_stream
{
    streamstat              m_Status;
    s16                     m_PendingReadahead;     // Number of blocks we are allowed to readahead
    s16                     m_BufferCount;          // Number of total buffers allocated to this stream
    xbool                   m_FirstHalfFull;        // Is the first buffer half full?
    s32                     m_ActualPlayPosition;   // Consumed data by hardware
    s32                     m_LastOffset;           // Last offset within buffer space
    s32                     m_ReadaheadRemaining;   // # blocks left to readahead
    s32                     m_PlayRemaining;        // # blocks left to play
    s32                     m_ReadaheadInProgress;  // # requests currently sent to the streamer
    s32                     m_ReadaheadPosition;    // Offset from the beginning of the sample 
    struct s_cfx_element    *m_pOwner;
    struct s_audio_voice    *m_pVoice;
    struct s_voice_stream   *m_pSibling;            // used only when a stereo sample is playing
    s32                     m_SpuBuffer;            // Buffer in spu memory for this stream
    iop_message_queue       m_qReadyData;
} voice_stream;

typedef struct s_stream_request
{
    s32             m_FileId;
    s32             m_Offset;
    s32             m_Length;
    byte            *m_pData;
    iop_message_queue   *m_pqReply;
} stream_request;

struct s_cfx_element;

void        voicestream_Init(s32 nBuffers,s32 size);
void        voicestream_Kill(void);
voice_stream *voicestream_Alloc(struct s_cfx_element *pElement);
xbool       voicestream_Free(voice_stream *pStream);
// Returns true if it needs a new block of data, false otherwise
xbool       voicestream_Update(voice_stream *pStream,cfx_state *pState,s32 DeltaTime);
void        voicestream_Rewind(voice_stream *pStream);
#endif