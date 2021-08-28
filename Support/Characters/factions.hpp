#ifndef __FACTIONS_HPP
#define __FACTIONS_HPP

#include "..\Auxiliary\MiscUtils\Property.hpp"
#include "..\Auxiliary\MiscUtils\PropertyEnum.hpp"

enum factions
{
    FACTION_NONE            = 0,
    FACTION_PLAYER_NORMAL   = BIT( 0 ),     //use for matrix only, no character will belong to this faction
    FACTION_PLAYER_STRAIN1  = BIT( 1 ),     //use for matrix only, no character will belong to this faction
    FACTION_PLAYER_STRAIN2  = BIT( 2 ),     //use for matrix only, no character will belong to this faction
    FACTION_PLAYER_STRAIN3  = BIT( 3 ),     //use for matrix only, no character will belong to this faction

    FACTION_NEUTRAL         = BIT( 7 ),     // always friendly
    FACTION_BLACK_OPS       = BIT( 8 ),
    FACTION_MILITARY        = BIT( 9 ),
    FACTION_MUTANTS_LESSER  = BIT( 10 ),
    FACTION_MUTANTS_GREATER = BIT( 11 ),
    FACTION_GRAY            = BIT( 12 ),
    FACTION_THETA           = BIT( 13 ),
    FACTION_WORKERS         = BIT( 14 ),

    FACTION_TEAM_ONE        = BIT( 15 ),
    FACTION_TEAM_TWO        = BIT( 16 ),
    FACTION_DEATHMATCH      = BIT( 17 ),

    FACTION_NOT_SET = 0xFFFFFFFF,
    
    MAX_FACTION_COUNT       = 18,

    INVALID_FACTION         = -1
} ;

class factions_manager
{
public:
    factions_manager      ( void );
    ~factions_manager     ( void );
    
    static enum_table<factions>    s_FactionList; 
    
    static         void        OnEnumProp( prop_enum& rList );
    static         xbool       OnProperty( prop_query& rPropQuery, factions& Faction, u32& Friends );

    static         void        OnEnumFaction( prop_enum& rList );
    static         void        OnEnumFriends( prop_enum& rList );

    static         xbool       OnPropertyFaction( prop_query& rPropQuery, factions& rFaction );
    static         xbool       OnPropertyFriends( prop_query& rPropQuery, u32& rFriends );

    static          s32        GetFactionBitIndex( const factions& rFaction );
};
    
#endif