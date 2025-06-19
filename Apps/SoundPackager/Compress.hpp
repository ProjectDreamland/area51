#ifndef __COMPRESS_HPP
#define __COMPRESS_HPP

#include "x_files.hpp"
#include "PackageTypes.hpp"
#include "audio_file.hpp"

u32     CompressAudioFilePC_PCM     ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );
u32     CompressAudioFilePC_ADPCM   ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );
u32     CompressAudioFilePC_MP3     ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );

u32     CompressAudioFileGCN_ADPCM  ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );
u32     CompressAudioFileGCN_MP3    ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );

u32     CompressAudioFilePS2_ADPCM  ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );

u32     CompressAudioFileXBOX_PCM   ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );
u32     CompressAudioFileXBOX_ADPCM ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize );

xbool   CompressAudioFiles          ( void );
xbool   ProcessMultiChannelAudio    ( void );
xbool   LoadTrueMultiChannelFile    ( file_info& File, multi_channel_data* Data, xbool LoadWaveform, s32 Target );
void    DumpSamples                 ( void );

#endif