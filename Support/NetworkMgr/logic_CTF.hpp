//==============================================================================
//
//  logic_CTF.hpp
//
//==============================================================================

#ifndef LOGIC_CTF_HPP
#define LOGIC_CTF_HPP	

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Base.hpp"

//==============================================================================
//	TYPES
//==============================================================================

class logic_ctf : public logic_base
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
   
public:
                logic_ctf        ( void );
virtual        ~logic_ctf        ( void );

virtual void    Connect         ( s32 PlayerIndex );
virtual void    Disconnect      ( s32 PlayerIndex );
virtual void    EnterGame       ( s32 PlayerIndex );
virtual void    ExitGame        ( s32 PlayerIndex );
virtual void    RequestSpawn    ( s32 PlayerIndex, xbool Immediate = FALSE );
virtual u32     GetTeamBits     ( s32 PlayerIndex );
virtual void    DropFlag        ( s32 PlayerIndex );
virtual void    PlayerDied      ( s32 Victim, s32 Killer, s32 PainType );
virtual void    FlagTouched     ( s32 PlayerIndex, s32 FlagNetSlot );
virtual void    FlagTimedOut    (                  s32 FlagNetSlot );

virtual void    BeginGame       ( void );
virtual void    EndGame         ( void );

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

        void    DropFlag        (       s32      FlagIndex,
                                        s32      PlayerIndex,
                                  const vector3& Position,
                                        radian   Yaw,
                                        s32      Zone1,
                                        s32      Zone2 );

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------
   
        f32         m_RespawnDelay  [32];
        xbool       m_Alive         [32];

        xbool       m_FlagSecure    [ 2];
        xbool       m_FlagDropped   [ 2];
        s32         m_FlagCarrier   [ 2];
        s32         m_FlagNetSlot   [ 2];
        vector3     m_FlagBase      [ 2];
        radian      m_FlagYaw       [ 2];
        u32         m_FlagZones     [ 2];
};

//==============================================================================
#endif // LOGIC_CTF_HPP
//==============================================================================
