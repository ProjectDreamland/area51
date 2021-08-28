//=========================================================================
//
//  GameState.hpp
//
//=========================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//=========================================================================

#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

enum game_state
{
    STATE_NULL,
    STATE_COMMON_INIT,
    STATE_COMMON_INIT_TITLES,
    STATE_COMMON_INIT_FRONTEND,
    STATE_COMMON_FRONTEND,
    
    // NOTE: If the states are altered, update sm_GetState() accordingly.
};

enum server_state
{
    STATE_SERVER_IDLE,
    STATE_SERVER_INIT,
    STATE_SERVER_LOAD_MISSION,
    STATE_SERVER_SYNC,
    STATE_SERVER_INGAME,
    STATE_SERVER_COOLDOWN,
    STATE_SERVER_SHUTDOWN,
    STATE_SERVER_KILL,
    
};

enum client_state
{
    STATE_CLIENT_IDLE,
        
    STATE_CLIENT_INIT,
    STATE_CLIENT_LOGIN,
    STATE_CLIENT_REQUEST_MISSION,
    STATE_CLIENT_LOAD_MISSION,
    STATE_CLIENT_VERIFY_MISSION,
    STATE_CLIENT_SYNC,

    STATE_CLIENT_INGAME,

    STATE_CLIENT_COOLDOWN,
    STATE_CLIENT_DISCONNECT,
    STATE_CLIENT_SHUTDOWN,

    STATE_CLIENT_KILL,
};

const char* GetStateName        (client_state State);
const char* GetStateName        (server_state State);

#endif

