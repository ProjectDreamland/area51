//==============================================================================
//
//  Msg.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_Bitstream.hpp"

#include "Messages.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class msg
{
public:
    s32      m_MsgID;        // Message type indexed into MSG_TYPES.
    msg_hear m_Audience;
    s32      m_Target;       // Recipient or group if not for everybody.
    s32      m_MsgSeq;

    // Only used for goals!
    s32     m_MsgGoalNum;   // This is used to explicitly reference a goal
    xbool   m_Enabled;      // Whether we are turning on or off a particular goal.
    
    // Used for goals and bonus:
    f32     m_Time;         // How long we want this to stick around, 
                            //-1 means it must be explicitly deleted

    s32     m_Data[ MAX_MSG_ARGS ];
    xwchar  m_StringData[ MAX_MSG_STRINGS ][ MAX_STRING_LENGTH ];

    xbool   m_bIsLocal;

    xbool   Write               (       bitstream& BS, xbool bIncludeInfo );
    xbool   Read                ( const bitstream& BS );    


    void    DeliverToClient     ( s32& ClientIndex, xbool bDeliver );
    xbool   NeedsDelivering     ( s32& ClientIndex ) const;
    xbool   DeliveredToAll      ( void ) const;
    
    void    ClearDelivered      ( void );

    void    SetStringData       ( s32 ArgNum, const xwchar* StringData );   
    void    GetStringData       ( s32 ArgNum, xwchar* StringData )  const;  
    void    SetArg              ( s32 ArgNum, s32 ArgVal ); 
    xbool   IsValidTarget       ( s32 TestTarget ) const  ;

    //void    ReadWString         ( const bitstream& BS,       xwchar* String ); 
    //void    WriteWString        (       bitstream& BS, const xwchar* String );

private:
    u32     m_BitfieldDelivered;  // One bit per unacked client recipient.
};

//==============================================================================

inline
void msg::ClearDelivered( void )
{
    m_BitfieldDelivered = 0;
}

//==============================================================================
#endif // MESSAGE_HPP
//==============================================================================
