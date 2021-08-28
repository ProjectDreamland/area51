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

#include "x_types.hpp"
#include "Objects\CorpsePain.hpp"


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

struct net_pain
{
    // These are always read/written
    s16         Origin;         // Pain origin player (-1 for hazards).
    s16         Victim;         // Slot of pain recipient.
    s16         PainType;       // Needed for kill msg.
    s16         LifeSeq;        // LifeSeq of victim.  (Or SlotSeq?)
    f32         Damage;         // If not a kill, amount of damage.
    s16         Seq;            // Utility variable.
    s8          Kill;           // BOOLEAN - TRUE if pain was fatal.
    s8          Sent;           // BOOLEAN - Utility variable.

    // This is only read/written if this pain killed the victim
    // so that the ragdoll has all the info it needs to re-construct
    // the death impact force.
    corpse_pain CorpseDeathPain;
    
    // Functions    
                net_pain            ( void );
    void        Read                ( bitstream& BitStream );
    void        Write               ( bitstream& BitStream );
};

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
                                
        xbool       AddPain         ( net_pain& Pain );
        void        GetPain         ( net_pain& Pain );

        s32         GetCount        ( void );

        void        AcceptPain      ( bitstream&   BitStream );
        void        ProvidePain     ( conn_packet& Packet, 
                                      bitstream&   BitStream,
                                      s32          NPainsAllowed );

        void        PacketAck       ( conn_packet& Packet, xbool Arrived );

        void        ReadEntry       ( s32 Index );
        void        WriteEntry      ( s32 Index );

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
    //  s32         m_Seq;

        s32         m_ClientIndex;
};

//==============================================================================
#endif // PAIN_QUEUE_HPP
//==============================================================================
