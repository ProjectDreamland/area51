//=========================================================================
//
//  PlayerProfile.cpp
//
//=========================================================================

#include "PlayerProfile.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "MemCardMgr/MemCardMgr.hpp"

//=========================================================================
//  Player Profile Functions
//=========================================================================

player_profile::player_profile( void )
{
    Reset();
}

void player_profile::Reset( void )
{
    s32 i=0;

    x_memset( this, 0, sizeof(this) );

    if( CONFIG_IS_AUTOSERVER )
    {
        x_strcpy( m_pProfileName, "Server" );
    }
    if( CONFIG_IS_AUTOCLIENT )
    {
        x_strcpy( m_pProfileName, "Client" );
    }

    m_Version               = PROFILE_VERSION_NUMBER;
    m_HashString            = 0;
    m_AvatarID              = 0;
    m_Sensitivity[0]        = 50;
    m_Sensitivity[1]        = 50;
    m_LoreTotal             = 0;
    m_bNewLoreUnlocked      = FALSE;
    m_NumSecretsUnlocked    = 0;
    m_bNewSecretUnlocked    = FALSE;
    m_bInvertY              = TRUE;
    m_bVibration            = TRUE;
    m_bCrouchOn             = FALSE;
    m_bLookspringOn         = FALSE;
    m_bIsVisibleOnline      = TRUE;
    m_bAutosaveOn           = FALSE;
    m_bAlienAvatarsOn       = FALSE;
    m_UniqueIdLength        = 0;
    m_CinemaMutatedMsgCount = 3;
    m_bIsMutated            = FALSE;
    m_DifficultyLevel       = 1;        // medium difficulty by default
    m_bWeaponAutoSwitch     = TRUE;     // if on/true, will auto-switch to a weapon with a > rating (default is TRUE)
    m_bSecretAwarded        = FALSE;
    m_bHardUnlocked         = FALSE;
    m_bDifficultyChanged    = FALSE;
    m_bGameFinished         = FALSE;
#ifdef TARGET_XBOX
    // always set to true on xbox because if the
    // player has a Live account, age is verified!
    m_bAgeVerified          = TRUE;
#else
    m_bAgeVerified          = FALSE;
#endif

    // Clear lore collected
    for( i=0; i<NUM_VAULTS; i++ )
    {
        m_Lore[i] = 0;
    }

    // Mark all checkpoints as having invalid level id, this will make it
    // available for use.
    for( i=0; i<MAX_SAVED_LEVELS; i++ )
    {
        m_Checkpoints[i].Init( -1 );
    }
}

//=========================================================================

void player_profile::SetHash( void )
{
    xstring TempString ( m_pProfileName );
    const char* pString     = TempString;
    u32 Hash                = 5381;

    // get the time and date
    datestamp Time = eng_GetDate();

    // convert the time to a string and add it to the hashstring
    TempString += xfs( "%d", Time );

    // Process each character to generate the hash key
    while( *pString )
    {
        Hash = (Hash * 33) ^ *pString++;
    }

    m_HashString = Hash;

}

//=========================================================================

void player_profile::RestoreControlDefaults( void )
{
    m_Sensitivity[0]    = 50;
    m_Sensitivity[1]    = 50;
    m_bLookspringOn     = FALSE;
    m_bCrouchOn         = FALSE;
    m_bInvertY          = TRUE;
    m_bVibration        = TRUE;
    m_bWeaponAutoSwitch = TRUE;
}

//=========================================================================

void player_profile::SetProfileName( const char* pProfileName )
{
    ASSERT( x_strlen( pProfileName ) < SM_PROFILE_NAME_LENGTH );
    x_strcpy( m_pProfileName, pProfileName );
}


//=========================================================================

xbool player_profile::GetLoreAcquired( u32 Vault, s32 Index )
{
    // check if this is a general inquiry
    if( Index == -1 )
    {
        return ( m_Lore[Vault] != 0 );
    }

    // return specific lore acquired
    return ( m_Lore[Vault] & (1<<Index) );
}

//=========================================================================

void player_profile::SetLoreAcquired( u32 Vault, u32 Index )
{
    // range checks
    ASSERT( Vault<NUM_VAULTS    );
    ASSERT( Index<NUM_PER_VAULT );

    // make sure we're not already acquired
    if( !GetLoreAcquired(Vault,Index) )
    {
        m_Lore[Vault] += (1<<Index);
        m_LoreTotal++;
        m_bNewLoreUnlocked = TRUE;
    }

    // check for unlocking secrets
    if( (m_LoreTotal % 5) == 0 )
    {
        if( m_LoreTotal == 90 )
        {
            m_NumSecretsUnlocked = 21;
        }
        else
        {
            m_NumSecretsUnlocked++;
        }
        m_bNewSecretUnlocked = TRUE;
    }

#if !defined(X_EDITOR)
    // Auto save 
    g_StateMgr.SilentSaveProfile();
#endif
}

//=========================================================================
void player_profile::AcquireSecret( void )
{
    // used to award a secret after deep underground
    if( !m_bSecretAwarded )
    {
        m_NumSecretsUnlocked++;
        m_bSecretAwarded     = TRUE;
        m_bNewSecretUnlocked = TRUE;
    }
}

//=========================================================================

level_check_points* player_profile::GetCheckpointByMapID( s32 MapID )
{
    for( s32 i=0; i<MAX_SAVED_LEVELS; i++ )
    {
        level_check_points* pCheckpoint = &m_Checkpoints[i];

        // Did we find one the same as the current map id? If so, bail out.
        if( pCheckpoint->MapID == MapID )
        {
            // found it!
            return pCheckpoint;
        }
        else if( pCheckpoint->MapID == -1 )
        {
            // no checkpoint for this MapID
            return NULL;
        }
    }
    // ran out of checkpoints!
    ASSERT( FALSE );
    return NULL;
}

//=========================================================================

void player_profile::Checksum( void )
{
    m_Checksum = 0;
    m_Checksum = x_chksum( this, sizeof(player_profile) );
}

//=========================================================================

xbool player_profile::Validate( void )
{
    s32 DesiredChecksum;
    s32 ActualChecksum;
    
    DesiredChecksum = m_Checksum;
    m_Checksum = 0;
    ActualChecksum = x_chksum( this, sizeof(player_profile) );
    m_Checksum = DesiredChecksum;
    return (ActualChecksum == DesiredChecksum);
}

//=========================================================================
// This is just a changed name for the validate function. We may put some
// additional checks to see if this profile has been modified.
xbool player_profile::HasChanged( void )
{
    return Validate()==FALSE;
}

//=========================================================================
void player_profile::MarkDirty( void )
{
    m_Checksum = 0;
}

//=========================================================================

void player_profile::SetUniqueId( const byte* pUniqueId, s32 Length )
{
    x_memset( m_UniqueId, 0, sizeof(m_UniqueId) );
    if( Length >= sizeof(m_UniqueId) )
    {
        Length = sizeof(m_UniqueId)-1;
    }
    x_memcpy( m_UniqueId, pUniqueId, Length );
    m_UniqueIdLength = Length;
}

//=========================================================================

const byte* player_profile::GetUniqueId( s32& Length )
{
    Length = m_UniqueIdLength;
    return m_UniqueId;
}

//=========================================================================

xbool player_profile::DisplayCinemaMutatedMsg( void )
{ 
    if( m_CinemaMutatedMsgCount > 0 ) 
    {
        m_CinemaMutatedMsgCount--; 
        return TRUE;
    }

    return FALSE;
}

//=========================================================================
#ifndef CONFIG_RETAIL
void player_profile::UnlockAll( void )
{
    // this function will unlock everything EXCEPT the checkpoints
    
    // unlock all lore 
    for( s32 i=0; i<NUM_VAULTS; i++ )
    {
        // check for deep underground - no lore for this level
        if( i!=1 )
        {
            // unlock all lore in vault
            m_Lore[i] = 31;
        }
    }
    m_LoreTotal             = 90;
    m_bNewLoreUnlocked      = TRUE;
    
    // unlock all secrets
    m_NumSecretsUnlocked    = 21;
    m_bNewSecretUnlocked    = TRUE;
    m_bSecretAwarded        = TRUE;

    // unlock hard 
    m_bHardUnlocked         = TRUE;
    // unlock alien avatars
    m_bAlienAvatarsOn       = TRUE;
    // unlock end game movie
    m_bGameFinished         = TRUE;
}
#endif
