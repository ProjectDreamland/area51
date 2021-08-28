
//==============================================================================
//  
//  MapList.hpp
//  
//==============================================================================
// The format of the map list manifest file is as below. The Parse() function will
// build an internal manifest list from a file passed to it and Serialize will
// produce an xstring of the properly laid out format.
// The file format takes the following form:
/*
    [Version]
    1149

    [GameType]
    0, "TDM", "Team Deathmatch",  "Team based deathmatch game"
                                    "Kill the guys on the other team";
    [Map]
    4010, "tinymp.gz", "Tiny MP downloaded", "This is the Tiny MP Description"
                                            "downloaded from the internet";

    [Play]
    4010, 0, 1, 16;
*/

//
// [Version] -  Specifies the version of this file. This is used during a content
//              check to see if the user needs to be informed of new additional maps.
// [GameType] - This defines what a specific game type index means. We have, by default
//              used -2 to 5 for built in gametypes but we can define new ones as 
//              necessary.
// [Map] -      Describes a specific map. Can have comments as to it's special features.
// [Play] -     Binds a game type, map description and minimum, maximum players for this
//              particular play mode.
//==============================================================================
//==============================================================================
//==============================================================================
#ifndef MAPLIST_HPP
#define MAPLIST_HPP

#include "x_files/x_types.hpp"
#include "NetworkMgr/GameMgr.hpp"

//=========================================================================
//  Defines
//=========================================================================

#define MAP_TABLE_SIZE      256
#define MAX_GAME_TYPES      16
#define MAX_MAPS            80

enum map_flags
{
    MF_DVD_MAP      = 0x00000001,       // this map is on the DVD
    MF_DOWNLOAD_MAP = 0x00000002,       // this map was downloaded
    MF_WIRE_MAP     = 0x00000004,       // this is a wire map
    MF_NOT_PRESENT  = 0,
};

//=========================================================================

// NOTE: These MUST match the mapping in the language mapping files (there is a set for each platform):
//       C:\GameData\A51\Release\PS2\ENG_DiskMaps.txt
//       C:\GameData\A51\Release\PS2\FRE_DiskMaps.txt
//       C:\GameData\A51\Release\PS2\GER_DiskMaps.txt
//       C:\GameData\A51\Release\PS2\ITA_DiskMaps.txt
//       C:\GameData\A51\Release\PS2\SPA_DiskMaps.txt
enum level_ids
{
    LEVELID_WELCOME_TO_DREAMLAND = 1000,
    LEVELID_DEEP_UNDERGROUND     = 1010,
    LEVELID_THE_HOT_ZONE         = 1020,
    LEVELID_THE_SEARCH           = 1030,
    LEVELID_THEY_GET_BIGGER      = 1040,
    LEVELID_THE_LAST_STAND       = 1050,
    LEVELID_ONE_OF_THEM          = 1060,
    LEVELID_INTERNAL_CHANGES     = 1070,
    LEVELID_LIFE_OR_DEATH        = 1075,
    LEVELID_DR_CRAY              = 1080,
    LEVELID_HATCHING_PARASITES   = 1090,
    LEVELID_PROJECT_BLUE_BOOK    = 1095,
    LEVELID_LIES_OF_THE_PAST     = 1100,
    LEVELID_BURIED_SECRETS       = 1105,
    LEVELID_NOW_BOARDING         = 1110,
    LEVELID_THE_GRAYS            = 1115,
    LEVELID_DESCENT              = 1120,
    LEVELID_THE_ASCENSION        = 1125,
    LEVELID_THE_LAST_EXIT        = 1130,
};

//=========================================================================

//=========================================================================
//  Structs                                                                
//=========================================================================
// Note: These structures will be dynamically sized so we're not doing tons
// of allocations.
class game_type_info
{
public:
                            game_type_info      ( void );
                           ~game_type_info      ( void );
    game_type               Type;               // GameType
    xstring                 ShortTypeName;     // Pointer to short game type for display
    xstring                 TypeName;          // Pointer to long game type for display
    xstring                 Rules;             // Pointer to rules for that game type.
    xbool                   operator==          ( const game_type_info& Right ) const;
};

class map_info
{
public:
                            map_info( void );
                           ~map_info( void );
    xbool                   IsAvailable         ( void );
    s32                     MapID;              // Unique map id (-1 for end of table)
    xstring                 Filename;           // Pointer to dfs file containing map
    xstring                 DisplayName;        // Pointer to display name
    xstring                 Description;        // Pointer to description of map
    s32                     Length;             // Length of the map when stored on the content server
    map_flags               Flags;              // map flags (see above)
    xbool                   operator==          ( const map_info& Right ) const;
};

class map_entry
{
public:
    s32                     GetMapID            ( void ) const;
    const char*             GetFilename         ( void ) const;
    const char*             GetDisplayName      ( void ) const;
    const char*             GetDescription      ( void ) const;
    map_flags               GetFlags            ( void ) const;
    game_type               GetGameType         ( void ) const;
    const char*             GetShortGameTypeName( void ) const;
    const char*             GetGameTypeName     ( void ) const;
    const char*             GetGameRules        ( void ) const;
    s32                     GetMaxPlayers       ( void ) const                      { return m_MaxPlayers;                      }
    s32                     GetMinPlayers       ( void ) const                      { return m_MinPlayers;                      }
    xbool                   IsAvailable         ( void ) const;
    s32                     GetLocation         ( void ) const                      { return m_Location;                        }
    void                    SetLocation         ( s32 Location )                    { m_Location = Location;                    }

    xbool                   operator==          ( const map_entry& MapEntry ) const;
    xbool                   operator!=          ( const map_entry& MapEntry ) const;

private:
    game_type               m_GameTypeID;
    s32                     m_MapID;
    s32                     m_MinPlayers;       // min players for mode/zoning
    s32                     m_MaxPlayers;       // max players for map in mode
    s32                     m_Location;

    friend class map_list;
};

//=========================================================================
//  Globals                                                                
//=========================================================================
class map_list
{
public:
                            map_list            ( void );
                           ~map_list            ( void );
    void                    Init                ( void );
    void                    LoadDefault         ( void );
    void                    Kill                ( void );
    void                    Clear               ( void );
    void                    Parse               ( const char* pMapFile, map_flags Flags, s32 Location );
    const map_entry*        Find                ( s32 MapID, s32 GameType=-1 );
    s32                     GetVersion          ( void );
    void                    SetVersion          ( s32 Version );
    const char*             GetDisplayName      ( s32 MapID );
    const char*             GetFileName         ( s32 MapID );
    map_entry*              GetByIndex          ( s32 MapIndex );
    map_entry*              GetNextMap          ( const map_entry* pCurrent );
    xbool                   Append              ( const map_entry& Entry, const map_list* pSourceMapList = NULL );
    s32                     GetGameTypeCount    ( void ) const                  { return m_GameTypes.GetCount();                }
    const game_type_info*   GetGameTypeInfo     ( s32 GameTypeID ) const;
    s32                     GetMapCount         ( void ) const                  { return m_Maps.GetCount();                     }
    const map_info*         GetMapInfo          ( s32 MapID ) const;
    void                    RemoveByFlags       ( map_flags Flags );
    void                    RemoveByMapID       ( s32 MapID );
    s32                     GetCount            ( void ) const                  { return m_MapList.GetCount();                  }
    s32                     Find                ( const map_entry& Entry ) const{ return m_MapList.Find(Entry);                 }
    xstring                 Serialize           ( void ) const;
    map_entry&              operator[]          ( s32 Index ) const             { return m_MapList[Index];                      }

private:
    xbool                   IsPresent           ( const char* pFilename );
    s32                     m_Version;
    xarray<map_entry>       m_MapList;
    xarray<map_info>        m_Maps;
    xarray<game_type_info>  m_GameTypes;
    friend class map_entry;
};

//==============================================================================
//  functions
//==============================================================================
extern map_list g_MapList;
//==============================================================================
#endif // MAPLIST_HPP
//==============================================================================
