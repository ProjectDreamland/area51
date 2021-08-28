//==============================================================================
//
//  ServerVersion.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef SERVERVERSION_HPP
#define SERVERVERSION_HPP

//==============================================================================
//  COMMENTS:
//==============================================================================
//
//  VALUE   WHO     WHAT CHANGED
//  1000    BW      First revision of connection protocol
//  1001    BW      Protocol updates.
//  1002    BW      Protocol update
//  1003    BW      Wide strings changed to narrow strings for login etc.
//  1004    MK      Added Diffie Hellman Key Exchange, and Blowfish encryption.
//  1005    BW      Added player count and server configuration to login response
//  1006    DMT     Added initial MsgMgr support.
//  1007    DMT     Revised MoveMgr protocol.
//  1008    DMT     Added team to move information from client to server.
//  1009    DMT     First pass on bit compression.
//  1010    BW      Removed bitstream markers from release builds. Fewer moves sent.
//  1011    BW      Lookup responses now react to voice enabled
//  1012    DMT     Additional data compression
//  1013    AY      Voice Communication Arbitration
//  1014    DMT     Added support for Server -> Client pain quadrant display.
//  1015    CJ      Added Melee attack support
//  1016    DMT     Added weapon set to map request.
//  1017    AY      Added flag designating voice manager active or not
//  1018    DMT     Sending "friendly fire" bit with map request.
//  1019    DMT     Sending "grenade type"  bit with map request.
//  1020    CJ      Mirroring client Ammo counts to server ghosts
//  1021    RB      Enabled encryption.
//  1022    BW      Login packet now includes ticket string
//  1023    DMT     Added support for skins.
//  1024    DMT     Changed bits used to send pain via client moves.
//  1025    DMT     Added score limit to data transmitted by the GameMgr.
//  1026    BW      Added voice support
//  1027    BW      State machines for client/server changed.
//  1028    DMT     Lots of new stuff in the network protocols.
//  1029    DMT     Added more net support into the GameMgr and pGameLogic.
//  1030    DMT     Added support for TDM game type.
//  1031    BW      Re-installed debug ticker for a debug build only.
//  1032    DMT     Fixed bug with ALT_FIRE and MELEE.
//  1033    ALH     MsgMgr Memory tweaks
//  1034    ALH     New msgs
//  1035    ALH     Team Message Fixes
//  1036    ALH     New String passing stuff
//  1037    ALH     Msg serialization changed
//  1038    ALH     Msg sending intelligence tweaked/bugs fixed
//  1039    DMT     Reordered ghost/player update data.
//  1040    DMT     Added LifeSeq to net traffic.
//  1041    BW      Initial xbox live support. Changed all strings to wide.
//  1042    DMT     Added support for CTF.
//  1043    DMT     Added MORE support for CTF.
//  1044    BW      Modified map response to include text form of mission name.
//  1045    DMT     Added support for 'tossables'.
//  1046    DMT     Changed health from s32 to f32 over the wire.
//  1047    DMT     Removed DeltaData and MoveMgr systems.
//  1048    DMT     Added data support for music levels in GameMgr.
//  1049    DMT     Added support for timed games in GameMgr.
//  1050    DMT     Integrating new pain system with networking.
//  1051    DMT     Removed MoveMgr, expanded UpdateMgr, added PainQueue.
//  1052    DMT     Added support for crouching and "airborn".
//  1053    DMT     No longer sending the map name, just its unique number.
//  1054    DMT     First pass at client side object management.  Inven, too.
//  1055    DMT     More work on inventory over the wire.
//  1056    DMT     Added TeamDamage to GameMgr net data.
//  1057    DMT     Voting!
//  1058    DMT     Transmit loadout from server to client during spawn.
//  1059    DMT     Include weapon bit for "mutation" weapon.
//  1060    ALH     Player relative aiming for net ghosts.
//  1061    DMT     Support for sending skin selection over wire.
//  1062    DMT     Support for Tag game type.
//  1063    BW      Added talk mode to voice data
//  1064    DMT     Support for begin/end weapon fire.  (Used for SMP.)
//  1065    BW      Changed format of login request to include PlayerID.
//  1066    DMT     Added support for 'circuits'.
//  1067    BW      Extended player identifier to 64 bits.
//  1068    ALH     TalkMode is now sent in VoiceMgr updates.
//  1069    DMT     Added support for leaning in the network protocol.
//  1070    DMT     Added support for JB grenades.
//  1071    DMT     HACK - Send game type to clients to fix circuits on client.
//  1072    BW      Added some security measures.  Reduced size of packet headers.
//  1073    DMT     Moved "Can P1 vote to kick P2?" logic into server.
//  1074    DMT     UpdateMgr: Collapsed 'state' into DirtyBits.
//                  PainQueue: Cut packet size of net_pain in half.
//  1075    DMT     Fixed a few problems with the UpdateMgr optimizations.
//  1076    DMT     More optimizations on the PainQueue.
//  1077    DMT     Added a teleport bit to the actor.
//  1078    DMT     Increased server side objects from 256 to 384.
//  1079    DMT     Now includes the active weapon with the spawn information.
//  1080    BW      Added packet encryption.
//  1081    DMT     Added flag_base class for CTF.
//  1082    DMT     Added flashlight support.  Prep for teleport & jump pad FX.
//  1083    DMT     Include state with pickup creation.  Fixed range checks.
//  1084    DMT     Added support for game event messages.
//  1085    DMT     Steve B added support for sending corpse pain over wire.
//  1086    JP      Player score structure now maintains voice peripheral status.
//  1087    DMT     Adding stats support.  (Ugh!)
//  1088    BW      Added Extended lookup response to XBOX requests to get game score
//  1089    ALH     Flag effect network support.
//  1090    DMT     NetObj slot allocation and tracking version 2.0.
//  1091    ALH     Clients send current JBean grenade count as well as frag.
//  1092    ALH     Changed the way dynamic and string messages are handled.
//  1093    DMT     Added support for blended teleport / jump pad rendering.
//  1094    DMT     Added support for net contagion.
//  1095    DMT     Packet compression.
//  1096    DMT     Fix for pickups.
//  1097    DMT     Fix for net projectiles attached flag.
//  1098    SB      Optimized bits for net projectiles attached data.
//  1099    ALH     Brought dynamic message code up to date.
//  1100    DMT     Had to add another bit for SMP ammo.  (600 rounds?)
//  1101    DMT     Slight logic cleanup in net_proj.
//  1102    DMT     Removed unused net bits from JB grenade.
//  1103    DMT     Removed unused net bits from parasite/contagion.
//  1104    SB      Always read/write mutate bit to make sure server + clients are in sync
//  1105    BW      Removed 'ticket' from login requests
//  1106    SB      Reverted mutate bit send/receive to use dirty bit to stop circular problems
//  1107    BW      Added more information to server lookup request
//  1108    DMT     Net pain health damage is now sent in fixed point.
//  1109    JP      Added bit for voice chat enabled in an online game.
//  1110    JP      Removed bit from 1109 and added it only when server config is sent.
//  1111    SB      Added fire sequence # to stop smp audio looping if begin/end packets get re-orderd
//==============================================================================

#define BASE_SERVER_VERSION     1111

//==============================================================================

#if defined(X_DEBUG)
#define SERVER_VERSION  (BASE_SERVER_VERSION+10000)
#else
#define SERVER_VERSION  (BASE_SERVER_VERSION)
#endif

extern s32 g_ServerVersion;

//==============================================================================
#endif // SERVERVERSION_HPP
//==============================================================================
