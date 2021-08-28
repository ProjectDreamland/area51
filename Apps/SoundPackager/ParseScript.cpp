#include "PackageTypes.hpp"
#include "ParseScript.hpp"
#include "ExportPackage.hpp"
#include "SoundPackager.hpp"
#include "Endian.hpp"
#include "parsing\tokenizer.hpp"

char* s_MusicTypes[NUM_MUSIC_TYPES] = 
                                {   "type:", 
                                    "intensity:" 
                                };
char* s_DescriptorTypes[NUM_DESCRIPTOR_TYPES] = 
                                {   "simple", 
                                    "complex", 
                                    "rlist", 
                                    "wlist" 
                                };
char* s_TemperatureTypes[NUM_TEMPERATURES] = 
                                {   "hot", 
                                    "warm", 
                                    "cold" 
                                };
char* s_ParameterTypes[NUM_PARAMETERS] = 
                                {   "pitch",
                                    "pitchvar",
                                    "volume",
                                    "volumevar",
                                    "volcenter",
                                    "vollfe",
                                    "volduck",
                                    "userdata",
                                    "replaydelay",
                                    "*~lastplay!@#$", // Place holder
                                    "pan",
                                    "priority",
                                    "effect",
                                    "nearclip",
                                    "farclip",
                                    "rolloff",
                                    "neardiff",
                                    "fardiff",
                                    "playpercent"
                                };

char* s_FlagTypes[NUM_FLAGS] =
                                {   "surroundon"
                                };
                                    
                                
char* s_RolloffTypes[NUM_ROLLOFFS] = 
{
    "linear", // linear    falloff
    "fast",   // x ^ 2     falloff
    "slow"    // x ^ (1/2) falloff
};

char* s_CompressionTypeNames[NUM_COMPRESSION_TYPES] = 
{ 
    "adpcm", 
    "pcm",
    "mp3"
};

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
        x_printf( "Label '%s' already used! All labels must be unique!\n", Label );
        x_DebugMsg( "Label '%s' already used! All labels must be unique!\n", Label );
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
            if( s_Debug )
            {
                x_printf  ( "%s\n", Tokenizer->String() );
                x_DebugMsg( "%s\n", Tokenizer->String() );
            }
            if( !x_strcmp( PACKAGE_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = PACKAGE;
            }
        }

        Tokenizer->Read();
    }

    if( s_Package.m_ParseSection == NONE )
    {
        x_printf( "Could not find \"%s\" keyword!\n", PACKAGE_SECTION );
        x_DebugMsg("Could not find \"%s\" keyword!\n", PACKAGE_SECTION );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseFlags( token_stream* Tokenizer, compressed_parameters* Params )
{
    if( !x_strcmp( Tokenizer->String(), "(" ) )
    {
        if( s_Debug )
        {
            x_printf( "( " );
            x_DebugMsg( "( " );
        }

        Tokenizer->Read();
        while( Tokenizer->Type() != token_stream::TOKEN_EOF )
        {
            if( Tokenizer->Type() == token_stream::TOKEN_DELIMITER )
            {
                if( x_strcmp( Tokenizer->String(), ")" ) )
                {
                    x_printf( "Error parsing flags\n" );
                    x_DebugMsg( "Error parsing flags\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    if( s_Debug )
                    {
                        x_printf( " )" );
                        x_DebugMsg( " )" );
                    }

                    // Done!
                    Tokenizer->Read();
                    return TRUE;
                }
            }
            else if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
            {
                x_printf( "Error parsing flags!\n" );
                x_DebugMsg( "Error parsing flags!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }
            else
            {
                s32 i;

                // its gotta be a flag....
                for( i=0 ; i<NUM_FLAGS; i++ )
                {
                    // find a flag?
                    if( !x_strcmp( Tokenizer->String(), s_FlagTypes[i] ) )
                    {
                        Tokenizer->Read();
                        if( x_strcmp( Tokenizer->String(), "=" ) )
                        {
                            x_printf( "'=' must follow flag name, found '%s' instead.\n", Tokenizer->String() );
                            x_DebugMsg( "'=' must follow flag name, found '%s' instead.\n", Tokenizer->String() );
                            s_Package.m_ParseError = TRUE;
                            return FALSE;
                        }
                        else
                        {
                            Tokenizer->Read();
                            if( x_stricmp( Tokenizer->String(), "on" ) == 0 )
                            {
                                // Set the bit for this flag.
                                Params->Flags |= 1<<i;
                            }
                            else if( x_stricmp( Tokenizer->String(), "off" ) == 0 )
                            {
                            }
                            else
                            {
                                x_printf  ( "valid flag value is 'on' or 'off'!\n" );
                                x_DebugMsg( "valid flag value is 'on' or 'off'!\n" );
                                s_Package.m_ParseError = TRUE;
                                return FALSE;
                            }

                            if( s_Debug )
                            {
                                x_printf( "=%s ", Tokenizer->String() );
                                x_DebugMsg( "=%s ", Tokenizer->String() );
                            }
                        }
                        break;
                    }
                }

                if( i >= NUM_FLAGS )
                {
                    x_printf( "'%s' is not a valid flag!\n", Tokenizer->String() );
                    x_DebugMsg( "'%s' is not a valid flag!\n", Tokenizer->String() );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
            }
        
            Tokenizer->Read();
        }

        x_printf( "Unexpected end of file parsing flags!\n" );
        x_DebugMsg( "Unexpected end of file parsing flags!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseParameters( token_stream* Tokenizer, compressed_parameters* Params )
{
    Tokenizer->Read();
    if( !x_strcmp( Tokenizer->String(), "[" ) )
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
                if( x_strcmp( Tokenizer->String(), "]" ) )
                {
                    x_printf( "Error parsing parameters\n" );
                    x_DebugMsg( "Error parsing parameters\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    if( s_Debug )
                    {
                        x_printf( " ]" );
                        x_DebugMsg( " ]" );
                    }

                    // Done!
                    Tokenizer->Read();
                    return TRUE;
                }
            }
            else if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
            {
                x_printf( "Error parsing parameters!\n" );
                x_DebugMsg( "Error parsing parameters!\n" );
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
                    if( !x_strcmp( Tokenizer->String(), s_ParameterTypes[i] ) )
                    {
                        Tokenizer->Read();
                        if( x_strcmp( Tokenizer->String(), "=" ) )
                        {
                            x_printf( "'=' must follow parameter name, found '%s' instead.\n", Tokenizer->String() );
                            x_DebugMsg( "'=' must follow parameter name, found '%s' instead.\n", Tokenizer->String() );
                            s_Package.m_ParseError = TRUE;
                            return FALSE;
                        }
                        else
                        {
                            Tokenizer->Read();
                            if( (Tokenizer->Type() != token_stream::TOKEN_NUMBER) && (i!=ROLLOFF_METHOD) )
                            {
                                x_printf( "parameter values must be numeric, found '%s' instead.\n", Tokenizer->String() );
                                x_DebugMsg( "parameter values must be numeric, found '%s' instead.\n", Tokenizer->String() );
                                s_Package.m_ParseError = TRUE;
                                return FALSE;
                            }
                            else
                            {
                                f32 f = Tokenizer->Float();
                                s32 j = Tokenizer->Int();

                                // Set the bit for this parameter.
                                if( i < 16 )
                                    Params->Bits1 |= 1<<i;
                                else
                                    Params->Bits2 |= 1<<(i-16);

                                switch( i )
                                {
                                    case PITCH:
                                        if( f < 0.015625f || f > 4.0f )
                                        {
                                            x_printf( "valid range for pitch is 0.015625 to 4.0\n" );
                                            x_DebugMsg( "valid range for pitch is 0.015625 to 4.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Pitch = FLOAT4_TO_U16BIT( f );
                                        break;

                                    case PITCH_VARIANCE:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for pitch variance is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for pitch variance is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->PitchVariance = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for volume is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for volume is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Volume = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME_VARIANCE:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for volume variance is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for volume variance is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->VolumeVariance = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME_CENTER:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for volume center is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for volume center is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->VolumeCenter = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME_LFE:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for volume lfe is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for volume lfe is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->VolumeLFE = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case VOLUME_DUCK:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for volume duck is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for volume duck is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->VolumeDuck = FLOAT1_TO_U16BIT( f );
                                        break;

                                    case USER_DATA:
                                        if( j < 0 || j > 65535 )
                                        {
                                            x_printf( "valid range for user data is 0 to 65535\n" );
                                            x_DebugMsg( "valid range for user data is 0 to 65535\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->UserData = j;
                                        break;

                                    case REPLAY_DELAY:
                                        if( f < 0.1f || f > 100.0f )
                                        {
                                            x_printf( "valid range for replay dealay is 0.1 to 100.0\n" );
                                            x_DebugMsg( "valid range for user data is 0.1 to 100.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->ReplayDelay = FLOAT_TENTH_TO_U16BIT( f );
                                        
                                        // Force last play on!
                                        if( LAST_PLAY < 16 )
                                            Params->Bits1 |= 1<<LAST_PLAY;
                                        else
                                            Params->Bits2 |= 1<<(LAST_PLAY-16);

                                        Params->LastPlay = 0;
                                        break;

                                     case PAN_2D:
                                        if( f < -1.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for pan is -1.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for pan is -1.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Pan2d = FLOAT1_TO_S8BIT( f );
                                        break;

                                    case PRIORITY:
                                        if( j < 0 || j > 255 )
                                        {
                                            x_printf( "valid range for priority is 0 to 255\n" );
                                            x_DebugMsg( "valid range for priority is 0 to 255\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->Priority = j;
                                        break;

                                    case EFFECT_SEND:
                                        if( f < 0.0f || f > 1.0f )
                                        {
                                            x_printf( "valid range for effect send is 0.0 to 1.0\n" );
                                            x_DebugMsg( "valid range for effect send is 0.0 to 1.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->EffectSend = FLOAT1_TO_U8BIT( f );
                                        break;

                                    case NEAR_FALLOFF:
                                        if( f < 0.0f || f > 10.0f )
                                        {
                                            x_printf( "valid range for effect send is 0.0 to 10.0\n" );
                                            x_DebugMsg( "valid range for effect send is 0.0 to 10.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->NearFalloff = FLOAT10_TO_U8BIT( f );
                                        break;

                                    case FAR_FALLOFF:
                                        if( f < 0.0f || f > 10.0f )
                                        {
                                            x_printf( "valid range for effect send is 0.0 to 10.0\n" );
                                            x_DebugMsg( "valid range for effect send is 0.0 to 10.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->FarFalloff = FLOAT10_TO_U8BIT( f );
                                        break;

                                    case ROLLOFF_METHOD:
                                        s32 k;

                                        for( k=0 ; k<NUM_ROLLOFFS ; k++ )
                                        {
                                            if( x_strcmp( Tokenizer->String(), s_RolloffTypes[k] ) == 0 )
                                                break;
                                        }

                                        if( k >= NUM_ROLLOFFS )
                                        {
                                            x_printf( "invalid rolloff curve specified!\n" );
                                            x_DebugMsg( "invalid rolloff curve specified!\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->RolloffCurve = k;
                                        break;

                                    case NEAR_DIFFUSE:
                                        if( f < 0.0f || f > 10.0f )
                                        {
                                            x_printf( "valid range for near diffuse is 0.0 to 10.0\n" );
                                            x_DebugMsg( "valid range for near diffuse is 0.0 to 10.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->NearDiffuse = FLOAT10_TO_U8BIT( f );
                                        break;

                                    case FAR_DIFFUSE:
                                        if( f < 0.0f || f > 10.0f )
                                        {
                                            x_printf( "valid range for far diffuse is 0.0 to 10.0\n" );
                                            x_DebugMsg( "valid range for far diffuse is 0.0 to 10.0\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->FarDiffuse = FLOAT10_TO_U8BIT( f );
                                        break;

                                    case PLAY_PERCENT:
                                        if( j < 0 || j > 100 )
                                        {
                                            x_printf( "valid range for play percent is 0 to 100\n" );
                                            x_DebugMsg( "valid range for far diffuse is 0 to 100\n" );
                                            s_Package.m_ParseError = TRUE;
                                            return FALSE;
                                        }
                                        Params->PlayPercent = (u8)j;
                                        break;
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
                    x_printf( "'%s' is not a valid parameter!\n", Tokenizer->String() );
                    x_DebugMsg( "'%s' is not a valid parameter!\n", Tokenizer->String() );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
            }
        
            Tokenizer->Read();
        }

        x_printf( "Unexpected end of file parsing parameters!\n" );
        x_DebugMsg( "Unexpected end of file parsing parameters!\n" );
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
            s_Package.m_Identifier = Tokenizer->String();
            s_Package.m_OutputFilename = xfs("%s.%s",Tokenizer->String(),PACKAGE_EXTENSION);
           
            if( s_Debug )
            {
                x_printf( "Package: %s", Tokenizer->String() );
                x_DebugMsg( "Package: %s", Tokenizer->String() );
            }

            // Now parse any parameters
            if( !ParseParameters( Tokenizer, &s_Package.m_Params ) )
                return FALSE;

            // Parse the flags
            if( !ParseFlags( Tokenizer, &s_Package.m_Params ) )
                return FALSE;
            
            if( s_Debug )
            {
                x_printf( "\n" );
                x_DebugMsg( "\n" );
            }

            if( !x_strcmp( FILES_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = FILES;
            }
            else
            {
                x_printf( "\'%s\' keyword expected!\n", FILES_SECTION );
                x_DebugMsg( "\'%s\' keyword expected!\n", FILES_SECTION );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }
        }

        Tokenizer->Read();
    }
    
    if( s_Package.m_ParseSection == PACKAGE )
    {
        x_printf( "Could not find \"%s\" keyword!\n", FILES_SECTION );
        x_DebugMsg("Could not find \"%s\" keyword!\n", FILES_SECTION );
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
            if( !x_strcmp( DESCRIPTOR_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = DESCRIPTORS;
            }
            else
            {
                char FileIdentifier[256];
        
                x_strncpy( FileIdentifier, (const char*)Tokenizer->String(), 255 );
                FileIdentifier[255]=0;

                // Convert to upper case.
                x_strtoupper( FileIdentifier );
        
                if( s_Debug )
                {
                    x_printf( " %s", FileIdentifier );
                    x_DebugMsg( " %s", FileIdentifier );
                }

                // Make sure label is unique.
                if( !LabelIsUnique( FileIdentifier ) )
                    return FALSE;

                // ok, this must be a label...
                file_info& Info = s_Package.m_Files.Append();
                Info.Identifier = FileIdentifier;

                // read the temperature
                Tokenizer->Read();
                if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
                {
                    x_printf( "Sample temperature expected!\n" );
                    x_DebugMsg( "Sample temperature expected!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                    {
                        if( !x_stricmp( Tokenizer->String(), s_TemperatureTypes[i] ) )
                            break;
                    }
                    
                    if( s_HotOnly )
                    {
                        i = 0;
                    }

                    if( i >= NUM_TEMPERATURES )
                    {
                        x_printf( "%s is not a valid sample temperature!\n", Tokenizer->String() );
                        x_DebugMsg( "%s is not a valid sample temperature!\n", Tokenizer->String() );
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
                
                // read the lipsync flag
                Tokenizer->Read();
                if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
                {
                    x_printf( "Lipsync flag expected!\n" );
                    x_DebugMsg( "Lipsync flag expected!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {                    
                    if( !x_strcmp( Tokenizer->String(), "NoLipSync" ) )
                    {
                        Info.UsesLipSync = FALSE;
                    }
                    else if( !x_strcmp( Tokenizer->String(), "LipSync" ) )
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
        x_printf( "Could not find \"%s\" keyword!\n", DESCRIPTOR_SECTION );
        x_DebugMsg("Could not find \"%s\" keyword!\n", DESCRIPTOR_SECTION );
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
        x_printf( "Simple descriptor label expected!\n" );
        x_DebugMsg( "Simple descriptor label expected!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }
    else
    {
        char ElementName[256];
        
        x_strncpy( ElementName, (const char*)Tokenizer->String(), 255 );
        ElementName[255]=0;

        // Convert to upper case.
        x_strtoupper( ElementName );
        
        if( s_Debug )
        {
            x_printf( " %s", ElementName );
            x_DebugMsg( " %s", ElementName );
        }

        element_info& Element = Descriptor.Elements.Append();
        Element.Identifier = ElementName;

        // Copy the default parameters.
        x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );

        // Clear the parameter bits, then parse parameters
        Element.Params.Bits1 = 0;
        Element.Params.Bits2 = 0;
        if( !ParseParameters( Tokenizer, &Element.Params ) )
            return FALSE;

        // Inherit flags from the descriptor, then parse the flags...
        Element.Params.Flags = Descriptor.Params.Flags;
        return ParseFlags( Tokenizer, &Element.Params );
    }
}

//------------------------------------------------------------------------------

xbool ParseComplex( token_stream* Tokenizer, descriptor_info& Descriptor )
{
    // Make sure we have '{'
    Tokenizer->Read();
    if( x_strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "'{' expected!\n" );
        x_DebugMsg( "'{' expected!\n" );
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
    while( x_strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Complex descriptor label expected!\n" );
            x_DebugMsg( "Complex descriptor label expected!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            char ElementName[256];
        
            x_strncpy( ElementName, (const char*)Tokenizer->String(), 255 );
            ElementName[255]=0;

            // Convert to upper case.
            x_strtoupper( ElementName );
        
            if( s_Debug )
            {
                x_printf( " %s", ElementName );
                x_DebugMsg( " %s", ElementName );
            }

            element_info& Element = Descriptor.Elements.Append();
            Element.Identifier = ElementName;

            // Copy the default parameters.
            x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );

            // Clear the parameter bits, then parse parameters
            Element.Params.Bits1 = 0;
            Element.Params.Bits2 = 0;
            if( !ParseParameters( Tokenizer, &Element.Params ) )
                return FALSE;

            // Inherit flags from the descriptor, then parse the flags...
            Element.Params.Flags = Descriptor.Params.Flags;
            if( !ParseFlags( Tokenizer, &Element.Params ) )
                return FALSE;

             // Read the time delay
            if( Tokenizer->Type() != token_stream::TOKEN_NUMBER || !Tokenizer->IsFloat() )
            {
                x_printf( "Floating point time delay (seconds) expected!\n" );
                x_DebugMsg( "Floating point time delay (seconds) expected!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            f32 f = Tokenizer->Float();

            // Error check
            if( (f < 0.0f) || (f > (65535.0f / 100.0f)) )
            {
                x_printf( "Time delay out of range: 0.0 to 655.35 seconds!\n" );
                x_DebugMsg( "Time delay out of range: 0.0 to 655.35 seconds!\n" );
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
    if( x_strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "'{' expected!\n" );
        x_DebugMsg( "'{' expected!\n" );
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
    while( x_strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Random list element descriptor label expected!\n" );
            x_DebugMsg( "Random list element descriptor label expected!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            char ElementName[256];
        
            x_strncpy( ElementName, (const char*)Tokenizer->String(), 255 );
            ElementName[255]=0;

            // Convert to upper case.
            x_strtoupper( ElementName );
        
            if( s_Debug )
            {
                x_printf( " %s", ElementName );
                x_DebugMsg( " %s", ElementName );
            }

            if( Descriptor.Elements.GetCount() >= 63 )
            {
                x_printf( "Random lists can only have 63 elements!\n" );
                x_DebugMsg( "Random lists can only have 63 elements!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            element_info& Element = Descriptor.Elements.Append();
            Element.Identifier = ElementName;

            // Copy the default parameters.
            x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );

            // Clear the parameter bits, then parse parameters
            Element.Params.Bits1 = 0;
            Element.Params.Bits2 = 0;
            if( !ParseParameters( Tokenizer, &Element.Params ) )
                return FALSE;

            // Inherit flags from the descriptor, then parse the flags...
            Element.Params.Flags = Descriptor.Params.Flags;
            if( !ParseFlags( Tokenizer, &Element.Params ) )
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
    if( x_strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "'{' expected!\n" );
        x_DebugMsg( "'{' expected!\n" );
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
    while( x_strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Weighted list element descriptor label expected!\n" );
            x_DebugMsg( "Weighted list element descriptor label expected!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            char ElementName[256];
        
            x_strncpy( ElementName, (const char*)Tokenizer->String(), 255 );
            ElementName[255]=0;

            // Convert to upper case.
            x_strtoupper( ElementName );
        
            if( s_Debug )
            {
                x_printf( " %s", ElementName );
                x_DebugMsg( " %s", ElementName );
            }

            element_info& Element = Descriptor.Elements.Append();
            Element.Identifier = ElementName;

            // Copy the default parameters.
            x_memcpy( &Element.Params, &Descriptor.Params, sizeof( compressed_parameters ) );

            // Clear the parameter bits, then parse parameters.
            Element.Params.Bits1 = 0;
            Element.Params.Bits2 = 0;
            if( !ParseParameters( Tokenizer, &Element.Params ) )
                return FALSE;

            // Inherit flags from the descriptor, then parse the flags...
            Element.Params.Flags = Descriptor.Params.Flags;
            if( !ParseFlags( Tokenizer, &Element.Params ) )
                return FALSE;

            // Read the weight.
            if( Tokenizer->Type() != token_stream::TOKEN_NUMBER || Tokenizer->IsFloat() )
            {
                x_printf( "Integer weight expected!\n" );
                x_DebugMsg( "Integer weight expected!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            u32 i = Tokenizer->Int();

            // Error check.
            if( (i < 0) || (i > 65535) )
            {
                x_printf( "Weight out of range: 0 to 65535!\n" );
                x_DebugMsg( "Weight out of range: 0 to 65535!\n" );
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
    while( (s_Package.m_ParseSection == DESCRIPTORS) && (Tokenizer->Type() != token_stream::TOKEN_EOF) )
    {
        // First thing has to be a label...
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Error parsing descriptors (label expected)!\n" );
            x_DebugMsg( "Error parsing descriptors (label expected)!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            if (x_strcmp( OUTPUT_SECTION, Tokenizer->String())==0 )
            {
                s_Package.m_ParseSection = OUTPUT;
            }
            else if( !x_strcmp( MUSIC_SECTION, Tokenizer->String() ) )
            {
                s_Package.m_ParseSection = MUSIC;
            }
            else
            {
                char DescriptorName[256];
                
                x_strncpy( DescriptorName, (const char*)Tokenizer->String(), 255 );
                DescriptorName[255]=0;

                // Convert to upper case.
                x_strtoupper( DescriptorName );

                // Make sure label is unique.
                if( !LabelIsUnique( DescriptorName ) )
                    return FALSE;

                if( s_Package.m_Descriptors.GetCount() >= 2047 )
                {
                    x_printf( "Limit of 2047 descriptors maximum exceeded!\n" );
                    x_DebugMsg( "Limit of 2047 descriptors maximum exceeded!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }

                // Ok this is a label...
                descriptor_info& Descriptor = s_Package.m_Descriptors.Append();
                Descriptor.Index            = s_Package.m_Descriptors.GetCount()-1;
                Descriptor.Identifier       = DescriptorName;

                if( s_Debug )
                {
                    x_printf( "\n%s", DescriptorName );
                    x_DebugMsg( "\n%s", DescriptorName );
                }

                // Copy the default parameters.
                x_memcpy( &Descriptor.Params, &s_Package.m_Params, sizeof( compressed_parameters ) );

                // Clear the parameter bits, then parse parameters.
                Descriptor.Params.Bits1 = 0;
                Descriptor.Params.Bits2 = 0;
                if( !ParseParameters( Tokenizer, &Descriptor.Params ) )
                    return FALSE;

                // Inherit flags from the package, then parse the flags...
                Descriptor.Params.Flags = s_Package.m_Params.Flags;
                if( !ParseFlags( Tokenizer, &Descriptor.Params ) )
                    return FALSE;

                // Now parse the type.
                if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
                {
                    x_printf( "Error parsing descriptors (type expected)!\n" );
                    x_DebugMsg( "Error parsing descriptors (type expected)!\n" );
                    s_Package.m_ParseError = TRUE;
                    return FALSE;
                }
                else
                {
                    s32 i;
                    for( i=0 ; i<NUM_DESCRIPTOR_TYPES ; i++ )
                    {
                        if( !x_strcmp( Tokenizer->String(), s_DescriptorTypes[i] ) )
                            break;
                    }

                    if( i >= NUM_DESCRIPTOR_TYPES )
                    {
                        x_printf( "%s is not a valid descriptor type!\n", Tokenizer->String() );
                        x_DebugMsg( "%s is not a valid descriptor type!\n", Tokenizer->String() );
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
    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseMusicType( token_stream* Tokenizer )
{
    Tokenizer->Read();

    x_strncpy( s_Package.m_MusicType, Tokenizer->String(), 32 );

    Tokenizer->Read();

    return TRUE;
}

//------------------------------------------------------------------------------

xbool ParseMusicIntensity( token_stream* Tokenizer )
{
    // Make sure we have '{'
    Tokenizer->Read();
    if( x_strcmp( Tokenizer->String(), "{" ) )
    {
        x_printf( "'{' expected!\n" );
        x_DebugMsg( "'{' expected!\n" );
        s_Package.m_ParseError = TRUE;
        return FALSE;
    }

    if( s_Debug )
    {
        x_printf( "\n{" );
        x_DebugMsg( "\n{" );
    }

    // Parse thru the intensity looking for the '}'
    Tokenizer->Read();
    while( x_strcmp( Tokenizer->String(), "}" ) )
    {
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Intensity descriptor expected!\n" );
            x_DebugMsg( "Intensity descriptor expected!\n" );
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

            music_intensity& Intensity = s_Package.m_Intensity.Append();
            x_strncpy( Intensity.Descriptor, Tokenizer->String(), 31 );

            // Read the intensity level.
            Tokenizer->Read();
            if( Tokenizer->Type() != token_stream::TOKEN_NUMBER || Tokenizer->IsFloat() )
            {
                x_printf( "Intensity level integer expected!\n" );
                x_DebugMsg( "Intensity level integer expected!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            s32 n = Tokenizer->Int();

            // Error check
            if( (n < 0) || (n > 255) )
            {
                x_printf( "Intensity level out of range: 0 to 255!\n" );
                x_DebugMsg( "Intensity level out of range: 0 to 255!\n" );
                s_Package.m_ParseError = TRUE;
                return FALSE;
            }

            Intensity.Level = (u8)n;

            if( s_Debug )
            {
                x_printf( " =%s", Tokenizer->String() );
                x_DebugMsg( " =%s", Tokenizer->String() );
            }
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

xbool ParseOutput( token_stream* Tokenizer)
{
    Tokenizer->Read();
    if (Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
    {
        x_printf( "ERROR: Output section had no filename defined.\n" );
        x_DebugMsg( "ERROR: Output section had no filename defined.\n" );
        s_Package.m_ParseError = TRUE;
    }
    s_Package.m_OutputFilename = Tokenizer->String();
    return FALSE;
}

//------------------------------------------------------------------------------

xbool ParseMusic( token_stream* Tokenizer )
{
    if( s_Verbose )
    {
        x_printf( "\nParsing music...\n" );
        x_DebugMsg( "\nParsing music...\n" );
    }

    // Loop over tokens...
    Tokenizer->Read();
    while( Tokenizer->Type() != token_stream::TOKEN_EOF )
    {
        // First thing has to be a label...
        if( Tokenizer->Type() != token_stream::TOKEN_SYMBOL )
        {
            x_printf( "Error parsing music (label expected)!\n" );
            x_DebugMsg( "Error parsing music (label expected)!\n" );
            s_Package.m_ParseError = TRUE;
            return FALSE;
        }
        else
        {
            if (x_stricmp( OUTPUT_SECTION, Tokenizer->String())==0 )
            {
                s_Package.m_ParseSection = OUTPUT;
                return TRUE;
            }
            else
            {
                s32 i;
                for( i=0 ; i<NUM_MUSIC_TYPES ; i++ )
                {
                    if( !x_strcmp( Tokenizer->String(), s_MusicTypes[i] ) )
                        break;
                }

                if( i >= NUM_MUSIC_TYPES )
                {
                    x_printf( "%s is not a valid music label!\n", Tokenizer->String() );
                    x_DebugMsg( "%s is not a valid music label!\n", Tokenizer->String() );
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

                    switch( i )
                    {
                        case MUSIC_TYPE:
                            if( !ParseMusicType( Tokenizer ) )
                                return FALSE;
                            break;

                        case MUSIC_INTENSITY:
                            if( !ParseMusicIntensity( Tokenizer ) )
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
            x_printf( "Error! Label '%s' is referenced, but not defined!\n", Element.Identifier );
            x_DebugMsg( "Error! Label '%s' is referenced, but not defined!\n", Element.Identifier );
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




