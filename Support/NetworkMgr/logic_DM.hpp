//==============================================================================
//
//  logic_DM.hpp
//
//==============================================================================

#ifndef LOGIC_DM_HPP
#define LOGIC_DM_HPP	

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Base.hpp"

//==============================================================================
//	TYPES
//==============================================================================

class logic_dm : public logic_base
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
   
public:
                logic_dm        ( void );
virtual        ~logic_dm        ( void );

virtual void    Connect         ( s32 PlayerIndex );
virtual void    EnterGame       ( s32 PlayerIndex );
virtual void    ExitGame        ( s32 PlayerIndex );
virtual void    RequestSpawn    ( s32 PlayerIndex, xbool Immediate = FALSE );
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

/*
virtual void    Initialize      ( void );

virtual void    Connect         ( s32 PlayerIndex );
virtual void    EnterGame       ( s32 PlayerIndex );
virtual void    ExitGame        ( s32 PlayerIndex );
virtual void    Disconnect      ( s32 PlayerIndex );

virtual void    ChangeTeam      ( s32 PlayerIndex );

virtual void    AdvanceTime     ( f32 DeltaTime );
virtual void    BeginGame       ( void );
virtual void    EndGame         ( void );

virtual void    AcceptUpdate    ( const bitstream& BitStream );
virtual void    ProvideUpdate   (       bitstream& BitStream, u32& DirtyBits );
*/

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------
   
        f32     m_RespawnDelay  [32];
        xbool   m_Alive         [32];
};

//==============================================================================
#endif // LOGIC_DM_HPP
//==============================================================================
