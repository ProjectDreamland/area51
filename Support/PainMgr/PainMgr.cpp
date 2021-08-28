//==============================================================================
//
//  PainMgr.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "PainMgr\PainMgr.hpp"
#include "parsing\tokenizer.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameMgr.hpp"

#define DO_LOG_PAIN

//==============================================================================
// SUPPORT FOR GENERIC PAIN
//==============================================================================

typedef enum_pair<generic_pain_type> generic_pain_type_enum_pair;
static generic_pain_type_enum_pair s_GenericPainTypeList[] = 
{
    generic_pain_type_enum_pair( "Generic_1",            TYPE_GENERIC_1 ),
    generic_pain_type_enum_pair( "Generic_2",            TYPE_GENERIC_2 ),
    generic_pain_type_enum_pair( "Generic_3",            TYPE_GENERIC_3 ),
    generic_pain_type_enum_pair( "Generic_Lethal",       TYPE_LETHAL    ),
    generic_pain_type_enum_pair( "Generic_Explosive",    TYPE_EXPLOSIVE ),
    generic_pain_type_enum_pair( "Generic_LASER",        TYPE_LASER     ),
    generic_pain_type_enum_pair( "ACID_WEAK",            TYPE_ACID_1    ),
    generic_pain_type_enum_pair( "ACID_STRONG",          TYPE_ACID_2    ),
    generic_pain_type_enum_pair( "ACID_LETHAL",          TYPE_ACID_3    ),

    generic_pain_type_enum_pair( "GOO_WEAK",             TYPE_GOO_1     ),
    generic_pain_type_enum_pair( "GOO_STRONG",           TYPE_GOO_2     ),
    generic_pain_type_enum_pair( "GOO_LETHAL",           TYPE_GOO_3     ),

    generic_pain_type_enum_pair( "FIRE_WEAK",            TYPE_FIRE_1    ),
    generic_pain_type_enum_pair( "FIRE_STRONG",          TYPE_FIRE_2    ),
    generic_pain_type_enum_pair( "FIRE_LETHAL",          TYPE_FIRE_3    ),

    generic_pain_type_enum_pair( "DROWNING",             TYPE_DROWNING  ),

    generic_pain_type_enum_pair( k_EnumEndStringConst,   TYPE_GENERIC_1 )
};
enum_table<generic_pain_type>  g_GenericPainTypeList( s_GenericPainTypeList );              

//==============================================================================

pain_handle GetPainHandleForGenericPain( generic_pain_type Type )
{
    ASSERT( g_GenericPainTypeList.DoesValueExist( Type ) );
    pain_handle Handle( g_GenericPainTypeList.GetString( Type ) );
    return Handle;
}

//==============================================================================
//  PAIN_HANDLE
//==============================================================================

pain_handle::pain_handle( void )
{
    data_handle::SetType( DATA_TYPE_PAIN_PROFILE );
}

//==============================================================================

pain_handle::~pain_handle( void )
{
}

//==============================================================================

pain_handle::pain_handle( const char* pPainDescriptor )
{
    data_handle::SetType( DATA_TYPE_PAIN_PROFILE );
    data_handle::SetName( pPainDescriptor );
}

//==============================================================================

const pain_profile* pain_handle::GetPainProfile( void ) const
{
    const pain_profile* pData = (pain_profile*)GetData();

    #if defined(DATA_VAULT_KEEP_NAMES)
        CLOG_ERROR((pData==NULL),"MISSING PAIN PROFILE","(%s)",GetName());
    #endif

    return pData;
}

//==============================================================================

pain_health_handle pain_handle::BuildPainHealthProfileHandle( const health_handle& HHandle ) const
{
    pain_health_handle PHHandle;
    const pain_profile* pPP = GetPainProfile();
    const health_profile* pHP = HHandle.GetHealthProfile();
    if( pPP && pHP )
    {
        PHHandle.SetName(xfs("PH(%03d)(%03d)",pPP->m_iPainHealthTable,pHP->m_iPainHealthTable));
    }
    return PHHandle;
}

//==============================================================================
// HEALTH_HANDLE
//==============================================================================

health_handle::health_handle( void )
{
    data_handle::SetType( DATA_TYPE_HEALTH_PROFILE );
}

//==============================================================================

health_handle::~health_handle( void )
{
}

//==============================================================================

health_handle::health_handle( const char* pHealthDescriptor )
{
    data_handle::SetType( DATA_TYPE_HEALTH_PROFILE );
    data_handle::SetName( pHealthDescriptor );
}

//==============================================================================

const health_profile* health_handle::GetHealthProfile( void ) const
{
    const health_profile* pData = (health_profile*)GetData();

    #if defined(DATA_VAULT_KEEP_NAMES)
        CLOG_ERROR((pData==NULL),"MISSING HEALTH PROFILE","(%s)",GetName());
    #endif

    return pData;
}

//==============================================================================
// PAIN_HEALTH_HANDLE
//==============================================================================

pain_health_handle::pain_health_handle( void )
{
    data_handle::SetType( DATA_TYPE_PAIN_HEALTH_PROFILE );
}

//==============================================================================

pain_health_handle::~pain_health_handle( void )
{
}

//==============================================================================

pain_health_handle::pain_health_handle( const char* pPainHealthDescriptor )
{
    data_handle::SetType( DATA_TYPE_PAIN_HEALTH_PROFILE );
    data_handle::SetName( pPainHealthDescriptor );
}

//==============================================================================

const pain_health_profile* pain_health_handle::GetPainHealthProfile( void ) const
{
    const pain_health_profile* pData = (pain_health_profile*)GetData();

    #if defined(DATA_VAULT_KEEP_NAMES)
        CLOG_ERROR((pData==NULL),"MISSING PAIN_HEALTH","(%s)",GetName());
    #endif

    return pData;
}

//==============================================================================
//  DATA_BLOCK
//==============================================================================

pain_profile::pain_profile( void )
{
    SetType( DATA_TYPE_PAIN_PROFILE );
}

//==============================================================================

pain_profile::~pain_profile( void )
{
}

//==============================================================================

health_profile::health_profile( void )
{
    SetType( DATA_TYPE_HEALTH_PROFILE );
}

//==============================================================================

health_profile::~health_profile( void )
{
}

//==============================================================================

pain_health_profile::pain_health_profile( void )
{
    SetType( DATA_TYPE_PAIN_HEALTH_PROFILE );
}

//==============================================================================

pain_health_profile::~pain_health_profile( void )
{
}


//==============================================================================
// LOAD PAIN PROFILES
//==============================================================================

xbool LoadPainProfiles( const char* pFileName )
{
    s32 i;
    xarray<pain_profile> PainProfileArray;
    pain_profile         PainProfile;
    xtimer              DeltaTime;

    LOG_MESSAGE("LoadPainProfiles","Begin");

    // Open the file for reading
    token_stream TOK;
    if( !TOK.OpenFile( pFileName ) )
        return FALSE;

    // Loop through the header tokens
    {
        TOK.SkipToNextLine();
    }

    DeltaTime.Start();
    // Loop through all the profiles and add them to the array
    while( TOK.IsEOF() == FALSE )
    {
        TOK.ReadString();

        // If this is a blank line then skip to next line
        if( x_stricmp(TOK.String(),"")==0 )
        {
            TOK.SkipToNextLine();
            continue;
        }

        // Load the pain_profile with its name
        PainProfile.SetName( TOK.String() );

        // Confirm there are no duplicate pain profiles
        {
            for( i=0; i<PainProfileArray.GetCount(); i++ )
            {
                if( PainProfile.GetDataID() == PainProfileArray[i].GetDataID() )
                    break;
            }
            if( i!=PainProfileArray.GetCount() )
            {
                ASSERTS(FALSE,"PainProfile duplicate!\n");
                TOK.SkipToNextLine();
                continue;
            }
        }

        // Skip Blank column
        TOK.ReadString();

        // Fill out the pain profile data
        PainProfile.m_DamageNearDist   = TOK.ReadF32FromString();
        PainProfile.m_DamageFarDist    = TOK.ReadF32FromString();
        PainProfile.m_ForceNearDist    = TOK.ReadF32FromString();
        PainProfile.m_ForceFarDist     = TOK.ReadF32FromString();
        PainProfile.m_bCheckLOS        = TOK.ReadBoolFromString();
        PainProfile.m_bSplash          = TOK.ReadBoolFromString();
        PainProfile.m_iPainHealthTable  = PainProfileArray.GetCount();

        if( PainProfile.m_DamageFarDist == PainProfile.m_DamageNearDist )
            PainProfile.m_DamageFarDist += 1.0f;

        if( PainProfile.m_ForceFarDist == PainProfile.m_ForceNearDist )
            PainProfile.m_ForceFarDist += 1.0f;

        // Build pain bbox
        f32 MaxRadius = MAX(PainProfile.m_DamageFarDist,PainProfile.m_ForceFarDist);
            MaxRadius = MAX(MaxRadius,10);
        PainProfile.m_BBox = bbox( vector3(0,0,0), MaxRadius );

        // Add profile to the array
        PainProfileArray.Append( PainProfile );

        // Fast forward to next line
        TOK.SkipToNextLine();
    }

    TOK.CloseFile();

    // Allocate final array of pain profiles 
    pain_profile* pPP = new pain_profile[ PainProfileArray.GetCount() ];
    for( i=0; i<PainProfileArray.GetCount(); i++ )
        pPP[i] = PainProfileArray[i];

    // Hand off to the data vault.  ..."Keep them out of trouble!"...
    g_DataVault.AddDataBlocks("P_PROFILE",pPP,PainProfileArray.GetCount(),sizeof(pain_profile));

    LOG_MESSAGE("LoadPainProfiles","End");
    return TRUE;
}

//==============================================================================
// LOAD HEALTH PROFILES
//==============================================================================

xbool LoadHealthProfiles( const char* pFileName )
{
    LOG_MESSAGE("LoadHealthProfiles","Begin");

    s32 i;
    xarray<health_profile> HealthProfileArray;
    health_profile         HealthProfile;

    // Open the file for reading
    token_stream TOK;
    if( !TOK.OpenFile( pFileName ) )
        return FALSE;

    // Loop through all the profiles and add them to the array
    s32 LN = TOK.GetLineNumber();
    while( 1 )
    {
        TOK.ReadString();

        if( TOK.GetLineNumber() != LN )
            break;

        // If this is a blank line then skip to next line
        if( x_stricmp(TOK.String(),"")==0 )
        {
            continue;
        }

        // Load the health_profile with its name
        HealthProfile.SetName( TOK.String() );

        // Confirm there are no duplicate health profiles
        {
            for( i=0; i<HealthProfileArray.GetCount(); i++ )
            {
                if( HealthProfile.GetDataID() == HealthProfileArray[i].GetDataID() )
                    break;
            }
            if( i!=HealthProfileArray.GetCount() )
            {
                ASSERTS(FALSE,"HealthProfile duplicate!\n");
                continue;
            }
        }

        // Fill out the health profile data
        HealthProfile.m_iPainHealthTable  = HealthProfileArray.GetCount();

        // Add profile to the array
        HealthProfileArray.Append( HealthProfile );
    }

    TOK.CloseFile();

    // Allocate final array of health profiles 
    health_profile* pHP = new health_profile[ HealthProfileArray.GetCount() ];
    for( i=0; i<HealthProfileArray.GetCount(); i++ )
        pHP[i] = HealthProfileArray[i];

    // Hand off to the data vault.  ..."Keep them out of trouble!"...
    g_DataVault.AddDataBlocks("H_PROFILE",pHP,HealthProfileArray.GetCount(),sizeof(health_profile));

    LOG_MESSAGE("LoadHealthProfiles","End");
    return TRUE;
}

//==============================================================================
// LOAD PAIN_HEALTH PROFILES
//==============================================================================

xbool BuildPainHealthProfiles( const char* pFileName )
{
    LOG_MESSAGE("BuildPainHealthProfiles","Begin");

    health_handle         BlankHealthHandle;
    xarray<health_handle> HealthHandle;
    xarray<pain_health_profile> PainHealthProfileArray;
    pain_health_profile PainHealthProfile;
    // Open the file for reading
    token_stream TOK;
    if( !TOK.OpenFile( pFileName ) )
        return FALSE;

    xtimer t;
    xtimer Time;

//#if defined(athyssen)
//#error From Bisc:Hey Andy, hope all went well! Here is some notes about this function!
    /*
    I was looking in to why this was taking 60+ seconds to calculate the pain tables. It
    all boiled down to the expansion of the PainHealthProfileArray. It was turning in to
    an n^2 problem as xarray growing was limited, by Craig, to 1 entry at a time (apparently
    to reduce the amount of wasted memory). I have used the number of health handles^2 
    as a guesstimate to the size of the final array. This dropped calculation time from 
    60+seconds to around 1.4 seconds. The guess is larger than the final array but not
    by too much.

    Couple of questions:

    1. Can we predict with greater accuracy the size of the final array?
    2. Can the inner loop of this code be sped up more?
    */
//#endif
    HealthHandle.SetCapacity( 100 );

    t.Start();
    Time.Start();

    // Read in the health_profile names
    s32 LN = TOK.GetLineNumber();

    while( 1 )
    {
        TOK.ReadString();
        if( TOK.GetLineNumber() != LN )
            break;

        // Load the health_handle with its name
        health_handle& HH = HealthHandle.Append();
        if( x_stricmp(TOK.String(),"")!=0 )
        {
            HH.SetName( TOK.String() );
        }
    }

    // This initial capacity should match the final capacity recorded in the logger.
    // If this matches or exceeds the logger value, we elimate any unnecessary
    // resizing of the xarray.
    PainHealthProfileArray.SetCapacity( 6785 );

    // Rewind and go to second line where the pain listing starts
    TOK.Rewind();
    TOK.SkipToNextLine();

    while( TOK.IsEOF() == FALSE )
    {
        TOK.ReadString();

        if( x_stricmp(TOK.String(),"")==0 )
        {
            TOK.SkipToNextLine();
            continue;
        }

        // Load the handle with its name
        pain_handle PainHandle( TOK.String() );

        // If the profile doesn't exist then move to next line
        const pain_profile* pPP = PainHandle.GetPainProfile();
        if( pPP == NULL )
        {
            TOK.SkipToNextLine();
            continue;
        }

        // Loop across reading in values for each health_handle
        s32 iHealthHandle=1;
        for( iHealthHandle=1; iHealthHandle<HealthHandle.GetCount(); iHealthHandle++ )
        {
            // If it's blank then just move on to next handle
            if( HealthHandle[iHealthHandle].GetDataID() == BlankHealthHandle.GetDataID() )
                continue;

            // Is this a valid health_handle?
            const health_profile* pHP = HealthHandle[iHealthHandle].GetHealthProfile();
            if( pHP == NULL )
                continue;

            // Build a pain_health_profile for this combination
            pain_health_handle PHHandle = PainHandle.BuildPainHealthProfileHandle( HealthHandle[iHealthHandle] );
            PainHealthProfile.SetDataID( PHHandle.GetDataID() );
            PainHealthProfile.m_Damage  = 0;
            PainHealthProfile.m_Force   = 0;
            PainHealthProfile.m_HitType = 0;

            // Append the new profile to the list
            PainHealthProfileArray.Append(PainHealthProfile);
        }

        // Fast forward to next line
        TOK.SkipToNextLine();
    }

    TOK.CloseFile();

    // Allocate final array
    pain_health_profile* pPHP = new pain_health_profile[ PainHealthProfileArray.GetCount() ];
    for( s32 i=0; i<PainHealthProfileArray.GetCount(); i++ )
        pPHP[i] = PainHealthProfileArray[i];

    // Hand off to the data vault.  ..."Keep them out of trouble!"...
    g_DataVault.AddDataBlocks("PH_PROFILE",pPHP,PainHealthProfileArray.GetCount(),sizeof(pain_health_profile));

    LOG_MESSAGE( "BuildPainHealthProfiles","PainHealthProfileArray Size : %d elements",PainHealthProfileArray.GetCount() );
    LOG_MESSAGE( "BuildPainHealthProfiles","HealthHandle Size : %d elements",HealthHandle.GetCount() );
    LOG_MESSAGE( "BuildPainHealthProfiles","End. Took %2.02f seconds.", Time.ReadSec() );
    
    
    return TRUE;
}

//==============================================================================

xbool FillPainHealthProfileData( const char* pFileName, s32 DataOffset, xbool bFloat )
{
    health_handle         BlankHealthHandle;
    xarray<health_handle> HealthHandle;
    
    // Open the file for reading
    token_stream TOK;
    if( !TOK.OpenFile( pFileName ) )
        return FALSE;

    // Read in the health_profile names
    s32 LN = TOK.GetLineNumber();
    xtimer t;
    xtimer Time;
    t.Start();
    Time.Start();
    while( 1 )
    {
        TOK.ReadString();
        if( TOK.GetLineNumber() != LN )
            break;

        // Load the health_handle with its name
        health_handle& HH = HealthHandle.Append();
        if( x_stricmp(TOK.String(),"")!=0 )
        {
            HH.SetName( TOK.String() );
        }
    }

    // Rewind and go to second line where the pain listing starts
    TOK.Rewind();
    TOK.SkipToNextLine();

    while( TOK.IsEOF() == FALSE )
    {
        TOK.ReadString();

        if( x_stricmp(TOK.String(),"")==0 )
        {
            TOK.SkipToNextLine();
            continue;
        }

        // Load the handle with its name
        pain_handle PainHandle( TOK.String() );

        // If the profile doesn't exist then move to next line
        const pain_profile* pPP = PainHandle.GetPainProfile();
        if( pPP == NULL )
        {
            TOK.SkipToNextLine();
            continue;
        }

        // Loop across reading in values for each health_handle
        s32 iHealthHandle=1;
        for( iHealthHandle=1; iHealthHandle<HealthHandle.GetCount(); iHealthHandle++ )
        {
            // Read value
            f32 V = TOK.ReadF32FromString();

            // If the healthhandle is blank then just move on to next handle
            if( HealthHandle[iHealthHandle].GetDataID() == BlankHealthHandle.GetDataID() )
                continue;

            // Is this a valid health_handle?
            const health_profile* pHP = HealthHandle[iHealthHandle].GetHealthProfile();
            if( pHP == NULL )
                continue;

            // Build a pain_health_profile for this combination
            pain_health_handle PHHandle = PainHandle.BuildPainHealthProfileHandle( HealthHandle[iHealthHandle] );
            pain_health_profile* pPHP = (pain_health_profile*)PHHandle.GetPainHealthProfile();
            if( pPHP )
            {
                if( bFloat )    ((f32*)(((byte*)pPHP) + DataOffset))[0] = (f32)V;
                else            ((s32*)(((byte*)pPHP) + DataOffset))[0] = (s32)V;
            }
        }

        // Fast forward to next line
        TOK.SkipToNextLine();
    }

    TOK.CloseFile();

    LOG_MESSAGE("FillPainHealthProfileData","Took %2.02fms to build profile data", Time.ReadSec() );
    return TRUE;
}

//==============================================================================

xbool LoadPainHealthProfiles( const char* pDirectory )
{
    // Go through damage table and build pain_health_profile entries
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        if( !BuildPainHealthProfiles(xfs("%s\\Tweak_MP_DamageTable.txt",pDirectory)) )
            return FALSE;
    }
    else
#endif
    {
        if( !BuildPainHealthProfiles(xfs("%s\\Tweak_DamageTable.txt",pDirectory)) )
            return FALSE;
    }



    pain_health_profile PHP;

#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        if( !FillPainHealthProfileData(xfs("%s\\Tweak_MP_DamageTable.txt",pDirectory),((u32)&PHP.m_Damage)-((u32)&PHP),TRUE) )
            return FALSE;
    }
    else
#endif
    {
        if( !FillPainHealthProfileData(xfs("%s\\Tweak_DamageTable.txt",pDirectory),((u32)&PHP.m_Damage)-((u32)&PHP),TRUE) )
            return FALSE;
    }
    
    if( !FillPainHealthProfileData(xfs("%s\\Tweak_ForceTable.txt",pDirectory),((u32)&PHP.m_Force)-((u32)&PHP),TRUE) )
        return FALSE;

    if( !FillPainHealthProfileData(xfs("%s\\Tweak_HitTypeTable.txt",pDirectory),((u32)&PHP.m_HitType)-((u32)&PHP),FALSE) )
        return FALSE;

    return TRUE;
}

//==============================================================================

xbool LoadPain( const char* pDirectory )
{
    if( !LoadPainProfiles( xfs("%s\\Tweak_PainProfile.txt",pDirectory) ) )
        return FALSE;
    
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        if( !LoadHealthProfiles( xfs("%s\\Tweak_MP_DamageTable.txt",pDirectory) ) )
            return FALSE;
    }     
    else 
#endif
    {
        if( !LoadHealthProfiles( xfs("%s\\Tweak_DamageTable.txt",pDirectory) ) )
            return FALSE;
    }

    if( !LoadPainHealthProfiles( pDirectory ) )
        return FALSE;

    return TRUE;
}

//==============================================================================

void UnloadPain( void )
{
    // Remove data from the vault
    pain_profile*        pPP  = (pain_profile*)g_DataVault.DelDataBlocks("P_PROFILE");
    health_profile*      pHP  = (health_profile*)g_DataVault.DelDataBlocks("H_PROFILE");
    pain_health_profile* pPHP = (pain_health_profile*)g_DataVault.DelDataBlocks("PH_PROFILE");

    // Deallocate the arrays
    delete[] pPP;
    delete[] pHP;
    delete[] pPHP;
}

//==============================================================================
