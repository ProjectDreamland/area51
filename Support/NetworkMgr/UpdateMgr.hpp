//==============================================================================
//
//  UpdateMgr.hpp
//
//==============================================================================
/* NOTES

 - I'm not keeping "lost dirty bits" like in Tribes.  Don't really see a need
   to keep them.  Any bits which are lost are fed back into the dirty bits!

 - As an optimization, when a packet is lost, rather than ORing all of the lost
   bits back into the dirty bits, see which of the lost bits are "on route" in
   unconfirmed packets and do NOT bother setting those to be dirty.
   
 - I don't see a need to really keep track of whether or not a particular 
   object is in use in here.  If it is "deactivated" then it should never have
   any dirty bits.

 - There is the chance that an object could get destroyed and recreated and 
   have that go unnoticed in here.  The client side object, though, will know
   because of the create and destroy bits.  SHOULD WE DO ANYTHING SPECIAL IN
   HERE ABOUT THIS?  For newly created object, the create bit should NOT get
   cleared until it is confirmed.  (But what if its a different create?  Use
   sequence numbers?)  This entire area is a little "mushy" right now.

 - Is it possible for the update manager to be used when there is no client?
   Tribes version has a lot of checks to that effect.  Shouldn't server just
   not invoke the update manager if there is no client?

 - When a new client connects, the objects which ALREADY EXIST will not have
   their ACTIVATE bits set.  The game server may need to force this.  Or,
   there could be a special way of starting up an update manager so that it
   automatically determines this and adds the needed bits.

*/
//==============================================================================

#ifndef UPDATEMGR_HPP
#define UPDATEMGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetObjMgr.hpp"
#include "ConnMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define MAX_UPDATES     256

//==============================================================================
//  TYPES
//==============================================================================

class update_mgr
{
//------------------------------------------------------------------------------
//  Public Types
//------------------------------------------------------------------------------

public:

//------------------------------------------------------------------------------
//  Private Types
//------------------------------------------------------------------------------

protected:

    struct update
    {
        u32     Bits;
        s16     Slot;
        s16     PacketSeq;
        s16     PacketNext;
        s16     Next;
        s16     Prev;
    };

    enum
    {
        USED_LIST = MAX_UPDATES-1,
        FREE_LIST = MAX_UPDATES-2,
        LIST_LAST = MAX_UPDATES-3,
    };

//------------------------------------------------------------------------------
//  Private Storage
//------------------------------------------------------------------------------

protected:

        s32         m_ClientIndex;  

        update      m_Update[ MAX_UPDATES ];
        s32         m_Used;
        s32         m_Free;
        s32         m_Cursor;
        s32         m_PlayerCursor;

        u32         m_ObjBits[ NET_MAX_OBJECTS ];

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------

protected:

        void        ResetUpdates    ( void );

        s32         AllocUpdate     ( s32 Slot  );
        void        ReleaseUpdate   ( s32 Index );

        xbool       ProvideObjUpdate( s32          Slot, 
                                      conn_packet& Packet,
                                      bitstream&   BitStream, 
                                      s32&         NUpdatesAllowed );

        netobj*     CreateNetObj    ( s32          Slot, 
                                      netobj::type Type );

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:

                    update_mgr      ( void );
                   ~update_mgr      ( void );

        void        Init            ( s32 ClientIndex );
        void        Kill            ( void );
        void        Reset           ( void );
        void        SyncActivates   ( void );

        void        DumpState       ( void );

        void        AcceptUpdates   ( bitstream&    BitStream       );
        void        ProvideUpdates  ( conn_packet&  Packet,
                                      bitstream&    BitStream,
                                      s32           NUpdatesAllowed );

        void        AddDirtyBits    ( s32 Slot, u32 DirtyBits );
        void        ObjectRemoved   ( s32 Slot );

        void        PacketAck       ( conn_packet& Packet, xbool Arrived );

        void        Sanity          ( void );
};

//==============================================================================
#endif // UPDATEMGR_HPP
//==============================================================================
