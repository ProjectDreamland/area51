#include "texinfo.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\tokenizer.hpp"

//=========================================================================
// IMPLEMENTATION
//=========================================================================

xbool tex_info::Load( const char* pFilename )
{
    // Get just the name of the texture file, since the info file has the same name
    // but a diffrent extension.
    xstring InfoFileName = pFilename;
    InfoFileName = InfoFileName.Left( InfoFileName.Find( '.' ) );
    InfoFileName += ".texinfo";

    // default values
    SoundMat     = MAT_TYPE_CONCRETE;
    PreferredBPP = PREF_BPP_DEFAULT;
    nMipsToBuild = 15;

    // check for the existence of a texinfo file
    X_FILE* pFile = x_fopen( InfoFileName, "rt" );
    if( pFile )
    {
        x_fclose( pFile );
    }
    else
    {
        x_printf( "WARNING: No texinfo defined for texture (defaulting to CONCRETE) [%s]\n", pFilename );	
        return FALSE;
    }

    // open the texinfo file for parsing
    xbool SoundMatFound = FALSE;
    xbool RetValue      = TRUE;
    text_in TextIn;
    TextIn.OpenFile( InfoFileName );
    while ( !TextIn.IsEOF() )
    {
        TextIn.ReadHeader();
        if ( !x_stricmp( "Sound", TextIn.GetHeaderName() ) )
        {
            //s32 HeaderCount = TextIn.GetHeaderCount();
            TextIn.ReadFields();

            // figure out the sound material
            char Buffer[128];
            if ( TextIn.GetField( "Sound_Material:s", Buffer ) )
            {
                xstring MatType = Buffer;
                MatType.MakeUpper();
                SoundMatFound = TRUE;
                if( MatType == "EARTH" )                    SoundMat = MAT_TYPE_EARTH;
                else if( MatType == "ROCK" )                SoundMat = MAT_TYPE_ROCK;
                else if( MatType == "CONCRETE" )            SoundMat = MAT_TYPE_CONCRETE;
                else if( MatType == "SOLID METAL" )         SoundMat = MAT_TYPE_SOLID_METAL;
                else if( MatType == "HOLLOW METAL" )        SoundMat = MAT_TYPE_HOLLOW_METAL;
                else if( MatType == "METAL GRATE" )         SoundMat = MAT_TYPE_METAL_GRATE;
                else if( MatType == "PLASTIC" )             SoundMat = MAT_TYPE_PLASTIC;
                else if( MatType == "WATER" )               SoundMat = MAT_TYPE_WATER;
                else if( MatType == "WOOD" )                SoundMat = MAT_TYPE_WOOD;
                else if( MatType == "ENERGY FIELD" )        SoundMat = MAT_TYPE_ENERGY_FIELD;
                else if( MatType == "BULLET PROOF GLASS" )  SoundMat = MAT_TYPE_BULLET_PROOF_GLASS;
                else if( MatType == "ICE" )                 SoundMat = MAT_TYPE_ICE;
                else if( MatType == "LEATHER" )             SoundMat = MAT_TYPE_LEATHER;
                else if( MatType == "EXOSKELETON" )         SoundMat = MAT_TYPE_EXOSKELETON;
                else if( MatType == "FLESH" )               SoundMat = MAT_TYPE_FLESH;
                else if( MatType == "BLOB" )                SoundMat = MAT_TYPE_BLOB;
                else if( MatType == "FIRE" )                SoundMat = MAT_TYPE_FIRE;
                else if( MatType == "GHOST" )               SoundMat = MAT_TYPE_GHOST;
                else if( MatType == "FABRIC" )              SoundMat = MAT_TYPE_FABRIC;
                else if( MatType == "CERAMIC" )             SoundMat = MAT_TYPE_CERAMIC;
                else if( MatType == "FENCE" )               SoundMat = MAT_TYPE_WIRE_FENCE;
                else if( MatType == "GLASS" )               SoundMat = MAT_TYPE_GLASS;
                else if( MatType == "RUBBER" )              SoundMat = MAT_TYPE_RUBBER;
                else if( MatType == "CARPET" )              SoundMat = MAT_TYPE_CARPET;
                else if( MatType == "CLOTH" )               SoundMat = MAT_TYPE_CLOTH;
                else if( MatType == "DRYWALL" )             SoundMat = MAT_TYPE_DRYWALL;
                else if( MatType == "FLESHHEAD" )           SoundMat = MAT_TYPE_FLESHHEAD;
                else if( MatType == "MARBLE" )              SoundMat = MAT_TYPE_MARBLE;
                else if( MatType == "TILE" )                SoundMat = MAT_TYPE_TILE;
                else
                {
                    x_printf( "WARNING: Unknown sound material for texture (defaulting to CONCRETE) [%s]\n", pFilename );	
                    SoundMat = MAT_TYPE_CONCRETE;
                    RetValue = FALSE;
                }
            }
        }
        else if ( !x_stricmp( "Properties", TextIn.GetHeaderName() ) )
        {
            s32 PrefBPP = 0;
            s32 nMips   = 0;
            TextIn.ReadFields();
            if ( TextIn.GetField( "Preferred_BPP:d", &PrefBPP ) )
            {
                switch( PrefBPP )
                {
                default:
                    PreferredBPP = PREF_BPP_DEFAULT;
                    break;

                case 32:
                    PreferredBPP = PREF_BPP_32;
                    break;

                case 16:
                    PreferredBPP = PREF_BPP_16;
                    break;

                case 8:
                    PreferredBPP = PREF_BPP_8;
                    break;

                case 4:
                    PreferredBPP = PREF_BPP_4;
                    break;
                }
            }

            if ( TextIn.GetField( "Num_Mips:d", &nMips) )
            {
                nMipsToBuild = nMips;
            }
        }
    }

    if ( !SoundMatFound )
    {
        x_printf( "WARNING: No sound material for texture (defaulting to CONCRETE) [%s]\n", pFilename );
        SoundMat = MAT_TYPE_CONCRETE;
        RetValue = FALSE;
    }

    // done reading texinfo
    TextIn.CloseFile();

    return RetValue;
}

// EOF