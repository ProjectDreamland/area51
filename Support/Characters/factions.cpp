//factions.cpp
#include "Factions.hpp"


//=========================================================================
// ENUMS
//=========================================================================

typedef enum_pair<factions> factions_enum_pair;

static factions_enum_pair s_lFList[] = 
{
    factions_enum_pair("PlayerNormal",                  FACTION_PLAYER_NORMAL),
    factions_enum_pair("PlayerStrain1",                 FACTION_PLAYER_STRAIN1),
    factions_enum_pair("PlayerStrain2",                 FACTION_PLAYER_STRAIN2),
    factions_enum_pair("PlayerStrain3",                 FACTION_PLAYER_STRAIN3),
    factions_enum_pair("Neutral",                       FACTION_NEUTRAL),
    factions_enum_pair("BlackOps",                      FACTION_BLACK_OPS),
    factions_enum_pair("Military",                      FACTION_MILITARY),
    factions_enum_pair("MutantsLesser",                 FACTION_MUTANTS_LESSER),
    factions_enum_pair("MutantsGreater",                FACTION_MUTANTS_GREATER),
    factions_enum_pair("Gray",                          FACTION_GRAY),
    factions_enum_pair("Theta",                         FACTION_THETA),
    factions_enum_pair("Workers",                       FACTION_WORKERS),
    factions_enum_pair("Team 1",                        FACTION_TEAM_ONE),
    factions_enum_pair("Team 2",                        FACTION_TEAM_TWO),
    factions_enum_pair("Deathmatch",                    FACTION_DEATHMATCH),
    factions_enum_pair( k_EnumEndStringConst,           INVALID_FACTION) //**MUST BE LAST**//
};

enum_table<factions>  factions_manager::s_FactionList( s_lFList );              

//=========================================================================
// FUNCTIONS
//=========================================================================

//==============================================================================

factions_manager::factions_manager(void)
{
}

//==============================================================================

factions_manager::~factions_manager(void)
{
}

//===========================================================================

void factions_manager::OnEnumProp( prop_enum& rList )
{
    OnEnumFaction( rList );
    OnEnumFriends( rList );
}

//===========================================================================

void factions_manager::OnEnumFaction( prop_enum& rList )
{
    rList.PropEnumEnum  ("Faction", factions_manager::s_FactionList.BuildString(), "Which faction does this character belong to.", PROP_TYPE_EXPOSE );
}

//===========================================================================

void factions_manager::OnEnumFriends( prop_enum& rList )
{
    rList.PropEnumHeader("FriendlyFactions", "List of friendly factions.", 0 );
    s32 iHeader = rList.PushPath( "FriendlyFactions\\" );        

    for (s32 i = 0; i < (factions_manager::s_FactionList.GetCount()-1); i++)
    {
        rList.PropEnumBool(factions_manager::s_FactionList.GetStringFromIndex(i), "Are you friendly towards this faction?", PROP_TYPE_EXPOSE );
    }

    rList.PopPath( iHeader );
}

//===========================================================================

xbool factions_manager::OnProperty( prop_query& rPropQuery, factions& Faction, u32& Friends )
{
    if ( OnPropertyFaction( rPropQuery, Faction ) )
    {
        return TRUE;
    }

    if ( OnPropertyFriends( rPropQuery, Friends ) )
    {
        return TRUE;
    }

    return FALSE;
}

//===========================================================================

xbool factions_manager::OnPropertyFaction( prop_query& rPropQuery, factions& Faction )
{
    if ( rPropQuery.IsVar( "Faction") )
    {
        if( rPropQuery.IsRead() )
        {
            if ( factions_manager::s_FactionList.DoesValueExist( Faction ) )
            {
                rPropQuery.SetVarEnum( factions_manager::s_FactionList.GetString( Faction ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            factions TempFaction;

            if( factions_manager::s_FactionList.GetValue( rPropQuery.GetVarEnum(), TempFaction ) )
            {
                Faction = TempFaction;
            }
        }
        
        return TRUE;
    }

    return FALSE;

}

//===========================================================================

xbool factions_manager::OnPropertyFriends( prop_query& rPropQuery, u32& Friends )
{
    if( !rPropQuery.IsBasePath("FriendlyFactions") )
        return FALSE;

    s32 iHeader = rPropQuery.PushPath( "FriendlyFactions\\" );        
    
    for (s32 i = 0; i < (factions_manager::s_FactionList.GetCount() -1); i++)
    {
        if (rPropQuery.IsVar(factions_manager::s_FactionList.GetStringFromIndex(i)))
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarBool(Friends & factions_manager::s_FactionList.GetValueFromIndex(i));
            }
            else
            {
                if (rPropQuery.GetVarBool())
                {
                    Friends = (factions)(Friends | (factions)factions_manager::s_FactionList.GetValueFromIndex(i));
                }
                else
                {
                    Friends = (factions)(Friends & ~(factions)factions_manager::s_FactionList.GetValueFromIndex(i));
                }
            }

            return TRUE;
        }
    } 

    rPropQuery.PopPath( iHeader );

    return FALSE;
}

//===========================================================================

s32 factions_manager::GetFactionBitIndex( const factions& rFaction )
{
    if( rFaction == FACTION_NONE )
    {
    //  TODO - The following assert is tripping up multiplayer.
    //         Removed for now.  Should be reconsidered and restored later.
    //  ASSERTS( 0, "No factions set" );
        return 0;
    }

    s32 Index       = 0;
    u32 iFaction    = (u32)rFaction;
    while( iFaction != 1 )
    {
        iFaction = iFaction>>1;
        Index++;
    }
    
    if( Index > MAX_FACTION_COUNT )
    {
        ASSERTS( 0, "Incorrect faction value" );
        return 0;
    }
    
    return Index;
}

//===========================================================================