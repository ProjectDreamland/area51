//==============================================================================
//
//  logic_Campaign.hpp
//
//==============================================================================

#ifndef LOGIC_CAMPAIGN_HPP
#define LOGIC_CAMPAIGN_HPP	

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Base.hpp"

//==============================================================================
//	TYPES
//==============================================================================

class logic_campaign : public logic_base
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
   
public:
                logic_campaign      ( void );
virtual        ~logic_campaign      ( void );

virtual void    Activate            ( void );
virtual void    PlayerDied          ( s32 Victim, s32 Killer, s32 PainType );
virtual void    RequestSpawn        ( s32 PlayerIndex, xbool Immediate = FALSE );
virtual void    SetPlayerSpawnInfo  ( const vector3& Position, 
                                            radian   Pitch, 
                                            radian   Yaw, 
                                            s32      Zone, 
                                      const guid&    Guid );

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------
   
protected:

virtual void    AdvanceTime     ( f32 DeltaTime );
virtual void    BeginGame       ( void );

/*
virtual void    Initialize      ( void );

virtual void    Connect         ( s32 PlayerIndex );
virtual void    EnterGame       ( s32 PlayerIndex );
virtual void    ExitGame        ( s32 PlayerIndex );
virtual void    Disconnect      ( s32 PlayerIndex );

virtual void    ChangeTeam      ( s32 PlayerIndex );

virtual void    EndGame         ( void );

virtual void    AcceptUpdate    ( const bitstream& BitStream );
virtual void    ProvideUpdate   (       bitstream& BitStream, u32& DirtyBits );
*/

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------
   
        f32     m_RespawnDelay;
        xbool   m_Alive;

        xbool   m_bSpawnInfoSet;
        vector3 m_PlayerSpawnPosition;
        radian  m_PlayerSpawnPitch;
        radian  m_PlayerSpawnYaw;
        s32     m_PlayerSpawnZone;
        guid    m_PlayerSpawnGuid;
};

//==============================================================================
#endif // LOGIC_CAMPAIGN_HPP
//==============================================================================
