//==============================================================================
//
//  MsgQueue.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef MSGQUEUE_HPP
#define MSGQUEUE_HPP

//==============================================================================
//==============================================================================

#include "Msg.hpp"
//#include "MsgMgr.hpp"

//==============================================================================
//==============================================================================

class msg_queue
{
//------------------------------------------------------------------------------
//  Public Functions

public:

                    msg_queue       ( void );
                   ~msg_queue       ( void );

        void        Activate        ( void );                    
        void        Deactivate      ( void );                    

        void        AddMsg          ( msg* pMsg );
        void        PackMsg         ( s32 PacketSeq, bitstream& BS );

        void        PacketAck       ( s32 PacketSeq, xbool Arrived );

        s32         GetPacketSeq    ( s32 QueueIndex );
        void        SetPacketSeq    ( s32 QueueIndex, s32 PacketSeqNum );
        
        s32     m_Client;
        s32     m_PacketSequenceNums[ MAX_MSG_QUEUE ];

        xbool   HasMessage[ NUM_MESSAGES ];

        s32     m_LastPacketSeq;

//------------------------------------------------------------------------------
//  Private Storage

protected: 
        xbool   m_Active;

};

//==============================================================================
#endif // MSGQUEUE_HPP
//==============================================================================

