//==============================================================================
//
//  TweakMgr.cpp
//
//==============================================================================

#ifdef athyssen
#define DO_LOG_TWEAKS 0
#else
#define DO_LOG_TWEAKS 0
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "TweakMgr.hpp"
#include "parsing\tokenizer.hpp"

#ifndef X_EDITOR
#include "NetworkMgr/GameMgr.hpp"
#endif

//==============================================================================
//  DATA_HANDLE
//==============================================================================

tweak_handle::tweak_handle( void )
{
    data_handle::SetType( DATA_TYPE_TWEAK );
}

//==============================================================================

tweak_handle::~tweak_handle( void )
{
}

//==============================================================================

tweak_handle::tweak_handle( const char* pDataDescriptorName )
{
    data_handle::SetType( DATA_TYPE_TWEAK );
    data_handle::SetName( pDataDescriptorName );
}

//==============================================================================

xbool tweak_handle::Exists( void ) const
{
    tweak_data_block* pTweak = (tweak_data_block*)g_DataVault.GetData( *this );
    return( pTweak != NULL );
}

//==============================================================================

f32 tweak_handle::GetF32( void ) const
{ 
    tweak_data_block* pTweak = (tweak_data_block*)g_DataVault.GetData( *this );
    //ASSERT( pTweak );
    f32 V = (pTweak) ? ((f32)pTweak->GetValue()) : (0);

    #ifdef DATA_VAULT_KEEP_NAMES
    CLOG_ERROR((pTweak==NULL),"MISSING TWEAK","(%s)", GetName() );
    CLOG_MESSAGE(DO_LOG_TWEAKS,"tweak_handle::GetF32","(%8.3f) (%s)", V, GetName() );
    #endif

    return V;
}

//==============================================================================

s32 tweak_handle::GetS32( void ) const
{ 
    tweak_data_block* pTweak = (tweak_data_block*)g_DataVault.GetData( *this );
    //ASSERT( pTweak );
    s32 V = (pTweak) ? ((s32)pTweak->GetValue()) : (0);

    #ifdef DATA_VAULT_KEEP_NAMES
    CLOG_ERROR((pTweak==NULL),"MISSING TWEAK","(%s)", GetName() );
    CLOG_MESSAGE(DO_LOG_TWEAKS,"tweak_handle::GetS32","(%8d) (%s)", V, GetName() );
    #endif

    return V;
}

//==============================================================================

radian tweak_handle::GetRadian( void ) const
{ 
    tweak_data_block* pTweak = (tweak_data_block*)g_DataVault.GetData( *this );
    //ASSERT( pTweak );
    radian V = (pTweak) ? ((radian)pTweak->GetValue()) : (0);

    // We're assuming this badboy was typed in degrees in the tables.
    V = DEG_TO_RAD(V);

    #ifdef DATA_VAULT_KEEP_NAMES
    CLOG_ERROR((pTweak==NULL),"MISSING TWEAK","(%s)", GetName() );
    CLOG_MESSAGE(DO_LOG_TWEAKS,"tweak_handle::GetRadian","(%8.3f) (%s)", V, GetName() );
    #endif

    return V;
}

//==============================================================================

xbool tweak_handle::GetBool( void ) const
{ 
    tweak_data_block* pTweak = (tweak_data_block*)g_DataVault.GetData( *this );
    //ASSERT( pTweak );
    xbool V = (pTweak) ? ((xbool)pTweak->GetValue()) : (0);

#ifdef DATA_VAULT_KEEP_NAMES
    CLOG_ERROR((pTweak==NULL),"MISSING TWEAK","(%s)", GetName() );
    CLOG_MESSAGE(DO_LOG_TWEAKS,"tweak_handle::GetBool","(%8d) (%s)", V, GetName() );
#endif

    return V;
}

//==============================================================================
//  DATA_HANDLE
//==============================================================================

tweak_data_block::tweak_data_block( void )
{
    SetType( DATA_TYPE_TWEAK );
}

//==============================================================================

tweak_data_block::~tweak_data_block( void )
{
}

//==============================================================================

f32 tweak_data_block::GetValue( void ) const
{
    return m_Value;
}

//==============================================================================

void tweak_data_block::SetValue( f32 Value )
{
    m_Value = Value;
}

//==============================================================================
// LOAD TWEAKS
//==============================================================================

#ifndef X_RETAIL

struct s_AudioData
{
    char   Desc[64];
    f32    fVolume;
    f32    fNearClip;
    f32    fFarClip;
};

s_AudioData *AudioTweakData = NULL;
xbool        AUDIO_TWEAK    = FALSE;

//=============================================================================
void LoadAudioTweakFile( const char* pDirectory )
{
    X_FILE* aFile;

    if( AUDIO_TWEAK )
    {
        // make sure we get rid of any data in the structure array and zero it.
        if( AudioTweakData )
        {
            x_free( AudioTweakData );
            AudioTweakData = NULL;
        }

        char tempName[128];
        x_sprintf( tempName,"%s\\Tweak_Audio.audiobin", pDirectory );
        aFile = x_fopen ( tempName , "rb" );
        
        // make sure we're good.  File does NOT have to be there.
        if( !aFile )
        {
            return;
        }

        // store of file and number of entries information
        s32 fileLength  = x_flength( aFile );
        s32 theSize     = sizeof(s_AudioData);
        s32 numEntries  = fileLength/theSize;
        s32 totalSize   = fileLength + sizeof(s_AudioData);

        // if you hit this, talk to Kevin or Rob B.
        ASSERTS( numEntries <= 1500, "Too many Audio Tweak data entries.");

        // get some memory
        AudioTweakData = (s_AudioData*)x_malloc( totalSize );
        
        // clear memory
        x_memset( AudioTweakData, 0, totalSize );

        // read data from file into struct array
        x_fread( AudioTweakData, fileLength, 1, aFile );    

        // loop through and null end of padded string (they're padded to 64 chars with spaces)
        for( s32 i = 0; i < numEntries; i++ )
        {
            if( AudioTweakData[i].Desc )
            {
                // found a space...
                char *pEOS = x_strstr(AudioTweakData[i].Desc, " ");

                // put a null in it's place
                if( pEOS )
                {
                    *pEOS = 0;
                }
            }
        }
        
        // shutdown the file
        x_fclose( aFile );
    }
}
#endif // X_RETAIL

//=============================================================================
xbool LoadTweaks( const char* pDirectory )
{
    s32 i;
    xarray<tweak_data_block> TweakDataArray;
    tweak_data_block         TweakData;

    xtimer Time;

    Time.Start();

#ifndef X_RETAIL
    LoadAudioTweakFile( pDirectory );
#endif //X_RETAIL

    // Open the file for reading
    token_stream TOK;

#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        if( !TOK.OpenFile( xfs("%s\\Tweak_MP_General.txt",pDirectory) ) )
            return FALSE;
    }
    else
#endif
    {
        if( !TOK.OpenFile( xfs("%s\\Tweak_General.txt",pDirectory) ) )
            return FALSE;
    }

    TweakDataArray.SetCapacity( 911 );
    
    // Loop through all the tweaks and add them to the array
    while( TOK.IsEOF() == FALSE )
    {
        TOK.ReadString();

        // If this is a blank line then skip to next line
        if( x_stricmp(TOK.String(),"")==0 )
        {
            TOK.SkipToNextLine();
            continue;
        }

        // Load the tweak_data_block with its name
        char CopyOfName[64];
        x_strcpy(CopyOfName,TOK.String());
        TweakData.SetName( TOK.String() );

        // Confirm there are no duplicate tweaks
        {
            for( i=0; i<TweakDataArray.GetCount(); i++ )
            {
                if( TweakData.GetDataID() == TweakDataArray[i].GetDataID() )
                    break;
            }
            if( i!=TweakDataArray.GetCount() )
            {
                ASSERTS(FALSE,"Tweak duplicate!\n");
                TOK.SkipToNextLine();
                continue;
            }
        }

        // Fill out the pain profile data
        TweakData.m_Value   = TOK.ReadF32FromString();

    //  LOG_MESSAGE("LoadTweaks","NewTweak (%s) (%8.2f)",CopyOfName, TweakData.m_Value);

        // Add profile to the array
        TweakDataArray.Append( TweakData );

        // Fast forward to next line
        TOK.SkipToNextLine();
    }

    TOK.CloseFile();

    // Allocate final array of tweaks
    tweak_data_block* pTW = new tweak_data_block[ TweakDataArray.GetCount() ];
    for( i=0; i<TweakDataArray.GetCount(); i++ )
        pTW[i] = TweakDataArray[i];

    LOG_MESSAGE( "LoadTweaks","Tweak array size: %d elements",TweakDataArray.GetCount() );
    LOG_MESSAGE( "LoadTweaks","End. Took %2.02f seconds.", Time.ReadSec() );

    // Hand off to the data vault.  ..."Keep them out of trouble!"...
    g_DataVault.AddDataBlocks("TWDATA",pTW,TweakDataArray.GetCount(),sizeof(tweak_data_block));

    return TRUE;
}

//==============================================================================

void UnloadTweaks( void )
{
    tweak_data_block* pTW  = (tweak_data_block*)g_DataVault.DelDataBlocks("TWDATA");
    delete[] pTW;
}

//==============================================================================
// TWEAK UTIL FUNCTIONS
//==============================================================================

f32 GetTweakF32( const char* pName )
{
    tweak_handle hTweak( pName );
    return hTweak.GetF32();
}

//==============================================================================

f32 GetTweakF32( const char* pName, f32 Default )
{
    tweak_handle hTweak( pName );
    if( hTweak.Exists() )
    {
        return hTweak.GetF32();
    }
    else
    {
        CLOG_MESSAGE( DO_LOG_TWEAKS, "tweak_handle::GetF32", "(%8.3f) (%s) (not present so using code specified default value)", Default, pName );
        return Default;
    }     
}

//==============================================================================

s32 GetTweakS32( const char* pName )
{
    tweak_handle hTweak( pName );
    return hTweak.GetS32();
}

//==============================================================================

s32 GetTweakS32( const char* pName, s32 Default )
{
    tweak_handle hTweak( pName );
    if( hTweak.Exists() )
    {
        return hTweak.GetS32();
    }
    else
    {
        CLOG_MESSAGE( DO_LOG_TWEAKS, "tweak_handle::GetS32", "(%8d) (%s) (not present so using code specified default value)", Default, pName );
        return Default;
    }     
}

//==============================================================================

radian GetTweakRadian( const char* pName )
{
    tweak_handle hTweak( pName );
    return hTweak.GetRadian();
}

//==============================================================================

radian GetTweakRadian( const char* pName, radian Default )
{
    tweak_handle hTweak( pName );
    if( hTweak.Exists() )
    {
        return hTweak.GetRadian();
    }
    else
    {
        CLOG_MESSAGE( DO_LOG_TWEAKS, "tweak_handle::GetRadian", "(%8.3f) (%s) (not present so using code specified default value)", RAD_TO_DEG( Default ), pName );
        return Default;
    }     
}

//==============================================================================

xbool GetTweakBool( const char* pName )
{
    tweak_handle hTweak( pName );
    return hTweak.GetBool();
}

//==============================================================================

xbool GetTweakBool( const char* pName, xbool Default )
{
    tweak_handle hTweak( pName );
    if( hTweak.Exists() )
    {
        return hTweak.GetBool();
    }
    else
    {
        CLOG_MESSAGE( DO_LOG_TWEAKS, "tweak_handle::GetBool","(%8d) (%s) (not present so using code specified default value)", Default, pName );
        return Default;
    }     
}

//==============================================================================





