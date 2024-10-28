//==============================================================================
// AUDIO_MANAGER.CPP
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "AudioMgr.hpp"
#include "GameLib\StatsMgr.hpp"
#include "..\ZoneMgr\ZoneMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Audio\audio_private_pkg.hpp"
#include "Audio\audio_hardware.hpp"
#include "..\ConversationMgr\ConversationMgr.hpp"

#ifdef X_EDITOR
#include "..\Apps\Editor\Project.hpp"
#endif

//==============================================================================
// EXTERNALS
//==============================================================================

audio_manager g_AudioManager;


//#define ENABLE_PROPAGATION
#define PS2_AUDIO_MEMORY            (5512*1024)
#define PRO_VIEW_AUDIO_MEMORY       (4512*1024)
#define PS2_STREAM_BUFFER           (512*1024)
#define AUDIO_MEM_STAT_RENDER_POS   15

//#ifdef dstewart
//#define DISABLE_PACKAGE_LOAD
//#endif

s32     s_PS2MemorySize = 0 ;

//==============================================================================
// DEBUG DATA
//==============================================================================
vector3 BeginPoint(0.0f, 0.0f,0.0f);
vector3 EndPoint  (0.0f, 0.0f,0.0f);
vector3 ClosePoint(0.0f, 0.0f,0.0f);

vector3 ZeroVec   (0.0f, 0.0f,0.0f);

xbool   s_DebugData     = FALSE;
xbool   s_DebugMarker   = FALSE;
xbool   s_LoadFailed    = FALSE; 
//==============================================================================
//  AUDIO PACKAGE LOADER
//==============================================================================

class audiopkg_loader : public rsc_loader
{
public:
            audiopkg_loader ( const char* pType, const char* pExt ) : rsc_loader(pType,pExt) {}
    void*   PreLoad         ( X_FILE* pFP   );
    void*   Resolve         ( void*   pData );
    void    Unload          ( void*   pData );
    void*   PreLoad         ( X_FILE*& Fp, const char* pFileName );
};

audiopkg_loader AudioPackageLoader("Audio Package",".audiopkg");

//==============================================================================

void* audiopkg_loader::PreLoad( X_FILE*& Fp, const char* pFileName )
{
    MEMORY_OWNER( "AUDIO PACKAGE DATA" );
    xbool Loaded = FALSE;

#ifdef DISABLE_PACKAGE_LOAD
    return NULL;
#endif

#ifdef X_EDITOR

    Fp = x_fopen( pFileName, "rb" );

#else

    char LocalizedName[X_MAX_PATH];

    // this just checks that the file exists. the unlocalized name is passed
    // to the loader, and will be converted again...
    // this is because it will fail if we remove the ENG_ name prefix in the future,
    // and because the loader may be called elsewhere.
    x_strcpy(LocalizedName, g_AudioMgr.GetLocalizedName(pFileName));

    Fp = x_fopen( LocalizedName, "rb" );

#endif // X_EDITOR

    //
    // We want to try to emulated the the memory footprint for the PS2 in the editor.
    //
#ifdef X_EDITOR

    X_FILE* pPS2File = NULL;
            
    x_try;

        xstring FullPathName( pFileName );
        s32 i = 0;
        for( i = FullPathName.GetLength()-1; i >= 0; i-- )
        {
            if( (FullPathName[i] == '/') || (FullPathName[i] == '\\') )
                break;
        }
        xstring FileName( FullPathName.Right( (FullPathName.GetLength()-1)-i ) );

        char PS2FilePath[256];
        x_sprintf( PS2FilePath, "%s\\PS2\\%s", g_Settings.GetReleasePath(), FileName );

        pPS2File = x_fopen( PS2FilePath, "rb" );
        
        if( pPS2File == NULL )
            x_throw( xfs("Unable to open file [%s]", PS2FilePath) );

        package_identifier  PackageID;
        package_header      PackageHeader;
        s32                 MRAM = 0;
        s32                 ARAM = 0;

        // Read in the package identifier.
        x_fread( &PackageID, sizeof(package_identifier), 1, pPS2File );

        // Correct version?
        if( !x_strncmp( PackageID.VersionID, PS2_PACKAGE_VERSION, VERSION_ID_SIZE ) )
        {
            // Correct platform?
            if( !x_strncmp( PackageID.TargetID, PS2_TARGET_ID, TARGET_ID_SIZE ) )
            {

                // Now read in the header.
                x_fread( &PackageHeader, sizeof(package_header), 1, pPS2File );
                
                MRAM    += PackageHeader.StringTableFootprint;
                MRAM    += PackageHeader.MusicDataFootprint;
                MRAM    += PackageHeader.LipSyncTableFootprint;
                MRAM    += PackageHeader.BreakPointTableFootprint;
                MRAM    += PackageHeader.nIdentifiers * sizeof(descriptor_identifier);
                MRAM    += PackageHeader.nDescriptors * sizeof(u32*);
                MRAM    += PackageHeader.DescriptorFootprint;

                // For each temperature...
                for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                {
                    if( PackageHeader.nSampleIndices[ i ] )
                    {
                        // Allocate memory for sample header index table.
                        MRAM += (PackageHeader.nSampleIndices[ i ]+1) * sizeof(u16);
                    }
                }

                // Allocate memory for the hot and cold samples
                if( PackageHeader.nSampleHeaders[ HOT ] )
                {
                    MRAM +=  PackageHeader.nSampleHeaders[ HOT ] * PackageHeader.HeaderSizes[ HOT ];
                }

                if( PackageHeader.nSampleHeaders[ COLD ] )
                {
                    MRAM += PackageHeader.nSampleHeaders[ COLD ] * PackageHeader.HeaderSizes[ COLD ];
                }
                
                ARAM    += PackageHeader.Aram;
            }
            else
            {
                x_throw( xfs("Incorrect audio package PLATFORM [%s]\nlast compile by [%s]", PS2FilePath, PackageID.UserId) );
            }
        }
        else
        {
            x_throw( xfs("Incorrect audio package VERSION [%s]\nlast compile by [%s]", PS2FilePath, PackageID.UserId) );
        }

        extern xbool g_IncludeInAudioBudget;
        if( g_IncludeInAudioBudget )
            s_PS2MemorySize -= ARAM;

        LOG_MESSAGE( "audiopkg_loader::PreLoad", "[%s] MainRam: %d, AudioRam: %d, Total ARAM Free: %d", pFileName, MRAM, ARAM, s_PS2MemorySize );

        x_fclose( pPS2File );

    x_catch_begin;
        
        x_display_exception_msg(xfs("Could not load audiopkg:\n%s",pFileName));

        if( pPS2File )
            x_fclose( pPS2File );

    x_catch_end;

#endif // X_EDITOR

    if( Fp )
    {
        x_DebugMsg( "audiopkg_loader::PreLoad [%s]\n", pFileName );
        Loaded = g_AudioMgr.LoadPackage( pFileName );
    }

    if( Loaded )
    {
#ifdef X_EDITOR
        char* pRetFileName = new char[ x_strlen(pFileName)+1 ];
        x_strcpy( pRetFileName, pFileName );
#else
        char* pRetFileName = new char[ x_strlen(LocalizedName)+1 ];
        x_strcpy( pRetFileName, LocalizedName );
#endif

        return (void*)pRetFileName;
    }
    else
    {

    //
    // If we fail to load an audio package on the PC, try to find out why it failed.
    //
#ifdef X_EDITOR
    X_FILE* pFileLoadFailed = NULL;

    x_try;
        
        pFileLoadFailed = x_fopen( pFileName, "rb" );

        if( pFileLoadFailed == NULL )
            x_throw( xfs("Unable to open file [%s]", pFileName) );

        package_identifier  PackageID;

        // Read in the package identifier.
        x_fread( &PackageID, sizeof(package_identifier), 1, pFileLoadFailed );

        // Correct version?
        if( !x_strncmp( PackageID.VersionID, PACKAGE_VERSION, VERSION_ID_SIZE ) )
        {
            // Correct platform?
            if( !x_strncmp( PackageID.TargetID, TARGET_ID, TARGET_ID_SIZE ) )
            {

            }
            else
            {
                x_throw( xfs("Incorrect audio package PLATFORM [%s]\nthis was last compile by [%s]", pFileName, PackageID.UserId) );
            }
        }
        else
        {
			x_throw( xfs("Incorrect audio package VERSION [%s]\nthis was last compile by [%s]", pFileName, PackageID.UserId) );
        }

        
        x_fclose( pFileLoadFailed );

    x_catch_begin;
        
        x_display_exception_msg(xfs("Could not load audiopkg:\n%s",pFileName));

        if( pFileLoadFailed )
            x_fclose( pFileLoadFailed );

    x_catch_end;
#endif // X_EDITOR
        
        if( Fp )
            s_LoadFailed = TRUE;

        return NULL;
    }
}

//==============================================================================

void* audiopkg_loader::PreLoad( X_FILE* pFP   )
{
    (void)pFP;
    return NULL;
}

//==============================================================================

void* audiopkg_loader::Resolve( void* pData )
{
    return pData;
}

//==============================================================================

void  audiopkg_loader::Unload( void* pData )
{
    if( pData != NULL )
    {
#ifdef X_EDITOR
    
        X_FILE* pPS2File = NULL;
            
        char PS2FilePath[256];
        x_try;

            xstring FullPathName( (const char*)pData );
            s32 i = 0;
            for( i = FullPathName.GetLength()-1; i >= 0; i-- )
            {
                if( (FullPathName[i] == '/') || (FullPathName[i] == '\\') )
                    break;
            }
            xstring FileName( FullPathName.Right( (FullPathName.GetLength()-1)-i ) );

            x_sprintf( PS2FilePath, "%s\\PS2\\%s", g_Settings.GetReleasePath(), FileName );

            pPS2File = x_fopen( PS2FilePath, "rb" );
        
            if( pPS2File == NULL )
                x_throw( xfs("Unable to open file [%s]", PS2FilePath) );

            package_identifier  PackageID;
            package_header      PackageHeader;
            s32                 MRAM = 0;

            // Read in the package identifier.
            x_fread( &PackageID, sizeof(package_identifier), 1, pPS2File );

            // Correct version?
            if( !x_strncmp( PackageID.VersionID, PS2_PACKAGE_VERSION, VERSION_ID_SIZE ) )
            {
                // Correct platform?
                if( !x_strncmp( PackageID.TargetID, PS2_TARGET_ID, TARGET_ID_SIZE ) )
                {

                    // Now read in the header.
                    x_fread( &PackageHeader, sizeof(package_header), 1, pPS2File );
                
                    MRAM    += PackageHeader.StringTableFootprint;
                    MRAM    += PackageHeader.MusicDataFootprint;
                    MRAM    += PackageHeader.LipSyncTableFootprint;
                    MRAM    += PackageHeader.BreakPointTableFootprint;
                    MRAM    += PackageHeader.nIdentifiers * sizeof(descriptor_identifier);
                    MRAM    += PackageHeader.nDescriptors * sizeof(u32*);
                    MRAM    += PackageHeader.DescriptorFootprint;

                    // For each temperature...
                    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                    {
                        if( PackageHeader.nSampleIndices[ i ] )
                        {
                            // Allocate memory for sample header index table.
                            MRAM += (PackageHeader.nSampleIndices[ i ]+1) * sizeof(u16);
                        }
                    }

                    // Allocate memory for the hot and cold samples
                    if( PackageHeader.nSampleHeaders[ HOT ] )
                    {
                        MRAM +=  PackageHeader.nSampleHeaders[ HOT ] * PackageHeader.HeaderSizes[ HOT ];
                    }

                    if( PackageHeader.nSampleHeaders[ COLD ] )
                    {
                        MRAM += PackageHeader.nSampleHeaders[ COLD ] * PackageHeader.HeaderSizes[ COLD ];
                    }
                
                    MRAM    += PackageHeader.Aram;
                }
                else
                {
                    x_throw( xfs("Incorrect audio package PLATFORM [%s]", PS2FilePath) );
                }
            }
            else
            {
                x_throw( xfs("Incorrect audio package VERSION [%s]", PS2FilePath) );
            }

            s_PS2MemorySize += MRAM;

            x_fclose( pPS2File );

        x_catch_begin;
        
            x_display_exception_msg(xfs("Could not load audiopkg:\n%s",PS2FilePath));

            if( pPS2File )
                x_fclose( pPS2File );

        x_catch_end;

#endif // X_EDITOR

        g_AudioMgr.UnloadPackage( (const char*)pData );
        delete [] (char *)pData;
    }
}

//==============================================================================


//==============================================================================
// AUDIO MANAGER
//==============================================================================

audio_manager::audio_manager( void )
{
}

//=========================================================================

audio_manager::~audio_manager( void )
{
    
}

//=========================================================================
                                        
void audio_manager::Init( s32 MemSize )
{
    MEMORY_OWNER( "audio_manager::Init()" );

    m_NearClip              = 0.0f;
    m_FarClip               = 0.0f;
    m_CurrentReceiverCursor = 0;
    m_ReceiverSendCursor    = 0;
    m_ReceiverSendCursorTime= 0;
    m_ReceiverFirstSendCursor       = 0;
    m_AlertReceiverFirstSendCursor  = 0;

    m_CurrentAlertReceiverCursor = 0;
    m_AlertReceiverSendCursor    = 0;
    m_AlertReceiverSendCursorTime= 0;
    m_AlertReceiverFirstSendCursor  = 0;

    x_DebugMsg( "..ClearReceiverQueue\n" );
    ClearReceiverQueue();
    ClearAlertReceiverQueue();

    g_AudioMgr.Init( MemSize );
    
    s_PS2MemorySize = PS2_AUDIO_MEMORY - PS2_STREAM_BUFFER;

    //matrix4 W2V;
    //W2V.Identity();
    g_AudioMgr.SetSpeakerConfig( SPEAKERS_STEREO );
    //g_AudioMgr.SetSpeakerConfig(-90,+90,0,0,2);
    //g_AudioMgr.SetSpeakerConfig(-45,45,45+90,45+180,4);
    //g_AudioManager.SetEar( W2V, NEAR_CLIP, FAR_CLIP );
    g_AudioMgr.SetClip( NEAR_CLIP, FAR_CLIP );
}

void audio_manager::ClearReceiverQueue( void )
{
    s32 i = 0;
    for( ; i < STORED_SOUND_STACK_SIZE; i++)
    {
        m_Receiver[i].Pos.Set( 0.0f, 0.0f, 0.0f );
        m_Receiver[i].Guid      = 0;
        m_Receiver[i].Time      = 0;
        m_Receiver[i].Type      = NULL_SOUND;
        m_Receiver[i].VoiceID   = 0;
        m_Receiver[i].ZoneID    = 0;
    }
}

//=========================================================================

void audio_manager::Render ( void )
{
#ifdef sansari
    if( eng_Begin( "Audio Manager Render" ) )
    {
        draw_Line( BeginPoint, EndPoint, XCOLOR_BLUE );

        draw_Marker( BeginPoint, XCOLOR_GREEN );
        draw_Marker( EndPoint, XCOLOR_GREEN );

        draw_Marker( ClosePoint, XCOLOR_RED );

        eng_End();
    }
#endif
}

//=========================================================================

void audio_manager::NewAudioAlert( voice_id         VoiceID, 
                                   sound_type       SoundType, 
                                   const vector3&   Position,
                                   s32              ZoneID, 
                                   guid             Guid )
{
    m_Receiver[ m_CurrentReceiverCursor ].Guid        = Guid;
    m_Receiver[ m_CurrentReceiverCursor ].Pos         = Position; // perceived position of the sound.
    m_Receiver[ m_CurrentReceiverCursor ].OriginalPos = Position; // actual position of the sound.
    m_Receiver[ m_CurrentReceiverCursor ].Time        = g_ObjMgr.GetGameTime();
    m_Receiver[ m_CurrentReceiverCursor ].ZoneID      = ZoneID;
    m_Receiver[ m_CurrentReceiverCursor ].Type        = SoundType;
    m_Receiver[ m_CurrentReceiverCursor ].VoiceID     = VoiceID;

    m_CurrentReceiverCursor++;
    if( m_CurrentReceiverCursor >= STORED_SOUND_STACK_SIZE )
        m_CurrentReceiverCursor = 0;
}

//=========================================================================

voice_id audio_manager::Play( const char* pIdentifier, sound_type SoundType, const vector3& Position,
                              s32 ZoneID, guid Guid, xbool AutoStart, xbool VolumeClipped )
{
    (void)pIdentifier;
    (void)SoundType;
    (void)Position;
    (void)ZoneID;
    (void)Guid;
    (void)AutoStart;

    voice_id    VoiceID   = 0;
    vector3     FinalPos  = Position;

    // #TODO:   Research the Zone argument in the PlayVolumeClipped and Play methods of audio_mgr class.
    //          For now ZONELESS

    if( VolumeClipped )
        VoiceID = g_AudioMgr.PlayVolumeClipped( pIdentifier, FinalPos, ZoneID, AutoStart );
    else
        VoiceID = g_AudioMgr.Play( pIdentifier, FinalPos, ZoneID, AutoStart, VolumeClipped );
  
    m_Receiver[ m_CurrentReceiverCursor ].Guid      = Guid;
    m_Receiver[ m_CurrentReceiverCursor ].Pos       = FinalPos;
    m_Receiver[ m_CurrentReceiverCursor ].OriginalPos = Position;
    m_Receiver[ m_CurrentReceiverCursor ].Time      = g_ObjMgr.GetGameTime();
    m_Receiver[ m_CurrentReceiverCursor ].ZoneID    = ZoneID;
    m_Receiver[ m_CurrentReceiverCursor ].Type      = SoundType;
    m_Receiver[ m_CurrentReceiverCursor ].VoiceID   = VoiceID;
    
    m_CurrentReceiverCursor++;
    if( m_CurrentReceiverCursor >= STORED_SOUND_STACK_SIZE )
        m_CurrentReceiverCursor = 0;

    return VoiceID;
}

//=========================================================================

void audio_manager::NewAudioAlert( voice_id VoiceID, sound_type SoundType, guid Guid )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
    if( pObj )
    {
        const matrix4& L2W = pObj->GetL2W();
        m_Receiver[ m_CurrentReceiverCursor ].Pos           = L2W.GetTranslation();
        m_Receiver[ m_CurrentReceiverCursor ].OriginalPos   = L2W.GetTranslation();
    }
    else
    {
        m_Receiver[ m_CurrentReceiverCursor ].Pos           .Set( 0.0f, 0.0f, 0.0f );
        m_Receiver[ m_CurrentReceiverCursor ].OriginalPos   .Set( 0.0f, 0.0f, 0.0f );
    }

    m_Receiver[ m_CurrentReceiverCursor ].Guid          = Guid;
    m_Receiver[ m_CurrentReceiverCursor ].Time          = g_ObjMgr.GetGameTime();
    m_Receiver[ m_CurrentReceiverCursor ].ZoneID        = m_PlayerZoneID;
    m_Receiver[ m_CurrentReceiverCursor ].Type          = SoundType;
    m_Receiver[ m_CurrentReceiverCursor ].VoiceID       = VoiceID;

    m_CurrentReceiverCursor++;
    if( m_CurrentReceiverCursor >= STORED_SOUND_STACK_SIZE )
        m_CurrentReceiverCursor = 0;
}
/*
voice_id audio_manager::Play( const char* pIdentifier, sound_type SoundType, guid Guid, xbool AutoStart)
{
    (void)pIdentifier;
    (void)SoundType;
    (void)Guid;
    (void)AutoStart;

    voice_id    VoiceID = g_AudioMgr.Play( pIdentifier, AutoStart );
    
    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
    if( pObj )
    {
        const matrix4& L2W = pObj->GetL2W();
        m_Receiver[ m_CurrentReceiverCursor ].Pos           = L2W.GetTranslation();
        m_Receiver[ m_CurrentReceiverCursor ].OriginalPos   = L2W.GetTranslation();
    }
    else
    {
        m_Receiver[ m_CurrentReceiverCursor ].Pos           .Set( 0.0f, 0.0f, 0.0f );
        m_Receiver[ m_CurrentReceiverCursor ].OriginalPos   .Set( 0.0f, 0.0f, 0.0f );
    }

    m_Receiver[ m_CurrentReceiverCursor ].Guid          = Guid;
    m_Receiver[ m_CurrentReceiverCursor ].Time          = g_ObjMgr.GetGameTime();
    m_Receiver[ m_CurrentReceiverCursor ].ZoneID        = m_PlayerZoneID;
    m_Receiver[ m_CurrentReceiverCursor ].Type          = SoundType;
    m_Receiver[ m_CurrentReceiverCursor ].VoiceID       = VoiceID;
    
    m_CurrentReceiverCursor++;
    if( m_CurrentReceiverCursor >= STORED_SOUND_STACK_SIZE )
        m_CurrentReceiverCursor = 0;

    return VoiceID;
}
*/
//=========================================================================

audio_manager::receiver* audio_manager::GetReceiverQueue ( void )
{
    return &m_Receiver[0];
}

//=========================================================================

audio_manager::receiver* audio_manager::GetFirstReceiverItem ( xtick Time )
{
    // We want to find the recevier item whoes time is closest the time that got passed in.
    // TODO: We can definitly optimize this routine to use the CurrentReceiverCursor as the entry point.

    s32 i           = 0;
    s32 RetIndex    = -1;
    for( ; i < STORED_SOUND_STACK_SIZE; i++)
    {
        if( (RetIndex != -1) && (m_Receiver[i].Type != NULL_SOUND) )
        { 
            if( (m_Receiver[i].Time >= Time) && (m_Receiver[i].Time < m_Receiver[RetIndex].Time) )
                RetIndex = i;
        }
        else if( (m_Receiver[i].Time >= Time) && (m_Receiver[i].Type != NULL_SOUND) )
        {
            RetIndex = i;
        }
    }
    
    m_ReceiverFirstSendCursor   = RetIndex;
    m_ReceiverSendCursor        = RetIndex;
    m_ReceiverSendCursorTime    = m_Receiver[ RetIndex ].Time;

    if( RetIndex == -1 )
        return NULL;
    else
        return &m_Receiver[ RetIndex ];
}

//=========================================================================

audio_manager::receiver* audio_manager::GetNextReceiverItem ( void )
{
    if( m_ReceiverSendCursor != -1 )
    {
        m_ReceiverSendCursor++;
        if( m_ReceiverSendCursor >= STORED_SOUND_STACK_SIZE )
            m_ReceiverSendCursor = 0;
        
        if( m_ReceiverSendCursor == m_ReceiverFirstSendCursor)
        {
            m_ReceiverSendCursor        = -1;
            return NULL;
        }            

        if( (m_Receiver[ m_ReceiverSendCursor ].Time > m_ReceiverSendCursorTime) && 
            m_Receiver[ m_ReceiverSendCursor ].Type != NULL_SOUND )
        {
            return &m_Receiver[ m_ReceiverSendCursor ];
        }
        else
        {
            m_ReceiverSendCursor = -1;
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

//=========================================================================

void audio_manager::AppendAlertReceiver( const vector3& Position, f32 AlertRadius, alert_package::alert_type AlertType,
                                         alert_package::alert_targets AlertTarget, guid& Origin, guid& Cause, 
                                         factions Factions )
{
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Position          = Position;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_AlertRadius       = AlertRadius;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Type              = AlertType;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Target            = AlertTarget;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Origin            = Origin;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Cause             = Cause;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_FactionsSpecific  = Factions;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Time              = g_ObjMgr.GetGameTime();
    
    m_CurrentAlertReceiverCursor++;
    if( m_CurrentAlertReceiverCursor >= STORED_SOUND_STACK_SIZE )
        m_CurrentAlertReceiverCursor = 0;
}

//=========================================================================
                                          
void audio_manager::AppendAlertReceiver( alert_package& AlertPackage )
{
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ]         = AlertPackage;
    m_AlertReceiver[ m_CurrentAlertReceiverCursor ].m_Time  = g_ObjMgr.GetGameTime();

    m_CurrentAlertReceiverCursor++;
    if( m_CurrentAlertReceiverCursor >= STORED_SOUND_STACK_SIZE )
        m_CurrentAlertReceiverCursor = 0;
}

//=========================================================================

void audio_manager::ClearAlertReceiverQueue( void )
{
    s32 i = 0;
    for( ; i < STORED_SOUND_STACK_SIZE; i++)
    {
        m_AlertReceiver[i].m_Position.Set( 0.0f, 0.0f, 0.0f );
        m_AlertReceiver[i].m_Origin     = 0;
        m_AlertReceiver[i].m_Cause      = 0;
        m_AlertReceiver[i].m_Time       = 0;
        m_AlertReceiver[i].m_Type       = alert_package::ALERT_TYPE_NULL;
    }
}

//=========================================================================

alert_package* audio_manager::GetAlertReceiverQueue( void )
{
    return &m_AlertReceiver[0];
}

//=========================================================================

alert_package* audio_manager::GetFirstAlertReceiverItem( xtick Time )
{
    // We want to find the recevier item whoes time is closest the time that got passed in.
    // TODO: We can definitly optimize this routine to use the CurrentReceiverCursor as the entry point.

    s32 i           = 0;
    s32 RetIndex    = -1;
    for( ; i < STORED_SOUND_STACK_SIZE; i++)
    {
        if( (RetIndex != -1) && (m_AlertReceiver[i].m_Type != alert_package::ALERT_TYPE_NULL) )
        { 
            if( (m_AlertReceiver[i].m_Time >= Time) && (m_AlertReceiver[i].m_Time < m_AlertReceiver[RetIndex].m_Time) )
                RetIndex = i;
        }
        else if( (m_AlertReceiver[i].m_Time >= Time) && (m_AlertReceiver[i].m_Type != alert_package::ALERT_TYPE_NULL) )
        {
            RetIndex = i;
        }
    }
    
    m_AlertReceiverFirstSendCursor   = RetIndex;
    m_AlertReceiverSendCursor        = RetIndex;
    m_AlertReceiverSendCursorTime    = m_AlertReceiver[ RetIndex ].m_Time;

    if( RetIndex == -1 )
        return NULL;
    else
        return &m_AlertReceiver[ RetIndex ];
}

//=========================================================================

alert_package* audio_manager::GetAlertNextReceiverItem( void )
{
    if( m_AlertReceiverSendCursor != -1 )
    {
        m_AlertReceiverSendCursor++;
        if( m_AlertReceiverSendCursor >= STORED_SOUND_STACK_SIZE )
            m_AlertReceiverSendCursor = 0;

        if( m_AlertReceiverSendCursor == m_AlertReceiverFirstSendCursor )
        {
            m_AlertReceiverSendCursor = -1;
            return NULL;
        }

        if( (m_AlertReceiver[ m_AlertReceiverSendCursor ].m_Time >= m_AlertReceiverSendCursorTime) && 
            (m_AlertReceiver[ m_AlertReceiverSendCursor ].m_Type != alert_package::ALERT_TYPE_NULL) )
        {
            return &m_AlertReceiver[ m_AlertReceiverSendCursor ];
        }
        else
        {
            m_AlertReceiverSendCursor = -1;
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}
