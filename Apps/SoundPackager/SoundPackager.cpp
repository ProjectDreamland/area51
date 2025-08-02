//==============================================================================
//==============================================================================
// SoundPackager.cpp
//==============================================================================
//==============================================================================

#include "stdafx.h"

#include "Auxiliary/CommandLine/CommandLine.hpp"
#include "x_bytestream.hpp"
#include "DspTool.hpp"
#include "parsing\tokenizer.hpp"
#include "x_debug.hpp"
#include "aiff_file.hpp"
#include "endian.hpp"
#include <windows.h>
#include <stdlib.h>
#include "audio_private_pkg.hpp"
#include "x_threads.hpp"

#include "PackageTypes.hpp"
#include "ParseScript.hpp"
#include "Compress.hpp"
#include "ExportPackage.hpp"

extern "C"
{
	#include "encvag.h"
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

xarray<s32>     s_Targets;
xbool           s_Verbose    = FALSE;
xbool           s_Debug      = FALSE;
xbool           s_Clean      = FALSE;
xbool           s_HotOnly    = FALSE;
xbool           s_Sample2Cpp = FALSE;

s32             s_DefaultCompressionTypes[EXPORT_NUM_TARGETS][NUM_TEMPERATURES] = 
                                {
                                    {   PCM,   PCM,   MP3 }, // PC
                                    { ADPCM, ADPCM, ADPCM }, // PS2
                                    { ADPCM, ADPCM,   MP3 }, // GameCube
                                    { ADPCM, ADPCM, ADPCM }  // xbox
                                };

s32             s_CompressionTypes[EXPORT_NUM_TARGETS][NUM_TEMPERATURES];
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
    x_printf( "         -dump              - Dump samples to .cpp files\n" );
    x_printf( "         -cmdline           - Entire script is on command line\n");
    x_printf( "\n" );
}


//==============================================================================
//  ProcessScript
//==============================================================================

void ProcessScript( const char* pFileName )
{
    token_stream    Tokenizer;
    s32             done = 0;

    // Set up default parameters
    s_Package.m_Params.Bits1           = 0;
    s_Package.m_Params.Bits2           = 0;
    s_Package.m_Params.Flags           = SURROUND_ENABLED;             // Default is surround enabled.
    s_Package.m_Params.Pitch           = FLOAT4_TO_U16BIT( 1.0f );     // u16-bit represents [0.0..4.0]
    s_Package.m_Params.PitchVariance   = FLOAT1_TO_U16BIT( 0.0f );     // u16-bit represents +/- [0.0..1.0]
    s_Package.m_Params.Volume          = FLOAT1_TO_U16BIT( 1.0f );     // u16-bit represents [0.0...1.0]
    s_Package.m_Params.VolumeVariance  = FLOAT1_TO_U16BIT( 0.0f );     // u16-bit represents +/- [0.0..1.0]
    s_Package.m_Params.VolumeCenter    = FLOAT1_TO_U16BIT( 0.0f );     // u16-bit represents [0.0...1.0]
    s_Package.m_Params.VolumeLFE       = FLOAT1_TO_U16BIT( 0.0f );     // u16-bit represents [0.0...1.0]
    s_Package.m_Params.VolumeDuck      = FLOAT1_TO_U16BIT( 1.0f );     // u16-bit represents [0.0...1.0]
    s_Package.m_Params.UserData        = 0;                            // u16-bit represents whatever... 
    s_Package.m_Params.ReplayDelay     = 0;                            // u16-bit represents [0.0..6553.5] seconds.
    s_Package.m_Params.LastPlay        = 0;                            // u16-bit represents 0.0..6553.5] the last time the descriptor was played.
    s_Package.m_Params.Pan2d           = FLOAT1_TO_S8BIT ( 0.0f );     // s8-bit represents [-1.0..1.0]
    s_Package.m_Params.Priority        = 128;                          // u8-bit represents 256 priorities
    s_Package.m_Params.EffectSend      = FLOAT1_TO_U8BIT ( 1.0f );     // u8-bit represents [0.0..1.0]
    s_Package.m_Params.NearFalloff     = FLOAT10_TO_U8BIT( 1.0f );     // u8-bit represents [0%..1000%]
    s_Package.m_Params.FarFalloff      = FLOAT10_TO_U8BIT( 1.0f );     // u8-bit represents [0%..1000%]
    s_Package.m_Params.RolloffCurve    = LINEAR_ROLLOFF;
    s_Package.m_Params.NearDiffuse     = FLOAT10_TO_U8BIT( 1.0f );     // u8-bit represents [0%..1000%]
    s_Package.m_Params.FarDiffuse      = FLOAT10_TO_U8BIT( 1.0f );     // u8-bit represents [0%..1000%]
    s_Package.m_Params.PlayPercent     = 100;                          // u8-bit represents [1..100]

    s_Package.m_MusicType[0] = 0;

    x_memcpy( s_CompressionTypes, s_DefaultCompressionTypes, sizeof(s_CompressionTypes) );

    Tokenizer.SetDelimeter( " ;[]()=" );

    // Open the token stream
    if( Tokenizer.OpenFile( pFileName ) )
    {
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
                case MUSIC:
                    done = !ParseMusic( &Tokenizer );
                    break;
                case OUTPUT:
                    done = !ParseOutput( &Tokenizer );
                    break;
            }
        }

        // Close the tokenizer
        Tokenizer.CloseFile();

        if( s_Package.m_ParseError )
        {
            x_printf( "Error in scriptfile: %s, line: %d - ABORTING!\n", Tokenizer.GetFilename(), Tokenizer.GetLineNumber() );
            x_DebugMsg( "Error in scriptfile: %s, line: %d - ABORTING!\n", Tokenizer.GetFilename(), Tokenizer.GetLineNumber() );
            done = 1;
        }
        else
        {
            // Compress the audio.
            if( CompressAudioFiles() )
            {
                // Dump samples to .cpp file?
                if( s_Sample2Cpp )
                {
                    DumpSamples();
                }

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
    else
    {
        x_printf( "Error: Can't open script '%s'\n", pFileName );
    }
}

//==============================================================================
//  MAIN
//==============================================================================

int main(int argc, char* argv[])
{
    command_line CommandLine;
    xbool CommandLineScript = FALSE;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "V"     );
    CommandLine.AddOptionDef( "D"     );
    CommandLine.AddOptionDef( "PC"    );
    CommandLine.AddOptionDef( "PS2"   );
    CommandLine.AddOptionDef( "GCN"   );
    CommandLine.AddOptionDef( "XBOX"  );
    CommandLine.AddOptionDef( "CLEAN" );
    CommandLine.AddOptionDef( "CMDLINE");
    CommandLine.AddOptionDef( "DUMP"  );

    // Display Banner
    x_printf( "\n" );
    x_printf( "SoundPackager v1.1 | (c)2025 Inevitable Entertainment Inc. | Intervelop\n" );
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
        x_printf( "Error loading 'dsptool.dll'\n" );
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
    if( CommandLine.FindOption( xstring("DUMP") ) != -1 )
    {
        s_Sample2Cpp = TRUE;
    }
    if( CommandLine.FindOption( xstring("PC") ) != -1 )
    {
        s_Targets.Append() = EXPORT_PC;
    }
    if( CommandLine.FindOption( xstring("PS2") ) != -1 )
    {
        s_Targets.Append() = EXPORT_PS2;
    }
    if( CommandLine.FindOption( xstring("GCN") ) != -1 )
    {
        s_Targets.Append() = EXPORT_GCN;
    }
    if( CommandLine.FindOption( xstring("XBOX") ) != -1 )
    {
        s_Targets.Append() = EXPORT_XBOX;
    }
    if( CommandLine.FindOption( xstring("CLEAN") ) != -1 )
    {
        s_Clean = TRUE;
    }

    if( CommandLine.FindOption( xstring("CMDLINE") ) != -1 )
    {
        CommandLineScript = TRUE;
    }

    if (CommandLineScript)
    {
        xstring TempFile;
        X_FILE* pFile;

        if (s_Verbose)
            x_printf("Processing command line script\n");

        TempFile = xfs("Temp_%d.txt",x_GetCurrentThread()->GetSystemId());
        pFile = x_fopen( TempFile, "wt" );
        if (!pFile)
        {
            x_printf("ERROR: Unable to create temporary file\n");
            exit(-1);
        }

        for( s32 j = 0; j < CommandLine.GetNumArguments(); j++ )
        {
            const xstring& Storage = CommandLine.GetArgument( j );
            x_fwrite( Storage, sizeof(char) ,Storage.GetLength(), pFile );
            x_fwrite( "\n", sizeof(char) ,1, pFile );
        }
    
        x_fclose( pFile );

        // Process Script
        ProcessScript( TempFile );

        //system( xfs("del %s",TempFile) );

    }
    else
    {
        // Loop through all the files
        for( s32 i=0 ; i<CommandLine.GetNumArguments() ; i++ )
        {
            // Get Pathname of file
            const xstring& ScriptName = CommandLine.GetArgument( i );

            // Display Script Name
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

