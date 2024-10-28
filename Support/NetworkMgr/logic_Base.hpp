//==============================================================================
//
//  logic_Base.hpp
//
//==============================================================================

#ifndef LOGIC_BASE_HPP
#define LOGIC_BASE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\PainMgr\Pain.hpp"
#include "NetLimits.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  TYPES
//==============================================================================

class game_mgr;
class bitstream;
class spawn_point;

//==============================================================================

class logic_base
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
   
public:

                logic_base          ( void );
virtual        ~logic_base          ( void );

virtual void    Activate            ( void );
virtual f32     GetGameProgress     ( void );
virtual void    PlayerDied          ( s32 Victim, s32 Killer, s32 PainType );
virtual void    RequestSpawn        ( s32 PlayerIndex, xbool Immediate = FALSE );
virtual u32     GetTeamBits         ( s32 PlayerIndex );
virtual void    SetTeam             ( s32 PlayerIndex, s32 Team );
virtual s32     GetTeam             ( s32 PlayerIndex );
virtual void    SetMutationRights   ( s32 PlayerIndex );
virtual void    DropFlag            ( s32 PlayerIndex );
virtual s32     BitsToTeam          ( u32 TeamBits );
virtual void    FlagTouched         ( s32 PlayerIndex, s32 FlagNetSlot );
virtual void    FlagTimedOut        (                  s32 FlagNetSlot );
virtual void    CircuitChanged      ( s32 Circuit, u32 TeamBits );
virtual void    SetPlayerSpawnInfo  ( const vector3& Position, 
                                            radian   Pitch, 
                                            radian   Yaw, 
                                            s32      Zone, 
                                      const guid&    Guid );

/*                
virtual xbool   RequestNewTeam      ( s32 PlayerIndex );

virtual void    RegisterFlag    ( const vector3& Position, radian Yaw, s32 FlagTeam );
virtual void    RegisterItem    ( object::id ItemID, const char* pTag );

virtual void    PlayerOnVehicle ( object::id PlayerID, object::id VehicleID );
virtual void    ItemDisabled    ( object::id ItemID,   object::id OriginID  );   
virtual void    ItemDestroyed   ( object::id ItemID,   object::id OriginID  );
virtual void    ItemEnabled     ( object::id ItemID,   object::id PlayerID  );
virtual void    ItemDeployed    ( object::id ItemID,   object::id PlayerID  );
virtual void    ItemAutoLocked  ( object::id ItemID,   object::id PlayerID  );
virtual void    ItemRepairing   ( object::id ItemID,   object::id PlayerID  ); 
virtual void    ItemRepaired    ( object::id ItemID,   object::id PlayerID, xbool Score );
virtual void    PickupTouched   ( object::id PickupID, object::id PlayerID  );
virtual void    FlagTouched     ( object::id FlagID,   object::id PlayerID  );
virtual void    FlagHitPlayer   ( object::id FlagID,   object::id PlayerID  );
virtual void    FlagTimedOut    ( object::id FlagID   );
virtual void    NexusTouched    ( object::id PlayerID );
virtual void    ThrowFlags      ( object::id PlayerID );
virtual void    StationUsed     ( object::id PlayerID );
virtual void    WeaponFired     ( object::id PlayerID = obj_mgr::NullID );
virtual void    WeaponExchanged ( object::id PlayerID );
virtual void    SatchelDetonated( object::id PlayerID );

virtual void    GetFlagStatus   ( u32 TeamBits, s32& Status, vector3& Position );

virtual void    Render          ( void );
*/

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------
   
protected:

virtual void    Initialize          ( void );

virtual void    Connect             ( s32 PlayerIndex );
virtual void    EnterGame           ( s32 PlayerIndex );
virtual void    ExitGame            ( s32 PlayerIndex );
virtual void    Disconnect          ( s32 PlayerIndex );

virtual void    ChangeTeam          ( s32 PlayerIndex );

virtual xbool   IsTeamBased         ( void );
virtual u32     GetScoreFields      ( void );

virtual void    BeginGame           ( void );
virtual void    EndGame             ( xbool Complete );
virtual void    AdvanceTime         ( f32 DeltaTime );

virtual void    UpdateMusic         ( void );

        void    Teamify             ( void );
        void    Unteamify           ( void );
        void    MoveToSpawnPt       ( s32 PlayerIndex, spawn_point& SpawnPt );

        void    NonTeamScoreMsgPrep ( void );
        void    NonTeamScoreMsgExec ( s32 ScoringPlayer = -1 );

/*
virtual void    AcceptUpdate    ( const bitstream& BitStream );
virtual void    ProvideUpdate   (       bitstream& BitStream, 
                                        u32&       DirtyBits, 
                                        u32&       ScoreBits, 
                                        u32&       PlayerBits );
virtual u32     GetAllDirtyBits ( void );
*/

/*
virtual void    EnforceBounds   ( f32 DeltaTime );

virtual void    SwitchTouched   ( object::id SwitchID, object::id PlayerID  );

    s32         IndexFromID     ( object::id PlayerID );
    object::id  IDFromIndex     ( s32 PlayerIndex );
    s32         BitsToTeam      ( u32 TeamBits );

    xbool       IsDefending     ( object::type  Type, 
                                  object::id    PlayerID, 
                                  object::id    OriginID );
*/

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------

        u32     m_LeadBefore;       // Mask   of players in 1st before updates.
        s32     m_NLeadBefore;      // Number of players in 1st before updates.

//------------------------------------------------------------------------------
//  Friends
//------------------------------------------------------------------------------

    friend game_mgr;
};

//==============================================================================
#endif // LOGIC_BASE_HPP
//==============================================================================
