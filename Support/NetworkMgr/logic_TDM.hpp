//==============================================================================
//
//  logic_TDM.hpp
//
//==============================================================================

#ifndef LOGIC_TDM_HPP
#define LOGIC_TDM_HPP	

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Base.hpp"

//==============================================================================
//	TYPES
//==============================================================================

class logic_tdm : public logic_base
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
   
public:
                logic_tdm        ( void );
virtual        ~logic_tdm        ( void );

virtual void    Connect         ( s32 PlayerIndex );
virtual void    Disconnect      ( s32 PlayerIndex );
virtual void    EnterGame       ( s32 PlayerIndex );
virtual void    ExitGame        ( s32 PlayerIndex );
virtual void    RequestSpawn    ( s32 PlayerIndex, xbool Immediate = FALSE );
virtual u32     GetTeamBits     ( s32 PlayerIndex );
virtual void    PlayerDied      ( s32 Victim, s32 Killer, s32 PainType );

virtual void    BeginGame       ( void );

/*
virtual f32     GetGameProgress ( void );
virtual void    PlayerDied      ( const pain& Pain );
*/

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------
   
protected:

virtual void    AdvanceTime     ( f32 DeltaTime );
virtual xbool   IsTeamBased     ( void );

/*
virtual void    AcceptUpdate    ( const bitstream& BitStream );
virtual void    ProvideUpdate   (       bitstream& BitStream, 
                                        u32&       DirtyBits, 
                                        u32&       ScoreBits, 
                                        u32&       PlayerBits );
*/
/*
virtual void    SpawnPlayer     ( s32 PlayerIndex );
virtual void    Initialize      ( void );

virtual void    Connect         ( s32 PlayerIndex );
virtual void    EnterGame       ( s32 PlayerIndex );
virtual void    ExitGame        ( s32 PlayerIndex );
virtual void    Disconnect      ( s32 PlayerIndex );

virtual void    ChangeTeam      ( s32 PlayerIndex );

virtual void    AdvanceTime     ( f32 DeltaTime );
virtual void    BeginGame       ( void );
virtual void    EndGame         ( void );

*/

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------
   
        f32     m_RespawnDelay  [32];
        xbool   m_Alive         [32];
};

//==============================================================================
#endif // LOGIC_TDM_HPP
//==============================================================================
