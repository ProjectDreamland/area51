//==============================================================================
//
//  GameState.cpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#include "GameState.hpp"

//------------------------------------------------------------------------------
const char* GetStateName(client_state State)
{
    switch (State)
    {
    case STATE_CLIENT_INIT:             return "STATE_CLIENT_INIT";
    case STATE_CLIENT_LOGIN:            return "STATE_CLIENT_LOGIN";
    case STATE_CLIENT_REQUEST_MISSION:  return "STATE_CLIENT_REQUEST_MISSION";
    case STATE_CLIENT_LOAD_MISSION:     return "STATE_CLIENT_LOAD_MISSION";
    case STATE_CLIENT_VERIFY_MISSION:   return "STATE_CLIENT_VERIFY_MISSION";
    case STATE_CLIENT_SYNC:             return "STATE_CLIENT_SYNC";
    case STATE_CLIENT_INGAME:           return "STATE_CLIENT_INGAME";
    case STATE_CLIENT_COOLDOWN:         return "STATE_CLIENT_COOLDOWN";
    case STATE_CLIENT_DISCONNECT:       return "STATE_CLIENT_DISCONNECT";
    case STATE_CLIENT_KILL:             return "STATE_CLIENT_KILL";
    case STATE_CLIENT_IDLE:             return "STATE_CLIENT_IDLE";
    default:                            return "<unknown>";
    }
}

//------------------------------------------------------------------------------
const char* GetStateName(server_state State)
{
    switch(State)
    {
    case STATE_SERVER_INIT:             return "STATE_SERVER_INIT";
    case STATE_SERVER_LOAD_MISSION:     return "STATE_SERVER_LOAD_MISSION";
    case STATE_SERVER_SYNC:             return "STATE_SERVER_SYNC";
    case STATE_SERVER_INGAME:           return "STATE_SERVER_INGAME";
    case STATE_SERVER_COOLDOWN:         return "STATE_SERVER_COOLDOWN";
    case STATE_SERVER_SHUTDOWN:         return "STATE_SERVER_SHUTDOWN";
    case STATE_SERVER_IDLE:             return "STATE_SERVER_IDLE";
    case STATE_SERVER_KILL:             return "STATE_SERVER_KILL";
    default:                            return "<unknown>";
    }
}

