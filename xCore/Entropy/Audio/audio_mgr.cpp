#include "e_Audio.hpp"
#include "audio_private_pkg.hpp"
#include "audio_hardware.hpp"
#include "audio_channel_mgr.hpp"
#include "audio_voice_mgr.hpp"
#include "audio_package.hpp"
#include "audio_stream_mgr.hpp"
#include "audio_inline.hpp"
#include "audio_debug.hpp"
#include "x_log.hpp"

#if (!defined(X_RETAIL) || defined(X_QA)) && defined(TARGET_PS2)
#include "sntty.h"  
#define ENABLE_AUDIO_DEBUG
#endif

#ifdef TARGET_PC
static xbool    s_bDisableAudio = FALSE;
void EnableAudio( xbool Enable )
{
    s_bDisableAudio = !Enable;
}
#else
static xbool    s_bDisableAudio = FALSE;
#endif

#if defined(rbrannon) || defined(mreed)
#define LOG_PLAY_SUCCESS "audio_mgr::Play(success)"
#define LOG_PLAY_CLIPPED "audio_mgr::Play(clipped)"
#define LOG_PLAY_FAILURE "audio_mgr::Play(failure)"
#define LOG_PLAY_WARNING "audio_mgr::Play(warning)"

voice*   g_DebugVoice   = NULL;
element* g_DebugElement = NULL;
channel* g_DebugChannel = NULL;
f32      g_DebugTime    = 0.0f; 
#endif

//#define TRAP_ON_IDENTIFIER

#ifdef TRAP_ON_IDENTIFIER
xbool    g_EnableIdentifierTrap = 1;
char     g_DebugIdentifier[64] = "FORCE_FIELD_ACTIVE";
#endif

//------------------------------------------------------------------------------

static xbool    s_Initialized = FALSE;

static xthread* s_PeriodicUpdateThread = NULL;
static xthread* s_HardwareUpdateThread = NULL;

#define FALLOFF_TABLE_SIZE (9)

typedef f32 falloff_table[FALLOFF_TABLE_SIZE];

static falloff_table s_FalloffTables[NUM_ROLLOFFS] = 
{
    { 1.00000f, 0.87500f, 0.75000f, 0.62500f, 0.50000f, 0.37500f, 0.25000f, 0.12500f, 0.00000f }, // LINEAR_ROLLOFF
    { 1.00000f, 0.76562f, 0.56250f, 0.39062f, 0.25000f, 0.14062f, 0.06250f, 0.01562f, 0.00000f }, // FAST_ROLLOFF
    { 1.00000f, 0.93541f, 0.86602f, 0.79056f, 0.70710f, 0.61237f, 0.50000f, 0.35355f, 0.00000f }  // SLOW_ROLLOFF
};
//------------------------------------------------------------------------------

#ifndef X_RETAIL

struct s_AudioData
{
    char   Desc[64];
    f32    fVolume;
    f32    fNearClip;
    f32    fFarClip;
};

extern s_AudioData *AudioTweakData;
extern xbool        AUDIO_TWEAK;
const char*         pTweakDescriptor = NULL;
#endif

//------------------------------------------------------------------------------

audio_mgr g_AudioMgr;

//------------------------------------------------------------------------------

static void AudioMgrPeriodicUpdate( void )
{
    xthread* pThread = x_GetCurrentThread();

    while( pThread->IsActive() )
    {
        // Check the queued voices...
        g_AudioVoiceMgr.UpdateCheckQueued();
        // Now do the periodic stream update.
        g_AudioMgr.PeriodicUpdate();
        x_DelayThread( 10 );
    }
}

//------------------------------------------------------------------------------

static void AudioMgrHardwareUpdate( void )
{
    xthread* pThread = x_GetCurrentThread();

    while( pThread->IsActive() )
    {
        // Now do the hardware update.
        g_AudioHardware.Lock();
        g_AudioHardware.Update();
        g_AudioHardware.Unlock();
        x_DelayThread( 10 );
    }
}

//------------------------------------------------------------------------------

#ifdef ENABLE_AUDIO_DEBUG
void AudioDebug( const char* pString )
{
#ifdef X_QA    // this used to be excluded from a QA build... 
    (void) pString;
#else
    if (x_IsAtomic())
    {
        //***** BIG NOTE *****
        // If you get here when running normally, this means text was attempted to be printed
        // while interrupts were disabled (a problem on PS2). Please contact Biscuit since, if
        // this happens, this should only be in a system defined function.
        BREAK;
    }

    s32 length = snputs( pString );
    if (length < 0)
        scePrintf("%s",pString);
#endif // X_QA
}
#endif // ENABLE_AUDIO_DEBUG

//------------------------------------------------------------------------------

static void DecodeParameters( uncompressed_parameters* pParams, u16* pDescriptor )
{
    xbool bHasParams = GET_DESCRIPTOR_HAS_PARAMS( *pDescriptor );
    
    // Get the flags.
    pParams->Flags = *(pDescriptor+1);

    // Parameter defined?
    if( bHasParams )
    {
        u8* pDescriptorBytes;
        u32 Bits1;
        u32 Bits2;
        u32 Bits;
        u16 DataU16;

        // Skip past descriptor type, index, etc..
        pDescriptor++;

        // Skip past the flags
        pDescriptor++;

        // Skip past the size of the parameters.
        pDescriptor++;

        // Get the parameter bits.
        Bits1 = (u32)(*pDescriptor++);
        Bits2 = (u32)(*pDescriptor++);
        Bits  = Bits1 + (Bits2 << 16);

        // Set the bits
        pParams->Bits = Bits;

        //--------------------------------//
        // Process the 16-bit parameters. //
        //--------------------------------//

        // Pitch specified?
        if( GET_PITCH_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);
            
            // Is it a compressed 1.0f?
            if( DataU16 == FLOAT4_TO_U16BIT( 1.0f ) )
            {
                // Set it to exactly 1.0.
                pParams->Pitch = 1.0f;
            }
            else
            {
                // Decompress it to a float.
                pParams->Pitch = U16BIT_TO_FLOAT4( DataU16 );
            }
        }
        else
        {
            // Default is 1.0 since this is multiplied.
            pParams->Pitch = 1.0f;
        }

        // Pitch variance specified?
        if( GET_PITCH_VARIANCE_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);
         
            // Is it a compressed 0.0f?
            if( DataU16 == FLOAT1_TO_U16BIT( 0.0f ) )
            {
                pParams->PitchVariance = 0.0f;
            }
            else
            {  
                pParams->PitchVariance = U16BIT_TO_FLOAT1( DataU16 );
            }
        }
        
        // Volume specified?
        if( GET_VOLUME_BIT( Bits )  )
        {
            DataU16 = (*pDescriptor++);

            // Is it a compressed 1.0f?
            if( DataU16 == FLOAT1_TO_U16BIT( 1.0f ) )
            {
                // Set it exactly to 1.0.
                pParams->Volume = 1.0f;
            }
            else
            {
                // Decompress it to a float.
                pParams->Volume = U16BIT_TO_FLOAT1( DataU16 );
            }
        }
        else
        {
            // Default is 1.0 since this is multiplied.
            pParams->Volume = 1.0f;
        }

        // Volume variance specified?
        if( GET_VOLUME_VARIANCE_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);

            // Is it a compressed 0.0f?
            if( DataU16 == FLOAT1_TO_U16BIT( 0.0f ) )
            {
                // Set it to exactly 0.0f.
                pParams->VolumeVariance = 0.0f;
            }
            else
            {
                // Decompress it to a float.
                pParams->VolumeVariance = U16BIT_TO_FLOAT1( DataU16 );
            }
        }

        // Center volume specified?
        if( GET_VOLUME_CENTER_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);

            // Is it a compressed 1.0f?
            if( DataU16 == FLOAT1_TO_U16BIT( 1.0f ) )
            {
                // Set it exactly to 1.0.
                pParams->VolumeCenter = 1.0f;
            }
            else
            {
                // Decompress it to a float.
                pParams->VolumeCenter = U16BIT_TO_FLOAT1( DataU16 );
            }
        }
        else
        {
            // Default is 1.0 since this is multiplied.
            pParams->VolumeCenter = 1.0f;
        }

        // LFE Volume specified?
        if( GET_VOLUME_LFE_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);

            // Is it a compressed 1.0f?
            if( DataU16 == FLOAT1_TO_U16BIT( 1.0f ) )
            {
                // Set it exactly to 1.0.
                pParams->VolumeLFE = 1.0f;
            }
            else
            {
                // Decompress it to a float.
                pParams->VolumeLFE = U16BIT_TO_FLOAT1( DataU16 );
            }
        }
        else
        {
            // Default is 1.0 since this is multiplied.
            pParams->VolumeLFE = 1.0f;
        }

        // Volume duck specified?
        if( GET_VOLUME_DUCK_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);

            // Is it a compressed 1.0f?
            if( DataU16 == FLOAT1_TO_U16BIT( 1.0f ) )
            {
                // Set it exactly to 1.0.
                pParams->VolumeDuck = 1.0f;
            }
            else
            {
                // Decompress it to a float.
                pParams->VolumeDuck = U16BIT_TO_FLOAT1( DataU16 );
            }
        }
        else
        {
            // Default is 1.0 since this is multiplied.
            pParams->VolumeDuck = 1.0f;
        }

        // User data specified?
        if( GET_USER_DATA_BIT( Bits ) )
            pParams->UserData = (u32)*pDescriptor++;

        // Replay delay specified?
        if( GET_REPLAY_DELAY_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);
            pParams->ReplayDelay = U16BIT_TO_FLOAT_TENTH( DataU16 );
        }

        // Last play specified?
        if( GET_LAST_PLAY_BIT( Bits ) )
        {
            DataU16 = (*pDescriptor++);
            pParams->LastPlay = U16BIT_TO_FLOAT_TENTH( DataU16 );
        }
        else
        {
            pParams->LastPlay = 0.0f;
        }

        //-------------------------------//
        // Process the 8-bit parameters. //
        //-------------------------------//

        // Coerce to 8-bit pointer.
        pDescriptorBytes = (u8*)pDescriptor;
        
        // Pan specified?        
        if( GET_PAN_2D_BIT( Bits ) )
            pParams->Pan2d = S8BIT_TO_FLOAT1( (*pDescriptorBytes++) );
        
        // Priority specified?
        if( GET_PRIORITY_BIT( Bits ) )
            pParams->Priority = (u32)(*pDescriptorBytes++);
        
        // Effect send specified? Default is 1.0 since this is multiplied.
        if( GET_EFFECT_SEND_BIT( Bits ) )
            pParams->EffectSend = U8BIT_TO_FLOAT1( (*pDescriptorBytes++) );
        else
            pParams->EffectSend = 1.0f;

        // Near falloff specified? Default is 1.0 since this is multiplied.
        if( GET_NEAR_FALLOFF_BIT( Bits ) )
            pParams->NearFalloff = U8BIT_TO_FLOAT10( (*pDescriptorBytes++) );
        else
            pParams->NearFalloff = 1.0f;

        // Far falloff specified? Default is 1.0 since this is multiplied.
        if( GET_FAR_FALLOFF_BIT( Bits ) )
            pParams->FarFalloff = U8BIT_TO_FLOAT10( (*pDescriptorBytes++) );
        else
            pParams->FarFalloff = 1.0f;

        // Rolloff curve specified?
        if( GET_ROLLOFF_METHOD_BIT( Bits ) )
            pParams->RolloffCurve = (u32)(*pDescriptorBytes++);

        // Near diffuse specified? Default is 1.0 since this is multiplied.
        if( GET_NEAR_DIFFUSE_BIT( Bits ) )
            pParams->NearDiffuse = U8BIT_TO_FLOAT10( (*pDescriptorBytes++) );
        else
            pParams->NearDiffuse = 1.0f;

        // Far diffuse specified? Default is 1.0 since this is multiplied.
        if( GET_FAR_DIFFUSE_BIT( Bits ) )
            pParams->FarDiffuse = U8BIT_TO_FLOAT10( (*pDescriptorBytes++) );
        else
            pParams->FarDiffuse = 1.0f;

        // Play percent specified?
        if( GET_PLAY_PERCENT_BIT( Bits ) )
            pParams->PlayPercent = (u32)(*pDescriptorBytes++);
    }
    else
    {
        // None are defined...
        pParams->Bits = 0;

        // These are multiplied, so set to 1.0
        pParams->Pitch        =
        pParams->Volume       =
        pParams->VolumeCenter =
        pParams->VolumeLFE    =
        pParams->VolumeDuck   =
        pParams->EffectSend   =
        pParams->NearFalloff  =
        pParams->FarFalloff   = 
        pParams->NearDiffuse  =
        pParams->FarDiffuse   = 1.0f;
   }

#ifndef X_RETAIL
    if( AUDIO_TWEAK && pTweakDescriptor && AudioTweakData )
    {
        s32 i=0;

        while( AudioTweakData[i].Desc[0] )
        {
            if( x_stricmp( AudioTweakData[i].Desc, pTweakDescriptor ) == 0 )
            {
                if( AudioTweakData[i].fVolume )
                    pParams->Volume = AudioTweakData[i].fVolume;

                if( AudioTweakData[i].fNearClip )
                    pParams->NearFalloff = AudioTweakData[i].fNearClip;

                if( AudioTweakData[i].fFarClip)
                    pParams->FarFalloff = AudioTweakData[i].fFarClip;

                return;
            }

            i++;
        }
    }
#endif // X_RETAIL
}

//------------------------------------------------------------------------------

inline voice* IdToVoice( voice_id VoiceID )
{
    voice* pVoice   = g_AudioVoiceMgr.GetVoiceBuffer();
    s32    Index    = ((VoiceID & 0xffff)-1);
    u32    Sequence = ((VoiceID >> 16) & 0x0000ffff);

    // Error check.
    if( (Index < 0) || (Index >= g_AudioVoiceMgr.GetNumVoices()) )
    {
        return NULL;
    }

    // Does the sequence match?
    if( ((pVoice+Index)->Sequence & 0x0000ffff) == Sequence )
        return pVoice+Index;
    else
        return NULL;
}

//------------------------------------------------------------------------------

inline voice_id VoiceToId( voice* pVoice )
{
    s32 Index = pVoice-g_AudioVoiceMgr.GetVoiceBuffer();

    // Error check.
    ASSERT( VALID_VOICE(pVoice) );

    // Encode index and sequence.
    return (((Index+1) & 0xffff) + (pVoice->Sequence << 16));
}

//------------------------------------------------------------------------------
// Class functions.

audio_mgr::audio_mgr( void )
{
    // Nuke the ear lists.
    m_pUsedEars = NULL;
    m_pFreeEars = NULL;

    // Init the time
    m_Time = 0.0f;

    // Clear ducking level.
    m_AudioDuckLevel = 0;

    // Nuke the package list.
    m_Link.pPrev    = &m_Link;
    m_Link.pNext    = &m_Link;
    m_Link.pPackage = NULL;

    m_WetDryMix = 0.0f;
}

//------------------------------------------------------------------------------

audio_mgr::~audio_mgr( void )       
{
}

void audio_mgr::SetReverbWetDryMix( f32 Mix )
{
    if( Mix != m_WetDryMix )
    {
#if defined(TARGET_PS2)
        g_AudioHardware.SetReverbWetDryMix( Mix );
#endif
        m_WetDryMix = Mix;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::Init( s32 MemSize )
{
    MEMORY_OWNER( "audio_mgr::Init" );

        // Error check.
    ASSERT( s_Initialized == FALSE );

    // FOr each ear...
    for( s32 i=0 ; i<MAX_EARS ; i++ )
    {
        // Initialize each ear.
        m_Ear[i].Volume   = 1.0f;
        m_Ear[i].Sequence = 0;

        // Link it.
        m_Ear[i].pNext = &m_Ear[i+1];
    }

    // Terminate list.
    m_Ear[MAX_EARS-1].pNext = NULL;

    // Init free and used ears.
    m_pUsedEars   = NULL;
    m_pFreeEars   = &m_Ear[0];
    m_pCurrentEar = NULL;

    // Init the time.
    m_Time = 0.0f;

    // Clear ducking level.
    m_AudioDuckLevel = 0;

    // Initialize the channel manager.
    g_AudioChannelMgr.Init();

    ASSERTS((MemSize & 2047)==0,"Memory must be in 2K increments");
    // Initialize the audio hardware.
    g_AudioHardware.Init(MemSize);

    // Initialize the stream manager.
    g_AudioStreamMgr.Init();

    // Initialize the virtual voices.
    g_AudioVoiceMgr.Init();

    // Nuke the identifier table.
    m_pIdentifiers.Clear();
    m_pIdentifiers.SetCapacity( 2048 );

    // Create the periodic update thread.
    ASSERT( s_PeriodicUpdateThread == NULL );
    ASSERT( s_HardwareUpdateThread == NULL );
    s_PeriodicUpdateThread = new xthread( AudioMgrPeriodicUpdate, (const char*)"AudioMgr PeriodicUpdate", 8192, 1 );
	s_HardwareUpdateThread = new xthread( AudioMgrHardwareUpdate, (const char*)"AudioMgr HardwareUpdate", 8192, 4 );

    // Set flag.
    s_Initialized = TRUE;
}

//------------------------------------------------------------------------------

void audio_mgr::Kill( void )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    // Release all voices.
    ReleaseAll();

    // Nuke the periodic thread.
    delete s_HardwareUpdateThread;
    delete s_PeriodicUpdateThread;

    s_HardwareUpdateThread = NULL;
    s_PeriodicUpdateThread = NULL;

    // Nuke the packages
    UnloadAllPackages();

    // Kill the virtual voices.
    g_AudioVoiceMgr.Kill();

    // Kill the streams.
    g_AudioStreamMgr.Kill();

    // Kill the audio hardware.
    g_AudioHardware.Kill();

    // Kill the channel manager.
    g_AudioChannelMgr.Kill();

    // Clear flag.
    s_Initialized = FALSE;
}

//------------------------------------------------------------------------------
void audio_mgr::ResizeMemory(s32 NewSize)
{
    g_AudioStreamMgr.Kill();
    g_AudioHardware.ResizeMemory( NewSize );
    g_AudioStreamMgr.Init();
}

//------------------------------------------------------------------------------
f32 audio_mgr::GetLengthSeconds( const char* pIdentifier )
{
#ifdef TARGET_PC
    f32 Result;
    voice_id VoiceID;
    ASSERT( s_Initialized);
    VoiceID = Play( pIdentifier, FALSE );
    Result = g_AudioVoiceMgr.GetVoiceTime( IdToVoice( VoiceID ) );
    Release( VoiceID, 0.0f );
    return Result;
#else
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif
    f32            Result = 0.0f;
    audio_package* pPackage;
    u16*           pDescriptor;
    char*          DescriptorName   = NULL;
    
    // Find the decriptor by name.
    pDescriptor = FindDescriptorByName( pIdentifier, &pPackage, DescriptorName );

    // Only if it could be found...
    if( pDescriptor )
    {
        u32 DescriptorIndex = (u32)(*pDescriptor++);
        u32 DescriptorType  = GET_DESCRIPTOR_TYPE( DescriptorIndex );
        u32 ParameterSize   = 0;

        // Does it have parameters?
        if( GET_DESCRIPTOR_HAS_PARAMS( DescriptorIndex ) )
        {
            // Get parameter size in WORDS (skip over flags to get to the size).
            ParameterSize = *(pDescriptor+1) >> 1;
        }

        // Bump past the flags and parameters.
        pDescriptor += 1+ParameterSize;

        switch( DescriptorType )
        {
            case SIMPLE:
            {
                u32 ElementIndex = (u32)(*pDescriptor);
                u32 ElementType  = GET_INDEX_TYPE( ElementIndex );
                u32 Index        = GET_INDEX( ElementIndex );

                switch( ElementType )
                {        
                    case COLD_INDEX:
                    {
                        // Calculate number of channels.
                        ASSERT( pPackage->m_SampleIndices[ COLD ] );
                        ASSERT( Index < (u32)pPackage->m_Header.nSampleIndices[ COLD ] );
                        if( Index >= (u32)pPackage->m_Header.nSampleIndices[ COLD ] || !pPackage->m_SampleIndices[ COLD ] )
                            return 0;
                        s32 nChannels = (s32)(pPackage->m_SampleIndices[ COLD ][ Index+1 ] - pPackage->m_SampleIndices[ COLD ][ Index ]);
                        (void)nChannels;
                        ASSERT( nChannels == 1 || nChannels == 2 );

                        u32 ColdIndex = (u32)pPackage->m_SampleIndices[ COLD ][ Index ];
                        u32 Base      = (u32)pPackage->m_ColdSamples + (ColdIndex * pPackage->m_Header.HeaderSizes[ COLD ]);
                        cold_sample* pColdSample = (cold_sample*)Base;
                
                        Result = (f32)pColdSample->nSamples / (f32)pColdSample->SampleRate;
                        break;
                    }

                    default:
                    {
                        ASSERT( 0 );
                        break;
                    }
                }
                break;
            }

            default:
                ASSERT( 0 );
                break;
        }
    }

    return Result;
#endif
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetLengthSeconds( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    f32 Result;

    Result = g_AudioVoiceMgr.GetVoiceTime( IdToVoice( VoiceID ) );
    return Result;
}

//=========================================================================
// Returns a localized filename for the current langauge. 
// The name will change ONLY if the file starts with "DX_" which indicates
// that the file is a dialogue. Note also, that this is different from 
// the naming for text files which are prepended.
const char* audio_mgr::GetLocalizedName( const char* pFileName ) const
{
    char Drive[X_MAX_DRIVE], Path[X_MAX_PATH], FName[X_MAX_FNAME], Ext[X_MAX_EXT];

    ASSERT(pFileName != NULL);
    x_splitpath(pFileName, Drive, Path, FName, Ext);

    // check that this is a dialog file
    if( (FName[0] == 'D') && (FName[1] == 'X') && (FName[2] == '_') &&
        // if this is English, leave the file name "unmangled". This should probably be fixed in the future.
        (x_strcmp(x_GetLocaleString(XL_LANG_ENGLISH), x_GetLocaleString()) != 0)
      )
    {
        static char Name[X_MAX_PATH];
        x_sprintf(Name, "%s%s%s_%s%s", Drive, Path, FName, x_GetLocaleString(), Ext);

#if defined(X_DEBUG) && defined(ctetrick)
        LOG_MESSAGE("audio_mgr::GetLocalizedName", "voice file (%s) returned as (%s)", pFileName, Name);
#endif

        return (const char*)Name;
    }
    else
    {
        return pFileName;
    }
}

//------------------------------------------------------------------------------

xbool audio_mgr::LoadPackage( const char* pFilename )
{
    CONTEXT( "audio_mgr::LoadPackage()" );
    MEMORY_OWNER( "audio_mgr::LoadPackage()" );

#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif
    xtimer t;

    t.Start();
    // Create a new audio package.
    audio_package* pPackage = new audio_package;

    char LocalizedName[X_MAX_PATH];

    x_strcpy(LocalizedName, GetLocalizedName(pFilename));

    // Load the package.
    if( pPackage->Init( LocalizedName ) )
    {
        // Set the package.
        pPackage->m_Link.pPackage = pPackage;

        // Now insert the package into the package list.
        pPackage->m_Link.pNext = m_Link.pNext;
        pPackage->m_Link.pPrev = &m_Link;
        m_Link.pNext->pPrev    = &pPackage->m_Link;
        m_Link.pNext           = &pPackage->m_Link;

        // Re-merge the identifier tables.
        MergeIdentifierTables();

        t.Stop();
        LOG_MESSAGE("audio_mgr::LoadPackage","Loaded package %s in %2.02fms",pFilename, t.ReadMs());
        // Its all good!

        return TRUE;
    }
    else
    {
        // Nuke it.
        delete pPackage;

        // Oops...
        return FALSE;
    }
}

//------------------------------------------------------------------------------

xbool audio_mgr::IsPackageLoaded( const char* pFilename )
{
    return( FindPackageByName( pFilename ) != NULL );
}

//------------------------------------------------------------------------------

xbool audio_mgr::UnloadPackage( const char* pFilename )
{
    // Find the package by its name.
    audio_package* pPackage = FindPackageByName( pFilename );

    if( (pPackage ) != NULL )
    {
        // Take it out of the list
        pPackage->m_Link.pPrev->pNext = pPackage->m_Link.pNext;
        pPackage->m_Link.pNext->pPrev = pPackage->m_Link.pPrev;

        // Kill it.
        pPackage->Kill();

        // Nuke it.
        delete pPackage;

        // Re-merge the identifier tables.
        MergeIdentifierTables();

        // All good!
        return TRUE;
    }
    else
    {
        // Oops couldn't find it.
        return FALSE;
    }
}

//------------------------------------------------------------------------------

xbool audio_mgr::LoadPackageStrings( const char* pFilename, xarray<xstring>& Strings )
{
    xbool          bNeedToLoad;
    xbool          bNeedToUnload;
    audio_package* pPackage;

    // Find the package by its name.
    pPackage    = FindPackageByName( pFilename );
    bNeedToLoad = (pPackage == NULL);

    // Need to load it?
    if( bNeedToLoad )
    {
        // Load it and find the package.
        bNeedToUnload = LoadPackage( pFilename );
        pPackage      = FindPackageByName( pFilename );
    }
    else
    {
        // No need to unload it.
        bNeedToUnload = FALSE;
    }
        
    // Found it?
    if( pPackage )
    {
        for( s32 i=0 ; i < pPackage->m_Header.nIdentifiers ; i++ )
        {
            u32   Offset;
            char* pString;

            // Calculate the string table offsets.
            Offset = pPackage->m_IdentifierTable[ i ].StringOffset;
            pString = pPackage->m_IdentifierStringTable + Offset;
            Strings.Append() = pString;
        }

        // Unload the package.
        if( bNeedToUnload )
            UnloadPackage( pFilename );         

        // Its all good!
        return TRUE;
    }
    else
    {
        // Could not find it!
        return FALSE;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::Update( f32 DeltaTime )
{
    CONTEXT("audio_mgr::Update");

    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    // Update the time.
    m_Time += DeltaTime;

#if defined( rbrannon )
    void AudioThrashUpdate(void);
    AudioThrashUpdate();
#endif

    // Update the channel manager.
    g_AudioChannelMgr.Update();

    // Update the voice manager.
    g_AudioVoiceMgr.Update( DeltaTime );

    // Ok to do the audio update in hardware now.
    g_AudioHardware.SetDoHardwareUpdate();
}

//------------------------------------------------------------------------------
void audio_mgr::PeriodicUpdate( void )
{
    // Update the streams.
    g_AudioStreamMgr.Update();

    // Update the hardware.
    //Now done in it's own thread: g_AudioHardware.Update();
}

//------------------------------------------------------------------------------

void audio_mgr::GetEar( ear_id EarID, matrix4& W2V, vector3& Position, s32& ZoneID, f32& Volume )
{
    // Error check.
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    // Convert id.
    audio_mgr_ear* pEar = IdToEar( EarID );
    ASSERT( pEar );

    if( pEar )
    {
        W2V      = pEar->W2V;
        Position = pEar->Position;
        Volume   = pEar->Volume;
        ZoneID   = pEar->ZoneID;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetEar( ear_id EarID, const matrix4& W2V, const vector3& Position, s32 ZoneID, f32 Volume )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif
    ASSERT( (Volume >= 0.0f) && (Volume <= 1.0f) );

    // Convert id.
    audio_mgr_ear* pEar = IdToEar( EarID );
    ASSERT( pEar );

    if( pEar )
    {
        pEar->W2V      = W2V;
        pEar->Volume   = Volume;
        pEar->Position = Position;
        pEar->ZoneID   = ZoneID;
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendHot( u32 Index, f32 DeltaTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    element* pElements[2];
    s32      nChannels;
    s32      i;

    // Calculate number of channels.
    ASSERT( pPackage->m_SampleIndices[ HOT ] );
    ASSERT( Index < (u32)pPackage->m_Header.nSampleIndices[ HOT ] );
    if( Index >= (u32)pPackage->m_Header.nSampleIndices[ HOT ] || !pPackage->m_SampleIndices[ HOT ] )
        return 0;
    nChannels = (s32)(pPackage->m_SampleIndices[ HOT ][ Index+1 ] - pPackage->m_SampleIndices[ HOT ][ Index ]);
    ASSERT( (nChannels > 0) && (nChannels <= 2 ) );
    if( (nChannels <=0) || (nChannels > 2) )
        return 0;

    // Aquire an element for each channel.
    for( i=0 ; i<nChannels ; i++ )
    {
        if( (pElements[ i ] = g_AudioVoiceMgr.AquireElement()) == NULL)
        {
            for( s32 j=0 ; j<i ; j++ )
                if( pElements[ j ] )
                    g_AudioVoiceMgr.ReleaseElement( pElements[ j ], FALSE );

            return 0;
        }
    }

    switch( nChannels )
    {
        case 1:
        {
            pElements[ 0 ]->pStereoElement = NULL;
            break;
        }

        // Stereo
        case 2:
        {
            pElements[ 0 ]->pStereoElement = pElements[ 1 ];
            pElements[ 1 ]->pStereoElement = pElements[ 0 ];
            break;
        }
    }

    // For each channel...
    u32 HotIndex = (u32)pPackage->m_SampleIndices[ HOT ][ Index ];
    u32 Base     = (u32)pPackage->m_HotSamples + (HotIndex * pPackage->m_Header.HeaderSizes[ HOT ]);
    for( i=0 ; i<nChannels ; i++, Base+=pPackage->m_Header.HeaderSizes[ HOT ] )
    {
        element* pElement = pElements[ i ];

        // Put element at end of voices element list.
        g_AudioVoiceMgr.AppendElementToVoice( pElement, pVoice );

        // Get the elements parameters.
        GetElementParameters( &pElement->Params, pDescriptor, pVoice );
        
        switch( nChannels )
        {
            case 1:
            {
                // If pan was specified, then it cannot be changed.
                if( pElement->Params.Bits & PAN_2D )
                    pElement->IsPanChangeable = FALSE;
                else
                    pElement->IsPanChangeable = TRUE;
                break;
            }

            case 2:
            {
                // Can't modify pan on stereo sample.
                pElement->IsPanChangeable = FALSE;
                if( i == 0 )
                    pElement->Params.Pan2d = -1.0f;
                else
                    pElement->Params.Pan2d =  1.0f;
            }
        }

        pElement->DeltaTime         = DeltaTime;
        pElement->State             = ELEMENT_READY;
        pElement->Type              = HOT_SAMPLE;
        pElement->Sample.pHotSample = (hot_sample*)Base;

        // Initialize the elements parameters.
        g_AudioVoiceMgr.InitSingleElement( pElement );
    }

    return nChannels;
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendWarm( u32 Index, f32 DeltaTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    (void)Index;
    (void)DeltaTime;
    (void)pDescriptor;
    (void)pVoice;
    (void)pPackage;
    return 0;
}

//------------------------------------------------------------------------------


s32 audio_mgr::AppendCold( u32 Index, f32 DeltaTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    element* pElements[2];
    s32      nChannels;
    s32      i;

    // Calculate number of channels.
    ASSERT( pPackage->m_SampleIndices[ COLD ] );
    ASSERT( Index < (u32)pPackage->m_Header.nSampleIndices[ COLD ] );
    if( Index >= (u32)pPackage->m_Header.nSampleIndices[ COLD ] || !pPackage->m_SampleIndices[ COLD ] )
        return 0;
    nChannels = (s32)(pPackage->m_SampleIndices[ COLD ][ Index+1 ] - pPackage->m_SampleIndices[ COLD ][ Index ]);
    ASSERT( (nChannels > 0) && (nChannels <= 2 ) );
    if( (nChannels <=0) || (nChannels > 2) )
        return 0;

    // Aquire an element for each channel.
    for( i=0 ; i<nChannels ; i++ )
    {
        if( (pElements[ i ] = g_AudioVoiceMgr.AquireElement()) == NULL)
        {
            for( s32 j=0 ; j<i ; j++ )
                if( pElements[ j ] )
                    g_AudioVoiceMgr.ReleaseElement( pElements[ j ], FALSE );

            return 0;
        }
    }

    switch( nChannels )
    {
        case 1:
        {
            pElements[ 0 ]->pStereoElement = NULL;
            break;
        }

        // Stereo
        case 2:
        {
            pElements[ 0 ]->pStereoElement = pElements[ 1 ];
            pElements[ 1 ]->pStereoElement = pElements[ 0 ];
            break;
        }
    }

    // For each channel...
    u32 ColdIndex = (u32)pPackage->m_SampleIndices[ COLD ][ Index ];
    u32 Base      = (u32)pPackage->m_ColdSamples + (ColdIndex * pPackage->m_Header.HeaderSizes[ COLD ]);
    for( i=0 ; i<nChannels ; i++, Base+=pPackage->m_Header.HeaderSizes[ COLD ] )
    {
        element* pElement = pElements[ i ];

        // Put element at end of voices element list.
        g_AudioVoiceMgr.AppendElementToVoice( pElement, pVoice );

        // Get the elements parameters.
        GetElementParameters( &pElement->Params, pDescriptor, pVoice );
        
        switch( nChannels )
        {
            case 1:
            {
                // If pan was specified, then it cannot be changed.
                if( pElement->Params.Bits & PAN_2D )
                    pElement->IsPanChangeable = FALSE;
                else
                    pElement->IsPanChangeable = TRUE;
                break;
            }

            case 2:
            {
                // Can't modify pan on stereo sample.
                pElement->IsPanChangeable = FALSE;
                if( i == 0 )
                    pElement->Params.Pan2d = -1.0f;
                else
                    pElement->Params.Pan2d =  1.0f;
            }
        }

        pElement->DeltaTime          = DeltaTime;
        pElement->State              = ELEMENT_NEEDS_TO_LOAD;
        pElement->Type               = COLD_SAMPLE;
        pElement->Sample.pColdSample = (cold_sample*)Base;

        // Nuke the aram (shouldn't mattter).
        pElement->Sample.pColdSample->AudioRam = 0;

        // Initialize the elements parameters.
        g_AudioVoiceMgr.InitSingleElement( pElement );
    }


    return nChannels;
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendSimple( f32 BaseTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    u32 ElementIndex  = (u32)(*pDescriptor);
    u32 ElementType   = GET_INDEX_TYPE( ElementIndex );
    u32 Index         = GET_INDEX( ElementIndex );

    switch( ElementType )
    {
        case DESCRIPTOR_INDEX:
        {
            // Look up the descriptor in the table...here we go again...WHEEE!!!
            u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
            return AppendDescriptor( BaseTime, pNewDescriptor, pVoice, pPackage );
            break;
        }

        case HOT_INDEX:
        {
            return AppendHot( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }

        case WARM_INDEX:
        {
            return AppendWarm( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }
        
        case COLD_INDEX:
        {
            return AppendCold( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }

        default:
        {
            ASSERT( 0 );
            return 0;
            break;
        }
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendComplex( f32 BaseTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    s32 Result       = 0;
    s32 ElementCount = (s32)(*pDescriptor++);

    // For each element...
    for( s32 i=0 ; i<ElementCount ; i++ )
    {
        u16 U16DeltaTime  = *pDescriptor++;
        f32 DeltaTime     = U16BIT_TO_FLOAT100( U16DeltaTime );
        u32 ElementIndex  = (u32)(*pDescriptor);
        u32 ElementType   = GET_INDEX_TYPE( ElementIndex );
        u32 Index         = GET_INDEX( ElementIndex );
        u32 ParameterSize = 0;
        
        // Does it have parameters?
        if( GET_INDEX_HAS_PARAMS( ElementIndex ) )
        {
            // Get the parameter size (convert from bytes to words)
            ParameterSize = (*(pDescriptor+2)) >> 1;
        }

        switch( ElementType )
        {
            case DESCRIPTOR_INDEX:
            {
                // Look up the descriptor in the table...here we go again...WHEEE!!!
                u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
                Result += AppendDescriptor( BaseTime+DeltaTime, pNewDescriptor, pVoice, pPackage );
                break;
            }

            case HOT_INDEX:
            {
                Result += AppendHot( Index, BaseTime+DeltaTime, pDescriptor, pVoice, pPackage );
                break;
            }

            case WARM_INDEX:
            {
                Result += AppendWarm( Index, BaseTime+DeltaTime, pDescriptor, pVoice, pPackage );
                break;
            }

            case COLD_INDEX:
            {
                Result += AppendCold( Index, BaseTime+DeltaTime, pDescriptor, pVoice, pPackage );
                break;
            }

            default:
            {
                ASSERT( 0 );
                break;
            }
        }

        // Bump past index, flags and parameters.
        pDescriptor += (ParameterSize+2);
    }

    return Result;
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendRandomList( f32 BaseTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    s32  ElementCount    = (s32)(*pDescriptor++);
    u16* pUsedWordBuffer = (u16*)pDescriptor;
    u16* pUsedWords;
    u64  UsedBits;
    s32  i;
    s32  nToPickFrom;
    s32  Choice;
    s32  Shift;
    u32  ElementIndex;
    u32  ElementType;
    u32  Index;
    u32  ParameterSize;
    u8   RandomList[64];

    // Error check
    ASSERT( ElementCount < 64 );
    
    // Construct the used bitfield (data is not 64-bit aligned, *sigh*)
    pUsedWords = pUsedWordBuffer;
    for( i=0, UsedBits=0, Shift=0 ; i<4 ; i++, Shift+=16 )
        UsedBits |= (((u64)(*pUsedWords++)) << Shift);
    
    // Have we used em all? If so, start fresh.
    if( UsedBits == (((u64)1<<ElementCount)-1) )
        UsedBits = 0;

    // Now construct the random list
    for( i=0, nToPickFrom=0 ; i<ElementCount ; i++ )
    {
        if( (UsedBits & 1<<i) == 0 )
            RandomList[ nToPickFrom++ ] = i;
    }

    // Pick one
    Choice = RandomList[ x_irand( 0, nToPickFrom-1 ) ];

    // Mark it used.
    UsedBits |= (1<<Choice);

    // Update the used bits (data *still* isn't 64-bit aligned...)
    pUsedWords = pUsedWordBuffer;
    for( i=0, Shift=0 ; i<4 ; i++, Shift+=16 )
        *pUsedWords++ = (u16)((UsedBits >> Shift) & 0xffff);

    // Now find the one we picked (skip over the used bits first).
    pDescriptor += 4;
    while( Choice-- )
    {
        ElementIndex  = (u32)(*pDescriptor);
        ParameterSize = 0;
        
        // Has parameters?
        if( GET_INDEX_HAS_PARAMS( ElementIndex ) )
        {
            // Get size of parameters in words.
            ParameterSize = (*(pDescriptor+2)) >> 1;
        }

        // Next...
        pDescriptor += (2+ParameterSize);
    }

    ElementIndex = (u32)(*pDescriptor);
    ElementType  = GET_INDEX_TYPE( ElementIndex );
    Index        = GET_INDEX( ElementIndex );

    switch( ElementType )
    {
        case DESCRIPTOR_INDEX:
        {
            // Look up the descriptor in the table...here we go again...WHEEE!!!
            u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
            return AppendDescriptor( BaseTime, pNewDescriptor, pVoice, pPackage );
            break;
        }

        case HOT_INDEX:
        {
            return AppendHot( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }

        case WARM_INDEX:
        {
            return AppendWarm( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }
        
        case COLD_INDEX:
        {
            return AppendCold( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }

        default:
        {
            ASSERT( 0 );
            return 0;
            break;
        }
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendWeightedList( f32 BaseTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{   
    s32  ElementCount  = (s32)(*pDescriptor++);
    u16* pWeights      = (u16*)pDescriptor;
    s32  Choice;
    u32  ElementIndex;
    u32  ElementType;
    u32  Index;
    u32  ParameterSize;
    u16  Weight;

    // Pick a random weight
    Weight = (u16)((x_irand( 0, 0x7fff )*2) & 0xffff);

    // Find the weight
    Choice   = 0;
    while( Choice < ElementCount )
    {
        if( Weight <= *pWeights++ )
            break;
        Choice++;
    }

    // This assert should NEVER fire unless the data gets porked.
    ASSERT( Choice < ElementCount );
    
    // Skip over the weights.
    pDescriptor += ElementCount;

    // Now find the one we picked
    while( Choice-- )
    {
        ElementIndex  = (u32)(*pDescriptor);
        ParameterSize = 0;

        // Has parameters?
        if( GET_INDEX_HAS_PARAMS( ElementIndex ) )
        {
            // Get size of parameters in WORDS.
            ParameterSize = (*(pDescriptor+2)) >> 1;
        }

        // Next!
        pDescriptor += (2+ParameterSize);
    }

    ElementIndex = (u32)(*pDescriptor);
    ElementType  = GET_INDEX_TYPE( ElementIndex );
    Index        = GET_INDEX( ElementIndex );

    switch( ElementType )
    {
        case DESCRIPTOR_INDEX:
        {
            // Look up the descriptor in the table...here we go again...WHEEE!!!
            u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
            return AppendDescriptor( BaseTime, pNewDescriptor, pVoice, pPackage );
            break;
        }

        case HOT_INDEX:
        {
            return AppendHot( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }

        case WARM_INDEX:
        {
            return AppendWarm( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }
        
        case COLD_INDEX:
        {
            return AppendCold( Index, BaseTime, pDescriptor, pVoice, pPackage );
            break;
        }

        default:
        {
            ASSERT( 0 );
            return 0;
            break;
        }
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::AppendDescriptor( f32 BaseTime, u16* pDescriptor, voice* pVoice, audio_package* pPackage )
{
    u32 DescriptorIndex = (u32)(*pDescriptor++);
    u32 DescriptorType  = GET_DESCRIPTOR_TYPE( DescriptorIndex );
    u32 ParameterSize   = 0;
    s32 Result          = 0;

    // Does it have parameters?
    if( GET_DESCRIPTOR_HAS_PARAMS( DescriptorIndex ) )
    {
        // Get parameter size in WORDS (skip over flags to get to the size).
        ParameterSize = *(pDescriptor+1) >> 1;
    }

    // Bump the recursion depth.
    if( ++pVoice->RecursionDepth > MAX_DESCRIPTOR_RECURSION_DEPTH )
    {
        ASSERT( 0 );
    }
    else
    {
        // Bump past the flags and parameters.
        pDescriptor += 1+ParameterSize;

        // Lets build us a voice!
        switch( DescriptorType )
        {
            case SIMPLE:
                Result = AppendSimple( BaseTime, pDescriptor, pVoice, pPackage );
                break;

            case COMPLEX:
                Result = AppendComplex( BaseTime, pDescriptor, pVoice, pPackage );
                break;

            case RANDOM_LIST:
                Result = AppendRandomList( BaseTime, pDescriptor, pVoice, pPackage );
                break;

            case WEIGHTED_LIST:
                Result = AppendWeightedList( BaseTime, pDescriptor, pVoice, pPackage );
                break;

            default:
                ASSERT( 0 );
                Result = 0;
                break;
        }
    }

    // Tell the world.
    --pVoice->RecursionDepth;
    return Result;
}

//------------------------------------------------------------------------------

s32 audio_mgr::IsCold( char* pIdentifier )
{
    audio_package* pPackage;
    u16*           pDescriptor;
    char*          pString;

    // Find the descriptor.
    pDescriptor = FindDescriptorByName( pIdentifier, &pPackage, pString );

    if( pDescriptor )
    {
        return IsDescriptorCold( pDescriptor, pPackage );
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::IsDescriptorCold( u16* pDescriptor, audio_package* pPackage )
{
    u32 DescriptorIndex = (u32)(*pDescriptor++);
    u32 DescriptorType  = GET_DESCRIPTOR_TYPE( DescriptorIndex );
    u32 ParameterSize   = 0;
    s32 Result          = 0;

    // Does it have parameters?
    if( GET_DESCRIPTOR_HAS_PARAMS( DescriptorIndex ) )
    {
        // Get parameter size in WORDS (skip over flags to get to the size).
        ParameterSize = *(pDescriptor+1) >> 1;
    }
    // Bump past the flags and parameters.
    pDescriptor += 1+ParameterSize;

    // Lets build us a voice!
    switch( DescriptorType )
    {
        case SIMPLE:
            Result = IsSimpleCold( pDescriptor, pPackage );
            break;

        case COMPLEX:
            Result = IsComplexCold( pDescriptor, pPackage );
            break;

        case RANDOM_LIST:
            Result = IsRandomListCold( pDescriptor, pPackage );
            break;

        case WEIGHTED_LIST:
            Result = IsWeightedListCold( pDescriptor, pPackage );
            break;

        default:
            ASSERT( 0 );
            Result = 0;
            break;
    }

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

s32 audio_mgr::IsSimpleCold( u16* pDescriptor, audio_package* pPackage )
{
    u32 ElementIndex  = (u32)(*pDescriptor);
    u32 ElementType   = GET_INDEX_TYPE( ElementIndex );
    u32 Index         = GET_INDEX( ElementIndex );

    switch( ElementType )
    {
        case DESCRIPTOR_INDEX:
        {
            // Look up the descriptor in the table...here we go again...WHEEE!!!
            u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
            return IsDescriptorCold( pNewDescriptor,pPackage );
            break;
        }

        case HOT_INDEX:
            return 0;

        case WARM_INDEX:
        case COLD_INDEX:
            return 1;


        default:
            ASSERT( 0 );
            return 0;
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::IsComplexCold( u16* pDescriptor, audio_package* pPackage )
{
    s32 Result       = 0;
    s32 ElementCount = (s32)(*pDescriptor++);

    // For each element...
    for( s32 i=0 ; i<ElementCount ; i++ )
    {
        // Skip over time.
        pDescriptor++;

        u32 ElementIndex  = (u32)(*pDescriptor);
        u32 ElementType   = GET_INDEX_TYPE( ElementIndex );
        u32 Index         = GET_INDEX( ElementIndex );
        u32 ParameterSize = 0;

        // Does it have parameters?
        if( GET_INDEX_HAS_PARAMS( ElementIndex ) )
        {
            // Get the parameter size (convert from bytes to words)
            ParameterSize = (*(pDescriptor+2)) >> 1;
        }

        switch( ElementType )
        {
            case DESCRIPTOR_INDEX:
            {
                // Look up the descriptor in the table...here we go again...WHEEE!!!
                u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
                if( IsDescriptorCold( pNewDescriptor, pPackage ) )
                    return 1;
                break;
            }

            case HOT_INDEX:
                break;

            case WARM_INDEX:
            case COLD_INDEX:
                return 1;

            default:
                ASSERT( 0 );
                break;
        }

        // Bump past index, flags and parameters.
        pDescriptor += (ParameterSize+2);
    }

    return Result;
}

//------------------------------------------------------------------------------

s32 audio_mgr::IsRandomListCold( u16* pDescriptor, audio_package* pPackage )
{
    s32  ElementCount = (s32)(*pDescriptor++);
    s32  i;
    u32  ElementIndex;
    u32  ElementType;
    u32  Index;
    u32  ParameterSize;

    // Error check
    ASSERT( ElementCount < 64 );

    // Skip over the used bits first.
    pDescriptor += 4;

    for( i=0 ; i<ElementCount ; i++ )
    {
        ElementIndex  = (u32)(*pDescriptor);
        ParameterSize = 0;
        ElementType   = GET_INDEX_TYPE( ElementIndex );
        Index         = GET_INDEX( ElementIndex );

        switch( ElementType )
        {
            case DESCRIPTOR_INDEX:
            {
                // Look up the descriptor in the table...here we go again...WHEEE!!!
                u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
                if( IsDescriptorCold( pNewDescriptor, pPackage ) )
                    return 1;
                break;
            }

            case HOT_INDEX:
                break;

            case WARM_INDEX:
            case COLD_INDEX:
                return 1;

            default:
                ASSERT( 0 );
                return 0;
        }

        // Has parameters?
        if( GET_INDEX_HAS_PARAMS( ElementIndex ) )
        {
            // Get size of parameters in words.
            ParameterSize = (*(pDescriptor+2)) >> 1;
        }

        // Next...
        pDescriptor += (2+ParameterSize);
    }

    return 0;
}

//------------------------------------------------------------------------------

s32 audio_mgr::IsWeightedListCold( u16* pDescriptor, audio_package* pPackage )
{    
    s32  ElementCount = (s32)(*pDescriptor++);
    s32  i;
    u32  ElementIndex;
    u32  ElementType;
    u32  Index;
    u32  ParameterSize;

    // Skip over the weights.
    pDescriptor += ElementCount;

    for( i=0 ; i<ElementCount ; i++ )
    {
        ElementIndex  = (u32)(*pDescriptor);
        ParameterSize = 0;
        ElementType   = GET_INDEX_TYPE( ElementIndex );
        Index         = GET_INDEX( ElementIndex );

        switch( ElementType )
        {
            case DESCRIPTOR_INDEX:
            {
                // Look up the descriptor in the table...here we go again...WHEEE!!!
                u16* pNewDescriptor = (u16*)pPackage->m_DescriptorTable[ Index ];
                if( IsDescriptorCold( pNewDescriptor, pPackage ) )
                    return 1;
                break;
            }

            case HOT_INDEX:
                break;

            case WARM_INDEX:
            case COLD_INDEX:
                return 1;

            default:
                ASSERT( 0 );
                return 0;
        }

        // Has parameters?
        if( GET_INDEX_HAS_PARAMS( ElementIndex ) )
        {
            // Get size of parameters in words.
            ParameterSize = (*(pDescriptor+2)) >> 1;
        }

        // Next...
        pDescriptor += (2+ParameterSize);
    }

    return 0;
}

//------------------------------------------------------------------------------

void audio_mgr::UnloadAllPackages(void)
{
#if 0
    audio_package::package_link* pLink;
    audio_package::package_link* pNext;

    // Get first package in the list.
    pLink = m_Link.pNext;

    // While we have packages...
    while( pLink->pPackage )
    {
        pNext = m_Link.pNext;
        UnloadPackage( pLink->pPackage->m_Filename );
        pLink = pNext;
    }

#else 
    // who put this code here?
    while (m_Link.pNext->pPackage)
        UnloadPackage(m_Link.pNext->pPackage->m_Filename);
#endif
}

//------------------------------------------------------------------------------

f32 audio_mgr::ComputeFalloff( f32 PercentToFarClip, s32 TableID )
{
    ASSERT( TableID >= 0 );
    ASSERT( TableID < NUM_ROLLOFFS );
    ASSERTS( PercentToFarClip >= 0.0f, xfs("%f", PercentToFarClip) );
    ASSERTS( PercentToFarClip <= 1.0f, xfs("%f", PercentToFarClip) );

    if( PercentToFarClip <= 0.0f )
    {
        return( s_FalloffTables[TableID][0] );
    }
    else if( PercentToFarClip >= 1.0f )
    {
        return( s_FalloffTables[TableID][FALLOFF_TABLE_SIZE-1] );
    }
    else
    {
        s32 i;
        f32 f;
        f32 d;

        // Convert from [0..1] to [0..FALLOFF_TABLE_SIZE-1]
        f = PercentToFarClip * (f32)(FALLOFF_TABLE_SIZE-1);

        // Snag integer portion.
        i = (s32)f;

        // Compute delta.
        d = f - (f32)i;

        // Tell the world...
        return (s_FalloffTables[TableID][i] * (1.0f - d)) + (s_FalloffTables[TableID][i+1] * d);
    }
}

//------------------------------------------------------------------------------

void audio_mgr::Calculate3dVolume( f32               NearClipOrig, 
                                   f32               FarClipOrig, 
                                   s32               VolumeRolloff,  
                                   const vector3&    WorldPosition, 
                                   s32               ZoneID,
                                   f32&              Volume )
{
    audio_mgr_ear* pEar = m_pUsedEars;

    // Init.
    Volume = 0.0f;

    // Do it for all the ears...
    while( pEar )
    {
        f32 NearClip = NearClipOrig;
        f32 FarClip  = FarClipOrig;
        f32 EarVolume;

        // Put sound in ear-space, get distance to sound squared
        vector3 Position  = pEar->W2V * WorldPosition;
        f32     Distance2 = Position.LengthSquared();

        // Calculate the actual far clip plane.
        FarClip *= m_FarClip;

        // Is sound beyond the far clip?
        if( Distance2 >= FarClip*FarClip )
        {
            // Can't hear it...
            EarVolume = 0.0f;
        }
        else
        {
            // Calculate the actual near clip plane.
            NearClip *= m_NearClip;
        
            // Sound inside the near clip? Is the near clip too close to (or beyond) the far clip?
            if( (Distance2 <= NearClip*NearClip) || ((NearClip+1.0f) >= FarClip) )
            {
                // Full volume.
                EarVolume = 1.0f;
            }
            else
            {
                // Calculate the actual distance to the sound.
                f32 Distance = x_sqrt( Distance2 );
        
                // Calculate percentage to the far clip.
                f32 Percent = (Distance-NearClip) / (FarClip-NearClip);

                // Calculate the distance attenuation.
                EarVolume = ComputeFalloff( Percent, VolumeRolloff );
            }
        }

        // Include the ears volume in the calculation.
        EarVolume *= pEar->Volume;

        // Now take zone volume into account
        EarVolume *= pEar->ZoneVolumes[ ZoneID ];

        // Is this ear more dominant? (= is required, do NOT remove it!)
        if( EarVolume >= Volume )
            Volume = EarVolume;

        // Walk the list.
        pEar = pEar->pNext;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::Calculate2dPan( f32      Pan2d,
                                vector4& Pan3d )
{
    s32     i;

    // Convert to [-90..90]
    Pan2d *= 90;

    i = (s32)Pan2d;
    if( i < -90 )
        i = -90;
    if( i > 90 )
        i = 90;

    // Get the stereo pan.
    Pan3d = m_StereoPan[ i+90 ];
}

f32 s_EarVolume       = 0.0f;
f32 s_ZoneVolume      = 0.0f;
s32 s_ZoneID          = 0;
f32 s_ZoneFinalVolume = 0.0f;

//------------------------------------------------------------------------------

void audio_mgr::Calculate3dVolumeAndPan( f32            NearClipOrig, 
                                         f32            FarClipOrig, 
                                         s32            VolumeRolloff,  
                                         f32            NearDiffusion, 
                                         f32            FarDiffusion, 
                                         const vector3& WorldPosition,
                                         s32            ZoneID,
                                         f32&           Volume, 
                                         vector4&       Pan,
                                         s32&           DegreesToSound,
                                         s32&           PrevDegreesToSound,
                                         s32            EarID )
{
    xbool          bDoOnce = FALSE;
    audio_mgr_ear* pEar    = m_pUsedEars;
    audio_mgr_ear* pBest   = NULL;
    vector3        Position;

    // Init.
    Volume = 0.0f;

    // Do it for all the ears (if no ear specified!)
    if( EarID )
    {
        pEar    = IdToEar( EarID );
        bDoOnce = TRUE;
    }

    if( !pEar )
    {
        // No ear?  Very bad.  Oh, well.  Life goes on.
        LOG_ERROR( "audio_mgr::Calculate3dVolumeAndPan", "No ear for 3D sound." );
        return;
    }

    while( pEar )
    {
		f32 NearClip = NearClipOrig;
		f32 FarClip  = FarClipOrig; 
        f32 EarVolume;

        // Put sound in ear-space.
        vector3 EarPosition = pEar->W2V * WorldPosition;
    
        // Get distance to sound squared.
        f32 Distance2 = EarPosition.LengthSquared();

        // Calculate the actual far clip plane.
        FarClip *= m_FarClip;

        // Is sound beyond the far clip?
        if( Distance2 >= FarClip*FarClip )
        {
            // Can't hear it...
            EarVolume = 0.0f;
        }
        else
        {
            f32 Distance;
            f32 Percent;

            // Calculate the actual near clip plane.
            NearClip *= m_NearClip;
    
            // Sound inside the near clip? Is the near clip too close too (or beyond) the far clip?
            if( (Distance2 <= NearClip*NearClip) || ((NearClip+1.0f) >= FarClip) )
            {
                // Full volume.
                EarVolume = 1.0f;
            }
            else
            {
                // Calculate the actual distance to the sound.
                Distance = x_sqrt( Distance2 );
    
                // Calculate percentage to the far clip.
                Percent = (Distance-NearClip) / (FarClip-NearClip);

                // Calculate the distance attenuation.
                EarVolume = ComputeFalloff( Percent, VolumeRolloff );
            }
        }

        // Inlude the ears volume in the calculation.
        EarVolume *= pEar->Volume;

        s_EarVolume  = EarVolume;
        s_ZoneVolume = pEar->ZoneVolumes[ ZoneID ];
        s_ZoneID     = ZoneID;

        // Now take zone volumes into consideration.
        EarVolume *= pEar->ZoneVolumes[ ZoneID ];

        s_ZoneFinalVolume = EarVolume;

        // More dominant? (= is required, do NOT remove it!)
        if( EarVolume >= Volume )
        {
            Volume   = EarVolume;
            pBest    = pEar;
            Position = EarPosition;
        }

        // Single ear or search entire list?
        if( bDoOnce )
        {
            // We are DONE!
            pEar = NULL;
        }
        else
        {
            // Next ear!
            pEar = pEar->pNext;
        }
    }

#ifndef X_EDITOR
    // Better have one...
    ASSERT( pBest );
#endif

    if( pBest )
    {
        // Now, re-calculate distance2 ignoring elevation.  This will make 
        // a sound directly overhead sound like a 100 percent diffuse sound.
        // Given that the current systems don't support speakers on the ceiling, 
        // this is the best approximation available.
        f32 Distance2 = Position.GetX()*Position.GetX() + Position.GetZ()*Position.GetZ();

        // Calculate the actual near and far diffusion plane.
        NearDiffusion *= m_NearClip;
        FarDiffusion  *= m_FarClip;

        // Inside the near diffusion plane? 
        if( Distance2 <= (NearDiffusion*NearDiffusion) )
        {
            // 100 percent diffuse...
            Pan = m_Diffuse;

            // Initialize em...
            DegreesToSound     = -1;
            PrevDegreesToSound = 0;
        }
        else
        {
            // Calculate the actual distance in the XZ plane.
            f32 Distance = x_sqrt( Distance2 );

            // Calculate "angle from the ear to the sound" *IN SPEAKER SPACE*, where: 
            //
            //   0 degrees is out of your nose.
            // -90 degrees is off your left shoulder.
            // +90 degrees is off your right shoulder.
            //
            f32 Angle = RAD_TO_DEG( x_atan2( -Position.GetX(), Position.GetZ() ) );
            while( Angle < 0.0f )
                Angle += 360.0f;
            while( Angle >= 360.0f )
                Angle -= 360.0f;
            s32 i = (s32)Angle;
            ASSERT( i>=0 );
            ASSERT( i<360 );

            // Set the degrees to the sound.
            DegreesToSound = i;

            // Get Delta to new sound position and and apply to old position % 720
            radian Delta = x_MinAngleDiff( DEG_TO_RAD(DegreesToSound), DEG_TO_RAD(PrevDegreesToSound % 360) );
            DegreesToSound = PrevDegreesToSound + (s32)(RAD_TO_DEG(Delta));
            while( DegreesToSound <    0 ) DegreesToSound += 720;
            while( DegreesToSound >= 720 ) DegreesToSound -= 720;

            // Store off position as previous position
            PrevDegreesToSound = DegreesToSound;
                                
            // Is sound beyond the diffusion far plane? Is the near diffusion plane too close to (or beyond) the far diffusion plane?
            if( Distance >= FarDiffusion || ((NearDiffusion+1.0f) >= FarDiffusion) )
            {
                // Yes? Its 100 percent directional then...
                Pan = m_Pan[i]; 
            }
            else
            {
/*
                // Calculate the percentage of diffusion.
                f32 Percent = 1.0f - (Distance-NearDiffusion) / (FarDiffusion-NearDiffusion);

                // Scale it based on percent diffusion.
                Pan = m_Pan[i] + Percent * (m_Diffuse - m_Pan[i]);

                // Normalize it to maintain constant power.
                Pan.Normalize(); 
*/
				// Hack to test!
				Pan = m_Pan[i]; 
            }
        }
    }
}

//------------------------------------------------------------------------------

voice_id audio_mgr::Play( const char* pIdentifier, xbool AutoStart )
{
    vector3  Dummy;
    vector3& DummyRef = Dummy;

    return PlayInternal( pIdentifier, AutoStart, DummyRef, -1, FALSE, FALSE );
}

//------------------------------------------------------------------------------

voice_id audio_mgr::Play( const char* pIdentifier, const vector3& Position, s32 ZoneID, xbool AutoStart )
{
    return PlayInternal( pIdentifier, AutoStart, Position, ZoneID, TRUE, FALSE );
}

//------------------------------------------------------------------------------

voice_id audio_mgr::PlayVolumeClipped( const char* pIdentifier, const vector3& Position, s32 ZoneID, xbool AutoStart )
{
    return PlayInternal( pIdentifier, AutoStart, Position, ZoneID, TRUE, TRUE );
}

//------------------------------------------------------------------------------

voice_id audio_mgr::Play( const char* pIdentifier, const vector3& Position, s32 ZoneID, xbool AutoStart, xbool VolumeClip )
{
    return PlayInternal( pIdentifier, AutoStart, Position, ZoneID, TRUE, VolumeClip );
}

//------------------------------------------------------------------------------

xbool DEBUG_PLAY_IDENTIFIER_NOT_FOUND = 0;
xbool DEBUG_PLAY_ACQUIRE_VOICE_FAILED = 0;
xbool DEBUG_PLAY_VOLUME_CLIPPED       = 0;
xbool DEBUG_PLAY_SUCCESS              = 0;
xbool DEBUG_FILTER_FOOTFALL           = 0;
xbool DEBUG_FILTER_VOX                = 0;

voice_id audio_mgr::PlayInternal( const char*    pIdentifier, 
                          xbool          AutoStart, 
                          const vector3& Position, 
                          s32            ZoneID,
                          xbool          IsPositional, 
                          xbool          bVolumeClip )
{
    (void)ZoneID;
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0;
#else
    ASSERT( s_Initialized );
#endif

    audio_package* pPackage;
    u16*           pDescriptor;
    //s32            Index;
    f32            PositionalVolume = 1.0f;
    f32            UserVolume       = 1.0f;
    char*          DescriptorName   = NULL;

    #if !defined(X_RETAIL) || defined(X_QA)
    xbool bDebug = TRUE;
    if( DEBUG_FILTER_FOOTFALL || DEBUG_FILTER_VOX )
    {
        bDebug = FALSE;
        if( DEBUG_FILTER_FOOTFALL && x_stristr( pIdentifier, "FOOTFALL" ) )
            bDebug = TRUE;
        if( DEBUG_FILTER_FOOTFALL && x_stristr( pIdentifier, "FF_" ) )
            bDebug = TRUE;
        else if( DEBUG_FILTER_VOX && x_stristr( pIdentifier, "VOX" ) )
            bDebug = TRUE;
        else if( DEBUG_FILTER_VOX && x_stristr( pIdentifier, "PAINGRUNT" ) )
            bDebug = TRUE;
    }
    #endif // !defined(X_RETAIL)

    if( s_bDisableAudio )
    {
        return 0;
    }

#ifdef TRAP_ON_IDENTIFIER
    if( g_EnableIdentifierTrap && (x_stricmp( pIdentifier, g_DebugIdentifier ) == 0) )
        BREAK;
#endif

    // Find the decriptor by name.
    pDescriptor = FindDescriptorByName( pIdentifier, &pPackage, DescriptorName );

    // Did not find it?            
    if( pDescriptor )
    {
        uncompressed_parameters Params;
        voice*                  pVoice;
        f32                     AbsoluteVolume;

        // Decode the voices parameters, this is required to determine the priority and volume.
        GetVoiceParameters( &Params, pDescriptor, pPackage, (char*)pIdentifier );
        
        // Is it positional?
        if( IsPositional )
        {
            // Calculate the falloffs.
            f32 Near = Params.NearFalloff * pPackage->GetComputedNearFalloff();
            f32 Far  = Params.FarFalloff  * pPackage->GetComputedFarFalloff();

            // Calculate the 3d volume.
            g_AudioMgr.Calculate3dVolume( Near, Far, Params.RolloffCurve, Position, ZoneID, PositionalVolume );
            
            // Is this sound volume clipped?
            if( bVolumeClip )
            {
                // Is it *REALLY* quiet...
                if( PositionalVolume <= 0.05f )
                {
                    // I'm sorry cap'n but I canna play this...the dilithium crystals are cracked!
                    #ifdef LOG_PLAY_CLIPPED            
                    LOG_MESSAGE( LOG_PLAY_CLIPPED, "'%s' was VOLUME CLIPPED!", pIdentifier );
                    #endif // LOG_PLAY_CLIPPED                   
                    
                    #if defined(ENABLE_AUDIO_DEBUG)
                    if( DEBUG_PLAY_VOLUME_CLIPPED && bDebug )
                        AudioDebug( xfs("'%s' was VOLUME CLIPPED!\n", pIdentifier) );
                    #endif //!defined(X_RETAIL)

                    return 0;
                }
            }
        }

        // Calculate voices volume.
        AbsoluteVolume = PositionalVolume * UserVolume * Params.Volume * pPackage->m_Volume;

        // TODO: Put in master fader calculation - apply it to AbsoluteVolume.

        // Attempt to aquire a voice.
        pVoice = g_AudioVoiceMgr.AcquireVoice( Params.Priority, AbsoluteVolume );
        if( pVoice )
        {
#if defined(rbrannon) && defined(TRAP_ON_IDENTIFIER)
            if( g_EnableIdentifierTrap && (x_stricmp( pIdentifier, g_DebugIdentifier ) == 0) )
            {
                if( g_DebugVoice == NULL )
                {
                    g_EnableIdentifierTrap = 0;
                    g_DebugVoice = pVoice;
                    LOG_MESSAGE( "AudioDebug(audio_mgr::Play)",
                        "Trapped: %s, pVoice: %08x",
                        pIdentifier,
                        pVoice );
                }
            }
#endif
            #ifdef LOG_PLAY_SUCCESS
            LOG_MESSAGE( LOG_PLAY_SUCCESS, "pVoice: 0x%08x [Id:%08x] '%s'", pVoice, VoiceToId(pVoice), pIdentifier );
            #endif //LOG_PLAY_SUCCESS

            #if defined(ENABLE_AUDIO_DEBUG)
            if( DEBUG_PLAY_SUCCESS && bDebug )
                AudioDebug( xfs("'%s' success!!\n", pIdentifier)  );
            #endif //!defined(X_RETAIL)
                
            // Save descriptor name
            pVoice->pDescriptorName = DescriptorName;

            // Now that we have a voice, copy the parameters.
            pVoice->Params = Params;

            // Default is no ear specified!
            pVoice->EarID = 0;

            // Positional sound?
			if( IsPositional )
			{
                // Set the voices position.
				pVoice->Position = Position;
                pVoice->ZoneID   = ZoneID;
			}
            else
            {
                // Not positional so use the 2d pan.
                g_AudioMgr.Calculate2dPan( pVoice->Params.Pan2d, pVoice->Params.Pan3d );
            }

            // If pan was specified, then it cannot be changed.
            if( pVoice->Params.Bits & PAN_2D && !IsPositional )
                pVoice->IsPanChangeable = FALSE;
            else
                pVoice->IsPanChangeable = TRUE;

            // Force voice positional flag until parameters have been set.
            pVoice->IsPositional = TRUE;
            g_AudioVoiceMgr.SetVoiceUserFalloff( pVoice, 1.0f, 1.0f );
            g_AudioVoiceMgr.SetVoiceUserDiffuse( pVoice, 1.0f, 1.0f );
            pVoice->IsPositional = IsPositional;

            // Set the voices parameters.
            g_AudioVoiceMgr.SetVoiceUserVolume( pVoice, UserVolume );
            g_AudioVoiceMgr.SetVoiceUserPitch( pVoice, 1.0f );
            g_AudioVoiceMgr.SetVoiceUserEffectSend( pVoice, 1.0f );
            
            // Ok, now initialize the voice.
            g_AudioVoiceMgr.InitSingleVoice( pVoice, pPackage );
            
            // Set the voices descriptor.
            pVoice->pDescriptor = pDescriptor;
            
            // Append the descriptor to the voices element list.
            if( AppendDescriptor( 0.0f, pDescriptor, pVoice, pPackage ) )
            {
                // Start the voice.
                if( AutoStart )
                    g_AudioVoiceMgr.StartVoice( pVoice );

                // Its all good!
                return( VoiceToId( pVoice ) );
            }
            else
            {
                #if defined(ENABLE_AUDIO_DEBUG)
                if( DEBUG_PLAY_ACQUIRE_VOICE_FAILED && bDebug )
                    AudioDebug( xfs("'%s' could not acquire voice! (element error)\n", pIdentifier) );
                #endif //!defined(X_RETAIL)

                // Had problems, no elements in voice, so release the voice...
                g_AudioVoiceMgr.ReleaseVoice( pVoice, 0.0f );
            }
        }
        else
        {
            #ifdef LOG_PLAY_FAILURE
            LOG_MESSAGE( LOG_PLAY_FAILURE, "'%s' could not acquire voice!", pIdentifier );
            #endif // LOG_PLAY_FAILURE            

            #if defined(ENABLE_AUDIO_DEBUG)
            if( DEBUG_PLAY_ACQUIRE_VOICE_FAILED && bDebug )
                AudioDebug( xfs("'%s' could not acquire voice! (voice error)\n", pIdentifier) );
            #endif //!defined(X_RETAIL)
        }
    }
    else
    {
        #ifdef LOG_PLAY_WARNING
        LOG_WARNING( LOG_PLAY_WARNING, "'%s' identifier NOT FOUND!!!", pIdentifier );
        #endif // LOG_PLAY_WARNING

        #if defined(ENABLE_AUDIO_DEBUG)
        if( DEBUG_PLAY_IDENTIFIER_NOT_FOUND && bDebug )
            AudioDebug( xfs("'%s' identifier NOT FOUND!!!\n", pIdentifier) );
        #endif //!defined(X_RETAIL)
    }
   
    return 0;
}

//------------------------------------------------------------------------------

xbool audio_mgr::Segue( voice_id VoiceID, voice_id VoiceToQ )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    voice* pVoice    = IdToVoice( VoiceID );
    voice* pVoiceToQ = IdToVoice( VoiceToQ );
    if( pVoice )
    {
        return g_AudioVoiceMgr.Segue( pVoice, pVoiceToQ );
    }
    else
    {
        return FALSE;
    }
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetReleaseTime( voice_id VoiceID, f32 Time )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    voice* pVoice = IdToVoice( VoiceID );
    if( pVoice )
    {
        return g_AudioVoiceMgr.SetReleaseTime( pVoice, Time );
    }
    else
    {
        return FALSE;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::Pause( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    g_AudioVoiceMgr.PauseVoice( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

void audio_mgr::PauseAll( void )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    g_AudioVoiceMgr.PauseAllVoices();
    Update( 0.015f ); // Run an update
    x_DelayThread( 15 );
    Update( 0.015f ); // Run an update
    x_DelayThread( 15 );
}

//------------------------------------------------------------------------------

void audio_mgr::Resume( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    g_AudioVoiceMgr.ResumeVoice( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

void audio_mgr::ResumeAll( void )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    while( g_IoMgr.GetDeviceQueueStatus( IO_DEVICE_DVD ) )
    {
        x_DelayThread( 10 );
//        Update( 10 );
    }
    g_AudioVoiceMgr.ResumeAllVoices();
    Update( 0.015f ); // Run an update
    x_DelayThread( 15 );
    Update( 0.015f ); // Run an update
    x_DelayThread( 15 );
}

//------------------------------------------------------------------------------
/*
void audio_mgr::ReleaseLoop( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    g_AudioVoiceMgr.ReleaseVoiceLoop( IdToVoice( VoiceID ) );
}
*/
//------------------------------------------------------------------------------

void audio_mgr::Release( voice_id VoiceID, f32 Time )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    g_AudioVoiceMgr.ReleaseVoice( IdToVoice( VoiceID ), Time );
}

//------------------------------------------------------------------------------

void audio_mgr::ReleaseAll( void )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    g_AudioVoiceMgr.ReleaseAllVoices();

    // Update
    Update( 1.0f );

    // Wait a bit...
    x_DelayThread( 15 );

    // Update
    Update( 1.0f );

    // Wait just a bit (make sure the hardware updates run).
    x_DelayThread( 15 );
}

//------------------------------------------------------------------------------

void audio_mgr::GetLoadedPackages( xarray<xstring>& Packages )
{
    ASSERT( s_Initialized );

    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    while( pLink->pPackage )
    {
        // search backwards looking for a \ or /
        s32   i = x_strlen( pLink->pPackage->m_Filename );
        char* p = &pLink->pPackage->m_Filename[ i ];

        while( (i--) && (*(p-1) != '\\') && (*(p-1) != '/') )
            p--;
        
        // Put it in the list
        Packages.Append( (xstring)p );

        // Walk the list.
        pLink = pLink->pNext;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::DisplayPackages()
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return;
#else
    ASSERT( s_Initialized );
#endif

    ASSERT( s_Initialized );

    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    while( pLink->pPackage )
    {
#ifdef ENABLE_AUDIO_DEBUG
        AudioDebug( (const char*)xfs("%s\n", pLink->pPackage->m_Filename ) );
#endif // ENABLE_AUDIO_DEBUG
        x_DebugMsg( (const char*)xfs("%s\n", pLink->pPackage->m_Filename ) );
        // Walk the list.
        pLink = pLink->pNext;
    }
}

//------------------------------------------------------------------------------

xbool audio_mgr::Start( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    voice* pVoice = IdToVoice( VoiceID );
    g_AudioVoiceMgr.StartVoice( pVoice );
    return( pVoice != NULL );
}

//------------------------------------------------------------------------------

xbool audio_mgr::IsReleasing( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.IsVoiceReleasing( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

xbool audio_mgr::IsValidVoiceId( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return( IdToVoice( VoiceID ) != NULL );
}

//------------------------------------------------------------------------------

xbool audio_mgr::IsVoiceReady( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.IsVoiceReady( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

const char* audio_mgr::GetVoiceDescriptor( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return "NULL";
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetVoiceDescriptor( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetVolume( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetVoiceUserVolume( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetVolume( voice_id VoiceID, f32 Volume )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.SetVoiceUserVolume( IdToVoice( VoiceID ), Volume );
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetPan( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetVoicePan( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetPan( voice_id VoiceID, f32 Pan )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.SetVoicePan( IdToVoice( VoiceID ), Pan );
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetPitch( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetVoiceUserPitch( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetPitch( voice_id VoiceID, f32 Pitch )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    if( Pitch < 0.99f )
        LOG_MESSAGE( "audio_mgr::SetPitch", "pVoice: %08x, Pitch: %f", IdToVoice( VoiceID ), Pitch );
    return g_AudioVoiceMgr.SetVoiceUserPitch( IdToVoice( VoiceID ), Pitch );
}

//------------------------------------------------------------------------------

xbool audio_mgr::GetPosition( voice_id VoiceID, vector3& Position, s32& ZoneID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetVoicePosition( IdToVoice( VoiceID ), Position, ZoneID );
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetPosition( voice_id VoiceID, const vector3& Position, s32 ZoneID )
{
    // Error check.
    ASSERT( s_Initialized );
    return g_AudioVoiceMgr.SetVoicePosition( IdToVoice( VoiceID ), Position, ZoneID );
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetFalloff( voice_id VoiceID, f32 Near, f32 Far )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.SetVoiceUserFalloff( IdToVoice( VoiceID ), Near, Far );
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetEffectSend( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetVoiceUserEffectSend( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

xbool audio_mgr::SetEffectSend( voice_id VoiceID, f32 EffectSend )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.SetVoiceUserEffectSend( IdToVoice( VoiceID ), EffectSend );
}

//------------------------------------------------------------------------------

xbool audio_mgr::HasLipSync( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.HasLipSync( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetLipSync( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetLipSync( IdToVoice( VoiceID ) ) * 2.0f;
}

//------------------------------------------------------------------------------

s32 audio_mgr::GetBreakPoints( voice_id VoiceID, f32* &BreakPoints )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetBreakPoints( IdToVoice( VoiceID ), BreakPoints );
}

//------------------------------------------------------------------------------

xbool audio_mgr::GetIsReady( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return FALSE;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetIsReady( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetCurrentPlayTime( voice_id VoiceID )
{
    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return 0.0f;
#else
    ASSERT( s_Initialized );
#endif

    return g_AudioVoiceMgr.GetCurrentPlayTime( IdToVoice( VoiceID ) );
}

//------------------------------------------------------------------------------

s32 audio_mgr::GetPackageARAM( const char* pName )
{
    ASSERT( s_Initialized );

    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    while( pLink->pPackage )
    {
        // search backwards looking for a \ or /
        s32   i = x_strlen( pLink->pPackage->m_Filename );
        char* p = &pLink->pPackage->m_Filename[ i ];

        while( (i--) && (*(p-1) != '\\') && (*(p-1) != '/') )
            p--;

        if( x_stricmp( pName, p ) == 0 )
        {
            return pLink->pPackage->m_Header.Aram;
        }

        // Walk the list.
        pLink = pLink->pNext;
    }

    return 0;
}

//------------------------------------------------------------------------------

audio_package* audio_mgr::FindPackageByName( const char* pFilename )
{
    audio_package::package_link* pLink;

    // Error check.
#ifndef AUDIO_ENABLE
    if( !s_Initialized )
        return NULL;
#else
    ASSERT( s_Initialized );
#endif

    ASSERT( pFilename );

    // Get first package in the list.
    pLink = m_Link.pNext;

    // While we have packages...
    while( pLink->pPackage )
    {
        // Names match?
        if( !x_strncmp( pFilename, pLink->pPackage->m_Filename, AUDIO_PACKAGE_FILENAME_LENGTH ) )
        {
            // All good!
            return pLink->pPackage;
        }             

        // Walk the list.
        pLink = pLink->pNext;
    }

    // Oops...couldn't find it...
    return NULL;
}

//------------------------------------------------------------------------------

char* audio_mgr::GetMusicType( const char* pFilename )
{
    audio_package* pPackage = FindPackageByName( pFilename );
    if( pPackage )
    {
        return pPackage->GetMusicType();
    }
    else
    {
        return NULL;
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::GetMusicIntensity( const char* pFilename, music_intensity* &Intensity )
{
    audio_package* pPackage = FindPackageByName( pFilename );
    if( pPackage )
    {
        return pPackage->GetMusicIntensity( Intensity );
    }
    else
    {
        Intensity = NULL;
        return      0;
    }
}


//------------------------------------------------------------------------------

void audio_mgr::ReMergeIdentifierTables( void )
{
    m_pIdentifiers.Clear();
    m_pIdentifiers.SetCapacity( 0 );
    m_pIdentifiers.FreeExtra();
    MergeIdentifierTables();
}

//------------------------------------------------------------------------------

static s32 s_nMerges = 0;

void audio_mgr::MergeIdentifierTables( void )
{
    CONTEXT("audio_mgr::MergeIdentifierTables");

    audio_package::package_link* pLink;
    xarray<audio_package*>       pPackages;
    xarray<s32>                  PackageIndices;
    s32                          i;

    // Nuke the identifer table
    m_pIdentifiers.Clear();
    pPackages.Clear();
    PackageIndices.Clear();

    // Calculate the size of the table
    pLink = m_Link.pNext;
    s32 TotalIdentifiers = 0;
    while( pLink->pPackage )
    {
        // If the package is loaded
        if( pLink->pPackage->m_IsLoaded )
        {
            // Add this package to the list
            pPackages.Append() = pLink->pPackage;

            // Initialize the number of indices processd to 0.
            PackageIndices.Append() = 0;

            // Keep track of total number of identifiers
            TotalIdentifiers += pLink->pPackage->m_Header.nIdentifiers;
        }

        // Walk the package list.
        pLink = pLink->pNext;
    }

    // Set the size of the identifier table.
    m_pIdentifiers.SetCapacity( TotalIdentifiers );

    // Perform the merge sort.
    while( pPackages.GetCount() )
    {
        i = 0;
        for( s32 j=1 ; j<pPackages.GetCount() ; j++ )
        {
            char* pSmallest;
            char* pCurrent;
            u32   SmallestOffset;
            u32   CurrentOffset;
            s32   Result;

            // Calculate the string table offsets.
            SmallestOffset = pPackages[ i ]->m_IdentifierTable[ PackageIndices[ i ] ].StringOffset;
            CurrentOffset  = pPackages[ j ]->m_IdentifierTable[ PackageIndices[ j ] ].StringOffset;

            // Get pointer to the string.
            pSmallest = pPackages[ i ]->m_IdentifierStringTable + SmallestOffset;
            pCurrent  = pPackages[ j ]->m_IdentifierStringTable + CurrentOffset;

            Result = x_strcmp( pCurrent, pSmallest );
            if( Result < 0 )
            {
                // It's smaller!
                i = j;
            }
            else if( Result == 0 )
            {
                // BAD! Had a name collision
                // TODO: Put in warning...
            }
        }

        // Merge 'em
        m_pIdentifiers.Append() = &pPackages[ i ]->m_IdentifierTable[ PackageIndices[ i ] ];
        PackageIndices[ i ]++;
        if( PackageIndices[ i ] >= pPackages[ i ]->m_Header.nIdentifiers )
        {
            pPackages.Delete( i );
            PackageIndices.Delete( i );
        }
    }
/*
    X_FILE* f = x_fopen( xfs( "ident%03d.txt", s_nMerges ), "w+t" );
    if( f )
    {
        descriptor_identifier* pIdentifier;
        audio_package*         pPackage;
        char*                  pString;

        for( i=0 ; i<m_pIdentifiers.GetCount() ; i++ )
        {
            // Get package, index and offset from table
            pIdentifier = m_pIdentifiers[ i ];
            pPackage    = pIdentifier->pPackage;
            pString     = pPackage->m_IdentifierStringTable + pIdentifier->StringOffset;
            x_fprintf( f, "%s\n", pString );
        }

        x_fclose( f );
    }
*/
    s_nMerges++;
}
//=========================================================================

xbool audio_mgr::IsValidDescriptor( const char* pName )
{
    u16*  pDescriptor;
    char* pString;

    // Find the descriptor.
    pDescriptor = g_AudioMgr.FindDescriptorByName( pName, NULL, pString );

    // Find it?
    return ( pDescriptor ) ? TRUE : FALSE;
}

//------------------------------------------------------------------------------

u16* audio_mgr::FindDescriptorByName( const char* pName, audio_package** pPackageResult, char* &DescriptorName )
{
    s32  Left  = 0;
    s32  Right = m_pIdentifiers.GetCount()-1;
    s32  Mid;
    char ucName[128];

    ASSERT( x_strlen(pName) < 128 );
    x_strncpy( ucName, pName, 128 );
    ucName[127] = 0;
    x_strtoupper( ucName );

    if( Right < 0 )
    {
        if( pPackageResult )
            *pPackageResult = NULL;
        return NULL;
    }

    while( 1 )
    {
        descriptor_identifier* pIdentifier;
        audio_package*         pPackage;
        char*                  pString;
        s32                    Result;

        // Get package, index and offset from table
        pIdentifier = m_pIdentifiers[ Mid = (Left+Right) >> 1 ];
        pPackage    = pIdentifier->pPackage;
        pString     = pPackage->m_IdentifierStringTable + pIdentifier->StringOffset;
        
        // Exact match?
        Result = x_strcmp( ucName, pString );

        // Smaller?
        if( Result < 0 )
        {
            if( Right == Left )
            {
                if( pPackageResult )
                    *pPackageResult = NULL;
                return NULL;
            }
            else if (Right == Mid )
            {
                Mid = Left;
            }

            Right = Mid;
        }
        // Bigger?
        else if( Result > 0 )
        {
            if( Left == Right )
            {
                if( pPackageResult )
                    *pPackageResult = NULL;
                return NULL;
            }
            else if (Left == Mid )
            {
                Mid = Right;
            }

            Left = Mid;
        }
        // Oooh! Found it!
        else
        {
            DescriptorName = pString;
            if( pPackageResult )
                *pPackageResult = pPackage;
            return( (u16*)pPackage->m_DescriptorTable[ pIdentifier->Index ] );
        }        
    }

    return NULL;
}

//------------------------------------------------------------------------------

void audio_mgr::GetVoiceParameters( uncompressed_parameters* pParams, u16* pDescriptor, audio_package* pPackage, char* pDescriptorName )
{
    (void)pDescriptorName;
    // Inherit some parameters from parent.
    pParams->PitchVariance  = pPackage->m_Header.Params.PitchVariance;
    pParams->VolumeVariance = pPackage->m_Header.Params.VolumeVariance;
    pParams->Pan2d          = pPackage->m_Header.Params.Pan2d;
    pParams->Pan3d          = pPackage->m_Header.Params.Pan3d;
    pParams->Priority       = pPackage->m_Header.Params.Priority;
    pParams->UserData       = pPackage->m_Header.Params.UserData;
    pParams->ReplayDelay    = pPackage->m_Header.Params.ReplayDelay;
    pParams->RolloffCurve   = pPackage->m_Header.Params.RolloffCurve;
    pParams->PlayPercent    = pPackage->m_Header.Params.PlayPercent;

#ifndef X_RETAIL
    if( AUDIO_TWEAK )
    {
        pTweakDescriptor = pDescriptorName;
    }
#endif // X_RETAIL

    // Decode the parameters.
    DecodeParameters( pParams, pDescriptor );

#ifndef X_RETAIL
    if( AUDIO_TWEAK )
    {
        pTweakDescriptor = NULL;
    }
#endif // X_RETAIL
}

//------------------------------------------------------------------------------

void audio_mgr::GetElementParameters( uncompressed_parameters* pParams, u16* pDescriptor, voice* pVoice )
{
    // Inherit some parameters from parent.
    pParams->PitchVariance  = pVoice->Params.PitchVariance;
    pParams->VolumeVariance = pVoice->Params.VolumeVariance;
    pParams->Pan2d          = pVoice->Params.Pan2d;
    pParams->Pan3d          = pVoice->Params.Pan3d;
    pParams->Priority       = pVoice->Params.Priority;
    pParams->UserData       = pVoice->Params.UserData;
    pParams->ReplayDelay    = pVoice->Params.ReplayDelay;
    pParams->RolloffCurve   = pVoice->Params.RolloffCurve;
    pParams->PlayPercent    = pVoice->Params.PlayPercent;

    // Decode the parameters.
    DecodeParameters( pParams, pDescriptor );
}

//------------------------------------------------------------------------------

void audio_mgr::SetMasterVolume( f32 Volume )
{
    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    // While we have packages...
    while( pLink->pPackage )
    {
        // Set the volume
        pLink->pPackage->SetUserVolume( Volume );

        // Walk the list.
        pLink = pLink->pNext;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetMusicVolume( f32 Volume )
{
    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    // While we have packages...
    while( pLink->pPackage )
    {
        // Names match?
        if( x_strstr( pLink->pPackage->m_Filename, "MUSIC_" ) )
        {
            // All good!
            pLink->pPackage->SetUserVolume( Volume );
        }             

        // Walk the list.
        pLink = pLink->pNext;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetSFXVolume( f32 Volume )
{
    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    // While we have packages...
    while( pLink->pPackage )
    {
        // Names match?
        // for SFX, if it's not music and not voice, it has to be a SFX
        if( (x_strstr( pLink->pPackage->m_Filename, "MUSIC_" ) == 0) &&
            (x_strstr( pLink->pPackage->m_Filename, "DX_"    ) == 0) )
        {
            // All good!
            pLink->pPackage->SetUserVolume( Volume );
        }             

        // Walk the list.
        pLink = pLink->pNext;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetVoiceVolume( f32 Volume )
{
    audio_package::package_link* pLink;

    // Get first package in the list.
    pLink = m_Link.pNext;

    // While we have packages...
    while( pLink->pPackage )
    {
        // Names match?
        if( x_strstr( pLink->pPackage->m_Filename, "DX_" ) )
        {
            // All good!
            pLink->pPackage->SetUserVolume( Volume );
        }             

        // Walk the list.
        pLink = pLink->pNext;
    }
}

//------------------------------------------------------------------------------

u32 audio_mgr::GetUserData( const char* pIdentifier )
{
    audio_package* pPackage;
    u16*           pDescriptor;
    char*          pString;
    u32            Result = 0;

    // Find the descriptor.
    pDescriptor = FindDescriptorByName( pIdentifier, &pPackage, pString );

    // Find it?
    if( pDescriptor )
    {
        uncompressed_parameters Params;
    
        // Decode the parameters.
        GetVoiceParameters( &Params, pDescriptor, pPackage, (char*)pIdentifier );

        // Get the user data.
        Result = Params.UserData;
    }

    // Tell the world!
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_mgr::GetVoiceParameters( const char* pIdentifier, uncompressed_parameters& Params )
{
    audio_package* pPackage;
    u16*           pDescriptor;
    char*          pString;

    // Find the descriptor.
    pDescriptor = FindDescriptorByName( pIdentifier, &pPackage, pString );

    // Find it?
    if( pDescriptor )
    {
        // Decode the parameters.
        GetVoiceParameters( &Params, pDescriptor, pPackage, (char*)pIdentifier );
        return TRUE;
    }
    return FALSE;
}

//------------------------------------------------------------------------------

s32 audio_mgr::GetPriority( voice_id VoiceID )
{
    voice* pVoice = IdToVoice( VoiceID );
    if( pVoice )
    {
        return pVoice->Params.Priority;
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------

s32 audio_mgr::GetPriority( const char* pIdentifier )
{
    uncompressed_parameters Params;
    
    // Decode the parameters.
    if( GetVoiceParameters( pIdentifier, Params ) )
        return Params.Priority;
    else
        return 0;
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetFarFalloff( const char* pIdentifier )
{
    uncompressed_parameters Params;
    
    // Decode the parameters.
    if( GetVoiceParameters( pIdentifier, Params ) )
        return Params.FarFalloff;
    else
        return 0.0f;
}

//------------------------------------------------------------------------------

f32 audio_mgr::GetNearFalloff( const char* pIdentifier )
{
    uncompressed_parameters Params;
    
    // Decode the parameters.
    if( GetVoiceParameters( pIdentifier, Params ) )
        return Params.NearFalloff;
    else
        return 0.0f;
}

//------------------------------------------------------------------------------

void audio_mgr::EnableAudioDucking( void )
{
    audio_package::package_link* pLink;

    m_AudioDuckLevel++;
    if( m_AudioDuckLevel == 1 )
    {
        // Error check.
        ASSERT( s_Initialized );

        // Get first package in the list.
        pLink = m_Link.pNext;

        // While we have packages...
        while( pLink->pPackage )
        {
            // Compute the package volume.
            pLink->pPackage->ComputeVolume();

            // Walk the list.
            pLink = pLink->pNext;
        }
    }
}

//------------------------------------------------------------------------------

void audio_mgr::DisableAudioDucking( void )
{
    audio_package::package_link* pLink;

    m_AudioDuckLevel--;
#if defined(rbrannon)
    ASSERTS( (m_AudioDuckLevel >= 0), "You have called DisableAudioDucking too many times.  Match them with the calls to EnableAudioDucking" );
#endif
    if( m_AudioDuckLevel < 0 )
    {
        m_AudioDuckLevel = 0;
    }    
    else if( m_AudioDuckLevel == 0 )
    {
        // Error check.
        ASSERT( s_Initialized );

        // Get first package in the list.
        pLink = m_Link.pNext;

        // While we have packages...
        while( pLink->pPackage )
        {
            // Compute the package volume.
            pLink->pPackage->ComputeVolume();

            // Walk the list.
            pLink = pLink->pNext;
        }
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetSpeakerAngles( s32 FrontLeft, s32 FrontRight, s32 BackRight, s32 BackLeft, s32 nSpeakers )
{
    s32 i;
    s32 j;
    f32 DeltaTheta;
    f32 Angle;
    f32 Q;
    f32 P0;
    f32 P1;

    // Set class variables
    m_FrontLeft  = FrontLeft;
    m_FrontRight = FrontRight;
    m_BackRight  = BackRight;
    m_BackLeft   = BackLeft;
    m_nSpeakers  = nSpeakers;

    // Default the stereo pan.
    DeltaTheta = 180.0f;
    for( i=0 ; i<180 ; i++ )
    {
        Angle = (f32)(i);
        Q = Angle / DeltaTheta;
        P0 = x_sqrt( 1.0f - Q );
        P1 = x_sqrt( Q );
        m_StereoPan[i].Set( P0, P1, 0.0f, 0.0f );
    }
    m_StereoPan[180].Set( 0.0f, 1.0f, 0.0f, 0.0f );

    switch( nSpeakers )
    {
        case 1:
            // Set the diffuse vector for 1 speaker.
            m_Diffuse.Set( 1.0f, 1.0f, 0.0f, 0.0f );

            for( i=0 ; i<PAN_TABLE_ENTRIES ; i++ )
            {
                m_Pan[i].Set( 1.0f, 1.0f, 0.0f, 0.0f );
            }
        
            for( i=0 ; i<=180 ; i++ )
            {
                m_StereoPan[i].Set( 1.0f, 0.0f, 0.0f, 0.0f );
            }
            break;
        
        // Stereo
        case 2:
            ASSERT( FrontLeft  <= 0 );
            ASSERT( FrontRight >= 0 );

            // Set the diffuse vector for 2 speakers.
            P0 = 1.0f / x_sqrt( 2.0f );
            m_Diffuse.Set( P0, P0, 0.0f, 0.0f );

            DeltaTheta = (f32)x_abs( FrontRight - FrontLeft );
            for( i=FrontLeft ; (i<=FrontRight)  && (i<360) ; i++ )
            {
                j = i;
                if( j<0 )
                    j += 360;
                Angle = (f32)(i-FrontLeft);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[j].Set( P0, P1, 0.0f, 0.0f );
                /*
                LOG_MESSAGE( "audio_mgr::SetSpeakerAngles",
                            "%d: [%f,%f]",
                            j, m_Pan[j].GetX(), m_Pan[j].GetY() );
                            */
            }
            
            DeltaTheta = (f32)x_abs( FrontLeft+360 - FrontRight );
            for( i=FrontRight ; (i<= FrontLeft+360) && (i<360) ; i++ )
            {
                Angle = (f32)(i-FrontRight);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[i].Set( P1, P0, 0.0f, 0.0f );
                /*
                LOG_MESSAGE( "audio_mgr::SetSpeakerAngles",
                            "%d: [%f,%f]",
                            i, m_Pan[i].GetX(), m_Pan[i].GetY() );
                            */
            }
            break;

        // Surround
        case 3:
            ASSERT( FrontLeft  <= 0 );
            ASSERT( FrontRight >= 0 );
            ASSERT( BackRight  >= 0 );
            ASSERT( FrontLeft  <= FrontRight );
            ASSERT( FrontRight <= BackRight );
            ASSERT( BackRight  <= (FrontLeft+360) );

            // Set the diffuse vector for 3 speakers.
            P0 = 1.0f / x_sqrt( 3.0f );
            m_Diffuse.Set( P0, P0, P0, 0.0f );

            DeltaTheta = (f32)x_abs( FrontRight - FrontLeft );
            for( i=FrontLeft ; i<=FrontRight ; i++ )
            {
                j = i;
                if( j<0 )
                    j += 360;
                Angle = (f32)(i-FrontLeft);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[j].Set( P0, P1, 0.0f, 0.0f );
                /*
                LOG_MESSAGE( "audio_mgr::SetSpeakerAngles",
                            "%d: [%f,%f]",
                            j, m_Pan[j].GetX(), m_Pan[j].GetY() );
                            */
            }

            DeltaTheta = (f32)x_abs( BackRight - FrontRight );
            for( i=FrontRight ; i<=BackRight ; i++ )
            {
                Angle = (f32)(i-FrontRight);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[i].Set( 0.0f, P0, P1, 0.0f );
                /*
                LOG_MESSAGE( "audio_mgr::SetSpeakerAngles",
                            "%d: [%f,%f]",
                            i, m_Pan[i].GetX(), m_Pan[i].GetY() );
                            */
            }

            DeltaTheta = (f32)x_abs( FrontLeft+360 - BackRight );
            for( i=BackRight ; i<= FrontLeft+360 ; i++ )
            {
                Angle = (f32)(i-BackRight);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[i].Set( P1, 0.0f, P0, 0.0f );
                /*
                LOG_MESSAGE( "audio_mgr::SetSpeakerAngles",
                            "%d: [%f,%f]",
                            i, m_Pan[i].GetX(), m_Pan[i].GetY() );
                            */
            }
            break;

        // 5.1
        case 4:
            ASSERT( FrontLeft  <= 0 );
            ASSERT( FrontRight >= 0 );
            ASSERT( BackRight  >= 0 );
            ASSERT( BackLeft   >= 0 );
            ASSERT( FrontLeft  <= FrontRight );
            ASSERT( FrontRight <= BackRight );
            ASSERT( BackRight  <= BackLeft );
            ASSERT( BackLeft   <= (FrontLeft+360) );

            // Set the diffuse vector for 4 speakers.
            P0 = 1.0f / x_sqrt( 4.0f );
            m_Diffuse.Set( P0, P0, P0, P0 );

            DeltaTheta = (f32)x_abs( FrontRight - FrontLeft );
            for( i=FrontLeft ; i<FrontRight ; i++ )
            {
                j = i;
                if( j<0 )
                    j += 360;
                Angle = (f32)(i-FrontLeft);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[j].Set( P0, P1, 0.0f, 0.0f );
            }

            DeltaTheta = (f32)x_abs( BackRight - FrontRight );
            for( i=FrontRight ; i<BackRight ; i++ )
            {
                Angle = (f32)(i-FrontRight);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[i].Set( 0.0f, P0, P1, 0.0f );
            }

            DeltaTheta = (f32)x_abs( BackLeft - BackRight );
            for( i=BackRight ; i<BackLeft ; i++ )
            {
                Angle = (f32)(i-BackRight);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[i].Set( 0.0f, 0.0f, P0, P1 );
            }

            DeltaTheta = (f32)x_abs( FrontLeft+360 - BackLeft );
            for( i=BackLeft ; i< FrontLeft+360 ; i++ )
            {
                Angle = (f32)(i-BackLeft);
                Q = Angle / DeltaTheta;
                P0 = x_sqrt( 1.0f - Q );
                P1 = x_sqrt( Q );
                m_Pan[i].Set( P1, 0.0f, 0.0f, P0 );
            }
            break;

        default:
            ASSERT(0);
            break;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::GetSpeakerAngles( s32& FrontLeft, s32& FrontRight, s32& BackRight, s32& BackLeft, s32& nSpeakers )
{
    // Set class variables
    FrontLeft  = m_FrontLeft;
    FrontRight = m_FrontRight;
    BackRight  = m_BackRight;
    BackLeft   = m_BackLeft;
    nSpeakers  = m_nSpeakers;
}

//------------------------------------------------------------------------------

audio_mgr::audio_mgr_ear* audio_mgr::IdToEar( s32 EarID )
{
    audio_mgr_ear* pEar = &g_AudioMgr.m_Ear[0];
    s32    Index    = ((EarID & 0xffff)-1);
    u32    Sequence = ((EarID >> 16) & 0x0000ffff);

    // Error check.
    if( (Index < 0) || (Index >= MAX_EARS) )
    {
        return NULL;
    }

    // Does the sequence match?
    if( ((pEar+Index)->Sequence & 0x0000ffff) == Sequence )
        return pEar+Index;
    else
        return NULL;
}

//------------------------------------------------------------------------------

s32 audio_mgr::EarToId( audio_mgr_ear* pEar )
{
    s32 Index = pEar-&g_AudioMgr.m_Ear[0];

    // Encode index and sequence.
    return (((Index+1) & 0xffff) + (pEar->Sequence << 16));
}

//------------------------------------------------------------------------------

ear_id audio_mgr::GetFirstEar( void )
{
    ASSERT( m_pCurrentEar == NULL );
    m_pCurrentEar = m_pUsedEars;
    if( m_pCurrentEar )
        return EarToId(m_pCurrentEar);
    else
        return 0;
}

//------------------------------------------------------------------------------

ear_id audio_mgr::GetNextEar( void )
{
    if( m_pCurrentEar && m_pCurrentEar->pNext )
    {
        m_pCurrentEar = m_pCurrentEar->pNext; 
        return EarToId(m_pCurrentEar);
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::ResetCurrentEar( void )
{
    m_pCurrentEar = NULL;
}

//------------------------------------------------------------------------------

ear_id audio_mgr::CreateEar( void )
{
    // Nuke the current ear.
    m_pCurrentEar = NULL;

    // Only if an ear is available...
    if( m_pFreeEars )
    {
        audio_mgr_ear* pEar;
        
        // Take it out of free list.
        pEar        = m_pFreeEars;
        m_pFreeEars = m_pFreeEars->pNext;

        // Put it in the used list
        pEar->pNext = m_pUsedEars;
        m_pUsedEars = pEar;

        // Initialize it.
        pEar->Volume   = 1.0f;
        pEar->ZoneID   = ZONELESS;

        // Set volumes.
        for( s32 i=0 ; i<MAX_ZONES ; i++ )
        {
            pEar->ZoneVolumes[i] = 1.0f;
        }

        // its all good
        return EarToId( pEar );
    }
    else
    {
        // Out of ears!
        x_DebugMsg( "Out of audio ears.\n" );
        return 0;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::DestroyEar( ear_id EarID )
{
    audio_mgr_ear* pEar = IdToEar( EarID );

    if( pEar )
    {
        // Nuke the current ear.
        m_pCurrentEar = NULL;
        
        audio_mgr_ear* pCurr = m_pUsedEars;
        audio_mgr_ear* pPrev = NULL;

        // Bump sequence
        if( ++pEar->Sequence >= 32768 )
            pEar->Sequence = 1;

        // Walk the entire list (all 4 of em!!!)
        while( pCurr )
        {
            // Found it?
            if( pCurr == pEar )
                break;

            // Next!
            pPrev = pCurr;
            pCurr = pCurr->pNext;
        }

        // Find it?
        if( pCurr )
        {
            // Make sure its not first in the list...
            if( pPrev )
            {
                // Remove it from the list.
                pPrev->pNext = pCurr->pNext;
            }
            else
            {
                // Its first in the list...
                m_pUsedEars  = m_pUsedEars->pNext;
            }

            // Put it in the free list.
            pCurr->pNext = m_pFreeEars;
            m_pFreeEars  = pCurr;
        }
    }
    else
    {
        // Trying to destroy an invalid ear...
        x_DebugMsg( "Trying to destroy an invalid ear.\n" );
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetSpeakerConfig( s32 SpeakerConfig )
{
    m_SpeakerConfig = SpeakerConfig;

    switch( SpeakerConfig )
    {
        case SPEAKERS_MONO:
            SetSpeakerAngles( 0, 0, 0, 0, 1 );
            break;
        case SPEAKERS_STEREO:
            SetSpeakerAngles( -90, 90, 0, 0, 2 );
            break;
        case SPEAKERS_PROLOGIC:
            SetSpeakerAngles( -45, 45, 45+90, 45+180, 4 );
            //SetSpeakerAngles( -90, 90, 0, 0, 2 );
            break;
        case SPEAKERS_DOLBY_DIGITAI_5_1:
            SetSpeakerAngles( -45, 45, 45+90, 45+180, 4 );
            break;
        default:
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetVoiceEar( voice_id VoiceID, ear_id EarID )
{
    voice* pVoice = IdToVoice( VoiceID );
    if( pVoice )
    {
        pVoice->EarID = EarID;
    }
}
            
//------------------------------------------------------------------------------

void audio_mgr::SetClip( f32 NearClip, f32 FarClip )
{
    m_NearClip = NearClip;
    m_FarClip  = FarClip;
}

//------------------------------------------------------------------------------

void audio_mgr::GetClip( f32& NearClip, f32& FarClip )
{
    NearClip = m_NearClip;
    FarClip  = m_FarClip;
}
//------------------------------------------------------------------------------

void audio_mgr::UpdateEarZoneVolume( ear_id EarID, s32 ZoneID, f32 Volume )
{
    ASSERT( s_Initialized );

    // Convert id.
    audio_mgr_ear* pEar = IdToEar( EarID );
    ASSERT( pEar );

    if( pEar )
    {
        ASSERT( (ZoneID >= 0) && (ZoneID <MAX_ZONES) );
        if( (ZoneID >= 0) && (ZoneID <MAX_ZONES) )
        {
            pEar->ZoneVolumes[ ZoneID ] = Volume;
        }
    }
}

//------------------------------------------------------------------------------

void audio_mgr::UpdateEarZoneVolumes( ear_id EarID, f32* pVolumes )
{
    ASSERT( s_Initialized );

    // Convert id.
    audio_mgr_ear* pEar = IdToEar( EarID );
    ASSERT( pEar );

    if( pEar )
    {
        for( s32 i=0 ; i<MAX_ZONES ; i++ )
        {
            pEar->ZoneVolumes[i] = pVolumes[i];
        }
    }
}

//------------------------------------------------------------------------------

void audio_mgr::SetPitchLock( voice_id          VoiceID,
                              xbool             bPitchLock )
{
    voice* pVoice = IdToVoice( VoiceID );
    if( pVoice )
    {
        g_AudioVoiceMgr.SetPitchLock( pVoice, bPitchLock );
    }
}

#if defined( rbrannon )

xbool AUDIO_THRASH = FALSE;
s32 ThrashID[32] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

char* ThrashDescriptors[5] = {
"Mutant1_Footfall_Toe_Metal",
"Mutant1_Vox_Run",
"SMP_Fire",
"Mutant1_PainGrunt",
"Mutant1_Vox_LeapAttack"
};

s32 iThrash = 0;

void AudioThrashUpdate( void )
{
    if( AUDIO_THRASH )
    {
        s32 i = x_rand() % 32;
        g_AudioMgr.Release( ThrashID[i], 0.0f );
        s32 j = x_rand() % 5;
        ThrashID[i] = g_AudioMgr.Play( ThrashDescriptors[j] );
        x_DebugMsg( "voice[%d]: %08x\n", i,ThrashID[i] );
    }
    else
    {
        for( s32 i=0 ; i<32 ; i++ )
        {
            if( g_AudioMgr.IsValidVoiceId( ThrashID[i] )  )
            {
                g_AudioMgr.Release( ThrashID[i], 0.0f );
            }
        }
    }
}

void AudioThrashCheck( void )
{
    s32 i;
    return;
    if( !AUDIO_THRASH )
    {
        // Turn it on.
        for( i=0 ; i<32 ; i++ )
        {
            s32 j = x_rand() % 5;
            ThrashID[i] = g_AudioMgr.Play( ThrashDescriptors[j] );
            x_DebugMsg( "voice[%d]: %08x\n", i, ThrashID[i] );
        }
        iThrash = 0;
    }
    else
    {
        // Turn it off.
        for( i=0 ; i<32 ; i++ )
        {
            g_AudioMgr.Release( ThrashID[i], 0.0f );
        }
    }
}
#endif
