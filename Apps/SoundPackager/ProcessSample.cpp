#include "compress.hpp"
#include "SoundPackager.hpp"
#include "ExportPackage.hpp"
#include "Endian.hpp"
#include "windows.h"

char*           s_FileExtensions[EXPORT_NUM_TARGETS] = 
                                {   "snd_pc", 
                                    "snd_ps2", 
                                    "snd_gcn", 
                                    "snd_xbox" 
                                };

char MP3Filename[256];
u32 g_SampleRate = 0;

//------------------------------------------------------------------------------
void SkipVersionIdentifier( X_FILE* f )
{
    char VersionString[VERSION_ID_SIZE];

    // Read the version id
    x_fread( VersionString, VERSION_ID_SIZE, 1, f );
}

//------------------------------------------------------------------------------
xbool ObjectFileNeedsToBeBuilt( const char* SourceFilename, const char* ObjectFilename, s32 Target ) 
{
    xbool Result = FALSE;
    HFILE WinFile;
    OFSTRUCT WinOfstruct;
    BY_HANDLE_FILE_INFORMATION WinInInfo;
    BY_HANDLE_FILE_INFORMATION WinOutInfo;

    // Get info on the source file...
    WinFile = OpenFile( SourceFilename, &WinOfstruct, OF_READ );
    if( WinFile == HFILE_ERROR )
    {
        x_printf( "Error! Source file '%s' not found!\n", SourceFilename  );
        x_DebugMsg( "Error! Source file '%s' not found!\n", SourceFilename  );
        s_Package.m_ParseError = TRUE;
        Result = FALSE;
    }
    else
    {
        GetFileInformationByHandle( (HANDLE)WinFile, &WinInInfo );
        CloseHandle( (HANDLE)WinFile );

        // Get info on the compressed file...
        WinFile = OpenFile( ObjectFilename, &WinOfstruct, OF_READ );
        if( WinFile == HFILE_ERROR )
        {
            Result = TRUE;
        }
        else
        {
            DWORD BytesRead;
            char  VersionString[VERSION_ID_SIZE];
            char* TargetString;

            switch( Target )
            {
                case EXPORT_PC:   TargetString = PC_PACKAGE_VERSION; break;
                case EXPORT_PS2:  TargetString = PS2_PACKAGE_VERSION; break;
                case EXPORT_GCN:  TargetString = GCN_PACKAGE_VERSION; break;
                case EXPORT_XBOX: TargetString = XBOX_PACKAGE_VERSION; break;
            }

            // Read in the version id.
            ReadFile( (HANDLE)WinFile, VersionString, VERSION_ID_SIZE, &BytesRead, NULL );
            if( x_strncmp( VersionString, TargetString, VERSION_ID_SIZE ) )
            {
                CloseHandle( (HANDLE)WinFile );
                Result = TRUE;
            }
            else
            {
                u64 source_time;
                u64 object_time;

                GetFileInformationByHandle( (HANDLE)WinFile, &WinOutInfo );
                CloseHandle( (HANDLE)WinFile );

                // Convert to 64 value for comparison.
                x_memcpy( &source_time, &WinInInfo.ftLastWriteTime, sizeof(u64) );
                x_memcpy( &object_time, &WinOutInfo.ftLastWriteTime, sizeof(u64) );

                Result = (source_time > object_time) || (WinOutInfo.nFileSizeLow <= 32);
            }
        }
    }

    return Result;
}
//------------------------------------------------------------------------------
void WriteVersionIdentifier( X_FILE* f, s32 Target )
{
    char  VersionString[VERSION_ID_SIZE];
    char* TargetString;

    switch( Target )
    {
        case EXPORT_PC:   TargetString = PC_PACKAGE_VERSION; break;
        case EXPORT_PS2:  TargetString = PS2_PACKAGE_VERSION; break;
        case EXPORT_GCN:  TargetString = GCN_PACKAGE_VERSION; break;
        case EXPORT_XBOX: TargetString = XBOX_PACKAGE_VERSION; break;
    }

    // Write out the version id
    x_memset( VersionString, 0, VERSION_ID_SIZE );
    x_strncpy( VersionString, TargetString, VERSION_ID_SIZE );
    x_fwrite( VersionString, VERSION_ID_SIZE, 1, f );
}

xbool ReadCompressedAudioFileHeader( const char* Filename, xbool ReverseEndian,
                                     s32* NumChannels, s32* CompressionMethod, 
                                     s32* CompressedSize, s32* LipSyncSize,
                                     s32* BreakPointSize )
{
    X_FILE* f;
    s32 nChannels;
    s32 TotalCompressedSize;
    s32 TotalLipSyncSize;

    f = x_fopen( Filename, "rb" );
    if( f )
    {
        s32 n;

        // Skip past identifier
        SkipVersionIdentifier( f );

        // Read compression method.
        x_fread( &n, sizeof(s32), 1, f );
        if( ReverseEndian )
            n = reverse_endian_32( n );
        if( CompressionMethod )
            *CompressionMethod = n;

        // Determine number of channels based on compression type.
        switch( n )
        {
            case GCN_ADPCM_MONO:
                nChannels = 1;
                break;

            case GCN_ADPCM_NON_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            case PS2_ADPCM_MONO:
                nChannels = 1;
                break;

            case PS2_ADPCM_NON_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            case PC_PCM_MONO:
                nChannels = 1;
                break;

            case PC_PCM_NON_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            case XBOX_ADPCM_MONO:
                nChannels = 1;
                break;

            case XBOX_ADPCM_NON_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            case PC_ADPCM_MONO:
                nChannels = 1;
                break;

            case PC_ADPCM_NON_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            case PC_MP3_MONO:
                nChannels = 1;
                break;

            case PC_MP3_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            case GCN_MP3_MONO:
                nChannels = 1;
                break;

            case GCN_MP3_INTERLEAVED_STEREO:
                nChannels = 2;
                break;

            default:
                ASSERT(0);
                break;
        }

        // Set number of channels.
        if( NumChannels )
            *NumChannels = nChannels;
        
        // Read the compressed size.
        x_fread( &n, sizeof(s32), 1, f );
        if( ReverseEndian )
            n = reverse_endian_32( n );

        // Calculate the total compressed size.
        TotalCompressedSize = n * nChannels;
        if( CompressedSize )
            *CompressedSize = TotalCompressedSize;

        // Read the lip sync size
        x_fread( &n, sizeof(s32), 1, f );
        if( ReverseEndian )
            n = reverse_endian_32( n );

        // Calculate the total lip sync size.
        TotalLipSyncSize = n * nChannels;
        if( LipSyncSize )
            *LipSyncSize = TotalLipSyncSize;

        // Read the break point size.
        x_fread( &n, sizeof(s32), 1, f );
        if( ReverseEndian )
            n = reverse_endian_32( n );

        // Set break point size.
        if( BreakPointSize )
            *BreakPointSize = n;

        // Close the file.
        x_fclose( f );

        // its all good!
        return TRUE;
    }
    else
    {
        // Could not open file!
        x_printf( "Error! File '%s' not found!\n", Filename );
        x_DebugMsg( "Error! File '%s' not found!\n", Filename );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }
}

//------------------------------------------------------------------------------

u32 CompressAudioFile( X_FILE* in, X_FILE* out, s32 Temperature, s32* NumChannels, s32* LipSyncSize, s32 Target )
{
    WriteVersionIdentifier( out, Target );
    s32 CompressionType;
    u32 Result        = 0;
    s32 SyncSize      = 0;

    CompressionType = s_CompressionTypes[Target][Temperature];

    switch( CompressionType )
    {
        case ADPCM:
            switch( Target )
            {
                case EXPORT_PC:
                    Result = CompressAudioFilePC_ADPCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_PS2:
                    return CompressAudioFilePS2_ADPCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_GCN:
                    Result = CompressAudioFileGCN_ADPCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_XBOX:
                    Result = CompressAudioFileXBOX_ADPCM( in, out, NumChannels, &SyncSize );
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }
            break;

        case PCM:
            switch( Target )
            {
                case EXPORT_PC:
                    Result = CompressAudioFilePC_PCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_XBOX:
                    Result = CompressAudioFileXBOX_PCM( in, out, NumChannels, &SyncSize );
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }
            break;

        case MP3:
            switch( Target )
            {
                case EXPORT_PC:
                    Result = CompressAudioFilePC_MP3( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_GCN:
                    Result = CompressAudioFileGCN_MP3( in, out, NumChannels, &SyncSize );
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }
            break;

        default:
            ASSERT( 0 );
            break;
    }

    if( Result && LipSyncSize )
        *LipSyncSize = SyncSize;

    x_fclose( out );
    x_fclose( in );

    return Result;
}

//------------------------------------------------------------------------------

xbool CompressAudioFiles( void )
{
    static char Drive[10];
    static char Dir[256];
    static char Filename[128];
    static char Ext[64];
    static char FullFilename[256];
    static char FullPathname[256];
    s32         i;
    s32         j;

    if( s_Verbose )
    {
        x_printf( "Compressing audio files...\n" );
        x_DebugMsg( "Compressing audio files...\n" );
    }

    // for each target
    for( i=0 ; i<s_Targets.GetCount() ; i++ )
    {
        s32   Target        = s_Targets[i];
        xbool ReverseEndian = (Target == EXPORT_GCN); 

        // for each file...
        for( j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
        {
            X_FILE*    In;
            X_FILE*    Out;
            xbool      Compress = FALSE;
            file_info& File = s_Package.m_Files[j];

            // Split out the full filename
            x_splitpath( File.Filename, Drive, Dir, Filename, Ext );

            // Create the pathname and filename
            x_sprintf( FullPathname, "%s%s%s", Drive, Dir, s_FileExtensions[ Target ] );
            x_sprintf( FullFilename, "%s\\%s.%s", FullPathname, Filename, s_FileExtensions[ Target ] );
            x_sprintf( MP3Filename, "%s\\%s.mp3", FullPathname, Filename );

            if( s_Verbose )
            {
                x_printf( "    %s%s...", Filename, Ext  );
                x_DebugMsg( "    %s%s...", Filename, Ext );
            }

            // Forced compression of all files?
            if( s_Clean )
            {
                // Ok compress it!
                Compress = TRUE;
            }
            else
            {
                // Check file date...
                Compress = ObjectFileNeedsToBeBuilt( (const char*)File.Filename, (const char*)FullFilename, Target );
                if( s_Package.m_ParseError )
                    return FALSE;
            }

            // Need to compress the file?
            if( !Compress )
            {
                if( s_Verbose )
                {
                    x_printf( "not compressing...\n"  );
                    x_DebugMsg( "not compressing...\n" );
                }

                File.CompressedFilename[Target] = FullFilename;
                if( !ReadCompressedAudioFileHeader( File.CompressedFilename[Target], ReverseEndian, &File.NumChannels[Target], NULL, &File.CompressedSize[Target], NULL, NULL ) )
                    return FALSE;
            }
            else
            {
                if( s_Verbose )
                {
                    x_printf( "compressing "  );
                    x_DebugMsg( "compressing " );
                }

                // Open the source file.
                In = x_fopen( File.Filename , "rb" );
                if( !In )
                {
                    x_printf( "Error! Source file '%s' not found!\n", File.Filename  );
                    x_DebugMsg( "Error! Source file '%s' not found!\n", File.Filename  );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {   
                    // Create the destination directory.
                    CreateDirectory( FullPathname, NULL );
                
                    // Open the output file.
                    Out = x_fopen( FullFilename, "wb" );
                    if( !Out )
                    {
                        // Close the input file.
                        x_fclose( In );
                        x_printf( "Error! Output file '%s' could not be opened!\n", FullFilename );
                        x_DebugMsg( "Error! Output file '%s' could not be opened!\n", FullFilename );
                        s_Package.m_ParseError = TRUE;
                        return FALSE;
                    }
                    else
                    {
                        // Compress the audio file.
                        File.CompressedSize[Target] = CompressAudioFile( In, Out, File.Temperature, &File.NumChannels[Target], NULL, Target );

                        if( g_SampleRate > 32000 )
                        {
                            x_printf  ( "SAMPLE RATE ERROR! Filename: %s\n", (const char*)File.Filename );
                            x_DebugMsg( "SAMPLE RATE ERROR! Filename: %s\n", (const char*)File.Filename );
                        }

                        // Error compressing the file?
                        if( File.CompressedSize[Target] )
                        {
                            File.CompressedFilename[Target] = FullFilename;
                        }
                        else
                        {
                            DeleteFile( FullFilename );    
                            File.CompressedFilename[Target] = "";
                            s_Package.m_ParseError = TRUE;
                            return FALSE;
                        }
                    }
                }
            }
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool LoadMultiChannelFile( file_info& File, multi_channel_data* Data, xbool LoadWaveform, s32 Target )
{
    X_FILE*      f;
    xbool        ReverseEndian = (Target == EXPORT_GCN);
    xbool        Result = TRUE;
    xstring      Filename;

    Data->WaveformData.Clear();
    Data->LipSyncData.Clear();
    Data->BreakPointData.Clear();
    Data->Headers.Clear();
    Data->FileHeader.Clear();

    Filename = File.CompressedFilename[ Target ];

//    x_DebugMsg( "LoadMultiChannelFile - ENTER\n" );

    f = x_fopen( (const char*)Filename, "rb" );
    if( f )
    {
        // Skip past identifier
        SkipVersionIdentifier( f );

        // Read compression method.
        x_fread( &Data->CompressionMethod, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->CompressionMethod, sizeof(s32) );
        if( ReverseEndian )
            Data->CompressionMethod = reverse_endian_32( Data->CompressionMethod );

        // Read compressed size
        x_fread( &Data->CompressedSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->CompressedSize, sizeof(s32) );
        if( ReverseEndian )
            Data->CompressedSize = reverse_endian_32( Data->CompressedSize );

        // Read lip sync size
        x_fread( &Data->LipSyncSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->LipSyncSize, sizeof(s32) );
        if( ReverseEndian )
            Data->LipSyncSize = reverse_endian_32( Data->LipSyncSize );
//        x_DebugMsg( "   LipSyncSize: %d\n", Data->LipSyncSize );

        // Read the break point size
        x_fread( &Data->BreakPointSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->BreakPointSize, sizeof(s32) );
        if( ReverseEndian )
            Data->BreakPointSize = reverse_endian_32( Data->BreakPointSize );    
//        x_DebugMsg( "   BreakPointSize: %d\n", Data->BreakPointSize );

        // Read header size.
        x_fread( &Data->HeaderSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->HeaderSize, sizeof(s32) );
        if( ReverseEndian )
            Data->HeaderSize = reverse_endian_32( Data->HeaderSize );

        // Determine number of channels and waveforms based on compression type.
        switch( Data->CompressionMethod )
        {
            case GCN_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case GCN_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case GCN_ADPCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case GCN_ADPCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case PS2_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PS2_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case PS2_ADPCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case PS2_ADPCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case PC_PCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PC_PCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case PC_PCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case PC_PCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case XBOX_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case XBOX_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case XBOX_ADPCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case XBOX_ADPCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case PC_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PC_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;

            case PC_MP3_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PC_MP3_INTERLEAVED_STEREO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case GCN_MP3_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case GCN_MP3_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            default:
                ASSERT(0);
                Result = FALSE;
                break;
        }

        // Still good?
        if( Result )
        {
            s32 i;
    
            // Only support stereo for now...
            ASSERT( (Data->nChannels == 1) || (Data->nChannels == 2) );

            // Read the headers.
            for( i=0 ; i<Data->nChannels ; i++ )
            {
                // Read the header.
                Data->Headers.Append() = x_malloc( Data->HeaderSize );
                x_fread( Data->Headers[i], Data->HeaderSize, 1, f );
                Data->FileHeader.Append( (u8*)Data->Headers[i], Data->HeaderSize );
            }

            // Load the waveforms?
            if( LoadWaveform )
            {
                // Allocate buffer to load into.
                void* pBuffer = x_malloc( Data->CompressedSize );

                // Now read the waveform data
                for( i=0 ; i<Data->nWaveforms ; i++ )
                {
                    x_fread( pBuffer, Data->CompressedSize, 1, f );
                    Data->WaveformData.Append();
                    Data->WaveformData[i].Append( (u8*)pBuffer, Data->CompressedSize );
                }

                // Free the buffer
                x_free( pBuffer );

                // Allocate buffer to load into.
                pBuffer = x_malloc( Data->LipSyncSize );

                // Now read the lip sync data.
                for( i=0 ; i<1 ; i++ )
                {
//                    x_DebugMsg( "   LipSyncOffset: %d\n", x_ftell( f ) );
                    x_fread( pBuffer, Data->LipSyncSize, 1, f );
                    Data->LipSyncData.Append();
                    Data->LipSyncData[i].Append( (u8*)pBuffer, Data->LipSyncSize );
                }

                // Free the buffer
                x_free( pBuffer );

                // Now read the break point data, if it exists.
                if( Data->BreakPointSize > 0 )
                {
                    // Allocate space.
                    pBuffer = x_malloc( Data->BreakPointSize );

//                    x_DebugMsg( "   BreakPointOffset: %d\n", x_ftell( f ) );
                    x_fread( pBuffer, Data->BreakPointSize, 1, f );
                    Data->BreakPointData.Append( (u8*)pBuffer, Data->BreakPointSize );
                    
                    // Free the buffer
                    x_free( pBuffer );
                }
            }
        }

        // Close the file.
        x_fclose( f );
    }

//    x_DebugMsg( "LoadMultiChannelFile - EXIT\n" );

    // Tell the world!
    return( Result );
}

//------------------------------------------------------------------------------

xbool LoadTrueMultiChannelFile( file_info& File, multi_channel_data* Data, xbool LoadWaveform, s32 Target )
{
    X_FILE*      f;
    xbool        ReverseEndian = (Target == EXPORT_GCN);
    xbool        Result = TRUE;
    xstring      Filename;

    Data->WaveformData.Clear();
    Data->LipSyncData.Clear();
    Data->BreakPointData.Clear();
    Data->Headers.Clear();
    Data->FileHeader.Clear();

//    x_DebugMsg( "LoadTrueMultiChannelFile - ENTER\n" );

    switch( File.Temperature )
    {
        case HOT:
            switch( File.NumChannels[ Target ] )
            {
                case 1:
                case 2:
                    Filename = File.CompressedFilename[ Target ];
                    break;
                default:
                    ASSERT( 0 );
                    return FALSE;
                    break;
            }
            break;

        case WARM:
            switch( File.NumChannels[ Target ] )
            {
                case 1:
                    ASSERT( 0 );
                    return FALSE;
                    break;
                case 2:
                    ASSERT( 0 );
                    return FALSE;
                    break;
                default:
                    ASSERT( 0 );
                    return FALSE;
                    break;
            }
            break;

        case COLD:
            switch( File.NumChannels[ Target ] )
            {
                case 1:
                case 2:
                    Filename = File.CompressedInterleavedFilename[ Target ];
                    break;
                default:
                    ASSERT( 0 );
                    return FALSE;
                    break;
            }
            break;
    }

    f = x_fopen( (const char*)Filename, "rb" );
    if( f )
    {
        // Skip past identifier
        SkipVersionIdentifier( f );

        // Read compression method.
        x_fread( &Data->CompressionMethod, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->CompressionMethod, sizeof(s32) );
        if( ReverseEndian )
            Data->CompressionMethod = reverse_endian_32( Data->CompressionMethod );

        // Read compressed size
        x_fread( &Data->CompressedSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->CompressedSize, sizeof(s32) );
        if( ReverseEndian )
            Data->CompressedSize = reverse_endian_32( Data->CompressedSize );

        // Read lip sync size
        x_fread( &Data->LipSyncSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->LipSyncSize, sizeof(s32) );
        if( ReverseEndian )
            Data->LipSyncSize = reverse_endian_32( Data->LipSyncSize );
//        x_DebugMsg( "   LipSyncSize: %d\n", Data->LipSyncSize );

        // Read the breakpoint size
        x_fread( &Data->BreakPointSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->BreakPointSize, sizeof(s32) );
        if( ReverseEndian )
            Data->BreakPointSize = reverse_endian_32( Data->BreakPointSize );
//        x_DebugMsg( "   BreakPointSize: %d\n", Data->BreakPointSize );

        // Read header size.
        x_fread( &Data->HeaderSize, sizeof(s32), 1, f );
        Data->FileHeader.Append( (u8*)&Data->HeaderSize, sizeof(s32) );
        if( ReverseEndian )
            Data->HeaderSize = reverse_endian_32( Data->HeaderSize );

        // Determine number of channels and waveforms based on compression type.
        switch( Data->CompressionMethod )
        {
            case GCN_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case GCN_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case GCN_ADPCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case GCN_ADPCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case PS2_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PS2_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case PS2_ADPCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case PS2_ADPCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case PC_PCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PC_PCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case PC_PCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case PC_PCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case XBOX_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case XBOX_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;
            
            case XBOX_ADPCM_MONO_STREAMED:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;
            
            case XBOX_ADPCM_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case PC_ADPCM_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PC_ADPCM_NON_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 2;
                break;

            case PC_MP3_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case PC_MP3_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            case GCN_MP3_MONO:
                Data->nChannels  = 1;
                Data->nWaveforms = 1;
                break;

            case GCN_MP3_INTERLEAVED_STEREO:
                Data->nChannels  = 2;
                Data->nWaveforms = 1;
                break;

            default:
                ASSERT(0);
                Result = FALSE;
                break;
        }

        // Still good?
        if( Result )
        {
            s32 i;
    
            // Only support stereo for now...
            ASSERT( (Data->nChannels == 1) || (Data->nChannels == 2) );

            // Read the headers.
            for( i=0 ; i<Data->nChannels ; i++ )
            {
                // Read the header.
                Data->Headers.Append() = x_malloc( Data->HeaderSize );
                x_fread( Data->Headers[i], Data->HeaderSize, 1, f );
                Data->FileHeader.Append( (u8*)Data->Headers[i], Data->HeaderSize );
            }

            // Load the waveforms?
            if( LoadWaveform )
            {
                // Allocate buffer to load into.
                void* pBuffer = x_malloc( Data->CompressedSize );

                // Now read the waveform data
                for( i=0 ; i<Data->nWaveforms ; i++ )
                {
                    x_fread( pBuffer, Data->CompressedSize, 1, f );
                    Data->WaveformData.Append();
                    Data->WaveformData[i].Append( (u8*)pBuffer, Data->CompressedSize );
                }

                // Free the buffer
                x_free( pBuffer );

                // Allocate buffer to load into.
                pBuffer = x_malloc( Data->LipSyncSize );

                // Now read the lip sync data.
                for( i=0 ; i<1 ; i++ )
                {
//                    x_DebugMsg( "   LipSyncOffset: %d\n", x_ftell( f ) );
                    x_fread( pBuffer, Data->LipSyncSize, 1, f );
                    if( i==0 )
                    {
                        Data->LipSyncData.Append();
                        Data->LipSyncData[i].Append( (u8*)pBuffer, Data->LipSyncSize );
                    }
                }

                // Free the buffer
                x_free( pBuffer );

                // Now read the breakpoints, if they exist.
                if( Data->BreakPointSize > 0 )
                {
                    // Allocate buffer to load into.
                    pBuffer = x_malloc( Data->BreakPointSize );

                    // Now read the breakpoint data.
//                    x_DebugMsg( "   BreakPointOffset: %d\n", x_ftell( f ) );
                    x_fread( pBuffer, Data->BreakPointSize, 1, f );
/*                    s32 *pnBreakPoints = (s32*)pBuffer;
                    s32 nBreakPoints = *pnBreakPoints++;
                    if( ReverseEndian )
                        nBreakPoints = reverse_endian_32( nBreakPoints );
                    x_DebugMsg( "Breakpoints[%d]: ", nBreakPoints );
                    f32 *pBreakPoints = (f32*)pnBreakPoints;
                    for( s32 k=0 ; k<nBreakPoints ; k++ )
                    {
                        f32 f = *pBreakPoints++;
                        if( ReverseEndian )
                            f = reverse_endian_f32( f );
                        x_DebugMsg( "%07.3f ", f );
                    }
                    x_DebugMsg( "\n" );
*/
                    Data->BreakPointData.Append( (u8*)pBuffer, Data->BreakPointSize );

                    // Free the buffer
                    x_free( pBuffer );
                }
            }
        }

        // Close the file.
        x_fclose( f );
    }

//    x_DebugMsg( "LoadTrueMultiChannelFile - EXIT\n" );

    // Tell the world!
    return( Result );
}

//------------------------------------------------------------------------------

xbool SplitStereoFile( file_info& File, s32 Target )
{
    multi_channel_data Data;
    xbool              Result = FALSE;
    xbool              ReverseEndian = (Target == EXPORT_GCN);

    // Load the multi channel file
    if( LoadMultiChannelFile( File, &Data, TRUE, Target ) )
    {
        // This only works for stereo files...
        if( (Data.nChannels == 2) && (Data.nWaveforms == 2) )
        {
            s32 i;

            // Default now is all good...
            Result = TRUE;

            // For each channel
            for( i=0 ; i<Data.nChannels ; i++ )
            {
                X_FILE* f;
                xbool Compress;
                static char  Drive[10];
                static char  Dir[256];
                static char  Filename[128];
                static char  Ext[64];
                static char* Extensions[2] = { "_left", "_right" };

                // Split out the full filename
                x_splitpath( File.CompressedFilename[Target], Drive, Dir, Filename, Ext );

                // Create the filename.
                File.CompressedStereoFilenames[Target][i].AddFormat( "%s%s%s%s%s", Drive, Dir, Filename, Extensions[i], Ext );
                
                // Check file date...
                Compress = ObjectFileNeedsToBeBuilt( (const char*)File.CompressedFilename[Target], (const char*)File.CompressedStereoFilenames[Target][i], Target );
                if( s_Package.m_ParseError )
                    return FALSE;

                if( Compress )
                {
                    if( s_Debug )
                    {
                        x_printf  ( "    Splitting out %s\n", (const char*)File.CompressedStereoFilenames[Target][i] );
                        x_DebugMsg( "    Splitting out %s\n", (const char*)File.CompressedStereoFilenames[Target][i] );
                    }

                    f = x_fopen( (const char*)File.CompressedStereoFilenames[Target][i], "wb" );
                    if( f == NULL )
                    {
                        Result = FALSE;
                    }
                    else
                    {
                        // Write the version identifier.
                        WriteVersionIdentifier( f, Target );

                        s32 xCompressionMethod = Data.CompressionMethod;
                        s32 xCompressedSize    = Data.CompressedSize;
                        s32 xLipSyncSize       = Data.LipSyncSize;
                        s32 xBreakPointSize    = Data.BreakPointSize;
                        s32 xHeaderSize        = Data.HeaderSize;

                        if( ReverseEndian )
                        {
                            xCompressionMethod = reverse_endian_32( xCompressionMethod );
                            xCompressedSize    = reverse_endian_32( xCompressedSize );
                            xLipSyncSize       = reverse_endian_32( xLipSyncSize );
                            xBreakPointSize    = reverse_endian_32( xBreakPointSize );
                            xHeaderSize        = reverse_endian_32( xHeaderSize );
                        }

                        // Write out the compression data.
                        x_fwrite( &xCompressionMethod, sizeof(s32), 1, f );
                        x_fwrite( &xCompressedSize, sizeof(s32), 1, f );
                        x_fwrite( &xLipSyncSize, sizeof(s32), 1, f );
                        x_fwrite( &xBreakPointSize, sizeof(s32), 1, f );
                        x_fwrite( &xHeaderSize, sizeof(s32), 1, f );

                        // Write out the sample header.
                        x_fwrite( Data.Headers[i], Data.HeaderSize, 1, f );

                        // Write out the sample waveform.
                        x_fwrite( Data.WaveformData[i].GetBuffer(), Data.CompressedSize, 1, f );

                        // Write out the lipsync data. only one damn lipsync.
                        x_fwrite( Data.LipSyncData[0].GetBuffer(), Data.LipSyncSize, 1, f );

                        // Write out the break points, if they exist.
                        if( Data.BreakPointSize )
                            x_fwrite( Data.BreakPointData.GetBuffer(), Data.BreakPointSize, 1, f );

                        // Close the file
                        x_fclose( f );
                    }
                }

                // Free the header memory.
                x_free( Data.Headers[i] );
            }
        }
    }

    return Result;
}

//------------------------------------------------------------------------------

xbool InterleaveStreamedAudio( file_info& File, s32 Target )
{
    multi_channel_data Data;
    xbool              Result        = FALSE;
    xbool              ReverseEndian = (Target == EXPORT_GCN);
    xbool              bIsXbox       = FALSE;
    xbool              bIsADPCM      = FALSE;
    xbool              bIsMP3        = FALSE;

//    x_DebugMsg( "InterleaveStreamedAudio - ENTER\n" );

    // Load the multi channel file
    if( LoadMultiChannelFile( File, &Data, TRUE, Target ) )
    {
        // This only works for mono and stereo files...
        if( ((Data.nChannels == 1) && (Data.nWaveforms == 1)) || 
            ((Data.nChannels == 2) && (Data.nWaveforms == 2)) ||
            ((Data.nChannels == 2) && (Data.nWaveforms == 1)) )
        {
            X_FILE* f;
            xbool   Compress;
            static char  Drive[10];
            static char  Dir[256];
            static char  Filename[128];
            static char  Ext[64];
            static char* Extension = "_stream";
            static char  ZeroBuffer[2048];

            // Default is now good.
            Result = TRUE;

            // Nuke the sector buffer
            x_memset( ZeroBuffer, 0, 2048 );

            // Split out the full filename
            x_splitpath( File.CompressedFilename[Target], Drive, Dir, Filename, Ext );

            // Create the filename.
            File.CompressedInterleavedFilename[Target].AddFormat( "%s%s%s%s%s", Drive, Dir, Filename, Extension, Ext );
            
            // Check file date...
            Compress = ObjectFileNeedsToBeBuilt( (const char*)File.CompressedFilename[Target], (const char*)File.CompressedInterleavedFilename[Target], Target );
            if( s_Package.m_ParseError )
                return FALSE;
            
            if( Compress )
            {
                f = x_fopen( (const char*)File.CompressedInterleavedFilename[Target], "wb" );
                if( f  )
                {
                    s32 BufferSize;
                    s32 CompressedSize;
                    s32 PaddedCompressedSize;
                    s32 DataSize[2];
                    s32 DataPadding[2];
                    s32 NewCompressionMethod;

                    // Default is the number of channels (but we "cheat" for stereo mp3 files).
                    s32 nChannels = Data.nChannels;

                    // Write the version identifier.
                    WriteVersionIdentifier( f, Target );

                    if( s_Debug )
                    { 
                        x_printf  ( "    Interleave.%d  %s\n", Data.nChannels, (const char*)File.CompressedInterleavedFilename[Target] );
                        x_DebugMsg( "    Interleave.%d  %s\n", Data.nChannels, (const char*)File.CompressedInterleavedFilename[Target] );
                    }

                    // Decide what to do based on compression method.
                    switch( Data.CompressionMethod )
                    {
                        case GCN_ADPCM_MONO:
                            BufferSize           = GCN_ADPCM_CHANNEL_BUFFER_SIZE;                            
                            NewCompressionMethod = GCN_ADPCM_MONO_STREAMED;
                            break;

                        case GCN_ADPCM_NON_INTERLEAVED_STEREO:
                            BufferSize           = GCN_ADPCM_CHANNEL_BUFFER_SIZE;
                            NewCompressionMethod = GCN_ADPCM_INTERLEAVED_STEREO;
                            break;

                        case PS2_ADPCM_MONO:
                            BufferSize           = PS2_ADPCM_CHANNEL_BUFFER_SIZE;                            
                            NewCompressionMethod = PS2_ADPCM_MONO_STREAMED;
                            break;

                        case PS2_ADPCM_NON_INTERLEAVED_STEREO:
                            BufferSize           = PS2_ADPCM_CHANNEL_BUFFER_SIZE;
                            NewCompressionMethod = PS2_ADPCM_INTERLEAVED_STEREO;
                            break;

                        case PC_PCM_MONO:
                            BufferSize           = 128 * 1024;
                            NewCompressionMethod = PC_PCM_MONO_STREAMED;
                            break;

                        case PC_PCM_NON_INTERLEAVED_STEREO:
                            BufferSize           = 128 * 1024;
                            NewCompressionMethod = PC_PCM_INTERLEAVED_STEREO;
                            break;

                        case XBOX_ADPCM_MONO:
                            BufferSize           = XBOX_ADPCM_CHANNEL_BUFFER_SIZE;
                            NewCompressionMethod = XBOX_ADPCM_MONO_STREAMED;
                            bIsADPCM             = TRUE;
                            bIsXbox              = TRUE;
                            break;

                        case XBOX_ADPCM_NON_INTERLEAVED_STEREO:
                            BufferSize           = XBOX_ADPCM_CHANNEL_BUFFER_SIZE;
                            NewCompressionMethod = XBOX_ADPCM_INTERLEAVED_STEREO;
                            bIsADPCM             = TRUE;
                            bIsXbox              = TRUE;
                            break;

                        case GCN_MP3_MONO:
                        case GCN_MP3_INTERLEAVED_STEREO:
                            nChannels            = 1; // cheat for "interleaved" stereo mp3 files.
                            BufferSize           = MP3_BUFFER_SIZE;
                            NewCompressionMethod = Data.CompressionMethod;
                            bIsMP3               = TRUE;
                            break;

						case PC_MP3_MONO:
						case PC_MP3_INTERLEAVED_STEREO:
                            nChannels            = 1; // cheat for "interleaved" stereo mp3 files.
                            BufferSize           = MP3_BUFFER_SIZE;
                            NewCompressionMethod = Data.CompressionMethod;
                            bIsMP3               = TRUE;
                            break;

                        default:
                            ASSERT(0);
                            Result = FALSE;
                            break;
                    }

                    // No odd data padding.
                    DataPadding[0] = 
                    DataPadding[1] = 0;

                    // No odd sizes.
                    DataSize[0] =
                    DataSize[1] = BufferSize;

                    // Compressed size is normal.
                    CompressedSize = Data.CompressedSize;

                    // Calculate the padded compressed size (funky for xbox admpcm).
                    if( bIsXbox && bIsADPCM )
                    {
                        // GAH...xbox is so screwy... 
                        // *  2 because the audio is double buffered.
                        // % 36 because 64 samples adpcm compressed take 36 bytes.
                        s32 Waste = (BufferSize * 2) % 36;
                        
                        // Evenly divisible?
                        if( Waste )
                        {
                            // Re-calculate compressed size.
                            s32 NewBufferSize = (BufferSize * 2) - Waste;
                            s32 Blocks        = CompressedSize / NewBufferSize;
                            s32 Remainder     = CompressedSize % NewBufferSize;
                            CompressedSize    = Blocks * BufferSize * 2 + Remainder;
                             
                            // Handle the odd buffers now.
                            DataPadding[1] = Waste;
                            DataSize[1]   -= Waste;
                        }
                    }

                    // Now compute the padded compressed size.
                    PaddedCompressedSize = (BufferSize * ((CompressedSize + BufferSize - 1) / BufferSize));

                    // Handle endian.
                    s32 xCompressionMethod = NewCompressionMethod;
                    s32 xCompressedSize    = PaddedCompressedSize * nChannels; // NOTE: this is a "cheat" for stereo mp3 files.
                    s32 xLipSyncSize       = Data.LipSyncSize;
                    s32 xBreakPointSize    = Data.BreakPointSize;
                    s32 xHeaderSize        = Data.HeaderSize;
                    if( ReverseEndian )
                    {
                        xCompressionMethod = reverse_endian_32( xCompressionMethod );
                        xCompressedSize    = reverse_endian_32( xCompressedSize );
                        xLipSyncSize       = reverse_endian_32( xLipSyncSize );
                        xBreakPointSize    = reverse_endian_32( xBreakPointSize );
                        xHeaderSize        = reverse_endian_32( xHeaderSize );
                    }

                    // Write out the compression data.
                    x_fwrite( &xCompressionMethod, sizeof(s32), 1, f );
                    x_fwrite( &xCompressedSize, sizeof(s32), 1, f );
                    x_fwrite( &xLipSyncSize, sizeof(s32), 1, f );
                    x_fwrite( &xBreakPointSize, sizeof(s32), 1, f );
                    x_fwrite( &xHeaderSize, sizeof(s32), 1, f );

                    // Write out the headers.
                    for( s32 i=0 ; i<Data.nChannels ; i++ )
                    {
                        // Write out the sample header.
                        x_fwrite( Data.Headers[i], Data.HeaderSize, 1, f );
                    }

                    // Interleave the waveform data.
                    s32 EvenOdd    = 0;
                    s32 BytesWrote = 0;
                    for( i=0 ; i<Data.CompressedSize ; i+=DataSize[ EvenOdd ], EvenOdd ^= 1 )
                    {
                        s32 nBytes    = DataSize   [ EvenOdd ];
                        s32 nPadBytes = DataPadding[ EvenOdd ];

                        // This the last bit?
                        if( (i+nBytes) > Data.CompressedSize )
                        {
                            // Calculate number of bytes.
                            nBytes = Data.CompressedSize - i;
                        }
                        ASSERT( (i+nBytes) <= Data.CompressedSize );

                        // Adjust for data written.
                        BytesWrote += nBytes;
                        
                        // Need to pad it out?
                        if( (i+nBytes) == Data.CompressedSize )
                        {
                            // Calculate pad.
                            nPadBytes = PaddedCompressedSize - BytesWrote;
                        }

                        // Adjust for pad written
                        BytesWrote += nPadBytes;

                        // For each channel...(NOTE: we "cheat" for mp3 stereo files)
                        for( s32 j=0 ; j<nChannels ; j++ )
                        {
                            ASSERT( nBytes );

                            // Write out the data.
                            s32 nBytes2 = x_fwrite( Data.WaveformData[j].GetBuffer()+i, nBytes, 1, f );
                            ASSERT( 1 == nBytes2 );

                            // Need to pad?
                            if( nPadBytes )
                            {
                                nBytes2 = nPadBytes;
                                while( nBytes2-- )
                                {
                                    // Pad it.
                                    s32 nBytes3 = x_fwrite( ZeroBuffer, 1, 1, f );
                                    ASSERT( 1 == nBytes3 );
                                }
                            }
                        }
                    }

                    ASSERT( BytesWrote == PaddedCompressedSize );

                    // Write out the lip sync data.
                    for( i=0 ; i<1 ; i++ )
                    {
//                        x_DebugMsg( "   LipSyncOffset: %d, Size: %d\n", x_ftell(f), Data.LipSyncSize );
                        if( bIsMP3 )
                        {
                            // Write out the lip sync data
                            x_fwrite( Data.LipSyncData[0].GetBuffer(), Data.LipSyncSize, 1, f );
                        }
                        else
                        {
                            // Write out the lip sync data
                            x_fwrite( Data.LipSyncData[i].GetBuffer(), Data.LipSyncSize, 1, f );
                        }
                    }

                    // Write out the break points, if they exist.
                    if( Data.BreakPointSize > 0 )
                    {
//                        x_DebugMsg( "   BreakPointOffset: %d, Size: %d\n", x_ftell(f), Data.BreakPointSize );
                        x_fwrite( Data.BreakPointData.GetBuffer(), Data.BreakPointSize, 1, f );
                    }                    

                    // All done!
                    x_fclose( f );
                }
            }

            // Free up the header buffers.
            for( s32 i=0 ; i<Data.nChannels ; i++ )
            {
                // Free the memory.
                x_free( Data.Headers[i] );
            }
        }
    }

//    x_DebugMsg( "InterleaveStreamedAudio - EXIT\n" );

    return Result;
}

//------------------------------------------------------------------------------

xbool ProcessMultiChannelAudio( void )
{
    s32 i;
    s32 j;

    if( s_Verbose )
    {
        x_printf( "Processing streamed/stereo files...\n" );
        x_DebugMsg( "Processing streamed/stereo files...\n" );
    }

    // for each target
    for( i=0 ; i<s_Targets.GetCount() ; i++ )
    {
        s32 Target = s_Targets[i];

        // for each file...
        for( j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
        {
            file_info& File = s_Package.m_Files[j];

            switch( File.Temperature )
            {
                case HOT:
                    if( File.NumChannels[Target] == 2 )
                    {
                        // Split out the stereo file...
                        if( !SplitStereoFile( File, Target ) )
                            return FALSE;
                    }
                    break;

                case WARM:
                    // TODO: Put in warm sample support.
                    ASSERT( 0 );
                    break;

                case COLD:
                    if( !InterleaveStreamedAudio( File, Target ) )
                        return FALSE;
                    break;
            }
        }
    }

    return TRUE;
}

static s32 nBytesInLine   = 0;
static s32 nBytesInBuffer = 0;

//------------------------------------------------------------------------------

void DumpByte( X_FILE* f, u8 data, xbool bIsAscii )
{
    if( nBytesInLine >= 16 )
    {
        nBytesInLine = 0;
        x_fprintf( f, "\n" );
    }

    if( bIsAscii )
    {
        x_fprintf( f, " '%c',", data );
    }
    else
    {
        x_fprintf( f, "0x%02x,", data );
    }

    nBytesInBuffer++;
    nBytesInLine++;
}

void DumpString( X_FILE* f, char* pData )
{
    // Write out length of string.
    u8 n = x_strlen( pData )+2;

    // 32 bit align it.
    n = (n+3) & ~3;

    DumpByte( f, (char)n, FALSE );
    n--;

    while( pData && *pData )
    {
        DumpByte( f, *pData, TRUE );
        pData++;
        n--;
    }

    // Terminate string.
    DumpByte( f, 0, FALSE );
    n--;

    while( n-- )
        DumpByte( f, 0, FALSE );
}

//------------------------------------------------------------------------------

void DumpSamples( void )
{
    static char Drive[10];
    static char Dir[256];
    static char Filename[128];
    static char Ext[64];
    static char FullFilename[256];
    s32         i;
    s32         j;
    s32         k;
    multi_channel_data Data;


    if( s_Verbose )
    {
        x_printf( "Dumping samples...\n" );
        x_DebugMsg( "Dumping samples...\n" );
    }

    // for each target
    for( i=0 ; i<s_Targets.GetCount() ; i++ )
    {
        s32   Target        = s_Targets[i];
        xbool ReverseEndian = (Target == EXPORT_GCN); 

        // for each file...
        for( j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
        {
            file_info&  File = s_Package.m_Files[j];
            X_FILE*     out;
            const char* pDefine = "";
            const char* pTarget = "";

            nBytesInLine   = 0;
            nBytesInBuffer = 0;
             
            // Load the multi channel file
            if( LoadMultiChannelFile( File, &Data, TRUE, Target ) )
            {
                if( (Data.nChannels==1) && (Data.nWaveforms==1) )
                {
                    // Split out the full filename
                    x_splitpath( File.Filename, Drive, Dir, Filename, Ext );
                    
                    switch( Target )
                    {
                        case EXPORT_GCN:
                            pDefine = "#ifdef TARGET_GCN";
                            pTarget = "gcn";
                            break;

                        case EXPORT_PC:
                            pDefine = "#ifdef TARGET_PC";
                            pTarget = "pc";
                            break;

                        case EXPORT_PS2:
                            pDefine = "#ifdef TARGET_PS2";
                            pTarget = "ps2";
                            break;

                        case EXPORT_XBOX:
                            pDefine = "#ifdef TARGET_XBOX";
                            pTarget = "xbox";
                            break;
                    }

                    // Build filename.
                    x_sprintf( FullFilename, "%s_%s.hpp", Filename, pTarget );

                    // Open the output file.
                    out = x_fopen( FullFilename, "w+t" );
                    if( out )
                    {
                        if( s_Verbose )
                        {
                            x_printf  ( "    %s <== %s\n", FullFilename, File.Filename  );
                            x_DebugMsg( "    %s <== %s\n", FullFilename, File.Filename );
                        }
                        
                        x_fprintf( out, "%s\n\n", pDefine );
                        x_fprintf( out, "unsigned char AUDIO_WARN_%s[] =\n", x_strtoupper( Filename ) );  
                        x_fprintf( out, "{\n" );

                        // Dump out the name.
                        DumpString( out, Filename );

                        // Make space for audio ram reference and 32-bit align the number of bytes.
                        s32 nBytes = Data.FileHeader.GetLength()+4+4;
                        s32 nPad   = (nBytesInBuffer + nBytes) & 31;
                        if( nPad )
                            nBytes += 32-nPad; 

                        // Dump total size of header.
                        DumpByte( out, nBytes, FALSE );
                        DumpByte( out, 0, FALSE );
                        DumpByte( out, 0, FALSE );
                        DumpByte( out, 0, FALSE );

                        // Make space for audio ram reference.
                        DumpByte( out, 0, FALSE );
                        DumpByte( out, 0, FALSE );
                        DumpByte( out, 0, FALSE );
                        DumpByte( out, 0, FALSE );

                        // Dump the header.
                        for( k=0 ; k<Data.FileHeader.GetLength() ; k++ )
                            DumpByte( out, Data.FileHeader.GetAt( k ), FALSE );
                        
                        // Calculate bytes to pad.
                        nBytes -= (Data.FileHeader.GetLength()+4+4);
                        while( nBytes-- )
                            DumpByte( out, 0, FALSE );                        

                        // Now dump the compressed sample data.
                        for( k=0 ; k<Data.CompressedSize ; k++ )
                        {   
                            DumpByte( out, Data.WaveformData[0].GetAt(k), FALSE ); 
                        }
                        
                        x_fprintf( out, "\n}; GCN_ALIGNMENT(32)\n\n#endif\n\n" );
                        x_fclose( out );                   
                    }
                }
            }
        }
    }
}

