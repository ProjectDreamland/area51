#include "Compress.hpp"
#include "SoundPackager.hpp"
#include "EncVag.h"
#include "ExportPackage.hpp"

extern u32 g_SampleRate;

const s32 SAMPLES_PER_ADPCM_BLOCK=28;
const s32 BYTES_PER_ADPCM_BLOCK=16;

#define ADPCM_BYTES_TO_SAMPLES(x)  ((x)*SAMPLES_PER_ADPCM_BLOCK/BYTES_PER_ADPCM_BLOCK)

u32 CompressAudioFilePS2_ADPCM( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    audio_file* AudioFile = audio_file::Create(in);
    s32         TotalCompressedSize = 0;

    if( AudioFile->Open( in ) )
    {
        xarray<audio_file::breakpoint> BreakPoints;
        s32  SampleRate         = AudioFile->GetSampleRate();
        s32  nChannels          = AudioFile->GetNumChannels();
        s32  nSamples           = AudioFile->GetNumSamples();
        // TODO: Put in looping stuff
        s32  LoopStart;
        s32  LoopEnd;
        s16* pSampleBuffer      = (s16*)x_malloc( sizeof(s16) * nSamples );
        s32  HeaderSize         = 4 * sizeof(s32);
        s32  CompressionMethod;
        s32  i;
        s32  CompressedSize;
        s32  BreakPointSize;
        s32  nBreakPoints;

        xarray<void*>         pCompressedBuffer;

        // Currently only support mono and stereo.
        ASSERT( (nChannels == 1) || (nChannels == 2) );
        g_SampleRate = SampleRate;

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
            CompressionMethod = PS2_ADPCM_NON_INTERLEAVED_STEREO;
        }
        else
        {
            CompressionMethod = PS2_ADPCM_MONO;
        }

        // Write out compression method.
        x_fwrite( &CompressionMethod, sizeof(s32), 1, out );

        // Pad out compressed data so it is 2K block aligned
        CompressedSize = ( (nSamples + BLKSIZ-1) / BLKSIZ) * 16;
        CompressedSize = (CompressedSize + 16 + 2047) &~2047;
        
        ASSERT( (CompressedSize % 16)==0);
        x_fwrite( &CompressedSize, sizeof(s32), 1, out );

        // Write out the lip sync size in bytes
        *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
        x_fwrite( LipSyncSize, sizeof(s32), 1, out );

        // Write out the break point size in byte.
        BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
        x_fwrite( &BreakPointSize, sizeof(s32), 1, out );

        // Write out header size in bytes.
        x_fwrite( &HeaderSize, sizeof(s32), 1, out );

        s16* pCompBuffer;
        s16* pSampBuffer;
        s32 Remain;
        s32 Length;

        // TODO: Put in looping stuff... 
        if( AudioFile->IsLooped() )
        {
            LoopStart   = AudioFile->GetLoopStart();
            LoopEnd     = AudioFile->GetLoopEnd();
        }
        else
        {
            LoopStart = 0;
            LoopEnd   = 0;
        }
        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {
            EncVagInit(ENC_VAG_MODE_NORMAL);
            // Allocate space for the compressed data.
            pCompressedBuffer.Append() = x_malloc( CompressedSize );

            // Read the the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBuffer, i );
            pCompBuffer = (s16*)pCompressedBuffer[i];
            pSampBuffer = (s16*)pSampleBuffer;
            Remain      = nSamples;
            Length      = 0;

            x_memset( pCompressedBuffer[i], 0, CompressedSize );
            // The first 16 bytes on a sample have to be zero to initialize the BRR
            // hardware
            pCompBuffer+=16/sizeof(s16);
            Length+=16;

            s16 BlankBuffer[BLKSIZ];
            while (Remain>0)
            {
                if (Remain < BLKSIZ)
                {
                    x_memset(BlankBuffer,0,sizeof(BlankBuffer));
                    x_memcpy(BlankBuffer,pSampBuffer,Remain*sizeof(s16));
                    pSampBuffer = BlankBuffer;
                }
                Length += 16;
                // We need to make sure at the end of every 2K block, we set the
                // LOOP_END flag so it'll properly jump to the next block.
                if (Remain <= BLKSIZ)
                {
                    if (LoopStart != LoopEnd)
                    {
                        EncVag(pSampBuffer,pCompBuffer,ENC_VAG_LOOP_END);
                    }
                    else
                    {
                        EncVag(pSampBuffer,pCompBuffer,ENC_VAG_1_SHOT_END);
                    }
                }
                else
                {
				    if ( Length & 2047)
				    {
					    EncVag(pSampBuffer,pCompBuffer,ENC_VAG_LOOP_BODY);
				    }
				    else
				    {
					    EncVag(pSampBuffer,pCompBuffer,ENC_VAG_LOOP_END);
				    }
                }
				Remain -= BLKSIZ;
				// 28 samples on in the sample buffer
				pSampBuffer+=BLKSIZ;
				// 16 bytes (8 words) on in the compressed buffer
				pCompBuffer+=16/sizeof(s16);

			}

			// For each channel, pad it out to a 2K boundary
			s16 TempBuffer[BLKSIZ];
			x_memset(TempBuffer,0,sizeof(TempBuffer));
			while (Length < CompressedSize)
			{
				Length+=16;

                if (LoopStart != LoopEnd)
                {
				    EncVag(TempBuffer,pCompBuffer,ENC_VAG_LOOP_END);
                }
                else
                {
				    EncVag(TempBuffer,pCompBuffer,ENC_VAG_1_SHOT_END);
                }
				pCompBuffer += 16/sizeof(s16);
			}

            //
            if( AudioFile->IsLooped() )
            {
                // We add 16 bytes to the loop start and end position to take 
                // in to account the 16 bytes of zero data at the start. 
                LoopStart   = AudioFile->GetLoopStart() + SAMPLES_PER_ADPCM_BLOCK;
                LoopEnd     = AudioFile->GetLoopEnd()   + SAMPLES_PER_ADPCM_BLOCK;

                // We round down the loop start position and round up the loop end 
                // to the ADPCM block boundaries so we have an accurate position with
                // respect to what the hardware will actually do to the looping points.
                LoopStart   = (LoopStart / SAMPLES_PER_ADPCM_BLOCK) * SAMPLES_PER_ADPCM_BLOCK;
                LoopEnd     = ((LoopEnd + SAMPLES_PER_ADPCM_BLOCK - 1) / SAMPLES_PER_ADPCM_BLOCK) * SAMPLES_PER_ADPCM_BLOCK;
            }
            else
            {
                LoopStart = 0;
                LoopEnd   = 0;
            }

            // Adjust number of samples to be based of exactly what was encoded.
            // This includes the 28 samples (16 bytes) to initialize the hardware
            // at the start as well as padding for the end of the loop.

            s32 nSamplesOut = ADPCM_BYTES_TO_SAMPLES(Length);

            // Write out the number of samples, sample rate and loop points
            x_fwrite( &nSamplesOut,  sizeof(s32), 1, out );
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

            for( s32 j=0 ; j<16 ; j++ )
            {
                u8* pChar = (u8*)pCompressedBuffer[i];
                pChar += j;
                x_DebugMsg( "%02x ", (u32)(*pChar) );
            }
            x_DebugMsg("\n");

            // Write out the compressed data.
            x_fwrite( pCompressedBuffer[i], CompressedSize, 1, out );
            
            // Free the compressed buffer memory.
            x_free( pCompressedBuffer[i] );
        }

        // Write out the lipsync data.
        WriteLipSyncData( AudioFile, out );

        // Write out the break points.
        WriteBreakPoints( BreakPoints, out, FALSE );
    }

    x_fclose( out );
    x_fclose( in );
    return TotalCompressedSize;
}
