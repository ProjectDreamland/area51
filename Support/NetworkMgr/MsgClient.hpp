//==============================================================================
//
//  MsgClient.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc. All rights reserved.
//
//==============================================================================

#ifndef MSGCLIENT_HPP
#define MSGCLIENT_HPP

//==============================================================================
//==============================================================================

#include "Msg.hpp"
#include "Messages.hpp"
#include "Objects\WeaponSMP.hpp"
#include "Obj_mgr\obj_mgr.hpp"

//==============================================================================
//==============================================================================

class msg_client
{
public:
                    msg_client          ( void );
                   ~msg_client          ( void );

        void        Init                ( void );

        void        AcceptMsgs          ( bitstream& BS );
        void        AcceptMsg           ( bitstream& BS );
        void        AcceptMsg           ( msg& Msg );

        xwstring    m_SelfName;         // This is storage for the term "you" for messages referring to the player.

private:
        void        DisplayMsg          ( const msg& Msg );
const   xwchar*     GetMsgString        ( msg Msg, s32 TargetPlayer );
        xwstring    GetFormattedString  ( const msg& Msg, s32 TargetPlayer );

        void        InsertNumber        ( s32 ArgVal, xwchar* Message, s32& CursorPos );
        void        InsertString        ( const xwchar* String, xwchar* Message, s32& CursorPos );

        void        InsertColor         ( u32 Color, xwchar* Message, s32& CursorPos );
        void        InsertColor         ( u8 Red, u8 Green, u8 Blue, xwchar* Message, s32& CursorPos );
        void        InsertCharacter     ( xwchar Char, xwchar* pMessage, s32& CursorPos );
        void        InsertWord          ( const xwchar* String, xwchar* pMessage, s32& CursorPos );

        xbool       ReplaceArg          ( const msg&        Msg,
                                        s32         TargetPlayer,
                                        s32         ArgNum,
                                        xwchar*     Message,
                                        s32&        CursorPos );
        msg_impact  GetGoodOrBad        ( s32 TargetPlayer, msg Msg );


    s32             m_LastAcked;

};

//==============================================================================
//==============================================================================

extern msg_client   MsgClient;

//==============================================================================
#endif // MSGCLIENT_HPP
//==============================================================================
