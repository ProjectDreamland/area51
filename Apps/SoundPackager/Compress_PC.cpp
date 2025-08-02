#include "Compress.hpp"
#include "SoundPackager.hpp"
#include "ExportPackage.hpp"
#include "Endian.hpp"
#include "..\3rdparty\miles6\include\mss.h"
#undef WAVE_FORMAT_IMA_ADPCM // CJ: Because mss.h defines this by hand, bad miles!
#include "imaadpcm.h"
#include "lame.h"

//#define WRITE_MP3_FILE

//------------------------------------------------------------------------------

u32 CompressAudioFilePC_PCM( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    audio_file* AudioFile = audio_file::Create(in);
    s32         TotalCompressedSize = 0;

    if( AudioFile->Open( in ) )
    {
        xarray<audio_file::breakpoint> BreakPoints;
        s32  SampleRate         = AudioFile->GetSampleRate();
        s32  nChannels          = AudioFile->GetNumChannels();
        s32  nSamples           = AudioFile->GetNumSamples();
        s32  LoopStart          = AudioFile->GetLoopStart();
        s32  LoopEnd            = AudioFile->GetLoopEnd();
        s16* pSampleBuffer      = (s16*)x_malloc( sizeof(s16) * nSamples );
        s32  HeaderSize         = 4 * sizeof(s32);
        s32  CompressionMethod;
        s32  i;
        s32  CompressedSize;
        s32  BreakPointSize;
        s32  nBreakPoints;

        xarray<void*> pCompressedBuffer;

        // Get the breakpoints.
        AudioFile->GetBreakpoints( BreakPoints );

        // Currently only support mono and stereo.
        ASSERT( (nChannels == 1) || (nChannels == 2) );

        // Set the number of channels.
        *NumChannels = nChannels;

        if( s_Verbose)
        {
            if( nChannels == 1 )
            {
                x_printf( "(PCM mono)\n" );
                x_DebugMsg( "(PCM mono)\n" );
            }
            else
            {
                x_printf( "(PCM stereo)\n" );
                x_DebugMsg( "(PCM stereo)\n" );
            }
        }

        // Multi-channel sound?
        if( nChannels > 1 )
        {
            CompressionMethod = PC_PCM_NON_INTERLEAVED_STEREO;
        }
        else
        {
            CompressionMethod = PC_PCM_MONO;
        }

        // Write out compression method.
        s32 pc_CompressionMethod = LITTLE_ENDIAN( CompressionMethod );
        x_fwrite( &pc_CompressionMethod, sizeof(s32), 1, out );

        // Write out the compressed size.
        CompressedSize = nSamples * sizeof(s16);
        s32 pc_CompressedSize = CompressedSize;
        pc_CompressedSize = LITTLE_ENDIAN( pc_CompressedSize );
        x_fwrite( &pc_CompressedSize, sizeof(s32), 1, out );

        // Write out the lip sync size in bytes
        *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
        s32 pc_LipSyncSize = *LipSyncSize;
        pc_LipSyncSize = LITTLE_ENDIAN( pc_LipSyncSize );
        x_fwrite( &pc_LipSyncSize, sizeof(s32), 1, out );

        // Write out the break point size in bytes.
        BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
        s32 pc_BreakPointSize = BreakPointSize;
        pc_BreakPointSize = LITTLE_ENDIAN( pc_BreakPointSize );
        x_fwrite( &pc_BreakPointSize, sizeof(s32), 1, out );

        // Write out header size in bytes.
        s32 pc_HeaderSize = LITTLE_ENDIAN( HeaderSize );
        x_fwrite( &pc_HeaderSize, sizeof(s32), 1, out );

        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {
            // Allocate space for the compressed data.
            pCompressedBuffer.Append() = x_malloc( CompressedSize );

            // Read the the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBuffer, i );

            // Copy the sample, no compression necessary
            x_memcpy( pCompressedBuffer[i], pSampleBuffer, nSamples * sizeof(s16) );

            // Write out the number of samples, sample rate and loop points
            s32 pc_nSamples     = LITTLE_ENDIAN( nSamples );
            s32 pc_SampleRate   = LITTLE_ENDIAN( SampleRate );
            s32 pc_LoopStart    = LITTLE_ENDIAN( LoopStart );
            s32 pc_LoopEnd      = LITTLE_ENDIAN( LoopEnd );
            x_fwrite( &pc_nSamples,     sizeof(s32), 1, out );
            x_fwrite( &pc_SampleRate,   sizeof(s32), 1, out );
            x_fwrite( &pc_LoopStart,    sizeof(s32), 1, out );
            x_fwrite( &pc_LoopEnd,      sizeof(s32), 1, out );
        }

        // Free buffer.
        x_free( pSampleBuffer );

        // Now write out each channels waveform data.
        for( i=0 ; i<nChannels ; i++ )
        {
            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            x_fwrite( pCompressedBuffer[i], CompressedSize, 1, out );
            
            // Free the compressed buffer memory.
            x_free( pCompressedBuffer[i] );
        }

        // Write out the lip sync data.
        WriteLipSyncData( AudioFile, out );

        // Write out the break points.
        WriteBreakPoints( BreakPoints, out, FALSE );
        
        // Close and delete audio file
        AudioFile->Close();
        delete AudioFile;
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------

u32 CompressAudioFilePC_ADPCM ( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    audio_file* AudioFile = audio_file::Create(in);
    s32         TotalCompressedSize = 0;

    if( AudioFile->Open( in ) )
    {
        xarray<audio_file::breakpoint> BreakPoints;
        s32  SampleRate         = AudioFile->GetSampleRate();
        s32  nChannels          = AudioFile->GetNumChannels();
        s32  nSamples           = AudioFile->GetNumSamples();
        s32  LoopStart          = AudioFile->GetLoopStart();
        s32  LoopEnd            = AudioFile->GetLoopEnd();
        s16* pSampleBuffer      = NULL;
        s32  HeaderSize         = 4 * sizeof(s32);
        s32  CompressionMethod;
        s32  i;
        U32  CompressedSize;
        s32  BreakPointSize;
        s32  nBreakPoints;

        xarray<void*> pCompressedBuffer;

        // Currently only support mono and stereo.
        ASSERT( (nChannels == 1) || (nChannels == 2) );

        // Get the breakpoints.
        AudioFile->GetBreakpoints( BreakPoints );

        // Set the number of channels.
        *NumChannels = nChannels;

        if( s_Verbose)
        {
            if( nChannels == 1 )
            {
                x_printf( "(ADPCM mono)\n" );
                x_DebugMsg( "(ADPCM mono)\n" );
            }
            else
            {
                x_printf( "(ADPCM stereo)\n" );
                x_DebugMsg( "(ADPCM stereo)\n" );
            }
        }

        // Multi-channel sound?
        if( nChannels > 1 )
        {
            CompressionMethod = PC_ADPCM_NON_INTERLEAVED_STEREO;
        }
        else
        {
            CompressionMethod = PC_ADPCM_MONO;
        }

        // Write out compression method.
        x_fwrite( &CompressionMethod, sizeof(s32), 1, out );

        // Allocate a buffer for the source data and clear it
        pSampleBuffer = (s16*)x_malloc( sizeof(s16) * nSamples );
        x_memset( pSampleBuffer, 0, sizeof(s16) * nSamples );

        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {
            // Read the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBuffer, i );

            // Compress and save
            AILSOUNDINFO info;
            info.format = WAVE_FORMAT_PCM;
            info.data_ptr = pSampleBuffer;
            info.data_len = nSamples * sizeof(s16);
            info.rate = SampleRate;
            info.bits = 16;
            info.channels = 1;
            info.samples = nSamples;
            info.block_size = 36;
            void* pCompressedData;
            AIL_compress_ADPCM( &info, &pCompressedData, &CompressedSize );
            pCompressedBuffer.Append() = pCompressedData;

            if( AudioFile->IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            // If this is the first channel then write out the header info
            if( i == 0 )
            {
                // Write out the compressed size.
                x_fwrite( &CompressedSize, sizeof(s32), 1, out );

                // Write out the lip sync size in bytes.
                *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
                x_fwrite( LipSyncSize, sizeof(s32), 1, out );

                // Write out the break point size in byte.
                BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
                x_fwrite( &BreakPointSize, sizeof(s32), 1, out );

                // Write out header size in bytes.
                x_fwrite( &HeaderSize, sizeof(s32), 1, out );
            }

            // Write out the number of samples, sample rate and loop points
            x_fwrite( &nSamples,     sizeof(s32), 1, out );
            x_fwrite( &SampleRate,   sizeof(s32), 1, out );
            x_fwrite( &LoopStart,    sizeof(s32), 1, out );
            x_fwrite( &LoopEnd,      sizeof(s32), 1, out );
        }

        // Free buffer.
        x_free( pSampleBuffer );

        // Now write out each channels waveform data.
        for( i=0 ; i<nChannels ; i++ )
        {
            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            x_fwrite( pCompressedBuffer[i], CompressedSize, 1, out );

            // Free the compressed buffer memory.
            AIL_mem_free_lock( pCompressedBuffer[i] );
        }

        // Write out the lip sync data.
        WriteLipSyncData( AudioFile, out );

        // Write out the break points.
        WriteBreakPoints( BreakPoints, out, FALSE );
        
        // Close and delete audio file
        AudioFile->Close();
        delete AudioFile;
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------

u32 CompressAudioFilePC_MP3( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    audio_file* AudioFile = audio_file::Create(in);
    s32         TotalCompressedSize = 0;

    if( AudioFile->Open( in ) )
    {
        xarray<audio_file::breakpoint> BreakPoints;
        s32  SampleRate         = AudioFile->GetSampleRate();
        s32  nChannels          = AudioFile->GetNumChannels();
        s32  nSamples           = AudioFile->GetNumSamples();
        s32  LoopStart          = AudioFile->GetLoopStart();
        s32  LoopEnd            = AudioFile->GetLoopEnd();
        s16* pSampleBuffer      = NULL;
        s16* pSampleBufferL     = NULL;
        s16* pSampleBufferR     = NULL;
        s32  HeaderSize         = 4 * sizeof(s32);
        s32  CompressionMethod;
        s32  i;
        s32  BreakPointSize;
        s32  nBreakPoints;

        void* pCompressedBuffer;
        s32   CompressedSize;

        // Get the breakpoints.
        AudioFile->GetBreakpoints( BreakPoints );

        // Set the number of channels.
        *NumChannels = nChannels;

        if( s_Verbose)
        {
            if( nChannels == 1 )
            {
                x_printf( "(MP3 mono)\n" );
                x_DebugMsg( "(MP3 mono)\n" );
            }
            else
            {
                x_printf( "(MP3 stereo)\n" );
                x_DebugMsg( "(MP3 stereo)\n" );
            }
        }

        // Multi-channel sound?
        if( nChannels == 1 )
        {
            // Set it.
            CompressionMethod = PC_MP3_MONO;

            // Write out compression method.
            x_fwrite( &CompressionMethod, sizeof(s32), 1, out );

            // Allocate a buffer for the source data and clear it
            pSampleBuffer = (s16*)x_malloc( sizeof(s16) * nSamples );
            x_memset( pSampleBuffer, 0, sizeof(s16) * nSamples );

            // Read the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBuffer, 0 );

            // Allocate buffer for compressed data
            s32 CompressedDataSize = (s32)(nSamples * 1.25f + 7200);
            u8* pCompressedData = (u8*)x_malloc( CompressedDataSize );
            ASSERT( pCompressedData );
            pCompressedBuffer = pCompressedData;

            // Compress to MP3
            lame_global_flags* gfp = lame_init();
            lame_set_num_samples        ( gfp, nSamples );
            lame_set_num_channels       ( gfp, 1 );
            lame_set_in_samplerate      ( gfp, SampleRate );
            lame_set_out_samplerate     ( gfp, SampleRate );
            lame_set_quality            ( gfp, 5 );             // TODO: Set this from parameters
            lame_set_mode               ( gfp, MONO );
            lame_set_compression_ratio  ( gfp, 11.0f );
            s32 Inited = lame_init_params( gfp );
            ASSERT( Inited != -1 );

            s32 Size = lame_encode_buffer( gfp, pSampleBuffer, NULL, nSamples, pCompressedData, CompressedDataSize );
            ASSERT( Size >= 0 );
            Size += lame_encode_finish( gfp, pCompressedData+Size, CompressedDataSize-Size );

            CompressedSize = Size;

            if( AudioFile->IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            // Write out the compressed size.
            x_fwrite( &CompressedSize, sizeof(s32), 1, out );

            // Write out the lip sync size in bytes.
            *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
            x_fwrite( LipSyncSize, sizeof(s32), 1, out );

            // Write out the break point size in byte.
            BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
            x_fwrite( &BreakPointSize, sizeof(s32), 1, out );

            // Write out header size in bytes.
            x_fwrite( &HeaderSize, sizeof(s32), 1, out );

            // Write out the number of samples, sample rate and loop points
            x_fwrite( &nSamples,     sizeof(s32), 1, out );
            x_fwrite( &SampleRate,   sizeof(s32), 1, out );
            x_fwrite( &LoopStart,    sizeof(s32), 1, out );
            x_fwrite( &LoopEnd,      sizeof(s32), 1, out );

            // Free buffer.
            x_free( pSampleBuffer );

            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            x_fwrite( pCompressedBuffer, CompressedSize, 1, out );

            // Free the compressed buffer memory.
            x_free( pCompressedBuffer );
        }
        else if( nChannels == 2 )
        {
            // Set it.
            CompressionMethod = PC_MP3_INTERLEAVED_STEREO;

            // Write out compression method.
            x_fwrite( &CompressionMethod, sizeof(s32), 1, out );

            // Allocate a buffer for the source data and clear it
            pSampleBufferL = (s16*)x_malloc( sizeof(s16) * nSamples );
            x_memset( pSampleBufferL, 0, sizeof(s16) * nSamples );
            pSampleBufferR = (s16*)x_malloc( sizeof(s16) * nSamples );
            x_memset( pSampleBufferR, 0, sizeof(s16) * nSamples );

            // Read the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBufferL, 0 );
            AudioFile->GetChannelData( pSampleBufferR, 1 );

            // Allocate buffer for compressed data
            s32 CompressedDataSize = (s32)(nSamples * 1.25f + 7200);
            u8* pCompressedData = (u8*)x_malloc( CompressedDataSize );
            ASSERT( pCompressedData );
            pCompressedBuffer = pCompressedData;

            // Compress to MP3
            lame_global_flags* gfp = lame_init();
            lame_set_num_samples        ( gfp, nSamples );
            lame_set_num_channels       ( gfp, 2 );
            lame_set_in_samplerate      ( gfp, SampleRate );
            lame_set_out_samplerate     ( gfp, SampleRate );
            lame_set_quality            ( gfp, 5 );             // TODO: Set this from parameters
            lame_set_mode               ( gfp, STEREO );
            lame_set_compression_ratio  ( gfp, 11.0f );
            s32 Inited = lame_init_params( gfp );
            ASSERT( Inited != -1 );

            s32 Size = lame_encode_buffer( gfp, pSampleBufferL, pSampleBufferR, nSamples, pCompressedData, CompressedDataSize );
            ASSERT( Size >= 0 );
            Size += lame_encode_finish( gfp, pCompressedData+Size, CompressedDataSize-Size );

            CompressedSize = Size;

            if( AudioFile->IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            // Write out the compressed size.
            x_fwrite( &CompressedSize, sizeof(s32), 1, out );

            // Write out the lip sync size in bytes.
            *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
            x_fwrite( LipSyncSize, sizeof(s32), 1, out );

            // Write out the break point size in byte.
            BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
            x_fwrite( &BreakPointSize, sizeof(s32), 1, out );

            // Write out header size in bytes.
            x_fwrite( &HeaderSize, sizeof(s32), 1, out );
            
            // Compress each channel...
            for( i=0 ; i<nChannels ; i++ )
            {
                // Write out the number of samples, sample rate and loop points
                x_fwrite( &nSamples,     sizeof(s32), 1, out );
                x_fwrite( &SampleRate,   sizeof(s32), 1, out );
                x_fwrite( &LoopStart,    sizeof(s32), 1, out );
                x_fwrite( &LoopEnd,      sizeof(s32), 1, out );
            }

            // Free buffer.
            x_free( pSampleBufferL );
            x_free( pSampleBufferR );

            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            x_fwrite( pCompressedBuffer, CompressedSize, 1, out );

            // Free the compressed buffer memory.
            x_free( pCompressedBuffer );
        }
        else
        {
            ASSERT(0); // Unsupported number of channels
        }

        // Write out the lip sync data.
        WriteLipSyncData( AudioFile, out );

        // Write out the break points.
        WriteBreakPoints( BreakPoints, out, FALSE );
        
        // Close and delete audio file
        AudioFile->Close();
        delete AudioFile;
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------
