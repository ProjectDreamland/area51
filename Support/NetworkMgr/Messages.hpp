//==============================================================================
//
//  Messages.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef MESSAGES_HPP
#define MESSAGES_HPP

//==============================================================================
//  DEFINES
//==============================================================================

// Number of messages the MsgMgr will keep around at any one time.
#define MAX_MSG_QUEUE            30            

// How long the strings sent as args in messages can be.
#define MAX_STRING_LENGTH        128

// Defines how long the buffer that display strings are composed in is. 
//  should be MAX_STRING_LENGTH and then some (for color coding etc)
//  HACK: For now, MAXDISPLAYLENGTH in HudObject.hpp needs to be tweaked in parallel
#define MAX_DISPLAY_LENGTH      256     

//just defining for clarity, if tweaked code/data needs changing
#define MAX_MSG_ARGS              4     

//determines the highest arg a string can be placed in a message
#define MAX_MSG_STRINGS           2               

//number of outgoing packets to wait before sending more message data
#define RETRY_DELAY               4               

#define MAX_CLIENTS              32
#define MAX_PLAYERS              32


#define NUM_MESSAGES             MSG_LAST

// Convenience defines to take the tedium out of adding new messages.
// All that needs to be done to add a new message in these categories now is
// to add an entry to the enum below.  However, care must be taken that the
// order in which the different message categories are defined does not change,
// or these defines must be changed appropriately.
#define NUM_DIED_MSGS           (MSG_P_DIED_FALLING_01          - MSG_P_DIED_01         )
#define NUM_FALL_MSGS           (MSG_TEAM_KILLED_01             - MSG_P_DIED_FALLING_01 )
#define NUM_TK_MSGS             (MSG_KILLED_01                  - MSG_TEAM_KILLED_01    )
#define NUM_KILL_MSGS           (MSG_SMP_01                     - MSG_KILLED_01         )

#define NUM_SMP_MSGS            (MSG_DEAGLE_01                  - MSG_SMP_01            )
#define NUM_DEAGLE_MSGS         (MSG_SNIPER_01                  - MSG_DEAGLE_01         )
#define NUM_SNIPER_MSGS         (MSG_SHOTGUN_01                 - MSG_SNIPER_01         )
#define NUM_SHOTGUN_MSGS        (MSG_MESON_01                   - MSG_SHOTGUN_01        )
#define NUM_MESON_MSGS          (MSG_BBG_01                     - MSG_MESON_01          )
#define NUM_BBG_MSGS            (MSG_MELEE_01                   - MSG_BBG_01            )
#define NUM_MELEE_MSGS          (MSG_MUTATION_01                - MSG_MELEE_01          )
#define NUM_MUTATION_MSGS       (MSG_EXTREMEMELEE_01            - MSG_MUTATION_01       )
#define NUM_EXTREMEMELEE_MSGS   (MSG_CONTAGION_01               - MSG_EXTREMEMELEE_01   )
#define NUM_CONTAGION_MSGS      (MSG_GRENADE_01                 - MSG_CONTAGION_01      )
#define NUM_GRENADE_MSGS        (MSG_JBEAN_01                   - MSG_GRENADE_01        )
#define NUM_JBEAN_MSGS          (MSG_KILLED_SELF_01             - MSG_JBEAN_01          )

#define NUM_SUICIDE_MSGS        (MSG_ACID_01                    - MSG_KILLED_SELF_01    )

#define NUM_ACID_MSGS           (MSG_OOZE_01                    - MSG_ACID_01           )
#define NUM_OOZE_MSGS           (MSG_FLAME_01                   - MSG_OOZE_01           )
#define NUM_FLAME_MSGS          (MSG_MANUAL_SUICIDE_01             - MSG_FLAME_01          ) 
#define NUM_MANUAL_SUICIDE_MSGS (MSG_DROWNED_01                 - MSG_MANUAL_SUICIDE_01 )
#define NUM_DROWNED_MSGS        (MSG_ZONED_OUT_01               - MSG_DROWNED_01        )
#define NUM_ZONED_OUT_MSGS      (MSG_PARANOIA_01                - MSG_ZONED_OUT_01      )

#define NUM_PARANOIA_MSGS       (MSG_P_CAPTURED_FLAG_GOOD       - MSG_PARANOIA_01       )

#define NUM_WELCOME_MSGS        (MSG_FIVE_MINUTES               - MSG_WELCOME_01        )

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_string.hpp"
#include "x_Bitstream.hpp"

//==============================================================================
//  TYPES
//==============================================================================

enum msg_arg
{
    ARG_NONE,
    ARG_PLAYER,
    ARG_PLAYER_NAME,    // like ARG_PLAYER but never replaces itself with "You"
    ARG_NUMBER,         // Add signed number?
    ARG_NUMBER_SIGNED,
    ARG_ITEM,
    ARG_TEAM,
    ARG_STRING,
    ARG_ORDINAL,
    ARG_TEAM_SCORE,
    ARG_PLAYER_SCORE,
};

enum msg_hear
{
    // Check the table...
    HEAR_NONE,              // Send to nobody.
    HEAR_ALL,               // Send to everybody.
    HEAR_CLIENT,            // Send to all players on a specific client.
    HEAR_PLAYER,            // Send to a single player.
    HEAR_TEAM,              // Send to an entire team.
    HEAR_OTHER_CLIENT,      // Send to everybody BUT a specific client.
    HEAR_OTHER_PLAYER,      // Send to everybody BUT a specific player.
    HEAR_LOCAL_MACHINE,     // So you don't have to worry about getting the right player or client ids.
    HEAR_LOCAL_PLAYER,      // Send to a local player.
    HEAR_LAST,
};

// This determines whether or not the message has good news or not for the recipient.
// It is based on the first arg val, which must be a player or a team.  If the recipient
// is on the same team as arg1, the recipient gets the same color coding (good or bad).  If on
// another team, the recipient gets the opposite color coding.  Neutral messages are not
// affected.
enum msg_impact
{
    IMPACT_NEUTRAL,
    IMPACT_GOOD,
    IMPACT_BAD,
    IMPACT_URGENT,
};

enum msg_type
{
    NORMAL,         // Shows up in the normal place.
    GOAL,           // Shows up as a goal and is persistent.
    BONUS,          // Shows up as a bonus somewhere in the middle of the screen and is temporary.
    WEAPON_INFO,    // Shows up as a weapon info down by ammo readout.
    SCORE_MSG,      // Shows up in the top right corner of the screen.
};

enum msg_id
{
    MSG_NULL,

    // Connection Stuff
    MSG_P_CONNECTED,
    MSG_P_CONNECTED_ALLY,
    MSG_P_ENTERED_GAME_1,
    MSG_P_ENTERED_GAME_2,
    MSG_P_JOINED_TEAM_ALLY,
    MSG_P_JOINED_TEAM_ENEMY_1,
    MSG_P_JOINED_TEAM_ENEMY_2,
    MSG_P_DISCONNECTED,

    // String MSG
    MSG_STRING,
    MSG_GOAL_STRING,
    MSG_BONUS_STRING,

    // Deathmatch
    MSG_P_DIED_01,
    MSG_P_DIED_02,
    MSG_P_DIED_03,
    MSG_P_DIED_04,

    MSG_P_DIED_FALLING_01,
    MSG_P_DIED_FALLING_02,
    MSG_P_DIED_FALLING_03,
    MSG_P_DIED_FALLING_04,

    MSG_TEAM_KILLED_01,
    MSG_TEAM_KILLED_02,
    MSG_TEAM_KILLED_03,
    MSG_TEAM_KILLED_04,
    MSG_TEAM_KILLED_05,
    MSG_TEAM_KILLED_06,
    MSG_TEAM_KILLED_07,
    MSG_TEAM_KILLED_08,

    MSG_KILLED_01,
    MSG_KILLED_02,
    MSG_KILLED_03,
    MSG_KILLED_04,
    MSG_KILLED_05,
    MSG_KILLED_06,
    MSG_KILLED_07,
    MSG_KILLED_08,
    MSG_KILLED_09,
    MSG_KILLED_10,
    MSG_KILLED_11,
    MSG_KILLED_12,
    MSG_KILLED_13,
    MSG_KILLED_14,
    MSG_KILLED_15,
    MSG_KILLED_16,
    MSG_KILLED_17,
    MSG_KILLED_18,
    MSG_KILLED_19,
    MSG_KILLED_20,
    MSG_KILLED_21,
    MSG_KILLED_22,
    MSG_KILLED_23,
    MSG_KILLED_24,
    MSG_KILLED_25,

    MSG_SMP_01,
    MSG_SMP_02,
    MSG_SMP_03,
    MSG_SMP_04,

    MSG_DEAGLE_01,
    MSG_DEAGLE_02,
    MSG_DEAGLE_03,
    MSG_DEAGLE_04,

    MSG_SNIPER_01,
    MSG_SNIPER_02,
    MSG_SNIPER_03,

    MSG_SHOTGUN_01,
    MSG_SHOTGUN_02,
    MSG_SHOTGUN_03,
    MSG_SHOTGUN_04,

    MSG_MESON_01,
    MSG_MESON_02,
    MSG_MESON_03,
    MSG_MESON_04,
    MSG_MESON_05,
    MSG_MESON_06,

    MSG_BBG_01,
    MSG_BBG_02,
    MSG_BBG_03,
    MSG_BBG_04,

    MSG_MELEE_01,
    MSG_MELEE_02,
    MSG_MELEE_03,
    MSG_MELEE_04,
    MSG_MELEE_05,

    MSG_MUTATION_01,
    MSG_MUTATION_02,
    MSG_MUTATION_03,
    MSG_MUTATION_04,
    MSG_MUTATION_05,

    MSG_EXTREMEMELEE_01,
    MSG_EXTREMEMELEE_02,
    MSG_EXTREMEMELEE_03,
    MSG_EXTREMEMELEE_04,
    MSG_EXTREMEMELEE_05,
    MSG_EXTREMEMELEE_06,
    MSG_EXTREMEMELEE_07,
    MSG_EXTREMEMELEE_08,
    MSG_EXTREMEMELEE_09,

    MSG_CONTAGION_01,
    MSG_CONTAGION_02,
    MSG_CONTAGION_03,
    MSG_CONTAGION_04,

    MSG_GRENADE_01,
    MSG_GRENADE_02,    
    MSG_GRENADE_03,
    MSG_GRENADE_04,
    MSG_GRENADE_05,

    MSG_JBEAN_01,
    MSG_JBEAN_02,
    MSG_JBEAN_03,

    MSG_KILLED_SELF_01,
    MSG_KILLED_SELF_02,
    MSG_KILLED_SELF_03,
    MSG_KILLED_SELF_04,
    MSG_KILLED_SELF_05,

    MSG_ACID_01,
    MSG_ACID_02,
    MSG_ACID_03,
    MSG_ACID_04,

    MSG_OOZE_01,
    MSG_OOZE_02,
    MSG_OOZE_03,

    MSG_FLAME_01,
    MSG_FLAME_02,
    MSG_FLAME_03,
    MSG_FLAME_04,
    MSG_FLAME_05,
    MSG_FLAME_06,
    MSG_FLAME_07,
    MSG_FLAME_08,    
    MSG_FLAME_09,
    MSG_FLAME_10,
    MSG_FLAME_11,
    MSG_FLAME_12,

    MSG_MANUAL_SUICIDE_01,
    MSG_MANUAL_SUICIDE_02,
    MSG_MANUAL_SUICIDE_03,
    MSG_MANUAL_SUICIDE_04,

    MSG_DROWNED_01,
    MSG_DROWNED_02,
    MSG_DROWNED_03,
    MSG_DROWNED_04,

    MSG_ZONED_OUT_01,
    MSG_ZONED_OUT_02,
    MSG_ZONED_OUT_03,
    MSG_ZONED_OUT_04,
    MSG_ZONED_OUT_05,

    MSG_PARANOIA_01,
    MSG_PARANOIA_02,
    MSG_PARANOIA_03,
    MSG_PARANOIA_04,
    MSG_PARANOIA_05,
    MSG_PARANOIA_06,
    MSG_PARANOIA_07,
    MSG_PARANOIA_08,
    MSG_PARANOIA_09,
    MSG_PARANOIA_10,
    MSG_PARANOIA_11,

    // CTF
    MSG_P_CAPTURED_FLAG_GOOD,
    MSG_P_DROPPED_FLAG_GOOD, 
    MSG_P_TOOK_FLAG_GOOD,    
    MSG_P_TOOK_FLAG_GOOD_2,    
    MSG_P_RETURNED_FLAG_GOOD,

    MSG_P_CAPTURED_FLAG_BAD,
    MSG_P_DROPPED_FLAG_BAD, 
    MSG_P_TOOK_FLAG_BAD,    
    MSG_P_TOOK_FLAG_BAD_2,    
    MSG_P_RETURNED_FLAG_BAD,

    MSG_GOAL_FLASHLIGHT,
    MSG_BONUS,
    MSG_GOAL_YOU_HAVE_FLAG,

    MSG_SOUND,

    MSG_P_IN_LEAD,
    MSG_P_N_POINTS,
    MSG_P_1_POINT,

    MSG_P_CALLED_VOTE,

    MSG_LOW_AMMO,
    MSG_FULL_AMMO,
    MSG_FULL_ITEM,
    MSG_ACQUIRED_ITEM,
    MSG_LORE_ITEM_ACQUIRED,
    MSG_SCAN_INVALID,
    MSG_NO_ITEM_AMMO_FAIL,
    MSG_SECRET_UNLOCKED,

    //
    // Begin VO messages.
    //
    MSG_WELCOME_01,
    MSG_WELCOME_02,
    MSG_WELCOME_03,
    MSG_WELCOME_04,
    MSG_WELCOME_05,
    MSG_WELCOME_06,
    MSG_WELCOME_07,

    MSG_FIVE_MINUTES,
    MSG_TWO_MINUTES,
    MSG_SIXTY_SECONDS,
    MSG_THIRTY_SECONDS,
    MSG_TEN_SECONDS,
    MSG_FIVE_FOUR_THREE,

    MSG_YOU_WIN,
    MSG_YOU_ARE_VICTORIOUS,
    MSG_ALLIED_TEAM_WINS,
    
    MSG_MSG_ENEMY_TEAM_WINS,
    MSG_GAME_OVER,
    MSG_OVER_TIME,
    MSG_SUDDEN_DEATH,
    
    MSG_YOU_TAKE_LEAD,
    MSG_YOU_TIE_LEAD,
    MSG_YOU_LOSE_LEAD,
    MSG_TEAM_TAKES_LEAD,
    MSG_ENEMY_TAKES_LEAD,

    MSG_NEW_CHALLENGER_ARRIVED,
    MSG_REINFORCEMENTS_ARRIVED,
    MSG_REINFORCEMENTS_INCOMING,
    MSG_RESISTANCE_INCREASED,
    MSG_NEW_ENEMY_ARRIVED,
    MSG_ENEMY_FORCES_INCREASED,

    MSG_MULTIKILL,
    MSG_BERSERKER,
    MSG_BERSERKER_KILL,
    MSG_BRUTALITY,
    MSG_SHREDDER,
    MSG_DEADEYE,
    MSG_SHELLSHOCK,
    MSG_GUNSLINGER,
    MSG_FUSION,
    MSG_MELTDOWN,
    MSG_MAULER,
    MSG_LEECHER,
    MSG_OUTBREAK,
    MSG_HEADSHOT,
    MSG_SHARPSHOOTER,
    MSG_MASOCHIST,
    MSG_CAPPER,
    MSG_DEFENDER,
    MSG_NOOB,
    MSG_MULTIKILL_BONUS,
    MSG_BERSERKER_BONUS,
    MSG_BERSERKER_KILL_BONUS,
    MSG_BRUTALITY_BONUS,
    MSG_SHREDDER_BONUS,
    MSG_DEADEYE_BONUS,
    MSG_SHELLSHOCK_BONUS,
    MSG_GUNSLINGER_BONUS,
    MSG_FUSION_BONUS,
    MSG_MELTDOWN_BONUS,
    MSG_MAULER_BONUS,
    MSG_LEECHER_BONUS,
    MSG_OUTBREAK_BONUS,
    MSG_HEADSHOT_BONUS,
    MSG_SHARPSHOOTER_BONUS,
    MSG_MASOCHIST_BONUS,
    MSG_CAPPER_BONUS,
    MSG_DEFENDER_BONUS,
    MSG_NOOB_BONUS,

    MSG_PLAYER_KICKED,
    MSG_TKER_KICKED,

    MSG_ALLIED_CAPTURE_SECTOR,
    MSG_ENEMY_CAPTURE_SECTOR,
    MSG_SECTOR_CAPTURED,
    MSG_SECTOR_SECURE,
    MSG_SECTOR_LOST,
    MSG_SECTOR_ATTACKED,
    MSG_SECTOR_NEUTRALIZED,

    /*
    MSG_SECTOR_CAPTURED_00,
    MSG_SECTOR_SECURE_00,
    MSG_SECTOR_LOST_00,
    MSG_SECTOR_CAPTURED_01,
    MSG_SECTOR_SECURE_01,
    MSG_SECTOR_LOST_01,
    MSG_SECTOR_CAPTURED_02,
    MSG_SECTOR_SECURE_02,
    MSG_SECTOR_LOST_02,
    MSG_SECTOR_CAPTURED_03,
    MSG_SECTOR_SECURE_03,
    MSG_SECTOR_LOST_03,
    MSG_SECTOR_CAPTURED_04,
    MSG_SECTOR_SECURE_04,
    MSG_SECTOR_LOST_04,
    MSG_SECTOR_CAPTURED_05,
    MSG_SECTOR_SECURE_05,
    MSG_SECTOR_LOST_05,
    MSG_SECTOR_CAPTURED_06,
    MSG_SECTOR_SECURE_06,
    MSG_SECTOR_LOST_06,
    MSG_SECTOR_CAPTURED_07,
    MSG_SECTOR_SECURE_07,
    MSG_SECTOR_LOST_07,
    MSG_SECTOR_CAPTURED_08,
    MSG_SECTOR_SECURE_08,
    MSG_SECTOR_LOST_08,
    MSG_SECTOR_CAPTURED_09,
    MSG_SECTOR_SECURE_09,
    MSG_SECTOR_LOST_09,
      */

    MSG_ALLIED_FLAG_TAKEN,
    MSG_ALLIED_FLAG_RETURNED,
    MSG_YOUR_TEAM_SCORES,
    MSG_ALLIED_TEAM_SCORES,
    MSG_ENEMY_FLAG_TAKEN,
    MSG_ENEMY_FLAG_RETURNED,
    MSG_ENEMY_TEAM_SCORES,
    MSG_YOU_HAVE_ENEMY_FLAG,

    MSG_PATIENT_ZERO_DETECTED,
    MSG_PATIENT_ZERO_IDENTIFIED,
    MSG_INFECTED,
    MSG_CONTAMINATED,
    MSG_CORRUPTED,
    MSG_HUMANS_REMAIN_1,
    MSG_HUMANS_REMAIN_2,
    MSG_HUMANS_REMAIN_3,
    MSG_HUMANS_REMAIN_4,
    MSG_HUMANS_REMAIN_5,
    MSG_LAST_SURVIVOR,
    MSG_YOU_ARE_PATIENT_ZERO,
    MSG_PLAYER_INFECTED,

    MSG_KILLS_REMAINING_10,
    MSG_KILLS_REMAINING_05,
    MSG_KILLS_REMAINING_04,
    MSG_KILLS_REMAINING_03,
    MSG_KILLS_REMAINING_02,
    MSG_KILLS_REMAINING_01,
    MSG_KILLS_LEFT_10,
    MSG_KILLS_LEFT_05,
    MSG_KILLS_LEFT_04,
    MSG_KILLS_LEFT_03,
    MSG_KILLS_LEFT_02,
    MSG_KILLS_LEFT_01,

    MSG_POINTS_REMAINING_50,
    MSG_POINTS_REMAINING_25,
    MSG_POINTS_REMAINING_10,
    MSG_POINTS_REMAINING_05,
    MSG_POINTS_REMAINING_04,
    MSG_POINTS_REMAINING_03,
    MSG_POINTS_REMAINING_02,
    MSG_POINTS_REMAINING_01,

    MSG_PREY_IS_CHOSEN,
    MSG_YOU_ARE_HUNTED,
    MSG_YOU_BECOME_HUNTED,
    MSG_YOU_ARE_PREY,
    MSG_YOU_BECOME_PREY,
    MSG_HUNT,
    MSG_SEEK_AND_DESTROY,

    MSG_YOU_ARE_IT,
    MSG_DOMINATOR,
    MSG_UNTOUCHABLE,
    MSG_BULLETPROOF,
    MSG_INVINCIBLE,
    MSG_FODDER,
    MSG_FEEBLE,

    MSG_ROUND_1,
    MSG_ROUND_2,
    MSG_ROUND_3,
    MSG_ROUND_4,
    MSG_ROUND_5,

    MSG_TEAMS_UNEVEN,
    MSG_TEAMS_UNBALANCED,
    MSG_ENEMY_OUTMANNED,
    MSG_ALLIES_OUTMANNED,

    /*
    MSG_TEAM_SCORES_00,
    MSG_TEAM_WINS_00,
    MSG_TEAM_SCORES_01,
    MSG_TEAM_WINS_01,
    MSG_TEAM_SCORES_02,
    MSG_TEAM_WINS_02,
    MSG_TEAM_SCORES_03,
    MSG_TEAM_WINS_03,
    MSG_TEAM_SCORES_04,
    MSG_TEAM_WINS_04,
    MSG_TEAM_SCORES_05,
    MSG_TEAM_WINS_05,
    MSG_TEAM_SCORES_06,
    MSG_TEAM_WINS_06,
    MSG_TEAM_SCORES_07,
    MSG_TEAM_WINS_07,
    MSG_TEAM_SCORES_08,
    MSG_TEAM_WINS_08,
    */

    MSG_VOTE_INITIATED,
    MSG_VOTE_FAILED,
    MSG_VOTE_PASSED,
    MSG_JUDGEMENT_PASSED,
    MSG_ANNOYANCE_NEUTRALIZED,
    MSG_DISMISSED,
    MSG_BOOTED,
    MSG_KICKED,
    MSG_BANISHED,
    //
    // End VO messages.
    //

    MSG_SCORE_DISPLAY_RANKED,
    MSG_SCORE_DISPLAY_TEAM,
    MSG_SCORE_DISPLAY_CNH,
    MSG_SCORE_DISPLAY_CUSTOM,

    MSG_SERVER_TEAMCHANGED_PLAYER,
    MSG_PLAYER_SWITCHED_TEAM,
    MSG_EVACUATE_AREA,
    MSG_WARNING_VOTE_TO_KICK,
    MSG_KEYBOARD_CHAT,

    MSG_SURVIVAL_BONUS,      
    MSG_LAST_SURVIVOR_BONUS,
    MSG_CAPTURE_FLAG_BONUS,
    MSG_TAKE_FLAG_BONUS,
    MSG_RETURN_FLAG_BONUS,
    MSG_KILL_CARRIER_BONUS,
    MSG_DEFENSE_BONUS,
    MSG_OFFENSE_BONUS,
    MSG_TEAM_KILL_PENALTY,
    MSG_SUICIDE_PENALTY,

    MSG_DOOR_LOCKED_TEAM,
    MSG_DOOR_LOCKED_PPZ_TOO_MANY,
    MSG_DOOR_LOCKED_PPZ_TOO_FEW,

    MSG_LAST_PRESET,

    MSG_BLANK_01,
    MSG_BLANK_02,
    MSG_BLANK_03,
    MSG_BLANK_04,
    MSG_BLANK_05,
    MSG_BLANK_06,
    MSG_BLANK_07,
    MSG_BLANK_08,
    MSG_BLANK_09,
    MSG_BLANK_10,
    MSG_BLANK_11,
    MSG_BLANK_12,
    MSG_BLANK_13,
    MSG_BLANK_14,
    MSG_BLANK_15,
    MSG_BLANK_16,

    MSG_LAST,
};

// Used for special message types.
enum msg_category
{
    MCAT_KILLED,
    
    MCAT_SMP,
    MCAT_DEAGLE,
    MCAT_SNIPER,
    MCAT_SHOTGUN,
    MCAT_MESON,
    MCAT_BBG,
    MCAT_MELEE,
    MCAT_MUTATION,
    MCAT_EXTREMEMELEE,
    MCAT_GRENADE,
    MCAT_JBEAN,

    MCAT_DIED,
    MCAT_SUICIDE,
    MCAT_FLAME,
    MCAT_FALL,
    MCAT_TKED,

    MCAT_PARANOIA,

    MCAT_ACID,
    MCAT_OOZE,
    MCAT_FIRE, 
    MCAT_DROWNING,

    MCAT_WELCOME,
};

const xwchar* ReadWString( const bitstream& BS );
void WriteWString( bitstream& BS, const xwchar* pString );

struct msg_info
{
    msg_id          m_Index;
    msg_hear        m_Audience;
    msg_impact      m_Impact;
    msg_arg         m_Types[ MAX_MSG_ARGS ];

    const xwchar*   m_pMsgString[ 3 ];
    const char*     m_pStringName;

    const char*     m_pMsgSound;  
    const char*     m_pMsgSound2; 

    msg_type        m_MsgType;

    xbool           m_Original;
    xbool           m_Active;

    inline void WriteMessageInfo( bitstream& bs )
    {
        bs.WriteMarker();
        bs.WriteS32( m_Index );
        bs.WriteS32( (s32) m_Audience );
        bs.WriteS32( (s32) m_Impact   );
        bs.WriteS32( (s32) m_MsgType  );
        bs.WriteFlag( m_Active );

        s32 i;
        for( i = 0; i < MAX_MSG_ARGS; i++ )
        {
            bs.WriteS32( m_Types[ i ] );
        }

        // Original can be assumed to be FALSE if we're having to send it over the wire.
        if( bs.WriteFlag( m_pMsgString[ 0 ] != NULL ) ) { WriteWString( bs, m_pMsgString[ 0 ] ); }
        if( bs.WriteFlag( m_pMsgString[ 1 ] != NULL ) ) { WriteWString( bs, m_pMsgString[ 1 ] ); }
        if( bs.WriteFlag( m_pMsgString[ 2 ] != NULL ) ) { WriteWString( bs, m_pMsgString[ 2 ] ); }
        
        if( bs.WriteFlag( m_pMsgSound  != NULL      ) ) bs.WriteString( m_pMsgSound  );
        if( bs.WriteFlag( m_pMsgSound2 != NULL      ) ) bs.WriteString( m_pMsgSound2 );
        
        bs.WriteMarker();
    }

    inline void ReadMessageInfo( const  bitstream& bs )
    {
        // Delete previous resident.
        if( !m_Original )
        {
            if( m_pMsgString[ 0 ] != NULL ) { delete m_pMsgString[ 0 ]; }
            if( m_pMsgString[ 1 ] != NULL ) { delete m_pMsgString[ 1 ]; }
            if( m_pMsgString[ 2 ] != NULL ) { delete m_pMsgString[ 2 ]; }

            if( m_pMsgSound != NULL  ) { delete m_pMsgSound;  }
            if( m_pMsgSound2 != NULL ) { delete m_pMsgSound2; }
        }

        m_pMsgString[ 0 ] = NULL;
        m_pMsgString[ 1 ] = NULL;
        m_pMsgString[ 2 ] = NULL;
        m_pMsgSound       = NULL;
        m_pMsgSound2      = NULL;

        bs.ReadMarker();

        s32    TempVar;

        bs.ReadS32( TempVar );  m_Index    = (msg_id)TempVar;
        bs.ReadS32( TempVar );  m_Audience = (msg_hear)TempVar;
        bs.ReadS32( TempVar );  m_Impact   = (msg_impact)TempVar;
        bs.ReadS32( TempVar );  m_MsgType  = (msg_type)TempVar;

        bs.ReadFlag( m_Active );

        m_pStringName = NULL;

        s32 i;
        for( i = 0; i < MAX_MSG_ARGS; i++ )
        {
            bs.ReadS32( TempVar );  m_Types[ i ] = (msg_arg)TempVar;
        }

        if( bs.ReadFlag() ) { m_pMsgString[ 0 ] = ReadWString( bs ); } 
        if( bs.ReadFlag() ) { m_pMsgString[ 1 ] = ReadWString( bs ); }
        if( bs.ReadFlag() ) { m_pMsgString[ 2 ] = ReadWString( bs ); }

        char SoundName[ 64 ];

        if( bs.ReadFlag() )   
        {
            bs.ReadString( SoundName, 63 );
            char* pStr = new char[ x_strlen( SoundName ) + 1 ];
            x_strcpy( pStr, SoundName );
            m_pMsgSound = pStr;
        }

        if( bs.ReadFlag() )   
        {
            bs.ReadString( SoundName, 63 );
            char* pStr = new char[ x_strlen( SoundName ) + 1 ];
            x_strcpy( pStr, SoundName );
            m_pMsgSound2 = pStr;
        }
        
        bs.ReadMarker();

        // Original can be assumed to be FALSE if we're having to send it over the wire.
        m_Original = FALSE;
    }
};                 

//==============================================================================

struct msg_text
{
    const char*     MsgName;
    xwstring        MsgText;
};

void InitMessages( void );

//==============================================================================

extern msg_text MsgStrings[];
extern msg_info MsgTable[];

/*
// to track performance of MsgMgr.
extern s32      g_MaxDiff; 
extern s32      g_Retransmits;
extern s32      g_NumLost; 
*/

//==============================================================================
#endif // MESSAGES_HPP
//==============================================================================
