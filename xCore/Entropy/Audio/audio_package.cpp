#include "audio_private.hpp"
#include "audio_hardware.hpp"
#include "audio_voice_mgr.hpp"
#include "audio_package.hpp"
#include "..\IOManager\io_mgr.hpp"
#include "x_files.hpp"
#include "e_virtual.hpp"
#include "audio_debug.hpp"
#include "e_audio.hpp"

#ifdef TARGET_GCN
extern "C" { void WriteARAMSynchronous( void* MRAM, u32 ARAM, s32 Length ); }
#endif

#if defined(TARGET_PS2)
#include "hardware/audio_hardware_ps2_private.hpp"
#endif
s32 N_ARAM_USED = 0; // 512*1024;

#define LOG_AUDIO_PACKAGE_LOAD "audio_package::Init"

//------------------------------------------------------------------------------

audio_package::audio_package( void )
{
    m_IsLoaded      = FALSE;
    m_Link.pPrev    = NULL;
    m_Link.pNext    = NULL;
    m_Link.pPackage = NULL;
    m_UserVolume      =
    m_UserPitch       =
    m_UserEffectSend  =
    m_UserNearFalloff =
    m_UserFarFalloff  = 1.0f;
    x_memset( &m_Header, 0, sizeof(m_Header) );
}

//------------------------------------------------------------------------------

audio_package::~audio_package( void )
{
}

//------------------------------------------------------------------------------

u32 audio_package::LoadHotSample( X_FILE* f, hot_sample* pHotSample, u32 Aram )
{
    CONTEXT("audio_package::LoadHotSample");

#ifdef TARGET_PS2
	io_request Request;
#endif

    switch( pHotSample->CompressionType )
    {
#ifdef TARGET_GCN
        case ADPCM:
        {
		    // Seek to sample offset within the file
		    x_fseek( f, pHotSample->WaveformOffset, X_SEEK_SET );

            // Read the sample waveform
            void* TempBuffer = x_malloc( pHotSample->WaveformLength );
            x_fread( TempBuffer, pHotSample->WaveformLength, 1, f );

            // TODO: Make this asyncronus.
            // Send it to the audio ram.
            WriteARAMSynchronous( TempBuffer, Aram, pHotSample->WaveformLength );

            // Done.
            x_free( TempBuffer );
            return pHotSample->WaveformLength;
        }
#endif

#ifdef TARGET_PC
        case ADPCM:
        case MP3:
        case PCM:
        {
			// Seek to sample offset within the file
			x_fseek( f, pHotSample->WaveformOffset, X_SEEK_SET );
            // Read the sample waveform
            x_fread( (void*)Aram, pHotSample->WaveformLength, 1, f );

            // Done.
            return pHotSample->WaveformLength;
        }
#endif

#ifdef TARGET_XBOX
        case ADPCM:
        case   PCM:
        {
			// Seek to sample offset within the file
			x_fseek( f, pHotSample->WaveformOffset, X_SEEK_SET );
            // Read the sample waveform
            x_fread( (void*)Aram, pHotSample->WaveformLength, 1, f );

            return pHotSample->WaveformLength;
        }
#endif


#ifdef TARGET_PS2
		case ADPCM:
        {
            byte* TempBuffer = (byte*)x_malloc( pHotSample->WaveformLength );
            {
                CONTEXT("HotSampleRead");

		        // Seek to sample offset within the file
		        x_fseek( f, pHotSample->WaveformOffset, X_SEEK_SET );

			    // Set up an io manager read request but going to the audio ram address space

                x_fread( TempBuffer, pHotSample->WaveformLength, 1, f );

            }
            {
                CONTEXT("HotSampleTransfer");
                xmesgq reply(1);
                // TODO: Make this asyncronus.
                // Send it to the audio ram.
                s32 Length = pHotSample->WaveformLength;
                byte* ptr = TempBuffer;
                while ( Length > (128 * 1024) )
                {
                    s32 BlockSize;
                    BlockSize = MIN((128*1024),Length);
                    s_ChannelManager.PushData(ptr,(void*)Aram,BlockSize,NULL);
                    ptr+=BlockSize;
                    Aram+=BlockSize;
                    Length-=BlockSize;
                }

                s_ChannelManager.PushData(ptr,(void*)Aram,Length,&reply);
                reply.Recv(MQ_BLOCK);
            }

            // Done.
            //****BW**** HACK ALERT HACK ALERT HACK ALERT
            // 
            // For some reason, the reply above was being received PRIOR to the package
            // transfer having completed. So if we then free'd and filled this buffer
            // with garbage prior to the transfer completing, we will get some crap
            // transferred to audio memory. The delay is to allow the system sufficient
            // time to complete all the transfers.
            //****BW**** END HACK ALERT END HACK ALERT
            x_free( TempBuffer );
			return pHotSample->WaveformLength;
        }
#endif
        default:
        {
            ASSERT( 0 );
            break;
        }
    }

    return 0;
}

#if 1
#undef vm_Alloc
#undef vm_Free

#define vm_Alloc x_malloc
#define vm_Free  x_free
#endif

//------------------------------------------------------------------------------

xbool audio_package::Init( const char* pFilename )
{
    CONTEXT("audio_package::Init");

    X_FILE*             f;
    package_identifier  PackageID;
    xbool               Result = FALSE;

    //
    // Clear out pointers
    //
    {
        m_IdentifierStringTable = NULL;
        m_IdentifierTable = NULL;
        m_DescriptorTable = NULL;
        m_DescriptorBuffer = NULL;
        m_MusicData = NULL;
        m_LipSyncTable = NULL;
        m_BreakPointTable = NULL;
        m_HotSamples = NULL;
        m_ColdSamples = NULL;

        for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
        {
            m_SampleIndices[ i ] = NULL;
        }
    }

#if defined(LOG_AUDIO_PACKAGE_LOAD)
    LOG_MESSAGE( LOG_AUDIO_PACKAGE_LOAD, "loading package: [%s]", pFilename );
#endif

    // Error check.
    ASSERT( pFilename );

    // Save the filename
    ASSERT( x_strlen( pFilename ) < AUDIO_PACKAGE_FILENAME_LENGTH );
    x_strncpy( m_Filename, pFilename, AUDIO_PACKAGE_FILENAME_LENGTH );
    m_Filename[ AUDIO_PACKAGE_FILENAME_LENGTH-1 ] = 0;

    // Open the file.
    f = x_fopen( pFilename, "rb" );
    if( f )
    {
        // Read in the package identifier.
        x_fread( &PackageID, sizeof(package_identifier), 1, f );

        // Correct version?
        if( !x_strncmp( PackageID.VersionID, PACKAGE_VERSION, VERSION_ID_SIZE ) )
        {
            // Correct platform?
            if( !x_strncmp( PackageID.TargetID, TARGET_ID, TARGET_ID_SIZE ) )
            {
                s32 i;

                // Now read in the header.
                x_fread( &m_Header, sizeof(package_header), 1, f );

                if( m_Header.nDescriptors <= 0 || m_Header.nDescriptors > 5000 )
                    return FALSE;
                if( m_Header.nIdentifiers <= 0 )
                    return FALSE;
                if( m_Header.DescriptorFootprint <= 0 )
                    return FALSE;

                ASSERT( m_Header.nDescriptors > 0 );
                ASSERT( m_Header.nIdentifiers > 0 );
                ASSERT( m_Header.DescriptorFootprint > 0 );

                if (m_Header.nDescriptors == 0 ||
                    m_Header.nIdentifiers == 0 ||
                    m_Header.DescriptorFootprint == 0)
                 return FALSE;

                // Allocate memory for the string table.
                m_IdentifierStringTable = (char*)vm_Alloc( m_Header.StringTableFootprint );

                // Allocate memory for the music data.
                if( m_Header.MusicDataFootprint )
                {
                    m_MusicData = (char*)vm_Alloc( m_Header.MusicDataFootprint );
                }
                else
                {
                    m_MusicData = NULL;
                }

                // Allocate memory for the lip sync table.
                if( m_Header.LipSyncTableFootprint )
                {
                    m_LipSyncTable = (char*)vm_Alloc( m_Header.LipSyncTableFootprint );
                }
                else
                {
                    m_LipSyncTable = NULL;
                }

                // Allocate memory for the break point table
                if( m_Header.BreakPointTableFootprint )
                {
                    m_BreakPointTable = (char*)vm_Alloc( m_Header.BreakPointTableFootprint );
                }
                else
                {
                    m_BreakPointTable = NULL;
                }

                // Allocate memory for the descriptor identifiers
                m_IdentifierTable = (descriptor_identifier*)vm_Alloc( m_Header.nIdentifiers * sizeof(descriptor_identifier) );

                // Allocate memory for the descriptor table.
                m_DescriptorTable = (u32*)vm_Alloc( m_Header.nDescriptors * sizeof(u32*) );

                // Allocate memory for the descriptors.
                m_DescriptorBuffer = (u16*)vm_Alloc( m_Header.DescriptorFootprint );

                // For each temperature...
                for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                {
                    if( m_Header.nSampleIndices[ i ] )
                    {
                        // Allocate memory for sample header index table.
                        m_SampleIndices[ i ] = (u16*)vm_Alloc( (m_Header.nSampleIndices[ i ]+1) * sizeof(u16) );
                    }
                    else
                    {
                        m_SampleIndices[ i ] = 0;
                    }
                }

                // Allocate memory for the hot and cold samples
                if( m_Header.nSampleHeaders[ HOT ] )
                {
                    m_HotSamples = (void*)vm_Alloc( m_Header.nSampleHeaders[ HOT ] * m_Header.HeaderSizes[ HOT ] );
                }
                else
                {
                    m_HotSamples = NULL;
                }

                if( m_Header.nSampleHeaders[ COLD ] )
                {
                    m_ColdSamples = (void*)vm_Alloc( m_Header.nSampleHeaders[ COLD ] * m_Header.HeaderSizes[ COLD ] );
                }
                else
                {
                    m_ColdSamples = NULL;
                }

                // TODO: Allocate memory for the warm samples.
                m_WarmSamples = NULL;

                // Read in the string table.
                x_fread( m_IdentifierStringTable, m_Header.StringTableFootprint, 1, f );

                // Read in the music data.
                if( m_MusicData )
                    x_fread( m_MusicData, m_Header.MusicDataFootprint, 1, f );

                // Read in the lipsync data
                if( m_LipSyncTable )
                    x_fread( m_LipSyncTable, m_Header.LipSyncTableFootprint, 1, f );

                // Read in the breakpoint data
                if( m_BreakPointTable )
                    x_fread( m_BreakPointTable, m_Header.BreakPointTableFootprint, 1, f );

                // Read in the identifiers.
                x_fread( m_IdentifierTable, sizeof(descriptor_identifier), m_Header.nIdentifiers, f );

                // Set the package for each identifier.
                for( i=0 ; i<m_Header.nIdentifiers ; i++ )
                    m_IdentifierTable[ i ].pPackage = this;

                // Read in the descriptor offsets.
                x_fread( m_DescriptorTable, sizeof(s32), m_Header.nDescriptors, f );

                // Fix up the descriptor pointers.
                for( i=0 ; i<m_Header.nDescriptors ; i++ )
                    m_DescriptorTable[ i ] += (u32)m_DescriptorBuffer;

                // Read in the descriptors.
                x_fread( m_DescriptorBuffer, m_Header.DescriptorFootprint, 1, f );

                // Read in the sample header indices.
                for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                {
                    // Only if buffer is available.
                    if( m_SampleIndices[ i ] )
                    {
                        // Read each temperatures sample index.
                        x_fread( m_SampleIndices[ i ], m_Header.nSampleIndices[ i ]+1, sizeof(u16), f );
                    }
                }

                // Read in the hot sample headers
                if( m_HotSamples )
                    x_fread( m_HotSamples, m_Header.HeaderSizes[ HOT ], m_Header.nSampleHeaders[ HOT ], f );

                // Read in the cold sample headers
                if( m_ColdSamples )
                    x_fread( m_ColdSamples, m_Header.HeaderSizes[ COLD ], m_Header.nSampleHeaders[ COLD ], f );
                
                // TODO: Load the warm sample headers.

                // TODO: Allocate individual aram for the samples.
                // Load the hot samples.
                N_ARAM_USED += m_Header.Aram;

#if defined(LOG_AUDIO_PACKAGE_LOAD)
                LOG_MESSAGE( LOG_AUDIO_PACKAGE_LOAD, "ARAM Required: %d, Total: %d", m_Header.Aram, N_ARAM_USED );
                LOG_FLUSH();
#endif
                m_AudioRam = 0;
                if( m_Header.Aram )
                {
                    m_AudioRam    = (u32)g_AudioHardware.AllocAudioRam( m_Header.Aram );
                    u32 Aram      = m_AudioRam;
                    s32 TotalAram = 0;
                    u32 BlockSize = 0;
                    u32 Base      = (u32)m_HotSamples;

                    #ifdef X_DEBUG
                    if( m_AudioRam==0 )
                    {
                        x_DebugMsg( "Audio package load failed!\n" );
                        x_DebugMsg( "-=> %s\n", pFilename );
                        x_DebugMsg( "Currently loaded packages:\n" );
                        g_AudioMgr.DisplayPackages();
                        ASSERT( m_AudioRam );
                    }
                    #endif

                    for( i=0 ; i<m_Header.nSampleHeaders[ HOT ] ; i++, Base += m_Header.HeaderSizes[HOT] )
                    {
                        hot_sample* pHotSample = (hot_sample*)Base;

                        if( m_AudioRam )
                        {
                            // Set the headers aram.
                            pHotSample->AudioRam = Aram;
                            // Load the hot sample.
                            BlockSize = LoadHotSample( f, pHotSample, Aram );
                            Aram += BlockSize;
                            TotalAram += BlockSize;
                            ASSERT( TotalAram <= m_Header.Aram );
                        }
#if 0
                        else
                        {
                            // Make it point to the not loaded sample
                            x_memcpy( pHotSample, g_AudioDebug.GetDebugSampleHeader( WARN_NOT_LOADED ), m_Header.HeaderSizes[HOT] );
                        }
#endif
                    }
                }


                // TODO: Load warm samples.

                // Set the user parameter defaults.
                SetUserVolume( 1.0f );
                SetUserPitch( 1.0f );
                SetUserEffectSend( 1.0f );
                SetUserNearFalloff( 1.0f );
                SetUserFarFalloff( 1.0f );
                SetUserNearDiffuse( 1.0f );
                SetUserFarDiffuse( 1.0f );

                // Nuke the packages 3d pan.
                m_Header.Params.Pan3d.Set( 0.0f, 0.0f, 0.0f, 0.0f );

                // All done!
                m_IsLoaded = TRUE;

                // Its all good!
                Result = ((m_AudioRam)||(!m_Header.Aram)) ? TRUE : FALSE;
            }
        }

        // Close the file.
        x_fclose( f );
    }

    /*
    //*BW* - If the system ran out of audio memory, during the loading, we need to nuke the entire package.
    if( Result == FALSE )
    {
        // Kill should do all the cleanup required.
        Kill();
    }
    */
    if( Result == FALSE )
    {
        vm_Free(m_IdentifierStringTable);   m_IdentifierStringTable = NULL;
        vm_Free(m_IdentifierTable);         m_IdentifierTable = NULL;
        vm_Free(m_DescriptorTable);         m_DescriptorTable = NULL;
        vm_Free(m_DescriptorBuffer);        m_DescriptorBuffer = NULL;
        vm_Free(m_MusicData);               m_MusicData = NULL;
        vm_Free(m_LipSyncTable);            m_LipSyncTable = NULL;
        vm_Free(m_BreakPointTable);         m_BreakPointTable = NULL;
        vm_Free(m_HotSamples);              m_HotSamples = NULL;
        vm_Free(m_ColdSamples);             m_ColdSamples = NULL;

        for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
        {
            vm_Free( m_SampleIndices[ i ] );
            m_SampleIndices[ i ] = NULL;
        }
    }

    // Tell the world...
    return Result;
}

//------------------------------------------------------------------------------

void audio_package::Kill( void )
{
    N_ARAM_USED -= m_Header.Aram;

    // Nuke voices that belong to this package.
    g_AudioHardware.Lock();
    g_AudioVoiceMgr.ReleasePackagesVoices( this );
    g_AudioHardware.Unlock();

    // Free the memory up
    vm_Free( m_IdentifierStringTable );
    vm_Free( m_IdentifierTable );
    vm_Free( m_DescriptorTable );
    vm_Free( m_DescriptorBuffer );
    if( m_MusicData )
        vm_Free( m_MusicData );
    if( m_LipSyncTable )
        vm_Free( m_LipSyncTable );
    if( m_BreakPointTable )
        vm_Free( m_BreakPointTable );
    for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        if( m_SampleIndices[ i ] )
            vm_Free( (void*)m_SampleIndices[ i ] );
    }
    if( m_HotSamples )
    {
        vm_Free( m_HotSamples );
    }
    if( m_ColdSamples )
        vm_Free( m_ColdSamples );
    if( m_AudioRam )
        g_AudioHardware.FreeAudioRam( (void*)m_AudioRam );
}

//------------------------------------------------------------------------------

void audio_package::SetUserVolume( f32 Volume )
{
    m_UserVolume = Volume;
    ComputeVolume();
}

//------------------------------------------------------------------------------

void audio_package::SetUserPitch( f32 Pitch )
{
    m_UserPitch = Pitch;
    ComputePitch();
}

//------------------------------------------------------------------------------

void audio_package::SetUserEffectSend( f32 EffectSend )
{
    m_UserEffectSend = EffectSend;
    ComputeEffectSend();
}

//------------------------------------------------------------------------------

void audio_package::SetUserNearFalloff( f32 NearFalloff )
{
    m_UserNearFalloff = NearFalloff;
    ComputeNearFalloff();
}

//------------------------------------------------------------------------------

void audio_package::SetUserFarFalloff( f32 FarFalloff )
{
    m_UserFarFalloff = FarFalloff;
    ComputeFarFalloff();
}

//------------------------------------------------------------------------------

void audio_package::SetUserNearDiffuse( f32 NearDiffuse )
{
    m_UserNearDiffuse = NearDiffuse;
    ComputeNearDiffuse();
}

//------------------------------------------------------------------------------

void audio_package::SetUserFarDiffuse( f32 FarDiffuse )
{
    m_UserFarDiffuse = FarDiffuse;
    ComputeFarDiffuse();
}

//------------------------------------------------------------------------------

void audio_package::ComputeVolume( void )
{
    f32 Volume;

    // TODO: Put in master fader.

    // Calculate the volume.
    Volume = m_UserVolume * m_Header.Params.Volume;

    // Ducking enabled?
    if( g_AudioMgr.IsAudioDuckingEnabled() )
        Volume *= m_Header.Params.VolumeDuck;

    // Was there a change?
    if( Volume != m_Volume )
    {
        // Set the volume.
        m_Volume = Volume;

        // Update all voices that reference this package.
        g_AudioVoiceMgr.UpdateVoiceVolume( this );
    }
}

//------------------------------------------------------------------------------

void audio_package::ComputePitch( void )
{
    f32 Pitch;

    // TODO: Put in master fader.
    
    // Calculate the pitch.
    Pitch = m_UserPitch * m_Header.Params.Pitch;

    // Was there a change?
    if( Pitch != m_Pitch )
    {
        // Set the pitch
        m_Pitch = Pitch;

        // Update all voices that reference this package.
        g_AudioVoiceMgr.UpdateVoicePitch( this );
    }
}

//------------------------------------------------------------------------------

void audio_package::ComputeEffectSend( void )
{
    f32 EffectSend;

    // TODO: Put in master fader.

    // Calculate the effect send.
    EffectSend = m_UserEffectSend * m_Header.Params.EffectSend;

    // Was there a change?
    if( EffectSend != m_EffectSend )
    {
        // Set the effect send.
        m_EffectSend = EffectSend;

        // Update all voices that reference this package.
        g_AudioVoiceMgr.UpdateVoiceEffectSend( this );
    }
}

//------------------------------------------------------------------------------

void audio_package::ComputeNearFalloff( void )
{
    f32 NearFalloff;

    // TODO: Put in master fader.

    // Calculate the near falloff.
    NearFalloff = m_UserNearFalloff * m_Header.Params.NearFalloff;

    // Was there a change?
    if( NearFalloff != m_NearFalloff )
    {
        // Set the near falloff.
        m_NearFalloff = NearFalloff;
    }
}

//------------------------------------------------------------------------------

void audio_package::ComputeFarFalloff( void )
{
    f32 FarFalloff;

    // TODO: Put in master fader.

    // Calculate the far fall off.
    FarFalloff = m_UserFarFalloff * m_Header.Params.FarFalloff;

    // Was there a change?
    if( FarFalloff != m_FarFalloff )
    {
        // Set the far falloff
        m_FarFalloff = FarFalloff;
    }
}

//------------------------------------------------------------------------------

void audio_package::ComputeNearDiffuse( void )
{
    f32 NearDiffuse;

    // TODO: Put in master fader.

    // Calculate the near falloff.
    NearDiffuse = m_UserNearDiffuse * m_Header.Params.NearDiffuse;

    // Was there a change?
    if( NearDiffuse != m_NearDiffuse )
    {
        // Set the near falloff.
        m_NearDiffuse = NearDiffuse;
    }
}

//------------------------------------------------------------------------------

void audio_package::ComputeFarDiffuse( void )
{
    f32 FarDiffuse;

    // TODO: Put in master fader.

    // Calculate the far fall off.
    FarDiffuse = m_UserFarDiffuse * m_Header.Params.FarDiffuse;

    // Was there a change?
    if( FarDiffuse != m_FarDiffuse )
    {
        // Set the far falloff
        m_FarDiffuse = FarDiffuse;
    }
}

//------------------------------------------------------------------------------

char* audio_package::GetMusicType( void )
{
    if( m_MusicData )
    {
        return m_MusicData;
    }
    else
    {
        return NULL;
    }
}

//------------------------------------------------------------------------------

s32 audio_package::GetMusicIntensity( music_intensity* & Intensity )
{
    s32 Result = 0;

    Intensity = NULL;

    if( m_MusicData )
    {
        s32 mi_size = sizeof( music_intensity );
        Intensity = (music_intensity*)(m_MusicData + 32);
        Result  =  m_Header.MusicDataFootprint;
        Result -= 32;
        Result /= mi_size;
    }

    return Result;
}
