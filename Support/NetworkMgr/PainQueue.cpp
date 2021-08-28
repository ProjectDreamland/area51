//==============================================================================
//
//  PainQueue.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//
//  NOTES:
//
//  - Assuming that pains received in a single packet all have sequential
//    sequence numbers.  WARNING: Is this always true?
//
//  - TO DO: If (statistically unlikely) a death pain completely fails to arrive
//    at the server, then we must somehow decide how to deal with the locally 
//    speculatively dead player.  Perhaps when the "lost pain" is over-written,
//    we can force the "dead" player to be alive again.  Alternately, if the 
//    pain queue fills up, no more pain.
//
//  - Need to CAREFULLY check out the pain sequence number system.
//
//  - HOW TO DEAL WITH LOST PACKETS
//
//    + Version 1 - On the receiving side, only accept pains which have the next 
//      sequential number.  When we get a NACK, mark ALL pains from the lost 
//      point forward as unsent and send them again.  
//
//    + Version 2 - On the receiving side, accept "future" pains, but only apply
//      them in strict sequential order.  Thus, a lost packet will make the pain
//      applications stall until the lost data is resent and received.  On the 
//      sending side, when we get a NACK, mark only the lost pains as unsent.
//      When sending, always send the oldest unsent pains, AND only send groups
//      of pains with strictly sequential numbers (this will help with 
//      compression).
//
//  - Consider reducing CONN_PAINS_PER_PACKET from 8 to 7.  This will save a
//    bit per packet.  Search for other such optimizations.
//
//==============================================================================

#if !defined(mtraub)
#define X_SUPPRESS_LOGS
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "PainQueue.hpp"
#include "NetworkMgr.hpp"
#include "ConnMgr.hpp"

#include "x_debug.hpp"
#include "x_log.hpp"

//==============================================================================
//  DEFINES
//==============================================================================


//==============================================================================
//  NET_PAIN FUNCTIONS
//==============================================================================

net_pain::net_pain( void )
{
    Seq  = -1;
    Sent = FALSE;
}

//==============================================================================

void net_pain::Read( bitstream& BS )
{
    xbool Flag;
    s32   Value;

    if( BS.ReadFlag() )    BS.ReadRangedS32( Value, 0, 31 );
    else                   Value = -1;
    Origin = Value;

    BS.ReadRangedS32( Value, 0, 31 );
    Victim = Value;

    BS.ReadS16( PainType );

    BS.ReadRangedS32( Value, 0, 7 );
    LifeSeq = Value;

    BS.ReadFlag( Flag );
    Kill = (s8)Flag;
    if( Kill )
    {
        Damage = 1000.0f;

        // Read extra corpse pain
        CorpseDeathPain.Read( BS );      
    }
    else
    {
        s16 Fixed;
        BS.ReadS16( Fixed );
        Damage = ((f32)Fixed) / 256.0f;
    }
}

//==============================================================================

void net_pain::Write( bitstream& BS )
{
    xbool Flag = Kill;
    s32   Value;

    if( BS.WriteFlag( Origin != -1 ) )
    {
        Value = Origin;
        BS.WriteRangedS32( Value, 0, 31 );
    }

    Value = Victim;
    BS.WriteRangedS32( Value, 0, 31 );

    BS.WriteS16( PainType );

    Value = LifeSeq;
    BS.WriteRangedS32( Value, 0, 7 );

    BS.WriteFlag( Flag );

    if( !Kill )
    {
        Damage = MIN( Damage, 127.0f );
        s16 Fixed = (s16)(Damage * 256.0f);
        BS.WriteS16( Fixed );
    }
    else
    {
        // Write extra corpse pain
        CorpseDeathPain.Write( BS );      
    }
}

//==============================================================================
//  PAIN_QUEUE FUNCTIONS
//==============================================================================

pain_queue::pain_queue( void )
{
    m_ClientIndex = -1;
    Reset();
}

//==============================================================================

pain_queue::~pain_queue( void )
{
}

//==============================================================================

void pain_queue::Init( s32 ClientIndex )
{
    m_ClientIndex = ClientIndex;
    Reset();
}

//==============================================================================

xbool pain_queue::AddPain( net_pain& Pain )
{
    if( m_Count == MAX_ENTRIES )
    {
        ASSERT( FALSE );

        // Umm...  Crap.  Now what?
        //
        // Well, why are we here?  There have been MAX_ENTRIES (128?) pain 
        // events against other players on this client machine which have NOT
        // yet been transmitted to and confirmed on the server.  In all 
        // likelihood, we are having some non-trivial problems at this point.
        // So, applying too much effort at best possible recovery may be a 
        // waste.  So...  For now, do NOTHING!
        //
        // But, what COULD be done?
        //  - Attempt to combine compatible unsent pain entries to make room.
        //  - Attempt to discard the OLDEST unsent pain entry.
        //
        // Beware: If we "lose" a KILL, this client could be left in a somewhat
        // problematic state until the "locally dead" player actually dies on
        // the server.  Workaround: Revive the "locally dead" player.
    }

    s32 Slot = (m_Head+1) % MAX_ENTRIES;

    if( Pain.Seq == -1 )
    {
        // This is pain caused by a local client player and queued for later
        // transmission to the server.  Set the sequence number.
        Pain.Seq = Slot;
    }
    else
    {
        // This is pain which came into the server from the client.  Make sure
        // it is the next pain in the sequence.

        if( Pain.Seq != Slot )
        {
            // The pain we received is not the next pain.  Reject it.
            LOG_WARNING( "pain_queue::AddPain", 
                         "Client:%d - Unexpected pain sequence value - "
                         "Expected:%d - Received:%d",
                         m_ClientIndex, Slot, Pain.Seq );
            return( FALSE );
        }
    }

    // Add the pain to the head to the queue.

    m_Count       += 1;  
    m_Head         = Slot;
    m_Pain[m_Head] = Pain;

    /*
    LOG_MESSAGE( "pain_queue::AddPain", 
                 "Client:%d - Count:%d - Seq:%d - Victim:%d - Damage:%d",
                 m_ClientIndex,
                 m_Count,
                 m_Pain[m_Head].Seq, 
                 m_Pain[m_Head].Victim,
                 m_Pain[m_Head].Damage );
    */

    LogQueue();
    return( TRUE );
}

//==============================================================================

void pain_queue::GetPain( net_pain& NetPain )
{
    ASSERT( m_Count > 0 );
    NetPain  = m_Pain[m_Tail];
    m_Count -= 1;
    m_Tail   = (m_Tail+1) % MAX_ENTRIES;
}

//==============================================================================

s32 pain_queue::GetCount( void )
{
    return( m_Count );
}

//==============================================================================

void pain_queue::Reset( void )
{
    s32 i;

    LOG_WARNING( "pain_queue::Reset", 
                 "Client:%d - Purged pain queue.", m_ClientIndex );

    m_Head  = MAX_ENTRIES-1;
    m_Tail  = 0;
    m_Count = 0;

    for( i = 0; i < MAX_ENTRIES; i++ )
    {
        m_Pain[i].Sent = FALSE;
    }
}

//==============================================================================

void pain_queue::ProvidePain( conn_packet& Packet,
                              bitstream&   BS,
                              s32          NPainsAllowed )
{
    ASSERT( g_NetworkMgr.IsClient() );

    // Cleanse the packet.
    for( s32 i = 0; i < CONN_PAINS_PER_PACKET; i++ )
    {
        Packet.PainID[i] = -1;
    }
    Packet.NPains = 0;

    // Reserve space in the bitstream for count.
    s32 StartCursor = BS.GetCursor();
    BS.WriteRangedS32( 0, 0, CONN_PAINS_PER_PACKET );

    // If queue is empty, then we are done!
    if( m_Count == 0 )
        return;

    //
    // Find the first pain to be sent in this packet.
    //

    s32 I    = m_Tail;
    s32 Stop = (m_Head+1) % MAX_ENTRIES;
    do
    {
        if( !m_Pain[I].Sent )
            break;
        I = (I+1) % MAX_ENTRIES;        
    }
    while( I != Stop );

    // If nothing to write, then we are done!
    if( I == Stop )
        return;

    // Write out the sequence number of the FIRST pain.
    BS.WriteRangedS32( I, 0, MAX_ENTRIES-1 );

    // Write all the pain that are "not Sent" or as many as will fit in the 
    // packet.  Note that the items written must all be consecutive.

    while( (NPainsAllowed--) && (!m_Pain[I].Sent) && (I != Stop) )
    {
        // Remember where this pain starts in the packet.
        s32 Cursor = BS.GetCursor();

        // Write the pain itself.
        m_Pain[I].Write( BS );

        // And back it all out if it didn't fit.
        if( BS.Overwrite() )
        {
            BS.SetCursor( Cursor );
            break;
        }

        // Update the packet.
        Packet.PainID[ Packet.NPains ] = I;

        // Logging.
        /*
        LOG_MESSAGE( "pain_queue::ProvidePain( Packing )",
                     "Pain[%d] - Packet[%d]",
                     I,
                     Packet.NPains );
        */

        // Update bookkeeping!
        m_Pain[I].Sent = TRUE;
        Packet.NPains++;
        I = (I+1) % MAX_ENTRIES;
    }

    // Go back and update the number of pains written.
    s32 EndCursor = BS.GetCursor();
    BS.SetCursor( StartCursor );
    BS.WriteRangedS32( Packet.NPains, 0, CONN_PAINS_PER_PACKET );
    BS.SetCursor( EndCursor );

    // And that's a wrap!

    /*
    LOG_MESSAGE( "pain_queue::ProvidePain",
                 "Client:%d - InQueue:%d - Packed:%d - "
                 "PainSeq:%d,%d,%d,%d,%d,%d,%d",
                 m_ClientIndex,
                 m_Count,
                 Packet.NPains,
                 Packet.NPains >= 1 ? Packet.PainID[0] : -1,
                 Packet.NPains >= 2 ? Packet.PainID[1] : -1,
                 Packet.NPains >= 3 ? Packet.PainID[2] : -1,
                 Packet.NPains >= 4 ? Packet.PainID[3] : -1,
                 Packet.NPains >= 5 ? Packet.PainID[4] : -1,
                 Packet.NPains >= 6 ? Packet.PainID[5] : -1,
                 Packet.NPains >= 7 ? Packet.PainID[6] : -1 );
    LogQueue();
    */
}

//==============================================================================

void pain_queue::AcceptPain( bitstream& BS )
{
    ASSERT( g_NetworkMgr.IsServer() );

    // Read the number of pains.
    s32 NPains;
    BS.ReadRangedS32( NPains, 0, CONN_PAINS_PER_PACKET );
    if( NPains == 0 )
        return;

    s32      FirstSeq;
    s32      Queued = 0;
    net_pain Pain;

    BS.ReadRangedS32( FirstSeq, 0, MAX_ENTRIES-1 );

    for( s32 i = 0; i < NPains; i++ )
    {
        // Read the pain from the packet.
        Pain.Read( BS );
        Pain.Seq = (FirstSeq + i) % MAX_ENTRIES;

        // Add the pain to the queue.
        if( AddPain( Pain ) )
            Queued++;
    }

    /*
    LOG_MESSAGE( "pain_queue::AcceptPain",
                 "Client:%d - Received:%d - First:%d - "
                 "Last:%d - Queued:%d - InQueue:%d",
                 m_ClientIndex,
                 NPains,
                 FirstSeq,
                 FirstSeq + NPains - 1,
                 Queued,
                 m_Count );
    */
}

//==============================================================================

void pain_queue::PacketAck( conn_packet& Packet, xbool Arrived )
{
    s32     i;

    ASSERT( g_NetworkMgr.IsClient() );

    /*
    if( Arrived )
    {
        LOG_MESSAGE( "pain_queue::PacketAck", 
                     "Client:%d - PacketSeq:%d - Status:RECEIVED - "
                     "Count:%d - PainSeq:%d,%d,%d,%d,%d,%d,%d",
                     m_ClientIndex,
                     Packet.Seq,
                     Packet.NPains,
                     Packet.PainID[0], 
                     Packet.PainID[1], 
                     Packet.PainID[2], 
                     Packet.PainID[3], 
                     Packet.PainID[4], 
                     Packet.PainID[5], 
                     Packet.PainID[6] );
    }
    else
    {
        LOG_WARNING( "pain_queue::PacketAck", 
                     "Client:%d - PacketSeq:%d - Status:DROPPED - "
                     "Count:%d - PainSeq:%d,%d,%d,%d,%d,%d,%d",
                     m_ClientIndex,
                     Packet.Seq,
                     Packet.NPains,
                     Packet.PainID[0], 
                     Packet.PainID[1], 
                     Packet.PainID[2], 
                     Packet.PainID[3], 
                     Packet.PainID[4], 
                     Packet.PainID[5], 
                     Packet.PainID[6] );
    }
    */

    // Update pains in queue based on this N/ACK.  Remove pains from the queue 
    // that are no longer needed.

    if( Arrived )
    {
        for( i = 0; i < Packet.NPains; i++ )
        {
            ASSERT( m_Count );
            if( m_Count == 0 )
                break;                  // Recover from ASSERT failure.

            s32 I = Packet.PainID[i];
            if( I == m_Tail )
            {
                m_Count -= 1;
                m_Tail   = (m_Tail+1) % MAX_ENTRIES;
            }
            else
            {
                LOG_ERROR( "pain_queue::PacketAck", 
                           "Client:%d - N/ACK SEQUENCE MISMATCH - Received:%d - Expected:%d",
                           m_ClientIndex, I, m_Tail );
            }
        }        
    }
    else if( Packet.NPains )
    {
        // Must resend everything that has been Sent.

        s32 I    = m_Tail;
        s32 Stop = (m_Head+1) % MAX_ENTRIES;
        do
        {
            if( !m_Pain[I].Sent )
                break;
            m_Pain[I].Sent = FALSE;
            I = (I+1) % MAX_ENTRIES;        
        }
        while( I != Stop );
    }

    LogQueue();
}

//==============================================================================

#define TAIL(n)     m_Pain[ (m_Tail+n) % MAX_ENTRIES ].Seq,  \
                    m_Pain[ (m_Tail+n) % MAX_ENTRIES ].Sent 

#define HEAD(n)     m_Pain[ (m_Head+MAX_ENTRIES-n) % MAX_ENTRIES ].Seq,  \
                    m_Pain[ (m_Head+MAX_ENTRIES-n) % MAX_ENTRIES ].Sent

void pain_queue::LogQueue( void )
{
}

#undef TAIL
#undef HEAD

//==============================================================================
