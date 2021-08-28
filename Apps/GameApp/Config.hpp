//==============================================================================
//
//  Config.hpp
//
//=============================================================================

#ifndef CONFIG_HPP
#define CONFIG_HPP

//=============================================================================
//  INCLUDES
//=============================================================================

#include "x_types.hpp"

#ifndef CONFIG_RETAIL

//=============================================================================
//  TYPES
//=============================================================================

struct config_options
{
    xbool       DemoBuild;
    xbool       AutoCampaign;
    xbool       AutoSplitScreen;
    xbool       AutoServer;
    xbool       AutoClient;
    s32         AutoLevel;              // If AutoCampaign or AutoServer.
    s32         AutoServerType;         // Only used if AutoServer.  GameMgr.hpp.
    s32         AutoMutateMode;         // Only used if AutoServer.  GameMgr.hpp.
    s32         AutoMonkeyMode;         // Used for specifying monkey settings on startup
    char        AutoServerName[ 32 ];   // Only used if AutoServer/Client.

    // Monkey options.
    // InfiniteAmmo.
    // Controller configuration.
    // Force fake players in scoreboard.
    // Force bots into game.
    // What else might go in here?

                config_options      ( void );
               ~config_options      ( void );
    void        Load                ( const char* pFileName );
};

//=============================================================================
//  STORAGE
//=============================================================================

extern config_options   g_Config;

#define CONFIG_IS_DEMO              FALSE
#define CONFIG_IS_AUTOCAMPAIGN      g_Config.AutoCampaign
#define CONFIG_IS_AUTOSPLITSCREEN   g_Config.AutoSplitScreen
#define CONFIG_IS_AUTOSERVER        g_Config.AutoServer
#define CONFIG_IS_AUTOCLIENT        g_Config.AutoClient

#else // CONFIG_RETAIL

//=============================================================================

#define CONFIG_IS_DEMO              0
#define CONFIG_IS_AUTOCAMPAIGN      0
#define CONFIG_IS_AUTOSPLITSCREEN   0
#define CONFIG_IS_AUTOSERVER        0
#define CONFIG_IS_AUTOCLIENT        0

//=============================================================================

#endif // CONFIG_RETAIL

//=============================================================================
#endif // CONFIG_HPP
//=============================================================================
