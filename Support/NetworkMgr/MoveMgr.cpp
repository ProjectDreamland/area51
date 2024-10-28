//==============================================================================
//
//  MoveMgr.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//
//  NOTES:
//
//  - Assuming that moves received in a single packet all have sequential
//    sequence numbers.  WARNING: This may not be true.
//
//  - Have added "locally originated pain caused to all players" to the move
//    structure.  Must be VERY CAREFUL not to apply same move twice!
//
//  - TO DO: Must signal in the move pain if the victim was killed and by what
//    type of pain.
//
//  - TO DO: If (statistically unlikely) a move with death pain completely fails
//    to arrive at the server, then we must somehow decide how to deal with the
//    locally speculatively dead player.  Perhaps when the "lost move" is over-
//    written, we can force the "dead" player to be alive again...?
//
//  - Need to CAREFULLY check out the move sequence number system.
//
//==============================================================================

#if 0

//#if !defined(mtraub)
#define X_SUPPRESS_LOGS
//#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "MoveMgr.hpp"
#include "NetworkMgr.hpp"

#include "x_debug.hpp"
#include "x_log.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  MOVE_MGR FUNCTIONS
//==============================================================================

move_mgr::move_mgr( void )
{
    m_ClientIndex = -1;
    Reset();
}

//==============================================================================

move_mgr::~move_mgr( void )
{
}

//==============================================================================

void move_mgr::Init( s32 ClientIndex )
{
    m_ClientIndex = ClientIndex;
    Reset();
}

//==============================================================================

xbool move_mgr::AddMove( move& Move )
{
//  ASSERT( m_Count < MAX_MOVES );    // TO DO - If full, overwrite oldest.

    if( m_Count == MAX_MOVES )
    {
        // Move queue is full.
        // Must check to see if the oldest move, which we will discard, has
        // any "critical" data within.  If so, that data must be reclaimed and
        // placed in a new move.

        // TO DO - Recover critical data from lost move.

        LOG_ERROR( "move_mgr::AddMove", "Client:%d - OVERWROTE:%d", 
                   m_ClientIndex, m_Move[m_Tail].Seq );

        m_Count -= 1;
        m_Tail   = (m_Tail+1) % MAX_MOVES;
    }

    if( Move.Seq == -1 )
    {
        // This is a move created by a local client player and queued for later
        // transmission to the server.  Set the sequence number.

        Move.Seq       = m_Seq++;
        Move.SendsLeft = 1; // <-------------CONTROL VALUE!
        Move.ACKs      = 0;
        Move.NACKs     = 0;
        Move.SendLimit = Move.SendsLeft;
    }
    else
    {
        // This is a move which came into the server from the client.  It may
        // already be in the queue.  Check for it.

        if( m_Count && (Move.Seq <= m_Seq) )
        {
            return( FALSE );
        }

        m_Seq = Move.Seq;

        ASSERT( (m_Count == 0) || (Move.Seq > m_Move[m_Head].Seq) );
    }

    // Add the move to the head to the queue.

    m_Count       += 1;  
    m_Head         = (m_Head+1) % MAX_MOVES;
    m_Move[m_Head] = Move;
/*
    LOG_MESSAGE( "move_mgr::AddMove", 
                 "Client:%d - QSlot:%d - Count:%d - Seq:%d - Fire:%d - Toss:%d - Pain[0]:%d - Pain[1]:%d",
                 m_ClientIndex,
                 m_Move[m_Head].Slot,
                 m_Count,
                 m_Move[m_Head].Seq, 
                 m_Move[m_Head].Fire,
                 m_Move[m_Head].Toss,
                 m_Move[m_Head].Pain[0].Pain,
                 m_Move[m_Head].Pain[1].Pain );
*/
    LogQueue();
    return( TRUE );
}

//==============================================================================

void move_mgr::GetMove( move& Move )
{
    ASSERT( m_Count > 0 );
    Move     = m_Move[m_Tail];
    m_Count -= 1;
    m_Tail   = (m_Tail+1) % MAX_MOVES;
}

//==============================================================================

s32 move_mgr::GetCount( void )
{
    return( m_Count );
}

//==============================================================================

void move_mgr::Reset( void )
{
    s32 i;

    LOG_WARNING( "move_mgr::Reset", 
                 "Client:%d - Purged move manager queue.", m_ClientIndex );

    m_Head  = MAX_MOVES-1;
    m_Tail  = 0;
    m_Count = 0;
    m_Seq   = 0;

    for( i = 0; i < MAX_MOVES; i++ )
    {
        m_Move[i].SendsLeft = 0;
    }

    x_memset( &m_LastAcked, 0xFFFFFFFF, sizeof(move) );

/*
    m_LastAcked.Jump      = -1;
    m_LastAcked.Crouch    = -1;
    m_LastAcked.Reload    = -1;
    m_LastAcked.Weapon    = -1;
    m_LastAcked.Fire      = -1;
    m_LastAcked.Toss      = -1;
    m_LastAcked.Respawn   = -1;
    m_LastAcked.FirePos( F32_MAX, F32_MAX, F32_MAX );
    m_LastAcked.FireVel( F32_MAX, F32_MAX, F32_MAX );
    m_LastAcked.FireStrength = F32_MAX;
    m_LastAcked.TossPos( F32_MAX, F32_MAX, F32_MAX );
    m_LastAcked.TossVel( F32_MAX, F32_MAX, F32_MAX );
    m_LastAcked.TossStrength = F32_MAX;

    for( i=0 ; i<4 ; i++ )
    {
        m_LastAcked.AmmoClip   [i] = 0;
        m_LastAcked.AmmoReserve[i] = 0;
    }
*/
}

//==============================================================================

void move_mgr::ProvideMoves( conn_packet& Packet,
                             bitstream&   BS,
                             s32          NMovesAllowed )
{
    ASSERT( g_NetworkMgr.IsClient() );

    // Cleanse the packet.
    for( s32 i = 0; i < CONN_MOVES_PER_PACKET; i++ )
    {
        Packet.MoveID [i] = -1;
        Packet.MoveSeq[i] = -1;
    }
    Packet.NMoves = 0;

    // Reserve space in the bitstream for count.
    s32 StartCursor = BS.GetCursor();
    BS.WriteRangedS32( 0, 0, CONN_MOVES_PER_PACKET );

    // If queue is empty, then we are done!
    if( m_Count == 0 )
        return;

    //
    // Find the first move to be sent in this packet.
    //

    s32 I    = m_Tail;
    s32 Stop = (m_Head+1) % MAX_MOVES;
    do
    {
        if( m_Move[I].SendsLeft > 0 )
            break;
        I = (I+1) % MAX_MOVES;        
    }
    while( I != Stop );

    // If nothing to write, then we are done!
    if( I == Stop )
        return;

    // Write out the sequence number of the FIRST move.
    BS.WriteS32( m_Move[I].Seq );

    // Write all the moves that still have "SendsLeft" or as many moves as will 
    // fit in the packet.

    while( (NMovesAllowed--) && (m_Move[I].SendsLeft) && (I != Stop) )
    {
        // Remember where this move starts in the packet.
        s32 Cursor = BS.GetCursor();

        // Write the move itself.
        m_Move[I].Write( BS, m_LastAcked );

        // And back it all out if it didn't fit.
        if( BS.Overwrite() )
        {
            BS.SetCursor( Cursor );
            break;
        }

        // Update the packet.
        Packet.MoveID [ Packet.NMoves ] = I;
        Packet.MoveSeq[ Packet.NMoves ] = m_Move[I].Seq & 0x7FFF;

/*
        // Logging.
        LOG_MESSAGE( "move_mgr::ProvideMoves( Packing )",
                     "Move[%d] - Packet.MoveID:%d - Move.Seq:%d - Packet.MoveSeq:%d - Packet[%d]",
                     I,
                     Packet.MoveID [ Packet.NMoves ],
                     m_Move[I].Seq, 
                     Packet.MoveSeq[ Packet.NMoves ],
                     Packet.NMoves );
*/

        // Update lots of counters!
        Packet.NMoves++;
        m_Move[I].SendsLeft--;
        I = (I+1) % MAX_MOVES;
    }

    // Go back and update the number of moves written.
    s32 EndCursor = BS.GetCursor();
    BS.SetCursor( StartCursor );
    BS.WriteRangedS32( Packet.NMoves, 0, CONN_MOVES_PER_PACKET );
    BS.SetCursor( EndCursor );

    // And that's a wrap!

    LOG_MESSAGE( "move_mgr::ProvideMoves",
                 "Client:%d - InQueue:%d - Packed:%d - MoveSeq:%d,%d,%d,%d,%d,%d,%d,%d",
                 m_ClientIndex,
                 m_Count,
                 Packet.NMoves,
                 Packet.NMoves >= 1 ? Packet.MoveSeq[0] : -1,
                 Packet.NMoves >= 2 ? Packet.MoveSeq[1] : -1,
                 Packet.NMoves >= 3 ? Packet.MoveSeq[2] : -1,
                 Packet.NMoves >= 4 ? Packet.MoveSeq[3] : -1,
                 Packet.NMoves >= 5 ? Packet.MoveSeq[4] : -1,
                 Packet.NMoves >= 6 ? Packet.MoveSeq[5] : -1,
                 Packet.NMoves >= 7 ? Packet.MoveSeq[6] : -1,
                 Packet.NMoves >= 8 ? Packet.MoveSeq[7] : -1 );
    LogQueue();
}

//==============================================================================

void move_mgr::AcceptMoves( bitstream& BS )
{
    ASSERT( g_NetworkMgr.IsServer() );

    // Read the number of moves.
    s32 NMoves;
    BS.ReadRangedS32( NMoves, 0, CONN_MOVES_PER_PACKET );
    if( NMoves == 0 )
        return;

    s32     FirstSeq;
    s32     Queued = 0;
    move    Move;

    Move.Slot = m_ClientIndex;

    BS.ReadS32( FirstSeq );                  // TO DO - Need full 32 bits here?

    for( s32 i = 0; i < NMoves; i++ )
    {
        // Read the move from the packet.
        Move.Seq = FirstSeq + i;
        Move.Read( BS, m_LastAcked );

        // Add the move to the queue.
        if( AddMove( Move ) )
            Queued++;
    }

    m_LastAcked = Move;

    LOG_MESSAGE( "move_mgr::AcceptMoves",
                 "Client:%d - Received:%d - First:%d - Last:%d - Queued:%d - InQueue:%d",
                 m_ClientIndex,
                 NMoves,
                 FirstSeq,
                 FirstSeq + NMoves - 1,
                 Queued,
                 m_Count );
}

//==============================================================================

void move_mgr::PacketAck( conn_packet& Packet, xbool Arrived )
{
    s32     i;

    ASSERT( g_NetworkMgr.IsClient() );

    if( Arrived )
    {
        LOG_MESSAGE( "move_mgr::PacketAck", 
                     "Client:%d - PacketSeq:%d - Status:RECEIVED - Count:%d - MoveSeq:%d,%d,%d,%d,%d,%d,%d,%d",
                     m_ClientIndex,
                     Packet.Seq,
                     Packet.NMoves,
                     Packet.MoveSeq[0], 
                     Packet.MoveSeq[1], 
                     Packet.MoveSeq[2], 
                     Packet.MoveSeq[3], 
                     Packet.MoveSeq[4], 
                     Packet.MoveSeq[5], 
                     Packet.MoveSeq[6], 
                     Packet.MoveSeq[7] );
    }
    else
    {
        LOG_WARNING( "move_mgr::PacketAck", 
                     "Client:%d - PacketSeq:%d - Status:DROPPED - Count:%d - MoveSeq:%d,%d,%d,%d,%d,%d,%d,%d",
                     m_ClientIndex,
                     Packet.Seq,
                     Packet.NMoves,
                     Packet.MoveSeq[0], 
                     Packet.MoveSeq[1], 
                     Packet.MoveSeq[2], 
                     Packet.MoveSeq[3], 
                     Packet.MoveSeq[4], 
                     Packet.MoveSeq[5], 
                     Packet.MoveSeq[6], 
                     Packet.MoveSeq[7] );
    }

    // Update moves in queue based on this N/ACK.

    for( i = 0; i < Packet.NMoves; i++ )
    {
        s32 I = Packet.MoveID [i];
        s32 S = Packet.MoveSeq[i];

        if( S == (m_Move[I].Seq & 0x7FFF) )
        {
            if( Arrived )   m_Move[I].ACKs++;
            else            m_Move[I].NACKs++;
        }
        else
        {
            LOG_ERROR( "move_mgr::PacketAck", 
                       "Client:%d - N/ACK SEQUENCE MISMATCH - QSlot:%d - MSeq:%d - PSeq:%d",
                       m_ClientIndex, I, (m_Move[I].Seq & 0x7FFF), S );
        }
    }

    // Now, attempt to remove moves from the queue that are no longer needed.

    while( m_Count && 
           ((m_Move[m_Tail].ACKs + m_Move[m_Tail].NACKs) == m_Move[m_Tail].SendLimit) )
    {
        if( m_Move[m_Tail].ACKs == 0 )
        {
            // This move was NEVER received.  
            // We need to retransmit it.

            LOG_WARNING( "move_mgr::PacketAck", "Client:%d - LOST:%d", 
                         m_ClientIndex, m_Move[m_Tail].Seq );

            s32 Retran = m_Head;
            s32 Maybe  = (Retran + MAX_MOVES - 1) % MAX_MOVES;

            while( m_Move[Maybe].SendsLeft == m_Move[Maybe].SendLimit )
            {
                Retran = Maybe;
                Maybe  = (Retran + MAX_MOVES - 1) % MAX_MOVES;
            }

// TO DO    m_Move[Retran].AddPain( m_Move[m_Tail] );
        }
        else
        {
            m_LastAcked.Position     = m_Move[m_Tail].Position;   
            m_LastAcked.Pitch        = m_Move[m_Tail].Pitch;   
            m_LastAcked.Yaw          = m_Move[m_Tail].Yaw;   
            m_LastAcked.JumpSeq      = m_Move[m_Tail].JumpSeq; 
            m_LastAcked.LifeSeq      = m_Move[m_Tail].LifeSeq;
            m_LastAcked.Weapon       = m_Move[m_Tail].Weapon; 
            m_LastAcked.ReloadSeq    = m_Move[m_Tail].ReloadSeq; 
            m_LastAcked.FireSeq      = m_Move[m_Tail].FireSeq;   
            m_LastAcked.TossSeq      = m_Move[m_Tail].TossSeq;   
            m_LastAcked.MeleeSeq     = m_Move[m_Tail].MeleeSeq;   
/*
            m_LastAcked.Crouch       = m_Move[m_Tail].Crouch; 
            m_LastAcked.Toss         = m_Move[m_Tail].Toss;   
            m_LastAcked.Respawn      = m_Move[m_Tail].Respawn;
            m_LastAcked.FirePos      = m_Move[m_Tail].FirePos;
            m_LastAcked.FireVel      = m_Move[m_Tail].FireVel;
            m_LastAcked.FireStrength = m_Move[m_Tail].FireStrength;
            m_LastAcked.TossPos      = m_Move[m_Tail].TossPos;
            m_LastAcked.TossVel      = m_Move[m_Tail].TossVel;
            m_LastAcked.TossStrength = m_Move[m_Tail].TossStrength;

            for( i=0 ; i<4 ; i++ )
            {
                m_LastAcked.AmmoClip   [i] = m_Move[m_Tail].AmmoClip   [i];
                m_LastAcked.AmmoReserve[i] = m_Move[m_Tail].AmmoReserve[i];
            }
*/
        }
                               
        m_Count -= 1;
        m_Tail   = (m_Tail+1) % MAX_MOVES;
    }

    LogQueue();
}

//==============================================================================

#define TAIL(n)     m_Move[ (m_Tail+n) % MAX_MOVES ].Seq,  \
                    m_Move[ (m_Tail+n) % MAX_MOVES ].SendsLeft 

#define HEAD(n)     m_Move[ (m_Head+MAX_MOVES-n) % MAX_MOVES ].Seq,  \
                    m_Move[ (m_Head+MAX_MOVES-n) % MAX_MOVES ].SendsLeft

void move_mgr::LogQueue( void )
{
    switch( m_Count )
    {

    case 0:
        LOG_MESSAGE( "move_mgr::LogQueue",
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:",
                     m_ClientIndex, m_Count, m_Head, m_Tail );
        break;

    case 1:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0) );
        break;

    case 2:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1) );
        break;

    case 3:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2) );
        break;

    case 4:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3) );
        break;

    case 5:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4) );
        break;

    case 6:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5) );
        break;

    case 7:
        LOG_MESSAGE( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5), HEAD( 6) );
        break;

    case 8:
        LOG_WARNING( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5), HEAD( 6), HEAD( 7) );
        break;

    case 9:
        LOG_WARNING( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5), HEAD( 6), HEAD( 7), 
                     HEAD( 8) );
        break;

    case 10:
        LOG_WARNING( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5), HEAD( 6), HEAD( 7), 
                     HEAD( 8), HEAD( 9) );
        break;

    case 11:
        LOG_WARNING( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5), HEAD( 6), HEAD( 7), 
                     HEAD( 8), HEAD( 9), HEAD(10) );
        break;

    case 12:
        LOG_WARNING( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), 
                     HEAD( 4), HEAD( 5), HEAD( 6), HEAD( 7), 
                     HEAD( 8), HEAD( 9), HEAD(10), HEAD(11) );
        break;

    default:
        LOG_ERROR  ( "move_mgr::LogQueue", 
                     "Client:%d - Count:%d - Head:%d - Tail:%d - Queue:%d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d) ... %d(%d), %d(%d), %d(%d), %d(%d), %d(%d), %d(%d)",
                     m_ClientIndex, m_Count, m_Head, m_Tail,
                     HEAD( 0), HEAD( 1), HEAD( 2), HEAD( 3), HEAD( 4), HEAD( 5),
                     TAIL( 5), TAIL( 4), TAIL( 3), TAIL( 2), TAIL( 1), TAIL( 0) );
        break;
    }
}

#undef TAIL
#undef HEAD

//==============================================================================
#endif
