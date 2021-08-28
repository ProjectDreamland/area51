//==============================================================================
//==============================================================================
// SoundPackager.cpp
//==============================================================================
//==============================================================================

#include "stdafx.h"

#include "Auxiliary/CommandLine/CommandLine.hpp"
#include "x_bytestream.hpp"
#include "DspTool.hpp"
#include "..\..\..\xCore\parsing\tokenizer.hpp"
#include "x_debug.hpp"
#include "aiff_file.hpp"
#include "endian.hpp"
#include <windows.h>
#include <stdlib.h>
#include "..\..\..\xCore\Entropy\Audio\audio_private.hpp"
extern "C"
{
	#include "encvag.h"
};

//------------------------------------------------------------------------------

enum export_targets
{
    EXPORT_PC,
    EXPORT_PS2,
    EXPORT_GCN,
    EXPORT_XBOX,
    EXPORT_NUM_TARGETS
};

enum {
    NONE=0,
    PACKAGE,
    FILES,
    DESCRIPTORS,
    DONE,
};

//------------------------------------------------------------------------------


#define PACKAGE_SECTION    "package:"
#define FILES_SECTION      "files:"
#define DESCRIPTOR_SECTION "descriptors:"

//------------------------------------------------------------------------------

struct multi_channel_data
{
    s32                     nChannels;
    s32                     nWaveforms;
    s32                     CompressionMethod;
    s32                     CompressedSize;
    s32                     LipSyncSize;
    s32                     HeaderSize;
    xarray<xbytestream>     WaveformData;
    xarray<xbytestream>     LipSyncData;
    xarray<void*>           Headers;
    xbytestream             FileHeader;
};                          

struct file_info
{
    xstring                 Identifier;
    u32                     Temperature;
    s32                     LipSyncOffset;
    xbool                   UsesLipSync;
    u32                     Index;
    s32                     NumChannels[EXPORT_NUM_TARGETS];
    xstring                 Filename;
    s32                     CompressedSize[EXPORT_NUM_TARGETS];
    xstring                 CompressedFilename[EXPORT_NUM_TARGETS];
    xstring                 CompressedStereoFilenames[EXPORT_NUM_TARGETS][2];
    xstring                 CompressedInterleavedFilename[EXPORT_NUM_TARGETS];
};

struct element_info
{
    xstring                 Identifier;
    u32                     Type;
    u32                     Index;
    compressed_parameters   Params;
    xbool                   IsExported;
    union 
    {
        u32                 Weight;
        f32                 StartDelay;
    };
};

struct descriptor_info
{
    xstring                 Identifier;
    u32                     Type;
    u32                     Index;
    xbool                   IdentifierProcessed;
    compressed_parameters   Params;
    xarray<element_info>    Elements;
};

struct package_info
{
    xstring                 m_Identifier[EXPORT_NUM_TARGETS];
    s32                     m_ParseSection;
    xbool                   m_ParseError;
    compressed_parameters   m_Params;
    s32                     m_AudioRamFootprint;
    s32                     m_MainRamFootprint;
    xbytestream             m_HeaderStream;
    xarray<u32>             m_DescriptorOffsets;
    xbytestream             m_DescriptorStream;
    s32                     m_TemperatureCount[NUM_TEMPERATURES];
    xarray<u32>             m_SampleOffsets[NUM_TEMPERATURES];    
    xbytestream             m_SampleStream[NUM_TEMPERATURES];
    xbytestream             m_StringTableStream;
    xbytestream             m_LipSyncTableStream;
    xbytestream             m_DescriptorIdentifierStream;
    xarray<descriptor_info> m_Descriptors;
    xarray<file_info>       m_Files;
};

//------------------------------------------------------------------------------

package_info    s_Package;
xarray<s32>     s_Targets;
xbool           s_Verbose = TRUE;
xbool           s_Debug   = TRUE;
xbool           s_Clean   = FALSE;
xbool           s_HotOnly = FALSE;
char*           s_DescriptorTypes[NUM_DESCRIPTOR_TYPES] = { "simple", "complex", "rlist", "wlist" };
char*           s_TemperatureTypes[NUM_TEMPERATURES] = { "hot", "warm", "cold" };
char*           s_ParameterTypes[NUM_PARAMETERS] = {"pitch","pitchvar","volume","volumevar","pan","priority","effect","nearclip","farclip" };
char*           s_FileExtensions[EXPORT_NUM_TARGETS] = { "snd_pc", "snd_ps2", "snd_gcn", "snd_xbox" };
//char*           s_PackageExtensions[EXPORT_NUM_TARGETS] = { "pkg_pc", "pkg_ps2", "pkg_gcn", "pkg_xbox" };
s32             s_ParameterSizes[NUM_PARAMETERS] = {2,2,2,2,1,1,1,1,1};
char*           s_CompressionTypeNames[NUM_COMPRESSION_TYPES] = { "adpcm", "pcm", "mp3" };
s32             s_DefaultCompressionTypes[EXPORT_NUM_TARGETS][NUM_TEMPERATURES] = {
{   PCM,   PCM,   PCM }, // PC
{ ADPCM, ADPCM, ADPCM }, // PS2
{ ADPCM, ADPCM, ADPCM }, // GameCube
{ ADPCM, ADPCM, ADPCM }};// Xbox
s32             s_CompressionTypes[EXPORT_NUM_TARGETS][NUM_TEMPERATURES];
s32             s_CompressionHeaderSizes[EXPORT_NUM_TARGETS][NUM_COMPRESSION_TYPES] = {
//                                                         ADPCM PCM MP3
{                                                              0, sizeof(sample_header),  0 }, // PC
{                                        sizeof( sample_header ),  0,  0 }, // PS2
{ sizeof(sample_header) + sizeof( gcn_adpcm_compression_header ),  0,  0 }, // GameCube
{                                                              0,  0,  0 }};// Xbox

//------------------------------------------------------------------------------

void DisplayHelp( void )
{
    x_printf( "  usage:\n" );
    x_printf( "         SoundPackager [-opt [param]] <script>\n" );
    x_printf( "\n" );
    x_printf( "options:\n" );
    x_printf( "         -pc                - Compile for PC\n" );
    x_printf( "         -ps2               - Compile for PS2\n" );
    x_printf( "         -gcn               - Compile for GCN\n" );
    x_printf( "         -xbox              - Compile for XBox\n" );
    x_printf( "\n" );
    x_printf( "         -v                 - Verbose\n" );
    x_printf( "         -d                 - Debug\n" );
    x_printf( "         -clean             - Recompresses ALL samples\n" );
    x_printf( "\n" );
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
        x_printf( "Error: Source file '%s' not found!\n", SourceFilename  );
        x_DebugMsg( "Error: Source file '%s' not found!\n", SourceFilename  );
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

s32 FindFileByLabel( xstring Label )
{
    s32     i;

    for( i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        if( s_Package.m_Files[i].Identifier == Label )
            return i;
    }

    return -1;
}

//------------------------------------------------------------------------------

s32 FindDescriptorByLabel( xstring Label )
{
    s32     i;

    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        if( s_Package.m_Descriptors[i].Identifier == Label )
            return i;
    }

    return -1;
}

//------------------------------------------------------------------------------

xbool LabelIsUnique( char* Label )
{
    s32     i;
    xbool   Unique = TRUE;
    xstring Temp;
                
    // Make sure label is unique.
    Temp = Label;

    for( i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        if( s_Package.m_Files[i].Identifier == Temp )
            Unique = FALSE;
    }

    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        if( s_Package.m_Descriptors[i].Identifier == Temp )
            Unique = FALSE;
    }

    if( Unique )
    {
        return TRUE;
    }
    else
    {
        x_printf( "Error: Label '%s' already used! All labels must be unique!\n", Label );
        x_DebugMsg( "Error: Label '%s' already used! All labels must be unique!\n", Label );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }
}

//------------------------------------------------------------------------------

xbool FindPackageKeyWord( token_stream* Tokenizer )
{
    // Loop over tokens
    while( (s_Package.m_ParseSection == NONE) && (Tokenizer->Type() != token_stream::TOKEN_EOF) )
    {
        if( Tokenizer->Type() == token_stream::TOKEN_SYMBOL )
        {
            x_printf( "%s\n", Tokenizer->String() );
            if( !strcmp( PACKAGE_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = PACKAGE;
            }
        }

        Tokenizer->Read();
    }

    if( s_Package.m_ParseSection == NONE )
    {
        x_printf( "Error: Could not find \"%s\" keyword!\n", PACKAGE_SECTION );
        x_DebugMsg("Error: Could not find \"%s\" keyword!\n", PACKAGE_SECTION );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseParameters( token_stream* Tokenizer, compressed_parameters* Params )
{
    Tokenizer->Read();
    if( !strcmp( Tokenizer->String(), "[" ) )
    {
        if( s_Debug )
        {
            x_printf( "[ " );
            x_DebugMsg( "[ " );
        }

        Tokenizer->Read();
        while( Tokenizer->Type() != token_stream::TOKEN_EOF )
        {
            if( Tokenizer->Type() == token_stream::TOKEN_DELIMITER )
            {
                if( strcmp( Tokenizer->String(), "]" ) )
                {
                    x_printf( "Error: Parsing parameters\n" );
                    x_DebugMsg( "Error: Parsing parameters\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    if( s_Debug )
                    {
                        x_printf( "]" );
                        x_DebugMsg( "]" );
                    }

                    // Done!
                    Tokenizer->Read();
                    return TRUE;
                }
            }
            else if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
            {
                x_printf( "Error: Parsing parameters!\n" );
                x_DebugMsg( "Error: Parsing parameters!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }
            else
            {
                s32 i;

                // its gotta be a parameter label
                for( i=0 ; i<NUM_PARAMETERS; i++ )
                {
                    // find a parameter name?
                    if( !strcmp( Tokenizer->String(), s_ParameterTypes[i] ) )
                    {
                        Tokenizer->Read();
                        if( strcmp( Tokenizer->String(), "=" ) )
                        {
                            x_printf( "Error: '=' must follow parameter name, found '%s' instead.\n", Tokenizer->String() );
                            x_DebugMsg( "Error: '=' must follow parameter name, found '%s' instead.\n", Tokenizer->String() );
                            s_Package.m_ParseError = TRUE;
                            return FALSE;
                        }
                        else
                        {
                            Tokenizer->Read();
                            if( Tokenizer->Type() != token_stream::TOKEN_NUMBER )
                            {
                                x_printf( "Error: Parameter values must be numeric, found '%s' instead.\n", Tokenizer->String() );
                                x_DebugMsg( "Error: Parameter values must be numeric, found '%s' instead.\n", Tokenizer->String() );
                                s_Package.m_ParseError = TRUE;
                                return FALSE;
                            }
                            else
                            {
                                f32   f = Tokenizer->Float();
                                s32   j = Tokenizer->Int();
                                xbool WantsFloat = TRUE;
                                xbool IsFloat = Tokenizer->IsFloat();

                                // Set the bit for this parameter.
                                Params->Bits |= 1<<i;

                                switch( i )
                                {
                                    case PITCH:
                                        if( f < 0.015625f || f > 4.0f )
                                        {
                                            x_printf( "Error: Valid range for pitch is 0.015625 to 4.0\n" );
                                            x_DebugMsg( "Error: Valid range for pitch is 0.015625 to 4.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }

                                        Params->Pitch = FLOAT4_TO_U16BIT( f );
                                        break;

                                    case PITCH_VARIANCE:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "Error: Valid range for pitch variance is 0.0 to 1.0\n" );
                                            x_DebugMsg( "Error: Valid range for pitch variance is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        
                                        Params->PitchVariance = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "Error: Valid range for volume is 0.0 to 1.0\n" );
                                            x_DebugMsg( "Error: Valid range for volume is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Volume = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME_VARIANCE:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "Error: Valid range for volume variance is 0.0 to 1.0\n" );
                                            x_DebugMsg( "Error: Valid range for volume variance is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->VolumeVariance = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case PAN:
                                        if( f < -1.0f || f > 1.0f )
                                        {
                                            x_printf( "Error: Valid range for pan is -1.0 to 1.0\n" );
                                            x_DebugMsg( "Error: Valid range for pan is -1.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Pan = FLOAT1_TO_S8BIT( f );
                                        break;

                                    case PRIORITY:
                                        WantsFloat = FALSE;
                                        if( j < 0 || j > 255 )
                                        {
                                            x_printf( "Error: Valid range for priority is 0 to 255\n" );
                                            x_DebugMsg( "Error: Valid range for priority is 0 to 255\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Priority = j;
                                        break;

                                    case EFFECT_SEND:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "Error: Valid range for effect send is 0.0 to 1.0\n" );
                                            x_DebugMsg( "Error: Valid range for effect send is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->EffectSend = FLOAT1_TO_U8BIT( f );
                                       break;

                                    case NEAR_FALLOFF:
                                        if( f < 0.0f || f > 10.0f )
                                        {
                                            x_printf( "Error: Valid range for effect send is 0.0 to 10.0\n" );
                                            x_DebugMsg( "Error: Valid range for effect send is 0.0 to 10.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->NearFalloff = FLOAT1_TO_U8BIT( f );
                                        break;

                                    case FAR_FALLOFF:
                                        if( f < 0.0f || f > 10.0f )
                                        {
                                            x_printf( "Error: Valid range for effect send is 0.0 to 10.0\n" );
                                            x_DebugMsg( "Error: Valid range for effect send is 0.0 to 10.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->FarFalloff = FLOAT1_TO_U8BIT( f );
                                        break;
#if 0
                                    case PROPAGATE_3D:
                                        if( j < 0 || j > 1 )
                                        {
                                            x_printf( "valid range for propagate 3d is 0 to 1\n" );
                                            x_DebugMsg( "valid range for propagate 3d is 0 to 1\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        break;
#endif
                                }
                                
                                if( s_Debug )
                                {
                                    x_printf( "%s=%s ", s_ParameterTypes[i], Tokenizer->String() );
                                    x_DebugMsg( "%s=%s ", s_ParameterTypes[i], Tokenizer->String() );
                                }
                            }
                        }

                        break;
                    }
                }

                if( i >= NUM_PARAMETERS )
                {
                    x_printf( "Error: '%s' is not a valid parameter!\n", Tokenizer->String() );
                    x_DebugMsg( "Error: '%s' is not a valid parameter!\n", Tokenizer->String() );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
            }
        
            Tokenizer->Read();
        }

        x_printf( "Error: Unexpected end of file parsing parameters!\n" );
        x_DebugMsg( "Error: Unexpected end of file parsing parameters!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParsePackage( token_stream* Tokenizer )
{
    if( s_Verbose )
    {
        x_printf( "Parsing package info...\n" );
        x_DebugMsg( "Parsing package info...\n" );
    }

    // Loop over tokens watching for the files keyword.
    while( (s_Package.m_ParseSection == PACKAGE) && (Tokenizer->Type() != token_stream::TOKEN_EOF) )
    {
        if( Tokenizer->Type() == token_stream::TOKEN_SYMBOL )
        {   
            // First token is the package name.
            s_Package.m_Identifier[0] = Tokenizer->String();
            
            // This is strictly temp so delete it.
            s_Package.m_Identifier[0].Clear();

            //xstring Identifier = Tokenizer->String();
           
            if( s_Debug )
            {
                x_printf( "Package: %s", Tokenizer->String() );
                x_DebugMsg( "Package: %s", Tokenizer->String() );
            }

            // Now parse any parameters
            ParseParameters( Tokenizer, &s_Package.m_Params );
            
            if( s_Debug )
            {
                x_printf( "\n" );
                x_DebugMsg( "\n" );
            }

            if( !strcmp( FILES_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = FILES;
            }
            else
            {
                x_printf( "Error: '%s\' keyword expected!\n" );
                x_DebugMsg( "Error: '%s\' keyword expected!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }
        }

        Tokenizer->Read();
    }
    
    if( s_Package.m_ParseSection == PACKAGE )
    {
        x_printf( "Error: Could not find \"%s\" keyword!\n", FILES_SECTION );
        x_DebugMsg("Error: Could not find \"%s\" keyword!\n", FILES_SECTION );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseFiles( token_stream* Tokenizer )
{
    s32 i;

    if( s_Verbose )
    {
        x_printf( "Parsing files...\n" );
        x_DebugMsg( "Parsing files...\n" );
    }

    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        s_Package.m_TemperatureCount[i] = 0;
    }

    // Loop over tokens watching for the descriptor keyword.
    while( (s_Package.m_ParseSection == FILES) && (Tokenizer->Type() != token_stream::TOKEN_EOF) )
    {
        if( Tokenizer->Type() == token_stream::TOKEN_SYMBOL )
        {   
            if( !strcmp( DESCRIPTOR_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = DESCRIPTORS;
            }
            else
            {
                // Make sure label is unique.
                if( !LabelIsUnique( Tokenizer->String() ) )
                    return FALSE;

                // ok, this must be a label...
                file_info& Info = s_Package.m_Files.Append();
                Info.Identifier = Tokenizer->String();

                // read the temperature
                Tokenizer->Read();
                if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
                {
                    x_printf( "Error: Sample temperature expected!\n" );
                    x_DebugMsg( "Error: Sample temperature expected!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                    {
                        if( !stricmp( Tokenizer->String(), s_TemperatureTypes[i] ) )
                            break;
                    }
                    
                    if( s_HotOnly )
                    {
                        i = 0;
                    }

                    if( i >= NUM_TEMPERATURES )
                    {
                        x_printf( "Error: %s is not a valid sample temperature!\n", Tokenizer->String() );
                        x_DebugMsg( "Error: %s is not a valid sample temperature!\n", Tokenizer->String() );
                        s_Package.m_ParseError = TRUE;
                        return FALSE;
                    }
                    else
                    {
                        Info.Index       = s_Package.m_TemperatureCount[i];
                        Info.Temperature = i;
                        s_Package.m_TemperatureCount[i]++;
                    }
                }

                // read the filename
                Tokenizer->Read();
                if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
                {
                    x_printf( "Error: Filename expected!\n" );
                    x_DebugMsg( "Error: Filename expected!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    char pBuf[256];

                    x_strcpy( pBuf, Tokenizer->String() );
                    x_printf( pBuf );
                    x_DebugMsg( pBuf );
                    x_printf( "\n" );
                    x_DebugMsg( "\n" );
                    
                    if( !strcmp( Tokenizer->String(), "NoLipSync" ) )
                    {
                        Info.UsesLipSync = FALSE;
                    }
                    else if( !strcmp( Tokenizer->String(), "LipSync" ) )
                    {
                        Info.UsesLipSync = TRUE;
                    }
                    else
                    {
                        x_printf( "Lipsync flag expected!\n" );
                        x_DebugMsg( "Lipsync flag expected!\n" );
                        s_Package.m_ParseError = TRUE;
                        return FALSE;
                    }
                }

                // read the filename
                Tokenizer->Read();
                if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
                {
                    x_printf( "Filename expected!\n" );
                    x_DebugMsg( "Filename expected!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    // Set the filename
                    Info.Filename = Tokenizer->String();
                }
            }
        }

        Tokenizer->Read();
    }

    if( s_Package.m_ParseSection == FILES )
    {
        x_printf( "Error: Could not find \"%s\" keyword!\n", DESCRIPTOR_SECTION );
        x_DebugMsg("Error: Could not find \"%s\" keyword!\n", DESCRIPTOR_SECTION );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseSimple( token_stream* Tokenizer, descriptor_info& Descriptor )
{
    Tokenizer->Read();
    if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
    {
        x_printf( "Error: Simple descriptor label expected!\n" );
        x_DebugMsg( "Error: Simple descriptor label expected!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }
    else
    {
        if( s_Debug )
        {
            x_printf( " %s", Tokenizer->String() );
            x_DebugMsg( " %s", Tokenizer->String() );
        }
        
        element_info& Element = Descriptor.Elements.Append();
        Element.Identifier = Tokenizer->String();

        // Copy the default parameters.
        x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );
        Element.Params.Bits = 0;

        // parse parameters
        return ParseParameters( Tokenizer, &Element.Params );
    }
}

//------------------------------------------------------------------------------

xbool ParseComplex( token_stream* Tokenizer, descriptor_info& Descriptor )
{
    // Make sure we have '{'
    Tokenizer->Read();
    if( strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "Error: '{' expected!\n" );
        x_DebugMsg( "Error: '{' expected!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    if( s_Debug )
    {
        x_printf( "\n{" );
        x_DebugMsg( "\n{" );
    }

    // Parse thru the complex sound looking for the '}'
    Tokenizer->Read();
    while( strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Error: Complex descriptor label expected!\n" );
            x_DebugMsg( "Error: Complex descriptor label expected!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            if( s_Debug )
            {
                x_printf( "\n    %s", Tokenizer->String() );
                x_DebugMsg( "\n     %s", Tokenizer->String() );
            }

            element_info& Element = Descriptor.Elements.Append();
            Element.Identifier    = Tokenizer->String();

            // Copy the default parameters.
            x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );
            Element.Params.Bits = 0;

            // Parse parameters
            if( !ParseParameters( Tokenizer, &Element.Params ) )
                return FALSE;

            // Read the time delay
            if( Tokenizer->Type() != token_stream::TOKEN_NUMBER || !Tokenizer->IsFloat() )
            {
                x_printf( "Error: Floating point time delay (seconds) expected!\n" );
                x_DebugMsg( "Error: Floating point time delay (seconds) expected!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            f32 f = Tokenizer->Float();

            // Error check
            if( (f < 0.0f) || (f > (65535.0f / 100.0f)) )
            {
                x_printf( "Error: Time delay out of range: 0.0 to 655.35 seconds!\n" );
                x_DebugMsg( "Error: Time delay out of range: 0.0 to 655.35 seconds!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            if( s_Debug )
            {
                x_printf( " delay=%s", Tokenizer->String() );
                x_DebugMsg( " delay=%s", Tokenizer->String() );
            }

            // Save the start delay.
            Element.StartDelay = f;
        }

        // Next!
        Tokenizer->Read();
    }

    if( s_Debug )
    {
        x_printf( "\n}\n" );
        x_DebugMsg( "\n}\n" );
    }

    Tokenizer->Read();
    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseRandomList( token_stream* Tokenizer, descriptor_info& Descriptor )
{
    // Make sure we have '{'
    Tokenizer->Read();
    if( strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "Error: '{' expected!\n" );
        x_DebugMsg( "Error: '{' expected!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    if( s_Debug )
    {
        x_printf( "\n{" );
        x_DebugMsg( "\n{" );
    }

    // Parse thru the random list looking for the '}'
    Tokenizer->Read();
    while( strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Error: Random list element descriptor label expected!\n" );
            x_DebugMsg( "Error: Random list element descriptor label expected!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            if( s_Debug )
            {
                x_printf( "\n    %s", Tokenizer->String() );
                x_DebugMsg( "\n     %s", Tokenizer->String() );
            }

            if( Descriptor.Elements.GetCount() >= 63 )
            {
                x_printf( "Error: Random lists can only have 63 elements!\n" );
                x_DebugMsg( "Error: Random lists can only have 63 elements!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            element_info& Element = Descriptor.Elements.Append();
            Element.Identifier    = Tokenizer->String();

            // Copy the default parameters.
            x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );
            Element.Params.Bits = 0;

            // Parse parameters
            if( !ParseParameters( Tokenizer, &Element.Params ) )
                return FALSE;
        }
    }

    if( s_Debug )
    {
        x_printf( "\n}\n" );
        x_DebugMsg( "\n}\n" );
    }

    Tokenizer->Read();
    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseWeightedList( token_stream* Tokenizer, descriptor_info& Descriptor )
{
    // Make sure we have '{'
    Tokenizer->Read();
    if( strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "Error: '{' expected!\n" );
        x_DebugMsg( "Error: '{' expected!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    if( s_Debug )
    {
        x_printf( "\n{" );
        x_DebugMsg( "\n{" );
    }

    // Parse thru the weighted list looking for the '}'
    Tokenizer->Read();
    while( strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Error: Weighted list element descriptor label expected!\n" );
            x_DebugMsg( "Error: Weighted list element descriptor label expected!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            if( s_Debug )
            {
                x_printf( "\n    %s", Tokenizer->String() );
                x_DebugMsg( "\n     %s", Tokenizer->String() );
            }

            element_info& Element = Descriptor.Elements.Append();
            Element.Identifier    = Tokenizer->String();

            // Copy the default parameters.
            x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );
            Element.Params.Bits = 0;

            // Parse parameters.
            if( !ParseParameters( Tokenizer, &Element.Params ) )
                return FALSE;

            // Read the weight.
            if( Tokenizer->Type() != token_stream::TOKEN_NUMBER || Tokenizer->IsFloat() )
            {
                x_printf( "Error: Integer weight expected!\n" );
                x_DebugMsg( "Error: Integer weight expected!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            u32 i = Tokenizer->Int();

            // Error check.
            if( (i < 0) || (i > 65535) )
            {
                x_printf( "Error: Weight out of range: 0 to 65535!\n" );
                x_DebugMsg( "Error: Weight out of range: 0 to 65535!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }
            
            if( s_Debug )
            {
                x_printf( " weight=%s", Tokenizer->String() );
                x_DebugMsg( " weight=%s", Tokenizer->String() );
            }

            // Save the weight.
            Element.Weight = i;

            // Next!
            Tokenizer->Read();
        }
    }

    if( s_Debug )
    {
        x_printf( "\n}\n" );
        x_DebugMsg( "\n}\n" );
    }

    Tokenizer->Read();
    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseDescriptors( token_stream* Tokenizer )
{
    if( s_Verbose )
    {
        x_printf( "Parsing descriptors...\n" );
        x_DebugMsg( "Parsing descriptors...\n" );
    }

    // Loop over tokens...
    while( Tokenizer->Type() != token_stream::TOKEN_EOF )
    {
        // First thing has to be a label...
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            // First token is the package name.
//            while( Tokenizer->Type() != token_stream::TOKEN_EOF )
//            {
//                Tokenizer->Read();
//                xstring Platform = Tokenizer->String();                
/*

//                if( !strcmp( "PC", Tokenizer->String() ) )
//                {
//                    s_Targets.Append() = EXPORT_PC;
//                    Tokenizer->Read();
                    
                    // Set the temperature for all the pc packages to be HOT.
//                    for( s32 i = 0; i < s_Package.m_Files.GetCount(); i++ )
//                        s_Package.m_Files[i].Temperature = 0;

                }
                else if( !strcmp( "PS2", Tokenizer->String() ) )
                {
                    s_Targets.Append() = EXPORT_PS2;
                    Tokenizer->Read();
                    s_Package.m_Identifier = Tokenizer->String();
                    s_Package.m_Identifier = s_Package.m_Identifier.Left( s_Package.m_Identifier.Find( '.' ) );

                }
                else if( !strcmp( "GNC", Tokenizer->String() ) )
                {
                    s_Targets.Append() = EXPORT_GCN;
                    Tokenizer->Read();
                    s_Package.m_Identifier = Tokenizer->String();
                    s_Package.m_Identifier = s_Package.m_Identifier.Left( s_Package.m_Identifier.Find( '.' ) );

                }
                else if( !strcmp( "XBOX", Tokenizer->String() ) )
                {
                    s_Targets.Append() = EXPORT_XBOX;
                    Tokenizer->Read();
                    s_Package.m_Identifier = Tokenizer->String();
                    s_Package.m_Identifier = s_Package.m_Identifier.Left( s_Package.m_Identifier.Find( '.' ) );

                }  
*/                          
//            }
                        
            return FALSE;
//            x_printf( "Error parsing descriptors (label expected)!\n" );
//            x_DebugMsg( "Error parsing descriptors (label expected)!\n" );
//            s_Package.m_ParseError = TRUE;
//            return FALSE;
        }
        else
        {
            xstring FullFileName = Tokenizer->String();

            // We want to make sure that we have a valid name.
            if( FullFileName.Find( ".audiopkg" ) != -1 )
            {
/*                if( FullFileName.Find( "PC" ) != -1 )
                {
                    s_Package.m_Identifier[EXPORT_PC] = FullFileName.Left( FullFileName.Find( '.' ) );
                }
                else if( FullFileName.Find( "PS2" ) != -1 )
                {
                    s_Package.m_Identifier[EXPORT_PS2] = FullFileName.Left( FullFileName.Find( '.' ) );
                }
                else if( FullFileName.Find( "GCN" ) != -1 )
                {   
                    s_Package.m_Identifier[EXPORT_GCN] = FullFileName.Left( FullFileName.Find( '.' ) );
                }   
                else if( FullFileName.Find( "XBOX" ) != -1 )
                {
                    s_Package.m_Identifier[EXPORT_XBOX] = FullFileName.Left( FullFileName.Find( '.' ) );
                }
                else
                {
                    x_printf( "Error: Invalid output name [%s].!\n", FullFileName );
                    x_DebugMsg( "Error: Invalid output name [%s].!\n", FullFileName );
                }
*/
                s_Package.m_Identifier[s_Targets[0]] = FullFileName.Left( FullFileName.Find( '.' ) );
                
                Tokenizer->Read();
                continue;
            }

            // Make sure label is unique.
            if( !LabelIsUnique( Tokenizer->String() ) )
                return FALSE;

            if( s_Package.m_Descriptors.GetCount() >= 2047 )
            {
                x_printf( "Error: Limit of 2047 descriptors maximum exceeded!\n" );
                x_DebugMsg( "Error: Limit of 2047 descriptors maximum exceeded!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            // Ok this is a label...
            descriptor_info& Descriptor = s_Package.m_Descriptors.Append();
            Descriptor.Index            = s_Package.m_Descriptors.GetCount()-1;
            Descriptor.Identifier       = Tokenizer->String();

            if( s_Debug )
            {
                x_printf( "\n%s", Tokenizer->String() );
                x_DebugMsg( "\n%s", Tokenizer->String() );
            }

            // Copy the default parameters.
            x_memcpy( &Descriptor.Params, &s_Package.m_Params, sizeof( compressed_parameters ) );
            Descriptor.Params.Bits = 0;

            // parse parameters
            if( !ParseParameters( Tokenizer, &Descriptor.Params ) )
                return FALSE;

            // Now parse the type.
            if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
            {
                x_printf( "Error: parsing descriptors (type expected)!\n" );
                x_DebugMsg( "Error: parsing descriptors (type expected)!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }
            else
            {
                s32 i;
                for( i=0 ; i<NUM_DESCRIPTOR_TYPES ; i++ )
                {
                    if( !strcmp( Tokenizer->String(), s_DescriptorTypes[i] ) )
                        break;
                }

                if( i >= NUM_DESCRIPTOR_TYPES )
                {
                    x_printf( "Error: %s is not a valid descriptor type!\n", Tokenizer->String() );
                    x_DebugMsg( "Error: %s is not a valid descriptor type!\n", Tokenizer->String() );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    if( s_Debug )
                    {
                        x_printf( " %s", Tokenizer->String() );
                        x_DebugMsg( " %s", Tokenizer->String() );
                    }

                    Descriptor.Type  = i;

                    switch( i )
                    {
                        case SIMPLE:
                            if( !ParseSimple( Tokenizer, Descriptor ) )
                                return FALSE;
                            break;

                        case COMPLEX:
                            if( !ParseComplex( Tokenizer, Descriptor ) )
                                return FALSE;
                            break;

                        case RANDOM_LIST:
                            if( !ParseRandomList( Tokenizer, Descriptor ) )
                                return FALSE;
                            break;

                        case WEIGHTED_LIST:
                            if( !ParseWeightedList( Tokenizer, Descriptor ) )
                                return FALSE;
                            break;
                    }
                }
            }
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ResolveElement( element_info& Element )
{
    s32 Index;

    Index = FindFileByLabel( Element.Identifier );

    if( Index >= 0 )
    {
        Element.Index = s_Package.m_Files[ Index ].Index;
        Element.Type  = s_Package.m_Files[ Index ].Temperature;
        return TRUE;
    }
    else
    {
        Index = FindDescriptorByLabel( Element.Identifier );

        if( Index >= 0 )
        {
            Element.Index = Index;
            Element.Type  = DESCRIPTOR_INDEX;
            return TRUE;
        }
        else
        {
            x_printf( "Error: Label '%s' is referenced, but not defined!\n", Element.Identifier );
            x_DebugMsg( "Error: Label '%s' is referenced, but not defined!\n", Element.Identifier );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
    }
}

//------------------------------------------------------------------------------

xbool ResolveReferences( void )
{
    s32 i;

    // Resolve all the labels...
    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        s32 j;
        s32 Limit = s_Package.m_Descriptors[i].Elements.GetCount();

        for( j=0 ; j<Limit ; j++ )
        {
            if( !ResolveElement( s_Package.m_Descriptors[i].Elements[j] ) )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
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

//------------------------------------------------------------------------------

void SkipVersionIdentifier( X_FILE* f )
{
    char VersionString[VERSION_ID_SIZE];

    // Read the version id
    x_fread( VersionString, VERSION_ID_SIZE, 1, f );
}

//------------------------------------------------------------------------------

xbool ReadCompressedAudioFileHeader( const char* Filename, xbool ReverseEndian,
                                     s32* NumChannels, s32* CompressionMethod, 
                                     s32* CompressedSize, s32* LipSyncSize )
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

        // Close the file.
        x_fclose( f );

        // its all good!
        return TRUE;
    }
    else
    {
        // Could not open file!
        x_printf( "Error: File '%s' not found!\n", Filename );
        x_DebugMsg( "Error: File '%s' not found!\n", Filename );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }
}

//------------------------------------------------------------------------------

u32 GetLipSyncSize( s32 nSamples, s32 SampleRate )
{
    f32 Rate     = 30.0f;                           // Sampling rate for lip sync data
    f32 Seconds  = (f32)nSamples / (f32)SampleRate; // Length of sample in seconds.
    u32 nLipSync = (u32)((Seconds * Rate) + 0.9999999999999f);

    nLipSync++; // Make space for rate in data stream

    return nLipSync;
}

//------------------------------------------------------------------------------

void WriteLipSyncData( aiff_file* Aiff, X_FILE* out )
{
    s32  i;
    s32  SampleRate    = Aiff->GetSampleRate();
    s32  nChannels     = Aiff->GetNumChannels();
    s32  nSamples      = Aiff->GetNumSamples();
    s16* pSampleBuffer = (s16*)x_malloc( sizeof(s16) * nSamples );

    // Currently only support mono and stereo.
    ASSERT( (nChannels == 1) || (nChannels == 2) );

    // Compress each channel...
    for( i=0 ; i<nChannels ; i++ )
    {
        f32 Rate     = 30.0f;                           // Sampling rate for lip sync data
        f32 Seconds  = (f32)nSamples / (f32)SampleRate; // Length of sample in seconds.
        s32 nLipSync = (u32)((Seconds * Rate) + 0.9999999999999f);
        u8  cRate    = (u8)Rate;

        // Write out sample rate
        x_fwrite( &cRate, 1, 1, out );

        // Read the the uncompressed waveform data.
        Aiff->GetChannelData( pSampleBuffer, i );

        for( s32 j=0 ; j<nLipSync ; j++ )
        {
            u8 Data;

            // Last sample?
            if( j==(nLipSync-1) )
            {
                Data = 0;
            }
            // Find largest sample...
            else
            {
                s32 StartSample = (s32)((f32)(j+0) * (f32)nSamples / (f32)nLipSync);
                s32 EndSample   = (s32)((f32)(j+1) * (f32)nSamples / (f32)nLipSync);
                u16 Big         = 0;

                for( s32 k=StartSample ; k<EndSample ; k++ )
                {
                    u16 Test = x_abs( pSampleBuffer[k] );
                    if( Test > Big )
                        Big = Test;
                }

                Data = (u8)(Big >> 8);
            }

            // Write out the data.
            x_fwrite( &Data, 1, 1, out );
        }
    }

    // Free the buffer.
    x_free( pSampleBuffer );
}

//------------------------------------------------------------------------------

u32 CompressAudioFilePC_PCM( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    aiff_file   Aiff;
    s32         TotalCompressedSize = 0;

    if( Aiff.Open( in ) )
    {
        s32  SampleRate         = Aiff.GetSampleRate();
        s32  nChannels          = Aiff.GetNumChannels();
        s32  nSamples           = Aiff.GetNumSamples();
        s32  LoopStart          = Aiff.GetLoopStart();
        s32  LoopEnd            = Aiff.GetLoopEnd();
        s16* pSampleBuffer      = (s16*)x_malloc( sizeof(s16) * nSamples );
        s32  HeaderSize         = 4 * sizeof(s32); // + sizeof( pc_pcminfo );
        s32  CompressionMethod;
        s32  i;
        s32  CompressedSize;

        xarray<void*>         pCompressedBuffer;

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

        // Write out header size in bytes.
        s32 pc_HeaderSize = LITTLE_ENDIAN( HeaderSize );
        x_fwrite( &pc_HeaderSize, sizeof(s32), 1, out );

        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {
//            pc_pcminfo PCM;

            // Allocate space for the compressed data.
            pCompressedBuffer.Append() = x_malloc( CompressedSize );

            // Read the the uncompressed waveform data.
            Aiff.GetChannelData( pSampleBuffer, i );

            // Nuke the header.
//            x_memset( &PCM, 0, sizeof(pc_pcminfo) );
//            x_memset( pCompressedBuffer[i], 0, CompressedSize );

            // Copy the sample, no compression necessary
            x_memcpy( pCompressedBuffer[i], pSampleBuffer, nSamples * sizeof(s16) );

            // Write out the number of samples, sample rate and loop points
            s32 pc_nSamples   = LITTLE_ENDIAN( nSamples );
            s32 pc_SampleRate = LITTLE_ENDIAN( SampleRate );
            s32 pc_LoopStart  = LITTLE_ENDIAN( LoopStart );
            s32 pc_LoopEnd    = LITTLE_ENDIAN( LoopEnd );
            x_fwrite( &pc_nSamples,   sizeof(s32), 1, out );
            x_fwrite( &pc_SampleRate, sizeof(s32), 1, out );
            x_fwrite( &pc_LoopStart,  sizeof(s32), 1, out );
            x_fwrite( &pc_LoopEnd,    sizeof(s32), 1, out );
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
        WriteLipSyncData( &Aiff, out );
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------

u32 CompressAudioFileGCN_ADPCM( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    aiff_file   Aiff;
    s32         TotalCompressedSize = 0;

    if( Aiff.Open( in ) )
    {
        s32  SampleRate         = Aiff.GetSampleRate();
        s32  nChannels          = Aiff.GetNumChannels();
        s32  nSamples           = Aiff.GetNumSamples();
        // TODO: Put in looping stuff
        s32  LoopStart          = Aiff.GetLoopStart();
        s32  LoopEnd            = Aiff.GetLoopEnd();
        s16* pSampleBuffer      = (s16*)x_malloc( sizeof(s16) * nSamples );
        s32  HeaderSize         = 4 * sizeof(s32) + sizeof( gcn_adpcminfo );
        s32  CompressionMethod;
        s32  i;
        s32  CompressedSize;

        xarray<void*>         pCompressedBuffer;

        // Currently only support mono and stereo.
        ASSERT( (nChannels == 1) || (nChannels == 2) );

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
            CompressionMethod = GCN_ADPCM_NON_INTERLEAVED_STEREO;
        }
        else
        {
            CompressionMethod = GCN_ADPCM_MONO;
        }

        // Write out compression method.
        s32 gcn_CompressionMethod = reverse_endian_32( CompressionMethod );
        x_fwrite( &gcn_CompressionMethod, sizeof(s32), 1, out );

        // Write out the compressed size.
        CompressedSize = (dsptool_getBytesForAdpcmBuffer( nSamples )+31)&~31;
        s32 gcn_CompressedSize = CompressedSize;
        gcn_CompressedSize = reverse_endian_32( gcn_CompressedSize );
        x_fwrite( &gcn_CompressedSize, sizeof(s32), 1, out );

        // Write out the lip sync size in bytes.
        *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
        s32 gcn_LipSyncSize = *LipSyncSize;
        gcn_LipSyncSize = reverse_endian_32( gcn_LipSyncSize );
        x_fwrite( &gcn_LipSyncSize, sizeof(s32), 1, out );

        // Write out header size in bytes.
        s32 gcn_HeaderSize = reverse_endian_32( HeaderSize );
        x_fwrite( &gcn_HeaderSize, sizeof(s32), 1, out );

        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {
            gcn_adpcminfo ADPCM;

            // Allocate space for the compressed data.
            pCompressedBuffer.Append() = x_malloc( CompressedSize );

            // Read the the uncompressed waveform data.
            Aiff.GetChannelData( pSampleBuffer, i );

            // Nuke the ADPCM header.
            x_memset( &ADPCM, 0, sizeof(gcn_adpcminfo) );
            x_memset( pCompressedBuffer[i], 0, CompressedSize );

            // ADPCM Compress the sample.
            dsptool_encode( (s16*)pSampleBuffer, (u8*)pCompressedBuffer[i], &ADPCM, nSamples );

            if( Aiff.IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;

                dsptool_getLoopContext( (u8*)pCompressedBuffer[i], &ADPCM, LoopStart );
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            // Write out the number of samples, sample rate and loop points
            s32 gcn_nSamples   = reverse_endian_32( nSamples );
            s32 gcn_SampleRate = reverse_endian_32( SampleRate );
            s32 gcn_LoopStart  = reverse_endian_32( LoopStart );
            s32 gcn_LoopEnd    = reverse_endian_32( LoopEnd );
            x_fwrite( &gcn_nSamples,   sizeof(s32), 1, out );
            x_fwrite( &gcn_SampleRate, sizeof(s32), 1, out );
            x_fwrite( &gcn_LoopStart,  sizeof(s32), 1, out );
            x_fwrite( &gcn_LoopEnd,    sizeof(s32), 1, out );

            // Byteswap ADPCM header
            for( s32 j=0 ; j<16 ; j++ )
            {
                ADPCM.coef[j] = reverse_endian_16( ADPCM.coef[j] );
            }

            ADPCM.gain            = reverse_endian_16( ADPCM.gain );
            ADPCM.pred_scale      = reverse_endian_16( ADPCM.pred_scale );
            ADPCM.yn1             = reverse_endian_16( ADPCM.yn1 );
            ADPCM.yn2             = reverse_endian_16( ADPCM.yn2 );
            ADPCM.loop_pred_scale = reverse_endian_16( ADPCM.loop_pred_scale );
            ADPCM.loop_yn1        = reverse_endian_16( ADPCM.loop_yn1 );
            ADPCM.loop_yn2        = reverse_endian_16( ADPCM.loop_yn2 );

            // Write out the header
            x_fwrite( &ADPCM, sizeof(ADPCM), 1, out );
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
        WriteLipSyncData( &Aiff, out );
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------

u32 CompressAudioFilePS2_ADPCM( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    aiff_file   Aiff;
    s32         TotalCompressedSize = 0;

    if( Aiff.Open( in ) )
    {
        s32  SampleRate         = Aiff.GetSampleRate();
        s32  nChannels          = Aiff.GetNumChannels();
        s32  nSamples           = Aiff.GetNumSamples();
        // TODO: Put in looping stuff
        s32  LoopStart;
        s32  LoopEnd;
        s16* pSampleBuffer      = (s16*)x_malloc( sizeof(s16) * nSamples );
        s32  HeaderSize         = 4 * sizeof(s32);
        s32  CompressionMethod;
        s32  i;
        s32  CompressedSize;

        xarray<void*>         pCompressedBuffer;

        // Currently only support mono and stereo.
        ASSERT( (nChannels == 1) || (nChannels == 2) );

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


        // Write out header size in bytes.
        x_fwrite( &HeaderSize, sizeof(s32), 1, out );

		s16* pCompBuffer;
		s16* pSampBuffer;
		s32 Remain;
		s32 Length;
        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {

			EncVagInit(ENC_VAG_MODE_NORMAL);
            // Allocate space for the compressed data.
            pCompressedBuffer.Append() = x_malloc( CompressedSize );

            // Read the the uncompressed waveform data.
            Aiff.GetChannelData( pSampleBuffer, i );
			pCompBuffer = (s16*)pCompressedBuffer[i];
			pSampBuffer = (s16*)pSampleBuffer;
			Remain      = nSamples;
			Length		= 0;

            x_memset( pCompressedBuffer[i], 0, CompressedSize );
			// The first 16 bytes on a sample have to be zero to initialize the BRR
			// hardware
			pCompBuffer+=16/sizeof(s16);
			Length+=16;

			while (Remain>0)
			{
				Length += 16;
				// We need to make sure at the end of every 2K block, we set the
				// LOOP_END flag so it'll properly jump to the next block.
				if ( Length & 2047)
				{
					EncVag(pSampBuffer,pCompBuffer,ENC_VAG_LOOP_BODY);
				}
				else
				{
					EncVag(pSampBuffer,pCompBuffer,ENC_VAG_LOOP_END);
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
				if ( Length & 2047)
				{
					EncVag(TempBuffer,pCompBuffer,ENC_VAG_LOOP_BODY);
				}
				else
				{
					EncVag(TempBuffer,pCompBuffer,ENC_VAG_LOOP_END);
				}
				pCompBuffer += 16/sizeof(s16);
			}

            // TODO: Put in looping stuff... 
            LoopStart = 0;
            LoopEnd   = 0;

            // Write out the number of samples, sample rate and loop points
            x_fwrite( &nSamples,   sizeof(s32), 1, out );
            x_fwrite( &SampleRate, sizeof(s32), 1, out );
            x_fwrite( &LoopStart,  sizeof(s32), 1, out );
            x_fwrite( &LoopEnd,    sizeof(s32), 1, out );
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
    }

    return TotalCompressedSize;
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
                    break;

                case EXPORT_PS2:
                    return CompressAudioFilePS2_ADPCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_GCN:
                    Result = CompressAudioFileGCN_ADPCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_XBOX:
                    break;
            }
            break;

        case PCM:
            switch( Target )
            {
                case EXPORT_PC:
                    Result = CompressAudioFilePC_PCM( in, out, NumChannels, &SyncSize );
                    break;

                case EXPORT_PS2:
                    break;

                case EXPORT_GCN:
                    break;

                case EXPORT_XBOX:
                    break;
            }
            break;

        case MP3:
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
                if( !ReadCompressedAudioFileHeader( File.CompressedFilename[Target], ReverseEndian, &File.NumChannels[Target], NULL, &File.CompressedSize[Target], NULL ) )
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
                    x_printf( "Error: Source file '%s' not found!\n", File.Filename  );
                    x_DebugMsg( "Error: Source file '%s' not found!\n", File.Filename  );
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
                        x_printf( "Error: Output file '%s' could not be opened!\n", FullFilename );
                        x_DebugMsg( "Error: Output file '%s' could not be opened!\n", FullFilename );
                        s_Package.m_ParseError = TRUE;
                        return FALSE;
                    }
                    else
                    {
                        // Compress the audio file.
                        File.CompressedSize[Target] = CompressAudioFile( In, Out, File.Temperature, &File.NumChannels[Target], NULL, Target );

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
    Data->Headers.Clear();
    Data->FileHeader.Clear();

    Filename = File.CompressedFilename[ Target ];

    x_DebugMsg( "filename: %s\n", (const char*)Filename );

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
                for( i=0 ; i<Data->nWaveforms ; i++ )
                {
                    x_fread( pBuffer, Data->LipSyncSize, 1, f );
                    Data->LipSyncData.Append();
                    Data->LipSyncData[i].Append( (u8*)pBuffer, Data->LipSyncSize );
                }

                // Free the buffer
                x_free( pBuffer );
            }
        }

        // Close the file.
        x_fclose( f );
    }

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
    Data->Headers.Clear();
    Data->FileHeader.Clear();

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

    x_DebugMsg( "filename: %s\n", (const char*)Filename );

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
                for( i=0 ; i<Data->nWaveforms ; i++ )
                {
                    x_fread( pBuffer, Data->LipSyncSize, 1, f );
                    Data->LipSyncData.Append();
                    Data->LipSyncData[i].Append( (u8*)pBuffer, Data->LipSyncSize );
                }

                // Free the buffer
                x_free( pBuffer );
            }
        }

        // Close the file.
        x_fclose( f );
    }

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
                        s32 xHeaderSize        = Data.HeaderSize;

                        if( ReverseEndian )
                        {
                            xCompressionMethod = reverse_endian_32( xCompressionMethod );
                            xCompressedSize    = reverse_endian_32( xCompressedSize );
                            xLipSyncSize       = reverse_endian_32( xLipSyncSize );
                            xHeaderSize        = reverse_endian_32( xHeaderSize );
                        }

                        // Write out the compression data.
                        x_fwrite( &xCompressionMethod, sizeof(s32), 1, f );
                        x_fwrite( &xCompressedSize, sizeof(s32), 1, f );
                        x_fwrite( &xLipSyncSize, sizeof(s32), 1, f );
                        x_fwrite( &xHeaderSize, sizeof(s32), 1, f );

                        // Write out the sample header.
                        x_fwrite( Data.Headers[i], Data.HeaderSize, 1, f );

                        // Write out the sample waveform.
                        x_fwrite( Data.WaveformData[i].GetBuffer(), Data.CompressedSize, 1, f );

                        // Write out the lipsync data
                        x_fwrite( Data.LipSyncData[i].GetBuffer(), Data.LipSyncSize, 1, f );

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
    xbool              Result = FALSE;
    xbool              ReverseEndian = (Target == EXPORT_GCN);

    // Load the multi channel file
    if( LoadMultiChannelFile( File, &Data, TRUE, Target ) )
    {
        // This only works for mono and stereo files...
        if( ((Data.nChannels == 1) && (Data.nWaveforms == 1)) || ((Data.nChannels == 2) && (Data.nWaveforms == 2)) )
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
                    s32 PaddedCompressedSize;
                    s32 TailPadding;
                    s32 NewCompressionMethod;
               
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
                            BufferSize           = 64*1024; // TODO: Fix this.
                            NewCompressionMethod = PC_PCM_MONO_STREAMED;
                            break;

                        case PC_PCM_NON_INTERLEAVED_STEREO:
                            BufferSize           = 64*1024; // TODO: Fix this.
                            NewCompressionMethod = PC_PCM_INTERLEAVED_STEREO;
                            break;

                        default:
                            ASSERT(0);
                            Result = FALSE;
                            break;
                    }

                    // Calculate the padded compressed size and tail padding.
                    PaddedCompressedSize = (BufferSize * ((Data.CompressedSize + BufferSize - 1) / BufferSize));
                    TailPadding          = PaddedCompressedSize - Data.CompressedSize;

                    s32 xCompressionMethod = NewCompressionMethod;
                    s32 xCompressedSize    = PaddedCompressedSize * Data.nChannels;
                    s32 xLipSyncSize       = Data.LipSyncSize;
                    s32 xHeaderSize        = Data.HeaderSize;

                    if( ReverseEndian )
                    {
                        xCompressionMethod = reverse_endian_32( xCompressionMethod );
                        xCompressedSize    = reverse_endian_32( xCompressedSize );
                        xLipSyncSize       = reverse_endian_32( xLipSyncSize );
                        xHeaderSize        = reverse_endian_32( xHeaderSize );
                    }

                    // Write out the compression data.
                    x_fwrite( &xCompressionMethod, sizeof(s32), 1, f );
                    x_fwrite( &xCompressedSize, sizeof(s32), 1, f );
                    x_fwrite( &xLipSyncSize, sizeof(s32), 1, f );
                    x_fwrite( &xHeaderSize, sizeof(s32), 1, f );

                    // Write out the headers.
                    for( s32 i=0 ; i<Data.nChannels ; i++ )
                    {
                        // Write out the sample header.
                        x_fwrite( Data.Headers[i], Data.HeaderSize, 1, f );
                    }

                    // Interleave the waveform data.
                    for( i=0 ; i<PaddedCompressedSize ; i+=BufferSize )
                    {
                        for( s32 j=0 ; j<Data.nChannels ; j++ )
                        {
                            s32 nBytes = BufferSize;

                            // Need to tail pad?
                            if( i+BufferSize > Data.CompressedSize )
                            {
                                nBytes = BufferSize - TailPadding;
                            }
                        
                            ASSERT( nBytes );

                            // Write out the data.
                            x_fwrite( Data.WaveformData[j].GetBuffer()+i, nBytes, 1, f );

                            // Need to tail pad?
                            if( nBytes < BufferSize )
                            {
                                nBytes = TailPadding;

                                while( nBytes-- )
                                {
                                    // Pad it.
                                    x_fwrite( ZeroBuffer, 1, 1, f );
                                }
                            }
                        }
                    }

                    // Write out the lip sync data.
                    for( i=0 ; i<Data.nChannels ; i++ )
                    {
                        // Write out the lip sync data
                        x_fwrite( Data.LipSyncData[i].GetBuffer(), Data.LipSyncSize, 1, f );
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

//------------------------------------------------------------------------------

s32 ParameterSize( compressed_parameters* Params )
{
    if( Params->Bits )
    {
        s32 Result=2;  // Make space for the bit field.

        for( s32 i=0 ; i<NUM_PARAMETERS ; i++ )
        {
            if( Params->Bits & (1<<i) )
                Result += s_ParameterSizes[i];
        }

        return( Result );
    }
    else
    {
        return( 0 );
    }
}

//------------------------------------------------------------------------------

void ExportParameters( compressed_parameters* Params, xbool ReverseEndian )
{
    // Only if any are defined...
    if( Params->Bits )
    {
        s16 Bits = Params->Bits;
        s16 Word;

        // Export the parameter bit field
        if( ReverseEndian )
            Bits = reverse_endian_16( Bits );
        s_Package.m_DescriptorStream.Append( (const u8*)&Bits, 2 );
        Bits = Params->Bits;

        if( Bits & (1<<PITCH) )
        {
            Word = Params->Pitch;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, 2 );
        }

        if( Bits & (1<<PITCH_VARIANCE) )
        {
            Word = Params->PitchVariance;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, 2 );
        }

        if( Bits & (1<<VOLUME) )
        {
            Word = Params->Volume;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, 2 );
        }

        if( Bits & (1<<VOLUME_VARIANCE) )
        {
            Word = Params->VolumeVariance;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, 2 );
        }

        if( Bits & (1<<PAN) )
        {
            s_Package.m_DescriptorStream += Params->Pan;
        }

        if( Bits & (1<<PRIORITY) )
        {
            s_Package.m_DescriptorStream += Params->Priority;
        }

        if( Bits & (1<<EFFECT_SEND) )
        {
            s_Package.m_DescriptorStream += Params->EffectSend;
        }

        if( Bits & (1<<NEAR_FALLOFF) )
        {
            s_Package.m_DescriptorStream += Params->NearFalloff;
        }

        if( Bits & (1<<FAR_FALLOFF) )
        {
            s_Package.m_DescriptorStream += Params->FarFalloff;
        }

        // Need to 16 bit align the stream
        if( s_Package.m_DescriptorStream.GetLength() & 1 )
            s_Package.m_DescriptorStream += '\0';
    }
}

//------------------------------------------------------------------------------

void ExportElement( s32 DescriptorIndex, s32 ElementIndex, xbool ReverseEndian )
{
    s32 ParamSize;
    u16 IndexId;

    // Set the elements type, parameter length and index.
    SET_INDEX_TYPE( IndexId, s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Type );
    ParamSize = ParameterSize( &s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Params );
    SET_INDEX_PARAM_LENGTH( IndexId, ParamSize );
    ASSERT( s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Index >= 0 );
    ASSERT( s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Index <= 2047 );
    SET_INDEX( IndexId, s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Index );

    // Export the element index id
    if( ReverseEndian )
        IndexId = reverse_endian_16( IndexId );
    s_Package.m_DescriptorStream.Append( (const u8*)&IndexId, 2 );

    // Export the elements parameters.
    ExportParameters( &s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Params, ReverseEndian );
}

//------------------------------------------------------------------------------

void ExportSimple( s32 DescriptorIndex, xbool ReverseEndian )
{
    // Export a single element.
    ExportElement( DescriptorIndex, 0, ReverseEndian );
}

//------------------------------------------------------------------------------

void ExportComplex( s32 DescriptorIndex, xbool ReverseEndian )
{
    s32 i;
    s32 nElements    = s_Package.m_Descriptors[ DescriptorIndex ].Elements.GetCount();
    u16 ElementCount = (u16)nElements;

    // Export the number of elements.
    if( ReverseEndian )
        ElementCount = reverse_endian_16( ElementCount );
    s_Package.m_DescriptorStream.Append( (const u8*)&ElementCount, 2 );

    // Clear the exported flag.
    for( i=0 ; i<nElements ; i++ )
    {
        s_Package.m_Descriptors[ DescriptorIndex ].Elements[i].IsExported = FALSE;
    }

    // Export them in ascending time deltas.
    for( i=0 ; i<nElements ; i++ )
    {
        s32 EarliestElement = -1;         // Not defined...
        f32 EarliestTime    = 1000000.0f; // Very large... 
        u16 TimeDelay; 

        // Damn n-squared search...
        for( s32 j=0 ; j<nElements ; j++ )
        {
            if( !s_Package.m_Descriptors[DescriptorIndex].Elements[j].IsExported && s_Package.m_Descriptors[DescriptorIndex].Elements[j].StartDelay < EarliestTime )
            {
                EarliestElement = j;
                EarliestTime    = s_Package.m_Descriptors[DescriptorIndex].Elements[j].StartDelay;
            }
        }

        // Mark it as exported.
        s_Package.m_Descriptors[DescriptorIndex].Elements[EarliestElement].IsExported = TRUE;

        // Export the time delta.
        TimeDelay = FLOAT100_TO_U16BIT( s_Package.m_Descriptors[DescriptorIndex].Elements[EarliestElement].StartDelay );
        if( ReverseEndian )
            TimeDelay = reverse_endian_16( TimeDelay );
        s_Package.m_DescriptorStream.Append( (const u8*)&TimeDelay, 2 );

        // Export the element.
        ExportElement( DescriptorIndex, EarliestElement, ReverseEndian );
    }
}

//------------------------------------------------------------------------------

void ExportRandomList( s32 DescriptorIndex, xbool ReverseEndian )
{
    s32 i;
    s32 nElements    = s_Package.m_Descriptors[ DescriptorIndex ].Elements.GetCount();
    u16 ElementCount = (u16)nElements;

    // Export the number of elements.
    if( ReverseEndian )
        ElementCount = reverse_endian_16( ElementCount );
    s_Package.m_DescriptorStream.Append( (const u8*)&ElementCount, 2 );

    // Now make space for the 64 bit flag field.
    for( i=0 ; i<8 ; i++ )
    {
        s_Package.m_DescriptorStream += '\0';
    }

    // Export each element.
    ASSERT( nElements <= 63 );
    for( i=0 ; i<nElements ; i++ )
    {
        // Export the element.
        ExportElement( DescriptorIndex, i, ReverseEndian );
    }
}

//------------------------------------------------------------------------------

void ExportWeightedList( s32 DescriptorIndex, xbool ReverseEndian )
{
    s32 i;
    f32 TotalWeight;
    f32 CurrentWeight;
    s32 nElements    = s_Package.m_Descriptors[ DescriptorIndex ].Elements.GetCount();
    u16 ElementCount = (u16)nElements;

    // Export the number of elements.
    if( ReverseEndian )
        ElementCount = reverse_endian_16( ElementCount );
    s_Package.m_DescriptorStream.Append( (const u8*)&ElementCount, 2 );

    // Calulate the total weight of all elements.
    for( i=0,TotalWeight=0.0f ; i<nElements ; i++ )
    {
        TotalWeight += (f32)s_Package.m_Descriptors[DescriptorIndex].Elements[i].Weight;
    }

    // Export the weights.
    for( i=0,CurrentWeight=0.0f ; i<nElements ; i++ )
    {
        u16 Weight;
        
        if( i == (nElements-1) )
        {
            Weight = 65535;
        }
        else
        {
            CurrentWeight += (f32)s_Package.m_Descriptors[DescriptorIndex].Elements[i].Weight / TotalWeight;
            if( CurrentWeight > 1.0f )
                CurrentWeight = 1.0f;
            Weight = (u16)(CurrentWeight * 65535.0f);
        }

        if( ReverseEndian )
            Weight = reverse_endian_16( Weight );
        s_Package.m_DescriptorStream.Append( (const u8*)&Weight, 2 );
    }

    // Export each element.
    for( i=0 ; i<nElements ; i++ )
    {
        // Export the element.
        ExportElement( DescriptorIndex, i, ReverseEndian );
    }
}

//------------------------------------------------------------------------------

void ExportDescriptorIdentifiers( xbool ReverseEndian )
{
    s32   StringSize;
    s32   i;
    s32   j;
    u8    Dummy[2] = { 0x7f, 0x00 };

    // Clear the identifier export streams and offsets.
    s_Package.m_StringTableStream.Clear();
    s_Package.m_DescriptorIdentifierStream.Clear();

    // Mark each identifier as not processed.
    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        // Mark identifier as not processed.
        s_Package.m_Descriptors[i].IdentifierProcessed = FALSE;
    }

    // Process each descriptor...
    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        const char* SmallString = (const char*)Dummy;
        s32         SmallIndex  = -1;
        u16         Word;
        u32         Offset;

        // N-squared sort...I know...
        for( j=0 ; j<s_Package.m_Descriptors.GetCount() ; j++ )
        {
            if( !s_Package.m_Descriptors[ j ].IdentifierProcessed )
            {
                if( x_strcmp( (const char*)s_Package.m_Descriptors[ j ].Identifier, SmallString ) < 0 )
                {
                    SmallString = (const char*)s_Package.m_Descriptors[ j ].Identifier;
                    SmallIndex  = j;
                }
            }
        }

        // Mark it as processed.
        s_Package.m_Descriptors[ SmallIndex ].IdentifierProcessed = TRUE;

        // Set the descriptor identifier offset
        StringSize = x_strlen( (const char *)s_Package.m_Descriptors[ SmallIndex ].Identifier );
        Offset     = s_Package.m_StringTableStream.GetLength(); 
        
        // Write null terminated string to the string table.
        s_Package.m_StringTableStream.Append( (const unsigned char*)((const char *)s_Package.m_Descriptors[ SmallIndex ].Identifier), StringSize );
        s_Package.m_StringTableStream += '\0';

        // Export the string table offset.
        ASSERT( Offset <= 65535 );
        Word = Offset;
        if( ReverseEndian )
            Word = reverse_endian_16( Word );
        s_Package.m_DescriptorIdentifierStream.Append( (const u8*)&Word, 2 );

        // Export the index.
        ASSERT( SmallIndex <= 65535 );
        Word = SmallIndex;
        if( ReverseEndian )
            Word = reverse_endian_16( Word );
        s_Package.m_DescriptorIdentifierStream.Append( (const u8*)&Word, 2 );

        // Make space for the pointer to the package.
        s_Package.m_DescriptorIdentifierStream += 0;
        s_Package.m_DescriptorIdentifierStream += 0;
        s_Package.m_DescriptorIdentifierStream += 0;
        s_Package.m_DescriptorIdentifierStream += 0;
    }

    // Pad string table to 32-bit alingment.
    while( s_Package.m_StringTableStream.GetLength() & 3 )
        s_Package.m_StringTableStream += 0;
}

//------------------------------------------------------------------------------

void ExportDescriptors( xbool ReverseEndian )
{
    // Clear the descriptor export stream and offsets.
    s_Package.m_DescriptorStream.Clear();
    s_Package.m_DescriptorOffsets.Clear();
    
    // Process each descriptor...
    for( s32 i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        u16 DescriptorId=0;
        s32 ParamSize;

        if( s_Debug )
        {
            x_printf( "    %s\n", s_Package.m_Descriptors[i].Identifier );
            x_DebugMsg( "    %s\n", s_Package.m_Descriptors[i].Identifier );
        }

        // Set the descriptor offset (must be 16 bit aligned).
        s_Package.m_DescriptorOffsets.Append() = s_Package.m_DescriptorStream.GetLength();
        ASSERT( (s_Package.m_DescriptorOffsets[i] & 1) == 0 );

        // Set type and parameter length of the descriptor.
        SET_DESCRIPTOR_TYPE( DescriptorId, s_Package.m_Descriptors[i].Type );
        ParamSize = ParameterSize( &s_Package.m_Descriptors[i].Params );
        SET_DESCRIPTOR_PARAM_LENGTH( DescriptorId, ParamSize );

        // Export descriptor id.
        if( ReverseEndian )
            DescriptorId = reverse_endian_16( DescriptorId );
        s_Package.m_DescriptorStream.Append( (const u8*)&DescriptorId, 2 );

        // Export the parameters.
        ExportParameters( &s_Package.m_Descriptors[i].Params, ReverseEndian );

        switch( s_Package.m_Descriptors[i].Type )
        {
            case SIMPLE:
                ExportSimple( i, ReverseEndian );
                break;

            case COMPLEX:
                ExportComplex( i, ReverseEndian );
                break;

            case RANDOM_LIST:
                ExportRandomList( i, ReverseEndian );
                break;

            case WEIGHTED_LIST:
                ExportWeightedList( i, ReverseEndian );
                break;
        }

        // 16 bit align
        if( s_Package.m_DescriptorStream.GetLength() & 1 )
            s_Package.m_DescriptorStream += '\0';
    }
}

//------------------------------------------------------------------------------

void ExportLipSyncData( s32 Target )
{
    multi_channel_data Data;

    // Clear the descriptor export stream and offsets.
    s_Package.m_LipSyncTableStream.Clear();
 
    for( s32 i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        file_info& File = s_Package.m_Files[ i ];

        if( File.UsesLipSync )
        {
            // Set the offset.
            File.LipSyncOffset = s_Package.m_LipSyncTableStream.GetLength();
            
            // Load the file.
            LoadTrueMultiChannelFile( File, &Data, TRUE, Target );

            // Mono files for now...
            ASSERT( Data.nChannels == 1 );

            // Append the lip sync data.
            s_Package.m_LipSyncTableStream.Append( Data.LipSyncData[0] );
        }
        else
        {
            File.LipSyncOffset = -1;
        }
    }
}

//------------------------------------------------------------------------------

s32 CalculateAudioRam( s32 Target )
{
    s32 i;
    s32 Footprint;

    for( i=0, Footprint=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        if( s_Package.m_Files[i].Temperature == HOT )
        {
            Footprint += s_Package.m_Files[i].CompressedSize[Target];
        }

        if( s_Package.m_Files[i].Temperature == WARM )
        {
            // TODO: Put in warm sample contribution.
        }
    }

    return Footprint;
}

//------------------------------------------------------------------------------

s32 CalculateMainRam( s32 Target )
{
    s32 Result = 0;
    
    // Descriptor identifiers.
    Result += s_Package.m_DescriptorIdentifierStream.GetLength();

    // String table.
    Result += s_Package.m_StringTableStream.GetLength();

    // Descriptor offset table.
    Result += s_Package.m_Descriptors.GetCount() * sizeof(s32);

    // Descriptors
    Result += s_Package.m_DescriptorStream.GetLength();

    // Number of samples for each temperature + offset table for each temperature.
    for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        Result += sizeof(s32);
        s32 Count = s_Package.m_TemperatureCount[ i ];
        Result += Count * sizeof(s32);
    }

    return Result;
}

//------------------------------------------------------------------------------

s32 CalculateNumberOfSampleHeaders( s32 Temperature, s32 Target )
{
    s32 Total = 0;

    for( s32 i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        file_info& File = s_Package.m_Files[ i ];

        if( File.Temperature == (u32)Temperature )
            Total += File.NumChannels[ Target ];
    }

    return Total;
}

//------------------------------------------------------------------------------

void ExportPackageHeader( xbool ReverseEndian, s32 Target )
{
    package_header PH;

    // Clear the package header stream.
    s_Package.m_HeaderStream.Clear();

    PH.Mram                  = s_Package.m_MainRamFootprint  = CalculateMainRam( Target );
    PH.Aram                  = s_Package.m_AudioRamFootprint = CalculateAudioRam( Target );
    PH.Params.Bits           = 0xffff;
    PH.Params.Pitch          = U16BIT_TO_FLOAT4( s_Package.m_Params.Pitch );
    PH.Params.PitchVariance  = U16BIT_TO_FLOAT1( s_Package.m_Params.PitchVariance );
    PH.Params.Volume         = U16BIT_TO_FLOAT1( s_Package.m_Params.Volume );
    PH.Params.VolumeVariance = U16BIT_TO_FLOAT1( s_Package.m_Params.VolumeVariance );
    PH.Params.Pan            =  S8BIT_TO_FLOAT1( s_Package.m_Params.Pan );
    PH.Params.Priority       = (u32)s_Package.m_Params.Priority;
    PH.Params.EffectSend     =  U8BIT_TO_FLOAT1( s_Package.m_Params.EffectSend );
    PH.Params.NearFalloff    = U8BIT_TO_FLOAT10( s_Package.m_Params.NearFalloff );
    PH.Params.FarFalloff     = U8BIT_TO_FLOAT10( s_Package.m_Params.FarFalloff );
    PH.nDescriptors          = s_Package.m_Descriptors.GetCount();
    // TODO: Not necessarily a one-to-one corespondence between nDescriptrs and nIdentifiers.
    PH.nIdentifiers          = s_Package.m_Descriptors.GetCount(); 
    PH.DescriptorFootprint   = s_Package.m_DescriptorStream.GetLength();
    PH.StringTableFootprint  = s_Package.m_StringTableStream.GetLength();
    PH.LipSyncTableFootprint = s_Package.m_LipSyncTableStream.GetLength();
    
    for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        PH.nSampleHeaders[ i ]  = CalculateNumberOfSampleHeaders( i, Target );
        PH.nSampleIndices[ i ]  = s_Package.m_TemperatureCount[ i ];
        PH.CompressionType[ i ] = s_CompressionTypes[ Target ][ i ];
        PH.HeaderSizes[ i ]     = s_CompressionHeaderSizes[ Target ][ PH.CompressionType[ i ] ];
    }

    if( ReverseEndian )
    {
        PH.Mram                  = reverse_endian_32( PH.Mram );
        PH.Aram                  = reverse_endian_32( PH.Aram );
        PH.Params.Bits           = reverse_endian_32( PH.Params.Bits );
        PH.Params.Pitch          = reverse_endian_f32( PH.Params.Pitch );
        PH.Params.PitchVariance  = reverse_endian_f32( PH.Params.PitchVariance );
        PH.Params.Volume         = reverse_endian_f32( PH.Params.Volume );
        PH.Params.VolumeVariance = reverse_endian_f32( PH.Params.VolumeVariance );
        PH.Params.Pan            = reverse_endian_f32( PH.Params.Pan );
        PH.Params.Priority       = reverse_endian_32( PH.Params.Priority );
        PH.Params.EffectSend     = reverse_endian_f32( PH.Params.EffectSend );
        PH.Params.NearFalloff    = reverse_endian_f32( PH.Params.NearFalloff );
        PH.Params.FarFalloff     = reverse_endian_f32( PH.Params.FarFalloff );
        PH.nDescriptors          = reverse_endian_32( PH.nDescriptors );
        PH.nIdentifiers          = reverse_endian_32( PH.nIdentifiers );
        PH.DescriptorFootprint   = reverse_endian_32( PH.DescriptorFootprint );
        PH.StringTableFootprint  = reverse_endian_32( PH.StringTableFootprint );
        PH.LipSyncTableFootprint = reverse_endian_32( PH.LipSyncTableFootprint );
        for( i=0 ; i<NUM_TEMPERATURES ; i++ )
        {
            PH.nSampleHeaders[ i ]  = reverse_endian_32( PH.nSampleHeaders[ i ] );
            PH.nSampleIndices[ i ]  = reverse_endian_32( PH.nSampleIndices[ i ] );
            PH.CompressionType[ i ] = reverse_endian_32( PH.CompressionType[ i ] );
            PH.HeaderSizes[ i ]     = reverse_endian_32( PH.HeaderSizes[ i ] );
        }
    }

    s_Package.m_HeaderStream.Append( (const u8*)&PH, sizeof(package_header) );
}

//------------------------------------------------------------------------------

void WritePackageToDisk( char*Filename, xbool ReverseEndian, s32 Target )
{
    xbytestream Final;
    xbytestream SampleStream;
    s32         TotalSampleTableSize = 0;    
    s32         TotalSampleTableSize2 = 0;
    s32         CompressionType;
    s32         HeaderSize;
    s32         Fixup;
    X_FILE*     Out;
    char        VersionID[VERSION_ID_SIZE];
    char        TargetID[TARGET_ID_SIZE];
    char        UserID[USER_ID_SIZE];
    char        DateTimeString[DATE_TIME_SIZE];
    char*       TargetString;
    SYSTEMTIME  SysTime;
    xarray<s16> IndexTable; 
    multi_channel_data Data;

    Final.Clear();
    SampleStream.Clear();

    if( s_Verbose )
    {
        x_printf( "Exporting to file '%s '\n", Filename  );
        x_DebugMsg( "Exporting to file '%s '\n", Filename );
    }

    // Write out the version id.
    x_memset( VersionID, 0, VERSION_ID_SIZE );
    switch( Target )
    {
        case EXPORT_PC:   TargetString = PC_PACKAGE_VERSION; break;
        case EXPORT_PS2:  TargetString = PS2_PACKAGE_VERSION; break;
        case EXPORT_GCN:  TargetString = GCN_PACKAGE_VERSION; break;
        case EXPORT_XBOX: TargetString = XBOX_PACKAGE_VERSION; break;
    }
    x_strncpy( VersionID, TargetString, VERSION_ID_SIZE );
    Final.Append( (u8*)VersionID, VERSION_ID_SIZE );

    // Write out the target id.
    x_memset( TargetID, 0, TARGET_ID_SIZE );
    switch( Target )
    {
        case EXPORT_PC:   TargetString = PC_TARGET_ID; break;
        case EXPORT_PS2:  TargetString = PS2_TARGET_ID; break;
        case EXPORT_GCN:  TargetString = GCN_TARGET_ID; break;
        case EXPORT_XBOX: TargetString = XBOX_TARGET_ID; break;
    }
    x_strncpy( TargetID, TargetString, TARGET_ID_SIZE );
    Final.Append( (u8*)TargetID, TARGET_ID_SIZE );

    // Write out the user name.
    x_memset( UserID, 0, USER_ID_SIZE );
    x_strncpy( UserID, "Unknown", USER_ID_SIZE );
    if( getenv("USERNAME") )  
    {
        // Use the username.
        x_strncpy( UserID, getenv("USERNAME"), USER_ID_SIZE );
    }
    Final.Append( (u8*)UserID, USER_ID_SIZE );
    
    // Write out the date and time.
    x_memset( DateTimeString, 0, 16 );
    GetSystemTime( &SysTime );
    x_sprintf( DateTimeString, "%02d/%02d/%02d %02d:%02d", SysTime.wMonth, SysTime.wDay, SysTime.wYear-2000, SysTime.wHour, SysTime.wMinute );
    Final.Append( (u8*)DateTimeString, DATE_TIME_SIZE );
    
    // Write the package header out.
    Final.Append( s_Package.m_HeaderStream );

    // Write out the string table.
    Final.Append( s_Package.m_StringTableStream );

    // Write out the lipsync table
    if( s_Package.m_LipSyncTableStream.GetLength() )
        Final.Append( s_Package.m_LipSyncTableStream );

    // Write out the descriptor identifiers.
    Final.Append( s_Package.m_DescriptorIdentifierStream );

    // Write out the descriptor offset table.
    for( s32 i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        s32 Offset;
        Offset = s_Package.m_DescriptorOffsets[i];
        if( ReverseEndian )
            Offset = reverse_endian_32( Offset );
        Final.Append( (const u8*)&Offset, sizeof(s32) );
    }

    // Write out the descriptors
    Final.Append( s_Package.m_DescriptorStream );

    // For each temperature.
    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        s16 Count = 0;

        if( s_Package.m_TemperatureCount[ i ] )
        {
            IndexTable.Clear();
            IndexTable.SetCapacity( s_Package.m_TemperatureCount[ i ] + 1 );

            // Get compression type based on target and temperature.
            CompressionType = s_CompressionTypes[ Target ][ i ];

            // Get header size based on 'target' and compression type.
            HeaderSize = s_CompressionHeaderSizes[ Target ][ CompressionType ];
        
            // For every file...
            for( s32 j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
            {
                file_info& File = s_Package.m_Files[ j ];

                // Is this file the correct temperature?
                if( File.Temperature == (u32)i )
                {
                    // Set the count.
                    IndexTable.Append() = Count;

                    // Bump size by number of channels * the header size.
                    TotalSampleTableSize += (File.NumChannels[ Target ] * HeaderSize);

                    // Bump count
                    Count += File.NumChannels[ Target ];
                }
            }

            // Dummy one at end for size calculations.
            IndexTable.Append() = Count;

            for( j=0 ; j<IndexTable.GetCount() ; j++ )
            {
                s16 Word = IndexTable[j];
                if( ReverseEndian )
                    Word = reverse_endian_16( Word );

                // Write out the index table
                Final.Append( (const u8*)&Word, sizeof(s16) );
            }
        }
    }

    // Calculate fixup for the samples.
    Fixup  = Final.GetLength();
    Fixup += TotalSampleTableSize;

    // For each temperature...
    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        s32 AudioRam = 0;

        // Get compression type based on file temperature.
        s32 xCompressionType = CompressionType = s_CompressionTypes[ Target ][ i ];
        if( ReverseEndian )
            xCompressionType = reverse_endian_32( xCompressionType );
            
        if( i == COLD )
        {
            // Pad to sector boundary within the file.
            s32 Base = SampleStream.GetLength() + Fixup;
            s32 Padding = (((Base + 2047) / 2048) * 2048) - Base;
            while( Padding-- )
                SampleStream += '\0';
        }

        // For every file...
        for( s32 j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
        {
            file_info& File = s_Package.m_Files[ j ];

            // Is this file the correct temperature?
            if( File.Temperature == (u32)i )
            {
                s32 WaveformOffset = SampleStream.GetLength() + Fixup;

                // Load the file.
                LoadTrueMultiChannelFile( File, &Data, TRUE, Target );

                // For each channel...
                for( s32 k=0 ; k<Data.nChannels ; k++ )
                {
                    // Make space for audio ram reference
                    Final.Append( (const u8*)&AudioRam, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out the offset.
                    if( Data.nWaveforms > 1 )
                        WaveformOffset = SampleStream.GetLength() + Fixup;

                    s32 xWaveformOffset = WaveformOffset; 
                    if( ReverseEndian )
                        xWaveformOffset = reverse_endian_32( xWaveformOffset );
                    Final.Append( (const u8*)&xWaveformOffset, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out length
                    s32 xWaveformLength = Data.CompressedSize;
                    if( ReverseEndian )
                        xWaveformLength = reverse_endian_32( xWaveformLength );
                    Final.Append( (const u8*)&xWaveformLength, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out offset of lipsync data
                    s32 xLipSyncOffset= File.LipSyncOffset;
                    if( ReverseEndian )
                        xLipSyncOffset = reverse_endian_32( xLipSyncOffset );
                    Final.Append( (const u8*)&xLipSyncOffset, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out compression type.
                    Final.Append( (const u8*)&xCompressionType, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out the header.
                    Final.Append( (const u8*)Data.Headers[ k ], Data.HeaderSize );
                    TotalSampleTableSize2 += Data.HeaderSize;

                    // Write out the sample
                    if( k < Data.nWaveforms )
                        SampleStream.Append( Data.WaveformData[ k ] );
                }
            }
        }
    }

    // Append the samples to the package stream.
    ASSERT( TotalSampleTableSize == TotalSampleTableSize2 );
    Final.Append( SampleStream );
    
    // Open the output file.
    Out = x_fopen( Filename , "wb" );
    if( !Out )
    {
        x_printf( "Error: opening output file!\n", Filename );
        x_DebugMsg( "Error: opening output file!\n", Filename );
        return;
    }
    else
    {
        x_fwrite( Final.GetBuffer(), Final.GetLength(), 1, Out );
        x_fclose( Out );
    }
}

//------------------------------------------------------------------------------

void ExportPackage( void )
{
    static char FullFilename[256];

    if( s_Verbose )
    {
        x_printf( "Exporting package.\n"  );
        x_DebugMsg( "Exporting package.\n" );
    }

    // for each target
    for( s32 i=0 ; i<s_Targets.GetCount() ; i++ )
    {
        s32     Target        = s_Targets[i];
        xbool   ReverseEndian = (Target == EXPORT_GCN);

        // Make sure that we don't have any empty filenames.
        if( s_Package.m_Identifier[Target].GetLength() > 0 )
        {        
            //char* pName = x_strrchr( (const char*)s_Package.m_Identifier[Target], '\\' );
            //char pDest[256];
            
            //s32 NameIndex = x_strlen( (const char*)s_Package.m_Identifier[Target] ) - x_strlen( pName );
            //x_strncpy( pDest, (const char*)s_Package.m_Identifier[Target], NameIndex );
            //pDest[NameIndex] = 0;
            
            //char* pDest = x_stristr( (const char*)s_Package.m_Identifier[Target], (const char*)pName );
            //x_splitpath( (const char*)&s_Package.m_Identifier[Target], NULL, NULL, &pName[1], NULL );

            // Construct the file name.
            //x_sprintf( FullFilename, "%s\\Audio%s.%s", pDest, pName , PACKAGE_EXTENSION );
            x_sprintf( FullFilename, "%s.%s", (const char*)s_Package.m_Identifier[Target] , PACKAGE_EXTENSION );

            // Export the lip sync data.
            ExportLipSyncData( Target );

            // Export the descriptors.
            ExportDescriptors( ReverseEndian );

            // Export the descriptor identifiers.
            ExportDescriptorIdentifiers( ReverseEndian );

            // Export the package header.
            ExportPackageHeader( ReverseEndian, Target );

            // Now write it all to the file.
            WritePackageToDisk( FullFilename, ReverseEndian, Target );
        }
        else
        {
            switch( Target )
            {
                case EXPORT_PC:
                {
                    x_printf( "Error: Empty Identifier string for the target PC" );
                    x_DebugMsg( "Error: Empty Identifier string for the target PC" );
                }
                break;
                case EXPORT_PS2:
                {
                    x_printf( "Error: Empty Identifier string for the target PS2" );
                    x_DebugMsg( "Error: Empty Identifier string for the target PS2" );                                
                }
                break;
                case EXPORT_GCN:
                {
                    x_printf( "Error: Empty Identifier string for the target GCN" );
                    x_DebugMsg( "Error: Empty Identifier string for the target GCN" );
                }
                break;
                case EXPORT_XBOX:
                {
                    x_printf( "Error: Empty Identifier string for the target XBOX" );
                    x_DebugMsg( "Error: Empty Identifier string for the target XBOX" );
                }
                break;
                default:
                break;
            }
        }
    }
}

//==============================================================================
//  ProcessScript
//==============================================================================

void ProcessScript( const char* pFileName )
{
    token_stream    Tokenizer;
    s32             done = 0;

    // Set up default parameters
    s_Package.m_Params.Bits            = 0;
    s_Package.m_Params.Pitch           = FLOAT4_TO_U16BIT( 1.0f );     // u16-bit represents [0.0..4.0]
    s_Package.m_Params.PitchVariance   = FLOAT1_TO_U16BIT( 0.0f );     // u16-bit represents +/- [0.0..1.0]
    s_Package.m_Params.Volume          = FLOAT1_TO_U16BIT( 0.75f );    // u16-bit represents [0.0...1.0]
    s_Package.m_Params.VolumeVariance  = FLOAT1_TO_U16BIT( 0.0f );     // u16-bit represents +/- [0.0..1.0]
    s_Package.m_Params.Pan             = FLOAT1_TO_S8BIT( 0.0f );      // s8-bit represents [-1.0..1.0]
    s_Package.m_Params.Priority        = 128;                          // u8-bit represents 256 priorities
    s_Package.m_Params.EffectSend      = FLOAT1_TO_U8BIT( 1.0f );      // u8-bit represents [0.0..1.0]
    s_Package.m_Params.NearFalloff     = FLOAT10_TO_U8BIT( 1.0f );     // u8-bit represents [0%..1000%]
    s_Package.m_Params.FarFalloff      = FLOAT10_TO_U8BIT( 1.0f );     // u8-bit represents [0%..1000%]

    x_memcpy( s_CompressionTypes, s_DefaultCompressionTypes, sizeof(s_CompressionTypes) );

    Tokenizer.SetDelimeter( " ;[]=" );

    // Open the token stream
    if( !(Tokenizer.OpenFile( pFileName )) && !(strcmp( PACKAGE_SECTION, Tokenizer.String() )) )
    {
        x_printf( "Error: Can't open script '%s'\n", pFileName );
    }   
    else
    {
        if( !strcmp( PACKAGE_SECTION, Tokenizer.String() ) )
            Tokenizer.Read();

        while( !done && (Tokenizer.Type() != token_stream::TOKEN_EOF) )
        {
            switch( s_Package.m_ParseSection )
            {
                case NONE:
                    done = !FindPackageKeyWord( &Tokenizer );
                    break;
                case PACKAGE:
                    done = !ParsePackage( &Tokenizer );
                    break;
                case FILES:
                    done = !ParseFiles( &Tokenizer );
                    break;
                case DESCRIPTORS:
                    done = !ParseDescriptors( &Tokenizer );
                    break;
            }
        }

        // Close the tokenizer
        Tokenizer.CloseFile();

        if( s_Package.m_ParseError )
        {
            x_printf( "Error: in scriptfile: %s, line: %d - ABORTING!\n", Tokenizer.GetFilename(), Tokenizer.GetLineNumber() );
            x_DebugMsg( "Error: in scriptfile: %s, line: %d - ABORTING!\n", Tokenizer.GetFilename(), Tokenizer.GetLineNumber() );
            done = 1;
        }
        else
        {
            // Compress the audio.
            if( CompressAudioFiles() )
            {
                // Process the multiple channel audio
                if( ProcessMultiChannelAudio() )
                {
                    // Now resolve everything.
                    if( ResolveReferences() )
                    {
                        // Now export the package.
                        ExportPackage();
                    }
                }
            }
        }
    }
}

//==============================================================================
//  MAIN
//==============================================================================

int main(int argc, char* argv[])
{

    command_line CommandLine;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "V"     );
    CommandLine.AddOptionDef( "D"     );
    CommandLine.AddOptionDef( "PC"    );
    CommandLine.AddOptionDef( "PS2"   );
    CommandLine.AddOptionDef( "GCN"   );
    CommandLine.AddOptionDef( "XBOX"  );
    CommandLine.AddOptionDef( "CLEAN" );

    // Display Banner
    x_printf( "\n" );
    x_printf( "SoundPackager (c)2002 Inevitable Entertainment Inc.\n" );
    x_printf( "\n" );

    // Parse command line & check to display help
    xbool NeedHelp = CommandLine.Parse( argc, argv );
    if( NeedHelp || (CommandLine.GetNumArguments() == 0) )
    {
        DisplayHelp();
        return 10;
    }

    // Try to load the nintendo dsp DLL
    if( !dsptool_Load() )
    {
        x_printf( "Error: loading 'dsptool.dll'\n" );
        return 10;
    }

    // Check command line options
    if( CommandLine.FindOption( xstring("V") ) != -1 )
    {
        s_Verbose = TRUE;
    }
    if( CommandLine.FindOption( xstring("D") ) != -1 )
    {
        s_Debug = TRUE;
        s_Verbose = TRUE;
    }


    if( CommandLine.FindOption( xstring("PC") ) != -1 )
    {
        s_Targets.Append() = EXPORT_PC;
        s_HotOnly = TRUE;
    }
    else if( CommandLine.FindOption( xstring("PS2") ) != -1 )
    {
        s_Targets.Append() = EXPORT_PS2;
    }
    else if( CommandLine.FindOption( xstring("GCN") ) != -1 )
    {
        s_Targets.Append() = EXPORT_GCN;
    }
    else if( CommandLine.FindOption( xstring("XBOX") ) != -1 )
    {
        s_Targets.Append() = EXPORT_XBOX;
    }
    else
    {
        x_printf( "Error: No platform defined.!\n" );
        x_DebugMsg( "Error: No platform defined.!\n" );
    }

    if( CommandLine.FindOption( xstring("CLEAN") ) != -1 )
    {
        s_Clean = TRUE;
    }

    X_FILE* pFile = NULL;


    if( argv[1][0] == '@' )
    {

        pFile = x_fopen( "Temp.txt", "wt" );

        for( s32 j = 0; j < CommandLine.GetNumArguments(); j++ )
        {
            const xstring& Storage = CommandLine.GetArgument( j );
            x_fwrite( Storage, sizeof(char) ,Storage.GetLength(), pFile );
            x_fwrite( "\n", sizeof(char) ,1, pFile );
        }
        
        x_fclose( pFile );

        // Process Script
        ProcessScript( "Temp.txt" );

        system( "del Temp.txt" );

    }
    else
    {
        // Loop through all the files
        for( s32 i=0 ; i<CommandLine.GetNumArguments() ; i++ )
        {
            // Get Pathname of file
            const xstring& ScriptName = CommandLine.GetArgument( i );

            // Display Script Name
            if( s_Verbose )
                x_printf( "Processing script '%s'\n", (const char*)ScriptName );

            // Process Script
            ProcessScript( (const char*)ScriptName );
        }
    }



    
    // Unload nintendo dsp DLL
    dsptool_Unload();

    // Return success
    return 0;
}

//==============================================================================
