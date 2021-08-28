//==============================================================================
//
//  logic_Infect.hpp
//
//==============================================================================

#ifndef LOGIC_INFECT_HPP
#define LOGIC_INFECT_HPP

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Base.hpp"

//==============================================================================
//	TYPES
//==============================================================================

class logic_infect : public logic_base
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
   
public:
                logic_infect    ( void );
virtual        ~logic_infect    ( void );

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

        void    EnterPreGame    ( void );
        void    EnterParanoia   ( void );
        void    EnterInGame     ( void );

        void    LogicPreGame    ( f32 DeltaTime );
        void    LogicParanoia   ( f32 DeltaTime );
        void    LogicInGame     ( f32 DeltaTime );

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
        xbool   m_Infected      [32];
        s32     m_NInfected;
        s32     m_NNormal;
        s32     m_NMutantDeaths;    // Mutant deaths since last human killed.
        f32     m_Timer;
        s32     m_LastRandom;
        s32     m_PatientZero;
        s32     m_State;    // 0 = PREGAME - 1 = PARANOIA - 2 = INGAME
};

//==============================================================================
#endif // LOGIC_INFECT_HPP
//==============================================================================
