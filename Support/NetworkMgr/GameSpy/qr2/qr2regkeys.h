

#ifndef _QR2REGKEYS_H_
#define _QR2REGKEYS_H_

// needed for gsi_sb_char
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__mips64) && !defined(_WIN32)
    #include "::nonport.h" 
#else
    #include "../nonport.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
    

#define MAX_REGISTERED_KEYS 254
#define NUM_RESERVED_KEYS 50
    
    
#define HOSTNAME_KEY    1
#define GAMENAME_KEY    2
#define GAMEVER_KEY        3
#define HOSTPORT_KEY    4
#define MAPNAME_KEY        5
#define GAMETYPE_KEY    6
#define GAMEVARIANT_KEY    7
#define NUMPLAYERS_KEY    8
#define NUMTEAMS_KEY    9
#define MAXPLAYERS_KEY    10
#define GAMEMODE_KEY    11
#define TEAMPLAY_KEY    12
#define FRAGLIMIT_KEY    13
#define TEAMFRAGLIMIT_KEY    14
#define TIMEELAPSED_KEY    15
#define TIMELIMIT_KEY    16
#define ROUNDTIME_KEY    17
#define ROUNDELAPSED_KEY    18
#define PASSWORD_KEY    19
#define GROUPID_KEY        20
#define PLAYER__KEY        21
#define SCORE__KEY        22
#define SKILL__KEY        23
#define PING__KEY        24
#define TEAM__KEY        25
#define DEATHS__KEY        26
#define PID__KEY        27
#define TEAM_T_KEY        28
#define SCORE_T_KEY        29
    
#ifndef GSI_UNICODE
    #define qr2_register_key    qr2_register_keyA
#else
    #define qr2_register_key    qr2_register_keyW
#endif

extern const char *qr2_registered_key_list[];
void qr2_register_key(int keyid, const gsi_char *key);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Necessary for unicode support.  Must store a copy of the UTF8 keys
// generated from qr2_register_keyW
void qr2_internal_key_list_append(char* theKey);
void qr2_internal_key_list_free(); // call this at qr2 shutdown


#ifdef __cplusplus
}
#endif


#endif
