//==============================================================================
//
//  MsgQueue.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "MsgQueue.hpp"
#include "MsgMgr.hpp"

#include "x_log.hpp"

#define MSG_INF_RECEIVED -2

// Performance tracking code.
/*
s32 g_Retransmits = 0;  
*/

//==============================================================================
//  FUNCTIONS
//==============================================================================

msg_queue::msg_queue( void )
{
    Deactivate();

}

//==============================================================================

msg_queue::~msg_queue( void )
{
}

//==============================================================================

void msg_queue::Activate( void )
{
    m_Active                = TRUE;
    m_LastPacketSeq         = 0;
    
    s32 i;
    for( i = 0; i < MAX_MSG_QUEUE; i++ ) 
    {
        m_PacketSequenceNums[ i ] = -1;
    }

    // Loop through all the messages currently in the msgmgr queue and make sure
    // none of them ever try to go through this msg_queue again.  This will
    // keep messages destined for one client from unexpectedly arriving at the 
    // next client to take his slot.
    for( i = MsgMgr.m_StartOfQueue; i < MsgMgr.m_EndOfQueue; i++ )
    {
        msg Msg = MsgMgr.GetMsg( i );
        Msg.DeliverToClient( m_Client, FALSE );
        MsgMgr.SetMsg( i, Msg );
    }

    for( i = 0; i < NUM_MESSAGES; i++ )
    {
        HasMessage[ i ] = -1;    
    }
}

//==============================================================================

void msg_queue::Deactivate( void )
{
    m_Active = FALSE;
}

//==============================================================================

void msg_queue::PackMsg( s32 PacketSeq, 
                         bitstream& BS )
{
    if( !m_Active )
    {
        Activate();
    }
    
    s32 i = -1;

    // 
    // Only send if we haven't sent in the last RETRY_DELAY packets.  This cuts
    // down on redundant data going over the wire and shouldn't ever be noticeable
    // (except in high packet loss situations).
    //
    if( (PacketSeq - m_LastPacketSeq) >= RETRY_DELAY )
    {
        /*
        CLOG_MESSAGE( (MsgMgr.m_EndOfQueue - MsgMgr.m_StartOfQueue) > 0, 
                      "msg_queue::PackMsg", 
                      "Client %i - Packet %d - Writing active messages from %d to %d.", 
                      m_Client, PacketSeq, MsgMgr.m_StartOfQueue, MsgMgr.m_EndOfQueue - 1 );
        */

        // Now put the messages into the stream.
        for( i = MsgMgr.m_StartOfQueue; i < MsgMgr.m_EndOfQueue; i++ )
        {
            msg Msg = MsgMgr.GetMsg( i );
            
            if( Msg.NeedsDelivering( m_Client ) )
            {
                // If this message is created at runtime, and the client doesn't have it, make sure to send the prototype.
                // Note that once the info has been received, HasMessage will be set to MSG_INF_RECEIVED (-2)
                xbool bIncludeInfo = 
                    !(MsgTable[ Msg.m_MsgID ].m_Original) && 
                    (HasMessage[ Msg.m_MsgID ] >= -1);
                
                if ( Msg.Write( BS, bIncludeInfo ) )
                {             
                    m_LastPacketSeq = PacketSeq;

                    // Write down only the first packet this msg was sent in.
                    if( GetPacketSeq( i ) == -1 )
                    {
                        SetPacketSeq( i, PacketSeq );
                    }
                    else
                    {
                       LOG_WARNING( "MsgQueue::PackMsg", 
                            "Retransmitting msg %d", 
                            Msg.m_MsgSeq );

                       // Performance tracking code.
                       /*
                       g_Retransmits++;
                       */
                    }

                    // Write down the packet seq if we sent the info for this message over.
                    if( bIncludeInfo )
                    {
                        HasMessage[ Msg.m_MsgID ] = PacketSeq;  
                    }
                }
                else 
                {
                    LOG_WARNING( "MsgQueue::PackMsg", 
                        "Packet %d: Not enough room for message %d!", 
                        PacketSeq, Msg.m_MsgSeq );

                    BS.WriteMarker();
                    
                    // Note that in this case Msg.Write would have 
                    // written the final 0 flag so we just return.
                    return;
                }
            }
        }
    }

    // To signal no more messages are coming.
    BS.WriteFlag( FALSE );
}

//==============================================================================

void msg_queue::PacketAck( s32 PacketSeq, xbool Arrived )
{
    if( !m_Active )
    {
        return;
    }

    // If the packet didn't arrive or we're getting a nack for an old packet, 
    // discard.
    if( Arrived )
    {   
        // Go through and see if this clears any message infos that needed to be sent                
        s32 j;
        for( j = 0; j < NUM_MESSAGES; j++ )
        {
            if( PacketSeq == HasMessage[ j ] )
            {
                HasMessage[ j ] = MSG_INF_RECEIVED;
            }
        }
        

        s32 LastAcked = -1;
        s32 FirstAcked = -1;

        // Loop through all messages the MsgMgr is keeping track of currently.
        s32 i;
        for( i = MsgMgr.m_StartOfQueue; i < MsgMgr.m_EndOfQueue; i++ )
        {
            s32 MsgPacketSeq = GetPacketSeq( i ); 
            
            // Check and see if this msg was delivered.
            if( (PacketSeq >= MsgPacketSeq) &&
                ((PacketSeq - MsgPacketSeq) % RETRY_DELAY == 0) &&
                (MsgPacketSeq != -1) )
            {
                SetPacketSeq( i, -1 );
                
                msg Msg = MsgMgr.GetMsg( i );
                Msg.DeliverToClient( m_Client, FALSE );
                MsgMgr.SetMsg( i, Msg );
                if( FirstAcked == -1 )
                {
                    FirstAcked = i;
                }
                LastAcked = i;

            }
        }
        if( FirstAcked != -1 )
        {
            /*
            LOG_MESSAGE( "msg_queue::PacketAck", 
                "Client %d, Packet %d: acked msgs %d to %d.", 
                 m_Client, PacketSeq, FirstAcked, LastAcked );
            */
        }
    }
}        

//==============================================================================

s32 msg_queue::GetPacketSeq( s32 QueueIndex )
{
    ASSERT( IN_RANGE( MsgMgr.m_StartOfQueue, QueueIndex, MsgMgr.m_EndOfQueue ) );
   
    s32 ArrayIndex = QueueIndex % MAX_MSG_QUEUE;    
    return m_PacketSequenceNums[ ArrayIndex ];
}

//==============================================================================

void msg_queue::SetPacketSeq( s32 QueueIndex, s32 PacketSeqNum )
{
    ASSERT( IN_RANGE( MsgMgr.m_StartOfQueue, QueueIndex, MsgMgr.m_EndOfQueue ) );
    
    s32 ArrayIndex = QueueIndex % MAX_MSG_QUEUE;
    m_PacketSequenceNums[ ArrayIndex ] = PacketSeqNum;
}

//==============================================================================

