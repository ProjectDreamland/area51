//==============================================================================
//
//  MsgMgr.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "MsgMgr.hpp"
#include "ConnMgr.hpp"
#include "GameMgr.hpp"
#include "NetworkMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "PainMgr\PainTypes.hpp"

#include "MsgClient.hpp"

#include "x_log.hpp"  

//==============================================================================
//  STORAGE
//==============================================================================

msg_mgr MsgMgr;

extern xwchar s_NullString[];

// Performance tracking code.
/*
s32 g_MaxDiff = 0;
*/

//==============================================================================
//  MSG MGR FUNCTIONS
//==============================================================================

msg_mgr::msg_mgr( void )
{
    //    Reset();
};

//==============================================================================

msg_mgr::~msg_mgr( void )
{
};

//==============================================================================

void msg_mgr::Init( void )
{
    InitMessages();

    // Make sure that the other clients don't think they have any messages they don't
    for( s32 i = 0; i < MAX_CLIENTS; i++ )
    {
        m_Queue[ i ].Activate();
        m_Queue[ i ].m_Client = i;

        s32 j;
        for( j = 0; j < NUM_MESSAGES; j++ )
        {
            m_Queue[ i ].HasMessage[ j ] = -1;
        }
    }

    m_StartOfQueue  = 0;
    m_EndOfQueue    = 0;

    m_LastUnused    = MSG_LAST_PRESET + 1;

    MsgClient.Init();
}

//==============================================================================

void msg_mgr::Reset( void )
{
    // Remove old message
    for( s32 i = 0; i < NUM_MESSAGES; i++ )
    {
        if( !MsgTable[ i ].m_Original )
        {
            for( s32 j = 0; j < 3; j++ )
            {
                if( MsgTable[ i ].m_pMsgString[ j ] != NULL )
                    delete MsgTable[ i ].m_pMsgString[ j ];
                
                MsgTable[ i ].m_pMsgString[ j ] = NULL;
            }
            
            if( MsgTable[ i ].m_pMsgSound != NULL )
                delete MsgTable[ i ].m_pMsgSound;
            MsgTable[ i ].m_pMsgSound = NULL;

            if( MsgTable[ i ].m_pMsgSound2 != NULL )
                delete MsgTable[ i ].m_pMsgSound2;
            MsgTable[ i ].m_pMsgSound2 = NULL;
        }
    }
    g_StringTableMgr.UnloadTable( "Msg" );                                                                                                                                          
}

//==============================================================================

void msg_mgr::Message(  msg_id   MsgID,
                        s32      Target,
                        s32      arg1,
                        s32      arg2,
                        s32      arg3,
                        s32      arg4,
                        f32      Time,
                        s32      GoalID,
                        xbool    Enabled )
{
    msg Msg;
    Msg.m_bIsLocal      = TRUE;
    Msg.m_MsgID         = MsgID;
    Msg.m_Target        = Target;
    Msg.m_Audience      = MsgTable[ MsgID ].m_Audience;

    Msg.m_Time          = Time;
    Msg.m_MsgGoalNum    = GoalID,
    Msg.m_Enabled       = Enabled;

    ASSERT( ((MsgTable[ MsgID ].m_Types[ 0 ] == ARG_NONE) && (arg1 == -12345)) ||
            ((MsgTable[ MsgID ].m_Types[ 0 ] != ARG_NONE) && (arg1 != -12345)) );

    ASSERT( ((MsgTable[ MsgID ].m_Types[ 1 ] == ARG_NONE) && (arg2 == -12345)) ||
            ((MsgTable[ MsgID ].m_Types[ 1 ] != ARG_NONE) && (arg2 != -12345)) );

    ASSERT( ((MsgTable[ MsgID ].m_Types[ 2 ] == ARG_NONE) && (arg3 == -12345)) ||
            ((MsgTable[ MsgID ].m_Types[ 2 ] != ARG_NONE) && (arg3 != -12345)) );

    ASSERT( ((MsgTable[ MsgID ].m_Types[ 3 ] == ARG_NONE) && (arg4 == -12345)) ||
            ((MsgTable[ MsgID ].m_Types[ 3 ] != ARG_NONE) && (arg4 != -12345) ));

    Msg.SetArg( 1, arg1 );
    Msg.SetArg( 2, arg2 );
    Msg.SetArg( 3, arg3 );
    Msg.SetArg( 4, arg4 );
    Msg.ClearDelivered();

    Msg.m_MsgSeq = m_EndOfQueue;

    // For local players.
    MsgClient.AcceptMsg( Msg );

    // For remote players.
    if( (MsgTable[ Msg.m_MsgID ].m_Audience != HEAR_LOCAL_MACHINE) &&
        (MsgTable[ Msg.m_MsgID ].m_Audience != HEAR_LOCAL_PLAYER ) )
    {
        // Add to remote queue.
        AddMsg( Msg );
        m_EndOfQueue++;
    }
}

//==============================================================================

void msg_mgr::AddMsg( msg& Msg )
{
    CleanQueue();

    s32 MsgSeqDiff = m_EndOfQueue - m_StartOfQueue;

    // Performance tracking code;
    /*
    if( MsgSeqDiff > g_MaxDiff ) 
    {
    g_MaxDiff = MsgSeqDiff;
    }
    */

    // If there are over MAX_MSG_QUEUE in unacked msgs, just drop some.
    if( MsgSeqDiff >= MAX_MSG_QUEUE )
    {
        LOG_WARNING( "MsgMgr::AddMsg", 
            "MsgMgr queue full, dropping message %d!", 
            MsgMgr.m_StartOfQueue );

        m_StartOfQueue = m_EndOfQueue - MAX_MSG_QUEUE + 1;
    }

    const game_score&   Score   = GameMgr.GetScore();

    // Go through and route the message to the appropriate clients.
    s32 i;
    for( i = 0; i < MAX_PLAYERS; i++ )
    {
        if( (Score.Player[ i ].IsInGame) &&
            (Score.Player[ i ].ClientIndex != -1) &&
            (Msg.IsValidTarget( i )) )
        {
            s32 ClientNum   = Score.Player[ i ].ClientIndex;
            Msg.DeliverToClient( ClientNum, TRUE );

            // -1 signifies that the message hasn't been sent yet.
            m_Queue[ ClientNum ].SetPacketSeq( Msg.m_MsgSeq, -1 );
        }
    }

    // Put the msg at the end of the queue.
    SetMsg( Msg.m_MsgSeq, Msg );
}

//==============================================================================
// This method basically just keeps m_LastMsgAcked current so that
// we know when the msg queue has overflowed, in case we want to do
// anything with that knowledge.

void msg_mgr::CleanQueue( void )
{
    s32 FirstCleared = -1;
    s32 LastCleared  = -1;

    for( s32 i = m_StartOfQueue; i < m_EndOfQueue; i++ )
    {
        if( GetMsg( i ).DeliveredToAll() )
        {
            m_StartOfQueue = i + 1;

            if( FirstCleared == -1 )
            {
                FirstCleared = i;
            }
            LastCleared = i;
        }
        else
        {
            break;
        }
    }

    if( FirstCleared != -1 )
    {
        /*
        LOG_MESSAGE( "MsgMgr::CleanQueue", 
                     "Cleared msgs %d through %d.", 
                     FirstCleared, LastCleared );
        */
    }
}

//==============================================================================

msg msg_mgr::GetMsg( s32 QueueIndex )
{
    ASSERT( IN_RANGE( m_StartOfQueue, QueueIndex, m_EndOfQueue ) );

    s32 ArrayIndex = QueueIndex % MAX_MSG_QUEUE;

    ASSERT( m_Messages[ ArrayIndex ].m_MsgSeq == QueueIndex );

    return m_Messages[ ArrayIndex ];
}

//==============================================================================

void msg_mgr::SetMsg( s32 QueueIndex, msg& Msg )
{
    ASSERT( IN_RANGE( m_StartOfQueue, QueueIndex, m_EndOfQueue ) );

    s32 ArrayIndex = QueueIndex % MAX_MSG_QUEUE;
    ASSERT( QueueIndex == Msg.m_MsgSeq );

    m_Messages[ ArrayIndex ] = Msg; 
}

//==============================================================================

void msg_mgr::PacketAck( s32 PacketSeq, xbool Arrived, s32 ClientIndex )
{
    CleanQueue();
    m_Queue[ ClientIndex ].PacketAck( PacketSeq, Arrived );
}

//==============================================================================

void msg_mgr::ProvideMsg( s32 PacketSeq, bitstream& BS, s32 ClientIndex )
{
    CleanQueue();
    m_Queue[ ClientIndex ].PackMsg( PacketSeq, BS );
}

//==============================================================================

xbool msg_mgr::IsLocalTarget( s32 Target )
{
    // AHARP TODO fix this!
    ( void ) Target;
    return TRUE;
}

//==============================================================================

msg_id msg_mgr::RegMsg(       msg_id     MsgIndex, 
                            msg_hear   Audience,
                            msg_impact Impact,
                            msg_arg    Type1,
                            msg_arg    Type2,
                            msg_arg    Type3,
                            msg_arg    Type4,
                      const xwchar*    pMsgString0,
                      const xwchar*    pMsgString1,
                      const xwchar*    pMsgString2,
                      const char*      pMsgSound,
                      const char*      pMsgSound2,
                            msg_type   MsgType,
                            xbool      Active )
{
    xbool Original = FALSE;
    msg_info MsgInfo;

    MsgInfo.m_Index        = MsgIndex; 
    MsgInfo.m_Audience     = Audience;
    MsgInfo.m_Impact       = Impact;

    MsgInfo.m_Types[ 0 ]   = Type1;
    MsgInfo.m_Types[ 1 ]   = Type2;
    MsgInfo.m_Types[ 2 ]   = Type3;
    MsgInfo.m_Types[ 3 ]   = Type4;

    MsgInfo.m_pMsgString[ 0 ] = NULL;
    MsgInfo.m_pMsgString[ 1 ] = NULL;
    MsgInfo.m_pMsgString[ 2 ] = NULL;
    MsgInfo.m_pMsgSound       = NULL;
    MsgInfo.m_pMsgSound2      = NULL;

    if( pMsgString0 )
    {
        xwchar* pString = new xwchar[ x_wstrlen( pMsgString0 ) + 1 ];
        x_wstrcpy( pString, pMsgString0 );
        MsgInfo.m_pMsgString[ 0 ] = pString;
    }

    if( pMsgString1 )
    {
        xwchar* pString  = new xwchar[ x_wstrlen( pMsgString1 ) + 1 ];
        x_wstrcpy( pString, pMsgString1 );
        MsgInfo.m_pMsgString[ 1 ] = pString;
    }

    if( pMsgString2 )
    {
        xwchar* pString = new xwchar[ x_wstrlen( pMsgString2 ) + 1 ];
        x_wstrcpy( pString, pMsgString2 );
        MsgInfo.m_pMsgString[ 2 ] = pString;
    }

    if( pMsgSound )
    {
        char* pString = new char[ x_strlen( pMsgSound ) + 1 ];
        x_strcpy( pString, pMsgSound );
        MsgInfo.m_pMsgSound = pString;
    }

    if( pMsgSound2 )
    {
        char* pString = new char[ x_strlen( pMsgSound2 ) + 1 ];
        x_strcpy( pString, pMsgSound2 );
        MsgInfo.m_pMsgSound2 = pString;
    }

    MsgInfo.m_MsgType           = MsgType;  
    MsgInfo.m_Active            = Active;   
    MsgInfo.m_Original          = Original; 

    return RegMsg( MsgInfo );
}

//==============================================================================

const xwchar* msg_mgr::GetStr( const char* pStrName )
{        
    const xwchar* pString = g_StringTableMgr( "Msg", pStrName, FALSE );

    /*
    #if defined X_DEBUG && (!defined bhapgood)
    {
        s32 L = x_strlen( pStrName );
        if( (pStrName[L-2] == '_') && (pStrName[L-1] == 'A') )
        {
            if( pString == s_NullString )
            {
                BREAK;
            }
        }
    }
    #endif
    */

    if( pString == s_NullString ) 
    {
        return( NULL );
    }
    else
    {
        return( pString );
    }    
}

//==============================================================================

/*
void msg_mgr::SetMsgEnabled( s32 MsgIndex, xbool Enabled )
{
MsgTable[ MsgIndex ].m_Enabled = Enabled;
}
*/

//==============================================================================

msg_id msg_mgr::RegMsg( msg_info MsgPrototype )
{
    // -1 denotes we are creating a new message, and not just overwriting an old one
    if( MsgPrototype.m_Index == -1 )
    {
        ASSERT( m_LastUnused < NUM_MESSAGES );
        MsgPrototype.m_Index = (msg_id)m_LastUnused++;
    }
    else
    {
#if 0
        static xbool Initialized = FALSE;
        static xbool MessageRegistered[ NUM_MESSAGES ];
        static msg_id ActualLast = (msg_id)(MSG_NULL - 1);

        if( !Initialized )
        {
            Initialized = TRUE;
            for( int i = 0; i < NUM_MESSAGES; i++ )
            {
                MessageRegistered[ i ] = FALSE;
            }
        }  

        msg_id ExpectedLast   = (msg_id)(MsgPrototype.m_Index - 1);
        msg_id ShouldHaveBeen = (msg_id)(ActualLast + 1);

        if( MessageRegistered[ MsgPrototype.m_Index ] )
        {
            BREAK;
        }

        if( ActualLast != ExpectedLast )
        {
            BREAK;
        }

        MessageRegistered[ MsgPrototype.m_Index ] = TRUE;
        ActualLast = MsgPrototype.m_Index;
        (void)ShouldHaveBeen;
#endif
    }

    MsgTable[ MsgPrototype.m_Index ] = MsgPrototype;

    // Reset all of the msg queues so they know the client needs the message
    if( !MsgPrototype.m_Original )
    {
        s32 i;
        for( i = 0; i < MAX_CLIENTS; i++ )
        {
            m_Queue[ i ].HasMessage[ MsgPrototype.m_Index ] = -1;
        }
    }

    return MsgPrototype.m_Index; 
}

//==============================================================================

void msg_mgr::NewClient( s32 ClientSlot )
{
    m_Queue[ ClientSlot ].Deactivate();
}

//==============================================================================

msg_id msg_mgr::GetRandomMessageID( msg_category MsgCat )
{
    s32 Lower = 0;
    s32 Range = 0;

    switch( MsgCat )
    {
    case MCAT_KILLED:
        Lower = MSG_KILLED_01;
        Range = NUM_KILL_MSGS;
        break;

    case MCAT_SMP:
        Lower = MSG_SMP_01;
        Range = NUM_SMP_MSGS;
        break;

    case MCAT_DEAGLE:
        Lower = MSG_DEAGLE_01;
        Range = NUM_DEAGLE_MSGS;
        break;

    case MCAT_SNIPER:
        Lower = MSG_SNIPER_01;
        Range = NUM_SNIPER_MSGS;
        break;

    case MCAT_SHOTGUN:
        Lower = MSG_SHOTGUN_01;
        Range = NUM_SHOTGUN_MSGS;
        break;

    case MCAT_MESON:
        Lower = MSG_MESON_01;
        Range = NUM_MESON_MSGS;
        break;

    case MCAT_BBG:
        Lower = MSG_BBG_01;
        Range = NUM_BBG_MSGS;
        break;

    case MCAT_MELEE:
        Lower = MSG_MELEE_01;
        Range = NUM_MELEE_MSGS;
        break;

    case MCAT_MUTATION:
        Lower = MSG_MUTATION_01;
        Range = NUM_MUTATION_MSGS;
        break;

    case MCAT_EXTREMEMELEE:
        Lower = MSG_EXTREMEMELEE_01;
        Range = NUM_EXTREMEMELEE_MSGS;
        break;

    case MCAT_GRENADE:
        Lower = MSG_GRENADE_01;
        Range = NUM_GRENADE_MSGS;
        break;

    case MCAT_JBEAN:
        Lower = MSG_JBEAN_01;
        Range = NUM_JBEAN_MSGS;
        break;

    case MCAT_DIED:
        Lower = MSG_P_DIED_01;
        Range = NUM_DIED_MSGS;
        break;

    case MCAT_SUICIDE:
        Lower = MSG_KILLED_SELF_01;
        Range = NUM_SUICIDE_MSGS;
        break;

    case MCAT_FLAME:
        Lower = MSG_FLAME_01;
        Range = NUM_FLAME_MSGS;
        break;

    case MCAT_FALL:
        Lower = MSG_P_DIED_FALLING_01;
        Range = NUM_FALL_MSGS;
        break;

    case MCAT_TKED:
        Lower = MSG_TEAM_KILLED_01;
        Range = NUM_TK_MSGS;
        break;

    case MCAT_WELCOME:
        Lower = MSG_WELCOME_01;
        Range = NUM_WELCOME_MSGS;
        break;

    case MCAT_ACID:
        Lower = MSG_ACID_01;
        Range = NUM_ACID_MSGS;
        break;

    case MCAT_OOZE:
        Lower = MSG_OOZE_01;
        Range = NUM_OOZE_MSGS;
        break;

    case MCAT_FIRE:
        Lower = MSG_FLAME_01;
        Range = NUM_FLAME_MSGS;
        break;

    case MCAT_DROWNING:
        Lower = MSG_DROWNED_01;
        Range = NUM_DROWNED_MSGS;
        break;

    case MCAT_PARANOIA:
        Lower = MSG_PARANOIA_01;
        Range = NUM_PARANOIA_MSGS;
        break;

    default:
        ASSERT( 0 );
    }

    return ((msg_id)x_irand( Lower, Lower + Range - 1 ));
}

//==============================================================================

void msg_mgr::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    msg_id MsgID;

    const game_score& Score = GameMgr.GetScore();

    // Accidental Weapon Suicide.
    if( Victim == Killer )
    {
        MsgID = GetRandomMessageID( MCAT_SUICIDE );  
        Message( MsgID, Killer, Killer );
    }

    // Environmental / Intentional suicide.
    else if( Killer == -1 )
    {
        switch( PainType )
        {
        case ACID_WEAK:
        case ACID_STRONG:
        case ACID_LETHAL:
            MsgID = GetRandomMessageID( MCAT_ACID );    
            break;

        case GOO_WEAK:
        case GOO_STRONG:
        case GOO_LETHAL:
            MsgID = GetRandomMessageID( MCAT_OOZE ); 
            break;

        case FIRE_WEAK:
        case FIRE_STRONG:
        case FIRE_LETHAL:
            MsgID = GetRandomMessageID( MCAT_FIRE ); 
            break;

        case DROWNING:
            MsgID = GetRandomMessageID( MCAT_DROWNING ); 
            break;

        case MANUAL_SUICIDE:
            MsgID = MSG_MANUAL_SUICIDE_01;             
            break;

        case ZONE_PAIN:
            MsgID = GetRandomMessageID( MCAT_DIED ); 
            break;

        case PLAYER_FALL_DAMAGE2:
            MsgID = GetRandomMessageID( MCAT_FALL );
            break;

        default:
            MsgID = GetRandomMessageID( MCAT_DIED );    
            break;
        };

        Message( MsgID, Victim, Victim );
    }
    
    // Teamkill.
    else if( (Score.Player[ Victim ].Team == Score.Player[ Killer ].Team) &&
             Score.IsTeamBased )
    {
        MsgID = GetRandomMessageID( MCAT_TKED );
        Message( MsgID, Killer, Killer, Victim );
        Message( MsgID, Victim, Killer, Victim );
    }

    // Legitimate Kill.
    else
    {
        s32 Chance = x_irand( 0, 99 );
        if( Chance > 25 )
        {
            switch( PainType )
            {
            case PLAYER_SMP:
                MsgID = GetRandomMessageID( MCAT_SMP );    
                break;

            case PLAYER_DEAGLE:
                MsgID = GetRandomMessageID( MCAT_DEAGLE );    
                break;

            case PLAYER_SNIPER:
                MsgID = GetRandomMessageID( MCAT_SNIPER );    
                break;

            case PLAYER_SHOTGUN_PRIMARY:
            case PLAYER_SHOTGUN_SECONDARY:
                MsgID = GetRandomMessageID( MCAT_SHOTGUN );    
                break;

            case PLAYER_MESON_PRIMARY:
            case PLAYER_MESON_SECONDARY:
                MsgID = GetRandomMessageID( MCAT_MESON );    
                break;

            case PLAYER_BBG_PRIMARY:
                MsgID = GetRandomMessageID( MCAT_BBG );    
                break;

            case PLAYER_MELEE_0:
            case PLAYER_MELEE_1:
            case PLAYER_MELEE_2:
            case PLAYER_MELEE_3:
                MsgID = GetRandomMessageID( MCAT_MELEE );    
                break;

            case PLAYER_MUTATION_1:
            case PLAYER_MUTATION_2:
                MsgID = GetRandomMessageID( MCAT_MUTATION );    
                break;

            case PLAYER_EXTREMEMELEE:
                MsgID = GetRandomMessageID( MCAT_EXTREMEMELEE );    
                break;

            case PLAYER_GRENADE:
                MsgID = GetRandomMessageID( MCAT_GRENADE );    
                break;

            case PLAYER_JBEAN:
                MsgID = GetRandomMessageID( MCAT_JBEAN );    
                break;

            default:
                MsgID = GetRandomMessageID( MCAT_KILLED );    
                break;
            }
        }
        else
        {
            MsgID = GetRandomMessageID( MCAT_KILLED );
        }

        Message( MsgID, Killer, Killer, Victim );
        Message( MsgID, Victim, Killer, Victim );
    }
}

//==============================================================================



