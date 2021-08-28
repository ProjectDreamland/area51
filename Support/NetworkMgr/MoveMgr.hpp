//==============================================================================
//
//  PainQueue.hpp
//
//==============================================================================

#ifndef PAIN_QUEUE_HPP
#define PAIN_QUEUE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

struct net_pain
{
    s32     SourceSlot;     // Could be from environmental hazard like fire.
    s32     VictimSlot;
    s32     DeltaHealth;
    xbool   Kill;
    s32     PainIndex;
    f32     Severity;
    xbool   UseYaw;         // Otherwise use source.
    s32     AbsoluteYaw;
};

#include "Move.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define MAX_ENTRIES   128

//==============================================================================
//  TYPES
//==============================================================================

struct  conn_packet;
class   bitstream;

//==============================================================================


//==============================================================================

class pain_queue
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:

                    pain_queue      ( void );
                   ~pain_queue      ( void ); 

        void        Init            ( s32 ClientIndex );
        void        Reset           ( void );

        xbool       AddPain         ( net_pain& NetPain );
        void        GetPain         ( net_pain& NetPain );

        s32         GetCount        ( void );

        void        AcceptPain      ( bitstream&   BitStream );
        void        ProvidePain     ( conn_packet& Packet, 
                                      bitstream&   BitStream,
                                      s32          NPainsAllowed );

        void        PacketAck       ( conn_packet& Packet, xbool Arrived );

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------

protected:

        void        LogQueue        ( void );

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------

protected:

        net_pain    m_Pain[ MAX_ENTRIES ];
        s32         m_Head;
        s32         m_Tail;
        s32         m_Count;
        s32         m_Seq;

        s32         m_ClientIndex;
};

//==============================================================================
#endif // PAIN_QUEUE_HPP
//==============================================================================
