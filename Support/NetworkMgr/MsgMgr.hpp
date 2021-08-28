//==============================================================================
//
//  MsgMgr.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef MSGMGR_HPP
#define MSGMGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_bitstream.hpp"

#include "Msg.hpp"  
#include "Messages.hpp"
#include "MsgQueue.hpp"

//==============================================================================
//  MSG MGR
//==============================================================================

class msg_mgr
{

public:                                                                                                     
        //--------------------------------------------------------------------------
        //  Public Functions
                msg_mgr             ( void );
               ~msg_mgr             ( void );

        void    Message             ( msg_id    MsgID,
                                      s32       Target,
                                      s32       arg1    = -12345,
                                      s32       arg2    = -12345,
                                      s32       arg3    = -12345,
                                      s32       arg4    = -12345,
                                      f32       Time    = 5.0f,
                                      s32       GoalID  = -1,
                                      xbool     Enabled = TRUE );

        msg_id  GetRandomMessageID  ( msg_category MsgCat );

        void    PlayerDied          ( s32 Victim, s32 Killer, s32 PainType );

        void    PacketAck           ( s32 PacketSeq, xbool Arrived, s32 PlayerIndex );
        void    ProvideMsg          ( s32 PacketSeq, bitstream& BS, s32 PlayerIndex );   

        //--------------------------------------------------------------------------
        //  Public Storage

        // The sequence number for the oldest message hanging around.
        s32         m_StartOfQueue;  

        // Sequence number for the most recent message added.
        s32         m_EndOfQueue;

        s32         m_LastUnused;

        msg         GetMsg          ( s32 QueueIndex );
        void        SetMsg          ( s32 QueueIndex, msg& Msg );

        msg_id      RegMsg          ( msg_id        MsgID, 
                                      msg_hear      Audience,
                                      msg_impact    Impact,
                                      msg_arg       Type1,
                                      msg_arg       Type2,
                                      msg_arg       Type3,
                                      msg_arg       Type4,
                                      const xwchar* pMsgString0,
                                      const xwchar* pMsgString1,
                                      const xwchar* pMsgString2,
                                      const char*   pMsgSound,
                                      const char*   pMsgSound2,
                                      msg_type      MsgType  = NORMAL,
                                      xbool         Active   = TRUE );

        msg_id      RegMsg          ( msg_info MsgPrototype );
const   xwchar*     GetStr          ( const char* pStrName );  

        void        NewClient       ( s32 ClientSlot );   
        void        Init            ( void );
        void        Reset           ( void );

private:
        //--------------------------------------------------------------------------
        //  Private Functions
        xbool       IsLocalTarget   ( s32  Target );
        void        AddMsg          ( msg& Msg );
        void        CleanQueue      ( void );

        //--------------------------------------------------------------------------
        //  Private Storage
        msg_queue   m_Queue   [ MAX_CLIENTS ];
        msg         m_Messages[ MAX_MSG_QUEUE ];
};

//==============================================================================
//  STORAGE
//==============================================================================

extern msg_mgr  MsgMgr;  

//==============================================================================
#endif // MSGMGR_HPP
//==============================================================================
